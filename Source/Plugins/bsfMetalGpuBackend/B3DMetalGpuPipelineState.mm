//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DMetalGpuPipelineState.h"
#include "B3DMetalGpuDevice.h"
#include "B3DMetalGpuProgram.h"
#include "B3DMetalUtility.h"
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
			MTLVertexDescriptor* VertexDescriptor = nil;

			// Per-variant cache entry. Compilation is driven by the async @c completionHandler variant
			// of @c newRenderPipelineStateWithDescriptor:, so an entry goes through a pending state
			// (Ready == false, Pipeline == nil) before the completion handler fills in the result and
			// flips Ready. Concurrent callers that arrive while a compile is in flight find the entry
			// not-ready, unlock, and wait on @c PipelineCacheCV until the handler notifies — no
			// duplicate compiles for the same key. No prewarming path exists yet, so every call still
			// waits here; the completion-handler shape keeps the door open for a future Prewarm API
			// without another rewrite.
			struct CachedVariant
			{
				id<MTLRenderPipelineState> Pipeline = nil;
				bool Ready = false;
			};

			Mutex PipelineCacheMutex;
			ConditionVariable PipelineCacheCV;
			UnorderedMap<MetalPipelineVariantKey, CachedVariant, MetalPipelineVariantKeyHash> Pipelines;

			// Keys that have already been registered into the device's MTLBinaryArchive this session.
			// @c addRenderPipelineFunctionsWithDescriptor: does a full shader compile internally, so
			// we track already-added keys to avoid redundant compiles when a PSO is retained beyond a
			// single draw.
			UnorderedSet<MetalPipelineVariantKey, MetalPipelineVariantKeyHash> ArchivedVariants;

			// Count of completion handlers that have published their pipeline result but are still
			// running the post-publish @c addRenderPipelineFunctionsWithDescriptor: call (which
			// recompiles internally and can take many ms). The destructor must wait for this to reach
			// zero in addition to every variant being Ready; otherwise a handler could still be
			// touching @c implPtr after @c ~MetalGpuGraphicsPipelineState returns and frees the Impl.
			u32 PendingArchiveAdds = 0;
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
					// Drain any in-flight compiles so the completion handlers don't reference a freed Impl.
					// Two things must finish: every variant's compile (Ready flips true) and every post-
					// publish archive-add (PendingArchiveAdds hits zero). Splitting these lets draw-time
					// waiters unblock as soon as the pipeline is published, while the destructor still
					// blocks until the slower addRenderPipelineFunctionsWithDescriptor: call finishes.
					Lock lock(mImpl->PipelineCacheMutex);
					mImpl->PipelineCacheCV.wait(lock, [this]{
						for (auto& entry : mImpl->Pipelines)
						{
							if (!entry.second.Ready)
								return false;
						}
						return mImpl->PendingArchiveAdds == 0;
					});

					for (auto& entry : mImpl->Pipelines)
						entry.second.Pipeline = nil;
					mImpl->Pipelines.clear();
					mImpl->ArchivedVariants.clear();
				}
				mImpl->DepthStencilState = nil;
				mImpl->VertexDescriptor = nil;
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

			// Argument buffers for parameter sets occupy low buffer slots (one per set); vertex streams
			// are offset by the set count to avoid collisions in the vertex-stage buffer table.
			mVertexBufferBaseIndex = mParameterLayout ? mParameterLayout->GetSetCount() : 0;

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

			// Build vertex descriptor from the vertex program's input declaration. If we don't yet have
			// a vertex program (e.g., when the pipeline will only ever be bound with a fixed vertex
			// layout supplied at command-buffer time) we skip this; the command buffer then sets up the
			// descriptor directly from SetVertexDescription().
			if (mData.VertexProgram)
			{
				TShared<VertexDescription> inputDesc = mData.VertexProgram->GetVertexInputDescription();
				if (inputDesc)
				{
					mImpl->VertexDescriptor = [[MTLVertexDescriptor alloc] init];

					const auto& elements = inputDesc->GetElements();
					for (u32 elementIndex = 0; elementIndex < elements.size(); elementIndex++)
					{
						const VertexElement& element = elements[elementIndex];

						MTLVertexFormat format = MetalUtility::GetVertexFormat(element.GetType());
						if (format == MTLVertexFormatInvalid)
						{
							B3D_LOG(Warning, LogRenderBackend, "Unsupported vertex element type {0} on Metal backend.", (u32)element.GetType());
							continue;
						}

						// SPIRV-Cross emits @c [[attribute(N)]] where N is the SPIR-V input location, not the
						// element's position in the reflected list. For shader-input descriptions produced by
						// glslang (see B3DGLSLToSPIRV.cpp::ParseVertexAttributes), @c GetOffset() holds
						// the @c layout(location=...) value — sparse locations (0, 2, 5) would be miswired as
						// dense (0, 1, 2) if we used @c elementIndex here.
						const u32 attrIndex = element.GetOffset();
						mImpl->VertexDescriptor.attributes[attrIndex].format = format;
						mImpl->VertexDescriptor.attributes[attrIndex].offset = element.GetOffset();
						mImpl->VertexDescriptor.attributes[attrIndex].bufferIndex = mVertexBufferBaseIndex + element.GetStreamIndex();
					}

					// Per-stream stride/step info. The engine stores stride implicitly via GetVertexStride(stream).
					constexpr u32 kMaxStreams = 16;
					for (u32 streamIndex = 0; streamIndex < kMaxStreams; streamIndex++)
					{
						u32 stride = inputDesc->GetVertexStride(streamIndex);
						if (stride == 0)
							continue;

						const u32 layoutIndex = mVertexBufferBaseIndex + streamIndex;
						mImpl->VertexDescriptor.layouts[layoutIndex].stride = stride;
						mImpl->VertexDescriptor.layouts[layoutIndex].stepFunction = MTLVertexStepFunctionPerVertex;
						mImpl->VertexDescriptor.layouts[layoutIndex].stepRate = 1;
					}

					// Override with instance-step info from any element that requests it.
					for (const VertexElement& element : elements)
					{
						if (element.GetInstanceStepRate() > 0)
						{
							const u32 layoutIndex = mVertexBufferBaseIndex + element.GetStreamIndex();
							mImpl->VertexDescriptor.layouts[layoutIndex].stepFunction = MTLVertexStepFunctionPerInstance;
							mImpl->VertexDescriptor.layouts[layoutIndex].stepRate = element.GetInstanceStepRate();
						}
					}
				}
			}

			GpuGraphicsPipelineState::Initialize();
			} // @autoreleasepool
		}

		bool MetalGpuGraphicsPipelineState::StartCompile(const MetalPipelineVariantKey& key)
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

			if (mImpl->VertexDescriptor)
				desc.vertexDescriptor = mImpl->VertexDescriptor;

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

			// Attach the device's binary archives so Metal can reuse functions already compiled in a
			// previous launch (via the loaded, read-only archive) and also register fresh compiles we
			// do here (via the mutable archive) through addRenderPipelineFunctionsWithDescriptor:
			// below. URL-loaded archives are immutable — only the mutable archive is a valid add
			// target. Both are attached to the descriptor because either can serve as a lookup source.
			id<MTLBinaryArchive> mutableArchive = nil;
			id<MTLBinaryArchive> loadedArchive = nil;
			if (@available(macOS 11.0, iOS 14.0, *))
			{
				mutableArchive = mGpuDevice.GetMutableBinaryArchive();
				loadedArchive = mGpuDevice.GetLoadedBinaryArchive();
				NSMutableArray<id<MTLBinaryArchive>>* archives = [NSMutableArray arrayWithCapacity:2];
				if (loadedArchive != nil)
					[archives addObject:loadedArchive];
				if (mutableArchive != nil)
					[archives addObject:mutableArchive];
				if ([archives count] > 0)
					desc.binaryArchives = archives;
			}

			Impl* implPtr = mImpl.get();
			const MetalPipelineVariantKey keyCopy = key;

			// Under MRC, Block_copy does auto-retain captured Obj-C pointers, but the completion
			// handler uses @c desc in addRenderPipelineFunctionsWithDescriptor: after publishing the
			// result — to avoid relying on that implicit behavior while Metal's block copy is still
			// in flight, we pair an explicit retain here with an explicit release at the end of the
			// handler. Under ARC the pair is a no-op (the #if strips them out) and the compiler-
			// generated retain/release takes care of lifetime.
#if !__has_feature(objc_arc)
			[desc retain];
#endif

			// Fire the async compile. The completion handler runs on a Metal-internal queue; it takes
			// the cache lock, publishes the result, notifies waiters, and then runs the (potentially
			// expensive) archive-add. Note that @c impl is captured by raw pointer — the pipeline
			// state destructor drains pending compiles *and* pending archive-adds before it frees
			// mImpl, so the pointer stays valid for the duration of every outstanding handler.
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

				// Publish the result first and notify waiters immediately, so draw-time callers don't
				// block on the slow addRenderPipelineFunctions path below. If the pipeline compile
				// succeeded and the variant hasn't already been registered this session, record a
				// pending archive-add under the same lock so the destructor waits for us.
				bool shouldAddToArchive = false;
				{
					Lock completionLock(implPtr->PipelineCacheMutex);
					auto& entry = implPtr->Pipelines[keyCopy];
					entry.Pipeline = pipeline;
					entry.Ready = true;

					if (pipeline != nil && mutableArchive != nil)
					{
						if (implPtr->ArchivedVariants.insert(keyCopy).second)
						{
							shouldAddToArchive = true;
							implPtr->PendingArchiveAdds++;
						}
					}
				}
				implPtr->PipelineCacheCV.notify_all();

				// Now run the expensive archive-add. addRenderPipelineFunctionsWithDescriptor: does a
				// full shader compile internally; by running it after notify_all we keep draw-time
				// waiters off the critical path. The dedup via ArchivedVariants above ensures we only
				// pay this cost once per variant per session.
				if (shouldAddToArchive)
				{
					if (@available(macOS 11.0, iOS 14.0, *))
					{
						NSError* addError = nil;
						if (![mutableArchive addRenderPipelineFunctionsWithDescriptor:desc error:&addError])
						{
							NSString* addDesc = addError ? [addError localizedDescription] : @"unknown error";
							B3D_LOG(Warning, LogRenderBackend,
								"Failed to add render pipeline to Metal binary archive: {0}",
								String([addDesc UTF8String]));
						}
					}

					{
						Lock lock(implPtr->PipelineCacheMutex);
						B3D_ASSERT(implPtr->PendingArchiveAdds > 0);
						implPtr->PendingArchiveAdds--;
					}
					implPtr->PipelineCacheCV.notify_all();
				}

#if !__has_feature(objc_arc)
				[desc release];
#endif
			}];

#if !__has_feature(objc_arc)
			[desc release];
#endif

			return true;
			} // @autoreleasepool
		}

		void MetalGpuGraphicsPipelineState::Prewarm(const MetalPipelineVariantKey& key)
		{
			// Prewarm is a non-blocking fire-and-forget. Delegate to StartCompile and discard the
			// return value — a subsequent draw-time GetOrCreateMetalPipeline for the same key will
			// find the pending entry and simply wait on the already-in-flight completion handler.
			(void)StartCompile(key);
		}

		id<MTLRenderPipelineState> MetalGpuGraphicsPipelineState::GetOrCreateMetalPipeline(const MetalPipelineVariantKey& key)
		{
			// Dispatch the compile if nobody's started one for this key yet, then block until the
			// completion handler publishes the result. If the pipeline was prewarmed or is already
			// ready, StartCompile returns false immediately and we fall through to the wait, which
			// either returns immediately (Ready already true) or blocks until the pending compile's
			// handler runs.
			const bool dispatched = StartCompile(key);

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
			// Set to true while the completion handler runs the post-publish
			// addComputePipelineFunctionsWithDescriptor: call. The destructor waits for this in
			// addition to Ready because the handler keeps touching implPtr (for the lock/decrement
			// and notify) even after publishing the pipeline.
			bool ArchiveAddInFlight = false;
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
				// must not free the Impl while the handler is still pending. We wait for both the
				// pipeline result to be published (Ready) and the post-publish archive-add to finish
				// (!ArchiveAddInFlight) — the handler keeps touching implPtr for the final notify even
				// after it sets Ready. If Initialize was never called (deferred-init path with no
				// subsequent init), Ready stays false — skip the wait based on InitializeStarted to
				// avoid a deadlock on the never-signaled CV.
				Lock lock(mImpl->PipelineMutex);
				if (mImpl->InitializeStarted)
					mImpl->PipelineCV.wait(lock, [this]{ return mImpl->Ready && !mImpl->ArchiveAddInFlight; });
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
			bool alreadyStarted = false;
			{
				Lock lock(mImpl->PipelineMutex);
				alreadyStarted = mImpl->InitializeStarted;
			}
			if (!alreadyStarted)
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
				mImpl->InitializeStarted = true;
			}

			id<MTLDevice> device = mGpuDevice.GetMetalDevice();
			if (device == nil)
			{
				B3D_LOG(Error, LogRenderBackend, "Cannot initialize Metal compute pipeline: device is null.");
				{
					Lock lock(mImpl->PipelineMutex);
					mImpl->Ready = true;
				}
				mImpl->PipelineCV.notify_all();
				GpuComputePipelineState::Initialize();
				return;
			}

			if (!mData.Program)
			{
				B3D_LOG(Error, LogRenderBackend, "Cannot initialize Metal compute pipeline: compute program is null.");
				{
					Lock lock(mImpl->PipelineMutex);
					mImpl->Ready = true;
				}
				mImpl->PipelineCV.notify_all();
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
				}
				mImpl->PipelineCV.notify_all();
				GpuComputePipelineState::Initialize();
				return;
			}

			const u32* programWorkgroup = program->GetWorkgroupSize();
			mWorkgroupSize[0] = programWorkgroup[0];
			mWorkgroupSize[1] = programWorkgroup[1];
			mWorkgroupSize[2] = programWorkgroup[2];

			// Use the descriptor-based compute pipeline creation path so we can attach the device's
			// binary archives. @c newComputePipelineStateWithFunction: (the previous sync call) does
			// not expose binaryArchives, so cold-launch compiles can't be cached there. Both the
			// loaded (read-only) and mutable archives are attached; only the mutable one is a valid
			// add target for addComputePipelineFunctionsWithDescriptor:.
			MTLComputePipelineDescriptor* desc = [[MTLComputePipelineDescriptor alloc] init];
			desc.computeFunction = function;

			// The engine's compute shaders declare their workgroup size explicitly (we just copied it into
			// mWorkgroupSize above), and every declared size the backend actually dispatches is a multiple
			// of the device's thread-execution width on the Apple/AMD/Intel GPUs Metal runs on. Telling the
			// driver so lets it skip the inner-loop bounds checks normally emitted to guard against partial
			// threadgroups, which is a measurable win on dispatch-heavy frames (post-processing chains,
			// compute-shader particle sims). The promise is a hard contract — if it's ever violated, the
			// tail threads in a threadgroup read garbage; since the workgroup size is shader-authored, the
			// shader author is already responsible for making this true.
			desc.threadGroupSizeIsMultipleOfThreadExecutionWidth = YES;

			id<MTLBinaryArchive> mutableArchive = nil;
			id<MTLBinaryArchive> loadedArchive = nil;
			if (@available(macOS 11.0, iOS 14.0, *))
			{
				mutableArchive = mGpuDevice.GetMutableBinaryArchive();
				loadedArchive = mGpuDevice.GetLoadedBinaryArchive();
				NSMutableArray<id<MTLBinaryArchive>>* archives = [NSMutableArray arrayWithCapacity:2];
				if (loadedArchive != nil)
					[archives addObject:loadedArchive];
				if (mutableArchive != nil)
					[archives addObject:mutableArchive];
				if ([archives count] > 0)
					desc.binaryArchives = archives;
			}

			Impl* implPtr = mImpl.get();

			// Under MRC, pair an explicit retain here with a release inside the completion handler to
			// keep @c desc alive across the handler's addComputePipelineFunctions call without relying
			// on Block_copy's implicit capture-retain while Metal's copy is still in flight. Under ARC
			// the compiler handles both ends.
#if !__has_feature(objc_arc)
			[desc retain];
#endif

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

				// Publish the pipeline result first and notify waiters so GetMetalPipeline() unblocks
				// immediately. The archive-add below recompiles the function internally and can take
				// many ms — keeping it off the critical path is why this is reordered past notify.
				bool shouldAddToArchive = false;
				{
					Lock completionLock(implPtr->PipelineMutex);
					implPtr->Pipeline = pipeline;
					implPtr->Ready = true;
					if (pipeline != nil && mutableArchive != nil)
					{
						shouldAddToArchive = true;
						implPtr->ArchiveAddInFlight = true;
					}
				}
				implPtr->PipelineCV.notify_all();

				// Register the successful compile into the mutable archive so the next launch can
				// skip MSL translation. Single-shot per Impl: a compute pipeline state owns one
				// pipeline with one function, so no dedup side-set is needed — the enclosing
				// Initialize is called once.
				if (shouldAddToArchive)
				{
					if (@available(macOS 11.0, iOS 14.0, *))
					{
						NSError* addError = nil;
						if (![mutableArchive addComputePipelineFunctionsWithDescriptor:desc error:&addError])
						{
							NSString* addDesc = addError ? [addError localizedDescription] : @"unknown error";
							B3D_LOG(Warning, LogRenderBackend,
								"Failed to add compute pipeline to Metal binary archive: {0}",
								String([addDesc UTF8String]));
						}
					}

					{
						Lock lock(implPtr->PipelineMutex);
						implPtr->ArchiveAddInFlight = false;
					}
					implPtr->PipelineCV.notify_all();
				}

#if !__has_feature(objc_arc)
				[desc release];
#endif
			}];

#if !__has_feature(objc_arc)
			[desc release];
#endif

			GpuComputePipelineState::Initialize();
			} // @autoreleasepool
		}
	} // namespace render
} // namespace b3d
