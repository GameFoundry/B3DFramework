//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DMetalGpuCommandBuffer.h"
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
#include "B3DMetalRenderWindowSurface.h"
#include "B3DMetalUtility.h"
#include "B3DIMetalRenderWindowSurface.h"
#include "GpuBackend/B3DRenderWindow.h"
#include "GpuBackend/B3DGpuSubmitThread.h"
#include "GpuBackend/B3DGpuVertexInputManager.h"
#include "B3DMetalVertexInputManager.h"
#include "Image/B3DPixelUtility.h"
#include "Debug/B3DLog.h"
#include <atomic>

// TODO(C14): migrate the two warn-once flags below (and the matching site in B3DMetalUtility.mm)
// to a shared @c LogOnce(Level, Category, ...) helper once its home is decided — see TODO in
// B3DMetalUtility.mm for context.

namespace b3d
{
	namespace render
	{
		struct MetalGpuCommandBuffer::Impl
		{
			id<MTLCommandBuffer> CommandBuffer = nil;
			id<MTLRenderCommandEncoder> RenderEncoder = nil;
			id<MTLComputeCommandEncoder> ComputeEncoder = nil;
			id<MTLBlitCommandEncoder> BlitEncoder = nil;
			struct PendingEventSignal
			{
				id<MTLSharedEvent> Event = nil;
				u64 Value = 0;
			};
			Vector<PendingEventSignal> PendingEventSignals;
			struct VertexBufferBinding
			{
				id<MTLBuffer> Buffer = nil;
				NSUInteger Offset = 0;
				NSUInteger Index = 0;
			};

			MTLRenderPassDescriptor* RestartRenderPassDescriptor = nil;
			Vector<VertexBufferBinding> VertexBufferBindings;
			MTLViewport Viewport = {};
			MTLScissorRect Scissor = {};
			MTLVisibilityResultMode VisibilityMode = MTLVisibilityResultModeDisabled;
			NSUInteger VisibilityOffset = 0;
			bool HasViewport = false;
			bool HasScissor = false;
			u32 DebugGroupDepth = 0;
	#if B3D_METAL_USE_EXPLICIT_RESOURCE_SYNCHRONIZATION
			id<MTLFence> ResourceFence = nil;
			bool FenceNeedsWait = false;
	#endif
		};

		namespace
		{
#if B3D_METAL_USE_EXPLICIT_RESOURCE_SYNCHRONIZATION
			void UpdateResourceFence(MetalGpuCommandBuffer::Impl& impl, id<MTLRenderCommandEncoder> encoder)
			{
				if (impl.ResourceFence == nil || encoder == nil)
					return;

				[encoder updateFence:impl.ResourceFence afterStages:MTLRenderStageFragment];
				impl.FenceNeedsWait = true;
			}

			void UpdateResourceFence(MetalGpuCommandBuffer::Impl& impl, id<MTLComputeCommandEncoder> encoder)
			{
				if (impl.ResourceFence == nil || encoder == nil)
					return;

				[encoder updateFence:impl.ResourceFence];
				impl.FenceNeedsWait = true;
			}

			void UpdateResourceFence(MetalGpuCommandBuffer::Impl& impl, id<MTLBlitCommandEncoder> encoder)
			{
				if (impl.ResourceFence == nil || encoder == nil)
					return;

				[encoder updateFence:impl.ResourceFence];
				impl.FenceNeedsWait = true;
			}

			void WaitForResourceFence(MetalGpuCommandBuffer::Impl& impl, id<MTLRenderCommandEncoder> encoder)
			{
				if (!impl.FenceNeedsWait || impl.ResourceFence == nil || encoder == nil)
					return;

				[encoder waitForFence:impl.ResourceFence beforeStages:MTLRenderStageVertex];
				impl.FenceNeedsWait = false;
			}

			void WaitForResourceFence(MetalGpuCommandBuffer::Impl& impl, id<MTLComputeCommandEncoder> encoder)
			{
				if (!impl.FenceNeedsWait || impl.ResourceFence == nil || encoder == nil)
					return;

				[encoder waitForFence:impl.ResourceFence];
				impl.FenceNeedsWait = false;
			}

			void WaitForResourceFence(MetalGpuCommandBuffer::Impl& impl, id<MTLBlitCommandEncoder> encoder)
			{
				if (!impl.FenceNeedsWait || impl.ResourceFence == nil || encoder == nil)
					return;

				[encoder waitForFence:impl.ResourceFence];
				impl.FenceNeedsWait = false;
			}
#endif

			/** Reports asynchronous Metal execution failures from a command-buffer completion handler. */
			void LogCommandBufferError(id<MTLCommandBuffer> commandBuffer)
			{
				if ([commandBuffer status] != MTLCommandBufferStatusError)
					return;

				NSError* error = [commandBuffer error];
				NSString* label = [commandBuffer label];
#if B3D_BUILD_TYPE_DEVELOPMENT
				NSArray<id<MTLCommandBufferEncoderInfo>>* encoderInformation =
					error ? [[error userInfo] objectForKey:MTLCommandBufferEncoderInfoErrorKey] : nil;
				for (id<MTLCommandBufferEncoderInfo> encoderInfo in encoderInformation)
				{
					B3D_LOG(Error, LogRenderBackend,
						"Metal encoder '{0}' failed with execution state {1}.",
						[encoderInfo label] ? String([[encoderInfo label] UTF8String]) : String("<unnamed>"),
						(u32)[encoderInfo errorState]);
					for (NSString* signpost in [encoderInfo debugSignposts])
						B3D_LOG(Error, LogRenderBackend, "  GPU signpost: {0}", String([signpost UTF8String]));
				}
#endif
				B3D_LOG(Fatal, LogRenderBackend,
					"Metal command buffer '{0}' failed during GPU execution ({1}, code {2}): {3}",
					label ? String([label UTF8String]) : String("<unnamed>"),
					error ? String([[error domain] UTF8String]) : String("<unknown domain>"),
					error ? (i64)[error code] : 0,
					error ? String([[error localizedDescription] UTF8String]) : String("No error details were provided."));
			}

			/**
			 * Attaches a parameter set's argument buffer to a render encoder at the correct set index.
			 * The layout's cached combined-stage mask lets us skip the per-binding walk the old
			 * version had to do (B7).
			 */
			void AttachArgumentBufferToRenderEncoder(id<MTLRenderCommandEncoder> encoder, MetalGpuParameters& params)
				{
					const MetalGpuPipelineParameterSetLayout* layout = params.GetMetalLayout();
					id<MTLBuffer> argumentBuffer = params.GetArgumentBuffer();
					if (!layout || argumentBuffer == nil || params.GetSet() >= kMetalVertexBufferSlotBase)
						return;

					const u32 stageMask = layout->GetCombinedStageMask();
					if (stageMask & ((u32)GpuProgramStageBit::Vertex | (u32)GpuProgramStageBit::Hull
						| (u32)GpuProgramStageBit::Domain))
						[encoder setVertexBuffer:argumentBuffer offset:(NSUInteger)params.GetArgumentBufferOffset()
							atIndex:params.GetSet()];
					if (stageMask & (u32)GpuProgramStageBit::Fragment)
						[encoder setFragmentBuffer:argumentBuffer offset:(NSUInteger)params.GetArgumentBufferOffset()
							atIndex:params.GetSet()];
				}

			/** Compute-encoder counterpart of @c AttachArgumentBufferToRenderEncoder. No stage axis. */
			void AttachArgumentBufferToComputeEncoder(id<MTLComputeCommandEncoder> encoder, MetalGpuParameters& params)
				{
					const MetalGpuPipelineParameterSetLayout* layout = params.GetMetalLayout();
					id<MTLBuffer> argumentBuffer = params.GetArgumentBuffer();
					if (!layout || argumentBuffer == nil || params.GetSet() >= kMetalVertexBufferSlotBase
						|| (layout->GetCombinedStageMask() & (u32)GpuProgramStageBit::Compute) == 0)
						return;

					[encoder setBuffer:argumentBuffer offset:(NSUInteger)params.GetArgumentBufferOffset()
						atIndex:params.GetSet()];
				}

			/**
			 * Shared core of @c EmitResidencyFor{Render,Compute}Encoder. Iterates over @p buckets,
			 * gathers the B6-cached @c id<MTLResource> handles per (usage, stages) bucket into a
			 * stack array (spilling to a heap Vector only on the rare big-set path), and delegates
			 * the actual @c useResources: emission to @p fnEmit so the render / compute variants
			 * keep their slightly different Metal call shapes.
			 *
			 * B2: bucket iteration collapses what used to be N @c useResource: calls into @<= 4
			 * plural calls (one per non-empty bucket). The bucket layout is computed once at
			 * @c MetalGpuPipelineParameterSetLayout construction time (see @c GetRenderBuckets /
			 * @c GetComputeBuckets) so the draw/dispatch hot path is a pure gather + Obj-C call.
			 *
			 * Contract: the caller must have committed pending bindings first (via
			 * @c MetalGpuParameters::CommitPendingBindings) so the B6 handle cache is populated.
			 */
			template <typename BucketContainer, typename FnEmit>
			void EmitResidencyImpl(MetalGpuParameters& params, const BucketContainer& buckets, FnEmit&& fnEmit)
			{
				const MetalGpuPipelineParameterSetLayout* layout = params.GetMetalLayout();
				if (!layout)
					return;

				for (const auto& bucket : buckets)
				{
					if (bucket.ResourceIndices.Empty())
						continue;

					// Small-N (typically @<= 8 resources per bucket). Stack array keeps this hot path
					// allocation-free; the @c kMaxStackResources cap covers the vast majority of
					// real-world sets. Anything over that (bindless-style huge sets) falls back to a
					// scratch @c Vector.
					constexpr u32 kMaxStackResources = 32;
					__unsafe_unretained id<MTLResource> stack[kMaxStackResources];
					Vector<__unsafe_unretained id<MTLResource>> spill;

					__unsafe_unretained id<MTLResource>* resources = stack;
					if (bucket.ResourceIndices.Size() > kMaxStackResources)
					{
						spill.resize((size_t)bucket.ResourceIndices.Size());
						resources = spill.data();
					}

					u32 resourceCount = 0;
					for (u32 resourceIndex : bucket.ResourceIndices)
					{
						// Read the resolved resource from the parameter set's dense cache.
						// Cache is populated by @c CommitPendingBindings; slots whose value changed
						// since the last commit have had their handle refreshed already.
						id<MTLResource> resource = params.GetCachedResource(resourceIndex);
						if (resource == nil)
							continue;
						resources[resourceCount++] = resource;
					}

					if (resourceCount > 0)
						fnEmit(bucket, resources, resourceCount);
				}
			}

			/** Emits @c useResources:count:usage:stages: once per precomputed render bucket. */
			void EmitResidencyForRenderEncoder(id<MTLRenderCommandEncoder> encoder, MetalGpuParameters& params)
			{
				const MetalGpuPipelineParameterSetLayout* layout = params.GetMetalLayout();
				if (!layout)
					return;

				EmitResidencyImpl(params, layout->GetRenderBuckets(),
					[encoder](const auto& bucket, __unsafe_unretained id<MTLResource>* resources, u32 resourceCount)
					{
						[encoder useResources:resources count:resourceCount usage:bucket.Usage stages:bucket.RenderStages];
					});
			}

			/** Compute-encoder counterpart of @c EmitResidencyForRenderEncoder. No stage mask axis. */
			void EmitResidencyForComputeEncoder(id<MTLComputeCommandEncoder> encoder, MetalGpuParameters& params)
			{
				const MetalGpuPipelineParameterSetLayout* layout = params.GetMetalLayout();
				if (!layout)
					return;

				EmitResidencyImpl(params, layout->GetComputeBuckets(),
					[encoder](const auto& bucket, __unsafe_unretained id<MTLResource>* resources, u32 resourceCount)
					{
						[encoder useResources:resources count:resourceCount usage:bucket.Usage];
					});
			}

			GpuResourceUseFlags GetBindingUseFlags(const MetalGpuPipelineParameterSetLayout& layout, GpuParameterType type, u32 slot, bool compute)
			{
				GpuResourceUseFlags useFlags = type == GpuParameterType::UniformBuffer
					? GpuResourceUseFlag::UniformBuffer
					: GpuResourceUseFlag::ShaderAccess;

				for (const MetalArgumentBufferBinding& binding : layout.GetBindings())
				{
					if (binding.Type != type || binding.Slot != slot)
						continue;

					if (compute)
					{
						if ((binding.StageMask & (u32)GpuProgramStageBit::Compute) == 0)
							return GpuResourceUseFlag::Undefined;

						useFlags |= GpuResourceUseFlag::StageComputeShader;
					}
					else
					{
						if (binding.StageMask & ((u32)GpuProgramStageBit::Vertex | (u32)GpuProgramStageBit::Hull | (u32)GpuProgramStageBit::Domain))
							useFlags |= GpuResourceUseFlag::StageVertexShader;
						if (binding.StageMask & (u32)GpuProgramStageBit::Fragment)
							useFlags |= GpuResourceUseFlag::StageFragmentShader;
						if (!useFlags.IsSetAny(GpuResourceUseFlag::StageVertexShader | GpuResourceUseFlag::StageFragmentShader))
							return GpuResourceUseFlag::Undefined;
					}

					return useFlags;
				}

				return GpuResourceUseFlag::Undefined;
			}

			GpuAccessFlags GetStorageBufferAccessFlags(const MetalGpuPipelineParameterSetLayout& layout, u32 slot)
			{
				for (const MetalArgumentBufferBinding& binding : layout.GetBindings())
				{
					if (binding.Type != GpuParameterType::StorageBuffer || binding.Slot != slot)
						continue;

					return binding.ObjectType == GPOT_RWSTRUCTURED_BUFFER
						? (GpuAccessFlag::Read | GpuAccessFlag::Write)
						: GpuAccessFlag::Read;
				}

				return GpuAccessFlag::Read;
			}

			GpuTextureSubresourceRange GetTextureRange(MetalImage& image, const TextureSurface& surface)
			{
				const GpuTextureSubresourceRange& fullRange = image.GetRange();
				GpuTextureSubresourceRange range;
				range.BaseArrayLayer = Math::Min(surface.Face, fullRange.ArrayLayerCount - 1);
				range.ArrayLayerCount = Math::Min(surface.FaceCount == 0 ? fullRange.ArrayLayerCount : surface.FaceCount,
					fullRange.ArrayLayerCount - range.BaseArrayLayer);
				range.BaseMipLevel = Math::Min(surface.MipLevel, fullRange.MipLevelCount - 1);
				range.MipLevelCount = Math::Min(surface.MipLevelCount == 0 ? fullRange.MipLevelCount : surface.MipLevelCount,
					fullRange.MipLevelCount - range.BaseMipLevel);
				range.AspectMask = fullRange.AspectMask;
				return range;
			}

			void TrackParameterResources(MetalGpuParameters& parameters, bool compute, MetalResourceTracker& tracker, MetalBarrierHelper& barrierHelper)
			{
				const MetalGpuPipelineParameterSetLayout* layout = parameters.GetMetalLayout();
				if (layout == nullptr)
					return;

				for (const MetalGpuParameters::UniformBufferBinding& binding : parameters.GetUniformBuffers())
				{
					auto buffer = std::static_pointer_cast<MetalGpuBuffer>(binding.Buffer);
					MetalBuffer* resource = buffer ? buffer->GetMetalResource() : nullptr;
					const GpuResourceUseFlags useFlags = GetBindingUseFlags(*layout, GpuParameterType::UniformBuffer, binding.Slot, compute);
					if (resource != nullptr && useFlags != GpuResourceUseFlag::Undefined)
						tracker.TrackBufferUsage(resource, useFlags, GpuAccessFlag::Read, barrierHelper, binding.Offset);
				}

				for (const MetalGpuParameters::StorageBufferBinding& binding : parameters.GetStorageBuffers())
				{
					auto buffer = std::static_pointer_cast<MetalGpuBuffer>(binding.Buffer);
					MetalBuffer* resource = buffer ? buffer->GetMetalResource() : nullptr;
					const GpuResourceUseFlags useFlags = GetBindingUseFlags(*layout, GpuParameterType::StorageBuffer, binding.Slot, compute);
					if (resource != nullptr && useFlags != GpuResourceUseFlag::Undefined)
						tracker.TrackBufferUsage(resource, useFlags, GetStorageBufferAccessFlags(*layout, binding.Slot), barrierHelper, binding.View.Offset);
				}

				auto fnTrackTextures = [&](const Vector<MetalGpuParameters::TextureBinding>& bindings, GpuParameterType type, GpuAccessFlags access)
				{
					for (const MetalGpuParameters::TextureBinding& binding : bindings)
					{
						auto texture = std::static_pointer_cast<MetalTexture>(binding.Texture);
						MetalImage* resource = texture ? texture->GetMetalResource() : nullptr;
						const GpuResourceUseFlags useFlags = GetBindingUseFlags(*layout, type, binding.Slot, compute);
						if (resource == nullptr || useFlags == GpuResourceUseFlag::Undefined)
							continue;

						const GpuTextureSubresourceRange range = GetTextureRange(*resource, binding.Surface);
						const GpuImageLayout imageLayout = type == GpuParameterType::StorageTexture ? GpuImageLayout::General : GpuImageLayout::ShaderReadOnly;
						tracker.TrackImageUsage(resource, range, imageLayout, imageLayout, useFlags, access, barrierHelper);
					}
				};

				fnTrackTextures(parameters.GetSampledTextures(), GpuParameterType::SampledTexture, GpuAccessFlag::Read);
				fnTrackTextures(parameters.GetStorageTextures(), GpuParameterType::StorageTexture, GpuAccessFlag::Read | GpuAccessFlag::Write);
			}
		} // namespace

		MetalGpuCommandBuffer::MetalGpuCommandBuffer(MetalGpuDevice& device, MetalGpuCommandBufferPool& pool, u32 id, ThreadId ownerThread, GpuQueueType queueType, const GpuCommandBufferCreateInformation& createInformation)
			: GpuCommandBuffer(device, ownerThread, queueType, createInformation)
			, mGpuDevice(device)
			, mPool(pool)
			, mImpl(B3DMakeUnique<Impl>())
			, mId(id)
			, mBarrierHelper(&mResourceTracker)
		{
		}

		namespace
		{
			/**
			 * Closes every encoder on @p impl that is currently open. Residency-cache invalidation is
			 * @b not performed here — callers that need it must reset @c mRenderResidencyCaches /
			 * @c mComputeResidencyCaches (per A'3, one slot per bound parameter set) based on which
			 * encoder transitions matter for them. Keeping the reset out of this helper avoids
			 * redundant resets in dtor / @c End / @c EnsureEncoderKind paths where the caller already
			 * knows what changed.
			 */
			void CloseAllEncoders(MetalGpuCommandBuffer::Impl& impl)
			{
				if (impl.RenderEncoder)
				{
					[impl.RenderEncoder endEncoding];
					impl.RenderEncoder = nil;
				}
				if (impl.ComputeEncoder)
				{
					[impl.ComputeEncoder endEncoding];
					impl.ComputeEncoder = nil;
				}
				if (impl.BlitEncoder)
				{
					[impl.BlitEncoder endEncoding];
					impl.BlitEncoder = nil;
				}
			}
		} // namespace

		MetalGpuCommandBuffer::~MetalGpuCommandBuffer()
		{
			if (mImpl)
			{
				// Close any open encoder. If the command buffer was never committed, Metal will release
				// it automatically under ARC when the pimpl drops its reference.
				CloseAllEncoders(*mImpl);
				mImpl->CommandBuffer = nil;
#if B3D_METAL_USE_EXPLICIT_RESOURCE_SYNCHRONIZATION && !__has_feature(objc_arc)
				[mImpl->ResourceFence release];
#endif
#if B3D_METAL_USE_EXPLICIT_RESOURCE_SYNCHRONIZATION
				mImpl->ResourceFence = nil;
#endif
			}
		}

		id<MTLCommandBuffer> MetalGpuCommandBuffer::GetOrAcquireMetalCommandBuffer()
		{
			if (mRecordingFailed)
				return nil;

			if (mImpl->CommandBuffer != nil)
				return mImpl->CommandBuffer;

			TShared<GpuQueue> queuePtr = mGpuDevice.GetQueue(mQueueType, 0);
			auto metalQueue = std::static_pointer_cast<MetalGpuQueue>(queuePtr);
			if (!metalQueue)
				return nil;

			id<MTLCommandQueue> mtlQueue = metalQueue->GetMetalQueue();
			if (mtlQueue == nil)
				return nil;

#if B3D_BUILD_TYPE_DEVELOPMENT
			MTLCommandBufferDescriptor* descriptor = [[MTLCommandBufferDescriptor alloc] init];
			descriptor.errorOptions = MTLCommandBufferErrorOptionEncoderExecutionStatus;
			mImpl->CommandBuffer = [mtlQueue commandBufferWithDescriptor:descriptor];
#if !__has_feature(objc_arc)
			[descriptor release];
#endif
#else
			mImpl->CommandBuffer = [mtlQueue commandBuffer];
#endif
			if (mImpl->CommandBuffer == nil)
				return nil;

#if B3D_METAL_USE_EXPLICIT_RESOURCE_SYNCHRONIZATION
			if (mImpl->ResourceFence == nil)
			{
				mImpl->ResourceFence = [mGpuDevice.GetMetalDevice() newFence];
				if (mImpl->ResourceFence == nil)
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
				NSString* nsName = [NSString stringWithUTF8String:mName.c_str()];
				[mImpl->CommandBuffer setLabel:nsName];
			}
#endif

			// First acquisition of the native buffer marks the start of recording. Metal has no
			// explicit Begin(); the core state machine's Ready -> Recording edge maps onto the lazy
			// acquire here so external observers (pool Reset/Destroy validation) can distinguish a
			// pristine buffer from one that has recorded work.
			if (mState == GpuCommandBufferState::Ready)
				mState = GpuCommandBufferState::Recording;

			return mImpl->CommandBuffer;
		}

		void MetalGpuCommandBuffer::EnsureEncoderKind(EncoderKind targetKind)
		{
			// Mismatched encoders are closed; a matching encoder is left alone so repeated calls with
			// the same @p targetKind are no-ops. @c EncoderKind::None means "close everything" (used
			// at @c BeginRenderPass / @c End). B3 residency caches are reset when the corresponding
			// encoder is actually closed — @c useResource: marks are scoped to an encoder's lifetime
			// on Metal, so a closed-and-later-reopened encoder of the same kind is a fresh scope and
			// the next bind correctly re-emits @c useResources:. Blit has no residency cache.
			if (targetKind != EncoderKind::Render && mImpl->RenderEncoder != nil)
			{
#if B3D_METAL_USE_EXPLICIT_RESOURCE_SYNCHRONIZATION
				UpdateResourceFence(*mImpl, mImpl->RenderEncoder);
#endif
				[mImpl->RenderEncoder endEncoding];
				mImpl->RenderEncoder = nil;
				ResetRenderResidencyCaches();
				EncodePendingEventSignals();
			}
			if (targetKind != EncoderKind::Compute && mImpl->ComputeEncoder != nil)
			{
#if B3D_METAL_USE_EXPLICIT_RESOURCE_SYNCHRONIZATION
				UpdateResourceFence(*mImpl, mImpl->ComputeEncoder);
#endif
				[mImpl->ComputeEncoder endEncoding];
				mImpl->ComputeEncoder = nil;
				ResetComputeResidencyCaches();
			}
			if (targetKind != EncoderKind::Blit && mImpl->BlitEncoder != nil)
			{
#if B3D_METAL_USE_EXPLICIT_RESOURCE_SYNCHRONIZATION
				UpdateResourceFence(*mImpl, mImpl->BlitEncoder);
#endif
				[mImpl->BlitEncoder endEncoding];
				mImpl->BlitEncoder = nil;
			}
		}

		bool MetalGpuCommandBuffer::RestartRenderPassForBarrier()
		{
#if B3D_METAL_USE_EXPLICIT_RESOURCE_SYNCHRONIZATION
			if (mImpl->RenderEncoder == nil || mImpl->RestartRenderPassDescriptor == nil)
			{
				B3D_LOG(Fatal, LogRenderBackend, "Cannot restart the Metal render pass required for explicit resource synchronization.");
				mRecordingFailed = true;
				EnsureEncoderKind(EncoderKind::None);
				return false;
			}

			EnsureEncoderKind(EncoderKind::None);
			return ResumeRenderPass(mImpl->RestartRenderPassDescriptor);
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

			mImpl->RenderEncoder = [commandBuffer renderCommandEncoderWithDescriptor:descriptor];
			if (mImpl->RenderEncoder == nil)
			{
				B3D_LOG(Fatal, LogRenderBackend, "Failed to resume a Metal render pass after an encoder boundary.");
				mRecordingFailed = true;
				return false;
			}
#if B3D_BUILD_TYPE_DEVELOPMENT
			mImpl->RenderEncoder.label = @"Render pass (resumed)";
#endif

#if B3D_METAL_USE_EXPLICIT_RESOURCE_SYNCHRONIZATION
			WaitForResourceFence(*mImpl, mImpl->RenderEncoder);
#endif
			if (mImpl->HasViewport)
				[mImpl->RenderEncoder setViewport:mImpl->Viewport];
			if (mImpl->HasScissor)
				[mImpl->RenderEncoder setScissorRect:mImpl->Scissor];

			for (const Impl::VertexBufferBinding& binding : mImpl->VertexBufferBindings)
				[mImpl->RenderEncoder setVertexBuffer:binding.Buffer offset:binding.Offset atIndex:binding.Index];

			[mImpl->RenderEncoder setVisibilityResultMode:mImpl->VisibilityMode offset:mImpl->VisibilityOffset];
			return true;
		}

		bool MetalGpuCommandBuffer::ActivateOcclusionQueryPool(const TShared<MetalGpuQueryPool>& queryPool)
		{
			if (!queryPool || mImpl->RenderEncoder == nil || mImpl->RestartRenderPassDescriptor == nil)
				return false;

			if (queryPool.get() == mActiveOcclusionQueryPool.get())
				return true;

			id<MTLBuffer> visibilityBuffer = queryPool->GetVisibilityBuffer();
			if (visibilityBuffer == nil)
			{
				B3D_LOG(Error, LogRenderBackend, "Cannot activate a Metal occlusion query pool without a visibility buffer.");
				return false;
			}

			// visibilityResultBuffer is immutable for the lifetime of a render encoder. Continue the logical
			// engine render pass in a fresh encoder whose attachments load the results stored by the previous
			// encoder. The continuation path restores dynamic state and invalidates residency caches, so the
			// next draw rebinds its pipeline and argument buffers normally.
			mImpl->RestartRenderPassDescriptor.visibilityResultBuffer = visibilityBuffer;
			EnsureEncoderKind(EncoderKind::None);
			mActiveOcclusionQueryPool.reset();

			if (!ResumeRenderPass(mImpl->RestartRenderPassDescriptor))
				return false;

			mActiveOcclusionQueryPool = queryPool;
			return true;
		}

		bool MetalGpuCommandBuffer::ExecutePendingBarriers()
		{
			if (mRecordingFailed)
				return false;

#if B3D_METAL_USE_EXPLICIT_RESOURCE_SYNCHRONIZATION
			if (mImpl->RenderEncoder != nil && mBarrierHelper.RequiresRenderPassRestart())
			{
				if (!RestartRenderPassForBarrier())
					return false;

				mBarrierHelper.Execute(nil, nil);
				return true;
			}

			if (mImpl->BlitEncoder != nil && mBarrierHelper.HasBarriers())
			{
				EnsureEncoderKind(EncoderKind::None);
				mBarrierHelper.Execute(nil, nil);
				return true;
			}
#endif

			mBarrierHelper.Execute(mImpl->RenderEncoder, mImpl->ComputeEncoder);
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
			// The command buffer only ever holds one live encoder at a time (see @c EnsureEncoderKind).
			// The branch order here matches @c BeginLabel / @c EndLabel / @c InsertLabel — it does not
			// reflect any priority ordering. Returns @c nil when no encoder is currently open; callers
			// that want to fall back to the command buffer for label emission must handle that case
			// explicitly (see @c BeginLabel / @c EndLabel).
			if (mImpl->RenderEncoder)
				return mImpl->RenderEncoder;
			if (mImpl->ComputeEncoder)
				return mImpl->ComputeEncoder;
			if (mImpl->BlitEncoder)
				return mImpl->BlitEncoder;
			return nil;
		}

		id<MTLBlitCommandEncoder> MetalGpuCommandBuffer::GetOrOpenBlitEncoder()
		{
			// blitCommandEncoder returns an autoreleased @c MTLBlitCommandEncoder. Under the fiber
			// scheduler the outer runloop may not drain, so we stage allocations inside a local pool;
			// the encoder itself is retained via mImpl->BlitEncoder before the pool drains.
			@autoreleasepool
			{
			// Close any non-blit encoder; residency caches are reset inside the helper. A blit encoder
			// already open is left alone below.
			EnsureEncoderKind(EncoderKind::Blit);

			if (mImpl->BlitEncoder != nil)
				return mImpl->BlitEncoder;

			id<MTLCommandBuffer> cmdBuffer = GetOrAcquireMetalCommandBuffer();
			if (cmdBuffer == nil)
				return nil;

			mImpl->BlitEncoder = [cmdBuffer blitCommandEncoder];
#if B3D_BUILD_TYPE_DEVELOPMENT
			mImpl->BlitEncoder.label = @"Blit pass";
#endif
#if B3D_METAL_USE_EXPLICIT_RESOURCE_SYNCHRONIZATION
			WaitForResourceFence(*mImpl, mImpl->BlitEncoder);
#endif
			return mImpl->BlitEncoder;
			} // @autoreleasepool
		}

		bool MetalGpuCommandBuffer::EncodeSignalEvent(id<MTLSharedEvent> event, u64 value)
		{
			EnsureValidThread();
			if (event == nil || mRecordingFailed)
				return false;

			if (mImpl->RenderEncoder != nil)
			{
				Impl::PendingEventSignal signal;
				signal.Event = event;
				signal.Value = value;
#if !__has_feature(objc_arc)
				[event retain];
#endif
				mImpl->PendingEventSignals.push_back(signal);
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
			if (mImpl->PendingEventSignals.empty())
				return;

			id<MTLCommandBuffer> commandBuffer = mImpl->CommandBuffer;
			if (commandBuffer != nil && !mRecordingFailed)
			{
				for (const Impl::PendingEventSignal& signal : mImpl->PendingEventSignals)
					[commandBuffer encodeSignalEvent:signal.Event value:signal.Value];
			}

#if !__has_feature(objc_arc)
			for (const Impl::PendingEventSignal& signal : mImpl->PendingEventSignals)
				[signal.Event release];
#endif

			mImpl->PendingEventSignals.clear();
		}

		void MetalGpuCommandBuffer::SetName(const StringView& name)
		{
			EnsureValidThread();
			mName = name;
			if (mImpl->CommandBuffer)
			{
				NSString* nsName = [NSString stringWithUTF8String:mName.c_str()];
				[mImpl->CommandBuffer setLabel:nsName];
			}
		}

		void MetalGpuCommandBuffer::SetGpuParameterSet(const TShared<GpuParameterSet>& parameters)
		{
			// B1: the per-parameter-set path below may walk retained Obj-C handles under the hood;
			// drain transients locally so the fiber scheduler does not accumulate them across frames.
			@autoreleasepool
			{
			EnsureValidThread();

			// A'3: null-in is a no-op (intentional narrowing relative to the old single-slot model).
			// The engine clears a set by binding the replacement set at the same index, so null
			// carries no set index we can use to locate the slot to clear. The old code
			// @c mBoundParameterSet = parameters wiped the single slot, but the multi-slot analog
			// cannot be expressed unambiguously.
			if (!parameters)
				return;

			const u32 setIndex = parameters->GetSet();
			if (setIndex >= kMetalVertexBufferSlotBase)
			{
				B3D_LOG(Error, LogRenderBackend,
					"Metal parameter set index {0} collides with reserved vertex-buffer slots.", setIndex);
				return;
			}
			if (setIndex >= (u32)mBoundParameterSets.Size())
			{
				mBoundParameterSets.Resize(setIndex + 1);
				// Keep the per-slot residency caches grown in lockstep so @c Draw / @c DispatchCompute
				// can index them symmetrically with @c mBoundParameterSets.
				mRenderResidencyCaches.Resize(setIndex + 1);
				mComputeResidencyCaches.Resize(setIndex + 1);
			}
			mBoundParameterSets[setIndex] = parameters;

			// B5: only attach the argument buffer to the encoder here. Do NOT commit pending SetX
			// writes and do NOT emit useResource: — those now live on the draw / dispatch path so a
			// set is only rewritten once per draw even when the engine issues SetGpuParameterSet
			// multiple times per draw.
			} // @autoreleasepool
		}

		void MetalGpuCommandBuffer::SetDynamicBufferOffset(u32 set, u32 bufferIndex, u32 offset)
		{
			// B1: SetDynamicOffset resolves handles for a direct argument-buffer write and can retain transient
			// Obj-C labels on the backing buffers. Scope the pool here so they drain within the call.
			@autoreleasepool
			{
			EnsureValidThread();

			// A'3: look up the set by its slot index — the old code ignored @p set and always hit the
			// single cached slot, which silently sent dynamic-offset updates to the wrong set when
			// the pipeline bound more than one.
			if (set >= (u32)mBoundParameterSets.Size() || !mBoundParameterSets[set])
				return;

			// SetDynamicOffset re-encodes the buffer at the new offset inside the argument buffer; the
			// encoder itself sees the argument buffer (not the individual uniform buffer), so no
			// encoder-level offset update is needed.
			auto metalParams = std::static_pointer_cast<MetalGpuParameters>(mBoundParameterSets[set]);
			metalParams->SetDynamicOffset(bufferIndex, offset);
			} // @autoreleasepool
		}

		void MetalGpuCommandBuffer::SetGpuGraphicsPipelineState(const TShared<GpuGraphicsPipelineState>& pipelineState)
		{
			EnsureValidThread();
			mBoundGraphicsPipeline = std::static_pointer_cast<MetalGpuGraphicsPipelineState>(pipelineState);
		}

		void MetalGpuCommandBuffer::SetGpuComputePipelineState(const TShared<GpuComputePipelineState>& pipelineState)
		{
			EnsureValidThread();
			mBoundComputePipeline = pipelineState;

			if (!pipelineState || !mImpl->ComputeEncoder)
				return;

			auto metalCompute = std::static_pointer_cast<MetalGpuComputePipelineState>(pipelineState);
			id<MTLComputePipelineState> pso = metalCompute->GetMetalPipeline();
			if (pso)
				[mImpl->ComputeEncoder setComputePipelineState:pso];
		}

		void MetalGpuCommandBuffer::SetVertexBuffers(u32 index, TShared<GpuBuffer>* buffers, u32 bufferCount)
		{
			// B1: setVertexBuffer:offset:atIndex: retains transient objects inside the encoder state
			// snapshot; drain them locally so they do not survive past the call under the fiber scheduler.
			@autoreleasepool
			{
			EnsureValidThread();
			if (mImpl->RenderEncoder == nil)
				return;

			// Argument buffers for parameter sets occupy low buffer slots; offset the vertex-stream slot
			// so it matches the corresponding slot the pipeline's vertex descriptor expects. If no
			// graphics pipeline has been bound yet (the engine should not be calling SetVertexBuffers in
			// that state) fall back to zero-offset.
			const u32 baseIndex = mBoundGraphicsPipeline ? mBoundGraphicsPipeline->GetVertexBufferBaseIndex() : 0;

			for (u32 bufferIndex = 0; bufferIndex < bufferCount; bufferIndex++)
			{
				auto metalBuffer = std::static_pointer_cast<MetalGpuBuffer>(buffers[bufferIndex]);
				id<MTLBuffer> buffer = metalBuffer ? metalBuffer->GetMetalBuffer() : nil;
				if (buffer == nil)
					continue;

				MetalBuffer* resource = metalBuffer->GetMetalResource();
				if (resource != nullptr)
					mResourceTracker.TrackBufferUsage(resource, GpuResourceUseFlag::VertexBuffer, GpuAccessFlag::Read, mBarrierHelper);

				const NSUInteger metalIndex = baseIndex + index + bufferIndex;
				[mImpl->RenderEncoder setVertexBuffer:buffer offset:0 atIndex:metalIndex];

				auto existing = std::find_if(mImpl->VertexBufferBindings.begin(), mImpl->VertexBufferBindings.end(),
					[metalIndex](const Impl::VertexBufferBinding& binding) { return binding.Index == metalIndex; });
				if (existing != mImpl->VertexBufferBindings.end())
				{
					existing->Buffer = buffer;
					existing->Offset = 0;
				}
				else
				{
					Impl::VertexBufferBinding binding;
					binding.Buffer = buffer;
					binding.Index = metalIndex;
					mImpl->VertexBufferBindings.push_back(binding);
				}
			}

			if (!ExecutePendingBarriers())
				return;
			} // @autoreleasepool
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

			// Pipelines that declare no vertex inputs (fullscreen passes, vertex-ID-only draws) need no
			// resolution; the variant key stays at VertexInputId 0.
			MetalGpuGraphicsPipelineState* pipeline = mBoundGraphicsPipeline.get();
			if (pipeline == nullptr || pipeline->GetInputDeclaration() == nullptr)
				return nullptr;

			// The pipeline expects vertex input but the caller never bound a vertex-buffer description.
			// Skipping the draw (rather than resolving against a null layout) matches the Vulkan backend,
			// which requires a bound vertex declaration whenever the shader declares inputs.
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

			// Bind the device's shared zero-filled null vertex buffer at the null stream's slot so shader
			// inputs with no matching vertex-buffer element fetch a full zeroed element. The slot is
			// offset by kMetalVertexBufferSlotBase to match the MTLVertexDescriptor the pipeline compiled.
			if (vertexInput->HasNullStream() && mImpl->RenderEncoder != nil)
			{
				[mImpl->RenderEncoder setVertexBuffer:mGpuDevice.GetNullVertexBuffer()
					offset:0
					atIndex:(kMetalVertexBufferSlotBase + vertexInput->GetNullStreamIndex())];
			}

			return vertexInput;
		}

		static bool BindGraphicsPipelineForDraw(MetalGpuCommandBuffer::Impl& impl, MetalGpuGraphicsPipelineState* pipeline, DrawOperationType drawOp, const MetalPipelineVariantKey& renderPassKey, const TShared<MetalVertexInput>& vertexInput)
		{
			if (!pipeline || impl.RenderEncoder == nil)
				return false;

			// The render pass populates color/depth/stencil formats and sample count at BeginRenderPass
			// time; the topology class is the one part of the key that varies per draw call. The vertex
			// input id (0 when the pipeline consumes no vertex input) selects the compiled variant whose
			// MTLVertexDescriptor matches the resolved vertex-buffer / shader-input pairing.
			MetalPipelineVariantKey key = renderPassKey;
			key.TopologyClass = (u16)MetalUtility::GetPrimitiveTopologyClass(drawOp);
			key.VertexInputId = vertexInput ? vertexInput->GetId() : 0;

			id<MTLRenderPipelineState> pso = pipeline->GetOrCreateMetalPipeline(key, vertexInput);
			if (pso == nil)
				return false;
			[impl.RenderEncoder setRenderPipelineState:pso];

			id<MTLDepthStencilState> depthStencil = pipeline->GetMetalDepthStencilState();
			if (depthStencil)
				[impl.RenderEncoder setDepthStencilState:depthStencil];

			[impl.RenderEncoder setCullMode:(MTLCullMode)pipeline->GetCullMode()];
			[impl.RenderEncoder setFrontFacingWinding:(MTLWinding)pipeline->GetWinding()];
			[impl.RenderEncoder setTriangleFillMode:(MTLTriangleFillMode)pipeline->GetFillMode()];

			[impl.RenderEncoder setDepthBias:pipeline->GetDepthBias()
				slopeScale:pipeline->GetSlopeScaledDepthBias()
				clamp:pipeline->GetDepthBiasClamp()];
			return true;
		}

		void MetalGpuCommandBuffer::Draw(u32 vertexOffset, u32 vertexCount, u32 instanceCount, u32 firstInstance)
		{
			EnsureValidThread();
			if (mImpl->RenderEncoder == nil || vertexCount == 0)
				return;

			// B1: commit + residency emission + draw encode all produce retained Obj-C transients
			// (pipeline state lookups, resource-handle temporaries, NSArray wrappers for
			// useResources:). Drain locally — the fiber scheduler may go many frames without a
			// runloop tick so accumulation across frames would otherwise be visible.
			@autoreleasepool
			{
			for (const TShared<GpuParameterSet>& slotSet : mBoundParameterSets)
			{
				if (slotSet)
					TrackParameterResources(*std::static_pointer_cast<MetalGpuParameters>(slotSet), false, mResourceTracker, mBarrierHelper);
			}
			if (!ExecutePendingBarriers())
				return;

			// A'1 + B5: for every bound parameter set, (1) attach the argument buffer on the render
			// encoder, (2) commit pending Set* writes, (3) emit useResource: for every referenced
			// resource. @c SetGpuParameterSet can run before the render encoder exists (the engine's
			// @c BeginRenderPass(target, params) flow), so this path attaches a set the first time it is
			// used by the current encoder. CommitPendingBindings internally short-circuits when the set
			// has no dirty entries and returns the residency generation under the same lock.
			for (u32 slotIndex = 0; slotIndex < (u32)mBoundParameterSets.Size(); slotIndex++)
			{
				const TShared<GpuParameterSet>& slotSet = mBoundParameterSets[slotIndex];
				if (!slotSet)
					continue;

				auto metalParams = std::static_pointer_cast<MetalGpuParameters>(slotSet);

				// B3 / A'3: skip the @c useResources: emission loop when the same parameter set at
				// the same generation was already marked resident against the currently-open render
				// encoder. Residency persists for the encoder's lifetime, so re-emitting is pure
				// overhead. A generation bump (triggered by any Set* that genuinely changed a
				// binding) forces a re-emit naturally. Cache is keyed per bound slot so multiple
				// sets on the same encoder don't thrash a single cache entry.
				ParameterSetResidencyCache& cacheEntry = mRenderResidencyCaches[slotIndex];
				const bool setChanged = cacheEntry.LastBoundSet != metalParams.get();
				if (setChanged)
					AttachArgumentBufferToRenderEncoder(mImpl->RenderEncoder, *metalParams);
				const u64 generation = metalParams->CommitPendingBindings();
				const bool cacheHit = !setChanged
					&& cacheEntry.LastBoundGeneration == generation;
				if (!cacheHit)
				{
					EmitResidencyForRenderEncoder(mImpl->RenderEncoder, *metalParams);
					cacheEntry.LastBoundSet = metalParams.get();
					cacheEntry.LastBoundGeneration = generation;
				}
			}

			// Resolve the vertex input (and bind the null stream) against the bound vertex description
			// before selecting the pipeline variant — its id participates in the PSO variant key.
			bool skipDraw = false;
			TShared<MetalVertexInput> vertexInput = ResolveVertexInputForDraw(skipDraw);
			if (skipDraw)
				return;

			if (!BindGraphicsPipelineForDraw(*mImpl, mBoundGraphicsPipeline.get(), mDrawOperation, mRenderPassPipelineKey, vertexInput))
				return;
			[mImpl->RenderEncoder setStencilReferenceValue:mStencilReference];

			MTLPrimitiveType primitive = MetalUtility::GetPrimitiveType(mDrawOperation);
			[mImpl->RenderEncoder drawPrimitives:primitive
				vertexStart:vertexOffset
				vertexCount:vertexCount
				instanceCount:std::max<u32>(1, instanceCount)
				baseInstance:firstInstance];
			} // @autoreleasepool
		}

		void MetalGpuCommandBuffer::DrawIndexed(u32 startIndex, u32 indexCount, u32 vertexOffset, u32 vertexCount, u32 instanceCount, u32 firstInstance)
		{
			EnsureValidThread();
			(void)vertexCount;

			if (mImpl->RenderEncoder == nil || !mBoundIndexBuffer || indexCount == 0)
				return;

			// B1: same rationale as @c Draw — drain transient Obj-C objects produced during commit,
			// residency emission, and the draw encode.
			@autoreleasepool
			{
			auto metalIndex = std::static_pointer_cast<MetalGpuBuffer>(mBoundIndexBuffer);
			MetalBuffer* indexResource = metalIndex ? metalIndex->GetMetalResource() : nullptr;
			if (indexResource != nullptr)
				mResourceTracker.TrackBufferUsage(indexResource, GpuResourceUseFlag::IndexBuffer, GpuAccessFlag::Read, mBarrierHelper);

			for (const TShared<GpuParameterSet>& slotSet : mBoundParameterSets)
			{
				if (slotSet)
					TrackParameterResources(*std::static_pointer_cast<MetalGpuParameters>(slotSet), false, mResourceTracker, mBarrierHelper);
			}
			if (!ExecutePendingBarriers())
				return;

			// A'1 + B5: same rationale as @c Draw — iterate every bound slot, attach + commit +
			// emit-residency per slot. See @c Draw for the full comment.
			for (u32 slotIndex = 0; slotIndex < (u32)mBoundParameterSets.Size(); slotIndex++)
			{
				const TShared<GpuParameterSet>& slotSet = mBoundParameterSets[slotIndex];
				if (!slotSet)
					continue;

				auto metalParams = std::static_pointer_cast<MetalGpuParameters>(slotSet);

				// B3 / A'3: residency-elision cache — see @c Draw. Per-slot keying.
				ParameterSetResidencyCache& cacheEntry = mRenderResidencyCaches[slotIndex];
				const bool setChanged = cacheEntry.LastBoundSet != metalParams.get();
				if (setChanged)
					AttachArgumentBufferToRenderEncoder(mImpl->RenderEncoder, *metalParams);
				const u64 generation = metalParams->CommitPendingBindings();
				const bool cacheHit = !setChanged
					&& cacheEntry.LastBoundGeneration == generation;
				if (!cacheHit)
				{
					EmitResidencyForRenderEncoder(mImpl->RenderEncoder, *metalParams);
					cacheEntry.LastBoundSet = metalParams.get();
					cacheEntry.LastBoundGeneration = generation;
				}
			}

			// Resolve the vertex input (and bind the null stream) before selecting the pipeline variant
			// — see Draw() for the full rationale.
			bool skipDraw = false;
			TShared<MetalVertexInput> vertexInput = ResolveVertexInputForDraw(skipDraw);
			if (skipDraw)
				return;

			if (!BindGraphicsPipelineForDraw(*mImpl, mBoundGraphicsPipeline.get(), mDrawOperation, mRenderPassPipelineKey, vertexInput))
				return;
			[mImpl->RenderEncoder setStencilReferenceValue:mStencilReference];

			id<MTLBuffer> indexBuffer = metalIndex->GetMetalBuffer();
			if (indexBuffer == nil)
				return;

			const IndexType engineIndexType = mBoundIndexBuffer->GetInformation().Index.Type;
			const MTLIndexType indexType = (engineIndexType == IT_32BIT) ? MTLIndexTypeUInt32 : MTLIndexTypeUInt16;
			const u32 indexSize = (engineIndexType == IT_32BIT) ? 4u : 2u;

			MTLPrimitiveType primitive = MetalUtility::GetPrimitiveType(mDrawOperation);
			[mImpl->RenderEncoder drawIndexedPrimitives:primitive
				indexCount:indexCount
				indexType:indexType
				indexBuffer:indexBuffer
				indexBufferOffset:(startIndex * indexSize)
				instanceCount:std::max<u32>(1, instanceCount)
				baseVertex:vertexOffset
				baseInstance:firstInstance];
			} // @autoreleasepool
		}

		void MetalGpuCommandBuffer::DispatchCompute(u32 groupCountX, u32 groupCountY, u32 groupCountZ)
		{
			EnsureValidThread();
			if (groupCountX == 0 || groupCountY == 0 || groupCountZ == 0)
			{
				B3D_LOG(Warning, LogRenderBackend, "Ignoring call to DispatchCompute(). Threadgroup count is zero.");
				return;
			}

			auto metalCompute = std::static_pointer_cast<MetalGpuComputePipelineState>(mBoundComputePipeline);
			if (!metalCompute)
				return;

			id<MTLComputePipelineState> pso = metalCompute->GetMetalPipeline();
			if (pso == nil)
				return;

			const u32* workgroup = metalCompute->GetWorkgroupSize();
			const u64 threadCountPerGroup = (u64)workgroup[0] * workgroup[1] * workgroup[2];
			if (threadCountPerGroup == 0 || threadCountPerGroup > (u64)pso.maxTotalThreadsPerThreadgroup)
			{
				B3D_LOG(Error, LogRenderBackend,
					"Ignoring call to DispatchCompute(). Pipeline workgroup size ({0}, {1}, {2}) exceeds Metal's {3}-thread limit.",
					workgroup[0], workgroup[1], workgroup[2], (u32)pso.maxTotalThreadsPerThreadgroup);
				return;
			}

			// Close any render / blit encoder; leave an already-open compute encoder alone. B3
			// residency-cache invalidation for closed encoders is handled inside @c EnsureEncoderKind.
			EnsureEncoderKind(EncoderKind::Compute);

			id<MTLCommandBuffer> cmdBuffer = GetOrAcquireMetalCommandBuffer();
			if (cmdBuffer == nil)
				return;

			// B1: drain transients produced by compute-encoder acquisition, commit, residency emission,
			// and dispatch. Entered after the cmd-buffer acquire early-bail so trivially-empty paths
			// skip the wrap cost.
			@autoreleasepool
			{
			if (mImpl->ComputeEncoder == nil)
			{
				mImpl->ComputeEncoder = [cmdBuffer computeCommandEncoder];
				if (mImpl->ComputeEncoder == nil)
					return;
#if B3D_BUILD_TYPE_DEVELOPMENT
				mImpl->ComputeEncoder.label = @"Compute pass";
#endif
#if B3D_METAL_USE_EXPLICIT_RESOURCE_SYNCHRONIZATION
				WaitForResourceFence(*mImpl, mImpl->ComputeEncoder);
#endif
			}

			[mImpl->ComputeEncoder setComputePipelineState:pso];

			for (const TShared<GpuParameterSet>& slotSet : mBoundParameterSets)
			{
				if (slotSet)
					TrackParameterResources(*std::static_pointer_cast<MetalGpuParameters>(slotSet), true, mResourceTracker, mBarrierHelper);
			}
			if (!ExecutePendingBarriers())
				return;

			// A'3: iterate every bound slot. Re-attach the argument buffer at the correct set
			// index — if the set was originally bound while a render encoder was open, the compute
			// encoder hasn't seen it yet. @c AttachArgumentBufferToComputeEncoder is idempotent so
			// re-attaching when @c SetGpuParameterSet already covered this slot is safe.
			for (u32 slotIndex = 0; slotIndex < (u32)mBoundParameterSets.Size(); slotIndex++)
			{
				const TShared<GpuParameterSet>& slotSet = mBoundParameterSets[slotIndex];
				if (!slotSet)
					continue;

				auto metalParams = std::static_pointer_cast<MetalGpuParameters>(slotSet);
				// B3 / A'3: residency-elision cache — see @c Draw. Keyed per slot against the
				// compute encoder's lifetime, so a render-then-compute sequence on the same set
				// correctly re-emits once on the compute side.
				ParameterSetResidencyCache& cacheEntry = mComputeResidencyCaches[slotIndex];
				const bool setChanged = cacheEntry.LastBoundSet != metalParams.get();
				if (setChanged)
					AttachArgumentBufferToComputeEncoder(mImpl->ComputeEncoder, *metalParams);
				const u64 generation = metalParams->CommitPendingBindings();
				const bool cacheHit = !setChanged
					&& cacheEntry.LastBoundGeneration == generation;
				if (!cacheHit)
				{
					EmitResidencyForComputeEncoder(mImpl->ComputeEncoder, *metalParams);
					cacheEntry.LastBoundSet = metalParams.get();
					cacheEntry.LastBoundGeneration = generation;
				}
			}

			MTLSize threadsPerGroup = MTLSizeMake(workgroup[0], workgroup[1], workgroup[2]);
			MTLSize groups = MTLSizeMake(groupCountX, groupCountY, groupCountZ);
			[mImpl->ComputeEncoder dispatchThreadgroups:groups threadsPerThreadgroup:threadsPerGroup];
			mResourceTracker.ClearShaderFlagsForAllRenderPassImageSubresources();
			} // @autoreleasepool
		}

		namespace
		{
			/** Returns true if the Metal pixel format carries a depth component. */
			bool MetalFormatHasDepth(MTLPixelFormat fmt)
			{
				switch (fmt)
				{
				case MTLPixelFormatDepth16Unorm:
				case MTLPixelFormatDepth32Float:
				case MTLPixelFormatDepth24Unorm_Stencil8:
				case MTLPixelFormatDepth32Float_Stencil8:
					return true;
				default:
					return false;
				}
			}

			/** Returns true if the Metal pixel format carries a stencil component. */
			bool MetalFormatHasStencil(MTLPixelFormat fmt)
			{
				switch (fmt)
				{
				case MTLPixelFormatStencil8:
				case MTLPixelFormatDepth24Unorm_Stencil8:
				case MTLPixelFormatDepth32Float_Stencil8:
				case MTLPixelFormatX24_Stencil8:
				case MTLPixelFormatX32_Stencil8:
					return true;
				default:
					return false;
				}
			}

			/**
			 * Fills in load/store actions on a color attachment descriptor from the render-pass load/clear
			 * masks. A surface that is neither loaded nor cleared is marked DontCare so the driver can
			 * discard its initial contents.
			 */
			void ConfigureColorAttachmentActions(MTLRenderPassColorAttachmentDescriptor* color, bool load, bool clear, const Color& clearColor)
			{
				if (clear)
				{
					color.loadAction = MTLLoadActionClear;
					color.clearColor = MTLClearColorMake(clearColor.R, clearColor.G, clearColor.B, clearColor.A);
				}
				else
				{
					color.loadAction = load ? MTLLoadActionLoad : MTLLoadActionDontCare;
				}
				color.storeAction = MTLStoreActionStore;
			}

			/**
			 * Sets the per-face/per-slice selector on a color attachment. 3D textures use @c depthPlane;
			 * 2D arrays and cube maps use @c slice.
			 */
			void ConfigureColorAttachmentSubresource(MTLRenderPassColorAttachmentDescriptor* color, const Texture& texture, u32 face, u32 mipLevel)
			{
				color.level = mipLevel;
				if (texture.GetProperties().Type == TEX_TYPE_3D)
				{
					color.slice = 0;
					color.depthPlane = face;
				}
				else
				{
					color.slice = face;
					color.depthPlane = 0;
				}
			}

			/**
			 * A'4: writes a MTLPixelFormat into the @c u16 slot at @p attachmentIndex of
			 * @p colorFormats. Previously packed 8 bits into a @c u64 which truncated format values
			 * > 255 and aliased distinct formats into the same PSO-cache entry. @c MTLPixelFormat
			 * values fit inside a @c u16 as of the public headers, so assignment is sufficient.
			 */
			void PackColorFormat(u16 (&colorFormats)[B3D_MAXIMUM_RENDER_TARGET_COUNT], u32 attachmentIndex, MTLPixelFormat fmt)
			{
				colorFormats[attachmentIndex] = (u16)fmt;
			}
		} // namespace

		void MetalGpuCommandBuffer::BeginRenderPass(const RenderPassCreateInformation& createInformation)
		{
			// Render-pass descriptor and encoder creation produce several autoreleased Obj-C objects
			// (descriptor, attachment descriptors, CAMetalDrawable). Drain them locally — the fiber
			// scheduler may go many frames without a runloop drain. @c mImpl->RenderEncoder retains
			// the encoder before the pool drains, so it survives.
			@autoreleasepool
			{
			EnsureValidThread();
			// Close every encoder before opening the new render encoder. B3 residency caches are
			// reset inside @c EnsureEncoderKind for the render/compute transitions that matter; since
			// useResource: marks are scoped to an encoder's lifetime, the next bind on the fresh
			// render encoder will re-emit useResources: regardless.
			EnsureEncoderKind(EncoderKind::None);

			mRenderPassPipelineKey = MetalPipelineVariantKey{};
			mAcquiredWindowSurface = nullptr;
			mRenderPassWidth = 0;
			mRenderPassHeight = 0;
			mRenderPassAttachmentImages.Clear();
#if !__has_feature(objc_arc)
			[mImpl->RestartRenderPassDescriptor release];
#endif
			mImpl->RestartRenderPassDescriptor = nil;
			mImpl->VertexBufferBindings.clear();
			mImpl->HasViewport = false;
			mImpl->HasScissor = false;
			mImpl->VisibilityMode = MTLVisibilityResultModeDisabled;
			mImpl->VisibilityOffset = 0;

			const TShared<RenderTarget>& target = createInformation.Target;
			if (!target)
				return;

			id<MTLCommandBuffer> cmdBuffer = GetOrAcquireMetalCommandBuffer();
			if (cmdBuffer == nil)
				return;

			MTLRenderPassDescriptor* desc = [MTLRenderPassDescriptor renderPassDescriptor];

			// BeginQuery selects the visibility buffer lazily. Metal requires an encoder boundary when that
			// pool changes, which is handled entirely inside the backend by ActivateOcclusionQueryPool().
			mActiveOcclusionQueryPool.reset();

			const RenderTargetProperties& targetProps = target->GetProperties();
			mRenderPassWidth = targetProps.Width;
			mRenderPassHeight = targetProps.Height;
			mRenderPassPipelineKey.SampleCount = (u16)std::max(1u, targetProps.MultisampleCount);

			const RenderSurfaceMask clearMask = createInformation.ClearMask;
			const RenderSurfaceMask loadMask = createInformation.LoadMask;

			if (targetProps.IsWindow)
			{
				auto* window = static_cast<RenderWindow*>(target.get());
				const TShared<IRenderWindowSurface>& windowSurface = window->GetRenderWindowSurface();
				auto* metalSurface = static_cast<IMetalRenderWindowSurface*>(windowSurface.get());
				if (metalSurface == nullptr)
				{
					B3D_LOG(Error, LogRenderBackend, "BeginRenderPass: render window has no Metal surface attached.");
					return;
				}

				// Rebuild the swap chain before acquiring if it went invalid (typically a window resize).
				// RebuildSwapChain() re-creates the drawable source at the window's current size.
				if (!metalSurface->IsSwapChainValid())
					window->RebuildSwapChain();

				id<MTLTexture> backBuffer = metalSurface->AcquireColorTexture();
				if (backBuffer == nil)
				{
					B3D_LOG(Error, LogRenderBackend, "BeginRenderPass: failed to acquire a back buffer color texture.");
					return;
				}

				mAcquiredWindowSurface = metalSurface;

				MTLRenderPassColorAttachmentDescriptor* color = desc.colorAttachments[0];
				color.texture = backBuffer;
				color.level = 0;
				color.slice = 0;
				color.depthPlane = 0;
				ConfigureColorAttachmentActions(color, loadMask.IsSet(RT_COLOR0), clearMask.IsSet(RT_COLOR0), createInformation.ClearColor);

				PackColorFormat(mRenderPassPipelineKey.ColorFormats, 0, metalSurface->GetColorFormat());

				// Attach the surface's depth/stencil texture when present, folding its format into the
				// PSO variant key so the compiled pipeline matches the render pass' attachment layout.
				id<MTLTexture> depthStencilTexture = metalSurface->GetDepthStencilTexture();
				if (depthStencilTexture != nil)
				{
					const MTLPixelFormat dsFormat = [depthStencilTexture pixelFormat];

					if (MetalFormatHasDepth(dsFormat))
					{
						MTLRenderPassDepthAttachmentDescriptor* depth = desc.depthAttachment;
						depth.texture = depthStencilTexture;
						depth.level = 0;
						depth.slice = 0;
						if (clearMask.IsSet(RT_DEPTH))
						{
							depth.loadAction = MTLLoadActionClear;
							depth.clearDepth = createInformation.ClearDepth;
						}
						else
						{
							depth.loadAction = loadMask.IsSet(RT_DEPTH) ? MTLLoadActionLoad : MTLLoadActionDontCare;
						}
						depth.storeAction = MTLStoreActionStore;

						mRenderPassPipelineKey.DepthFormat = (u32)dsFormat;
					}

					if (MetalFormatHasStencil(dsFormat))
					{
						MTLRenderPassStencilAttachmentDescriptor* stencil = desc.stencilAttachment;
						stencil.texture = depthStencilTexture;
						stencil.level = 0;
						stencil.slice = 0;
						if (clearMask.IsSet(RT_STENCIL))
						{
							stencil.loadAction = MTLLoadActionClear;
							stencil.clearStencil = createInformation.ClearStencil;
						}
						else
						{
							stencil.loadAction = loadMask.IsSet(RT_STENCIL) ? MTLLoadActionLoad : MTLLoadActionDontCare;
						}
						stencil.storeAction = MTLStoreActionStore;

						mRenderPassPipelineKey.StencilFormat = (u32)dsFormat;
					}
				}
			}
			else
			{
				auto* renderTexture = static_cast<MetalRenderTexture*>(target.get());

				for (u32 attachmentIndex = 0; attachmentIndex < B3D_MAXIMUM_RENDER_TARGET_COUNT; attachmentIndex++)
				{
					const RenderSurfaceInformation& surface = renderTexture->GetColorSurface(attachmentIndex);
					if (!surface.Texture)
						continue;

					auto metalTex = std::static_pointer_cast<MetalTexture>(surface.Texture);
					id<MTLTexture> mtlTex = metalTex ? metalTex->GetMetalTexture() : nil;
					if (mtlTex == nil)
						continue;

					MTLRenderPassColorAttachmentDescriptor* color = desc.colorAttachments[attachmentIndex];
					color.texture = mtlTex;
					ConfigureColorAttachmentSubresource(color, *metalTex, surface.Face, surface.MipLevel);

					const RenderSurfaceMaskBits bit = (RenderSurfaceMaskBits)(RT_COLOR0 << attachmentIndex);
					ConfigureColorAttachmentActions(color, loadMask.IsSet(bit), clearMask.IsSet(bit), createInformation.ClearColor);

					PackColorFormat(mRenderPassPipelineKey.ColorFormats, attachmentIndex, [mtlTex pixelFormat]);

					MetalImage* image = metalTex->GetMetalResource();
					if (image != nullptr)
					{
						GpuTextureSubresourceRange range = image->GetRange();
						range.BaseArrayLayer = surface.Face;
						range.ArrayLayerCount = 1;
						range.BaseMipLevel = surface.MipLevel;
						range.MipLevelCount = 1;
						const GpuAccessFlags access = loadMask.IsSet(bit) ? (GpuAccessFlag::Read | GpuAccessFlag::Write) : GpuAccessFlags(GpuAccessFlag::Write);
						mResourceTracker.TrackAttachmentUsage(image, range, GpuImageLayout::ColorAttachment, GpuImageLayout::ColorAttachment,
							GpuResourceUseFlag::ColorAttachment, access, mBarrierHelper);
						mRenderPassAttachmentImages.Add(image);
					}
				}

				const RenderSurfaceInformation& dsSurface = renderTexture->GetDepthStencilSurface();
				if (dsSurface.Texture)
				{
					auto metalDs = std::static_pointer_cast<MetalTexture>(dsSurface.Texture);
					id<MTLTexture> mtlDs = metalDs ? metalDs->GetMetalTexture() : nil;
					if (mtlDs != nil)
					{
						const MTLPixelFormat dsFormat = [mtlDs pixelFormat];

						if (MetalFormatHasDepth(dsFormat))
						{
							MTLRenderPassDepthAttachmentDescriptor* depth = desc.depthAttachment;
							depth.texture = mtlDs;
							depth.level = dsSurface.MipLevel;
							depth.slice = dsSurface.Face;
							if (clearMask.IsSet(RT_DEPTH))
							{
								depth.loadAction = MTLLoadActionClear;
								depth.clearDepth = createInformation.ClearDepth;
							}
							else
							{
								depth.loadAction = loadMask.IsSet(RT_DEPTH) ? MTLLoadActionLoad : MTLLoadActionDontCare;
							}
							depth.storeAction = MTLStoreActionStore;

							mRenderPassPipelineKey.DepthFormat = (u32)dsFormat;
						}

						if (MetalFormatHasStencil(dsFormat))
						{
							MTLRenderPassStencilAttachmentDescriptor* stencil = desc.stencilAttachment;
							stencil.texture = mtlDs;
							stencil.level = dsSurface.MipLevel;
							stencil.slice = dsSurface.Face;
							if (clearMask.IsSet(RT_STENCIL))
							{
								stencil.loadAction = MTLLoadActionClear;
								stencil.clearStencil = createInformation.ClearStencil;
							}
							else
							{
								stencil.loadAction = loadMask.IsSet(RT_STENCIL) ? MTLLoadActionLoad : MTLLoadActionDontCare;
							}
							stencil.storeAction = MTLStoreActionStore;

							mRenderPassPipelineKey.StencilFormat = (u32)dsFormat;
						}

						MetalImage* image = metalDs->GetMetalResource();
						if (image != nullptr)
						{
							GpuTextureSubresourceRange range = image->GetRange();
							range.BaseArrayLayer = dsSurface.Face;
							range.ArrayLayerCount = 1;
							range.BaseMipLevel = dsSurface.MipLevel;
							range.MipLevelCount = 1;
							const bool loads = loadMask.IsSet(RT_DEPTH) || loadMask.IsSet(RT_STENCIL);
							const GpuAccessFlags access = loads ? (GpuAccessFlag::Read | GpuAccessFlag::Write) : GpuAccessFlags(GpuAccessFlag::Write);
							mResourceTracker.TrackAttachmentUsage(image, range, GpuImageLayout::DepthStencilAttachment, GpuImageLayout::DepthStencilAttachment,
								GpuResourceUseFlag::DepthStencilAttachment, access, mBarrierHelper);
							mRenderPassAttachmentImages.Add(image);
						}
					}
				}
			}

			if (!ExecutePendingBarriers())
				return;

			mImpl->RestartRenderPassDescriptor = [desc copy];
			for (u32 attachmentIndex = 0; attachmentIndex < B3D_MAXIMUM_RENDER_TARGET_COUNT; attachmentIndex++)
			{
				MTLRenderPassColorAttachmentDescriptor* attachment = mImpl->RestartRenderPassDescriptor.colorAttachments[attachmentIndex];
				if (attachment.texture != nil)
					attachment.loadAction = MTLLoadActionLoad;
			}
			if (mImpl->RestartRenderPassDescriptor.depthAttachment.texture != nil)
				mImpl->RestartRenderPassDescriptor.depthAttachment.loadAction = MTLLoadActionLoad;
			if (mImpl->RestartRenderPassDescriptor.stencilAttachment.texture != nil)
				mImpl->RestartRenderPassDescriptor.stencilAttachment.loadAction = MTLLoadActionLoad;

			mImpl->RenderEncoder = [cmdBuffer renderCommandEncoderWithDescriptor:desc];
#if B3D_BUILD_TYPE_DEVELOPMENT
			mImpl->RenderEncoder.label = @"Render pass";
#endif
#if B3D_METAL_USE_EXPLICIT_RESOURCE_SYNCHRONIZATION
			WaitForResourceFence(*mImpl, mImpl->RenderEncoder);
#endif

			// Guard the "drawable acquired but encoder creation failed" path. Without this, the
			// back buffer we pulled via @c AcquireColorTexture would remain held until the next
			// successful @c SwapBuffers, stalling the drawable pool for one or more frames
			// (typically manifesting as @c nextDrawable returning nil on the next acquire).
			if (mImpl->RenderEncoder == nil && mAcquiredWindowSurface != nullptr)
			{
				B3D_LOG(Error, LogRenderBackend,
					"BeginRenderPass: failed to create MTLRenderCommandEncoder after acquiring a drawable; aborting drawable.");
				mAcquiredWindowSurface->AbortCurrentDrawable();
				mAcquiredWindowSurface = nullptr;
				return;
			}

			if (mImpl->RenderEncoder == nil)
				return;

			// A'14: tell the window surface that a render encoder has successfully been opened
			// against its drawable, so @c SwapBuffers can distinguish "drawable acquired but never
			// rendered into" from the normal present path and avoid presenting garbage / holding a
			// dangling drawable.
			if (mAcquiredWindowSurface != nullptr)
				mAcquiredWindowSurface->MarkDrawableAsRendered();

			// A'2: honor @c RenderPassCreateInformation::Parameters — the engine's declared contract
			// at the base header is that the command buffer pre-registers every set listed here so
			// the first draw finds the argument buffer already attached. Combined with A'1's
			// draw-time re-attach this wires the engine's "declare in BeginRenderPass" pattern.
			// Attach only here: the draw-time path still commits pending writes and emits
			// useResource:, so nothing from B5's draw-time contract moves earlier.
			for (const TShared<GpuParameterSet>& paramSet : createInformation.Parameters)
			{
				if (!paramSet)
					continue;

				const u32 setIndex = paramSet->GetSet();
				if (setIndex >= kMetalVertexBufferSlotBase)
				{
					B3D_LOG(Error, LogRenderBackend,
						"Metal parameter set index {0} collides with reserved vertex-buffer slots.", setIndex);
					continue;
				}
				if (setIndex >= (u32)mBoundParameterSets.Size())
				{
					mBoundParameterSets.Resize(setIndex + 1);
					mRenderResidencyCaches.Resize(setIndex + 1);
					mComputeResidencyCaches.Resize(setIndex + 1);
				}
				mBoundParameterSets[setIndex] = paramSet;

			}
			} // @autoreleasepool
		}

		void MetalGpuCommandBuffer::EndRenderPass()
		{
			EnsureValidThread();
			// Only the render encoder is expected to be open during a render pass (one-encoder
			// invariant). Close just it here so any stray compute/blit encoder — a bug elsewhere —
			// would surface later rather than being silently ended by this path. B3 residency-cache
			// invalidation for the render encoder happens inside @c EnsureEncoderKind when it
			// transitions away from Render.
			if (mImpl->RenderEncoder != nil)
			{
#if B3D_METAL_USE_EXPLICIT_RESOURCE_SYNCHRONIZATION
				UpdateResourceFence(*mImpl, mImpl->RenderEncoder);
#endif
				[mImpl->RenderEncoder endEncoding];
				mImpl->RenderEncoder = nil;
				ResetRenderResidencyCaches();
				EncodePendingEventSignals();
			}
			mAcquiredWindowSurface = nullptr;
			mRenderPassPipelineKey = MetalPipelineVariantKey{};
			mRenderPassWidth = 0;
			mRenderPassHeight = 0;
			mActiveOcclusionQueryPool.reset();
#if !__has_feature(objc_arc)
			[mImpl->RestartRenderPassDescriptor release];
#endif
			mImpl->RestartRenderPassDescriptor = nil;
			mImpl->VertexBufferBindings.clear();
			mImpl->HasViewport = false;
			mImpl->HasScissor = false;
			mImpl->VisibilityMode = MTLVisibilityResultModeDisabled;
			mImpl->VisibilityOffset = 0;

			for (MetalImage* image : mRenderPassAttachmentImages)
			{
				mResourceTracker.MoveAllAttachmentsToFinalLayouts(image);
				mResourceTracker.ClearFramebufferFlagsForImage(image);
			}
			mResourceTracker.ClearShaderFlagsForAllRenderPassImageSubresources();
			mRenderPassAttachmentImages.Clear();
		}

		bool MetalGpuCommandBuffer::IsInRenderPass() const
		{
			return mImpl->RenderEncoder != nil;
		}

		void MetalGpuCommandBuffer::SetViewport(const Area2& area)
		{
			EnsureValidThread();
			if (mImpl->RenderEncoder == nil)
				return;

			MTLViewport vp;
			vp.originX = area.X;
			vp.originY = area.Y;
			vp.width = (double)area.Width;
			vp.height = (double)area.Height;
			vp.znear = 0.0;
			vp.zfar = 1.0;
			mImpl->Viewport = vp;
			mImpl->HasViewport = true;
			[mImpl->RenderEncoder setViewport:vp];
		}

		void MetalGpuCommandBuffer::ClearRenderTarget(RenderSurfaceMask mask, const Color& color, float depth, u16 stencil)
		{
			EnsureValidThread();
			if (mask == RT_NONE || mImpl->RenderEncoder == nil || mImpl->RestartRenderPassDescriptor == nil)
				return;

			@autoreleasepool
			{
			MTLRenderPassDescriptor* clearDescriptor = [mImpl->RestartRenderPassDescriptor copy];
			bool hasAttachment = false;
			for (u32 attachmentIndex = 0; attachmentIndex < B3D_MAXIMUM_RENDER_TARGET_COUNT; attachmentIndex++)
			{
				const RenderSurfaceMaskBits bit = (RenderSurfaceMaskBits)(RT_COLOR0 << attachmentIndex);
				MTLRenderPassColorAttachmentDescriptor* attachment = clearDescriptor.colorAttachments[attachmentIndex];
				if (!mask.IsSet(bit) || attachment.texture == nil)
					continue;

				attachment.loadAction = MTLLoadActionClear;
				attachment.clearColor = MTLClearColorMake(color.R, color.G, color.B, color.A);
				hasAttachment = true;
			}

			if (mask.IsSet(RT_DEPTH) && clearDescriptor.depthAttachment.texture != nil)
			{
				clearDescriptor.depthAttachment.loadAction = MTLLoadActionClear;
				clearDescriptor.depthAttachment.clearDepth = depth;
				hasAttachment = true;
			}
			if (mask.IsSet(RT_STENCIL) && clearDescriptor.stencilAttachment.texture != nil)
			{
				clearDescriptor.stencilAttachment.loadAction = MTLLoadActionClear;
				clearDescriptor.stencilAttachment.clearStencil = stencil;
				hasAttachment = true;
			}

			if (!hasAttachment)
			{
#if !__has_feature(objc_arc)
				[clearDescriptor release];
#endif
				return;
			}

			EnsureEncoderKind(EncoderKind::None);
			ResumeRenderPass(clearDescriptor);
#if !__has_feature(objc_arc)
			[clearDescriptor release];
#endif
			} // @autoreleasepool
		}

		void MetalGpuCommandBuffer::ClearViewport(RenderSurfaceMask mask, const Color& color, float depth, u16 stencil)
		{
			EnsureValidThread();
			if (mask == RT_NONE || mImpl->RenderEncoder == nil)
				return;

			const bool coversRenderTarget = !mImpl->HasViewport ||
				(mImpl->Viewport.originX == 0.0 && mImpl->Viewport.originY == 0.0 &&
				mImpl->Viewport.width == (double)mRenderPassWidth && mImpl->Viewport.height == (double)mRenderPassHeight);
			if (coversRenderTarget)
			{
				ClearRenderTarget(mask, color, depth, stencil);
				return;
			}

			B3D_LOG(Error, LogRenderBackend,
				"MetalGpuCommandBuffer::ClearViewport cannot clear a partial viewport without the backend's internal clear pipeline.");
		}

		void MetalGpuCommandBuffer::EnableScissorTest(u32 left, u32 top, u32 right, u32 bottom)
		{
			EnsureValidThread();
			if (mImpl->RenderEncoder == nil)
				return;

			MTLScissorRect rect;
			rect.x = left;
			rect.y = top;
			rect.width = (right > left) ? (right - left) : 0;
			rect.height = (bottom > top) ? (bottom - top) : 0;
			mImpl->Scissor = rect;
			mImpl->HasScissor = true;
			[mImpl->RenderEncoder setScissorRect:rect];
		}

		void MetalGpuCommandBuffer::DisableScissorTest()
		{
			EnsureValidThread();
			if (mImpl->RenderEncoder == nil)
				return;

			// Metal has no "disable scissor" concept; clamp the rectangle to the current render-pass
			// dimensions so the driver does not reject an out-of-bounds scissor.
			MTLScissorRect rect;
			rect.x = 0;
			rect.y = 0;
			rect.width = mRenderPassWidth;
			rect.height = mRenderPassHeight;
			mImpl->Scissor = rect;
			mImpl->HasScissor = true;
			[mImpl->RenderEncoder setScissorRect:rect];
		}

		void MetalGpuCommandBuffer::SetStencilReferenceValue(u32 value)
		{
			EnsureValidThread();
			mStencilReference = value;
			if (mImpl->RenderEncoder)
				[mImpl->RenderEncoder setStencilReferenceValue:value];
		}

		namespace
		{
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

			bool GetTextureTransferInformation(const Texture& texture, const GpuBuffer& buffer, u32 bufferOffset,
				u32 mipLevel, u32 arrayLayer, const char* operation, MetalTextureTransferInformation& output)
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
					B3D_LOG(Error, LogRenderBackend, "{0}: buffer offset {1} must be 16-byte aligned on Apple GPUs.",
						operation, bufferOffset);
					return false;
				}

				PixelUtility::GetSizeForMipLevel(properties.Width, properties.Height, properties.Depth,
					mipLevel, output.Width, output.Height, output.Depth);
				if (properties.Type == TEX_TYPE_1D)
				{
					output.Height = 1;
					output.Depth = 1;
				}
				else if (properties.Type != TEX_TYPE_3D)
					output.Depth = 1;

				if (properties.Format == PF_D32_S8X24)
				{
					// Match Vulkan's aspect selection. The buffer-side contract for this command is packed
					// Depth32Float values (not interleaved PF_D32_S8X24 texels); Metal requires the option.
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
					B3D_LOG(Error, LogRenderBackend,
						"{0}: buffer range [{1}, {2}) exceeds the buffer size ({3}).", operation, bufferOffset,
						(u64)bufferOffset + output.RequiredBufferSize, buffer.GetTotalSize());
					return false;
				}

				if (properties.Format == PF_D32_S8X24 &&
					(u64)buffer.GetTotalSize() - bufferOffset != output.RequiredBufferSize)
				{
					B3D_LOG(Error, LogRenderBackend,
						"{0}: combined depth/stencil transfers require an exactly-sized packed Depth32Float buffer. "
						"Interleaved PF_D32_S8X24 staging data is not a valid Metal depth-plane layout.", operation);
					return false;
				}

				return true;
			}
		}

		void MetalGpuCommandBuffer::CopyBufferToBuffer(const TShared<GpuBuffer>& source, const TShared<GpuBuffer>& destination, u32 sourceOffset, u32 destinationOffset, u32 length)
		{
			EnsureValidThread();
			if (!source || !destination || length == 0)
				return;

			if ((sourceOffset % kMetalBufferCopyAlignment) != 0 ||
				(destinationOffset % kMetalBufferCopyAlignment) != 0 ||
				(length % kMetalBufferCopyAlignment) != 0)
			{
				B3D_LOG(Error, LogRenderBackend,
					"MetalGpuCommandBuffer::CopyBufferToBuffer requires 4-byte-aligned offsets and length on macOS.");
				return;
			}

			if (!IsBufferRangeValid(*source, sourceOffset, length) || !IsBufferRangeValid(*destination, destinationOffset, length))
			{
				B3D_LOG(Error, LogRenderBackend, "MetalGpuCommandBuffer::CopyBufferToBuffer range exceeds a buffer's size.");
				return;
			}

			// Validate native handles before changing encoder kind so a failed copy doesn't split an
			// otherwise valid render or compute pass.
			auto srcMetal = std::static_pointer_cast<MetalGpuBuffer>(source);
			auto dstMetal = std::static_pointer_cast<MetalGpuBuffer>(destination);
			id<MTLBuffer> src = srcMetal ? srcMetal->GetMetalBuffer() : nil;
			id<MTLBuffer> dst = dstMetal ? dstMetal->GetMetalBuffer() : nil;
			if (src == nil || dst == nil)
			{
				// Guard against the race where a copy is queued before GpuObjectCreateFlag::DeferredInitialize
				// has finished for either endpoint.
				B3D_LOG(Error, LogRenderBackend,
					"MetalGpuCommandBuffer::CopyBufferToBuffer: source or destination MTLBuffer is nil (deferred init not complete).");
				return;
			}

			if (src == dst)
			{
				const u64 sourceEnd = (u64)sourceOffset + length;
				const u64 destinationEnd = (u64)destinationOffset + length;
				if ((u64)sourceOffset < destinationEnd && (u64)destinationOffset < sourceEnd)
				{
					B3D_LOG(Error, LogRenderBackend,
						"MetalGpuCommandBuffer::CopyBufferToBuffer does not support overlapping ranges in one buffer.");
					return;
				}
			}

			EnsureEncoderKind(EncoderKind::Blit);

			MetalBuffer* sourceResource = srcMetal->GetMetalResource();
			MetalBuffer* destinationResource = dstMetal->GetMetalResource();
			if (sourceResource != nullptr)
				mResourceTracker.TrackBufferUsage(sourceResource, GpuResourceUseFlag::Transfer, GpuAccessFlag::Read, mBarrierHelper, sourceOffset);
			if (destinationResource != nullptr)
				mResourceTracker.TrackBufferUsage(destinationResource, GpuResourceUseFlag::Transfer, GpuAccessFlag::Write, mBarrierHelper, destinationOffset);
			if (!ExecutePendingBarriers())
				return;

			id<MTLBlitCommandEncoder> blit = GetOrOpenBlitEncoder();
			if (blit == nil)
				return;

			[blit copyFromBuffer:src
				sourceOffset:sourceOffset
				toBuffer:dst
				destinationOffset:destinationOffset
				size:length];
		}

		void MetalGpuCommandBuffer::CopyBufferToTexture(const TShared<GpuBuffer>& source, const TShared<Texture>& destination, u32 bufferOffset, u32 mipLevel, u32 arrayLayer)
		{
			EnsureValidThread();
			if (!source || !destination)
				return;

			MetalTextureTransferInformation transferInformation;
			if (!GetTextureTransferInformation(*destination, *source, bufferOffset, mipLevel, arrayLayer,
				"MetalGpuCommandBuffer::CopyBufferToTexture", transferInformation))
				return;

			auto srcBuffer = std::static_pointer_cast<MetalGpuBuffer>(source);
			auto dstTexture = std::static_pointer_cast<MetalTexture>(destination);
			id<MTLBuffer> src = srcBuffer ? srcBuffer->GetMetalBuffer() : nil;
			id<MTLTexture> dst = dstTexture ? dstTexture->GetMetalTexture() : nil;
			if (src == nil || dst == nil)
			{
				B3D_LOG(Error, LogRenderBackend,
					"MetalGpuCommandBuffer::CopyBufferToTexture: source buffer or destination texture is nil (deferred init not complete).");
				return;
			}

			EnsureEncoderKind(EncoderKind::Blit);

			MetalBuffer* sourceResource = srcBuffer->GetMetalResource();
			MetalImage* destinationResource = dstTexture->GetMetalResource();
			GpuTextureSubresourceRange destinationRange = destinationResource != nullptr ? destinationResource->GetRange() : GpuTextureSubresourceRange{};
			destinationRange.BaseArrayLayer = arrayLayer;
			destinationRange.ArrayLayerCount = 1;
			destinationRange.BaseMipLevel = mipLevel;
			destinationRange.MipLevelCount = 1;
			if (destination->GetProperties().Format == PF_D32_S8X24)
				destinationRange.AspectMask = GpuTextureAspectFlag::Depth;
			if (sourceResource != nullptr)
				mResourceTracker.TrackBufferUsage(sourceResource, GpuResourceUseFlag::Transfer, GpuAccessFlag::Read, mBarrierHelper, bufferOffset);
			if (destinationResource != nullptr)
				mResourceTracker.TrackImageUsage(destinationResource, destinationRange, GpuImageLayout::TransferDestination, GpuImageLayout::TransferDestination, GpuResourceUseFlag::Transfer, GpuAccessFlag::Write, mBarrierHelper);
			if (!ExecutePendingBarriers())
				return;

			id<MTLBlitCommandEncoder> blit = GetOrOpenBlitEncoder();
			if (blit == nil)
				return;

			[blit copyFromBuffer:src
				sourceOffset:bufferOffset
				sourceBytesPerRow:transferInformation.RowPitch
				sourceBytesPerImage:(transferInformation.Depth > 1 ? transferInformation.SlicePitch : 0)
				sourceSize:MTLSizeMake(transferInformation.Width, transferInformation.Height, transferInformation.Depth)
				toTexture:dst
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
			if (!GetTextureTransferInformation(*source, *destination, bufferOffset, mipLevel, arrayLayer,
				"MetalGpuCommandBuffer::CopyTextureToBuffer", transferInformation))
				return;

			auto srcTexture = std::static_pointer_cast<MetalTexture>(source);
			auto dstBuffer = std::static_pointer_cast<MetalGpuBuffer>(destination);
			id<MTLTexture> src = srcTexture ? srcTexture->GetMetalTexture() : nil;
			id<MTLBuffer> dst = dstBuffer ? dstBuffer->GetMetalBuffer() : nil;
			if (src == nil || dst == nil)
			{
				B3D_LOG(Error, LogRenderBackend,
					"MetalGpuCommandBuffer::CopyTextureToBuffer: source texture or destination buffer is nil (deferred init not complete).");
				return;
			}

			EnsureEncoderKind(EncoderKind::Blit);

			MetalImage* sourceResource = srcTexture->GetMetalResource();
			MetalBuffer* destinationResource = dstBuffer->GetMetalResource();
			GpuTextureSubresourceRange sourceRange = sourceResource != nullptr ? sourceResource->GetRange() : GpuTextureSubresourceRange{};
			sourceRange.BaseArrayLayer = arrayLayer;
			sourceRange.ArrayLayerCount = 1;
			sourceRange.BaseMipLevel = mipLevel;
			sourceRange.MipLevelCount = 1;
			if (source->GetProperties().Format == PF_D32_S8X24)
				sourceRange.AspectMask = GpuTextureAspectFlag::Depth;
			if (sourceResource != nullptr)
				mResourceTracker.TrackImageUsage(sourceResource, sourceRange, GpuImageLayout::TransferSource, GpuImageLayout::TransferSource, GpuResourceUseFlag::Transfer, GpuAccessFlag::Read, mBarrierHelper);
			if (destinationResource != nullptr)
				mResourceTracker.TrackBufferUsage(destinationResource, GpuResourceUseFlag::Transfer, GpuAccessFlag::Write, mBarrierHelper, bufferOffset);
			if (!ExecutePendingBarriers())
				return;

			id<MTLBlitCommandEncoder> blit = GetOrOpenBlitEncoder();
			if (blit == nil)
				return;

			[blit copyFromTexture:src
				sourceSlice:arrayLayer
				sourceLevel:mipLevel
				sourceOrigin:MTLOriginMake(0, 0, 0)
				sourceSize:MTLSizeMake(transferInformation.Width, transferInformation.Height, transferInformation.Depth)
				toBuffer:dst
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
				B3D_LOG(Error, LogRenderBackend,
					"MetalGpuCommandBuffer::CopyTexture: source or destination texture is nil (deferred init not complete).");
				return false;
			}

			if (sourceHandle.pixelFormat != destinationHandle.pixelFormat)
			{
				B3D_LOG(Error, LogRenderBackend,
					"MetalGpuCommandBuffer::CopyTexture requires identical native pixel formats (including sRGB state).");
				return false;
			}

			const TextureProperties& sourceProperties = source->GetProperties();
			const TextureProperties& destinationProperties = destination->GetProperties();
			u32 sourceWidth, sourceHeight, sourceDepth;
			PixelUtility::GetSizeForMipLevel(sourceProperties.Width, sourceProperties.Height, sourceProperties.Depth,
				copyInformation.SourceMip, sourceWidth, sourceHeight, sourceDepth);

			const bool copyEntireSurface = copyInformation.SourceVolume.GetWidth() == 0 ||
				copyInformation.SourceVolume.GetHeight() == 0 || copyInformation.SourceVolume.GetDepth() == 0;
			const MTLOrigin sourceOrigin = copyEntireSurface
				? MTLOriginMake(0, 0, 0)
				: MTLOriginMake(copyInformation.SourceVolume.Left, copyInformation.SourceVolume.Top, copyInformation.SourceVolume.Front);
			const MTLSize copySize = copyEntireSurface
				? MTLSizeMake(sourceWidth, sourceHeight, sourceDepth)
				: MTLSizeMake(copyInformation.SourceVolume.GetWidth(), copyInformation.SourceVolume.GetHeight(), copyInformation.SourceVolume.GetDepth());
			const MTLOrigin destinationOrigin = MTLOriginMake(copyInformation.DestinationPosition.X,
				copyInformation.DestinationPosition.Y, copyInformation.DestinationPosition.Z);

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
				const bool alignedOrigins = (sourceOrigin.x % blockDimensions.X) == 0 && (sourceOrigin.y % blockDimensions.Y) == 0 &&
					(destinationOrigin.x % blockDimensions.X) == 0 && (destinationOrigin.y % blockDimensions.Y) == 0;
				u32 destinationWidth, destinationHeight, destinationDepth;
				PixelUtility::GetSizeForMipLevel(destinationProperties.Width, destinationProperties.Height, destinationProperties.Depth,
					copyInformation.DestinationMip, destinationWidth, destinationHeight, destinationDepth);
				(void)destinationDepth;
				const bool alignedExtent = ((copySize.width % blockDimensions.X) == 0 ||
					(sourceOrigin.x + copySize.width == sourceWidth && destinationOrigin.x + copySize.width == destinationWidth)) &&
					((copySize.height % blockDimensions.Y) == 0 ||
					(sourceOrigin.y + copySize.height == sourceHeight && destinationOrigin.y + copySize.height == destinationHeight));
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

			EnsureEncoderKind(EncoderKind::Blit);
			mResourceTracker.TrackImageUsage(sourceResource, sourceRange, GpuImageLayout::TransferSource,
				GpuImageLayout::TransferSource, GpuResourceUseFlag::Transfer, GpuAccessFlag::Read, mBarrierHelper);
			mResourceTracker.TrackImageUsage(destinationResource, destinationRange, GpuImageLayout::TransferDestination,
				GpuImageLayout::TransferDestination, GpuResourceUseFlag::Transfer, GpuAccessFlag::Write, mBarrierHelper);
			if (!ExecutePendingBarriers())
				return false;

			const bool resolveMultisample = sourceProperties.SampleCount > 1 && destinationProperties.SampleCount == 1;
			if (resolveMultisample)
			{
				u32 destinationWidth, destinationHeight, destinationDepth;
				PixelUtility::GetSizeForMipLevel(destinationProperties.Width, destinationProperties.Height, destinationProperties.Depth,
					copyInformation.DestinationMip, destinationWidth, destinationHeight, destinationDepth);
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

				EnsureEncoderKind(EncoderKind::None);
				id<MTLCommandBuffer> commandBuffer = GetOrAcquireMetalCommandBuffer();
				if (commandBuffer == nil)
					return false;

				for (u32 faceOffset = 0; faceOffset < copyInformation.FaceCount; faceOffset++)
				{
					@autoreleasepool
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

					mImpl->RenderEncoder = [commandBuffer renderCommandEncoderWithDescriptor:resolveDescriptor];
					if (mImpl->RenderEncoder == nil)
						return false;
#if B3D_METAL_USE_EXPLICIT_RESOURCE_SYNCHRONIZATION
					WaitForResourceFence(*mImpl, mImpl->RenderEncoder);
#endif
					EnsureEncoderKind(EncoderKind::None);
					} // @autoreleasepool
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
			PixelUtility::GetSizeForMipLevel(sourceProperties.Width, sourceProperties.Height, sourceProperties.Depth,
				blitInformation.SourceMip, sourceWidth, sourceHeight, sourceDepth);
			u32 destinationWidth, destinationHeight, destinationDepth;
			PixelUtility::GetSizeForMipLevel(destinationProperties.Width, destinationProperties.Height, destinationProperties.Depth,
				blitInformation.DestinationMip, destinationWidth, destinationHeight, destinationDepth);

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

			const bool matchingExtents = sourceVolume.GetWidth() == destinationVolume.GetWidth() &&
				sourceVolume.GetHeight() == destinationVolume.GetHeight() && sourceVolume.GetDepth() == destinationVolume.GetDepth();
			if (!matchingExtents || sourceProperties.Format != destinationProperties.Format || sourceProperties.Type != destinationProperties.Type)
			{
				B3D_LOG(Error, LogRenderBackend,
					"MetalGpuCommandBuffer::BlitTexture scaling and format conversion require the backend's internal shader blit path.");
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

			if (mImpl->RenderEncoder != nil && mGpuDevice.SupportsRenderEncoderTimestamps())
				[mImpl->RenderEncoder sampleCountersInBuffer:counterBuffer atSampleIndex:query.Id withBarrier:YES];
			else if (mImpl->ComputeEncoder != nil && mGpuDevice.SupportsComputeEncoderTimestamps())
				[mImpl->ComputeEncoder sampleCountersInBuffer:counterBuffer atSampleIndex:query.Id withBarrier:YES];
			else
			{
				id<MTLBlitCommandEncoder> blitEncoder = mImpl->BlitEncoder;
				if (blitEncoder == nil && mImpl->RenderEncoder == nil && mImpl->ComputeEncoder == nil)
					blitEncoder = GetOrOpenBlitEncoder();

				if (blitEncoder == nil || !mGpuDevice.SupportsBlitEncoderTimestamps())
				{
					B3D_LOG(Error, LogRenderBackend,
						"WriteTimestamp cannot sample the active Metal encoder on this device.");
					return;
				}

				[mImpl->BlitEncoder sampleCountersInBuffer:counterBuffer atSampleIndex:query.Id withBarrier:YES];
			}

			AddUniqueUsedQueryPool(metalPool);
		}

		void MetalGpuCommandBuffer::BeginQuery(GpuQueryId query, const TShared<GpuQueryPool>& queryPool, GpuQueryFlags flags)
		{
			EnsureValidThread();

			if (!queryPool || queryPool->GetQueryType() != GpuQueryType::Occlusion || mImpl->RenderEncoder == nil)
			{
				B3D_LOG(Error, LogRenderBackend,
					"BeginQuery requires an active render pass and an occlusion query pool.");
				return;
			}

			auto metalPool = std::static_pointer_cast<MetalGpuQueryPool>(queryPool);
			if (!metalPool->IsQueryAllocated(query))
			{
				B3D_LOG(Error, LogRenderBackend, "BeginQuery received an invalid or unallocated query ID.");
				return;
			}
			if (mImpl->VisibilityMode != MTLVisibilityResultModeDisabled)
			{
				B3D_LOG(Error, LogRenderBackend, "Metal does not support nested occlusion queries.");
				return;
			}
			if (!ActivateOcclusionQueryPool(metalPool))
				return;

			// Counting mode returns the exact sample count per query; Boolean returns 0 or 1. We use the
			// precise flag to drive the distinction. ActivateOcclusionQueryPool ensures the pool's visibility
			// buffer was attached when the current native render encoder was created.
			const MTLVisibilityResultMode mode = flags.IsSet(GpuQueryFlag::PreciseOcclusion)
				? MTLVisibilityResultModeCounting
				: MTLVisibilityResultModeBoolean;
			const NSUInteger offset = metalPool->GetQueryOffset(query);
			mImpl->VisibilityMode = mode;
			mImpl->VisibilityOffset = offset;
			[mImpl->RenderEncoder setVisibilityResultMode:mode offset:offset];

			AddUniqueUsedQueryPool(metalPool);
		}

		void MetalGpuCommandBuffer::EndQuery(GpuQueryId query, const TShared<GpuQueryPool>& queryPool)
		{
			EnsureValidThread();

			if (mImpl->RenderEncoder == nil || !queryPool || queryPool->GetQueryType() != GpuQueryType::Occlusion)
				return;

			auto metalPool = std::static_pointer_cast<MetalGpuQueryPool>(queryPool);
			if (!metalPool->IsQueryAllocated(query) || metalPool.get() != mActiveOcclusionQueryPool.get()
				|| mImpl->VisibilityMode == MTLVisibilityResultModeDisabled
				|| mImpl->VisibilityOffset != metalPool->GetQueryOffset(query))
			{
				B3D_LOG(Error, LogRenderBackend, "EndQuery does not match the active Metal occlusion query.");
				return;
			}

			mImpl->VisibilityMode = MTLVisibilityResultModeDisabled;
			mImpl->VisibilityOffset = 0;
			[mImpl->RenderEncoder setVisibilityResultMode:MTLVisibilityResultModeDisabled offset:0];
		}

		void MetalGpuCommandBuffer::ResetQueries(const TShared<GpuQueryPool>& queryPool)
		{
			EnsureValidThread();

			if (!queryPool)
				return;
			if (mImpl->RenderEncoder != nil)
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

			NSString* nsName = [[NSString alloc] initWithBytes:name.data() length:name.size() encoding:NSUTF8StringEncoding];
			[commandBuffer pushDebugGroup:nsName];
			mImpl->DebugGroupDepth++;
#if !__has_feature(objc_arc)
			[nsName release];
#endif
#else
			(void)name;
#endif
		}

		void MetalGpuCommandBuffer::EndLabel()
		{
#if B3D_BUILD_TYPE_DEVELOPMENT
			EnsureValidThread();
			if (mImpl->CommandBuffer == nil || mImpl->DebugGroupDepth == 0)
				return;

			[mImpl->CommandBuffer popDebugGroup];
			mImpl->DebugGroupDepth--;
#endif
		}

		void MetalGpuCommandBuffer::InsertLabel(const StringView& name)
		{
#if B3D_BUILD_TYPE_DEVELOPMENT
			if (name.empty())
				return;

			// Intentionally no command-buffer fallback: @c insertDebugSignpost is an encoder-level
			// API on Metal, so outside an encoder the signpost has nowhere to attach.
			id<MTLCommandEncoder> encoder = GetActiveEncoder();
			if (encoder == nil)
				return;

			NSString* nsName = [[NSString alloc] initWithBytes:name.data() length:name.size() encoding:NSUTF8StringEncoding];
			[encoder insertDebugSignpost:nsName];
#if !__has_feature(objc_arc)
			[nsName release];
#endif
#else
			(void)name;
#endif
		}

		void MetalGpuCommandBuffer::End()
		{
#if B3D_BUILD_TYPE_DEVELOPMENT
			while (mImpl->CommandBuffer != nil && mImpl->DebugGroupDepth > 0)
			{
				[mImpl->CommandBuffer popDebugGroup];
				mImpl->DebugGroupDepth--;
			}
#endif

			// A'8: the public End() transitions the buffer into @c RecordingDone, *not* @c Done. The
			// old code flipped straight to @c Done here, which let any external observer of
			// @c GetState() between End() and @c CommitInternal() see a false "completed on the GPU"
			// signal. The corrected state machine is:
			//
			//   Recording(RenderPass) --End()--> RecordingDone --CommitInternal--> Executing
			//     --completion handler--> Done
			//
			// @c CommitInternal closes any still-open encoder itself (via @c EnsureEncoderKind) and
			// does *not* route through this public End(); see the comment at the top of
			// @c CommitInternal.
			//
			// B3: every encoder is gone after the call below, so every residency mark is gone with
			// them. A recycled buffer must start with an empty cache or the next user would observe
			// stale hits against the prior submission's encoder — handled inside @c EnsureEncoderKind.
			EnsureEncoderKind(EncoderKind::None);
			mState = GpuCommandBufferState::RecordingDone;
		}

		void MetalGpuCommandBuffer::IssueBarriers(const GpuBarriers& barriers)
		{
			EnsureValidThread();
			if (mRecordingFailed)
				return;

			// Translate each engine barrier into tracker-aware helper barriers. Source state is
			// deduced from this command buffer's tracked state when the caller left SourceUsage
			// undefined; barriers for resources that were never tracked here are dropped (there is
			// nothing recorded to synchronize against). Mirrors VulkanGpuCommandBuffer::IssueBarriers
			// with Metal type substitutions and the Metal-specific encoder-based Execute.
			for (const GpuBufferBarrier& bufferBarrier : barriers.BufferBarriers)
			{
				auto* metalGpuBuffer = static_cast<MetalGpuBuffer*>(bufferBarrier.Object.get());
				if (metalGpuBuffer == nullptr)
					continue;

				MetalBuffer* resource = metalGpuBuffer->GetMetalResource();
				if (resource == nullptr)
					continue;

				if (bufferBarrier.SourceUsage == GpuResourceUseFlag::Undefined)
					mBarrierHelper.AddBufferBarrier(resource, bufferBarrier.DestinationUsage, bufferBarrier.DestinationAccess);
				else
					mBarrierHelper.AddBufferBarrier(resource, bufferBarrier.SourceUsage, bufferBarrier.SourceAccess, bufferBarrier.DestinationUsage, bufferBarrier.DestinationAccess);
			}

			for (const GpuTextureBarrier& textureBarrier : barriers.TextureBarriers)
			{
				auto* metalTexture = static_cast<MetalTexture*>(textureBarrier.Object.get());
				if (metalTexture == nullptr)
					continue;

				MetalImage* resource = metalTexture->GetMetalResource();
				if (resource == nullptr)
					continue;

				if (textureBarrier.SourceUsage == GpuResourceUseFlag::Undefined)
					mBarrierHelper.AddImageBarrier(resource, textureBarrier.SubresourceRange, textureBarrier.DestinationUsage, textureBarrier.DestinationAccess, textureBarrier.DestinationLayout);
				else
					mBarrierHelper.AddImageBarrier(resource, textureBarrier.SubresourceRange, textureBarrier.SourceUsage, textureBarrier.SourceAccess, textureBarrier.DestinationUsage, textureBarrier.DestinationAccess, textureBarrier.SourceLayout, textureBarrier.DestinationLayout);
			}

			// Metal has no framebuffer object to resolve a surface mask against here. In explicit mode
			// an active render pass therefore uses a pass boundary, which is the only Apple-family-safe
			// conservative synchronization for an unspecified attachment dependency.
			const bool hasRenderTargetBarriers = !barriers.RenderTargetBarriers.Empty();
#if B3D_METAL_USE_EXPLICIT_RESOURCE_SYNCHRONIZATION
			if (hasRenderTargetBarriers && mImpl->RenderEncoder != nil)
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

		void MetalGpuCommandBuffer::EncodeQueueWaits(id<MTLCommandBuffer> cmdBuffer, MetalGpuQueue& submitQueue, GpuQueueMask syncMask)
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
					const u64 waitValue = waitQueue->GetLastCommittedEventValue();
					if (waitEvent != nil && waitValue != 0)
						[cmdBuffer encodeWaitForEvent:waitEvent value:waitValue];
				}
			}
		}

		u64 MetalGpuCommandBuffer::EncodeQueueSyncAndSignal(id<MTLCommandBuffer> cmdBuffer, MetalGpuQueue& submitQueue, GpuQueueMask syncMask)
		{
			// This helper is used with waits only for an otherwise-empty command buffer, where the
			// wait is naturally before the signal. Recorded work uses a separate wait prologue in
			// CommitInternal because appending a wait here would place it after that work.

			// Encode cross-queue waits for the empty-command-buffer path.
			// wait on its shared event at the latest value that queue has *committed* so far. Metal's
			// unified queue family still lets submissions on different MTLCommandQueues execute in any
			// order, so this is where the engine's explicit sync crosses into the driver.
			MetalGpuDevice& device = mGpuDevice;
			const GpuQueueMask selfMask = GpuQueueId(submitQueue.GetType(), submitQueue.GetIndex());
			const GpuQueueMask waitMask = syncMask & ~selfMask;
			if (!waitMask.IsEmpty())
			{
				for (u32 queueTypeIndex = 0; queueTypeIndex < GQT_COUNT; queueTypeIndex++)
				{
					const GpuQueueType queueType = (GpuQueueType)queueTypeIndex;
					const u32 queueCount = device.GetQueueCount(queueType);
					for (u32 queueIndex = 0; queueIndex < queueCount; queueIndex++)
					{
						const GpuQueueId waitQueueId(queueType, queueIndex);
						if (!waitMask.IsSet(waitQueueId))
							continue;

						TShared<GpuQueue> queuePtr = device.GetQueue(queueType, queueIndex);
						auto waitQueue = std::static_pointer_cast<MetalGpuQueue>(queuePtr);
						if (!waitQueue)
							continue;

						id<MTLSharedEvent> waitEvent = waitQueue->GetSharedEvent();
						// Wait on the producer queue's *committed* event value (A8). Using the reserved
						// value here was a race: a concurrent submit on the producer queue could bump
						// the reservation counter to N+1 before calling @c [cmdBuffer commit], and a
						// consumer reading that value would wait forever on a value Metal never gets a
						// chance to signal. The committed value is only ever bumped after
						// @c [cmdBuffer commit] returns, so it is safe to wait on.
						const u64 waitValue = waitQueue->GetLastCommittedEventValue();
						if (waitEvent == nil || waitValue == 0)
							continue;

						[cmdBuffer encodeWaitForEvent:waitEvent value:waitValue];
					}
				}
			}

			// Reserve the next signal value for this queue and encode the signal on the command buffer.
			// Completion handlers run after the signal, so any CPU-side waiter blocked on the event
			// value will unblock before OnDidComplete fires.
			const u64 signalValue = submitQueue.ReserveNextEventValue();
			id<MTLSharedEvent> signalEvent = submitQueue.GetSharedEvent();
			if (signalEvent != nil)
				[cmdBuffer encodeSignalEvent:signalEvent value:signalValue];

			return signalValue;
		}

		namespace
		{
			/**
			 * Encodes one encodeSignalEvent:value: per user-supplied timeline fence on @p cmdBuffer.
			 * Must be called before [cmdBuffer commit]; ordering against the queue's own signal is
			 * the command buffer's FIFO order, so calling this *after* EncodeQueueSyncAndSignal
			 * means user fences fire after the queue event, after every preceding command's GPU work.
			 */
			void EncodeUserFenceSignals(id<MTLCommandBuffer> cmdBuffer, TArrayView<const GpuTimelineFenceAndValue> signalFences)
			{
				for (const GpuTimelineFenceAndValue& entry : signalFences)
				{
					if (!entry.Fence)
						continue;

					auto* metalFence = static_cast<MetalGpuTimelineFence*>(entry.Fence.get());
					id<MTLSharedEvent> sharedEvent = metalFence->GetSharedEvent();
					if (sharedEvent != nil)
						[cmdBuffer encodeSignalEvent:sharedEvent value:entry.Value];
				}
			}
		}

		void MetalGpuCommandBuffer::CommitInternal(MetalGpuQueue& submitQueue, GpuQueueMask syncMask, TArrayView<const GpuTimelineFenceAndValue> signalFences)
		{
			// Submissions are serialized on the submit thread: GpuSubmitThread::QueueSubmit posts this
			// call via MetalGpuDevice::ExecuteSubmit. By this point the owner thread has released its
			// recording state (NotifyWillQueueForSubmit) and transitioned the buffer to Executing
			// (MetalGpuQueue::SubmitCommandBuffer), so only mImpl, mQueueSyncMask (already folded into
			// @p syncMask by GpuSubmitThread::QueueSubmit) and mUsedQueryPools are touched here.
			AssertIfNotSubmitThread();

			// Record the submission queue now. Resource use is promoted from Bound to Used only after
			// every native command buffer needed by the submission has been allocated successfully.
			mSubmittedQueueId = submitQueue.GetId();

			// The commit path creates several autoreleased Obj-C objects (completion-handler block,
			// NSArray of encoded waits, label strings). Draining locally guarantees they don't
			// accumulate across the fiber-scheduled frames that never hit a runloop.
			@autoreleasepool
			{
			// A'8: close any still-open encoder without touching @c mState. The submitting queue is
			// the transition authority for @c Executing (owner thread) and the completion handler
			// below for @c Done (posted back to the owner thread); this method must not flip states
			// of its own making. Note that the caller usually has already invoked @c End() on this
			// path; calling @c EnsureEncoderKind a second time is a cheap no-op when every encoder is
			// already closed.
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
				mImpl->CommandBuffer = nil;

				TShared<GpuCommandBuffer> selfShared = GetShared();
				TShared<WaitGroup> ownerCompletion = B3DMakeShared<WaitGroup>(1);
				submitQueue.NotifySubmissionFailed(ownerCompletion);
				mPool.GetMessageQueue().PostCommand([selfShared, ownerCompletion]()
				{
					auto* owner = static_cast<MetalGpuCommandBuffer*>(selfShared.get());
					owner->mState = GpuCommandBufferState::Done;
					owner->OnDidComplete();
					owner->Cleanup();
					owner->mPool.NotifyCommandBufferReady(owner->mId);
					ownerCompletion->NotifyDone();
				}, "MetalGpuCommandBuffer failed submission");
			};

			if (mRecordingFailed)
			{
				fnPostFailedSubmissionCompletion();
				return;
			}

			// A'9: the empty path still has to go through the queue's event so cross-queue waits
			// the engine accumulated via @c AddQueueSyncMask are honored. Acquire a throwaway
			// MTLCommandBuffer from the submit queue, encode the waits + signal into it, install
			// the same completion handler as the real path, and commit. If queue acquisition fails
			// (queue torn down), fall back to the legacy "post completion via message queue"
			// behavior so listeners are still released.
			if (mImpl->CommandBuffer == nil)
			{
				for (const TShared<MetalGpuQueryPool>& pool : mUsedQueryPools)
					pool->MarkSubmissionFailed();
				mUsedQueryPools.clear();
				mQueryPoolsQueuedForSubmission = false;

				id<MTLCommandQueue> mtlQueue = submitQueue.GetMetalQueue();
				id<MTLCommandBuffer> emptyCmdBuffer = mtlQueue ? [mtlQueue commandBuffer] : nil;
				if (emptyCmdBuffer == nil)
				{
					B3D_LOG(Fatal, LogRenderBackend, "Failed to allocate an empty Metal submission command buffer.");
					fnPostFailedSubmissionCompletion();
					return;
				}

				mResourceTracker.NotifyUsed(mSubmittedQueueId);
				mResourcesSubmitted = true;

				// Reuse the shared wait+signal encoder so the empty path observes the exact same
				// cross-queue sync contract as the real path. Without this, any consumer queue that
				// waited on this queue's next signaled value would deadlock when the producer's
				// recording happened to be empty.
				const u64 signalValue = EncodeQueueSyncAndSignal(emptyCmdBuffer, submitQueue, syncMask);

				// Encode any user-provided timeline fence signals on the same throwaway buffer so the
				// device-level fence and any caller-supplied fences advance even
				// when no real work was recorded.
				EncodeUserFenceSignals(emptyCmdBuffer, signalFences);

				TShared<GpuCommandBuffer> selfShared = GetShared();
				TShared<WaitGroup> ownerCompletion = B3DMakeShared<WaitGroup>(1);
				[emptyCmdBuffer addCompletedHandler:^(id<MTLCommandBuffer> completedBuffer)
				{
					LogCommandBufferError(completedBuffer);
					auto* metalSelf = static_cast<MetalGpuCommandBuffer*>(selfShared.get());
					metalSelf->mPool.GetMessageQueue().PostCommand([selfShared, ownerCompletion]()
					{
						auto* owner = static_cast<MetalGpuCommandBuffer*>(selfShared.get());
						owner->mState = GpuCommandBufferState::Done;
						owner->mPool.NotifyCommandBufferReady(owner->mId);
						owner->OnDidComplete();
						// See the fallback path above — release listeners + cached state on the
						// owner thread once completion has been observed.
						owner->Cleanup();
						ownerCompletion->NotifyDone();
					}, "MetalGpuCommandBuffer empty completion");
				}];

				[emptyCmdBuffer commit];

				// A8: publish the committed value after commit returns — same ordering rules as the
				// real path. See comment in the non-empty branch.
				submitQueue.NotifySubmissionCommitted(signalValue, emptyCmdBuffer, ownerCompletion);

				// A6: reset the buffer-level sync mask now that it has been consumed.
				mQueueSyncMask = GpuQueueMask();
				return;
			}

			id<MTLCommandBuffer> cmdBuffer = mImpl->CommandBuffer;

			// Waits appended to an already-recorded command buffer would execute after the work they
			// are meant to protect. Put them in a prologue committed immediately before the work;
			// command buffers on one MTLCommandQueue retain FIFO ordering.
			const GpuQueueMask selfMask = GpuQueueId(submitQueue.GetType(), submitQueue.GetIndex());
			bool needsWaitPrologue = !(syncMask & ~selfMask).IsEmpty();
#if B3D_METAL_USE_EXPLICIT_RESOURCE_SYNCHRONIZATION
			needsWaitPrologue |= submitQueue.GetLastCommittedEventValue() != 0;
#endif
			if (needsWaitPrologue)
			{
				id<MTLCommandQueue> mtlQueue = submitQueue.GetMetalQueue();
				id<MTLCommandBuffer> waitCommandBuffer = mtlQueue ? [mtlQueue commandBuffer] : nil;
				if (waitCommandBuffer == nil)
				{
					B3D_LOG(Fatal, LogRenderBackend, "Failed to allocate Metal cross-queue wait command buffer.");
					fnPostFailedSubmissionCompletion();
					return;
				}

				EncodeQueueWaits(waitCommandBuffer, submitQueue, syncMask);
				[waitCommandBuffer addCompletedHandler:^(id<MTLCommandBuffer> completedBuffer)
				{
					LogCommandBufferError(completedBuffer);
				}];
				[waitCommandBuffer commit];
			}

			mResourceTracker.NotifyUsed(mSubmittedQueueId);
			mResourcesSubmitted = true;

			const u64 signalValue = EncodeQueueSyncAndSignal(cmdBuffer, submitQueue, GpuQueueMask());

			// User-provided timeline fence signals are appended after the queue's own signal so they
			// fire only once all of this command buffer's GPU work has retired (FIFO on the cmd buffer).
			EncodeUserFenceSignals(cmdBuffer, signalFences);

			// The completion handler flips the buffer to Done on GPU finish. It captures a strong
			// pointer so the command buffer stays alive until completion, and posts the state
			// transition + OnDidComplete + Cleanup back to the pool's owner thread so callers see
			// these notifications on the same thread they recorded the buffer on. This matches the
			// bsfVulkanGpuBackend backend's message-queue-back pattern.
			TShared<GpuCommandBuffer> selfShared = GetShared();
			TShared<WaitGroup> ownerCompletion = B3DMakeShared<WaitGroup>(1);
			[cmdBuffer addCompletedHandler:^(id<MTLCommandBuffer> completedBuffer)
			{
				LogCommandBufferError(completedBuffer);
				auto* metalSelf = static_cast<MetalGpuCommandBuffer*>(selfShared.get());
				metalSelf->mPool.GetMessageQueue().PostCommand([selfShared, ownerCompletion]()
				{
					auto* owner = static_cast<MetalGpuCommandBuffer*>(selfShared.get());
					owner->mState = GpuCommandBufferState::Done;
					owner->mPool.NotifyCommandBufferReady(owner->mId);
					owner->OnDidComplete();
					// Release completion listeners and cached recording state on the owner thread;
					// listener closures may hold the last references to GPU resources (e.g.
					// transient buffers) that must not outlive the submission.
					owner->Cleanup();
					ownerCompletion->NotifyDone();
				}, "MetalGpuCommandBuffer completion");
			}];

			[cmdBuffer commit];
			mImpl->CommandBuffer = nil;

			// A8: publish the committed value to the queue so cross-queue waits on this queue observe
			// a race-free high-water mark. Must happen after @c [cmdBuffer commit] returns — before
			// that, a reader would see a value that might never actually be signaled if this thread
			// is preempted between reservation and commit. Uses release-store semantics so subsequent
			// acquire-loads on other queues see the committed value at the right time.
			submitQueue.NotifySubmissionCommitted(signalValue, cmdBuffer, ownerCompletion);

			// A6: reset the buffer-level sync mask now that it has been consumed by this submit.
			// Matches Vulkan's cleanup pattern (B3DVulkanGpuCommandBuffer.cpp) so subsequent
			// Add/Submit cycles on a recycled buffer start with a fresh mask.
			mQueueSyncMask = GpuQueueMask();

			// Notify every query pool that wrote into this command buffer *after* the commit has
			// happened, not before. Previously MarkSubmitted ran before @c [cmdBuffer commit], which
			// left a TOCTOU window: if a resolver thread observed the pool's (queue, value) pair
			// between MarkSubmitted and commit, it might wait on @c [event signaledValue] >= value
			// where value is a reserved counter the GPU never starts — benign in practice because
			// @c TryResolve(wait=true) routes through the listener anyway, but visibly wrong for a
			// wait=false probe. Running MarkSubmitted after commit closes the window cleanly; the
			// signal was already encoded on the committed buffer so the pair is now genuinely
			// in-flight from the GPU's perspective.
			for (const TShared<MetalGpuQueryPool>& pool : mUsedQueryPools)
			{
				pool->MarkSubmitted(submitQueue, signalValue);
				mSubmittedQueryPools.Add(pool);
			}
			mUsedQueryPools.clear();
			mQueryPoolsQueuedForSubmission = false;
			} // @autoreleasepool
		}

		void MetalGpuCommandBuffer::AddUniqueUsedQueryPool(const TShared<MetalGpuQueryPool>& pool)
		{
			// B10: linear scan. Typical cardinality is <= 4 (one visibility pool plus one or two
			// timestamp pools), so the scan is cheaper than the hash-set allocation the old path paid
			// on every BeginQuery / WriteTimestamp.
			for (const TShared<MetalGpuQueryPool>& existing : mUsedQueryPools)
			{
				if (existing.get() == pool.get())
					return;
			}
			pool->MarkRecorded();
			mUsedQueryPools.Add(pool);
		}

		void MetalGpuCommandBuffer::NotifyWillQueueForSubmit()
		{
			for (const TShared<MetalGpuQueryPool>& pool : mUsedQueryPools)
				pool->MarkQueuedForSubmission();
			mQueryPoolsQueuedForSubmission = !mUsedQueryPools.Empty();

			// Clear everything not allowed to be touched from the submit thread (mirrors
			// VulkanGpuCommandBuffer::NotifyWillQueueForSubmit). Runs on the owner thread inside
			// GpuSubmitThread::QueueSubmit, before the submit command is posted. The submit thread
			// only needs the recorded MTLCommandBuffer (mImpl) and mUsedQueryPools — the latter is
			// intentionally kept: CommitInternal fans MarkSubmitted out to those pools after commit.
			mBoundGraphicsPipeline = nullptr;
			mBoundComputePipeline = nullptr;
			mBoundParameterSets.Clear();
			mBoundIndexBuffer = nullptr;
			mBoundVertexDescription = nullptr;
			mActiveOcclusionQueryPool.reset();
			ResetRenderResidencyCaches();
			ResetComputeResidencyCaches();
		}

		void MetalGpuCommandBuffer::Cleanup()
		{
			// Cleanup per the base-class contract (B3DGpuCommandBuffer.h): resource-tracker
			// notify+clear, queue sync mask reset, event clearing, and cached-state teardown. Runs on
			// the owner thread — either from the completion handler's message-queue lambda
			// (CommitInternal) or from pool-level reset / destroy. Clearing OnDidComplete releases
			// listener closures, which may hold the last references to GPU resources (e.g. transient
			// buffers) that must be freed before GpuWorkContext::WaitAndReclaim drains its transient
			// allocators.
			//
			// Resource tracker: mirror VulkanGpuCommandBuffer::Cleanup — a buffer whose resources were
			// promoted to submitted use notifies the tracker its work is done on the submitted queue; one
			// that failed before native submission (or was reset pristine) is notified as unbound. Then
			// the tracked state is cleared.
			if (mResourcesSubmitted)
				mResourceTracker.NotifyDone(mSubmittedQueueId);
			else
				mResourceTracker.NotifyUnbound();

			mResourceTracker.Clear();
			mResourcesSubmitted = false;
			mRecordingFailed = false;
			mBarrierHelper.Clear();

			mQueueSyncMask = GpuQueueMask();
			OnDidComplete.Clear();
			OnDestroyed.Clear();

			// Cached recording state. Usually already cleared by NotifyWillQueueForSubmit(); repeated
			// clears are cheap and keep the pool-reset path (which never goes through a submit)
			// correct as well.
			mBoundGraphicsPipeline = nullptr;
			mBoundComputePipeline = nullptr;
			mBoundParameterSets.Clear();
			mBoundIndexBuffer = nullptr;
			mBoundVertexDescription = nullptr;
			mDrawOperation = DOT_TRIANGLE_LIST;
			mStencilReference = 0;
			mRenderPassPipelineKey = MetalPipelineVariantKey{};
			mAcquiredWindowSurface = nullptr;
			mRenderPassWidth = 0;
			mRenderPassHeight = 0;
			mRenderPassAttachmentImages.Clear();
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
#if !__has_feature(objc_arc)
			for (const Impl::PendingEventSignal& signal : mImpl->PendingEventSignals)
				[signal.Event release];
			[mImpl->RestartRenderPassDescriptor release];
#endif
			mImpl->PendingEventSignals.clear();
			mImpl->RestartRenderPassDescriptor = nil;
			mImpl->VertexBufferBindings.clear();
			mImpl->HasViewport = false;
			mImpl->HasScissor = false;
			mImpl->DebugGroupDepth = 0;
			mImpl->VisibilityMode = MTLVisibilityResultModeDisabled;
			mImpl->VisibilityOffset = 0;
	#if B3D_METAL_USE_EXPLICIT_RESOURCE_SYNCHRONIZATION
			mImpl->FenceNeedsWait = false;
	#endif
			ResetRenderResidencyCaches();
			ResetComputeResidencyCaches();
		}

		void MetalGpuCommandBuffer::NotifyParentPoolReset()
		{
			// Pool-level reset is pure bookkeeping on Metal: the native MTLCommandBuffer is one-shot
			// and was released at commit time (or never acquired). Only completed (Done) or pristine
			// (Ready) buffers may be reset — anything else means the caller violated the
			// GpuCommandBufferPool::Reset contract ("must be called only after all previously
			// allocated command buffers have completed executing").
			if (!B3D_ENSURE(mState == GpuCommandBufferState::Done || mState == GpuCommandBufferState::Ready))
				return;

			if (mState == GpuCommandBufferState::Done)
				Cleanup();

			mState = GpuCommandBufferState::Ready;
		}

		void MetalGpuCommandBuffer::Destroy()
		{
			if (IsDestroyed())
				return;

			// Drop listeners and cached state first — OnDestroyed is cleared here so the base
			// destructor's OnDestroyed(...) fire becomes a no-op, matching the explicit-destroy
			// semantics (the pool has already decided the buffer's fate).
			Cleanup();

			if (mImpl)
			{
				// Close any open encoder and drop the native buffer. If it was never committed, Metal
				// releases it once the strong reference drops.
				CloseAllEncoders(*mImpl);
				mImpl->CommandBuffer = nil;
			}

			GpuCommandBuffer::Destroy();
		}
	} // namespace render
} // namespace b3d
