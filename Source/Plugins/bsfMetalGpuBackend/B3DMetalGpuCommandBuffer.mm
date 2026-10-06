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
#include "B3DMetalShaderABI.h"
#include "B3DMetalUtility.h"
#include "B3DIMetalRenderWindowSurface.h"
#include "GpuBackend/B3DRenderWindow.h"
#include "GpuBackend/B3DGpuSubmitThread.h"
#include "GpuBackend/B3DGpuVertexInputManager.h"
#include "B3DMetalVertexInputManager.h"
#include "Image/B3DPixelUtility.h"
#include "Debug/B3DLog.h"

namespace b3d
{
	namespace render
	{
		namespace
		{
			/** Reports asynchronous Metal execution failures from a command-buffer completion handler. */
			void LogCommandBufferError(id<MTLCommandBuffer> commandBuffer)
			{
				if ([commandBuffer status] != MTLCommandBufferStatusError)
					return;

				NSError* error = [commandBuffer error];
				NSString* label = [commandBuffer label];
#if B3D_BUILD_TYPE_DEVELOPMENT
				NSArray<id<MTLCommandBufferEncoderInfo>>* encoderInformationArray = error ? [[error userInfo] objectForKey:MTLCommandBufferEncoderInfoErrorKey] : nil;
				for (id<MTLCommandBufferEncoderInfo> encoderInformation in encoderInformationArray)
				{
					B3D_LOG(Error, LogRenderBackend, "Metal encoder '{0}' failed with execution state {1}.",
						[encoderInformation label] ? String([[encoderInformation label] UTF8String]) : String("<unnamed>"),
						(u32)[encoderInformation errorState]);

					for (NSString* signpost in [encoderInformation debugSignposts])
						B3D_LOG(Error, LogRenderBackend, "  GPU signpost: {0}", String([signpost UTF8String]));
				}
#endif
				B3D_LOG(Fatal, LogRenderBackend, "Metal command buffer '{0}' failed during GPU execution ({1}, code {2}): {3}",
					label ? String([label UTF8String]) : String("<unnamed>"),
					error ? String([[error domain] UTF8String]) : String("<unknown domain>"),
					error ? (i64)[error code] : 0,
					error ? String([[error localizedDescription] UTF8String]) : String("No error details were provided."));
			}

			/**
			 * Registers a parameter set's argument buffer with the command buffer, so a heap-placed buffer the set
			 * releases (copy-on-write or destruction) is not returned to its allocator while this command buffer still
			 * reads it. Transient pool slices have no resource and rely on the pool's Reset contract instead.
			 */
			void TrackArgumentBuffer(const MetalGpuParameters& parameters, MetalResourceTracker& resourceTracker)
			{
				if (MetalArgumentBuffer* resource = parameters.GetArgumentBufferResource())
					resourceTracker.TrackResourceUsage(resource, GpuAccessFlag::Read);
			}

			/** Binds a parameter set's argument buffer to a render encoder at the buffer index matching its set index. */
			void AttachArgumentBufferToRenderEncoder(id<MTLRenderCommandEncoder> encoder, const MetalGpuParameters& parameters, MetalResourceTracker& resourceTracker)
			{
				const MetalGpuPipelineParameterSetLayout* layout = parameters.GetMetalLayout();
				id<MTLBuffer> argumentBuffer = parameters.GetArgumentBuffer();
				if (!layout || argumentBuffer == nil || parameters.GetSet() >= kMetalVertexBufferSlotBase)
					return;

				TrackArgumentBuffer(parameters, resourceTracker);

				const GpuProgramStageBits stages = layout->GetCombinedStages();
				if (stages.IsSetAny(GpuProgramStageBit::Vertex | GpuProgramStageBit::Hull | GpuProgramStageBit::Domain))
					[encoder setVertexBuffer:argumentBuffer offset:(NSUInteger)parameters.GetArgumentBufferOffset() atIndex:parameters.GetSet()];

				if (stages.IsSet(GpuProgramStageBit::Fragment))
					[encoder setFragmentBuffer:argumentBuffer offset:(NSUInteger)parameters.GetArgumentBufferOffset() atIndex:parameters.GetSet()];
			}

			/** Compute-encoder counterpart of AttachArgumentBufferToRenderEncoder. */
			void AttachArgumentBufferToComputeEncoder(id<MTLComputeCommandEncoder> encoder, const MetalGpuParameters& parameters, MetalResourceTracker& resourceTracker)
			{
				const MetalGpuPipelineParameterSetLayout* layout = parameters.GetMetalLayout();
				id<MTLBuffer> argumentBuffer = parameters.GetArgumentBuffer();
				if (!layout || argumentBuffer == nil || parameters.GetSet() >= kMetalVertexBufferSlotBase || !layout->GetCombinedStages().IsSet(GpuProgramStageBit::Compute))
					return;

				TrackArgumentBuffer(parameters, resourceTracker);
				[encoder setBuffer:argumentBuffer offset:(NSUInteger)parameters.GetArgumentBufferOffset() atIndex:parameters.GetSet()];
			}

			/**
			 * Replaces the contents of @p resources with the parameter set's cached resource handles at @p resourceIndices,
			 * skipping unbound (nil) entries. The caller must have called MetalGpuParameters::PrepareForBind() first.
			 */
			void GatherBucketResources(const MetalGpuParameters& parameters, const TArray<u32>& resourceIndices, Vector<__unsafe_unretained id<MTLResource>>& resources)
			{
				resources.clear();
				for (u32 resourceIndex : resourceIndices)
				{
					id<MTLResource> resource = parameters.GetCachedResource(resourceIndex);
					if (resource != nil)
						resources.push_back(resource);
				}
			}

			/**
			 * Makes the parameter set's resources resident on the render encoder, issuing one useResources: call per
			 * (usage, stages) bucket precomputed by the parameter set layout.
			 */
			void UseResourcesOnRenderEncoder(id<MTLRenderCommandEncoder> encoder, const MetalGpuParameters& parameters, Vector<__unsafe_unretained id<MTLResource>>& resources)
			{
				const MetalGpuPipelineParameterSetLayout* layout = parameters.GetMetalLayout();
				if (!layout)
					return;

				for (const auto& bucket : layout->GetRenderBuckets())
				{
					GatherBucketResources(parameters, bucket.ResourceIndices, resources);
					if (!resources.empty())
						[encoder useResources:resources.data() count:(NSUInteger)resources.size() usage:bucket.Usage stages:bucket.RenderStages];
				}
			}

			/** Compute-encoder counterpart of UseResourcesOnRenderEncoder. */
			void UseResourcesOnComputeEncoder(id<MTLComputeCommandEncoder> encoder, const MetalGpuParameters& parameters, Vector<__unsafe_unretained id<MTLResource>>& resources)
			{
				const MetalGpuPipelineParameterSetLayout* layout = parameters.GetMetalLayout();
				if (!layout)
					return;

				for (const auto& bucket : layout->GetComputeBuckets())
				{
					GatherBucketResources(parameters, bucket.ResourceIndices, resources);
					if (!resources.empty())
						[encoder useResources:resources.data() count:(NSUInteger)resources.size() usage:bucket.Usage];
				}
			}

			bool BindGraphicsPipelineForDraw(id<MTLRenderCommandEncoder> renderEncoder, MetalGpuGraphicsPipelineState* pipeline, DrawOperationType drawOperation, const MetalPipelineVariantKey& renderPassKey, const TShared<MetalVertexInput>& vertexInput)
			{
				if (!pipeline || renderEncoder == nil)
					return false;

				// The render pass supplies the attachment formats and sample count, only topology and vertex input vary per draw
				MetalPipelineVariantKey key = renderPassKey;
				key.TopologyClass = (u16)MetalUtility::GetPrimitiveTopologyClass(drawOperation);
				key.VertexInputId = vertexInput ? vertexInput->GetId() : 0;

				id<MTLRenderPipelineState> metalPipeline = pipeline->GetOrCreateMetalPipeline(key, vertexInput);
				if (metalPipeline == nil)
					return false;

				[renderEncoder setRenderPipelineState:metalPipeline];

				id<MTLDepthStencilState> depthStencil = pipeline->GetMetalDepthStencilState((key.ReadOnlyMask & RT_DEPTH) != 0, (key.ReadOnlyMask & RT_STENCIL) != 0);
				if (depthStencil)
					[renderEncoder setDepthStencilState:depthStencil];

				[renderEncoder setCullMode:(MTLCullMode)pipeline->GetCullMode()];
				[renderEncoder setFrontFacingWinding:(MTLWinding)pipeline->GetWinding()];
				[renderEncoder setTriangleFillMode:(MTLTriangleFillMode)pipeline->GetFillMode()];

				[renderEncoder setDepthBias:pipeline->GetDepthBias()
					slopeScale:pipeline->GetSlopeScaledDepthBias()
					clamp:pipeline->GetDepthBiasClamp()];
				return true;
			}

			/**
			 * Sets the load and store actions of every attachment @p descriptor has a texture for. Attachments neither
			 * loaded nor cleared use DontCare.
			 */
			void ConfigureAttachmentActions(MTLRenderPassDescriptor* descriptor, RenderSurfaceMask loadMask, RenderSurfaceMask clearMask, const RenderTargetClearValues& clearValues)
			{
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
					color.storeAction = MTLStoreActionStore;
				}

				MTLRenderPassDepthAttachmentDescriptor* depth = descriptor.depthAttachment;
				if (depth.texture != nil)
				{
					depth.loadAction = fnGetLoadAction(RT_DEPTH);
					depth.clearDepth = clearValues.Depth;
					depth.storeAction = MTLStoreActionStore;
				}

				MTLRenderPassStencilAttachmentDescriptor* stencil = descriptor.stencilAttachment;
				if (stencil.texture != nil)
				{
					stencil.loadAction = fnGetLoadAction(RT_STENCIL);
					stencil.clearStencil = clearValues.Stencil;
					stencil.storeAction = MTLStoreActionStore;
				}
			}

			/** Stores the attachment formats of @p descriptor in the pipeline variant key. Every MTLPixelFormat value fits in 16 bits. */
			void PackAttachmentFormats(MTLRenderPassDescriptor* descriptor, MetalPipelineVariantKey& key)
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

			class MetalSubmissionTransitionVisitor : public GpuSubmissionTransitionVisitor
			{
			public:
				void VisitBuffer(const GpuSubmissionBufferTransition& transition) override
				{
					mRequiredWaitMask |= transition.ParallelAccessWaitMask;
				}

				void VisitImage(const GpuSubmissionImageTransition& transition) override
				{
					mRequiredWaitMask |= transition.ParallelAccessWaitMask;
					transition.NativeState->Layout = (u32)transition.FinalLayout;
				}

				GpuQueueMask GetRequiredWaitMask() const { return mRequiredWaitMask; }

			private:
				GpuQueueMask mRequiredWaitMask = GpuQueueMask::kNone;
			};

			/**
			 * Encodes a signal for every user timeline fence. Signals execute in encoding order, so calling this after
			 * EncodeQueueSignal() makes the fences signal after the queue's own event.
			 */
			void EncodeUserFenceSignals(id<MTLCommandBuffer> commandBuffer, TArrayView<const GpuTimelineFenceAndValue> signalFences)
			{
				for (const GpuTimelineFenceAndValue& entry : signalFences)
				{
					if (!entry.Fence)
						continue;

					auto* metalFence = static_cast<MetalGpuTimelineFence*>(entry.Fence.get());
					id<MTLSharedEvent> sharedEvent = metalFence->GetSharedEvent();
					if (sharedEvent != nil)
						[commandBuffer encodeSignalEvent:sharedEvent value:entry.Value];
				}
			}
		} // namespace

		MetalGpuCommandBuffer::MetalGpuCommandBuffer(MetalGpuDevice& device, MetalGpuCommandBufferPool& pool, u32 id, ThreadId ownerThread, GpuQueueType queueType, const GpuCommandBufferCreateInformation& createInformation)
			: GpuCommandBuffer(device, ownerThread, queueType, createInformation), mGpuDevice(device), mPool(pool), mId(id), mBarrierHelper(&mResourceTracker)
		{
		}

		MetalGpuCommandBuffer::~MetalGpuCommandBuffer()
		{
			CloseAllEncoders();

			mCommandBuffer = nil;
#if B3D_METAL_USE_EXPLICIT_RESOURCE_SYNCHRONIZATION
			mResourceFence = nil;
#endif
		}

		void MetalGpuCommandBuffer::CloseAllEncoders()
		{
			if (mRenderEncoder)
			{
				[mRenderEncoder endEncoding];
				mRenderEncoder = nil;
			}

			if (mComputeEncoder)
			{
				[mComputeEncoder endEncoding];
				mComputeEncoder = nil;
			}

			if (mBlitEncoder)
			{
				[mBlitEncoder endEncoding];
				mBlitEncoder = nil;
			}
		}

#if B3D_METAL_USE_EXPLICIT_RESOURCE_SYNCHRONIZATION
		// TODO - Every encoder waits on the previous one through a single fence, serializing all encoders regardless of
		// actual hazards. Refactor to the MTL4CommandQueue stage-to-stage barrier model once macOS 26 can be targeted.
		void MetalGpuCommandBuffer::UpdateResourceFence(id<MTLRenderCommandEncoder> encoder)
		{
			if (mResourceFence == nil || encoder == nil)
				return;

			[encoder updateFence:mResourceFence afterStages:MTLRenderStageFragment];
			mFenceNeedsWait = true;
		}

		void MetalGpuCommandBuffer::UpdateResourceFence(id<MTLComputeCommandEncoder> encoder)
		{
			if (mResourceFence == nil || encoder == nil)
				return;

			[encoder updateFence:mResourceFence];
			mFenceNeedsWait = true;
		}

		void MetalGpuCommandBuffer::UpdateResourceFence(id<MTLBlitCommandEncoder> encoder)
		{
			if (mResourceFence == nil || encoder == nil)
				return;

			[encoder updateFence:mResourceFence];
			mFenceNeedsWait = true;
		}

		void MetalGpuCommandBuffer::WaitForResourceFence(id<MTLRenderCommandEncoder> encoder)
		{
			if (!mFenceNeedsWait || mResourceFence == nil || encoder == nil)
				return;

			[encoder waitForFence:mResourceFence beforeStages:MTLRenderStageVertex];
			mFenceNeedsWait = false;
		}

		void MetalGpuCommandBuffer::WaitForResourceFence(id<MTLComputeCommandEncoder> encoder)
		{
			if (!mFenceNeedsWait || mResourceFence == nil || encoder == nil)
				return;

			[encoder waitForFence:mResourceFence];
			mFenceNeedsWait = false;
		}

		void MetalGpuCommandBuffer::WaitForResourceFence(id<MTLBlitCommandEncoder> encoder)
		{
			if (!mFenceNeedsWait || mResourceFence == nil || encoder == nil)
				return;

			[encoder waitForFence:mResourceFence];
			mFenceNeedsWait = false;
		}
#endif

		id<MTLCommandBuffer> MetalGpuCommandBuffer::GetOrAcquireMetalCommandBuffer()
		{
			if (mRecordingFailed)
				return nil;

			if (mCommandBuffer != nil)
				return mCommandBuffer;

			auto metalQueue = std::static_pointer_cast<MetalGpuQueue>(mGpuDevice.GetQueue(mQueueType, 0));
			if (!metalQueue)
				return nil;

			id<MTLCommandQueue> metalCommandQueue = metalQueue->GetMetalQueue();
			if (metalCommandQueue == nil)
				return nil;

#if B3D_BUILD_TYPE_DEVELOPMENT
			MTLCommandBufferDescriptor* descriptor = [[MTLCommandBufferDescriptor alloc] init];
			descriptor.errorOptions = MTLCommandBufferErrorOptionEncoderExecutionStatus;
			mCommandBuffer = [metalCommandQueue commandBufferWithDescriptor:descriptor];
#else
			mCommandBuffer = [metalCommandQueue commandBuffer];
#endif
			if (mCommandBuffer == nil)
				return nil;

#if B3D_METAL_USE_EXPLICIT_RESOURCE_SYNCHRONIZATION
			if (mResourceFence == nil)
			{
				mResourceFence = [mGpuDevice.GetMetalDevice() newFence];
				if (mResourceFence == nil)
				{
					B3D_LOG(Fatal, LogRenderBackend, "Explicit Metal resource synchronization requires MTLFence support.");
					mRecordingFailed = true;
					return nil;
				}
			}
#endif

#if B3D_BUILD_TYPE_DEVELOPMENT
			if (!mName.empty())
			{
				NSString* label = [NSString stringWithUTF8String:mName.c_str()];
				[mCommandBuffer setLabel:label];
			}
#endif

			// Metal has no explicit Begin(), so the first acquisition marks the start of recording
			if (mState == GpuCommandBufferState::Ready)
				mState = GpuCommandBufferState::Recording;

			return mCommandBuffer;
		}

		void MetalGpuCommandBuffer::EnsureEncoderKind(EncoderKind targetKind)
		{
			// Metal scopes useResource: to a single encoder, so closing an encoder also resets its residency caches
			if (targetKind != EncoderKind::Render && mRenderEncoder != nil)
			{
#if B3D_METAL_USE_EXPLICIT_RESOURCE_SYNCHRONIZATION
				UpdateResourceFence(mRenderEncoder);
#endif
				[mRenderEncoder endEncoding];
				mRenderEncoder = nil;

				ResetRenderResidencyCaches();
				ResetArgumentTableBindings();
				EncodePendingEventSignals();
			}

			if (targetKind != EncoderKind::Compute && mComputeEncoder != nil)
			{
#if B3D_METAL_USE_EXPLICIT_RESOURCE_SYNCHRONIZATION
				UpdateResourceFence(mComputeEncoder);
#endif
				[mComputeEncoder endEncoding];
				mComputeEncoder = nil;

				ResetComputeResidencyCaches();
				ResetArgumentTableBindings();
			}

			if (targetKind != EncoderKind::Blit && mBlitEncoder != nil)
			{
#if B3D_METAL_USE_EXPLICIT_RESOURCE_SYNCHRONIZATION
				UpdateResourceFence(mBlitEncoder);
#endif
				[mBlitEncoder endEncoding];
				mBlitEncoder = nil;
			}
		}

		bool MetalGpuCommandBuffer::RestartRenderPassForBarrier()
		{
#if B3D_METAL_USE_EXPLICIT_RESOURCE_SYNCHRONIZATION
			if (mRenderEncoder == nil || mRestartRenderPassDescriptor == nil)
			{
				B3D_LOG(Fatal, LogRenderBackend, "Cannot restart the Metal render pass required for explicit resource synchronization.");
				mRecordingFailed = true;

				EnsureEncoderKind(EncoderKind::None);
				return false;
			}

			EnsureEncoderKind(EncoderKind::None);
			return ResumeRenderPass(mRestartRenderPassDescriptor);
#else
			return false;
#endif
		}

		bool MetalGpuCommandBuffer::ResumeRenderPass(MTLRenderPassDescriptor* descriptor)
		{
			if (descriptor == nil || mRecordingFailed)
				return false;

			id<MTLCommandBuffer> commandBuffer = GetOrAcquireMetalCommandBuffer();
			if (commandBuffer == nil)
			{
				mRecordingFailed = true;
				return false;
			}

			mRenderEncoder = [commandBuffer renderCommandEncoderWithDescriptor:descriptor];
			if (mRenderEncoder == nil)
			{
				B3D_LOG(Fatal, LogRenderBackend, "Failed to resume a Metal render pass after an encoder boundary.");
				mRecordingFailed = true;

				return false;
			}

			mGraphicsPushConstantsRequireBind = true;
#if B3D_BUILD_TYPE_DEVELOPMENT
			mRenderEncoder.label = @"Render pass (resumed)";
#endif

#if B3D_METAL_USE_EXPLICIT_RESOURCE_SYNCHRONIZATION
			WaitForResourceFence(mRenderEncoder);
#endif
			if (mHasViewport)
				[mRenderEncoder setViewport:mViewport];

			if (mHasScissor)
				[mRenderEncoder setScissorRect:mScissor];

			for (const VertexBufferBinding& binding : mVertexBufferBindings)
				[mRenderEncoder setVertexBuffer:binding.Buffer offset:binding.Offset atIndex:binding.Index];

			[mRenderEncoder setVisibilityResultMode:mVisibilityMode offset:mVisibilityOffset];
			return true;
		}

		bool MetalGpuCommandBuffer::ActivateOcclusionQueryPool(const TShared<MetalGpuQueryPool>& queryPool)
		{
			if (!queryPool || mRenderEncoder == nil || mRestartRenderPassDescriptor == nil)
				return false;

			if (queryPool.get() == mActiveOcclusionQueryPool.get())
				return true;

			id<MTLBuffer> visibilityBuffer = queryPool->GetVisibilityBuffer();
			if (visibilityBuffer == nil)
			{
				B3D_LOG(Error, LogRenderBackend, "Cannot activate a Metal occlusion query pool without a visibility buffer.");
				return false;
			}

			// visibilityResultBuffer cannot change within a render encoder, so continue the render pass in a new encoder
			// that loads the attachments the previous one stored
			mRestartRenderPassDescriptor.visibilityResultBuffer = visibilityBuffer;
			EnsureEncoderKind(EncoderKind::None);
			mActiveOcclusionQueryPool.reset();

			if (!ResumeRenderPass(mRestartRenderPassDescriptor))
				return false;

			mActiveOcclusionQueryPool = queryPool;
			return true;
		}

		bool MetalGpuCommandBuffer::ExecutePendingBarriers()
		{
			if (mRecordingFailed)
				return false;

#if B3D_METAL_USE_EXPLICIT_RESOURCE_SYNCHRONIZATION
			if (mRenderEncoder != nil && mBarrierHelper.RequiresRenderPassRestart())
			{
				if (!RestartRenderPassForBarrier())
					return false;

				mBarrierHelper.Execute(nil, nil);
				return true;
			}

			if (mBlitEncoder != nil && mBarrierHelper.HasBarriers())
			{
				EnsureEncoderKind(EncoderKind::None);
				mBarrierHelper.Execute(nil, nil);
				return true;
			}
#endif

			mBarrierHelper.Execute(mRenderEncoder, mComputeEncoder);
			return true;
		}

		void MetalGpuCommandBuffer::ResetRenderResidencyCaches()
		{
			for (u32 slotIndex = 0; slotIndex < (u32)mRenderResidencyCaches.Size(); slotIndex++)
				mRenderResidencyCaches[slotIndex].Reset();
		}

		void MetalGpuCommandBuffer::ResetComputeResidencyCaches()
		{
			for (u32 slotIndex = 0; slotIndex < (u32)mComputeResidencyCaches.Size(); slotIndex++)
				mComputeResidencyCaches[slotIndex].Reset();
		}

		id<MTLCommandEncoder> MetalGpuCommandBuffer::GetActiveEncoder() const
		{
			// At most one encoder is open at a time
			if (mRenderEncoder)
				return mRenderEncoder;

			if (mComputeEncoder)
				return mComputeEncoder;

			if (mBlitEncoder)
				return mBlitEncoder;

			return nil;
		}

		id<MTLBlitCommandEncoder> MetalGpuCommandBuffer::GetOrOpenBlitEncoder()
		{
			EnsureEncoderKind(EncoderKind::Blit);

			if (mBlitEncoder != nil)
				return mBlitEncoder;

			id<MTLCommandBuffer> commandBuffer = GetOrAcquireMetalCommandBuffer();
			if (commandBuffer == nil)
				return nil;

			mBlitEncoder = [commandBuffer blitCommandEncoder];
#if B3D_BUILD_TYPE_DEVELOPMENT
			mBlitEncoder.label = @"Blit pass";
#endif
#if B3D_METAL_USE_EXPLICIT_RESOURCE_SYNCHRONIZATION
			WaitForResourceFence(mBlitEncoder);
#endif
			return mBlitEncoder;
		}

		bool MetalGpuCommandBuffer::EncodeSignalEvent(id<MTLSharedEvent> event, u64 value)
		{
			EnsureValidThread();

			if (event == nil || mRecordingFailed)
				return false;

			if (mRenderEncoder != nil)
			{
				PendingEventSignal signal;
				signal.Event = event;
				signal.Value = value;
				mPendingEventSignals.push_back(signal);
				return true;
			}

			EnsureEncoderKind(EncoderKind::None);
			id<MTLCommandBuffer> commandBuffer = GetOrAcquireMetalCommandBuffer();
			if (commandBuffer == nil)
				return false;

			[commandBuffer encodeSignalEvent:event value:value];
			return true;
		}

		void MetalGpuCommandBuffer::EncodePendingEventSignals()
		{
			if (mPendingEventSignals.empty())
				return;

			id<MTLCommandBuffer> commandBuffer = mCommandBuffer;
			if (commandBuffer != nil && !mRecordingFailed)
			{
				for (const PendingEventSignal& signal : mPendingEventSignals)
					[commandBuffer encodeSignalEvent:signal.Event value:signal.Value];
			}

			mPendingEventSignals.clear();
		}

		void MetalGpuCommandBuffer::SetName(const StringView& name)
		{
			EnsureValidThread();
			mName = name;
			if (mCommandBuffer)
			{
				NSString* label = [NSString stringWithUTF8String:mName.c_str()];
				[mCommandBuffer setLabel:label];
			}
		}

		void MetalGpuCommandBuffer::SetGpuParameterSet(const TShared<GpuParameterSet>& parameters)
		{
			EnsureValidThread();

			// A null set carries no set index, so there is no slot to clear. Sets are replaced by binding another set at the same index.
			if (!parameters)
				return;

			if (!BindParameterSet(parameters))
				return;

			mGraphicsResourcesRequireTracking = true;

			// Pending writes are committed and residency emitted at draw / dispatch, so binding a set several times
			// before a draw only does that work once
		}

		bool MetalGpuCommandBuffer::BindParameterSet(const TShared<GpuParameterSet>& parameters)
		{
			const u32 setIndex = parameters->GetSet();
			if (setIndex >= kMetalVertexBufferSlotBase)
			{
				B3D_LOG(Error, LogRenderBackend, "Metal parameter set index {0} collides with reserved vertex-buffer slots.", setIndex);
				return false;
			}

			if (setIndex >= (u32)mBoundParameterSets.Size())
			{
				mBoundParameterSets.Resize(setIndex + 1);
				mRenderResidencyCaches.Resize(setIndex + 1);
				mComputeResidencyCaches.Resize(setIndex + 1);
				mDynamicOffsetOverridesPerSet.Resize(setIndex + 1);
			}

			// The caches identify sets by address, and the replaced set may be freed and its address reused by a new set
			if (mBoundParameterSets[setIndex] != parameters)
			{
				mRenderResidencyCaches[setIndex].Reset();
				mComputeResidencyCaches[setIndex].Reset();
			}

			mBoundParameterSets[setIndex] = parameters;
			ResetDynamicOffsetOverrides(setIndex);
			return true;
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
			EnsureValidThread();

			mBoundGraphicsPipeline = std::static_pointer_cast<MetalGpuGraphicsPipelineState>(pipelineState);
			mGraphicsResourcesRequireTracking = true;
			mGraphicsPushConstantsRequireBind = true;
		}

		void MetalGpuCommandBuffer::SetGpuComputePipelineState(const TShared<GpuComputePipelineState>& pipelineState)
		{
			EnsureValidThread();

			mBoundComputePipeline = pipelineState;
			mGraphicsResourcesRequireTracking = true;
			mComputePushConstantsRequireBind = true;

			if (!pipelineState || !mComputeEncoder)
				return;

			auto metalPipelineState = std::static_pointer_cast<MetalGpuComputePipelineState>(pipelineState);
			id<MTLComputePipelineState> metalPipeline = metalPipelineState->GetMetalPipeline();
			if (metalPipeline)
				[mComputeEncoder setComputePipelineState:metalPipeline];
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
				if(isGraphics)
				{
					B3D_ASSERT(mRenderEncoder != nil);
					if(stages.IsSet(GpuProgramStageBit::Vertex))
						[mRenderEncoder setVertexBytes:mPushConstants.GetData() length:pushConstantBufferSize atIndex:kMetalPushConstantBufferIndex];
					if(stages.IsSet(GpuProgramStageBit::Fragment))
						[mRenderEncoder setFragmentBytes:mPushConstants.GetData() length:pushConstantBufferSize atIndex:kMetalPushConstantBufferIndex];
				}
				else
				{
					B3D_ASSERT(mComputeEncoder != nil);
					if(stages.IsSet(GpuProgramStageBit::Compute))
						[mComputeEncoder setBytes:mPushConstants.GetData() length:pushConstantBufferSize atIndex:kMetalPushConstantBufferIndex];
				}
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

					const u32 bufferIndex = binding.BufferIndex;
					if (bufferIndex == ~0u)
						continue;

					B3D_ASSERT(bufferIndex >= kMetalDynamicUniformBufferIndexBase && bufferIndex < kMetalDynamicUniformBufferIndexBase + kMetalDynamicUniformBufferCount);
					const u32 tableIndex = bufferIndex - kMetalDynamicUniformBufferIndexBase;

					// An encoder retains every buffer handed to it, so a cached address cannot be recycled by another buffer
					// while the encoder is open; a matching address means the same buffer and only the offset moves
					auto fnNeedsBind = [metalBuffer, offset](ArgumentTableBinding& outCached, bool& outBufferChanged)
					{
						outBufferChanged = outCached.Buffer != metalBuffer;
						if (!outBufferChanged && outCached.Offset == offset)
							return false;

						outCached.Buffer = metalBuffer;
						outCached.Offset = offset;
						return true;
					};

					bool bufferChanged = false;
					if (!isGraphics)
					{
						if (binding.Stages.IsSet(GpuProgramStageBit::Compute) && fnNeedsBind(mComputeArgumentTable[tableIndex], bufferChanged))
						{
							if (bufferChanged)
								[mComputeEncoder setBuffer:metalBuffer offset:offset atIndex:bufferIndex];
							else
								[mComputeEncoder setBufferOffset:offset atIndex:bufferIndex];
						}

						continue;
					}

					const GpuProgramStageBits vertexStages = GpuProgramStageBit::Vertex | GpuProgramStageBit::Hull | GpuProgramStageBit::Domain;
					if (binding.Stages.IsSetAny(vertexStages) && fnNeedsBind(mVertexArgumentTable[tableIndex], bufferChanged))
					{
						if (bufferChanged)
							[mRenderEncoder setVertexBuffer:metalBuffer offset:offset atIndex:bufferIndex];
						else
							[mRenderEncoder setVertexBufferOffset:offset atIndex:bufferIndex];
					}

					if (binding.Stages.IsSet(GpuProgramStageBit::Fragment) && fnNeedsBind(mFragmentArgumentTable[tableIndex], bufferChanged))
					{
						if (bufferChanged)
							[mRenderEncoder setFragmentBuffer:metalBuffer offset:offset atIndex:bufferIndex];
						else
							[mRenderEncoder setFragmentBufferOffset:offset atIndex:bufferIndex];
					}
				}
			}
		}

		void MetalGpuCommandBuffer::ResetArgumentTableBindings()
		{
			for (u32 tableIndex = 0; tableIndex < kMetalDynamicUniformBufferCount; tableIndex++)
			{
				mVertexArgumentTable[tableIndex] = ArgumentTableBinding();
				mFragmentArgumentTable[tableIndex] = ArgumentTableBinding();
				mComputeArgumentTable[tableIndex] = ArgumentTableBinding();
			}
		}

		void MetalGpuCommandBuffer::SetVertexBuffers(u32 index, TShared<GpuBuffer>* buffers, u32 bufferCount)
		{
			EnsureValidThread();

			// Native handles are resolved at draw time, see mBoundVertexBuffers
			const u32 endIndex = index + bufferCount;
			while ((u32)mBoundVertexBuffers.Size() < endIndex)
				mBoundVertexBuffers.Add(nullptr);

			for (u32 bufferIndex = 0; bufferIndex < bufferCount; bufferIndex++)
				mBoundVertexBuffers[index + bufferIndex] = std::static_pointer_cast<MetalGpuBuffer>(buffers[bufferIndex]);
		}

		void MetalGpuCommandBuffer::ApplyVertexBuffersToRenderEncoder()
		{
			if (mRenderEncoder == nil)
				return;

			for (u32 streamIndex = 0; streamIndex < (u32)mBoundVertexBuffers.Size(); streamIndex++)
			{
				const TShared<MetalGpuBuffer>& metalBuffer = mBoundVertexBuffers[streamIndex];
				id<MTLBuffer> buffer = metalBuffer ? metalBuffer->GetMetalBuffer() : nil;
				if (buffer == nil)
					continue;

				MetalBuffer* resource = metalBuffer->GetMetalResource();
				if (resource != nullptr)
					mResourceTracker.TrackBufferAccess(resource, GpuStageFlag::VertexInputAttributes, GpuAccessFlag::Read, mBarrierHelper);

				// Vertex streams sit above the argument buffer slots, matching the pipeline's vertex descriptor
				const NSUInteger metalIndex = kMetalVertexBufferSlotBase + streamIndex;

				auto existing = std::find_if(mVertexBufferBindings.begin(), mVertexBufferBindings.end(), [metalIndex](const VertexBufferBinding& binding) { return binding.Index == metalIndex; });
				if (existing != mVertexBufferBindings.end())
				{
					if (existing->Buffer == buffer && existing->Offset == 0)
						continue;

					existing->Buffer = buffer;
					existing->Offset = 0;
				}
				else
				{
					VertexBufferBinding binding;
					binding.Buffer = buffer;
					binding.Index = metalIndex;
					mVertexBufferBindings.push_back(binding);
				}

				[mRenderEncoder setVertexBuffer:buffer offset:0 atIndex:metalIndex];
			}
		}

		void MetalGpuCommandBuffer::SetIndexBuffer(const TShared<GpuBuffer>& buffer)
		{
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

		TShared<MetalVertexInput> MetalGpuCommandBuffer::ResolveVertexInputForDraw(bool& outSkipDraw)
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

			TShared<MetalVertexInput> vertexInput = MetalVertexInputManager::Instance().GetVertexInput(mBoundVertexDescription, pipeline->GetInputDeclaration());
			if (!vertexInput)
			{
				B3D_LOG(Warning, LogRenderBackend, "Skipping draw: vertex input could not be resolved for the bound vertex description on Metal.");
				outSkipDraw = true;
				return nullptr;
			}

			// Shader inputs with no matching vertex buffer element read zeroes from the null stream
			if (vertexInput->HasNullStream() && mRenderEncoder != nil)
			{
				[mRenderEncoder setVertexBuffer:mGpuDevice.GetNullVertexBuffer()
					offset:0
					atIndex:(kMetalVertexBufferSlotBase + vertexInput->GetNullStreamIndex())];
			}

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

			for(u32 setIndex = 0; setIndex < (u32)mBoundParameterSets.Size(); setIndex++)
			{
				const TShared<GpuParameterSet>& parameters = mBoundParameterSets[setIndex];
				if(parameters == nullptr)
					continue;

				if(!static_cast<MetalGpuParameters&>(*parameters).TrackResources(mResourceTracker, mBarrierHelper, mDynamicOffsetOverridesPerSet[setIndex], compute))
					return false;

#if B3D_BUILD_TYPE_DEVELOPMENT
				if(!compute && mBoundGraphicsPipeline != nullptr && parameters->GetSet() < mBoundGraphicsPipeline->GetParameterLayout()->GetSetCount())
					mDrawAccessValidator.AddParameterSet(*parameters, *mBoundGraphicsPipeline->GetParameterLayout()->GetSet(parameters->GetSet()));
#endif
			}

			mGraphicsResourcesRequireTracking = compute;
			return true;
		}

		void MetalGpuCommandBuffer::AttachParameterSetsToEncoder(bool isGraphics)
		{
			// Sets can be bound before the encoder exists, so they are attached and made resident here. Residency lasts for
			// the encoder's lifetime, so it is skipped for a set already emitted at the same generation.
			TInlineArray<ParameterSetResidencyCache, 4>& residencyCaches = isGraphics ? mRenderResidencyCaches : mComputeResidencyCaches;
			for (u32 slotIndex = 0; slotIndex < (u32)mBoundParameterSets.Size(); slotIndex++)
			{
				const TShared<GpuParameterSet>& slotSet = mBoundParameterSets[slotIndex];
				if (!slotSet)
					continue;

				auto& metalParameters = static_cast<MetalGpuParameters&>(*slotSet);
				ParameterSetResidencyCache& cacheEntry = residencyCaches[slotIndex];

				// Prepare before attaching: a copy-on-write can move the argument buffer, and the generation bump it performs forces the re-attach below
				const u64 generation = metalParameters.PrepareForBind();
				if (cacheEntry.LastBoundSet == &metalParameters && cacheEntry.LastBoundGeneration == generation)
					continue;

				if (isGraphics)
				{
					AttachArgumentBufferToRenderEncoder(mRenderEncoder, metalParameters, mResourceTracker);
					UseResourcesOnRenderEncoder(mRenderEncoder, metalParameters, mResidencyResources);
				}
				else
				{
					AttachArgumentBufferToComputeEncoder(mComputeEncoder, metalParameters, mResourceTracker);
					UseResourcesOnComputeEncoder(mComputeEncoder, metalParameters, mResidencyResources);
				}

				cacheEntry.LastBoundSet = &metalParameters;
				cacheEntry.LastBoundGeneration = generation;
			}
		}

		bool MetalGpuCommandBuffer::PrepareDraw(MetalBuffer* indexBuffer)
		{
			// Must precede the barrier flush, so vertex buffer usage is tracked and a pass restart replays the current bindings
			ApplyVertexBuffersToRenderEncoder();

			if (indexBuffer != nullptr)
				mResourceTracker.TrackBufferAccess(indexBuffer, GpuStageFlag::VertexInputIndices, GpuAccessFlag::Read, mBarrierHelper);

			if(!TrackShaderResources(false))
				return false;

			if (!ExecutePendingBarriers())
				return false;

			AttachParameterSetsToEncoder(true);
			BindDynamicUniformBuffers(true);

			bool skipDraw = false;
			TShared<MetalVertexInput> vertexInput = ResolveVertexInputForDraw(skipDraw);
			if (skipDraw)
				return false;

			if (!BindGraphicsPipelineForDraw(mRenderEncoder, mBoundGraphicsPipeline.get(), mDrawOperation, mRenderPassPipelineKey, vertexInput))
				return false;

			BindPushConstants(true);
			[mRenderEncoder setStencilReferenceValue:mStencilReference];

#if B3D_BUILD_TYPE_DEVELOPMENT
			if(!ValidateDrawAccesses())
				return false;
#endif

			return true;
		}

		void MetalGpuCommandBuffer::Draw(u32 vertexOffset, u32 vertexCount, u32 instanceCount, u32 firstInstance)
		{
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
		}

		void MetalGpuCommandBuffer::DrawIndexed(u32 startIndex, u32 indexCount, u32 vertexOffset, u32 vertexCount, u32 instanceCount, u32 firstInstance)
		{
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

			[mRenderEncoder drawIndexedPrimitives:MetalUtility::GetPrimitiveType(mDrawOperation)
				indexCount:indexCount
				indexType:indexType
				indexBuffer:indexBuffer
				indexBufferOffset:(startIndex * indexSize)
				instanceCount:std::max<u32>(1, instanceCount)
				baseVertex:vertexOffset
				baseInstance:firstInstance];
		}

		void MetalGpuCommandBuffer::DispatchCompute(u32 groupCountX, u32 groupCountY, u32 groupCountZ)
		{
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

			EnsureEncoderKind(EncoderKind::Compute);

			id<MTLCommandBuffer> commandBuffer = GetOrAcquireMetalCommandBuffer();
			if (commandBuffer == nil)
				return;

			if (mComputeEncoder == nil)
			{
				mComputeEncoder = [commandBuffer computeCommandEncoder];
				if (mComputeEncoder == nil)
					return;

				mComputePushConstantsRequireBind = true;
#if B3D_BUILD_TYPE_DEVELOPMENT
				mComputeEncoder.label = @"Compute pass";
#endif
#if B3D_METAL_USE_EXPLICIT_RESOURCE_SYNCHRONIZATION
				WaitForResourceFence(mComputeEncoder);
#endif
			}

			[mComputeEncoder setComputePipelineState:metalPipeline];

			if(!TrackShaderResources(true))
				return;

			if (!ExecutePendingBarriers())
				return;

			AttachParameterSetsToEncoder(false);
			BindDynamicUniformBuffers(false);
			BindPushConstants(false);

			MTLSize threadsPerGroup = MTLSizeMake(workgroupSize[0], workgroupSize[1], workgroupSize[2]);
			MTLSize groups = MTLSizeMake(groupCountX, groupCountY, groupCountZ);
			[mComputeEncoder dispatchThreadgroups:groups threadsPerThreadgroup:threadsPerGroup];
		}

		void MetalGpuCommandBuffer::BeginRenderPass(const RenderPassCreateInformation& createInformation)
		{
			EnsureValidThread();
			EnsureEncoderKind(EncoderKind::None);

			mRenderPassPipelineKey = MetalPipelineVariantKey{};
			mAcquiredWindowSurface = nullptr;
			mRenderPassWidth = 0;
			mRenderPassHeight = 0;
			mRestartRenderPassDescriptor = nil;
			mVertexBufferBindings.clear();
			mBoundVertexBuffers.Clear();
			// The normalized viewport persists across passes and is converted to pixels once the encoder is open
			mHasScissor = false;
			mVisibilityMode = MTLVisibilityResultModeDisabled;
			mVisibilityOffset = 0;

			const TShared<RenderTarget>& target = createInformation.Target;
			if (!target)
				return;

			id<MTLCommandBuffer> commandBuffer = GetOrAcquireMetalCommandBuffer();
			if (commandBuffer == nil)
				return;

			MTLRenderPassDescriptor* descriptor = [MTLRenderPassDescriptor renderPassDescriptor];

			// BeginQuery() selects the visibility buffer lazily, see ActivateOcclusionQueryPool()
			mActiveOcclusionQueryPool.reset();

			const RenderTargetProperties& targetProperties = target->GetProperties();
			mRenderPassWidth = targetProperties.Width;
			mRenderPassHeight = targetProperties.Height;
			mRenderPassPipelineKey.SampleCount = (u16)std::max(1u, targetProperties.MultisampleCount);
			mRenderPassClearValues = target->GetClearValues();

			const RenderSurfaceMask clearMask = createInformation.ClearMask;
			const RenderSurfaceMask loadMask = createInformation.LoadMask;
			const RenderSurfaceMask readOnlyMask = createInformation.ReadOnlyMask;
			GpuRenderPassAttachmentUsageArray renderPassAttachmentUsages;

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

				id<MTLTexture> depthStencilTexture = metalSurface->GetDepthStencilTexture();
				if (depthStencilTexture != nil)
				{
					const MTLPixelFormat depthStencilFormat = [depthStencilTexture pixelFormat];
					if (MetalUtility::PixelFormatHasDepth(depthStencilFormat))
						descriptor.depthAttachment.texture = depthStencilTexture;

					if (MetalUtility::PixelFormatHasStencil(depthStencilFormat))
						descriptor.stencilAttachment.texture = depthStencilTexture;
				}
			}
			else
			{
				MetalFramebuffer* framebuffer = static_cast<MetalRenderTexture*>(target.get())->GetFramebuffer();
				if (!B3D_ENSURE(framebuffer != nullptr))
					return;

				framebuffer->ApplyAttachments(descriptor);
				renderPassAttachmentUsages = framebuffer->BuildRenderPassAttachmentUsages(readOnlyMask, loadMask, MetalFramebuffer::GetLayoutPolicy());
			}

			ConfigureAttachmentActions(descriptor, loadMask, clearMask, mRenderPassClearValues);
			PackAttachmentFormats(descriptor, mRenderPassPipelineKey);

#if B3D_BUILD_TYPE_DEVELOPMENT
			mDrawAccessValidator.BeginRenderPass();
#endif
			mGraphicsResourcesRequireTracking = true;
			mResourceTracker.PrepareRenderPass(renderPassAttachmentUsages);
			const TArrayView<const GpuResolvedRenderPassAttachmentUsage> resolvedAttachments = mResourceTracker.BeginRenderPass(mBarrierHelper);
			mRenderPassTrackingActive = true;

			// Pipelines bound in this pass mask off writes to whatever resolved as read-only.
			RenderSurfaceMask resolvedReadOnlyMask = RT_NONE;
			for (const GpuResolvedRenderPassAttachmentUsage& attachment : resolvedAttachments)
			{
				if (attachment.Access == GpuAccessFlag::Read)
					resolvedReadOnlyMask.Set(attachment.Surface);
			}

			mRenderPassPipelineKey.ReadOnlyMask = (u32)resolvedReadOnlyMask;

			if (!ExecutePendingBarriers())
				return;

			// Encoders that continue this pass (barriers, occlusion pool changes) load what the previous encoder stored
			mRestartRenderPassDescriptor = [descriptor copy];
			for (u32 attachmentIndex = 0; attachmentIndex < B3D_MAXIMUM_RENDER_TARGET_COUNT; attachmentIndex++)
			{
				MTLRenderPassColorAttachmentDescriptor* attachment = mRestartRenderPassDescriptor.colorAttachments[attachmentIndex];
				if (attachment.texture != nil)
					attachment.loadAction = MTLLoadActionLoad;
			}

			if (mRestartRenderPassDescriptor.depthAttachment.texture != nil)
				mRestartRenderPassDescriptor.depthAttachment.loadAction = MTLLoadActionLoad;

			if (mRestartRenderPassDescriptor.stencilAttachment.texture != nil)
				mRestartRenderPassDescriptor.stencilAttachment.loadAction = MTLLoadActionLoad;

			mRenderEncoder = [commandBuffer renderCommandEncoderWithDescriptor:descriptor];
#if B3D_BUILD_TYPE_DEVELOPMENT
			mRenderEncoder.label = @"Render pass";
#endif
#if B3D_METAL_USE_EXPLICIT_RESOURCE_SYNCHRONIZATION
			WaitForResourceFence(mRenderEncoder);
#endif

			// Otherwise the acquired drawable stays held until the next present and starves the drawable pool
			if (mRenderEncoder == nil && mAcquiredWindowSurface != nullptr)
			{
				B3D_LOG(Error, LogRenderBackend, "BeginRenderPass: failed to create MTLRenderCommandEncoder after acquiring a drawable; aborting drawable.");

				mAcquiredWindowSurface->AbortCurrentDrawable();
				mAcquiredWindowSurface = nullptr;
				return;
			}

			if (mRenderEncoder == nil)
				return;

			mGraphicsPushConstantsRequireBind = true;

			// Lets SwapBuffers() tell a rendered drawable apart from one that was acquired but never rendered to
			if (mAcquiredWindowSurface != nullptr)
				mAcquiredWindowSurface->MarkDrawableAsRendered();

			ApplyViewportToRenderEncoder();

			// Binds the pass's parameter sets like SetGpuParameterSet() would, they are attached at the first draw
			for (const TShared<GpuParameterSet>& parameterSet : createInformation.Parameters)
			{
				if (parameterSet)
					BindParameterSet(parameterSet);
			}
		}

		void MetalGpuCommandBuffer::EndRenderPass()
		{
			EnsureValidThread();
			// Only the render encoder should be open here. Others are left alone so a stray one is not silently ended.
			if (mRenderEncoder != nil)
			{
#if B3D_METAL_USE_EXPLICIT_RESOURCE_SYNCHRONIZATION
				UpdateResourceFence(mRenderEncoder);
#endif
				[mRenderEncoder endEncoding];
				mRenderEncoder = nil;
				ResetRenderResidencyCaches();
				ResetArgumentTableBindings();
				EncodePendingEventSignals();
			}

			mAcquiredWindowSurface = nullptr;
			mRenderPassPipelineKey = MetalPipelineVariantKey{};
			mRenderPassWidth = 0;
			mRenderPassHeight = 0;
			mActiveOcclusionQueryPool.reset();
			mRestartRenderPassDescriptor = nil;
			mVertexBufferBindings.clear();
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

		void MetalGpuCommandBuffer::ClearRenderTarget(RenderSurfaceMask mask)
		{
			EnsureValidThread();
			if (mask == RT_NONE || mRenderEncoder == nil || mRestartRenderPassDescriptor == nil)
				return;

			MTLRenderPassDescriptor* clearDescriptor = [mRestartRenderPassDescriptor copy];
			bool hasAttachment = false;
			for (u32 attachmentIndex = 0; attachmentIndex < B3D_MAXIMUM_RENDER_TARGET_COUNT; attachmentIndex++)
			{
				const RenderSurfaceMaskBits bit = (RenderSurfaceMaskBits)(RT_COLOR0 << attachmentIndex);
				MTLRenderPassColorAttachmentDescriptor* attachment = clearDescriptor.colorAttachments[attachmentIndex];
				if (!mask.IsSet(bit) || attachment.texture == nil)
					continue;

				const Color& color = mRenderPassClearValues.Colors[attachmentIndex];
				attachment.loadAction = MTLLoadActionClear;
				attachment.clearColor = MTLClearColorMake(color.R, color.G, color.B, color.A);
				hasAttachment = true;
			}

			if (mask.IsSet(RT_DEPTH) && clearDescriptor.depthAttachment.texture != nil)
			{
				clearDescriptor.depthAttachment.loadAction = MTLLoadActionClear;
				clearDescriptor.depthAttachment.clearDepth = mRenderPassClearValues.Depth;
				hasAttachment = true;
			}

			if (mask.IsSet(RT_STENCIL) && clearDescriptor.stencilAttachment.texture != nil)
			{
				clearDescriptor.stencilAttachment.loadAction = MTLLoadActionClear;
				clearDescriptor.stencilAttachment.clearStencil = mRenderPassClearValues.Stencil;
				hasAttachment = true;
			}

			if (!hasAttachment)
				return;

			EnsureEncoderKind(EncoderKind::None);
			ResumeRenderPass(clearDescriptor);
		}

		void MetalGpuCommandBuffer::ClearViewport(RenderSurfaceMask mask)
		{
			EnsureValidThread();
			if (mask == RT_NONE || mRenderEncoder == nil)
				return;

			const bool coversRenderTarget = !mHasViewport ||
				(mViewport.originX == 0.0 && mViewport.originY == 0.0 &&
				mViewport.width == (double)mRenderPassWidth && mViewport.height == (double)mRenderPassHeight);
			if (coversRenderTarget)
			{
				ClearRenderTarget(mask);
				return;
			}

			// Metal only clears whole attachments (through load actions), so a partial clear draws a triangle covering the
			// viewport. Stencil is written by the depth-stencil state's replace operation using the reference value.
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

			// The clear area is defined by the viewport alone, so a scissor left over from earlier draws must not narrow it
			const MTLScissorRect previousScissor = mScissor;
			const bool hadScissor = mHasScissor;

			MTLScissorRect clearScissor;
			clearScissor.x = (NSUInteger)mViewport.originX;
			clearScissor.y = (NSUInteger)mViewport.originY;
			clearScissor.width = (NSUInteger)mViewport.width;
			clearScissor.height = (NSUInteger)mViewport.height;

			[mRenderEncoder setScissorRect:clearScissor];
			[mRenderEncoder setRenderPipelineState:pipelineState];
			[mRenderEncoder setDepthStencilState:depthStencilState];
			[mRenderEncoder setCullMode:MTLCullModeNone];
			[mRenderEncoder setTriangleFillMode:MTLTriangleFillModeFill];
			[mRenderEncoder setDepthBias:0.0f slopeScale:0.0f clamp:0.0f];
			[mRenderEncoder setStencilReferenceValue:mRenderPassClearValues.Stencil];
			[mRenderEncoder setFragmentBytes:&parameters length:sizeof(parameters) atIndex:kMetalClearParametersBufferSlot];
			[mRenderEncoder drawPrimitives:MTLPrimitiveTypeTriangle vertexStart:0 vertexCount:3];

			// The next draw rebinds all other state the clear changed, only the scissor needs restoring
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
				[mRenderEncoder setStencilReferenceValue:value];
		}

		void MetalGpuCommandBuffer::CopyBufferToBuffer(const TShared<GpuBuffer>& source, const TShared<GpuBuffer>& destination, u32 sourceOffset, u32 destinationOffset, u32 length)
		{
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

			EnsureEncoderKind(EncoderKind::Blit);

			MetalBuffer* sourceResource = metalSource->GetMetalResource();
			MetalBuffer* destinationResource = metalDestination->GetMetalResource();
			if (sourceResource != nullptr)
				mResourceTracker.TrackBufferAccess(sourceResource, GpuStageFlag::Transfer, GpuAccessFlag::Read, mBarrierHelper, sourceOffset);

			if (destinationResource != nullptr)
				mResourceTracker.TrackBufferAccess(destinationResource, GpuStageFlag::Transfer, GpuAccessFlag::Write, mBarrierHelper, destinationOffset);

			if (!ExecutePendingBarriers())
				return;

			id<MTLBlitCommandEncoder> blit = GetOrOpenBlitEncoder();
			if (blit == nil)
				return;

			[blit copyFromBuffer:sourceBuffer
				sourceOffset:sourceOffset
				toBuffer:destinationBuffer
				destinationOffset:destinationOffset
				size:length];
		}

		void MetalGpuCommandBuffer::CopyBufferToTexture(const TShared<GpuBuffer>& source, const TShared<Texture>& destination, u32 bufferOffset, u32 mipLevel, u32 arrayLayer)
		{
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

			EnsureEncoderKind(EncoderKind::Blit);

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
				mResourceTracker.TrackBufferAccess(sourceResource, GpuStageFlag::Transfer, GpuAccessFlag::Read, mBarrierHelper, bufferOffset);

			if (destinationResource != nullptr)
				mResourceTracker.TrackImageAccess(destinationResource, destinationRange, GpuImageLayout::TransferDestination, GpuStageFlag::Transfer, GpuAccessFlag::Write, mBarrierHelper);

			if (!ExecutePendingBarriers())
				return;

			id<MTLBlitCommandEncoder> blit = GetOrOpenBlitEncoder();
			if (blit == nil)
				return;

			[blit copyFromBuffer:sourceBuffer
				sourceOffset:bufferOffset
				sourceBytesPerRow:transferInformation.RowPitch
				sourceBytesPerImage:(transferInformation.Depth > 1 ? transferInformation.SlicePitch : 0)
				sourceSize:MTLSizeMake(transferInformation.Width, transferInformation.Height, transferInformation.Depth)
				toTexture:destinationTexture
				destinationSlice:arrayLayer
				destinationLevel:mipLevel
				destinationOrigin:MTLOriginMake(0, 0, 0)
				options:transferInformation.Options];
		}

		void MetalGpuCommandBuffer::CopyTextureToBuffer(const TShared<Texture>& source, const TShared<GpuBuffer>& destination, u32 mipLevel, u32 arrayLayer, u32 bufferOffset)
		{
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

			EnsureEncoderKind(EncoderKind::Blit);

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
				mResourceTracker.TrackImageAccess(sourceResource, sourceRange, GpuImageLayout::TransferSource, GpuStageFlag::Transfer, GpuAccessFlag::Read, mBarrierHelper);

			if (destinationResource != nullptr)
				mResourceTracker.TrackBufferAccess(destinationResource, GpuStageFlag::Transfer, GpuAccessFlag::Write, mBarrierHelper, bufferOffset);

			if (!ExecutePendingBarriers())
				return;

			id<MTLBlitCommandEncoder> blit = GetOrOpenBlitEncoder();
			if (blit == nil)
				return;

			[blit copyFromTexture:sourceTexture
				sourceSlice:arrayLayer
				sourceLevel:mipLevel
				sourceOrigin:MTLOriginMake(0, 0, 0)
				sourceSize:MTLSizeMake(transferInformation.Width, transferInformation.Height, transferInformation.Depth)
				toBuffer:destinationBuffer
				destinationOffset:bufferOffset
				destinationBytesPerRow:transferInformation.RowPitch
				destinationBytesPerImage:(transferInformation.Depth > 1 ? transferInformation.SlicePitch : 0)
				options:transferInformation.Options];
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

			const GpuImageLayout sourceLayout = resolveMultisample ? GpuImageLayout::ResolveSource : GpuImageLayout::TransferSource;
			const GpuImageLayout destinationLayout = resolveMultisample ? GpuImageLayout::ResolveDestination : GpuImageLayout::TransferDestination;

			// Metal resolves through a render pass store action, not a blit
			EnsureEncoderKind(resolveMultisample ? EncoderKind::None : EncoderKind::Blit);

			mResourceTracker.TrackImageAccess(sourceResource, sourceRange, sourceLayout, (resolveMultisample ? GpuStageFlag::Resolve : GpuStageFlag::Transfer), GpuAccessFlag::Read, mBarrierHelper);
			mResourceTracker.TrackImageAccess(destinationResource, destinationRange, destinationLayout, (resolveMultisample ? GpuStageFlag::Resolve : GpuStageFlag::Transfer), GpuAccessFlag::Write, mBarrierHelper);

			if (!ExecutePendingBarriers())
				return false;

			if (resolveMultisample)
			{
				id<MTLCommandBuffer> commandBuffer = GetOrAcquireMetalCommandBuffer();
				if (commandBuffer == nil)
					return false;

				for (u32 faceOffset = 0; faceOffset < copyInformation.FaceCount; faceOffset++)
				{
					MTLRenderPassDescriptor* resolveDescriptor = [MTLRenderPassDescriptor renderPassDescriptor];
					MTLRenderPassColorAttachmentDescriptor* attachment = resolveDescriptor.colorAttachments[0];
					attachment.texture = sourceHandle;
					attachment.level = copyInformation.SourceMip;
					attachment.slice = copyInformation.SourceFace + faceOffset;
					attachment.resolveTexture = destinationHandle;
					attachment.resolveLevel = copyInformation.DestinationMip;
					attachment.resolveSlice = copyInformation.DestinationFace + faceOffset;
					attachment.loadAction = MTLLoadActionLoad;
					attachment.storeAction = MTLStoreActionStoreAndMultisampleResolve;

					mRenderEncoder = [commandBuffer renderCommandEncoderWithDescriptor:resolveDescriptor];
					if (mRenderEncoder == nil)
						return false;

#if B3D_METAL_USE_EXPLICIT_RESOURCE_SYNCHRONIZATION
					WaitForResourceFence(mRenderEncoder);
#endif
					EnsureEncoderKind(EncoderKind::None);
				}
				return true;
			}

			id<MTLBlitCommandEncoder> blit = GetOrOpenBlitEncoder();
			if (blit == nil)
				return false;

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
				[blit copyFromTexture:sourceHandle
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
					[blit copyFromTexture:sourceHandle
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

			id<MTLCounterSampleBuffer> counterBuffer = metalPool->GetCounterBuffer();
			if (counterBuffer == nil)
				return;

			if (mRenderEncoder != nil && mGpuDevice.SupportsRenderEncoderTimestamps())
				[mRenderEncoder sampleCountersInBuffer:counterBuffer atSampleIndex:query.Id withBarrier:YES];
			else if (mComputeEncoder != nil && mGpuDevice.SupportsComputeEncoderTimestamps())
				[mComputeEncoder sampleCountersInBuffer:counterBuffer atSampleIndex:query.Id withBarrier:YES];
			else
			{
				id<MTLBlitCommandEncoder> blitEncoder = mBlitEncoder;
				if (blitEncoder == nil && mRenderEncoder == nil && mComputeEncoder == nil)
					blitEncoder = GetOrOpenBlitEncoder();

				if (blitEncoder == nil || !mGpuDevice.SupportsBlitEncoderTimestamps())
				{
					B3D_LOG(Error, LogRenderBackend,
						"WriteTimestamp cannot sample the active Metal encoder on this device.");
					return;
				}

				[blitEncoder sampleCountersInBuffer:counterBuffer atSampleIndex:query.Id withBarrier:YES];
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

			if (!ActivateOcclusionQueryPool(metalPool))
				return;

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
			if (!metalPool->IsQueryAllocated(query) || metalPool.get() != mActiveOcclusionQueryPool.get()
				|| mVisibilityMode == MTLVisibilityResultModeDisabled
				|| mVisibilityOffset != metalPool->GetQueryOffset(query))
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
			if (id<MTLBuffer> visibilityBuffer = metalPool->GetVisibilityBuffer())
			{
				id<MTLBlitCommandEncoder> blitEncoder = GetOrOpenBlitEncoder();
				if (blitEncoder == nil)
				{
					B3D_LOG(Error, LogRenderBackend, "Failed to encode a Metal occlusion-query reset.");
					return;
				}

				[blitEncoder fillBuffer:visibilityBuffer range:NSMakeRange(0, [visibilityBuffer length]) value:0];
				AddUniqueUsedQueryPool(metalPool);
			}

			metalPool->ResetAllocation();
		}

		void MetalGpuCommandBuffer::BeginLabel(const StringView& name)
		{
#if B3D_BUILD_TYPE_DEVELOPMENT
			EnsureValidThread();
			if (name.empty())
				return;

			id<MTLCommandBuffer> commandBuffer = GetOrAcquireMetalCommandBuffer();
			if (commandBuffer == nil)
				return;

			NSString* label = [[NSString alloc] initWithBytes:name.data() length:name.size() encoding:NSUTF8StringEncoding];
			[commandBuffer pushDebugGroup:label];
			mDebugGroupDepth++;
#else
			(void)name;
#endif
		}

		void MetalGpuCommandBuffer::EndLabel()
		{
#if B3D_BUILD_TYPE_DEVELOPMENT
			EnsureValidThread();
			if (mCommandBuffer == nil || mDebugGroupDepth == 0)
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
			id<MTLCommandEncoder> encoder = GetActiveEncoder();
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
#if B3D_BUILD_TYPE_DEVELOPMENT
			while (mCommandBuffer != nil && mDebugGroupDepth > 0)
			{
				[mCommandBuffer popDebugGroup];
				mDebugGroupDepth--;
			}
#endif

			// Done is only set once the GPU finishes executing the buffer, see ExecuteSubmitOnSubmitThread()
			EnsureEncoderKind(EncoderKind::None);
			mState = GpuCommandBufferState::RecordingDone;
		}

		void MetalGpuCommandBuffer::IssueBarriers(const GpuBarriers& barriers)
		{
			EnsureValidThread();
			if (mRecordingFailed)
				return;

			for (const GpuBufferBarrier& bufferBarrier : barriers.BufferBarriers)
			{
				auto* metalGpuBuffer = static_cast<MetalGpuBuffer*>(bufferBarrier.Object.get());
				if (metalGpuBuffer == nullptr)
					continue;

				MetalBuffer* resource = metalGpuBuffer->GetMetalResource();
				if (resource == nullptr)
					continue;

				mResourceTracker.TrackExplicitBufferBarrier(resource, GpuBackendUtility::GetStageFlags(bufferBarrier.DestinationUsage), bufferBarrier.DestinationAccess, mBarrierHelper, bufferBarrier.AliasAcquire);
			}

			for (const GpuTextureBarrier& textureBarrier : barriers.TextureBarriers)
			{
				auto* metalTexture = static_cast<MetalTexture*>(textureBarrier.Object.get());
				if (metalTexture == nullptr)
					continue;

				MetalImage* resource = metalTexture->GetMetalResource();
				if (resource == nullptr)
					continue;

				mResourceTracker.TrackExplicitImageBarrier(resource, textureBarrier.SubresourceRange, GpuBackendUtility::GetStageFlags(textureBarrier.DestinationUsage), textureBarrier.DestinationAccess, textureBarrier.DestinationLayout, mBarrierHelper, textureBarrier.AliasAcquire);
			}

			for (const GpuRenderTargetBarrier& renderTargetBarrier : barriers.RenderTargetBarriers)
				B3D_ENSURE_LOG(renderTargetBarrier.AliasAcquire == nullptr, "Render target barriers cannot alias acquire. Use a texture barrier instead.");

			// Metal has no framebuffer to resolve the surface mask against, so an active render pass is split instead
			const bool hasRenderTargetBarriers = !barriers.RenderTargetBarriers.IsEmpty();
#if B3D_METAL_USE_EXPLICIT_RESOURCE_SYNCHRONIZATION
			if (hasRenderTargetBarriers && mRenderEncoder != nil)
			{
				if (!RestartRenderPassForBarrier())
					return;

				mBarrierHelper.Execute(nil, nil);
				return;
			}
#else
			(void)hasRenderTargetBarriers;
#endif

			if (!ExecutePendingBarriers())
				return;
		}

		void MetalGpuCommandBuffer::EncodeQueueWaits(id<MTLCommandBuffer> commandBuffer, MetalGpuQueue& submitQueue, GpuQueueMask syncMask)
		{
			const GpuQueueMask selfMask = GpuQueueId(submitQueue.GetType(), submitQueue.GetIndex());
			GpuQueueMask waitMask = syncMask & ~selfMask;
#if B3D_METAL_USE_EXPLICIT_RESOURCE_SYNCHRONIZATION
			// Untracked resources also need ordering between command buffers on the same queue.
			// Waiting for the queue's prior signal is conservative but complete.
			waitMask |= selfMask;
#endif
			if (waitMask.IsEmpty())
				return;

			for (u32 queueTypeIndex = 0; queueTypeIndex < GQT_COUNT; queueTypeIndex++)
			{
				const GpuQueueType queueType = (GpuQueueType)queueTypeIndex;
				const u32 queueCount = mGpuDevice.GetQueueCount(queueType);
				for (u32 queueIndex = 0; queueIndex < queueCount; queueIndex++)
				{
					const GpuQueueId waitQueueId(queueType, queueIndex);
					if (!waitMask.IsSet(waitQueueId))
						continue;

					auto waitQueue = std::static_pointer_cast<MetalGpuQueue>(mGpuDevice.GetQueue(queueType, queueIndex));
					if (!waitQueue)
						continue;

					id<MTLSharedEvent> waitEvent = waitQueue->GetSharedEvent();
					// Not the reserved value, which may belong to a submission that isn't committed yet and would never signal
					const u64 waitValue = waitQueue->GetLastCommittedEventValue();
					if (waitEvent != nil && waitValue != 0)
						[commandBuffer encodeWaitForEvent:waitEvent value:waitValue];
				}
			}
		}

		void MetalGpuCommandBuffer::EncodeFrameFenceWaits(id<MTLCommandBuffer> commandBuffer, MetalGpuQueue& submitQueue, GpuQueueMask syncMask,
			TArrayView<const u64> frameFenceValues)
		{
			if (frameFenceValues.IsEmpty())
				return;

			// Frame fence values are committed event values, so waiting on them cannot deadlock. Waiting on this queue's own
			// value also orders the frame after its earlier work when resources are untracked.
			const GpuQueueId submitQueueId = submitQueue.GetId();
			mGpuDevice.DoForEachQueue([commandBuffer, submitQueueId, syncMask, frameFenceValues](GpuQueue& queue)
			{
				const GpuQueueId queueId = queue.GetId();
				const u64 waitValue = frameFenceValues[queueId.Id];
				if (waitValue == 0)
					return;

				if (queueId.Id != submitQueueId.Id && syncMask.IsSet(queueId))
					return;

				id<MTLSharedEvent> waitEvent = static_cast<MetalGpuQueue&>(queue).GetSharedEvent();
				if (waitEvent != nil)
					[commandBuffer encodeWaitForEvent:waitEvent value:waitValue];
			});
		}

		u64 MetalGpuCommandBuffer::EncodeQueueSignal(id<MTLCommandBuffer> commandBuffer, MetalGpuQueue& submitQueue)
		{
			// Completion handlers run after the signal, so CPU waiters on the event unblock before OnDidComplete fires
			const u64 signalValue = submitQueue.ReserveNextEventValue();
			id<MTLSharedEvent> signalEvent = submitQueue.GetSharedEvent();
			if (signalEvent != nil)
				[commandBuffer encodeSignalEvent:signalEvent value:signalValue];

			return signalValue;
		}

		void MetalGpuCommandBuffer::ExecuteSubmitOnSubmitThread(MetalGpuQueue& submitQueue, GpuQueueMask syncMask, TArrayView<const GpuTimelineFenceAndValue> signalFences)
		{
			// The owner thread already released its recording state (NotifyWillQueueForSubmit()), so only the native encoding state and
			// mUsedQueryPools may be touched here. mQueueSyncMask is already folded into @p syncMask.
			AssertIfNotSubmitThread();

			mSubmittedQueueId = submitQueue.GetId();

			// Does not change mState: the queue sets Executing and the completion handler sets Done
			EnsureEncoderKind(EncoderKind::None);

			auto fnPostFailedSubmissionCompletion = [this, &submitQueue]()
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
				mCommandBuffer = nil;

				TShared<GpuCommandBuffer> selfShared = GetShared();
				TShared<WaitGroup> ownerCompletion = B3DMakeShared<WaitGroup>(1);
				submitQueue.NotifySubmissionFailed(ownerCompletion);
				mPool.GetMessageQueue().PostCommand([selfShared, ownerCompletion]()
				{
					auto* owner = static_cast<MetalGpuCommandBuffer*>(selfShared.get());
					owner->mState = GpuCommandBufferState::Done;
					owner->OnDidComplete();
					owner->ClearRecordingState();
					owner->mPool.NotifyCommandBufferReady(owner->mId);
					ownerCompletion->NotifyDone();
				}, "MetalGpuCommandBuffer failed submission");
			};

			if (mRecordingFailed)
			{
				fnPostFailedSubmissionCompletion();
				return;
			}

			// Nothing was recorded, but the submission still goes through the queue's event. Otherwise its cross-queue
			// waits are skipped, and queues waiting for this queue's next value deadlock.
			if (mCommandBuffer == nil)
			{
				for (const TShared<MetalGpuQueryPool>& pool : mUsedQueryPools)
					pool->MarkSubmissionFailed();

				mUsedQueryPools.clear();
				mQueryPoolsQueuedForSubmission = false;

				id<MTLCommandQueue> metalCommandQueue = submitQueue.GetMetalQueue();
				id<MTLCommandBuffer> emptyCommandBuffer = metalCommandQueue ? [metalCommandQueue commandBuffer] : nil;
				if (emptyCommandBuffer == nil)
				{
					B3D_LOG(Fatal, LogRenderBackend, "Failed to allocate an empty Metal submission command buffer.");
					fnPostFailedSubmissionCompletion();
					return;
				}

				GpuSubmitThread& submitThread = mGpuDevice.GetSubmitThread();
				MetalSubmissionTransitionVisitor transitionVisitor;
				mResourceTracker.ResolveSubmissionTransitions(mSubmittedQueueId, submitThread.GetFrameIndex(), transitionVisitor);
				syncMask |= transitionVisitor.GetRequiredWaitMask();

				mResourceTracker.NotifyUsed(mSubmittedQueueId);
				mResourcesSubmitted = true;

				// The resolved transitions rely on the frame fence ordering this submission after all earlier frames
				EncodeQueueWaits(emptyCommandBuffer, submitQueue, syncMask);
				EncodeFrameFenceWaits(emptyCommandBuffer, submitQueue, syncMask, submitThread.ConsumeFrameFence(submitQueue));
				const u64 signalValue = EncodeQueueSignal(emptyCommandBuffer, submitQueue);
				EncodeUserFenceSignals(emptyCommandBuffer, signalFences);

				TShared<GpuCommandBuffer> selfShared = GetShared();
				TShared<WaitGroup> ownerCompletion = B3DMakeShared<WaitGroup>(1);
				[emptyCommandBuffer addCompletedHandler:^(id<MTLCommandBuffer> completedBuffer)
				{
					LogCommandBufferError(completedBuffer);
					auto* metalSelf = static_cast<MetalGpuCommandBuffer*>(selfShared.get());
					metalSelf->mPool.GetMessageQueue().PostCommand([selfShared, ownerCompletion]()
					{
						auto* owner = static_cast<MetalGpuCommandBuffer*>(selfShared.get());
						owner->mState = GpuCommandBufferState::Done;
						owner->mPool.NotifyCommandBufferReady(owner->mId);
						owner->OnDidComplete();
						owner->ClearRecordingState();
						ownerCompletion->NotifyDone();
					}, "MetalGpuCommandBuffer empty completion");
				}];

				[emptyCommandBuffer commit];

				// Must follow the commit, see the recorded path below
				submitQueue.NotifySubmissionCommitted(signalValue, emptyCommandBuffer, ownerCompletion);

				mQueueSyncMask = GpuQueueMask();
				return;
			}

			id<MTLCommandBuffer> commandBuffer = mCommandBuffer;

			// Waits appended to recorded work would execute after it, so they go into a prologue command buffer committed
			// just before. Command buffers on one MTLCommandQueue execute in order.
			const GpuQueueMask selfMask = GpuQueueId(submitQueue.GetType(), submitQueue.GetIndex());
			bool needsWaitPrologue = !(syncMask & ~selfMask).IsEmpty();
#if B3D_METAL_USE_EXPLICIT_RESOURCE_SYNCHRONIZATION
			needsWaitPrologue |= submitQueue.GetLastCommittedEventValue() != 0;
#endif
			// Resolving the transitions below publishes their state, so the prologue must be allocated before, in case
			// they add waits
			GpuSubmitThread& submitThread = mGpuDevice.GetSubmitThread();
			const bool mayNeedResourceWait = !mResourceTracker.GetBuffers().empty() || !mResourceTracker.GetImages().empty();
			const bool needsFrameFenceWait = submitThread.IsFrameFencePending(submitQueue);
			id<MTLCommandBuffer> waitCommandBuffer = nil;
			if (needsWaitPrologue || mayNeedResourceWait || needsFrameFenceWait)
			{
				id<MTLCommandQueue> metalCommandQueue = submitQueue.GetMetalQueue();
				waitCommandBuffer = metalCommandQueue ? [metalCommandQueue commandBuffer] : nil;
				if (waitCommandBuffer == nil)
				{
					B3D_LOG(Fatal, LogRenderBackend, "Failed to allocate Metal cross-queue wait command buffer.");
					fnPostFailedSubmissionCompletion();
					return;
				}
			}

			MetalSubmissionTransitionVisitor transitionVisitor;
			mResourceTracker.ResolveSubmissionTransitions(mSubmittedQueueId, submitThread.GetFrameIndex(), transitionVisitor);
			syncMask |= transitionVisitor.GetRequiredWaitMask();

			// The resolved transitions rely on the frame fence ordering this submission after all earlier frames
			const TArrayView<const u64> frameFenceValues = submitThread.ConsumeFrameFence(submitQueue);

			needsWaitPrologue = !(syncMask & ~selfMask).IsEmpty() || !frameFenceValues.IsEmpty();
#if B3D_METAL_USE_EXPLICIT_RESOURCE_SYNCHRONIZATION
			needsWaitPrologue |= submitQueue.GetLastCommittedEventValue() != 0;
#endif

			if (needsWaitPrologue)
			{
				B3D_ASSERT(waitCommandBuffer != nil);
				EncodeQueueWaits(waitCommandBuffer, submitQueue, syncMask);
				EncodeFrameFenceWaits(waitCommandBuffer, submitQueue, syncMask, frameFenceValues);
				[waitCommandBuffer addCompletedHandler:^(id<MTLCommandBuffer> completedBuffer)
				{
					LogCommandBufferError(completedBuffer);
				}];
				[waitCommandBuffer commit];
			}

			mResourceTracker.NotifyUsed(mSubmittedQueueId);
			mResourcesSubmitted = true;

			const u64 signalValue = EncodeQueueSignal(commandBuffer, submitQueue);

			EncodeUserFenceSignals(commandBuffer, signalFences);

			// Keeps the command buffer alive until the GPU finishes, then reports completion on the owner thread
			TShared<GpuCommandBuffer> selfShared = GetShared();
			TShared<WaitGroup> ownerCompletion = B3DMakeShared<WaitGroup>(1);
			[commandBuffer addCompletedHandler:^(id<MTLCommandBuffer> completedBuffer)
			{
				LogCommandBufferError(completedBuffer);
				auto* metalSelf = static_cast<MetalGpuCommandBuffer*>(selfShared.get());
				metalSelf->mPool.GetMessageQueue().PostCommand([selfShared, ownerCompletion]()
				{
					auto* owner = static_cast<MetalGpuCommandBuffer*>(selfShared.get());
					owner->mState = GpuCommandBufferState::Done;
					owner->mPool.NotifyCommandBufferReady(owner->mId);
					owner->OnDidComplete();
					// Listener closures may hold the last references to resources used by this submission
					owner->ClearRecordingState();
					ownerCompletion->NotifyDone();
				}, "MetalGpuCommandBuffer completion");
			}];

			[commandBuffer commit];
			mCommandBuffer = nil;

			// Must follow the commit, otherwise other queues could wait on a value that is never signaled
			submitQueue.NotifySubmissionCommitted(signalValue, commandBuffer, ownerCompletion);

			mQueueSyncMask = GpuQueueMask();

			// Also after the commit, so a query resolve never waits on a value from an uncommitted submission
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
			mActiveOcclusionQueryPool.reset();
			ResetRenderResidencyCaches();
			ResetComputeResidencyCaches();
			ResetArgumentTableBindings();
		}

		void MetalGpuCommandBuffer::ClearRecordingState()
		{
			// Same as VulkanGpuCommandBuffer::ClearRecordingState(). Resources of a buffer that failed before submission
			// were never marked used, so they are only unbound.
			if (mResourcesSubmitted)
				mResourceTracker.NotifyDone(mSubmittedQueueId);
			else
				mResourceTracker.NotifyUnbound();

			mResourceTracker.Clear();
			mResourcesSubmitted = false;
			mRecordingFailed = false;
			mBarrierHelper.Clear();

			mQueueSyncMask = GpuQueueMask();

			// Listener closures may hold the last references to transient resources, which must be released before
			// their allocators are reclaimed
			OnDidComplete.Clear();
			OnDestroyed.Clear();

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
			mActiveOcclusionQueryPool.reset();
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
			mRestartRenderPassDescriptor = nil;
			mVertexBufferBindings.clear();
			mBoundVertexBuffers.Clear();
			mHasViewport = false;
			mNormalizedViewport = Area2(0.0f, 0.0f, 1.0f, 1.0f);
			mHasScissor = false;
			mDebugGroupDepth = 0;
			mVisibilityMode = MTLVisibilityResultModeDisabled;
			mVisibilityOffset = 0;
	#if B3D_METAL_USE_EXPLICIT_RESOURCE_SYNCHRONIZATION
			mFenceNeedsWait = false;
	#endif
			ResetRenderResidencyCaches();
			ResetComputeResidencyCaches();
			ResetArgumentTableBindings();
		}

		void MetalGpuCommandBuffer::NotifyParentPoolReset()
		{
			// The native command buffer is single-use and was already released at commit, so this is bookkeeping only.
			// GpuCommandBufferPool::Reset() may only be called once all its command buffers have finished executing.
			if (!B3D_ENSURE(mState == GpuCommandBufferState::Done || mState == GpuCommandBufferState::Ready))
				return;

			if (mState == GpuCommandBufferState::Done)
				ClearRecordingState();

			mState = GpuCommandBufferState::Ready;
		}

		void MetalGpuCommandBuffer::Destroy()
		{
			if (IsDestroyed())
				return;

			// Also clears OnDestroyed, so the base destructor doesn't trigger it
			ClearRecordingState();

			CloseAllEncoders();
			mCommandBuffer = nil;

			GpuCommandBuffer::Destroy();
		}
	} // namespace render
} // namespace b3d
