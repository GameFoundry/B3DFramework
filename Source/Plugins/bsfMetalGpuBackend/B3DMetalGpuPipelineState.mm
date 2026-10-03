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
		MetalGpuGraphicsPipelineState::MetalGpuGraphicsPipelineState(MetalGpuDevice& gpuDevice, const GpuGraphicsPipelineStateCreateInformation& createInformation)
			: GpuGraphicsPipelineState(gpuDevice, createInformation), mGpuDevice(gpuDevice)
		{ }

		MetalGpuGraphicsPipelineState::~MetalGpuGraphicsPipelineState()
		{
			{
				// Drain any in-flight compiles so completion handlers cannot reference a destroyed pipeline state.
				Lock lock(mPipelineCacheMutex);
				mVariantReadySignal.wait(lock, [this]
				{
					for (auto& entry : mPipelines)
					{
						if (!entry.second.Ready)
							return false;
					}

					return true;
				});

				for (auto& entry : mPipelines)
					entry.second.Pipeline = nil;

				mPipelines.clear();
			}
			for (u32 variantIndex = 0; variantIndex < 4; variantIndex++)
				mDepthStencilStates[variantIndex] = nil;

		}

		static id<MTLDepthStencilState> CreateDepthStencilState(id<MTLDevice> device, const DepthStencilStateInformation& depthStencil, bool depthReadOnly, bool stencilReadOnly);

		id<MTLDepthStencilState> MetalGpuGraphicsPipelineState::GetMetalDepthStencilState(bool depthReadOnly, bool stencilReadOnly)
		{
			const u32 variantIndex = (depthReadOnly ? 1u : 0u) | (stencilReadOnly ? 2u : 0u);

			Lock lock(mPipelineCacheMutex);
			if (mDepthStencilStates[variantIndex] == nil)
				mDepthStencilStates[variantIndex] = CreateDepthStencilState(mGpuDevice.GetMetalDevice(), mData.DepthStencilState, depthReadOnly, stencilReadOnly);

			return mDepthStencilStates[variantIndex];
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
			}

			id<MTLDepthStencilState> state = [device newDepthStencilStateWithDescriptor:descriptor];
			return state;
		}

		void MetalGpuGraphicsPipelineState::Initialize()
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
			mDepthStencilStates[0] = CreateDepthStencilState(device, mData.DepthStencilState, false, false);

			GpuGraphicsPipelineState::Initialize();
		}

		bool MetalGpuGraphicsPipelineState::StartCompile(const MetalPipelineVariantKey& key, const TShared<MetalVertexInput>& vertexInput)
		{
			// Fast path: variant already compiled (success or failure) on a prior call, or compile in flight.
			{
				Lock lock(mPipelineCacheMutex);
				if (mPipelines.find(key) != mPipelines.end())
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
				Lock lock(mPipelineCacheMutex);
				if (mPipelines.find(key) != mPipelines.end())
					return false;

				mPipelines[key] = CachedVariant{};
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

			const MetalPipelineVariantKey keyCopy = key;

			// The destructor drains pending compiles, so the handler can safely capture this.
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
					Lock completionLock(mPipelineCacheMutex);
					auto& entry = mPipelines[keyCopy];
					entry.Pipeline = pipeline;
					entry.Ready = true;

					mVariantReadySignal.notify_all();
				}

			}];

			return true;
		}

		id<MTLRenderPipelineState> MetalGpuGraphicsPipelineState::GetOrCreateMetalPipeline(const MetalPipelineVariantKey& key, const TShared<MetalVertexInput>& vertexInput)
		{
			const bool dispatched = StartCompile(key, vertexInput);

			Lock lock(mPipelineCacheMutex);
			if (!dispatched && mPipelines.find(key) == mPipelines.end())
			{
				return nil;
			}

			mVariantReadySignal.wait(lock, [this, &key]
			{
				auto found = mPipelines.find(key);
				return found != mPipelines.end() && found->second.Ready;
			});

			return mPipelines[key].Pipeline;
		}

		MetalGpuComputePipelineState::MetalGpuComputePipelineState(MetalGpuDevice& gpuDevice, const GpuComputePipelineStateCreateInformation& createInformation)
			: GpuComputePipelineState(gpuDevice, createInformation), mGpuDevice(gpuDevice)
		{ }

		MetalGpuComputePipelineState::~MetalGpuComputePipelineState()
		{
			Lock lock(mPipelineMutex);
			if (mInitializeStarted)
				mPipelineReadySignal.wait(lock, [this]{ return mReady; });

			mPipeline = nil;
		}

		id<MTLComputePipelineState> MetalGpuComputePipelineState::GetMetalPipeline() const
		{
			Lock lock(mPipelineMutex);
			mPipelineReadySignal.wait(lock, [this]{ return mReady; });

			return mPipeline;
		}

		void MetalGpuComputePipelineState::Initialize()
		{
			{
				Lock lock(mPipelineMutex);
				mInitializeStarted = true;
			}

			id<MTLDevice> device = mGpuDevice.GetMetalDevice();
			if (device == nil)
			{
				B3D_LOG(Error, LogRenderBackend, "Cannot initialize Metal compute pipeline: device is null.");
				{
					Lock lock(mPipelineMutex);
					mReady = true;
					mPipelineReadySignal.notify_all();
				}

				GpuComputePipelineState::Initialize();
				return;
			}

			if (!mData.Program)
			{
				B3D_LOG(Error, LogRenderBackend, "Cannot initialize Metal compute pipeline: compute program is null.");
				{
					Lock lock(mPipelineMutex);
					mReady = true;
					mPipelineReadySignal.notify_all();
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
					Lock lock(mPipelineMutex);
					mReady = true;
					mPipelineReadySignal.notify_all();
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

			[device newComputePipelineStateWithDescriptor:descriptor options:MTLPipelineOptionNone completionHandler:^(id<MTLComputePipelineState> pipeline, MTLComputePipelineReflection* /*reflection*/, NSError* error)
			{
				if (pipeline == nil)
				{
					NSString* errorString = error ? [error localizedDescription] : @"unknown error";
					B3D_LOG(Error, LogRenderBackend, "Failed to create Metal compute pipeline state: {0}", String([errorString UTF8String]));
				}

				// Publish the pipeline result and notify all waiters.
				{
					Lock completionLock(mPipelineMutex);
					mPipeline = pipeline;
					mReady = true;
					mPipelineReadySignal.notify_all();
				}

			}];

			GpuComputePipelineState::Initialize();
		}
	} // namespace render
} // namespace b3d
