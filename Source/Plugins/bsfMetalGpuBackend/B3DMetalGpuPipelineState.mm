//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DMetalGpuPipelineState.h"
#include "B3DMetalGpuDevice.h"
#include "B3DMetalGpuProgram.h"
#include "B3DMetalUtility.h"
#include "B3DMetalVertexInputManager.h"
#include "GpuBackend/B3DVertexDescription.h"
#include "Threading/B3DThreading.h"
#include "Profiling/B3DRenderStats.h"
#include "Debug/B3DLog.h"

namespace b3d
{
	namespace render
	{
		struct MetalGpuGraphicsPipelineState::Impl
		{
			id<MTLDepthStencilState> DepthStencilState = nil;

			// Per-variant cache entry. Compilation is driven by the async @c completionHandler variant
			// of @c newRenderPipelineStateWithDescriptor:, so an entry goes through a pending state
			// (Ready == false, Pipeline == nil) before the completion handler fills in the result and
			// flips Ready. Concurrent callers that arrive while a compile is in flight find the entry
			// not-ready, unlock, and wait on @c PipelineCacheCV until the handler notifies — no
			// duplicate compiles for the same key. Prewarm starts this work without waiting; draw-time
			// lookup waits only when the requested variant is still pending.
			struct CachedVariant
			{
				id<MTLRenderPipelineState> Pipeline = nil;
				bool Ready = false;
			};

			Mutex PipelineCacheMutex;
			ConditionVariable PipelineCacheCV;
			UnorderedMap<MetalPipelineVariantKey, CachedVariant, MetalPipelineVariantKeyHash> Pipelines;

		};

		MetalGpuGraphicsPipelineState::MetalGpuGraphicsPipelineState(MetalGpuDevice& gpuDevice, const GpuGraphicsPipelineStateCreateInformation& createInformation)
			: GpuGraphicsPipelineState(gpuDevice, createInformation)
			, mGpuDevice(gpuDevice)
			, mImpl(B3DMakeUnique<Impl>())
		{ }

		MetalGpuGraphicsPipelineState::~MetalGpuGraphicsPipelineState()
		{
			if (mImpl)
			{
				{
					// Drain any in-flight compiles so completion handlers cannot reference a freed Impl.
					Lock lock(mImpl->PipelineCacheMutex);
					mImpl->PipelineCacheCV.wait(lock, [this]{
						for (auto& entry : mImpl->Pipelines)
						{
							if (!entry.second.Ready)
								return false;
						}
						return true;
					});

					for (auto& entry : mImpl->Pipelines)
					{
#if !__has_feature(objc_arc)
						[entry.second.Pipeline release];
#endif
						entry.second.Pipeline = nil;
					}
					mImpl->Pipelines.clear();
				}
#if !__has_feature(objc_arc)
				[mImpl->DepthStencilState release];
#endif
				mImpl->DepthStencilState = nil;
			}

		}

		id<MTLDepthStencilState> MetalGpuGraphicsPipelineState::GetMetalDepthStencilState() const
		{
			return mImpl->DepthStencilState;
		}

		static void FillStencilDescriptor(MTLStencilDescriptor* desc, const DepthStencilStateInformation& state, bool front, u8 readMask, u8 writeMask)
		{
			desc.readMask = readMask;
			desc.writeMask = writeMask;

			if (front)
			{
				desc.stencilCompareFunction = MetalUtility::GetCompareFunction(state.FrontStencilComparisonFunc);
				desc.stencilFailureOperation = MetalUtility::GetStencilOperation(state.FrontStencilFailOp);
				desc.depthFailureOperation = MetalUtility::GetStencilOperation(state.FrontStencilZFailOp);
				desc.depthStencilPassOperation = MetalUtility::GetStencilOperation(state.FrontStencilPassOp);
			}
			else
			{
				desc.stencilCompareFunction = MetalUtility::GetCompareFunction(state.BackStencilComparisonFunc);
				desc.stencilFailureOperation = MetalUtility::GetStencilOperation(state.BackStencilFailOp);
				desc.depthFailureOperation = MetalUtility::GetStencilOperation(state.BackStencilZFailOp);
				desc.depthStencilPassOperation = MetalUtility::GetStencilOperation(state.BackStencilPassOp);
			}
		}

		void MetalGpuGraphicsPipelineState::Initialize()
		{
			// Descriptor creation below autoreleases several Obj-C temporaries. Under the fiber
			// scheduler there's no implicit runloop drain between invocations; wrap locally so
			// these temporaries are reclaimed at function exit.
			@autoreleasepool
			{
			// Vertex streams live at a fixed base slot above the parameter-set argument buffers (see
			// kMetalVertexBufferSlotBase). A fixed constant — rather than the parameter-set count —
			// keeps the MTLVertexDescriptor cached inside a MetalVertexInput valid across pipelines
			// with differing set counts, and lets the command buffer compute a stream's slot without
			// consulting this pipeline's layout. (The previous set-count-derived value was also read
			// before the base Initialize() created mParameterLayout, so it always evaluated to 0 and
			// vertex streams would have collided with argument-buffer slot 0.)
			mVertexBufferBaseIndex = kMetalVertexBufferSlotBase;

			// Publish the shader-side vertex input declaration. The command buffer resolves it against
			// the bound vertex-buffer VertexDescription via MetalVertexInputManager at bind time —
			// mirroring VulkanGpuCommandBuffer::BindGraphicsPipeline — and the resolved MetalVertexInput
			// supplies the per-variant MTLVertexDescriptor to StartCompile. Building the descriptor
			// eagerly from the shader's own declaration (the old behavior) never reconciled against the
			// actually-bound buffers and could not serve unmatched shader inputs from a null stream.
			if (mData.VertexProgram != nullptr)
				mVertexDescription = mData.VertexProgram->GetVertexInputDescription();

			id<MTLDevice> device = mGpuDevice.GetMetalDevice();
			if (device == nil)
			{
				B3D_LOG(Error, LogRenderBackend, "Cannot initialize Metal graphics pipeline: device is null.");
				GpuGraphicsPipelineState::Initialize();
				return;
			}

			// Cache rasterizer state for later application on the render encoder.
			const RasterizerStateInformation& raster = mData.RasterizerState;
			mCullMode = (u32)MetalUtility::GetCullMode(raster.CullMode);
			mWinding = (u32)MetalUtility::GetFrontFaceWinding(raster.CullMode);
			mFillMode = (u32)MetalUtility::GetFillMode(raster.PolygonMode);
			mDepthBias = raster.DepthBias;
			mSlopeScaledDepthBias = raster.SlopeScaledDepthBias;
			mDepthBiasClamp = raster.DepthBiasClamp;
			mScissorEnabled = raster.ScissorEnable;

			// Build depth-stencil state.
			const DepthStencilStateInformation& depthStencil = mData.DepthStencilState;
			MTLDepthStencilDescriptor* dsDesc = [[MTLDepthStencilDescriptor alloc] init];
			dsDesc.depthCompareFunction = depthStencil.DepthReadEnable
				? MetalUtility::GetCompareFunction(depthStencil.DepthComparisonFunc)
				: MTLCompareFunctionAlways;
			dsDesc.depthWriteEnabled = depthStencil.DepthWriteEnable ? YES : NO;

			if (depthStencil.StencilEnable)
			{
				MTLStencilDescriptor* front = [[MTLStencilDescriptor alloc] init];
				MTLStencilDescriptor* back = [[MTLStencilDescriptor alloc] init];
				FillStencilDescriptor(front, depthStencil, true, depthStencil.StencilReadMask, depthStencil.StencilWriteMask);
				FillStencilDescriptor(back, depthStencil, false, depthStencil.StencilReadMask, depthStencil.StencilWriteMask);
				dsDesc.frontFaceStencil = front;
				dsDesc.backFaceStencil = back;
#if !__has_feature(objc_arc)
				[front release];
				[back release];
#endif
			}

			mImpl->DepthStencilState = [device newDepthStencilStateWithDescriptor:dsDesc];
#if !__has_feature(objc_arc)
			[dsDesc release];
#endif

			GpuGraphicsPipelineState::Initialize();
			} // @autoreleasepool
		}

		bool MetalGpuGraphicsPipelineState::StartCompile(const MetalPipelineVariantKey& key, const TShared<MetalVertexInput>& vertexInput)
		{
			// Pipeline compile allocates a chain of autoreleased Obj-C objects (descriptor, NSError,
			// localized-description strings). Drain them locally rather than letting them accumulate
			// on a potentially-missing outer runloop under the fiber scheduler.
			@autoreleasepool
			{
			// Fast path: variant already compiled (success or failure) on a prior call, or compile in
			// flight. Either case means there is no new async compile to dispatch — return false so the
			// caller (Prewarm / GetOrCreateMetalPipeline) knows no new work was kicked off.
			{
				Lock lock(mImpl->PipelineCacheMutex);
				if (mImpl->Pipelines.find(key) != mImpl->Pipelines.end())
					return false;
			}

			// Check the device *before* inserting a pending entry so a nil-device attempt doesn't
			// poison the cache with a permanently-unfulfillable promise. If we inserted first and
			// then bailed here, a concurrent caller could observe the stale pending entry and wait
			// forever on a completion handler that will never run. Returning false without inserting
			// lets a subsequent call retry cleanly once the device becomes available.
			id<MTLDevice> device = mGpuDevice.GetMetalDevice();
			if (device == nil)
			{
				B3D_LOG(Error, LogRenderBackend, "Cannot compile Metal graphics pipeline variant: device is null. No cache entry inserted; a subsequent call will retry once the device is available.");
				return false;
			}

			{
				// Re-check under the lock: another thread may have inserted a pending entry between
				// the fast-path find() above and now. If so, they own the dispatch — we return false
				// and let the caller wait on their completion handler. Otherwise insert our own
				// pending entry so concurrent callers see "compile in flight" and wait on the
				// completion handler rather than redundantly recompiling.
				Lock lock(mImpl->PipelineCacheMutex);
				if (mImpl->Pipelines.find(key) != mImpl->Pipelines.end())
					return false;
				mImpl->Pipelines[key] = Impl::CachedVariant{};
			}

			MTLRenderPipelineDescriptor* desc = [[MTLRenderPipelineDescriptor alloc] init];

			// Shader functions.
			if (mData.VertexProgram)
			{
				auto vp = std::static_pointer_cast<MetalGpuProgram>(mData.VertexProgram);
				desc.vertexFunction = vp->GetMetalFunction();
			}
			if (mData.FragmentProgram)
			{
				auto fp = std::static_pointer_cast<MetalGpuProgram>(mData.FragmentProgram);
				desc.fragmentFunction = fp->GetMetalFunction();
			}

			// The vertexDescriptor property is a copying property: desc owns its own copy after this
			// assignment, so the MetalVertexInput only needs to stay alive for the duration of this
			// (synchronous) call — the async completion handler never touches it.
			if (vertexInput != nullptr)
				desc.vertexDescriptor = vertexInput->GetVertexDescriptor();

			desc.inputPrimitiveTopology = (MTLPrimitiveTopologyClass)key.TopologyClass;
			desc.rasterSampleCount = std::max<u16>(1, key.SampleCount);
			desc.alphaToCoverageEnabled = mData.BlendState.EnableAlphaToCoverage ? YES : NO;

			// Color attachments. A'4: one @c u16 slot per attachment in @c key.ColorFormats (was
			// packed 8 bits into a u64 which truncated format values > 255).
			for (u32 attachmentIndex = 0; attachmentIndex < B3D_MAXIMUM_RENDER_TARGET_COUNT; attachmentIndex++)
			{
				MTLPixelFormat colorFormat = (MTLPixelFormat)key.ColorFormats[attachmentIndex];
				if (colorFormat == MTLPixelFormatInvalid)
					continue;

				MTLRenderPipelineColorAttachmentDescriptor* color = desc.colorAttachments[attachmentIndex];
				color.pixelFormat = colorFormat;

				const u32 blendIndex = mData.BlendState.EnableIndependantBlend ? attachmentIndex : 0;
				const RenderTargetBlendStateInformation& blend = mData.BlendState.RenderTargets[blendIndex];

				color.blendingEnabled = blend.BlendEnable ? YES : NO;
				color.sourceRGBBlendFactor = MetalUtility::GetBlendFactor(blend.ColorSourceFactor);
				color.destinationRGBBlendFactor = MetalUtility::GetBlendFactor(blend.ColorDestinationFactor);
				color.rgbBlendOperation = MetalUtility::GetBlendOperation(blend.ColorBlendOperation);
				color.sourceAlphaBlendFactor = MetalUtility::GetBlendFactor(blend.AlphaSourceFactor);
				color.destinationAlphaBlendFactor = MetalUtility::GetBlendFactor(blend.AlphaDestinationFactor);
				color.alphaBlendOperation = MetalUtility::GetBlendOperation(blend.AlphaBlendOperation);

				MTLColorWriteMask writeMask = MTLColorWriteMaskNone;
				if (blend.RenderTargetWriteMask & 0x1) writeMask |= MTLColorWriteMaskRed;
				if (blend.RenderTargetWriteMask & 0x2) writeMask |= MTLColorWriteMaskGreen;
				if (blend.RenderTargetWriteMask & 0x4) writeMask |= MTLColorWriteMaskBlue;
				if (blend.RenderTargetWriteMask & 0x8) writeMask |= MTLColorWriteMaskAlpha;
				color.writeMask = writeMask;
			}

			if (key.DepthFormat != 0)
				desc.depthAttachmentPixelFormat = (MTLPixelFormat)key.DepthFormat;
			if (key.StencilFormat != 0)
				desc.stencilAttachmentPixelFormat = (MTLPixelFormat)key.StencilFormat;

			// Use the archive loaded from an earlier offline/prewarm pass for pipeline lookup.
			id<MTLBinaryArchive> loadedArchive = nil;
			if (@available(macOS 11.0, iOS 14.0, *))
			{
				// Runtime archive population recompiles this descriptor. Only consume the archive loaded
				// from an earlier offline/prewarm pass on this latency-sensitive path.
				loadedArchive = mGpuDevice.GetLoadedBinaryArchive();
				NSMutableArray<id<MTLBinaryArchive>>* archives = [NSMutableArray arrayWithCapacity:1];
				if (loadedArchive != nil)
					[archives addObject:loadedArchive];
				if ([archives count] > 0)
					desc.binaryArchives = archives;
			}

			Impl* implPtr = mImpl.get();
			const MetalPipelineVariantKey keyCopy = key;

			// The destructor drains pending compiles before freeing the raw Impl captured by the handler.
			[device newRenderPipelineStateWithDescriptor:desc
				completionHandler:^(id<MTLRenderPipelineState> pipeline, NSError* error)
			{
				if (pipeline == nil)
				{
					NSString* errorString = error ? [error localizedDescription] : @"unknown error";
					B3D_LOG(Error, LogRenderBackend,
						"Failed to create Metal render pipeline state: {0}",
						String([errorString UTF8String]));
				}

				// Publish the result and notify every waiter for this variant.
				{
					Lock completionLock(implPtr->PipelineCacheMutex);
					auto& entry = implPtr->Pipelines[keyCopy];
#if !__has_feature(objc_arc)
					[pipeline retain];
#endif
					entry.Pipeline = pipeline;
					entry.Ready = true;
					implPtr->PipelineCacheCV.notify_all();
				}

			}];

#if !__has_feature(objc_arc)
			[desc release];
#endif

			return true;
			} // @autoreleasepool
		}

		void MetalGpuGraphicsPipelineState::Prewarm(const MetalPipelineVariantKey& key, const TShared<MetalVertexInput>& vertexInput)
		{
			// Prewarm is a non-blocking fire-and-forget. Delegate to StartCompile and discard the
			// return value — a subsequent draw-time GetOrCreateMetalPipeline for the same key will
			// find the pending entry and simply wait on the already-in-flight completion handler.
			(void)StartCompile(key, vertexInput);
		}

		id<MTLRenderPipelineState> MetalGpuGraphicsPipelineState::GetOrCreateMetalPipeline(const MetalPipelineVariantKey& key, const TShared<MetalVertexInput>& vertexInput)
		{
			// Dispatch the compile if nobody's started one for this key yet, then block until the
			// completion handler publishes the result. If the pipeline was prewarmed or is already
			// ready, StartCompile returns false immediately and we fall through to the wait, which
			// either returns immediately (Ready already true) or blocks until the pending compile's
			// handler runs.
			const bool dispatched = StartCompile(key, vertexInput);

			Lock lock(mImpl->PipelineCacheMutex);
			if (!dispatched && mImpl->Pipelines.find(key) == mImpl->Pipelines.end())
			{
				// StartCompile aborted without inserting (e.g. device was nil). Nothing is in flight
				// for this key, so there is no completion handler that will ever signal the CV —
				// waiting here would deadlock. Return nil and let the caller treat pipeline
				// unavailability as a transient error (skip the draw, retry next frame, etc.).
				return nil;
			}
			mImpl->PipelineCacheCV.wait(lock, [&]{
				auto found = mImpl->Pipelines.find(key);
				return found != mImpl->Pipelines.end() && found->second.Ready;
			});
			return mImpl->Pipelines[key].Pipeline;
		}

		struct MetalGpuComputePipelineState::Impl
		{
			// Compute-pipeline compile is driven by the async completionHandler variant, so Initialize
			// returns immediately with a pending entry and GetMetalPipeline waits on the CV if the
			// compile hasn't landed yet. This lets a resource-loader thread kick off Initialize in
			// parallel with render-thread work without blocking either side on the MSL compile.
			id<MTLComputePipelineState> Pipeline = nil;
			bool Ready = false;
			// True once Initialize has fired (success or early-out). Consumed by the destructor to
			// decide whether a wait is required; in the deferred-init case where Initialize is never
			// called, Ready stays false and without this flag the destructor would hang forever.
			bool InitializeStarted = false;
			Mutex PipelineMutex;
			ConditionVariable PipelineCV;
		};

		MetalGpuComputePipelineState::MetalGpuComputePipelineState(MetalGpuDevice& gpuDevice, const GpuComputePipelineStateCreateInformation& createInformation)
			: GpuComputePipelineState(gpuDevice, createInformation)
			, mGpuDevice(gpuDevice)
			, mImpl(B3DMakeUnique<Impl>())
		{ }

		MetalGpuComputePipelineState::~MetalGpuComputePipelineState()
		{
			if (mImpl)
			{
				// Drain the in-flight async compile if Initialize() fired one off and no caller has
				// picked up the result yet. The completion handler captures mImpl.get() raw, so we
				// must not free the Impl while the handler is still pending. If Initialize was never
				// called (deferred-init path with no
				// subsequent init), Ready stays false — skip the wait based on InitializeStarted to
				// avoid a deadlock on the never-signaled CV.
				Lock lock(mImpl->PipelineMutex);
				if (mImpl->InitializeStarted)
					mImpl->PipelineCV.wait(lock, [this]{ return mImpl->Ready; });
#if !__has_feature(objc_arc)
				[mImpl->Pipeline release];
#endif
				mImpl->Pipeline = nil;
			}

		}

		id<MTLComputePipelineState> MetalGpuComputePipelineState::GetMetalPipeline() const
		{
			// First access after Initialize() may hit before the async compile finishes — block until
			// the completion handler has published the result (or installed nil on failure).
			Lock lock(mImpl->PipelineMutex);
			mImpl->PipelineCV.wait(lock, [this]{ return mImpl->Ready; });
			return mImpl->Pipeline;
		}

		void MetalGpuComputePipelineState::Prewarm()
		{
			// Compute pipeline state kicks off its async newComputePipelineStateWithDescriptor: call
			// inside Initialize(), so the prewarm path simply ensures Initialize has fired. If the
			// pipeline was created with GpuObjectCreateFlag::DeferredInitialize, Initialize hasn't
			// been called yet and GetMetalPipeline would deadlock at draw time — calling Initialize
			// here makes the compute compile effectively asynchronous from the caller's perspective.
			// Safe to re-enter: the bool guard in the async Initialize protects against double-start.
			Initialize();
		}

		void MetalGpuComputePipelineState::Initialize()
		{
			// Pipeline state creation allocates autoreleased NSError instances (and potentially
			// localizedDescription strings on failure). Drain them locally — fiber-scheduled frames
			// may never hit a runloop.
			@autoreleasepool
			{
			{
				// Mark the async compile path as engaged so the destructor knows whether to wait on
				// the completion-handler CV. All early-outs below also set Ready = true before
				// returning, so the destructor's wait finds a satisfied predicate in those cases.
				Lock lock(mImpl->PipelineMutex);
				if (mImpl->InitializeStarted)
					return;
				mImpl->InitializeStarted = true;
			}

			id<MTLDevice> device = mGpuDevice.GetMetalDevice();
			if (device == nil)
			{
				B3D_LOG(Error, LogRenderBackend, "Cannot initialize Metal compute pipeline: device is null.");
				{
					Lock lock(mImpl->PipelineMutex);
					mImpl->Ready = true;
					mImpl->PipelineCV.notify_all();
				}
				GpuComputePipelineState::Initialize();
				return;
			}

			if (!mData.Program)
			{
				B3D_LOG(Error, LogRenderBackend, "Cannot initialize Metal compute pipeline: compute program is null.");
				{
					Lock lock(mImpl->PipelineMutex);
					mImpl->Ready = true;
					mImpl->PipelineCV.notify_all();
				}
				GpuComputePipelineState::Initialize();
				return;
			}

			auto program = std::static_pointer_cast<MetalGpuProgram>(mData.Program);
			id<MTLFunction> function = program->GetMetalFunction();
			if (function == nil)
			{
				B3D_LOG(Error, LogRenderBackend, "Cannot initialize Metal compute pipeline: program has no Metal function.");
				{
					Lock lock(mImpl->PipelineMutex);
					mImpl->Ready = true;
					mImpl->PipelineCV.notify_all();
				}
				GpuComputePipelineState::Initialize();
				return;
			}

			const u32* programWorkgroup = program->GetWorkgroupSize();
			mWorkgroupSize[0] = programWorkgroup[0];
			mWorkgroupSize[1] = programWorkgroup[1];
			mWorkgroupSize[2] = programWorkgroup[2];

			// Use the descriptor path so an offline-populated binary archive can accelerate lookup.
			MTLComputePipelineDescriptor* desc = [[MTLComputePipelineDescriptor alloc] init];
			desc.computeFunction = function;

			// No engine-level contract guarantees a multiple of threadExecutionWidth. Keep Metal's safe
			// default; incorrectly promising a multiple makes non-conforming dispatches undefined.
			desc.threadGroupSizeIsMultipleOfThreadExecutionWidth = NO;

			id<MTLBinaryArchive> loadedArchive = nil;
			if (@available(macOS 11.0, iOS 14.0, *))
			{
				loadedArchive = mGpuDevice.GetLoadedBinaryArchive();
				NSMutableArray<id<MTLBinaryArchive>>* archives = [NSMutableArray arrayWithCapacity:1];
				if (loadedArchive != nil)
					[archives addObject:loadedArchive];
				if ([archives count] > 0)
					desc.binaryArchives = archives;
			}

			Impl* implPtr = mImpl.get();

			[device newComputePipelineStateWithDescriptor:desc
				options:MTLPipelineOptionNone
				completionHandler:^(id<MTLComputePipelineState> pipeline, MTLComputePipelineReflection* /*reflection*/, NSError* error)
			{
				if (pipeline == nil)
				{
					NSString* errorString = error ? [error localizedDescription] : @"unknown error";
					B3D_LOG(Error, LogRenderBackend,
						"Failed to create Metal compute pipeline state: {0}",
						String([errorString UTF8String]));
				}

				// Publish the pipeline result and notify all waiters.
				{
					Lock completionLock(implPtr->PipelineMutex);
#if !__has_feature(objc_arc)
					[pipeline retain];
#endif
					implPtr->Pipeline = pipeline;
					implPtr->Ready = true;
					implPtr->PipelineCV.notify_all();
				}

			}];

#if !__has_feature(objc_arc)
			[desc release];
#endif

			GpuComputePipelineState::Initialize();
			} // @autoreleasepool
		}
	} // namespace render
} // namespace b3d
