//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DMetalGpuCommandBuffer.h"
#include "GpuBackend/B3DGpuBackendUtility.h"
#include "B3DMetalGpuDevice.h"
#include "B3DMetalGpuQueue.h"
#include "B3DMetalGpuCommandBufferPool.h"
#include "B3DMetalGpuProgram.h"
#include "B3DMetalGpuBuffer.h"
#include "B3DMetalTexture.h"
#include "B3DMetalSamplerState.h"
#include "B3DMetalGpuParameterSet.h"
#include "B3DMetalGpuPipelineParameterLayout.h"
#include "B3DMetalGpuQueryPool.h"
#include "B3DMetalGpuTimelineFence.h"
#include "B3DMetalRenderTexture.h"
#include "B3DMetalFramebuffer.h"
#include "B3DMetalRenderWindowSurface.h"
#include "B3DMetalClearPipeline.h"
#include "B3DMetalHeapAllocator.h"
#include "B3DMetalShaderABI.h"
#include "B3DMetalUtility.h"
#include "B3DIMetalRenderWindowSurface.h"
#include "GpuBackend/B3DRenderWindow.h"
#include "GpuBackend/B3DGpuSubmitThread.h"
#include "GpuBackend/B3DGpuVertexInputManager.h"
#include "B3DMetalVertexInput.h"
#include "Image/B3DPixelUtility.h"
#include "Utility/B3DBitwise.h"
#include "Debug/B3DLog.h"

namespace b3d
{
	namespace render
	{
		namespace
		{
			/** Stages a render encoder executes. */
			constexpr MTLStages kRenderEncoderStages = MTLStageVertex | MTLStageFragment | MTLStageTile | MTLStageObject | MTLStageMesh;

			/** Stages a compute encoder executes. */
			constexpr MTLStages kComputeEncoderStages = MTLStageDispatch | MTLStageBlit | MTLStageAccelerationStructure;

			/** Stages an intra-encoder barrier of a render encoder can wait for. Apple GPUs cannot wait for fragment work within a render encoder. */
			constexpr MTLStages kRenderEncoderSourceStages = MTLStageVertex | MTLStageObject | MTLStageMesh;

			/** Size of the shared buffers upload memory is suballocated from. */
			constexpr u64 kUploadChunkSize = 64 * 1024;

			/** Alignment of upload suballocations, which Metal requires for constant buffer offsets. */
			constexpr u32 kUploadAlignment = 256;

			/**
			 * Sets the load and store actions of every attachment @p descriptor has a texture for. Attachments neither loaded nor
			 * cleared use DontCare, and so do attachments not in @p storeMask when the pass ends.
			 */
			void ConfigureAttachmentActions(MTL4RenderPassDescriptor* descriptor, RenderSurfaceMask loadMask, RenderSurfaceMask clearMask, RenderSurfaceMask storeMask,
				const RenderTargetClearValues& clearValues)
			{
				auto fnGetStoreAction = [&storeMask](RenderSurfaceMaskBits surface)
				{
					return storeMask.IsSet(surface) ? MTLStoreActionStore : MTLStoreActionDontCare;
				};

				auto fnGetLoadAction = [&loadMask, &clearMask](RenderSurfaceMaskBits surface)
				{
					if (clearMask.IsSet(surface))
						return MTLLoadActionClear;

					return loadMask.IsSet(surface) ? MTLLoadActionLoad : MTLLoadActionDontCare;
				};

				for (u32 attachmentIndex = 0; attachmentIndex < B3D_MAXIMUM_RENDER_TARGET_COUNT; attachmentIndex++)
				{
					MTLRenderPassColorAttachmentDescriptor* color = descriptor.colorAttachments[attachmentIndex];
					if (color.texture == nil)
						continue;

					const Color& clearColor = clearValues.Colors[attachmentIndex];
					color.loadAction = fnGetLoadAction((RenderSurfaceMaskBits)(RT_COLOR0 << attachmentIndex));
					color.clearColor = MTLClearColorMake(clearColor.R, clearColor.G, clearColor.B, clearColor.A);
					color.storeAction = fnGetStoreAction((RenderSurfaceMaskBits)(RT_COLOR0 << attachmentIndex));
				}

				MTLRenderPassDepthAttachmentDescriptor* depth = descriptor.depthAttachment;
				if (depth.texture != nil)
				{
					depth.loadAction = fnGetLoadAction(RT_DEPTH);
					depth.clearDepth = clearValues.Depth;
					depth.storeAction = fnGetStoreAction(RT_DEPTH);
				}

				MTLRenderPassStencilAttachmentDescriptor* stencil = descriptor.stencilAttachment;
				if (stencil.texture != nil)
				{
					stencil.loadAction = fnGetLoadAction(RT_STENCIL);
					stencil.clearStencil = clearValues.Stencil;
					stencil.storeAction = fnGetStoreAction(RT_STENCIL);
				}
			}

			/** Stores the attachment formats of @p descriptor in the pipeline variant key. Every MTLPixelFormat value fits in 16 bits. */
			void PackAttachmentFormats(MTL4RenderPassDescriptor* descriptor, MetalPipelineVariantKey& key)
			{
				for (u32 attachmentIndex = 0; attachmentIndex < B3D_MAXIMUM_RENDER_TARGET_COUNT; attachmentIndex++)
				{
					id<MTLTexture> texture = descriptor.colorAttachments[attachmentIndex].texture;
					if (texture != nil)
						key.ColorFormats[attachmentIndex] = (u16)[texture pixelFormat];
				}

				if (descriptor.depthAttachment.texture != nil)
					key.DepthFormat = (u32)[descriptor.depthAttachment.texture pixelFormat];

				if (descriptor.stencilAttachment.texture != nil)
					key.StencilFormat = (u32)[descriptor.stencilAttachment.texture pixelFormat];
			}

			constexpr u32 kMetalBufferCopyAlignment = 4;
			constexpr u32 kMetalTextureBufferCopyAlignment = 16;

			bool IsBufferRangeValid(const GpuBuffer& buffer, u32 offset, u64 length)
			{
				return (u64)offset + length <= (u64)buffer.GetTotalSize();
			}

			struct MetalTextureTransferInformation
			{
				u32 Width = 0;
				u32 Height = 0;
				u32 Depth = 0;
				u32 RowPitch = 0;
				u32 SlicePitch = 0;
				u64 RequiredBufferSize = 0;
				MTLBlitOption Options = MTLBlitOptionNone;
			};

			bool GetTextureTransferInformation(const Texture& texture, const GpuBuffer& buffer, u32 bufferOffset, u32 mipLevel, u32 arrayLayer, const char* operation, MetalTextureTransferInformation& output)
			{
				const TextureProperties& properties = texture.GetProperties();
				const u32 faceCount = properties.Type == TEX_TYPE_3D ? 1u : properties.GetFaceCount();
				if (mipLevel > properties.MipMapCount || arrayLayer >= faceCount)
				{
					B3D_LOG(Error, LogRenderBackend, "{0}: texture mip level or array layer is out of range.", operation);
					return false;
				}

				if (properties.SampleCount > 1)
				{
					B3D_LOG(Error, LogRenderBackend, "{0}: buffer transfers do not support multisampled textures.", operation);
					return false;
				}

				if ((bufferOffset % kMetalTextureBufferCopyAlignment) != 0)
				{
					B3D_LOG(Error, LogRenderBackend, "{0}: buffer offset {1} must be 16-byte aligned on Apple GPUs.", operation, bufferOffset);
					return false;
				}

				PixelUtility::GetSizeForMipLevel(properties.Width, properties.Height, properties.Depth, mipLevel, output.Width, output.Height, output.Depth);

				if (properties.Type == TEX_TYPE_1D)
				{
					output.Height = 1;
					output.Depth = 1;
				}
				else if (properties.Type != TEX_TYPE_3D)
					output.Depth = 1;

				if (properties.Format == PF_D32_S8X24)
				{
					// Same as Vulkan, only the depth aspect is transferred, as tightly packed 32-bit floats
					const u64 depthRowPitch = (u64)std::max(1u, output.Width) * sizeof(float);
					const u64 depthSlicePitch = depthRowPitch * std::max(1u, output.Height);
					if (depthRowPitch > (u64)~0u || depthSlicePitch > (u64)~0u)
						return false;

					output.RowPitch = (u32)depthRowPitch;
					output.SlicePitch = (u32)depthSlicePitch;
					output.Options = MTLBlitOptionDepthFromDepthStencil;
				}
				else
				{
					output.RowPitch = MetalUtility::GetTextureRowPitch(properties.Format, output.Width);
					output.SlicePitch = MetalUtility::GetTextureSlicePitch(properties.Format, output.Width, output.Height);
				}

				if (output.RowPitch == 0 || output.SlicePitch == 0)
				{
					B3D_LOG(Error, LogRenderBackend, "{0}: texture transfer pitch overflowed or is invalid.", operation);
					return false;
				}

				output.RequiredBufferSize = (u64)output.SlicePitch * output.Depth;
				if (!IsBufferRangeValid(buffer, bufferOffset, output.RequiredBufferSize))
				{
					B3D_LOG(Error, LogRenderBackend, "{0}: buffer range [{1}, {2}) exceeds the buffer size ({3}).", operation, bufferOffset, (u64)bufferOffset + output.RequiredBufferSize, buffer.GetTotalSize());
					return false;
				}

				if (properties.Format == PF_D32_S8X24 && (u64)buffer.GetTotalSize() - bufferOffset != output.RequiredBufferSize)
				{
					B3D_LOG(Error, LogRenderBackend, "{0}: combined depth/stencil transfers require an exactly-sized packed Depth32Float buffer. Interleaved PF_D32_S8X24 staging data is not a valid Metal depth-plane layout.", operation);
					return false;
				}

				return true;
			}
		} // namespace

		void MetalGpuCommandBuffer::ArgumentTable::SetAddress(MTLGPUAddress address, u32 index)
		{
			B3D_ASSERT(index < kArgumentTableBufferCount);
			if (Addresses[index] == address)
				return;

			[Table setAddress:address atIndex:index];
			Addresses[index] = address;
		}

		MetalGpuCommandBuffer::MetalGpuCommandBuffer(MetalGpuDevice& device, MetalGpuCommandBufferPool& pool, u32 id, GpuQueueType queueType, const GpuCommandBufferCreateInformation& createInformation)
			: GpuCommandBuffer(device, pool, queueType, createInformation), mGpuDevice(device), mPool(pool), mId(id)
		{
			// The id parameter hides the Objective-C id type
			const auto metalDevice = device.GetMetalDevice();
			mCommandBuffer = [metalDevice newCommandBuffer];
			mCommandAllocator = [metalDevice newCommandAllocator];

			MTL4ArgumentTableDescriptor* argumentTableDescriptor = [[MTL4ArgumentTableDescriptor alloc] init];
			argumentTableDescriptor.maxBufferBindCount = kArgumentTableBufferCount;

			NSError* error = nil;
			mVertexArgumentTable.Table = [metalDevice newArgumentTableWithDescriptor:argumentTableDescriptor error:&error];
			mFragmentArgumentTable.Table = [metalDevice newArgumentTableWithDescriptor:argumentTableDescriptor error:&error];
			mComputeArgumentTable.Table = [metalDevice newArgumentTableWithDescriptor:argumentTableDescriptor error:&error];

			if (mCommandBuffer == nil || mCommandAllocator == nil || mVertexArgumentTable.Table == nil || mFragmentArgumentTable.Table == nil || mComputeArgumentTable.Table == nil)
			{
				B3D_LOG(Fatal, LogRenderBackend, "Failed to create the native objects of a Metal command buffer: {0}",
					error ? String([[error localizedDescription] UTF8String]) : String("No error details were provided."));
				mRecordingFailed = true;
			}
		}

		MetalGpuCommandBuffer::~MetalGpuCommandBuffer()
		{
			CloseEncoder();
			if (mIsNativeRecording)
				[mCommandBuffer endCommandBuffer];

			MetalHeapAllocator& heapAllocator = mGpuDevice.GetHeapAllocator();
			for (UploadChunk& chunk : mUploadChunks)
				heapAllocator.Free(chunk.Buffer, chunk.Allocation);
		}

		bool MetalGpuCommandBuffer::BeginNativeCommandBuffer()
		{
			if (mRecordingFailed)
				return false;

			if (mIsNativeRecording)
				return true;

			[mCommandBuffer beginCommandBufferWithAllocator:mCommandAllocator];
			mIsNativeRecording = true;
			mHasNativeCommands = true;

#if B3D_BUILD_TYPE_DEVELOPMENT
			if (!mName.empty())
				mCommandBuffer.label = [NSString stringWithUTF8String:mName.c_str()];
#endif

			// Metal has no explicit Begin(), so the first native command marks the start of recording
			if (mState == GpuCommandBufferState::Ready)
				mState = GpuCommandBufferState::Recording;

			return true;
		}

		void MetalGpuCommandBuffer::CloseEncoder()
		{
			if (mRenderEncoder != nil)
			{
				[mRenderEncoder endEncoding];
				mRenderEncoder = nil;
			}

			if (mComputeEncoder != nil)
			{
				[mComputeEncoder endEncoding];
				mComputeEncoder = nil;
			}

			mEncoderHasWork = false;
		}

		id<MTL4ComputeCommandEncoder> MetalGpuCommandBuffer::OpenComputeEncoder()
		{
			if (mComputeEncoder != nil)
				return mComputeEncoder;

			CloseEncoder();
			if (!BeginNativeCommandBuffer())
				return nil;

			mComputeEncoder = [mCommandBuffer computeCommandEncoder];
			if (mComputeEncoder == nil)
			{
				B3D_LOG(Fatal, LogRenderBackend, "Failed to open a Metal compute encoder.");
				mRecordingFailed = true;
				return nil;
			}

#if B3D_BUILD_TYPE_DEVELOPMENT
			mComputeEncoder.label = @"Compute pass";
#endif

			[mComputeEncoder setArgumentTable:mComputeArgumentTable.Table];
			mCurrentComputePipeline = nil;

			EmitPendingBarrier();
			return mComputeEncoder;
		}

		bool MetalGpuCommandBuffer::OpenRenderEncoder(MTL4RenderPassDescriptor* descriptor)
		{
			CloseEncoder();
			if (!BeginNativeCommandBuffer())
				return false;

			mRenderEncoder = [mCommandBuffer renderCommandEncoderWithDescriptor:descriptor];
			if (mRenderEncoder == nil)
				return false;

			[mRenderEncoder setArgumentTable:mVertexArgumentTable.Table atStages:MTLRenderStageVertex];
			[mRenderEncoder setArgumentTable:mFragmentArgumentTable.Table atStages:MTLRenderStageFragment];
			mCurrentRenderPipeline = nil;
			mCurrentDepthStencilState = nil;
			mCurrentStencilReference = 0;

			EmitPendingBarrier();
			return true;
		}

		void MetalGpuCommandBuffer::EmitBarrier(const MetalStageBarrier& barrier)
		{
			if (barrier.IsEmpty())
				return;

			if (mRenderEncoder == nil && mComputeEncoder == nil)
			{
				mPendingBarrier.Merge(barrier);
				return;
			}

			const bool isRenderEncoder = mRenderEncoder != nil;
			id<MTL4CommandEncoder> encoder = isRenderEncoder ? (id<MTL4CommandEncoder>)mRenderEncoder : (id<MTL4CommandEncoder>)mComputeEncoder;

			// Work the open encoder will encode waits for earlier encoders through a consumer barrier, and for earlier work
			// in the encoder through an intra-encoder barrier
			const MTLStages encoderDestinationStages = barrier.DestinationStages & (isRenderEncoder ? kRenderEncoderStages : kComputeEncoderStages);
			if (encoderDestinationStages != 0)
			{
				[encoder barrierAfterQueueStages:barrier.SourceStages beforeStages:encoderDestinationStages visibilityOptions:barrier.Visibility];

				const MTLStages encoderSourceStages = barrier.SourceStages & (isRenderEncoder ? kRenderEncoderSourceStages : kComputeEncoderStages);
				if (mEncoderHasWork && encoderSourceStages != 0)
					[encoder barrierAfterEncoderStages:encoderSourceStages beforeEncoderStages:encoderDestinationStages visibilityOptions:barrier.Visibility];
			}

			// Consumer barriers only order the encoder they are encoded in. Every later encoder, including those of command
			// buffers committed to the queue later, waits through a producer barrier.
			[encoder barrierAfterStages:barrier.SourceStages beforeQueueStages:barrier.DestinationStages visibilityOptions:barrier.Visibility];
		}

		void MetalGpuCommandBuffer::EmitPendingBarrier()
		{
			const MetalStageBarrier pendingBarrier = mPendingBarrier;
			mPendingBarrier = MetalStageBarrier();
			EmitBarrier(pendingBarrier);
		}

		bool MetalGpuCommandBuffer::QueueEventSignal(id<MTLSharedEvent> event, u64 value)
		{
			EnsureValidThread();

			if (event == nil || mRecordingFailed)
				return false;

			PendingEventSignal signal;
			signal.Event = event;
			signal.Value = value;
			mPendingEventSignals.push_back(signal);

			return true;
		}

		void MetalGpuCommandBuffer::RetainUntilComplete(id object)
		{
			if (object != nil)
				mRetainedObjects.push_back(object);
		}

		void MetalGpuCommandBuffer::TrackSurfaceImage(MetalImage& image, GpuAccessFlags access)
		{
			mResourceTracker.TrackResourceUsage(&image, access);
		}

		void MetalGpuCommandBuffer::UseDrawableResidency(id<CAMetalDrawable> drawable)
		{
			id<MTLResidencySet> residencySet = drawable.layer.residencySet;
			if (residencySet == nil || std::find(mDrawableResidencySets.begin(), mDrawableResidencySets.end(), residencySet) != mDrawableResidencySets.end())
				return;

			B3D_ASSERT(mIsNativeRecording);
			[mCommandBuffer useResidencySet:residencySet];
			mDrawableResidencySets.push_back(residencySet);
		}

		void MetalGpuCommandBuffer::SetName(const StringView& name)
		{
			EnsureValidThread();
			mName = name;

			NSString* label = [NSString stringWithUTF8String:mName.c_str()];
			mCommandBuffer.label = label;
		}

		void MetalGpuCommandBuffer::SetGpuParameterSet(const TShared<GpuParameterSet>& parameters)
		{
			GpuCommandBuffer::SetGpuParameterSet(parameters);
			EnsureValidThread();

			// A null set carries no set index, so there is no slot to clear. Sets are replaced by binding another set at the same index.
			if (!parameters)
				return;

			const u32 setIndex = parameters->GetSet();
			if (setIndex >= kMetalVertexBufferSlotBase)
			{
				B3D_LOG(Error, LogRenderBackend, "Metal parameter set index {0} collides with reserved vertex-buffer slots.", setIndex);
				return;
			}

			if (setIndex >= (u32)mBoundParameterSets.Size())
			{
				mBoundParameterSets.Resize(setIndex + 1);
				mDynamicOffsetOverridesPerSet.Resize(setIndex + 1);
			}

			mBoundParameterSets[setIndex] = parameters;
			ResetDynamicOffsetOverrides(setIndex);

			// Pending writes are committed and argument buffers bound at draw / dispatch, so binding a set several times
			// before a draw only does that work once
			mGraphicsResourcesRequireTracking = true;
		}

		void MetalGpuCommandBuffer::ResetDynamicOffsetOverrides(u32 setIndex)
		{
			const TShared<GpuParameterSet>& parameters = mBoundParameterSets[setIndex];
			const u32 dynamicOffsetCount = parameters != nullptr && parameters->GetLayout() != nullptr ? parameters->GetLayout()->GetDynamicOffsetCount() : 0;

			TInlineArray<u32, 4>& overrides = mDynamicOffsetOverridesPerSet[setIndex];
			overrides.Clear();
			overrides.Resize(dynamicOffsetCount, ~0u);
		}

		void MetalGpuCommandBuffer::SetDynamicBufferOffset(u32 set, u32 bufferIndex, u32 offset)
		{
			EnsureValidThread();

			if (set >= (u32)mBoundParameterSets.Size() || !mBoundParameterSets[set])
				return;

			TInlineArray<u32, 4>& overrides = mDynamicOffsetOverridesPerSet[set];
			if (bufferIndex >= (u32)overrides.Size())
			{
				B3D_LOG(Error, LogRenderBackend, "Dynamic offset index {0} is out of range for parameter set {1}.", bufferIndex, set);
				return;
			}

#if B3D_BUILD_TYPE_DEVELOPMENT
			// Only uniform buffers declared with a dynamic offset bind through the argument table
			const auto& parameters = static_cast<const MetalGpuParameters&>(*mBoundParameterSets[set]);
			const MetalDynamicUniformBufferBinding* dynamicBinding = nullptr;
			for (const MetalDynamicUniformBufferBinding& binding : parameters.GetMetalLayout()->GetDynamicUniformBufferBindings())
			{
				if (binding.DynamicOffsetIndex == bufferIndex)
					dynamicBinding = &binding;
			}

			if (!B3D_ENSURE_LOG(dynamicBinding != nullptr, "Metal supports dynamic offsets only on uniform buffers declared with a dynamic offset. Set: {0}, index: {1}.", set, bufferIndex))
				return;

			u32 boundOffset = 0;
			if (const GpuBuffer* buffer = parameters.GetBoundUniformBuffer(dynamicBinding->Slot, boundOffset))
			{
				if (!B3D_ENSURE_LOG((offset & 15u) == 0 && offset <= buffer->GetTotalSize() && buffer->GetSuballocationSize() <= buffer->GetTotalSize() - offset, "Dynamic offset {0} is misaligned or leaves insufficient space in the bound uniform buffer ({1} bytes). Offsets must be 16-byte aligned.", offset, buffer->GetTotalSize()))
					return;
			}
#endif

			if (overrides[bufferIndex] == offset)
				return;

			overrides[bufferIndex] = offset;
			mGraphicsResourcesRequireTracking = true;
		}

		void MetalGpuCommandBuffer::SetPushConstants(u32 offsetInBytes, u32 sizeInBytes, const void* data)
		{
			EnsureValidThread();

			if(sizeInBytes == 0)
				return;

			if(!B3D_ENSURE_LOG(data != nullptr, "Push-constant data cannot be null for a non-empty update."))
				return;

			if(!B3D_ENSURE_LOG((offsetInBytes & 3u) == 0 && (sizeInBytes & 3u) == 0, "Push-constant offsets and sizes must be aligned to four bytes."))
				return;

			if(!B3D_ENSURE_LOG(offsetInBytes <= kMaxPushConstantSizeInBytes && sizeInBytes <= kMaxPushConstantSizeInBytes - offsetInBytes, "Push-constant update at offset {0} with size {1} exceeds the {2}-byte Metal block.", offsetInBytes, sizeInBytes, kMaxPushConstantSizeInBytes))
				return;

			mPushConstants.Write(offsetInBytes, sizeInBytes, data);
			mGraphicsPushConstantsRequireBind = true;
			mComputePushConstantsRequireBind = true;
		}

		void MetalGpuCommandBuffer::SetGpuGraphicsPipelineState(const TShared<GpuGraphicsPipelineState>& pipelineState)
		{
			GpuCommandBuffer::SetGpuGraphicsPipelineState(pipelineState);
			EnsureValidThread();

			mBoundGraphicsPipeline = std::static_pointer_cast<MetalGpuGraphicsPipelineState>(pipelineState);
			mGraphicsResourcesRequireTracking = true;
			mGraphicsPushConstantsRequireBind = true;
		}

		void MetalGpuCommandBuffer::SetGpuComputePipelineState(const TShared<GpuComputePipelineState>& pipelineState)
		{
			GpuCommandBuffer::SetGpuComputePipelineState(pipelineState);
			EnsureValidThread();

			mBoundComputePipeline = pipelineState;
			mGraphicsResourcesRequireTracking = true;
			mComputePushConstantsRequireBind = true;
		}

		MTLGPUAddress MetalGpuCommandBuffer::Upload(const void* data, u32 size)
		{
			const u64 alignedSize = Bitwise::AlignUp<u64>(size, kUploadAlignment);
			while (mUploadChunkIndex < (u32)mUploadChunks.size())
			{
				UploadChunk& chunk = mUploadChunks[mUploadChunkIndex];
				if (chunk.Cursor + alignedSize <= (u64)chunk.Buffer.length)
					break;

				mUploadChunkIndex++;
			}

			if (mUploadChunkIndex == (u32)mUploadChunks.size())
			{
				const u64 chunkSize = std::max(kUploadChunkSize, alignedSize);
				MetalHeapAllocator& heapAllocator = mGpuDevice.GetHeapAllocator();
				const GpuAllocation requestedAllocation = GpuAllocation::CreatePending(heapAllocator.GetAllocator(MetalHeapAllocator::kMemoryTypeShared));

				UploadChunk chunk;
				id<MTLBuffer> buffer = heapAllocator.AllocateBuffer(chunkSize, MetalHeapAllocator::kMemoryTypeShared, requestedAllocation, chunk.Allocation);
				if (buffer == nil)
				{
					B3D_LOG(Error, LogRenderBackend, "Failed to allocate a {0}-byte Metal upload buffer.", chunkSize);
					return 0;
				}

#if B3D_BUILD_TYPE_DEVELOPMENT
				buffer.label = @"Command buffer upload memory";
#endif

				chunk.Buffer = buffer;
				mUploadChunks.push_back(chunk);
			}

			UploadChunk& chunk = mUploadChunks[mUploadChunkIndex];
			const u64 offset = chunk.Cursor;
			std::memcpy((u8*)[chunk.Buffer contents] + offset, data, size);
			chunk.Cursor += alignedSize;

			return chunk.Buffer.gpuAddress + offset;
		}

		void MetalGpuCommandBuffer::BindPushConstants(bool isGraphics)
		{
			if((isGraphics && !mGraphicsPushConstantsRequireBind) || (!isGraphics && !mComputePushConstantsRequireBind))
				return;

			const MetalGpuPipelineParameterLayout* parameterLayout = nullptr;
			if(isGraphics && mBoundGraphicsPipeline != nullptr)
				parameterLayout = static_cast<MetalGpuPipelineParameterLayout*>(mBoundGraphicsPipeline->GetParameterLayout().get());
			else if(!isGraphics && mBoundComputePipeline != nullptr)
				parameterLayout = static_cast<MetalGpuPipelineParameterLayout*>(mBoundComputePipeline->GetParameterLayout().get());

			if(parameterLayout == nullptr)
				return;

			const u32 pushConstantBufferSize = parameterLayout->GetPushConstantBufferSize();
			const GpuProgramStageBits stages = parameterLayout->GetPushConstantStages();
			B3D_ASSERT((pushConstantBufferSize & 3u) == 0);
			B3D_ASSERT(pushConstantBufferSize <= kMaxPushConstantSizeInBytes);

			if(pushConstantBufferSize != 0)
			{
				const MTLGPUAddress address = Upload(mPushConstants.GetData(), pushConstantBufferSize);
				if(address == 0)
					return;

				if(isGraphics)
				{
					if(stages.IsSet(GpuProgramStageBit::Vertex))
						mVertexArgumentTable.SetAddress(address, kMetalPushConstantBufferIndex);

					if(stages.IsSet(GpuProgramStageBit::Fragment))
						mFragmentArgumentTable.SetAddress(address, kMetalPushConstantBufferIndex);
				}
				else if(stages.IsSet(GpuProgramStageBit::Compute))
					mComputeArgumentTable.SetAddress(address, kMetalPushConstantBufferIndex);
			}

			if(isGraphics)
				mGraphicsPushConstantsRequireBind = false;
			else
				mComputePushConstantsRequireBind = false;
		}

		void MetalGpuCommandBuffer::BindDynamicUniformBuffers(bool isGraphics)
		{
			const GpuPipelineParameterLayout* pipelineLayout = nullptr;
			if (isGraphics && mBoundGraphicsPipeline != nullptr)
				pipelineLayout = mBoundGraphicsPipeline->GetParameterLayout().get();
			else if (!isGraphics && mBoundComputePipeline != nullptr)
				pipelineLayout = mBoundComputePipeline->GetParameterLayout().get();

			if (pipelineLayout == nullptr)
				return;

			id<MTLBuffer> dummyBuffer = mGpuDevice.GetDummyArgumentBuffer();
			const u32 setCount = pipelineLayout->GetSetCount();
			for (u32 setIndex = 0; setIndex < setCount; setIndex++)
			{
				const auto* parameters = setIndex < (u32)mBoundParameterSets.Size() ? static_cast<const MetalGpuParameters*>(mBoundParameterSets[setIndex].get()) : nullptr;

				// The pipeline's reflected layout supplies the argument-table indices; the parameter set, whose own
				// layout may be an explicitly created compatible one, supplies the buffers matched by slot
				const auto* pipelineSetLayout = static_cast<const MetalGpuPipelineParameterSetLayout*>(pipelineLayout->GetSet(setIndex).get());
				for (const MetalDynamicUniformBufferBinding& binding : pipelineSetLayout->GetDynamicUniformBufferBindings())
				{
					const u32 bufferIndex = binding.BufferIndex;
					if (bufferIndex == ~0u)
						continue;

					B3D_ASSERT(bufferIndex >= kMetalDynamicUniformBufferIndexBase && bufferIndex < kMetalDynamicUniformBufferIndexBase + kMetalDynamicUniformBufferCount);

					u32 offset = 0;
					auto* buffer = parameters != nullptr ? static_cast<MetalGpuBuffer*>(parameters->GetBoundUniformBuffer(binding.Slot, offset)) : nullptr;
					id<MTLBuffer> metalBuffer = buffer != nullptr ? buffer->GetMetalBuffer() : nil;
					if (metalBuffer == nil)
					{
						// An unbound slot reads zeroes from the dummy buffer instead of faulting on a missing binding
						metalBuffer = dummyBuffer;
						offset = 0;
					}
					else
					{
						const TInlineArray<u32, 4>& dynamicOffsetOverrides = mDynamicOffsetOverridesPerSet[setIndex];
						const u32 dynamicOffsetIndex = parameters->GetLayout()->GetDynamicOffsetIndex(binding.Slot);
						if (dynamicOffsetIndex < (u32)dynamicOffsetOverrides.Size() && dynamicOffsetOverrides[dynamicOffsetIndex] != ~0u)
							offset = dynamicOffsetOverrides[dynamicOffsetIndex];
					}

					const MTLGPUAddress address = metalBuffer.gpuAddress + offset;
					if (!isGraphics)
					{
						if (binding.Stages.IsSet(GpuProgramStageBit::Compute))
							mComputeArgumentTable.SetAddress(address, bufferIndex);

						continue;
					}

					if (binding.Stages.IsSetAny(GpuProgramStageBit::Vertex | GpuProgramStageBit::Hull | GpuProgramStageBit::Domain))
						mVertexArgumentTable.SetAddress(address, bufferIndex);

					if (binding.Stages.IsSet(GpuProgramStageBit::Fragment))
						mFragmentArgumentTable.SetAddress(address, bufferIndex);
				}
			}
		}

		void MetalGpuCommandBuffer::BindParameterSets(bool isGraphics)
		{
			for (u32 slotIndex = 0; slotIndex < (u32)mBoundParameterSets.Size(); slotIndex++)
			{
				const TShared<GpuParameterSet>& slotSet = mBoundParameterSets[slotIndex];
				if (!slotSet)
					continue;

				auto& metalParameters = static_cast<MetalGpuParameters&>(*slotSet);

				// Prepare before binding: a copy-on-write can move the argument buffer
				metalParameters.PrepareForBind();

				const MetalGpuPipelineParameterSetLayout* layout = metalParameters.GetMetalLayout();
				id<MTLBuffer> argumentBuffer = metalParameters.GetArgumentBuffer();
				if (!layout || argumentBuffer == nil)
					continue;

				// Defers the release of a heap-placed argument buffer the set replaces (copy-on-write or destruction) until
				// this command buffer retires. Transient pool slices have no resource and rely on the pool's Reset contract.
				if (MetalArgumentBuffer* resource = metalParameters.GetArgumentBufferResource())
					mResourceTracker.TrackResourceUsage(resource, GpuAccessFlag::Read);

#if B3D_BUILD_TYPE_DEVELOPMENT
				// Validation cannot detect missing residency, the GPU reads garbage or faults instead
				const MetalResidencySet& residencySet = mGpuDevice.GetHeapAllocator().GetResidencySet();
				B3D_ASSERT(residencySet.IsResident(argumentBuffer));
				for (const MetalArgumentBufferBinding& binding : layout->GetBindings())
				{
					for (u32 arrayIndex = 0; arrayIndex < binding.ArraySize; arrayIndex++)
					{
						id<MTLResource> resource = metalParameters.GetCachedResource(binding.FirstResourceIndex + arrayIndex);
						B3D_ENSURE_LOG(residencySet.IsResident(resource), "Resource bound to parameter set {0}, slot {1}, is not resident.", slotIndex, binding.Slot);
					}
				}
#endif

				const MTLGPUAddress address = argumentBuffer.gpuAddress + metalParameters.GetArgumentBufferOffset();
				const GpuProgramStageBits stages = layout->GetCombinedStages();
				if (!isGraphics)
				{
					if (stages.IsSet(GpuProgramStageBit::Compute))
						mComputeArgumentTable.SetAddress(address, metalParameters.GetSet());

					continue;
				}

				if (stages.IsSetAny(GpuProgramStageBit::Vertex | GpuProgramStageBit::Hull | GpuProgramStageBit::Domain))
					mVertexArgumentTable.SetAddress(address, metalParameters.GetSet());

				if (stages.IsSet(GpuProgramStageBit::Fragment))
					mFragmentArgumentTable.SetAddress(address, metalParameters.GetSet());
			}
		}

		void MetalGpuCommandBuffer::SetVertexBuffers(u32 index, TShared<GpuBuffer>* buffers, u32 bufferCount)
		{
			GpuCommandBuffer::SetVertexBuffers(index, buffers, bufferCount);
			EnsureValidThread();

			// Native handles are resolved at draw time, see mBoundVertexBuffers
			const u32 endIndex = index + bufferCount;
			while ((u32)mBoundVertexBuffers.Size() < endIndex)
				mBoundVertexBuffers.Add(nullptr);

			for (u32 bufferIndex = 0; bufferIndex < bufferCount; bufferIndex++)
				mBoundVertexBuffers[index + bufferIndex] = std::static_pointer_cast<MetalGpuBuffer>(buffers[bufferIndex]);
		}

		void MetalGpuCommandBuffer::BindVertexBuffers()
		{
			for (u32 streamIndex = 0; streamIndex < (u32)mBoundVertexBuffers.Size(); streamIndex++)
			{
				const TShared<MetalGpuBuffer>& metalBuffer = mBoundVertexBuffers[streamIndex];
				id<MTLBuffer> buffer = metalBuffer ? metalBuffer->GetMetalBuffer() : nil;
				if (buffer == nil)
					continue;

				if (MetalBuffer* resource = metalBuffer->GetMetalResource())
					mResourceTracker.TrackBufferUsage(resource, GpuAccessFlag::Read);

				// Vertex streams sit above the argument buffer slots, matching the pipeline's vertex descriptor
				mVertexArgumentTable.SetAddress(buffer.gpuAddress, kMetalVertexBufferSlotBase + streamIndex);
			}
		}

		void MetalGpuCommandBuffer::SetIndexBuffer(const TShared<GpuBuffer>& buffer)
		{
			GpuCommandBuffer::SetIndexBuffer(buffer);
			EnsureValidThread();
			mBoundIndexBuffer = buffer;
		}

		void MetalGpuCommandBuffer::SetVertexDescription(const TShared<VertexDescription>& vertexDescription)
		{
			EnsureValidThread();
			mBoundVertexDescription = vertexDescription;
		}

		void MetalGpuCommandBuffer::SetDrawOperation(DrawOperationType operation)
		{
			EnsureValidThread();
			mDrawOperation = operation;
		}

		MetalVertexInput* MetalGpuCommandBuffer::ResolveVertexInputForDraw(bool& outSkipDraw)
		{
			outSkipDraw = false;

			// Pipelines without vertex inputs (e.g. fullscreen passes) keep VertexInputId 0 in the variant key
			MetalGpuGraphicsPipelineState* pipeline = mBoundGraphicsPipeline.get();
			if (pipeline == nullptr || pipeline->GetInputDeclaration() == nullptr)
				return nullptr;

			if (!mBoundVertexDescription)
			{
				B3D_LOG(Warning, LogRenderBackend, "Skipping draw: graphics pipeline declares vertex inputs but no vertex description is bound.");
				outSkipDraw = true;
				return nullptr;
			}

			MetalVertexInput* vertexInput = static_cast<MetalVertexInput*>(mGpuDevice.GetVertexInputManager().GetVertexInput(mBoundVertexDescription, pipeline->GetInputDeclaration(), mResourceTracker));
			if (!vertexInput)
			{
				B3D_LOG(Warning, LogRenderBackend, "Skipping draw: vertex input could not be resolved for the bound vertex description on Metal.");
				outSkipDraw = true;
				return nullptr;
			}

			// Shader inputs with no matching vertex buffer element read zeroes from the null stream
			if (vertexInput->HasNullStream())
				mVertexArgumentTable.SetAddress(mGpuDevice.GetNullVertexBuffer().gpuAddress, kMetalVertexBufferSlotBase + vertexInput->GetNullStreamIndex());

			return vertexInput;
		}

		bool MetalGpuCommandBuffer::TrackShaderResources(bool compute)
		{
			if(!compute && !mGraphicsResourcesRequireTracking)
				return true;

#if B3D_BUILD_TYPE_DEVELOPMENT
			if(!compute)
				mDrawAccessValidator.ClearBindings();
#endif

			const GpuPipelineParameterLayout* const pipelineLayout = compute
				? (mBoundComputePipeline != nullptr ? mBoundComputePipeline->GetParameterLayout().get() : nullptr)
				: (mBoundGraphicsPipeline != nullptr ? mBoundGraphicsPipeline->GetParameterLayout().get() : nullptr);

			for(u32 setIndex = 0; setIndex < (u32)mBoundParameterSets.Size(); setIndex++)
			{
				const TShared<GpuParameterSet>& parameters = mBoundParameterSets[setIndex];
				if(parameters == nullptr)
					continue;

				// A set the pipeline declares no resources in is not accessed, even if one remains bound from an earlier draw
				const GpuPipelineParameterSetLayout* const setLayout = pipelineLayout != nullptr && setIndex < pipelineLayout->GetSetCount() ? pipelineLayout->GetSet(setIndex).get() : nullptr;
				if(pipelineLayout != nullptr && (setLayout == nullptr || setLayout->GetResourceCount() == 0))
					continue;

				if(!static_cast<MetalGpuParameters&>(*parameters).TrackResources(mResourceTracker, mDynamicOffsetOverridesPerSet[setIndex], compute))
					return false;

#if B3D_BUILD_TYPE_DEVELOPMENT
				if(!compute && setLayout != nullptr)
					mDrawAccessValidator.AddParameterSet(*parameters, *setLayout);
#endif
			}

			mGraphicsResourcesRequireTracking = compute;
			return true;
		}

		void MetalGpuCommandBuffer::SetRenderPipelineState(id<MTLRenderPipelineState> pipeline)
		{
			if (pipeline == mCurrentRenderPipeline)
				return;

			[mRenderEncoder setRenderPipelineState:pipeline];
			mCurrentRenderPipeline = pipeline;
			RetainUntilComplete(pipeline);
		}

		void MetalGpuCommandBuffer::SetDepthStencilState(id<MTLDepthStencilState> state)
		{
			if (state == mCurrentDepthStencilState)
				return;

			[mRenderEncoder setDepthStencilState:state];
			mCurrentDepthStencilState = state;
			RetainUntilComplete(state);
		}

		void MetalGpuCommandBuffer::SetEncoderStencilReference(u32 value)
		{
			if (value == mCurrentStencilReference)
				return;

			[mRenderEncoder setStencilReferenceValue:value];
			mCurrentStencilReference = value;
		}

		bool MetalGpuCommandBuffer::PrepareDraw(MetalBuffer* indexBuffer)
		{
			BindVertexBuffers();

			if (indexBuffer != nullptr)
				mResourceTracker.TrackBufferUsage(indexBuffer, GpuAccessFlag::Read);

			if(!TrackShaderResources(false))
				return false;

			if (mRecordingFailed)
				return false;

			BindParameterSets(true);
			BindDynamicUniformBuffers(true);

			bool skipDraw = false;
			MetalVertexInput* vertexInput = ResolveVertexInputForDraw(skipDraw);
			if (skipDraw)
				return false;

			MetalGpuGraphicsPipelineState* pipeline = mBoundGraphicsPipeline.get();
			if (pipeline == nullptr)
				return false;

			// The render pass supplies the attachment formats and sample count, only topology and vertex input vary per draw
			MetalPipelineVariantKey key = mRenderPassPipelineKey;
			key.TopologyClass = (u16)MetalUtility::GetPrimitiveTopologyClass(mDrawOperation);
			key.VertexInputId = vertexInput ? vertexInput->GetId() : 0;

			id<MTLRenderPipelineState> metalPipeline = pipeline->GetOrCreateMetalPipeline(key, vertexInput);
			if (metalPipeline == nil)
				return false;

			SetRenderPipelineState(metalPipeline);

			id<MTLDepthStencilState> depthStencil = pipeline->GetMetalDepthStencilState((key.ReadOnlyMask & RT_DEPTH) != 0, (key.ReadOnlyMask & RT_STENCIL) != 0);
			if (depthStencil)
				SetDepthStencilState(depthStencil);

			[mRenderEncoder setCullMode:(MTLCullMode)pipeline->GetCullMode()];
			[mRenderEncoder setFrontFacingWinding:(MTLWinding)pipeline->GetWinding()];
			[mRenderEncoder setTriangleFillMode:(MTLTriangleFillMode)pipeline->GetFillMode()];
			[mRenderEncoder setDepthBias:pipeline->GetDepthBias() slopeScale:pipeline->GetSlopeScaledDepthBias() clamp:pipeline->GetDepthBiasClamp()];

			BindPushConstants(true);
			SetEncoderStencilReference(mStencilReference);

#if B3D_BUILD_TYPE_DEVELOPMENT
			if(!ValidateDrawAccesses())
				return false;
#endif

			return true;
		}

		void MetalGpuCommandBuffer::NotifyDrawEncoded()
		{
			mEncoderHasWork = true;
		}

		void MetalGpuCommandBuffer::Draw(u32 vertexOffset, u32 vertexCount, u32 instanceCount, u32 firstInstance)
		{
			GpuCommandBuffer::Draw(vertexOffset, vertexCount, instanceCount, firstInstance);
			EnsureValidThread();
			if (mRenderEncoder == nil || vertexCount == 0)
				return;

			if (!PrepareDraw(nullptr))
				return;

			[mRenderEncoder drawPrimitives:MetalUtility::GetPrimitiveType(mDrawOperation)
				vertexStart:vertexOffset
				vertexCount:vertexCount
				instanceCount:std::max<u32>(1, instanceCount)
				baseInstance:firstInstance];

			NotifyDrawEncoded();
		}

		void MetalGpuCommandBuffer::DrawIndexed(u32 startIndex, u32 indexCount, u32 vertexOffset, u32 vertexCount, u32 instanceCount, u32 firstInstance)
		{
			GpuCommandBuffer::DrawIndexed(startIndex, indexCount, vertexOffset, vertexCount, instanceCount, firstInstance);
			EnsureValidThread();
			(void)vertexCount;

			if (mRenderEncoder == nil || !mBoundIndexBuffer || indexCount == 0)
				return;

			auto* metalIndexBuffer = static_cast<MetalGpuBuffer*>(mBoundIndexBuffer.get());
			id<MTLBuffer> indexBuffer = metalIndexBuffer->GetMetalBuffer();
			if (indexBuffer == nil)
				return;

			if (!PrepareDraw(metalIndexBuffer->GetMetalResource()))
				return;

			const IndexType engineIndexType = mBoundIndexBuffer->GetInformation().Index.Type;
			const MTLIndexType indexType = (engineIndexType == IT_32BIT) ? MTLIndexTypeUInt32 : MTLIndexTypeUInt16;
			const u32 indexSize = (engineIndexType == IT_32BIT) ? 4u : 2u;
			const u64 indexBufferOffset = (u64)startIndex * indexSize;
			if (!B3D_ENSURE(indexBufferOffset + (u64)indexCount * indexSize <= (u64)indexBuffer.length))
				return;

			[mRenderEncoder drawIndexedPrimitives:MetalUtility::GetPrimitiveType(mDrawOperation)
				indexCount:indexCount
				indexType:indexType
				indexBuffer:indexBuffer.gpuAddress + indexBufferOffset
				indexBufferLength:(NSUInteger)(indexBuffer.length - indexBufferOffset)
				instanceCount:std::max<u32>(1, instanceCount)
				baseVertex:vertexOffset
				baseInstance:firstInstance];

			NotifyDrawEncoded();
		}

		void MetalGpuCommandBuffer::DispatchCompute(u32 groupCountX, u32 groupCountY, u32 groupCountZ)
		{
			GpuCommandBuffer::DispatchCompute(groupCountX, groupCountY, groupCountZ);
			EnsureValidThread();
			if (groupCountX == 0 || groupCountY == 0 || groupCountZ == 0)
			{
				B3D_LOG(Warning, LogRenderBackend, "Ignoring call to DispatchCompute(). Threadgroup count is zero.");
				return;
			}

			auto metalPipelineState = std::static_pointer_cast<MetalGpuComputePipelineState>(mBoundComputePipeline);
			if (!metalPipelineState)
				return;

			id<MTLComputePipelineState> metalPipeline = metalPipelineState->GetMetalPipeline();
			if (metalPipeline == nil)
				return;

			const u32* workgroupSize = metalPipelineState->GetWorkgroupSize();
			const u64 threadCountPerGroup = (u64)workgroupSize[0] * workgroupSize[1] * workgroupSize[2];
			if (threadCountPerGroup == 0 || threadCountPerGroup > (u64)metalPipeline.maxTotalThreadsPerThreadgroup)
			{
				B3D_LOG(Error, LogRenderBackend, "Ignoring call to DispatchCompute(). Pipeline workgroup size ({0}, {1}, {2}) exceeds Metal's {3}-thread limit.",
					workgroupSize[0], workgroupSize[1], workgroupSize[2], (u32)metalPipeline.maxTotalThreadsPerThreadgroup);
				return;
			}

			if (OpenComputeEncoder() == nil)
				return;

			if(!TrackShaderResources(true))
				return;

			if (mRecordingFailed)
				return;

			if (metalPipeline != mCurrentComputePipeline)
			{
				[mComputeEncoder setComputePipelineState:metalPipeline];
				mCurrentComputePipeline = metalPipeline;
				RetainUntilComplete(metalPipeline);
			}

			BindParameterSets(false);
			BindDynamicUniformBuffers(false);
			BindPushConstants(false);

			MTLSize threadsPerGroup = MTLSizeMake(workgroupSize[0], workgroupSize[1], workgroupSize[2]);
			MTLSize groups = MTLSizeMake(groupCountX, groupCountY, groupCountZ);
			[mComputeEncoder dispatchThreadgroups:groups threadsPerThreadgroup:threadsPerGroup];
			mEncoderHasWork = true;
		}

		void MetalGpuCommandBuffer::BeginRenderPass(const RenderPassCreateInformation& createInformation)
		{
			GpuCommandBuffer::BeginRenderPass(createInformation);
			EnsureValidThread();
			CloseEncoder();

			mRenderPassPipelineKey = MetalPipelineVariantKey{};
			mAcquiredWindowSurface = nullptr;
			mRenderPassWidth = 0;
			mRenderPassHeight = 0;
			mBoundVertexBuffers.Clear();
			// The normalized viewport persists across passes and is converted to pixels once the encoder is open
			mHasScissor = false;
			mVisibilityMode = MTLVisibilityResultModeDisabled;
			mVisibilityOffset = 0;

			const TShared<RenderTarget>& target = createInformation.Target;
			if (!target || !BeginNativeCommandBuffer())
				return;

			MTL4RenderPassDescriptor* descriptor = [[MTL4RenderPassDescriptor alloc] init];

			// Every occlusion query pool owns slots of the device's visibility result buffer, so the pass can use any of them. Results accumulate,
			// as the buffer holds the results of other passes too, and ResetQueries() zeroes a pool's slots.
			descriptor.visibilityResultBuffer = mGpuDevice.GetVisibilityResultBuffer();
			descriptor.visibilityResultType = MTLVisibilityResultTypeAccumulate;

			const RenderTargetProperties& targetProperties = target->GetProperties();
			mRenderPassWidth = targetProperties.Width;
			mRenderPassHeight = targetProperties.Height;
			mRenderPassPipelineKey.SampleCount = (u16)std::max(1u, targetProperties.MultisampleCount);
			mRenderPassClearValues = target->GetClearValues();

			RenderTargetClearValues renderPassStartClearValues;
			const RenderSurfaceMask clearMask = GpuBackendUtility::GetRenderPassClears(createInformation, renderPassStartClearValues);
			const RenderSurfaceMask loadMask = createInformation.LoadMask;
			const RenderSurfaceMask readOnlyMask = createInformation.ReadOnlyMask;
			GpuRenderPassAttachmentUsageArray renderPassAttachmentUsages;

			id<CAMetalDrawable> drawable = nil;
			if (targetProperties.IsWindow)
			{
				auto* window = static_cast<RenderWindow*>(target.get());
				const TShared<IRenderWindowSurface>& windowSurface = window->GetRenderWindowSurface();
				auto* metalSurface = static_cast<IMetalRenderWindowSurface*>(windowSurface.get());
				if (metalSurface == nullptr)
				{
					B3D_LOG(Error, LogRenderBackend, "BeginRenderPass: render window has no Metal surface attached.");
					return;
				}

				// The swap chain goes invalid when the window is resized
				if (!metalSurface->IsSwapChainValid())
					window->RebuildSwapChain();

				id<MTLTexture> backBuffer = metalSurface->AcquireColorTexture();
				if (backBuffer == nil)
				{
					B3D_LOG(Error, LogRenderBackend, "BeginRenderPass: failed to acquire a back buffer color texture.");
					return;
				}

				mAcquiredWindowSurface = metalSurface;
				descriptor.colorAttachments[0].texture = backBuffer;
				drawable = metalSurface->GetCurrentDrawable();

				if (MetalImage* colorImage = metalSurface->GetCurrentColorImage())
					TrackSurfaceImage(*colorImage, GpuAccessFlag::Write);

				id<MTLTexture> depthStencilTexture = metalSurface->GetDepthStencilTexture();
				if (depthStencilTexture != nil)
				{
					if (MetalImage* depthStencilImage = metalSurface->GetDepthStencilImage())
						TrackSurfaceImage(*depthStencilImage, GpuAccessFlag::Write);

					const MTLPixelFormat depthStencilFormat = [depthStencilTexture pixelFormat];
					if (MetalUtility::PixelFormatHasDepth(depthStencilFormat))
						descriptor.depthAttachment.texture = depthStencilTexture;

					if (MetalUtility::PixelFormatHasStencil(depthStencilFormat))
						descriptor.stencilAttachment.texture = depthStencilTexture;
				}

				// Window surfaces are not tracked resources, so the pass waits for all earlier work on them, including
				// rendering and readback of earlier frames
				EmitBarrier(MetalStageBarrier(GpuBarrierScope(GpuStageFlag::ColorAttachment | GpuStageFlag::LateFragmentTests | GpuStageFlag::Transfer, GpuAccessFlag::Read | GpuAccessFlag::Write,
					GpuStageFlag::ColorAttachment | GpuStageFlag::EarlyFragmentTests, GpuAccessFlag::Read | GpuAccessFlag::Write)));
			}
			else
			{
				MetalFramebuffer* framebuffer = static_cast<MetalRenderTexture*>(target.get())->GetFramebuffer();
				if (!B3D_ENSURE(framebuffer != nullptr))
					return;

				framebuffer->ApplyAttachments(descriptor);
				renderPassAttachmentUsages = framebuffer->BuildRenderPassAttachmentUsages(readOnlyMask);
			}

			ConfigureAttachmentActions(descriptor, loadMask, clearMask, createInformation.StoreMask | readOnlyMask, renderPassStartClearValues);
			PackAttachmentFormats(descriptor, mRenderPassPipelineKey);

#if B3D_BUILD_TYPE_DEVELOPMENT
			mDrawAccessValidator.BeginRenderPass();
#endif
			mGraphicsResourcesRequireTracking = true;
			mResourceTracker.BeginRenderPass(renderPassAttachmentUsages);
			mRenderPassTrackingActive = true;

			// Pipelines bound in this pass mask off writes to the read-only attachments
			RenderSurfaceMask attachmentReadOnlyMask = RT_NONE;
			for (const GpuRenderPassAttachmentUsage& attachment : renderPassAttachmentUsages)
			{
				if (attachment.Access == GpuAccessFlag::Read)
					attachmentReadOnlyMask.Set(attachment.Surface);
			}

			mRenderPassPipelineKey.ReadOnlyMask = (u32)attachmentReadOnlyMask;

			if (mRecordingFailed)
				return;

			// Otherwise the acquired drawable stays held until the next present and starves the drawable pool
			if (!OpenRenderEncoder(descriptor))
			{
				if (mAcquiredWindowSurface != nullptr)
				{
					B3D_LOG(Error, LogRenderBackend, "BeginRenderPass: failed to create the render encoder after acquiring a drawable; aborting drawable.");

					mAcquiredWindowSurface->AbortCurrentDrawable();
					mAcquiredWindowSurface = nullptr;
				}

				return;
			}

#if B3D_BUILD_TYPE_DEVELOPMENT
			mRenderEncoder.label = @"Render pass";
#endif

			if (drawable != nil && std::find(mDrawables.begin(), mDrawables.end(), drawable) == mDrawables.end())
				mDrawables.push_back(drawable);

			UseDrawableResidency(drawable);

			// Lets SwapBuffers() tell a rendered drawable apart from one that was acquired but never rendered to
			if (mAcquiredWindowSurface != nullptr)
				mAcquiredWindowSurface->MarkDrawableAsRendered();

			ApplyViewportToRenderEncoder();
		}

		void MetalGpuCommandBuffer::EndRenderPass()
		{
			EnsureValidThread();

			// Only the render encoder should be open here. A stray compute encoder is not silently ended.
			if (mRenderEncoder != nil)
				CloseEncoder();

			mAcquiredWindowSurface = nullptr;
			mRenderPassPipelineKey = MetalPipelineVariantKey{};
			mRenderPassWidth = 0;
			mRenderPassHeight = 0;
			mBoundVertexBuffers.Clear();
			// The normalized viewport persists across passes, see SetViewport()
			mHasScissor = false;
			mVisibilityMode = MTLVisibilityResultModeDisabled;
			mVisibilityOffset = 0;

			if(mRenderPassTrackingActive)
			{
				mResourceTracker.EndRenderPass();
				mRenderPassTrackingActive = false;
			}
		}

		void MetalGpuCommandBuffer::SetViewport(const Area2& area)
		{
			EnsureValidThread();

			// Metal viewports are in pixels, so the normalized area is kept and converted against each render pass's size
			mNormalizedViewport = area;
			mHasViewport = true;

			ApplyViewportToRenderEncoder();
		}

		void MetalGpuCommandBuffer::ApplyViewportToRenderEncoder()
		{
			if (mRenderEncoder == nil || !mHasViewport || mRenderPassWidth == 0 || mRenderPassHeight == 0)
				return;

			MTLViewport viewport;
			viewport.originX = (double)mNormalizedViewport.X * mRenderPassWidth;
			viewport.originY = (double)mNormalizedViewport.Y * mRenderPassHeight;
			viewport.width = (double)mNormalizedViewport.Width * mRenderPassWidth;
			viewport.height = (double)mNormalizedViewport.Height * mRenderPassHeight;
			viewport.znear = 0.0;
			viewport.zfar = 1.0;
			mViewport = viewport;
			[mRenderEncoder setViewport:viewport];
		}

		MTLViewport MetalGpuCommandBuffer::GetRenderPassViewport() const
		{
			MTLViewport viewport = {};
			viewport.width = (double)mRenderPassWidth;
			viewport.height = (double)mRenderPassHeight;
			viewport.zfar = 1.0;

			return viewport;
		}

		void MetalGpuCommandBuffer::ClearRenderTarget(RenderSurfaceMask mask)
		{
			EnsureValidThread();
			if (!B3D_ENSURE(IsInRenderPass()))
				return;

			EncodeClear(mask, GetRenderPassViewport());
		}

		void MetalGpuCommandBuffer::ClearViewport(RenderSurfaceMask mask)
		{
			EnsureValidThread();
			if (!B3D_ENSURE(IsInRenderPass()))
				return;

			EncodeClear(mask, mHasViewport ? mViewport : GetRenderPassViewport());
		}

		void MetalGpuCommandBuffer::EncodeClear(RenderSurfaceMask mask, const MTLViewport& area)
		{
			if (mask == RT_NONE)
				return;

			// Stencil is written by the depth-stencil state's replace operation using the reference value.
			const bool clearsDepth = mask.IsSet(RT_DEPTH) && mRenderPassPipelineKey.DepthFormat != 0;
			const bool clearsStencil = mask.IsSet(RT_STENCIL) && mRenderPassPipelineKey.StencilFormat != 0;

			MetalClearPipeline::Key key;
			std::memcpy(key.ColorFormats, mRenderPassPipelineKey.ColorFormats, sizeof(key.ColorFormats));
			key.DepthFormat = mRenderPassPipelineKey.DepthFormat;
			key.StencilFormat = mRenderPassPipelineKey.StencilFormat;
			key.SampleCount = mRenderPassPipelineKey.SampleCount;
			key.WritesDepth = clearsDepth;

			MetalClearPipeline::Parameters parameters;
			parameters.Depth = mRenderPassClearValues.Depth;

			for (u32 attachmentIndex = 0; attachmentIndex < B3D_MAXIMUM_RENDER_TARGET_COUNT; attachmentIndex++)
			{
				const RenderSurfaceMaskBits bit = (RenderSurfaceMaskBits)(RT_COLOR0 << attachmentIndex);
				if (!mask.IsSet(bit) || mRenderPassPipelineKey.ColorFormats[attachmentIndex] == 0)
					continue;

				const Color& color = mRenderPassClearValues.Colors[attachmentIndex];
				parameters.Color[attachmentIndex][0] = color.R;
				parameters.Color[attachmentIndex][1] = color.G;
				parameters.Color[attachmentIndex][2] = color.B;
				parameters.Color[attachmentIndex][3] = color.A;

				key.ColorWriteMask |= (u8)(1u << attachmentIndex);
			}

			if (key.ColorWriteMask == 0 && !clearsDepth && !clearsStencil)
				return;

			MetalClearPipeline& clearPipeline = mGpuDevice.GetClearPipeline();
			id<MTLRenderPipelineState> pipelineState = clearPipeline.GetOrCreatePipelineState(key);
			id<MTLDepthStencilState> depthStencilState = clearPipeline.GetOrCreateDepthStencilState(clearsDepth, clearsStencil);
			if (pipelineState == nil || depthStencilState == nil)
				return;

			const MTLGPUAddress parametersAddress = Upload(&parameters, sizeof(parameters));
			if (parametersAddress == 0)
				return;

			// The clear area alone defines what is cleared, so a scissor left over from earlier draws must not narrow it
			const MTLScissorRect previousScissor = mScissor;
			const bool hadScissor = mHasScissor;

			MTLScissorRect clearScissor;
			clearScissor.x = (NSUInteger)area.originX;
			clearScissor.y = (NSUInteger)area.originY;
			clearScissor.width = (NSUInteger)area.width;
			clearScissor.height = (NSUInteger)area.height;

			// The triangle covers the viewport, which is set to the clear area. The clear must not count towards an occlusion query.
			[mRenderEncoder setViewport:area];
			[mRenderEncoder setScissorRect:clearScissor];
			if (mVisibilityMode != MTLVisibilityResultModeDisabled)
				[mRenderEncoder setVisibilityResultMode:MTLVisibilityResultModeDisabled offset:0];

			SetRenderPipelineState(pipelineState);
			SetDepthStencilState(depthStencilState);
			[mRenderEncoder setCullMode:MTLCullModeNone];
			[mRenderEncoder setTriangleFillMode:MTLTriangleFillModeFill];
			[mRenderEncoder setDepthBias:0.0f slopeScale:0.0f clamp:0.0f];
			SetEncoderStencilReference(mRenderPassClearValues.Stencil);
			mFragmentArgumentTable.SetAddress(parametersAddress, kMetalClearParametersBufferSlot);
			[mRenderEncoder drawPrimitives:MTLPrimitiveTypeTriangle vertexStart:0 vertexCount:3];
			mEncoderHasWork = true;

			// The clear parameters share the push constant slot. The next draw rebinds all other state the clear changed, only the
			// viewport, scissor and occlusion query need restoring.
			mGraphicsPushConstantsRequireBind = true;
			[mRenderEncoder setViewport:mHasViewport ? mViewport : GetRenderPassViewport()];

			if (mVisibilityMode != MTLVisibilityResultModeDisabled)
				[mRenderEncoder setVisibilityResultMode:mVisibilityMode offset:mVisibilityOffset];

			if (hadScissor)
				[mRenderEncoder setScissorRect:previousScissor];
			else
			{
				DisableScissorTest();
				mHasScissor = false;
			}
		}

		void MetalGpuCommandBuffer::EnableScissorTest(u32 left, u32 top, u32 right, u32 bottom)
		{
			EnsureValidThread();
			if (mRenderEncoder == nil)
				return;

			MTLScissorRect rect;
			rect.x = left;
			rect.y = top;
			rect.width = (right > left) ? (right - left) : 0;
			rect.height = (bottom > top) ? (bottom - top) : 0;
			mScissor = rect;
			mHasScissor = true;
			[mRenderEncoder setScissorRect:rect];
		}

		void MetalGpuCommandBuffer::DisableScissorTest()
		{
			EnsureValidThread();
			if (mRenderEncoder == nil)
				return;

			// Metal cannot disable the scissor test, so cover the whole render pass. A larger rectangle fails validation.
			MTLScissorRect rect;
			rect.x = 0;
			rect.y = 0;
			rect.width = mRenderPassWidth;
			rect.height = mRenderPassHeight;
			mScissor = rect;
			mHasScissor = true;
			[mRenderEncoder setScissorRect:rect];
		}

		void MetalGpuCommandBuffer::SetStencilReferenceValue(u32 value)
		{
			EnsureValidThread();
			mStencilReference = value;
			if (mRenderEncoder)
				SetEncoderStencilReference(value);
		}

		void MetalGpuCommandBuffer::CopyBufferToBuffer(const TShared<GpuBuffer>& source, const TShared<GpuBuffer>& destination, u32 sourceOffset, u32 destinationOffset, u32 length)
		{
			GpuCommandBuffer::CopyBufferToBuffer(source, destination, sourceOffset, destinationOffset, length);
			EnsureValidThread();

			if (!source || !destination || length == 0)
				return;

			if ((sourceOffset % kMetalBufferCopyAlignment) != 0 || (destinationOffset % kMetalBufferCopyAlignment) != 0 || (length % kMetalBufferCopyAlignment) != 0)
			{
				B3D_LOG(Error, LogRenderBackend, "MetalGpuCommandBuffer::CopyBufferToBuffer requires 4-byte-aligned offsets and length on macOS.");
				return;
			}

			if (!IsBufferRangeValid(*source, sourceOffset, length) || !IsBufferRangeValid(*destination, destinationOffset, length))
			{
				B3D_LOG(Error, LogRenderBackend, "MetalGpuCommandBuffer::CopyBufferToBuffer range exceeds a buffer's size.");
				return;
			}

			// Validated before switching encoders, so a failed copy does not split the current pass
			auto metalSource = std::static_pointer_cast<MetalGpuBuffer>(source);
			auto metalDestination = std::static_pointer_cast<MetalGpuBuffer>(destination);
			id<MTLBuffer> sourceBuffer = metalSource ? metalSource->GetMetalBuffer() : nil;
			id<MTLBuffer> destinationBuffer = metalDestination ? metalDestination->GetMetalBuffer() : nil;
			if (sourceBuffer == nil || destinationBuffer == nil)
			{
				B3D_LOG(Error, LogRenderBackend, "MetalGpuCommandBuffer::CopyBufferToBuffer: source or destination MTLBuffer is nil (deferred init not complete).");
				return;
			}

			if (sourceBuffer == destinationBuffer)
			{
				const u64 sourceEnd = (u64)sourceOffset + length;
				const u64 destinationEnd = (u64)destinationOffset + length;
				if ((u64)sourceOffset < destinationEnd && (u64)destinationOffset < sourceEnd)
				{
					B3D_LOG(Error, LogRenderBackend, "MetalGpuCommandBuffer::CopyBufferToBuffer does not support overlapping ranges in one buffer.");
					return;
				}
			}

			id<MTL4ComputeCommandEncoder> encoder = OpenComputeEncoder();
			if (encoder == nil)
				return;

			MetalBuffer* sourceResource = metalSource->GetMetalResource();
			MetalBuffer* destinationResource = metalDestination->GetMetalResource();
			if (sourceResource != nullptr)
				mResourceTracker.TrackBufferUsage(sourceResource, GpuAccessFlag::Read, sourceOffset);

			if (destinationResource != nullptr)
				mResourceTracker.TrackBufferUsage(destinationResource, GpuAccessFlag::Write, destinationOffset);

			if (mRecordingFailed)
				return;

			[encoder copyFromBuffer:sourceBuffer
				sourceOffset:sourceOffset
				toBuffer:destinationBuffer
				destinationOffset:destinationOffset
				size:length];

			mEncoderHasWork = true;
		}

		void MetalGpuCommandBuffer::CopyBufferToTexture(const TShared<GpuBuffer>& source, const TShared<Texture>& destination, u32 bufferOffset, u32 mipLevel, u32 arrayLayer)
		{
			GpuCommandBuffer::CopyBufferToTexture(source, destination, bufferOffset, mipLevel, arrayLayer);
			EnsureValidThread();

			if (!source || !destination)
				return;

			MetalTextureTransferInformation transferInformation;
			if (!GetTextureTransferInformation(*destination, *source, bufferOffset, mipLevel, arrayLayer, "MetalGpuCommandBuffer::CopyBufferToTexture", transferInformation))
				return;

			auto metalSource = std::static_pointer_cast<MetalGpuBuffer>(source);
			auto metalDestination = std::static_pointer_cast<MetalTexture>(destination);
			id<MTLBuffer> sourceBuffer = metalSource ? metalSource->GetMetalBuffer() : nil;
			id<MTLTexture> destinationTexture = metalDestination ? metalDestination->GetMetalTexture() : nil;
			if (sourceBuffer == nil || destinationTexture == nil)
			{
				B3D_LOG(Error, LogRenderBackend, "MetalGpuCommandBuffer::CopyBufferToTexture: source buffer or destination texture is nil (deferred init not complete).");
				return;
			}

			id<MTL4ComputeCommandEncoder> encoder = OpenComputeEncoder();
			if (encoder == nil)
				return;

			MetalBuffer* sourceResource = metalSource->GetMetalResource();
			MetalImage* destinationResource = metalDestination->GetMetalResource();
			GpuTextureSubresourceRange destinationRange = destinationResource != nullptr ? destinationResource->GetRange() : GpuTextureSubresourceRange{};
			destinationRange.BaseArrayLayer = arrayLayer;
			destinationRange.ArrayLayerCount = 1;
			destinationRange.BaseMipLevel = mipLevel;
			destinationRange.MipLevelCount = 1;
			if (destination->GetProperties().Format == PF_D32_S8X24)
				destinationRange.AspectMask = GpuTextureAspectFlag::Depth;
			if (sourceResource != nullptr)
				mResourceTracker.TrackBufferUsage(sourceResource, GpuAccessFlag::Read, bufferOffset);

			if (destinationResource != nullptr)
				mResourceTracker.TrackImageUsage(destinationResource, destinationRange, GpuAccessFlag::Write);

			if (mRecordingFailed)
				return;

			[encoder copyFromBuffer:sourceBuffer
				sourceOffset:bufferOffset
				sourceBytesPerRow:transferInformation.RowPitch
				sourceBytesPerImage:(transferInformation.Depth > 1 ? transferInformation.SlicePitch : 0)
				sourceSize:MTLSizeMake(transferInformation.Width, transferInformation.Height, transferInformation.Depth)
				toTexture:destinationTexture
				destinationSlice:arrayLayer
				destinationLevel:mipLevel
				destinationOrigin:MTLOriginMake(0, 0, 0)
				options:transferInformation.Options];

			mEncoderHasWork = true;
		}

		void MetalGpuCommandBuffer::CopyTextureToBuffer(const TShared<Texture>& source, const TShared<GpuBuffer>& destination, u32 mipLevel, u32 arrayLayer, u32 bufferOffset)
		{
			GpuCommandBuffer::CopyTextureToBuffer(source, destination, mipLevel, arrayLayer, bufferOffset);
			EnsureValidThread();
			if (!source || !destination)
				return;

			MetalTextureTransferInformation transferInformation;
			if (!GetTextureTransferInformation(*source, *destination, bufferOffset, mipLevel, arrayLayer, "MetalGpuCommandBuffer::CopyTextureToBuffer", transferInformation))
				return;

			auto metalSource = std::static_pointer_cast<MetalTexture>(source);
			auto metalDestination = std::static_pointer_cast<MetalGpuBuffer>(destination);
			id<MTLTexture> sourceTexture = metalSource ? metalSource->GetMetalTexture() : nil;
			id<MTLBuffer> destinationBuffer = metalDestination ? metalDestination->GetMetalBuffer() : nil;
			if (sourceTexture == nil || destinationBuffer == nil)
			{
				B3D_LOG(Error, LogRenderBackend, "MetalGpuCommandBuffer::CopyTextureToBuffer: source texture or destination buffer is nil (deferred init not complete).");
				return;
			}

			id<MTL4ComputeCommandEncoder> encoder = OpenComputeEncoder();
			if (encoder == nil)
				return;

			MetalImage* sourceResource = metalSource->GetMetalResource();
			MetalBuffer* destinationResource = metalDestination->GetMetalResource();
			GpuTextureSubresourceRange sourceRange = sourceResource != nullptr ? sourceResource->GetRange() : GpuTextureSubresourceRange{};
			sourceRange.BaseArrayLayer = arrayLayer;
			sourceRange.ArrayLayerCount = 1;
			sourceRange.BaseMipLevel = mipLevel;
			sourceRange.MipLevelCount = 1;
			if (source->GetProperties().Format == PF_D32_S8X24)
				sourceRange.AspectMask = GpuTextureAspectFlag::Depth;

			if (sourceResource != nullptr)
				mResourceTracker.TrackImageUsage(sourceResource, sourceRange, GpuAccessFlag::Read);

			if (destinationResource != nullptr)
				mResourceTracker.TrackBufferUsage(destinationResource, GpuAccessFlag::Write, bufferOffset);

			if (mRecordingFailed)
				return;

			[encoder copyFromTexture:sourceTexture
				sourceSlice:arrayLayer
				sourceLevel:mipLevel
				sourceOrigin:MTLOriginMake(0, 0, 0)
				sourceSize:MTLSizeMake(transferInformation.Width, transferInformation.Height, transferInformation.Depth)
				toBuffer:destinationBuffer
				destinationOffset:bufferOffset
				destinationBytesPerRow:transferInformation.RowPitch
				destinationBytesPerImage:(transferInformation.Depth > 1 ? transferInformation.SlicePitch : 0)
				options:transferInformation.Options];

			mEncoderHasWork = true;
		}

		void MetalGpuCommandBuffer::CopyWindowToBuffer(const TShared<RenderWindow>& source, const TShared<GpuBuffer>& destination)
		{
			GpuCommandBuffer::CopyWindowToBuffer(source, destination);
			EnsureValidThread();
			if (!source || !destination)
				return;

			auto* const surface = static_cast<IMetalRenderWindowSurface*>(source->GetRenderWindowSurface().get());
			if (surface == nullptr)
				return;

			const TOptional<RenderWindowImageLayout> layout = surface->GetImageLayout();
			id<MTLTexture> sourceTexture = surface->GetCurrentColorTexture();
			auto metalDestination = std::static_pointer_cast<MetalGpuBuffer>(destination);
			id<MTLBuffer> destinationBuffer = metalDestination->GetMetalBuffer();
			if (!layout || sourceTexture == nil || destinationBuffer == nil)
				return;

			id<MTL4ComputeCommandEncoder> encoder = OpenComputeEncoder();
			if (encoder == nil)
				return;

			if (MetalImage* colorImage = surface->GetCurrentColorImage())
				TrackSurfaceImage(*colorImage, GpuAccessFlag::Read);
			else
				RetainUntilComplete(sourceTexture);

			UseDrawableResidency(surface->GetCurrentDrawable());

			MetalBuffer* destinationResource = metalDestination->GetMetalResource();
			if (destinationResource != nullptr)
				mResourceTracker.TrackBufferUsage(destinationResource, GpuAccessFlag::Write, 0);

			if (mRecordingFailed)
				return;

			const u32 rowPitchBytes = layout->Pitch.RowPitch * PixelUtility::GetBlockSize(layout->Format);
			[encoder copyFromTexture:sourceTexture
				sourceSlice:0
				sourceLevel:0
				sourceOrigin:MTLOriginMake(0, 0, 0)
				sourceSize:MTLSizeMake(layout->Width, layout->Height, 1)
				toBuffer:destinationBuffer
				destinationOffset:0
				destinationBytesPerRow:rowPitchBytes
				destinationBytesPerImage:(NSUInteger)rowPitchBytes * layout->Pitch.SliceHeight];

			mEncoderHasWork = true;
		}

		bool MetalGpuCommandBuffer::CopyTexture(const TShared<Texture>& source, const TShared<Texture>& destination, const TextureCopyInformation& copyInformation)
		{
			if (!GpuCommandBuffer::CopyTexture(source, destination, copyInformation) || mRecordingFailed)
				return false;

			auto sourceTexture = std::static_pointer_cast<MetalTexture>(source);
			auto destinationTexture = std::static_pointer_cast<MetalTexture>(destination);
			id<MTLTexture> sourceHandle = sourceTexture ? sourceTexture->GetMetalTexture() : nil;
			id<MTLTexture> destinationHandle = destinationTexture ? destinationTexture->GetMetalTexture() : nil;
			if (sourceHandle == nil || destinationHandle == nil)
			{
				B3D_LOG(Error, LogRenderBackend, "MetalGpuCommandBuffer::CopyTexture: source or destination texture is nil (deferred init not complete).");
				return false;
			}

			if (sourceHandle.pixelFormat != destinationHandle.pixelFormat)
			{
				B3D_LOG(Error, LogRenderBackend, "MetalGpuCommandBuffer::CopyTexture requires identical native pixel formats (including sRGB state).");
				return false;
			}

			const TextureProperties& sourceProperties = source->GetProperties();
			const TextureProperties& destinationProperties = destination->GetProperties();
			const bool resolveMultisample = sourceProperties.SampleCount > 1 && destinationProperties.SampleCount == 1;
			u32 sourceWidth, sourceHeight, sourceDepth;
			PixelUtility::GetSizeForMipLevel(sourceProperties.Width, sourceProperties.Height, sourceProperties.Depth, copyInformation.SourceMip, sourceWidth, sourceHeight, sourceDepth);

			const bool copyEntireSurface = copyInformation.SourceVolume.GetWidth() == 0 || copyInformation.SourceVolume.GetHeight() == 0 || copyInformation.SourceVolume.GetDepth() == 0;
			const MTLOrigin sourceOrigin = copyEntireSurface ? MTLOriginMake(0, 0, 0) : MTLOriginMake(copyInformation.SourceVolume.Left, copyInformation.SourceVolume.Top, copyInformation.SourceVolume.Front);
			const MTLSize copySize = copyEntireSurface ? MTLSizeMake(sourceWidth, sourceHeight, sourceDepth) : MTLSizeMake(copyInformation.SourceVolume.GetWidth(), copyInformation.SourceVolume.GetHeight(), copyInformation.SourceVolume.GetDepth());
			const MTLOrigin destinationOrigin = MTLOriginMake(copyInformation.DestinationPosition.X, copyInformation.DestinationPosition.Y, copyInformation.DestinationPosition.Z);

			if (sourceHandle == destinationHandle && copyInformation.SourceMip == copyInformation.DestinationMip)
			{
				const u32 sourceFaceEnd = copyInformation.SourceFace + copyInformation.FaceCount;
				const u32 destinationFaceEnd = copyInformation.DestinationFace + copyInformation.FaceCount;
				if (copyInformation.SourceFace < destinationFaceEnd && copyInformation.DestinationFace < sourceFaceEnd)
				{
					B3D_LOG(Error, LogRenderBackend, "MetalGpuCommandBuffer::CopyTexture does not support overlapping copies within one texture subresource.");
					return false;
				}
			}

			if (PixelUtility::IsCompressed(sourceProperties.Format))
			{
				const Vector2I blockDimensions = PixelUtility::GetBlockDimensions(sourceProperties.Format);
				const bool alignedOrigins = (sourceOrigin.x % blockDimensions.X) == 0 && (sourceOrigin.y % blockDimensions.Y) == 0 && (destinationOrigin.x % blockDimensions.X) == 0 && (destinationOrigin.y % blockDimensions.Y) == 0;
				u32 destinationWidth, destinationHeight, destinationDepth;
				PixelUtility::GetSizeForMipLevel(destinationProperties.Width, destinationProperties.Height, destinationProperties.Depth, copyInformation.DestinationMip, destinationWidth, destinationHeight, destinationDepth);
				(void)destinationDepth;
				const bool alignedExtent = ((copySize.width % blockDimensions.X) == 0 || (sourceOrigin.x + copySize.width == sourceWidth && destinationOrigin.x + copySize.width == destinationWidth)) &&
					((copySize.height % blockDimensions.Y) == 0 || (sourceOrigin.y + copySize.height == sourceHeight && destinationOrigin.y + copySize.height == destinationHeight));
				if (!alignedOrigins || !alignedExtent)
				{
					B3D_LOG(Error, LogRenderBackend, "MetalGpuCommandBuffer::CopyTexture compressed regions must be aligned to format block boundaries.");
					return false;
				}
			}

			MetalImage* sourceResource = sourceTexture->GetMetalResource();
			MetalImage* destinationResource = destinationTexture->GetMetalResource();
			if (sourceResource == nullptr || destinationResource == nullptr)
				return false;

			GpuTextureSubresourceRange sourceRange = sourceResource->GetRange();
			sourceRange.BaseArrayLayer = copyInformation.SourceFace;
			sourceRange.ArrayLayerCount = copyInformation.FaceCount;
			sourceRange.BaseMipLevel = copyInformation.SourceMip;
			sourceRange.MipLevelCount = 1;
			GpuTextureSubresourceRange destinationRange = destinationResource->GetRange();
			destinationRange.BaseArrayLayer = copyInformation.DestinationFace;
			destinationRange.ArrayLayerCount = copyInformation.FaceCount;
			destinationRange.BaseMipLevel = copyInformation.DestinationMip;
			destinationRange.MipLevelCount = 1;

			if (resolveMultisample)
			{
				u32 destinationWidth, destinationHeight, destinationDepth;
				PixelUtility::GetSizeForMipLevel(destinationProperties.Width, destinationProperties.Height, destinationProperties.Depth, copyInformation.DestinationMip, destinationWidth, destinationHeight, destinationDepth);
				const bool fullSurfaceResolve = sourceOrigin.x == 0 && sourceOrigin.y == 0 && sourceOrigin.z == 0 &&
					destinationOrigin.x == 0 && destinationOrigin.y == 0 && destinationOrigin.z == 0 &&
					copySize.width == sourceWidth && copySize.height == sourceHeight && copySize.depth == sourceDepth &&
					sourceWidth == destinationWidth && sourceHeight == destinationHeight && sourceDepth == destinationDepth &&
					sourceProperties.Type != TEX_TYPE_3D;
				if (!fullSurfaceResolve)
				{
					B3D_LOG(Error, LogRenderBackend, "Metal supports multisample resolves only for complete, equally-sized texture subresources.");
					return false;
				}
			}

			// Metal resolves through a render pass store action, not a blit, so the barriers wait for the resolve encoders
			id<MTL4ComputeCommandEncoder> encoder = nil;
			if (resolveMultisample)
				CloseEncoder();
			else
			{
				encoder = OpenComputeEncoder();
				if (encoder == nil)
					return false;
			}

			mResourceTracker.TrackImageUsage(sourceResource, sourceRange, GpuAccessFlag::Read);
			mResourceTracker.TrackImageUsage(destinationResource, destinationRange, GpuAccessFlag::Write);

			if (mRecordingFailed)
				return false;

			if (resolveMultisample)
			{
				for (u32 faceOffset = 0; faceOffset < copyInformation.FaceCount; faceOffset++)
				{
					MTL4RenderPassDescriptor* resolveDescriptor = [[MTL4RenderPassDescriptor alloc] init];
					MTLRenderPassColorAttachmentDescriptor* attachment = resolveDescriptor.colorAttachments[0];
					attachment.texture = sourceHandle;
					attachment.level = copyInformation.SourceMip;
					attachment.slice = copyInformation.SourceFace + faceOffset;
					attachment.resolveTexture = destinationHandle;
					attachment.resolveLevel = copyInformation.DestinationMip;
					attachment.resolveSlice = copyInformation.DestinationFace + faceOffset;
					attachment.loadAction = MTLLoadActionLoad;
					attachment.storeAction = MTLStoreActionStoreAndMultisampleResolve;

					if (!OpenRenderEncoder(resolveDescriptor))
						return false;

					CloseEncoder();
				}

				return true;
			}

			u32 destinationWidth, destinationHeight, destinationDepth;
			PixelUtility::GetSizeForMipLevel(destinationProperties.Width, destinationProperties.Height, destinationProperties.Depth,
				copyInformation.DestinationMip, destinationWidth, destinationHeight, destinationDepth);
			const bool copiesFullSubresources = sourceOrigin.x == 0 && sourceOrigin.y == 0 && sourceOrigin.z == 0 &&
				destinationOrigin.x == 0 && destinationOrigin.y == 0 && destinationOrigin.z == 0 &&
				copySize.width == sourceWidth && copySize.height == sourceHeight && copySize.depth == sourceDepth &&
				sourceWidth == destinationWidth && sourceHeight == destinationHeight && sourceDepth == destinationDepth &&
				sourceProperties.Type != TEX_TYPE_3D;

			if (copiesFullSubresources)
			{
				[encoder copyFromTexture:sourceHandle
					sourceSlice:copyInformation.SourceFace
					sourceLevel:copyInformation.SourceMip
					toTexture:destinationHandle
					destinationSlice:copyInformation.DestinationFace
					destinationLevel:copyInformation.DestinationMip
					sliceCount:copyInformation.FaceCount
					levelCount:1];
			}
			else
			{
				for (u32 faceOffset = 0; faceOffset < copyInformation.FaceCount; faceOffset++)
				{
					[encoder copyFromTexture:sourceHandle
						sourceSlice:copyInformation.SourceFace + faceOffset
						sourceLevel:copyInformation.SourceMip
						sourceOrigin:sourceOrigin
						sourceSize:copySize
						toTexture:destinationHandle
						destinationSlice:copyInformation.DestinationFace + faceOffset
						destinationLevel:copyInformation.DestinationMip
						destinationOrigin:destinationOrigin];
				}
			}

			mEncoderHasWork = true;
			return true;
		}

		bool MetalGpuCommandBuffer::BlitTexture(const TShared<Texture>& source, const TShared<Texture>& destination, const TextureBlitInformation& blitInformation)
		{
			if (!GpuCommandBuffer::BlitTexture(source, destination, blitInformation))
				return false;

			const TextureProperties& sourceProperties = source->GetProperties();
			const TextureProperties& destinationProperties = destination->GetProperties();
			u32 sourceWidth, sourceHeight, sourceDepth;
			PixelUtility::GetSizeForMipLevel(sourceProperties.Width, sourceProperties.Height, sourceProperties.Depth, blitInformation.SourceMip, sourceWidth, sourceHeight, sourceDepth);

			u32 destinationWidth, destinationHeight, destinationDepth;
			PixelUtility::GetSizeForMipLevel(destinationProperties.Width, destinationProperties.Height, destinationProperties.Depth, blitInformation.DestinationMip, destinationWidth, destinationHeight, destinationDepth);

			PixelVolume sourceVolume = blitInformation.SourceVolume;
			if (sourceVolume.GetWidth() == 0 || sourceVolume.GetHeight() == 0 || sourceVolume.GetDepth() == 0)
				sourceVolume = PixelVolume(0, 0, 0, sourceWidth, sourceHeight, sourceDepth);

			PixelVolume destinationVolume = blitInformation.DestinationVolume;
			if (destinationVolume.GetWidth() == 0 || destinationVolume.GetHeight() == 0 || destinationVolume.GetDepth() == 0)
				destinationVolume = PixelVolume(0, 0, 0, destinationWidth, destinationHeight, destinationDepth);

			const bool validVolumes = sourceVolume.Left < sourceVolume.Right && sourceVolume.Top < sourceVolume.Bottom && sourceVolume.Front < sourceVolume.Back &&
				sourceVolume.Right <= sourceWidth && sourceVolume.Bottom <= sourceHeight && sourceVolume.Back <= sourceDepth &&
				destinationVolume.Left < destinationVolume.Right && destinationVolume.Top < destinationVolume.Bottom && destinationVolume.Front < destinationVolume.Back &&
				destinationVolume.Right <= destinationWidth && destinationVolume.Bottom <= destinationHeight && destinationVolume.Back <= destinationDepth;
			if (!validVolumes)
			{
				B3D_LOG(Error, LogRenderBackend, "MetalGpuCommandBuffer::BlitTexture source or destination volume is outside its texture subresource.");
				return false;
			}

			const bool matchingExtents = sourceVolume.GetWidth() == destinationVolume.GetWidth() && sourceVolume.GetHeight() == destinationVolume.GetHeight() && sourceVolume.GetDepth() == destinationVolume.GetDepth();
			if (!matchingExtents || sourceProperties.Format != destinationProperties.Format || sourceProperties.Type != destinationProperties.Type)
			{
				B3D_LOG(Error, LogRenderBackend, "MetalGpuCommandBuffer::BlitTexture scaling and format conversion require the backend's internal shader blit path.");
				return false;
			}

			TextureCopyInformation copyInformation;
			copyInformation.SourceFace = blitInformation.SourceFace;
			copyInformation.SourceMip = blitInformation.SourceMip;
			copyInformation.SourceVolume = sourceVolume;
			copyInformation.DestinationFace = blitInformation.DestinationFace;
			copyInformation.DestinationMip = blitInformation.DestinationMip;
			copyInformation.FaceCount = blitInformation.FaceCount;
			copyInformation.DestinationPosition = Vector3I(destinationVolume.Left, destinationVolume.Top, destinationVolume.Front);
			return CopyTexture(source, destination, copyInformation);
		}

		void MetalGpuCommandBuffer::WriteTimestamp(GpuQueryId query, const TShared<GpuQueryPool>& queryPool)
		{
			EnsureValidThread();

			if (!queryPool || queryPool->GetQueryType() != GpuQueryType::Timestamp)
			{
				B3D_LOG(Error, LogRenderBackend, "WriteTimestamp requires a timestamp query pool.");
				return;
			}

			auto metalPool = std::static_pointer_cast<MetalGpuQueryPool>(queryPool);
			if (!metalPool->IsQueryAllocated(query))
			{
				B3D_LOG(Error, LogRenderBackend, "WriteTimestamp received an invalid or unallocated query ID.");
				return;
			}

			id<MTL4CounterHeap> counterHeap = metalPool->GetCounterHeap();
			if (counterHeap == nil)
				return;

			// Outside encoders the timestamp is written once all earlier work in the command buffer completes
			if (mRenderEncoder != nil)
				[mRenderEncoder writeTimestampWithGranularity:MTL4TimestampGranularityPrecise afterStage:MTLRenderStageFragment intoHeap:counterHeap atIndex:query.Id];
			else if (mComputeEncoder != nil)
				[mComputeEncoder writeTimestampWithGranularity:MTL4TimestampGranularityPrecise intoHeap:counterHeap atIndex:query.Id];
			else
			{
				if (!BeginNativeCommandBuffer())
					return;

				[mCommandBuffer writeTimestampIntoHeap:counterHeap atIndex:query.Id];
			}

			AddUniqueUsedQueryPool(metalPool);
		}

		void MetalGpuCommandBuffer::BeginQuery(GpuQueryId query, const TShared<GpuQueryPool>& queryPool, GpuQueryFlags flags)
		{
			EnsureValidThread();

			if (!queryPool || queryPool->GetQueryType() != GpuQueryType::Occlusion || mRenderEncoder == nil)
			{
				B3D_LOG(Error, LogRenderBackend, "BeginQuery requires an active render pass and an occlusion query pool.");
				return;
			}

			auto metalPool = std::static_pointer_cast<MetalGpuQueryPool>(queryPool);
			if (!metalPool->IsQueryAllocated(query))
			{
				B3D_LOG(Error, LogRenderBackend, "BeginQuery received an invalid or unallocated query ID.");
				return;
			}

			if (mVisibilityMode != MTLVisibilityResultModeDisabled)
			{
				B3D_LOG(Error, LogRenderBackend, "Metal does not support nested occlusion queries.");
				return;
			}

			const MTLVisibilityResultMode mode = flags.IsSet(GpuQueryFlag::PreciseOcclusion) ? MTLVisibilityResultModeCounting : MTLVisibilityResultModeBoolean;
			const NSUInteger offset = metalPool->GetQueryOffset(query);
			mVisibilityMode = mode;
			mVisibilityOffset = offset;
			[mRenderEncoder setVisibilityResultMode:mode offset:offset];

			AddUniqueUsedQueryPool(metalPool);
		}

		void MetalGpuCommandBuffer::EndQuery(GpuQueryId query, const TShared<GpuQueryPool>& queryPool)
		{
			EnsureValidThread();

			if (mRenderEncoder == nil || !queryPool || queryPool->GetQueryType() != GpuQueryType::Occlusion)
				return;

			auto metalPool = std::static_pointer_cast<MetalGpuQueryPool>(queryPool);
			if (!metalPool->IsQueryAllocated(query) || mVisibilityMode == MTLVisibilityResultModeDisabled || mVisibilityOffset != metalPool->GetQueryOffset(query))
			{
				B3D_LOG(Error, LogRenderBackend, "EndQuery does not match the active Metal occlusion query.");
				return;
			}

			mVisibilityMode = MTLVisibilityResultModeDisabled;
			mVisibilityOffset = 0;
			[mRenderEncoder setVisibilityResultMode:MTLVisibilityResultModeDisabled offset:0];
		}

		void MetalGpuCommandBuffer::ResetQueries(const TShared<GpuQueryPool>& queryPool)
		{
			EnsureValidThread();

			if (!queryPool)
				return;

			if (mRenderEncoder != nil)
			{
				B3D_LOG(Error, LogRenderBackend, "ResetQueries must be called outside a render pass.");
				return;
			}

			auto metalPool = std::static_pointer_cast<MetalGpuQueryPool>(queryPool);
			const NSRange visibilityResultRange = metalPool->GetVisibilityResultRange();
			if (visibilityResultRange.length > 0)
			{
				id<MTL4ComputeCommandEncoder> encoder = OpenComputeEncoder();
				if (encoder == nil)
				{
					B3D_LOG(Error, LogRenderBackend, "Failed to encode a Metal occlusion-query reset.");
					return;
				}

				// Visibility results accumulate, so render passes that write them wait for the reset, and the reset for the
				// render passes that wrote them earlier
				const GpuBarrierScope visibilityWriteToResetScope(GpuStageFlag::LateFragmentTests, GpuAccessFlag::Write, GpuStageFlag::Transfer, GpuAccessFlag::Write);
				EmitBarrier(MetalStageBarrier(visibilityWriteToResetScope));

				[encoder fillBuffer:mGpuDevice.GetVisibilityResultBuffer() range:visibilityResultRange value:0];
				mEncoderHasWork = true;

				const GpuBarrierScope resetToVisibilityWriteScope(GpuStageFlag::Transfer, GpuAccessFlag::Write, GpuStageFlag::EarlyFragmentTests | GpuStageFlag::LateFragmentTests, GpuAccessFlag::Write);
				EmitBarrier(MetalStageBarrier(resetToVisibilityWriteScope));

				AddUniqueUsedQueryPool(metalPool);
			}

			metalPool->ResetAllocation();
		}

		void MetalGpuCommandBuffer::BeginLabel(const StringView& name)
		{
#if B3D_BUILD_TYPE_DEVELOPMENT
			EnsureValidThread();
			if (name.empty() || !BeginNativeCommandBuffer())
				return;

			NSString* label = [[NSString alloc] initWithBytes:name.data() length:name.size() encoding:NSUTF8StringEncoding];
			[mCommandBuffer pushDebugGroup:label];
			mDebugGroupDepth++;
#else
			(void)name;
#endif
		}

		void MetalGpuCommandBuffer::EndLabel()
		{
#if B3D_BUILD_TYPE_DEVELOPMENT
			EnsureValidThread();
			if (!mIsNativeRecording || mDebugGroupDepth == 0)
				return;

			[mCommandBuffer popDebugGroup];
			mDebugGroupDepth--;
#endif
		}

		void MetalGpuCommandBuffer::InsertLabel(const StringView& name)
		{
#if B3D_BUILD_TYPE_DEVELOPMENT
			if (name.empty())
				return;

			// Metal only supports signposts on encoders
			id<MTL4CommandEncoder> encoder = mRenderEncoder != nil ? (id<MTL4CommandEncoder>)mRenderEncoder : (id<MTL4CommandEncoder>)mComputeEncoder;
			if (encoder == nil)
				return;

			NSString* label = [[NSString alloc] initWithBytes:name.data() length:name.size() encoding:NSUTF8StringEncoding];
			[encoder insertDebugSignpost:label];
#else
			(void)name;
#endif
		}

		void MetalGpuCommandBuffer::End()
		{
			// Barriers recorded last order the work of command buffers submitted later on the same queue, which a producer
			// barrier makes wait for this one
			if (!mPendingBarrier.IsEmpty() && !mRecordingFailed)
			{
				CloseEncoder();
				if (BeginNativeCommandBuffer())
				{
					id<MTL4ComputeCommandEncoder> encoder = [mCommandBuffer computeCommandEncoder];
					[encoder barrierAfterStages:mPendingBarrier.SourceStages beforeQueueStages:mPendingBarrier.DestinationStages visibilityOptions:mPendingBarrier.Visibility];
					[encoder endEncoding];
				}

				mPendingBarrier = MetalStageBarrier();
			}

			CloseEncoder();

			if (mIsNativeRecording)
			{
#if B3D_BUILD_TYPE_DEVELOPMENT
				for (; mDebugGroupDepth > 0; mDebugGroupDepth--)
					[mCommandBuffer popDebugGroup];
#endif

				[mCommandBuffer endCommandBuffer];
				mIsNativeRecording = false;
			}

			// Done is only set once the GPU finishes executing the buffer, see ExecuteSubmitOnSubmitThread()
			mState = GpuCommandBufferState::RecordingDone;
		}

		bool MetalGpuCommandBuffer::RecordBarriers(const GpuBarriers& barriers, GpuBarrierPhase phase, GpuSplitBarrier* split)
		{
			// What the barriers of each case record:
			//
			// Case            | Stage barrier
			// --------------- | -------------------------------------
			// Full            | source -> destination, at the barrier
			// SplitRelease    | -
			// SplitAcquire    | source -> destination, at the acquire
			// TransferRelease | -
			// TransferAcquire | -
			//
			// Metal 4 barriers wait on stages rather than on points in the command stream: a consumer barrier waits for the
			// stages of all work encoded before its encoder on the queue, including earlier submissions. The acquire of a
			// split barrier on one queue therefore records the whole barrier, so the work recorded between the halves is
			// never blocked by the release, and the release records nothing. A fence could not wait from the encoder the
			// release is in, nor cover work in the encoders before it.
			//
			// Halves on queues of different types are ordered by the queue wait of the acquire's submission on the release's
			// submission event, which also makes the writes visible, so neither half records a barrier.
			enum class BarrierCase { Full, SplitRelease, SplitAcquire, TransferRelease, TransferAcquire };

			if (mRecordingFailed)
				return false;

			const bool transfersQueue = split != nullptr && split->IsQueueTransfer();
			BarrierCase barrierCase = BarrierCase::Full;
			if (phase == GpuBarrierPhase::Release)
				barrierCase = transfersQueue ? BarrierCase::TransferRelease : BarrierCase::SplitRelease;
			else if (phase == GpuBarrierPhase::Acquire)
				barrierCase = transfersQueue ? BarrierCase::TransferAcquire : BarrierCase::SplitAcquire;

			const bool recordsBarrier = barrierCase == BarrierCase::Full || barrierCase == BarrierCase::SplitAcquire;

			MetalStageBarrier stageBarrier;
			const auto fnAddBarrier = [&](const GpuBarrier& barrier)
			{
				if (!recordsBarrier)
					return;

				stageBarrier.Merge(MetalStageBarrier(GpuBarrierScope(barrier.Source.Stages, barrier.Source.Access, barrier.Destination.Stages, barrier.Destination.Access), barrier.Flags));
			};

			for (const GpuBufferBarrier& barrier : barriers.BufferBarriers)
			{
				auto* const gpuBuffer = static_cast<MetalGpuBuffer*>(barrier.Object.get());
				MetalBuffer* const buffer = gpuBuffer != nullptr ? gpuBuffer->GetMetalResource() : nullptr;
				if (buffer == nullptr)
					continue;

				mResourceTracker.TrackBufferUsage(buffer, barrier.Destination.Access | GpuAccessFlag::Read);
				fnAddBarrier(barrier);
			}

			const auto fnAddImageBarrier = [&](MetalImage& image, const GpuTextureSubresourceRange& range, const GpuBarrier& barrier)
			{
				mResourceTracker.TrackImageUsage(&image, range, barrier.Destination.Access | GpuAccessFlag::Read);
				fnAddBarrier(barrier);
			};

			for (const GpuTextureBarrier& barrier : barriers.TextureBarriers)
			{
				// Metal has no layouts, and the alias acquire that supersedes the texture orders the work of the memory
				if (barrier.Flags.IsSet(GpuBarrierFlag::AliasRelease))
					continue;

				auto* const texture = static_cast<MetalTexture*>(barrier.Object.get());
				MetalImage* const image = texture != nullptr ? texture->GetMetalResource() : nullptr;
				if (image == nullptr)
					continue;

				fnAddImageBarrier(*image, GpuBackendUtility::ClampRange(barrier.SubresourceRange, image->GetRange()), barrier);
			}

			for (const GpuRenderTargetBarrier& barrier : barriers.RenderTargetBarriers)
			{
				RenderTarget* const renderTarget = barrier.Object.get();
				if (renderTarget == nullptr)
					continue;

				// Window back buffers are not tracked resources, and only need their work ordered
				if (renderTarget->GetProperties().IsWindow)
				{
					fnAddBarrier(barrier);
					continue;
				}

				MetalFramebuffer* const framebuffer = static_cast<MetalRenderTexture*>(renderTarget)->GetFramebuffer();
				const GpuFramebufferAttachment* const attachment = framebuffer != nullptr ? framebuffer->FindAttachment(barrier.SurfaceMask) : nullptr;
				if (attachment == nullptr)
					continue;

				// The barrier covers the surface's faces and mip levels, and the aspects it selects
				GpuTextureSubresourceRange range = attachment->Range;
				range.AspectMask &= barrier.SubresourceRange.AspectMask;

				fnAddImageBarrier(static_cast<MetalImage&>(*attachment->Image), range, barrier);
			}

			if (stageBarrier.IsEmpty())
				return true;

			EmitBarrier(stageBarrier);
			return true;
		}

		void MetalGpuCommandBuffer::ExecuteSubmitOnSubmitThread(MetalGpuQueue& submitQueue, GpuQueueMask syncMask, TArrayView<const GpuTimelineFenceAndValue> signalFences)
		{
			// The owner thread already released its recording state (NotifyWillQueueForSubmit()), so only the native encoding state and
			// mUsedQueryPools may be touched here. mQueueSyncMask is already folded into @p syncMask.
			AssertIfNotSubmitThread();

			mSubmittedQueueId = submitQueue.GetId();

			if (mRecordingFailed)
			{
				mQueueSyncMask = GpuQueueMask();
				for (const TShared<MetalGpuQueryPool>& pool : mUsedQueryPools)
				{
					if (mQueryPoolsQueuedForSubmission)
						pool->MarkSubmissionFailed();
					else
						pool->MarkRecordingAbandoned();
				}
				mUsedQueryPools.clear();
				mQueryPoolsQueuedForSubmission = false;

				TShared<GpuCommandBuffer> selfShared = GetShared();
				TShared<WaitGroup> ownerCompletion = B3DMakeShared<WaitGroup>(1);
				submitQueue.NotifySubmissionFailed(ownerCompletion);
				mPool.PostCommand([selfShared, ownerCompletion]()
				{
					auto* owner = static_cast<MetalGpuCommandBuffer*>(selfShared.get());
					owner->mState = GpuCommandBufferState::Done;
					owner->OnDidComplete();
					owner->ClearRecordingState();
					owner->mPool.NotifyCommandBufferReady(owner->mId);
					ownerCompletion->NotifyDone();
				}, "MetalGpuCommandBuffer failed submission");

				return;
			}

			GpuSubmitThread& submitThread = mGpuDevice.GetSubmitThread();

			mResourceTracker.NotifyUsed(mSubmittedQueueId);
			mResourcesSubmitted = true;

			// Queue waits apply to all work committed after them, so they precede the commit. Frame fence values are committed
			// event values, so waiting on them cannot deadlock, and queues in the sync mask are already waited on at a later value.
			submitQueue.WaitForQueues(syncMask);

			const TArrayView<const u64> frameFenceValues = submitThread.ConsumeFrameFence(submitQueue);
			if (!frameFenceValues.IsEmpty())
			{
				const GpuQueueId submitQueueId = submitQueue.GetId();
				mGpuDevice.DoForEachQueue([&submitQueue, submitQueueId, syncMask, frameFenceValues](GpuQueue& queue)
				{
					const GpuQueueId queueId = queue.GetId();
					const u64 waitValue = frameFenceValues[queueId.Id];
					if (waitValue == 0 || (queueId.Id != submitQueueId.Id && syncMask.IsSet(queueId)))
						return;

					submitQueue.WaitForEvent(static_cast<MetalGpuQueue&>(queue).GetSharedEvent(), waitValue);
				});
			}

			for (id<CAMetalDrawable> drawable : mDrawables)
				submitQueue.WaitForDrawable(drawable);

			id<MTL4CommandBuffer> commandBuffers[1];
			u32 commandBufferCount = 0;
			if (mHasNativeCommands)
				commandBuffers[commandBufferCount++] = mCommandBuffer;

			mGpuDevice.GetHeapAllocator().GetResidencySet().Commit([&submitQueue, &commandBuffers, commandBufferCount]()
			{
				submitQueue.Commit(commandBuffers, commandBufferCount);
			});

			for (const PendingEventSignal& signal : mPendingEventSignals)
				submitQueue.SignalEvent(signal.Event, signal.Value);

			// Keeps the command buffer alive until the GPU finishes, then reports completion on the owner thread
			TShared<GpuCommandBuffer> selfShared = GetShared();
			TShared<WaitGroup> ownerCompletion = B3DMakeShared<WaitGroup>(1);
			const u64 signalValue = submitQueue.SignalSubmission([selfShared, ownerCompletion]()
			{
				auto* metalSelf = static_cast<MetalGpuCommandBuffer*>(selfShared.get());
				metalSelf->mPool.PostCommand([selfShared, ownerCompletion]()
				{
					auto* owner = static_cast<MetalGpuCommandBuffer*>(selfShared.get());
					owner->mState = GpuCommandBufferState::Done;
					owner->mPool.NotifyCommandBufferReady(owner->mId);
					owner->OnDidComplete();
					// Listener closures may hold the last references to resources used by this submission
					owner->ClearRecordingState();
					ownerCompletion->NotifyDone();
				}, "MetalGpuCommandBuffer completion");
			}, ownerCompletion);

			// Signaled after the queue's own event
			for (const GpuTimelineFenceAndValue& entry : signalFences)
			{
				if (entry.Fence)
					submitQueue.SignalEvent(static_cast<MetalGpuTimelineFence*>(entry.Fence.get())->GetSharedEvent(), entry.Value);
			}

			mQueueSyncMask = GpuQueueMask();

			// After the commit, so a query resolve never waits on a value from an uncommitted submission
			for (const TShared<MetalGpuQueryPool>& pool : mUsedQueryPools)
			{
				pool->MarkSubmitted(submitQueue, signalValue);
				mSubmittedQueryPools.Add(pool);
			}
			mUsedQueryPools.clear();
			mQueryPoolsQueuedForSubmission = false;
		}

		void MetalGpuCommandBuffer::AddUniqueUsedQueryPool(const TShared<MetalGpuQueryPool>& pool)
		{
			// A command buffer rarely uses more than a few pools, so a linear scan beats a hash set
			for (const TShared<MetalGpuQueryPool>& existing : mUsedQueryPools)
			{
				if (existing.get() == pool.get())
					return;
			}

			pool->MarkRecorded();
			mUsedQueryPools.Add(pool);
		}

		void MetalGpuCommandBuffer::NotifyWillQueueForSubmit(GpuQueueId queueId, GpuQueueMask syncMask)
		{
			GpuCommandBuffer::NotifyWillQueueForSubmit(queueId, syncMask);

			for (const TShared<MetalGpuQueryPool>& pool : mUsedQueryPools)
				pool->MarkQueuedForSubmission();

			mQueryPoolsQueuedForSubmission = !mUsedQueryPools.Empty();

			// Clears everything the submit thread must not touch. mUsedQueryPools stays, ExecuteSubmitOnSubmitThread() needs it.
			mBoundGraphicsPipeline = nullptr;
			mBoundComputePipeline = nullptr;
			mBoundParameterSets.Clear();
			mDynamicOffsetOverridesPerSet.Clear();
			mGraphicsResourcesRequireTracking = true;
			mBoundIndexBuffer = nullptr;
			mBoundVertexDescription = nullptr;
		}

		void MetalGpuCommandBuffer::ClearRecordingState()
		{
			// Same as VulkanGpuCommandBuffer::ClearRecordingState(). Resources of a buffer that failed before submission
			// were never marked used, so they are only unbound.
			if (mResourcesSubmitted)
				mResourceTracker.NotifyDone(mSubmittedQueueId);
			else
				mResourceTracker.NotifyUnbound();

			ClearBarrierState(mResourcesSubmitted);

			mResourceTracker.Clear();
			mResourcesSubmitted = false;

			mQueueSyncMask = GpuQueueMask();

			// Listener closures may hold the last references to transient resources, which must be released before
			// their allocators are reclaimed
			OnDidComplete.Clear();
			OnDestroyed.Clear();

			// The GPU finished the native command buffers, or they were never committed, so their memory can be reused
			CloseEncoder();
			if (mIsNativeRecording)
			{
				[mCommandBuffer endCommandBuffer];
				mIsNativeRecording = false;
			}

			if (mHasNativeCommands)
			{
				[mCommandAllocator reset];
				mHasNativeCommands = false;
			}

			for (UploadChunk& chunk : mUploadChunks)
				chunk.Cursor = 0;

			mRecordingFailed = mCommandBuffer == nil || mCommandAllocator == nil;
			mUploadChunkIndex = 0;
			mRetainedObjects.clear();
			mDrawables.clear();
			mDrawableResidencySets.clear();
			mPendingBarrier = MetalStageBarrier();
			mCurrentRenderPipeline = nil;
			mCurrentDepthStencilState = nil;
			mCurrentComputePipeline = nil;

			// Usually already cleared by NotifyWillQueueForSubmit(), but a pool reset never goes through a submit
			mBoundGraphicsPipeline = nullptr;
			mBoundComputePipeline = nullptr;
			mBoundParameterSets.Clear();
			mDynamicOffsetOverridesPerSet.Clear();
			mGraphicsResourcesRequireTracking = true;
			mBoundIndexBuffer = nullptr;
			mBoundVertexDescription = nullptr;
			mDrawOperation = DOT_TRIANGLE_LIST;
			mStencilReference = 0;
			mPushConstants.Clear();
			mGraphicsPushConstantsRequireBind = false;
			mComputePushConstantsRequireBind = false;
			mRenderPassPipelineKey = MetalPipelineVariantKey{};
			mAcquiredWindowSurface = nullptr;
			mRenderPassTrackingActive = false;
			mRenderPassWidth = 0;
			mRenderPassHeight = 0;
			for (const TShared<MetalGpuQueryPool>& pool : mUsedQueryPools)
			{
				if (mQueryPoolsQueuedForSubmission)
					pool->MarkSubmissionFailed();
				else
					pool->MarkRecordingAbandoned();
			}
			mUsedQueryPools.clear();
			mSubmittedQueryPools.clear();
			mQueryPoolsQueuedForSubmission = false;
			mPendingEventSignals.clear();
			mBoundVertexBuffers.Clear();
			mHasViewport = false;
			mNormalizedViewport = Area2(0.0f, 0.0f, 1.0f, 1.0f);
			mHasScissor = false;
			mDebugGroupDepth = 0;
			mVisibilityMode = MTLVisibilityResultModeDisabled;
			mVisibilityOffset = 0;
		}

		void MetalGpuCommandBuffer::NotifyParentPoolReset()
		{
			// GpuCommandBufferPool::Reset() may only be called once none of its command buffers are executing. Command buffers
			// that were recorded but never submitted are discarded.
			if (!B3D_ENSURE(mState != GpuCommandBufferState::Executing))
				return;

			if (mState != GpuCommandBufferState::Ready)
				ClearRecordingState();

			mState = GpuCommandBufferState::Ready;
		}

		void MetalGpuCommandBuffer::Destroy()
		{
			if (IsDestroyed())
				return;

			// Also clears OnDestroyed, so the base destructor doesn't trigger it
			ClearRecordingState();

			GpuCommandBuffer::Destroy();
		}
	} // namespace render
} // namespace b3d
