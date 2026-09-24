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
			/** Depth-stencil state variants indexed by (depthReadOnly | stencilReadOnly << 1), created on demand. */
			id<MTLDepthStencilState> DepthStencilStates[4] = { nil, nil, nil, nil };

			// Per-variant cache entry. Compilation is driven by the async @c completionHandler variant
			// of @c newRenderPipelineStateWithDescriptor:, so an entry goes through a pending state
			// (Ready == false, Pipeline == nil) before the completion handler fills in the result and
			// flips Ready. Concurrent callers that arrive while a compile is in flight find the entry
			// not-ready, unlock, and wait on @c VariantReadySignal until the handler notifies — no
			// duplicate compiles for the same key.
			struct CachedVariant
			{
				id<MTLRenderPipelineState> Pipeline = nil;
				bool Ready = false;
			};

			Mutex PipelineCacheMutex;
			ConditionVariable VariantReadySignal;
			UnorderedMap<MetalPipelineVariantKey, CachedVariant, MetalPipelineVariantKeyHash> Pipelines;

		};

		MetalGpuGraphicsPipelineState::MetalGpuGraphicsPipelineState(MetalGpuDevice& gpuDevice, const GpuGraphicsPipelineStateCreateInformation& createInformation)
			: GpuGraphicsPipelineState(gpuDevice, createInformation), mGpuDevice(gpuDevice), mImpl(B3DMakeUnique<Impl>())
		{ }

		MetalGpuGraphicsPipelineState::~MetalGpuGraphicsPipelineState()
		{
			if (mImpl)
			{
				{
					// Drain any in-flight compiles so completion handlers cannot reference a freed Impl.
					Lock lock(mImpl->PipelineCacheMutex);
					mImpl->VariantReadySignal.wait(lock, [this]
					{
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
				for (u32 variantIndex = 0; variantIndex < 4; variantIndex++)
				{
#if !__has_feature(objc_arc)
					[mImpl->DepthStencilStates[variantIndex] release];
#endif
					mImpl->DepthStencilStates[variantIndex] = nil;
				}
			}

		}

		static id<MTLDepthStencilState> CreateDepthStencilState(id<MTLDevice> device, const DepthStencilStateInformation& depthStencil, bool depthReadOnly, bool stencilReadOnly);

		id<MTLDepthStencilState> MetalGpuGraphicsPipelineState::GetMetalDepthStencilState(bool depthReadOnly, bool stencilReadOnly)
		{
			const u32 variantIndex = (depthReadOnly ? 1u : 0u) | (stencilReadOnly ? 2u : 0u);

			Lock lock(mImpl->PipelineCacheMutex);
			if (mImpl->DepthStencilStates[variantIndex] == nil)
				mImpl->DepthStencilStates[variantIndex] = CreateDepthStencilState(mGpuDevice.GetMetalDevice(), mData.DepthStencilState, depthReadOnly, stencilReadOnly);

			return mImpl->DepthStencilStates[variantIndex];
		}

		static void FillStencilDescriptor(MTLStencilDescriptor* descriptor, const DepthStencilStateInformation& state, bool front, u8 readMask, u8 writeMask)
		{
			descriptor.readMask = readMask;
			descriptor.writeMask = writeMask;

			if (front)
			{
				descriptor.stencilCompareFunction = MetalUtility::GetCompareFunction(state.FrontStencilComparisonFunc);
				descriptor.stencilFailureOperation = MetalUtility::GetStencilOperation(state.FrontStencilFailOp);
				descriptor.depthFailureOperation = MetalUtility::GetStencilOperation(state.FrontStencilZFailOp);
				descriptor.depthStencilPassOperation = MetalUtility::GetStencilOperation(state.FrontStencilPassOp);
			}
			else
			{
				descriptor.stencilCompareFunction = MetalUtility::GetCompareFunction(state.BackStencilComparisonFunc);
				descriptor.stencilFailureOperation = MetalUtility::GetStencilOperation(state.BackStencilFailOp);
				descriptor.depthFailureOperation = MetalUtility::GetStencilOperation(state.BackStencilZFailOp);
				descriptor.depthStencilPassOperation = MetalUtility::GetStencilOperation(state.BackStencilPassOp);
			}
		}

		/** Builds a depth-stencil state, with writes to read-only attachments masked off. Returns nil without a device. */
		static id<MTLDepthStencilState> CreateDepthStencilState(id<MTLDevice> device, const DepthStencilStateInformation& depthStencil, bool depthReadOnly, bool stencilReadOnly)
		{
			if (device == nil)
				return nil;

			@autoreleasepool
			{
			MTLDepthStencilDescriptor* descriptor = [[MTLDepthStencilDescriptor alloc] init];
			descriptor.depthCompareFunction = depthStencil.DepthReadEnable ? MetalUtility::GetCompareFunction(depthStencil.DepthComparisonFunc) : MTLCompareFunctionAlways;
			descriptor.depthWriteEnabled = (depthStencil.DepthWriteEnable && !depthReadOnly) ? YES : NO;

			if (depthStencil.StencilEnable)
			{
				const u8 writeMask = stencilReadOnly ? 0 : depthStencil.StencilWriteMask;

				MTLStencilDescriptor* front = [[MTLStencilDescriptor alloc] init];
				MTLStencilDescriptor* back = [[MTLStencilDescriptor alloc] init];
				FillStencilDescriptor(front, depthStencil, true, depthStencil.StencilReadMask, writeMask);
				FillStencilDescriptor(back, depthStencil, false, depthStencil.StencilReadMask, writeMask);
				descriptor.frontFaceStencil = front;
				descriptor.backFaceStencil = back;

#if !__has_feature(objc_arc)
				[front release];
				[back release];
#endif
			}

			id<MTLDepthStencilState> state = [device newDepthStencilStateWithDescriptor:descriptor];
#if !__has_feature(objc_arc)
			[descriptor release];
#endif
			return state;
			} // @autoreleasepool
		}

		void MetalGpuGraphicsPipelineState::Initialize()
		{
			@autoreleasepool
			{
			mVertexBufferBaseIndex = kMetalVertexBufferSlotBase;

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
			const RasterizerStateInformation& rasterizerState = mData.RasterizerState;
			mCullMode = (u32)MetalUtility::GetCullMode(rasterizerState.CullMode);
			mWinding = (u32)MetalUtility::GetFrontFaceWinding(rasterizerState.CullMode);
			mFillMode = (u32)MetalUtility::GetFillMode(rasterizerState.PolygonMode);
			mDepthBias = rasterizerState.DepthBias;
			mSlopeScaledDepthBias = rasterizerState.SlopeScaledDepthBias;
			mDepthBiasClamp = rasterizerState.DepthBiasClamp;
			mScissorEnabled = rasterizerState.ScissorEnable;

			// Build the fully writable depth-stencil state up front; read-only variants are created on demand.
			mImpl->DepthStencilStates[0] = CreateDepthStencilState(device, mData.DepthStencilState, false, false);

			GpuGraphicsPipelineState::Initialize();
			} // @autoreleasepool
		}

		bool MetalGpuGraphicsPipelineState::StartCompile(const MetalPipelineVariantKey& key, const TShared<MetalVertexInput>& vertexInput)
		{
			@autoreleasepool
			{
			// Fast path: variant already compiled (success or failure) on a prior call, or compile in flight.
			{
				Lock lock(mImpl->PipelineCacheMutex);
				if (mImpl->Pipelines.find(key) != mImpl->Pipelines.end())
					return false;
			}

			id<MTLDevice> device = mGpuDevice.GetMetalDevice();
			if (device == nil)
			{
				B3D_LOG(Error, LogRenderBackend, "Cannot compile Metal graphics pipeline variant: device is null. No cache entry inserted; a subsequent call will retry once the device is available.");
				return false;
			}

			{
				// Re-check under the lock: another thread may have inserted a pending entry between the fast-path find() above and now.
				Lock lock(mImpl->PipelineCacheMutex);
				if (mImpl->Pipelines.find(key) != mImpl->Pipelines.end())
					return false;

				mImpl->Pipelines[key] = Impl::CachedVariant{};
			}

			MTLRenderPipelineDescriptor* descriptor = [[MTLRenderPipelineDescriptor alloc] init];

			// Label the pipeline with the vertex program's name
			if (mData.VertexProgram && !mData.VertexProgram->GetName().empty())
				descriptor.label = [NSString stringWithUTF8String:mData.VertexProgram->GetName().c_str()];

			// Shader functions
			if (mData.VertexProgram)
			{
				auto vertexProgram = std::static_pointer_cast<MetalGpuProgram>(mData.VertexProgram);
				descriptor.vertexFunction = vertexProgram->GetMetalFunction();
			}
			if (mData.FragmentProgram)
			{
				auto fragmentProgram = std::static_pointer_cast<MetalGpuProgram>(mData.FragmentProgram);
				descriptor.fragmentFunction = fragmentProgram->GetMetalFunction();
			}

			if (vertexInput != nullptr)
				descriptor.vertexDescriptor = vertexInput->GetVertexDescriptor();

			descriptor.inputPrimitiveTopology = (MTLPrimitiveTopologyClass)key.TopologyClass;
			descriptor.rasterSampleCount = std::max<u16>(1, key.SampleCount);
			descriptor.alphaToCoverageEnabled = mData.BlendState.EnableAlphaToCoverage ? YES : NO;

			for (u32 attachmentIndex = 0; attachmentIndex < B3D_MAXIMUM_RENDER_TARGET_COUNT; attachmentIndex++)
			{
				MTLPixelFormat colorFormat = (MTLPixelFormat)key.ColorFormats[attachmentIndex];
				if (colorFormat == MTLPixelFormatInvalid)
					continue;

				MTLRenderPipelineColorAttachmentDescriptor* color = descriptor.colorAttachments[attachmentIndex];
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

				// Read-only attachments get no writes at all, regardless of the blend state's mask.
				MTLColorWriteMask writeMask = MTLColorWriteMaskNone;
				if ((key.ReadOnlyMask & (RT_COLOR0 << attachmentIndex)) == 0)
				{
					if (blend.RenderTargetWriteMask & 0x1) writeMask |= MTLColorWriteMaskRed;
					if (blend.RenderTargetWriteMask & 0x2) writeMask |= MTLColorWriteMaskGreen;
					if (blend.RenderTargetWriteMask & 0x4) writeMask |= MTLColorWriteMaskBlue;
					if (blend.RenderTargetWriteMask & 0x8) writeMask |= MTLColorWriteMaskAlpha;
				}
				color.writeMask = writeMask;
			}

			if (key.DepthFormat != 0)
				descriptor.depthAttachmentPixelFormat = (MTLPixelFormat)key.DepthFormat;

			if (key.StencilFormat != 0)
				descriptor.stencilAttachmentPixelFormat = (MTLPixelFormat)key.StencilFormat;

			// TODO - Attach an offline-built MTLBinaryArchive (descriptor.binaryArchives) to render and compute pipelines to skip first-launch compiles.

			Impl* impl = mImpl.get();
			const MetalPipelineVariantKey keyCopy = key;

			// The destructor drains pending compiles before freeing the raw Impl captured by the handler.
			[device newRenderPipelineStateWithDescriptor:descriptor completionHandler:^(id<MTLRenderPipelineState> pipeline, NSError* error)
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
					Lock completionLock(impl->PipelineCacheMutex);
					auto& entry = impl->Pipelines[keyCopy];
#if !__has_feature(objc_arc)
					[pipeline retain];
#endif
					entry.Pipeline = pipeline;
					entry.Ready = true;

					impl->VariantReadySignal.notify_all();
				}

			}];

#if !__has_feature(objc_arc)
			[descriptor release];
#endif

			return true;
			} // @autoreleasepool
		}

		id<MTLRenderPipelineState> MetalGpuGraphicsPipelineState::GetOrCreateMetalPipeline(const MetalPipelineVariantKey& key, const TShared<MetalVertexInput>& vertexInput)
		{
			const bool dispatched = StartCompile(key, vertexInput);

			Lock lock(mImpl->PipelineCacheMutex);
			if (!dispatched && mImpl->Pipelines.find(key) == mImpl->Pipelines.end())
			{
				return nil;
			}

			mImpl->VariantReadySignal.wait(lock, [this, &key]
			{
				auto found = mImpl->Pipelines.find(key);
				return found != mImpl->Pipelines.end() && found->second.Ready;
			});

			return mImpl->Pipelines[key].Pipeline;
		}

		struct MetalGpuComputePipelineState::Impl
		{
			id<MTLComputePipelineState> Pipeline = nil;
			bool Ready = false;
			bool InitializeStarted = false;
			Mutex PipelineMutex;
			ConditionVariable PipelineReadySignal;
		};

		MetalGpuComputePipelineState::MetalGpuComputePipelineState(MetalGpuDevice& gpuDevice, const GpuComputePipelineStateCreateInformation& createInformation)
			: GpuComputePipelineState(gpuDevice, createInformation), mGpuDevice(gpuDevice), mImpl(B3DMakeUnique<Impl>())
		{ }

		MetalGpuComputePipelineState::~MetalGpuComputePipelineState()
		{
			if (mImpl)
			{
				Lock lock(mImpl->PipelineMutex);
				if (mImpl->InitializeStarted)
					mImpl->PipelineReadySignal.wait(lock, [this]{ return mImpl->Ready; });

#if !__has_feature(objc_arc)
				[mImpl->Pipeline release];
#endif
				mImpl->Pipeline = nil;
			}

		}

		id<MTLComputePipelineState> MetalGpuComputePipelineState::GetMetalPipeline() const
		{
			Lock lock(mImpl->PipelineMutex);
			mImpl->PipelineReadySignal.wait(lock, [this]{ return mImpl->Ready; });

			return mImpl->Pipeline;
		}

		void MetalGpuComputePipelineState::Initialize()
		{
			// Pipeline state creation allocates autoreleased NSError instances (and potentially
			// localizedDescription strings on failure). Drain them locally — fiber-scheduled frames
			// may never hit a runloop.
			@autoreleasepool
			{
			{
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
					mImpl->PipelineReadySignal.notify_all();
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
					mImpl->PipelineReadySignal.notify_all();
				}

				GpuComputePipelineState::Initialize();
				return;
			}

			auto program = std::static_pointer_cast<MetalGpuProgram>(mData.Program);
			id<MTLFunction> function = program->GetMetalFunction();
			if (function == nil)
			{
				B3D_LOG(Error, LogRenderBackend, "Cannot initialize Metal compute pipeline: program '{0}' has no Metal function. Compiler output: {1}", program->GetName(), program->GetCompileErrorMessage());
				{
					Lock lock(mImpl->PipelineMutex);
					mImpl->Ready = true;
					mImpl->PipelineReadySignal.notify_all();
				}

				GpuComputePipelineState::Initialize();
				return;
			}

			const u32* programWorkgroup = program->GetWorkgroupSize();
			mWorkgroupSize[0] = programWorkgroup[0];
			mWorkgroupSize[1] = programWorkgroup[1];
			mWorkgroupSize[2] = programWorkgroup[2];

			MTLComputePipelineDescriptor* descriptor = [[MTLComputePipelineDescriptor alloc] init];
			descriptor.computeFunction = function;

			// Every dispatch uses exactly this workgroup size (dispatchThreadgroups, never partial groups), so
			// the promise can be made per pipeline. 32 is the SIMD-group width of every Apple-silicon GPU.
			// Promising a multiple when it isn't one makes dispatches undefined, so smaller groups keep NO.
			constexpr u32 kSimdGroupWidth = 32;
			const u32 threadCountPerGroup = mWorkgroupSize[0] * mWorkgroupSize[1] * mWorkgroupSize[2];
			descriptor.threadGroupSizeIsMultipleOfThreadExecutionWidth = (threadCountPerGroup != 0 && threadCountPerGroup % kSimdGroupWidth == 0) ? YES : NO;

			Impl* impl = mImpl.get();

			[device newComputePipelineStateWithDescriptor:descriptor options:MTLPipelineOptionNone completionHandler:^(id<MTLComputePipelineState> pipeline, MTLComputePipelineReflection* /*reflection*/, NSError* error)
			{
				if (pipeline == nil)
				{
					NSString* errorString = error ? [error localizedDescription] : @"unknown error";
					B3D_LOG(Error, LogRenderBackend, "Failed to create Metal compute pipeline state: {0}", String([errorString UTF8String]));
				}

				// Publish the pipeline result and notify all waiters.
				{
					Lock completionLock(impl->PipelineMutex);
#if !__has_feature(objc_arc)
					[pipeline retain];
#endif
					impl->Pipeline = pipeline;
					impl->Ready = true;
					impl->PipelineReadySignal.notify_all();
				}

			}];

#if !__has_feature(objc_arc)
			[descriptor release];
#endif

			GpuComputePipelineState::Initialize();
			} // @autoreleasepool
		}
	} // namespace render
} // namespace b3d
