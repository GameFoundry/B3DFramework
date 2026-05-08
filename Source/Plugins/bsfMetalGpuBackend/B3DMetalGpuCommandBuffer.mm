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
#include "GpuBackend/B3DRenderWindow.h"
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
		};

		namespace
		{
			/**
			 * Shared core of @c AttachArgumentBufferTo{Render,Compute}Encoder. Resolves the argument
			 * buffer / slice offset / set index, then delegates the actual Metal bind call(s) to
			 * @p fnBind so render (two stage-gated calls) and compute (one unconditional call) share
			 * the same null-check + offset-resolution preamble.
			 *
			 * Contract: does @b not commit pending SetX writes and does @b not emit @c useResource: —
			 * both are drain-time responsibilities owned by @c EmitResidencyFor{Render,Compute}Encoder
			 * at draw/dispatch time (B5).
			 */
			template <typename FnBind>
			void AttachArgumentBufferImpl(MetalGpuParameters& params, FnBind&& fnBind)
			{
				const MetalGpuPipelineParameterSetLayout* layout = params.GetMetalLayout();
				if (!layout)
					return;

				id<MTLBuffer> argBuffer = params.GetArgumentBuffer();
				if (argBuffer == nil)
					return;

				const u32 setIndex = params.GetSet();
				// B9: the argument buffer may be a shared pool block; encoder bindings must land at the
				// set's slice origin, not at 0. GetArgumentBufferOffset returns 0 for the non-pooled path.
				const NSUInteger sliceOffset = (NSUInteger)params.GetArgumentBufferOffset();

				fnBind(*layout, argBuffer, sliceOffset, setIndex);
			}

			/**
			 * Attaches a parameter set's argument buffer to a render encoder at the correct set index.
			 * The layout's cached combined-stage mask lets us skip the per-binding walk the old
			 * version had to do (B7).
			 */
			void AttachArgumentBufferToRenderEncoder(id<MTLRenderCommandEncoder> encoder, MetalGpuParameters& params)
			{
				AttachArgumentBufferImpl(params,
					[encoder](const MetalGpuPipelineParameterSetLayout& layout, id<MTLBuffer> argBuffer, NSUInteger sliceOffset, u32 setIndex)
					{
						const u32 combinedStages = layout.GetCombinedStageMask();
						if (combinedStages & (u32)GpuProgramStageBit::Vertex)
							[encoder setVertexBuffer:argBuffer offset:sliceOffset atIndex:setIndex];
						if (combinedStages & (u32)GpuProgramStageBit::Fragment)
							[encoder setFragmentBuffer:argBuffer offset:sliceOffset atIndex:setIndex];
					});
			}

			/** Compute-encoder counterpart of @c AttachArgumentBufferToRenderEncoder. No stage axis. */
			void AttachArgumentBufferToComputeEncoder(id<MTLComputeCommandEncoder> encoder, MetalGpuParameters& params)
			{
				AttachArgumentBufferImpl(params,
					[encoder](const MetalGpuPipelineParameterSetLayout& /*layout*/, id<MTLBuffer> argBuffer, NSUInteger sliceOffset, u32 setIndex)
					{
						[encoder setBuffer:argBuffer offset:sliceOffset atIndex:setIndex];
					});
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

				const auto& bindings = layout->GetBindings();

				for (const auto& bucket : buckets)
				{
					if (bucket.BindingIndices.Empty())
						continue;

					// Small-N (typically @<= 8 resources per bucket). Stack array keeps this hot path
					// allocation-free; the @c kMaxStackResources cap covers the vast majority of
					// real-world sets. Anything over that (bindless-style huge sets) falls back to a
					// scratch @c Vector.
					constexpr u32 kMaxStackResources = 32;
					id<MTLResource> stack[kMaxStackResources];
					Vector<id<MTLResource>> spill;

					id<MTLResource>* resources = stack;
					if (bucket.BindingIndices.Size() > kMaxStackResources)
					{
						spill.resize((size_t)bucket.BindingIndices.Size());
						resources = spill.data();
					}

					u32 resourceCount = 0;
					for (u16 bindingIndex : bucket.BindingIndices)
					{
						// B6: read the resolved resource from the parameter set's per-argIndex cache.
						// Cache is populated by @c CommitPendingBindings; slots whose value changed
						// since the last commit have had their handle refreshed already.
						id<MTLResource> resource = params.GetCachedResourceForArgIndex(bindings[bindingIndex].ArgIndex);
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
					[encoder](const auto& bucket, id<MTLResource>* resources, u32 resourceCount)
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
					[encoder](const auto& bucket, id<MTLResource>* resources, u32 resourceCount)
					{
						[encoder useResources:resources count:resourceCount usage:bucket.Usage];
					});
			}
		} // namespace

		MetalGpuCommandBuffer::MetalGpuCommandBuffer(MetalGpuDevice& device, MetalGpuCommandBufferPool& pool, u32 id, ThreadId ownerThread, GpuQueueType queueType, const GpuCommandBufferCreateInformation& createInformation)
			: GpuCommandBuffer(device, ownerThread, queueType, createInformation)
			, mGpuDevice(device)
			, mPool(pool)
			, mImpl(B3DMakeUnique<Impl>())
			, mId(id)
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
			}
		}

		id<MTLCommandBuffer> MetalGpuCommandBuffer::GetOrAcquireMetalCommandBuffer()
		{
			if (mImpl->CommandBuffer != nil)
				return mImpl->CommandBuffer;

			TShared<GpuQueue> queuePtr = mGpuDevice.GetQueue(mQueueType, 0);
			auto metalQueue = std::static_pointer_cast<MetalGpuQueue>(queuePtr);
			if (!metalQueue)
				return nil;

			id<MTLCommandQueue> mtlQueue = metalQueue->GetMetalQueue();
			if (mtlQueue == nil)
				return nil;

			mImpl->CommandBuffer = [mtlQueue commandBuffer];
			if (!mName.empty())
			{
				NSString* nsName = [NSString stringWithUTF8String:mName.c_str()];
				[mImpl->CommandBuffer setLabel:nsName];
			}

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
				[mImpl->RenderEncoder endEncoding];
				mImpl->RenderEncoder = nil;
				ResetRenderResidencyCaches();
			}
			if (targetKind != EncoderKind::Compute && mImpl->ComputeEncoder != nil)
			{
				[mImpl->ComputeEncoder endEncoding];
				mImpl->ComputeEncoder = nil;
				ResetComputeResidencyCaches();
			}
			if (targetKind != EncoderKind::Blit && mImpl->BlitEncoder != nil)
			{
				[mImpl->BlitEncoder endEncoding];
				mImpl->BlitEncoder = nil;
			}
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
			return mImpl->BlitEncoder;
			} // @autoreleasepool
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

			auto metalParams = std::static_pointer_cast<MetalGpuParameters>(parameters);

			const u32 setIndex = parameters->GetSet();
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
			if (mImpl->RenderEncoder)
				AttachArgumentBufferToRenderEncoder(mImpl->RenderEncoder, *metalParams);
			else if (mImpl->ComputeEncoder)
				AttachArgumentBufferToComputeEncoder(mImpl->ComputeEncoder, *metalParams);
			} // @autoreleasepool
		}

		void MetalGpuCommandBuffer::SetDynamicBufferOffset(u32 set, u32 bufferIndex, u32 offset)
		{
			// B1: SetDynamicOffset invokes into the argument-encoder path which can retain transient
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

				[mImpl->RenderEncoder setVertexBuffer:buffer offset:0 atIndex:(baseIndex + index + bufferIndex)];
			}
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

		static void BindGraphicsPipelineForDraw(MetalGpuCommandBuffer::Impl& impl, MetalGpuGraphicsPipelineState* pipeline, DrawOperationType drawOp, const MetalPipelineVariantKey& renderPassKey)
		{
			if (!pipeline || impl.RenderEncoder == nil)
				return;

			// The render pass populates color/depth/stencil formats and sample count at BeginRenderPass
			// time; the topology class is the one part of the key that varies per draw call.
			MetalPipelineVariantKey key = renderPassKey;
			key.TopologyClass = (u16)MetalUtility::GetPrimitiveTopologyClass(drawOp);

			id<MTLRenderPipelineState> pso = pipeline->GetOrCreateMetalPipeline(key);
			if (pso)
				[impl.RenderEncoder setRenderPipelineState:pso];

			id<MTLDepthStencilState> depthStencil = pipeline->GetMetalDepthStencilState();
			if (depthStencil)
				[impl.RenderEncoder setDepthStencilState:depthStencil];

			[impl.RenderEncoder setCullMode:(MTLCullMode)pipeline->GetCullMode()];
			[impl.RenderEncoder setFrontFacingWinding:(MTLWinding)pipeline->GetWinding()];
			[impl.RenderEncoder setTriangleFillMode:(MTLTriangleFillMode)pipeline->GetFillMode()];

			if (pipeline->GetDepthBias() != 0.0f || pipeline->GetSlopeScaledDepthBias() != 0.0f)
			{
				[impl.RenderEncoder setDepthBias:pipeline->GetDepthBias()
					slopeScale:pipeline->GetSlopeScaledDepthBias()
					clamp:pipeline->GetDepthBiasClamp()];
			}
		}

		void MetalGpuCommandBuffer::Draw(u32 vertexOffset, u32 vertexCount, u32 instanceCount, u32 firstInstance)
		{
			EnsureValidThread();
			if (mImpl->RenderEncoder == nil)
				return;

			// B1: commit + residency emission + draw encode all produce retained Obj-C transients
			// (pipeline state lookups, argument-encoder temporaries, NSArray wrappers for
			// useResources:). Drain locally — the fiber scheduler may go many frames without a
			// runloop tick so accumulation across frames would otherwise be visible.
			@autoreleasepool
			{
			// A'1 + B5: for every bound parameter set, (1) attach the argument buffer on the render
			// encoder, (2) commit pending Set* writes, (3) emit useResource: for every referenced
			// resource. @c SetGpuParameterSet can run before the render encoder exists (the engine's
			// @c BeginRenderPass(target, params) flow), so attaching here — not at bind time — is
			// what guarantees the argument buffer actually lands on the vertex / fragment slot for
			// the first draw. @c setVertexBuffer:offset:atIndex: / @c setFragmentBuffer:…: are
			// idempotent per Apple's documentation so the re-attach every draw is safe.
			//
			// HasPendingBindings short-circuits CommitPendingBindings when the dirty set is empty
			// (set-and-forget bindings cost nothing here).
			for (u32 slotIndex = 0; slotIndex < (u32)mBoundParameterSets.Size(); slotIndex++)
			{
				const TShared<GpuParameterSet>& slotSet = mBoundParameterSets[slotIndex];
				if (!slotSet)
					continue;

				auto metalParams = std::static_pointer_cast<MetalGpuParameters>(slotSet);

				AttachArgumentBufferToRenderEncoder(mImpl->RenderEncoder, *metalParams);

				if (metalParams->HasPendingBindings())
					metalParams->CommitPendingBindings();

				// B3 / A'3: skip the @c useResources: emission loop when the same parameter set at
				// the same generation was already marked resident against the currently-open render
				// encoder. Residency persists for the encoder's lifetime, so re-emitting is pure
				// overhead. A generation bump (triggered by any Set* that genuinely changed a
				// binding) forces a re-emit naturally. Cache is keyed per bound slot so multiple
				// sets on the same encoder don't thrash a single cache entry.
				ParameterSetResidencyCache& cacheEntry = mRenderResidencyCaches[slotIndex];
				const u64 generation = metalParams->GetGeneration();
				const bool cacheHit = cacheEntry.LastBoundSet == metalParams.get()
					&& cacheEntry.LastBoundGeneration == generation;
				if (!cacheHit)
				{
					EmitResidencyForRenderEncoder(mImpl->RenderEncoder, *metalParams);
					cacheEntry.LastBoundSet = metalParams.get();
					cacheEntry.LastBoundGeneration = generation;
				}
			}

			BindGraphicsPipelineForDraw(*mImpl, mBoundGraphicsPipeline.get(), mDrawOperation, mRenderPassPipelineKey);
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

			if (mImpl->RenderEncoder == nil || !mBoundIndexBuffer)
				return;

			// B1: same rationale as @c Draw — drain transient Obj-C objects produced during commit,
			// residency emission, and the draw encode.
			@autoreleasepool
			{
			// A'1 + B5: same rationale as @c Draw — iterate every bound slot, attach + commit +
			// emit-residency per slot. See @c Draw for the full comment.
			for (u32 slotIndex = 0; slotIndex < (u32)mBoundParameterSets.Size(); slotIndex++)
			{
				const TShared<GpuParameterSet>& slotSet = mBoundParameterSets[slotIndex];
				if (!slotSet)
					continue;

				auto metalParams = std::static_pointer_cast<MetalGpuParameters>(slotSet);

				AttachArgumentBufferToRenderEncoder(mImpl->RenderEncoder, *metalParams);

				if (metalParams->HasPendingBindings())
					metalParams->CommitPendingBindings();

				// B3 / A'3: residency-elision cache — see @c Draw. Per-slot keying.
				ParameterSetResidencyCache& cacheEntry = mRenderResidencyCaches[slotIndex];
				const u64 generation = metalParams->GetGeneration();
				const bool cacheHit = cacheEntry.LastBoundSet == metalParams.get()
					&& cacheEntry.LastBoundGeneration == generation;
				if (!cacheHit)
				{
					EmitResidencyForRenderEncoder(mImpl->RenderEncoder, *metalParams);
					cacheEntry.LastBoundSet = metalParams.get();
					cacheEntry.LastBoundGeneration = generation;
				}
			}

			BindGraphicsPipelineForDraw(*mImpl, mBoundGraphicsPipeline.get(), mDrawOperation, mRenderPassPipelineKey);
			[mImpl->RenderEncoder setStencilReferenceValue:mStencilReference];

			auto metalIndex = std::static_pointer_cast<MetalGpuBuffer>(mBoundIndexBuffer);
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
			}

			auto metalCompute = std::static_pointer_cast<MetalGpuComputePipelineState>(mBoundComputePipeline);
			if (!metalCompute)
				return;

			id<MTLComputePipelineState> pso = metalCompute->GetMetalPipeline();
			if (pso == nil)
				return;

			[mImpl->ComputeEncoder setComputePipelineState:pso];

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
				AttachArgumentBufferToComputeEncoder(mImpl->ComputeEncoder, *metalParams);

				// B5: commit-and-emit-residency at dispatch time. Mirrors Draw / DrawIndexed.
				if (metalParams->HasPendingBindings())
					metalParams->CommitPendingBindings();

				// B3 / A'3: residency-elision cache — see @c Draw. Keyed per slot against the
				// compute encoder's lifetime, so a render-then-compute sequence on the same set
				// correctly re-emits once on the compute side.
				ParameterSetResidencyCache& cacheEntry = mComputeResidencyCaches[slotIndex];
				const u64 generation = metalParams->GetGeneration();
				const bool cacheHit = cacheEntry.LastBoundSet == metalParams.get()
					&& cacheEntry.LastBoundGeneration == generation;
				if (!cacheHit)
				{
					EmitResidencyForComputeEncoder(mImpl->ComputeEncoder, *metalParams);
					cacheEntry.LastBoundSet = metalParams.get();
					cacheEntry.LastBoundGeneration = generation;
				}
			}

			const u32* workgroup = metalCompute->GetWorkgroupSize();
			MTLSize threadsPerGroup = MTLSizeMake(workgroup[0], workgroup[1], workgroup[2]);
			MTLSize groups = MTLSizeMake(groupCountX, groupCountY, groupCountZ);
			[mImpl->ComputeEncoder dispatchThreadgroups:groups threadsPerThreadgroup:threadsPerGroup];
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

			const TShared<RenderTarget>& target = createInformation.Target;
			if (!target)
				return;

			id<MTLCommandBuffer> cmdBuffer = GetOrAcquireMetalCommandBuffer();
			if (cmdBuffer == nil)
				return;

			MTLRenderPassDescriptor* desc = [MTLRenderPassDescriptor renderPassDescriptor];

			// Occlusion queries require the visibility buffer to be attached at encoder creation time.
			// The engine stages the pool via SetPendingVisibilityPool before BeginRenderPass; if a pool is
			// present we record it in UsedQueryPools now so CommitInternal can notify it regardless of
			// whether any BeginQuery calls land inside the pass.
			if (mPendingVisibilityPool)
			{
				id<MTLBuffer> visibilityBuffer = mPendingVisibilityPool->GetVisibilityBuffer();
				if (visibilityBuffer != nil)
				{
					desc.visibilityResultBuffer = visibilityBuffer;
					AddUniqueUsedQueryPool(mPendingVisibilityPool.get());
				}
			}

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
				auto metalSurface = std::static_pointer_cast<MetalRenderWindowSurface>(windowSurface);
				if (!metalSurface)
				{
					B3D_LOG(Error, LogRenderBackend, "BeginRenderPass: render window has no Metal surface attached.");
					return;
				}

				id<CAMetalDrawable> drawable = metalSurface->AcquireDrawable();
				if (drawable == nil)
				{
					B3D_LOG(Error, LogRenderBackend, "BeginRenderPass: failed to acquire a CAMetalDrawable.");
					return;
				}

				mAcquiredWindowSurface = metalSurface.get();

				MTLRenderPassColorAttachmentDescriptor* color = desc.colorAttachments[0];
				color.texture = drawable.texture;
				color.level = 0;
				color.slice = 0;
				color.depthPlane = 0;
				ConfigureColorAttachmentActions(color, loadMask.IsSet(RT_COLOR0), clearMask.IsSet(RT_COLOR0), createInformation.ClearColor);

				PackColorFormat(mRenderPassPipelineKey.ColorFormats, 0, metalSurface->GetColorFormat());
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
					}
				}
			}

			mImpl->RenderEncoder = [cmdBuffer renderCommandEncoderWithDescriptor:desc];

			// Guard the "drawable acquired but encoder creation failed" path. Without this, the
			// CAMetalLayer drawable we pulled via @c AcquireDrawable would remain held until the
			// next successful @c SwapBuffers, stalling the drawable pool for one or more frames
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
				if (setIndex >= (u32)mBoundParameterSets.Size())
				{
					mBoundParameterSets.Resize(setIndex + 1);
					mRenderResidencyCaches.Resize(setIndex + 1);
					mComputeResidencyCaches.Resize(setIndex + 1);
				}
				mBoundParameterSets[setIndex] = paramSet;

				auto metalParams = std::static_pointer_cast<MetalGpuParameters>(paramSet);
				AttachArgumentBufferToRenderEncoder(mImpl->RenderEncoder, *metalParams);
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
				[mImpl->RenderEncoder endEncoding];
				mImpl->RenderEncoder = nil;
				ResetRenderResidencyCaches();
			}
			mAcquiredWindowSurface = nullptr;
			mRenderPassPipelineKey = MetalPipelineVariantKey{};
			mRenderPassWidth = 0;
			mRenderPassHeight = 0;
			mPendingVisibilityPool.reset();
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
			[mImpl->RenderEncoder setViewport:vp];
		}

		void MetalGpuCommandBuffer::ClearRenderTarget(RenderSurfaceMask mask, const Color& color, float depth, u16 stencil)
		{
			// In Metal, clears are performed as part of the render pass descriptor's load action. A mid-
			// render-pass clear requires a full-screen clear pipeline; not yet implemented.
			(void)mask; (void)color; (void)depth; (void)stencil;
		}

		void MetalGpuCommandBuffer::ClearViewport(RenderSurfaceMask mask, const Color& color, float depth, u16 stencil)
		{
			// Same constraint as ClearRenderTarget. Viewport-scoped clears need a full-screen pipeline
			// with a scissor rect. Deferred.
			(void)mask; (void)color; (void)depth; (void)stencil;
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
			[mImpl->RenderEncoder setScissorRect:rect];
		}

		void MetalGpuCommandBuffer::SetStencilReferenceValue(u32 value)
		{
			EnsureValidThread();
			mStencilReference = value;
			if (mImpl->RenderEncoder)
				[mImpl->RenderEncoder setStencilReferenceValue:value];
		}

		void MetalGpuCommandBuffer::CopyBufferToBuffer(const TShared<GpuBuffer>& source, const TShared<GpuBuffer>& destination, u32 sourceOffset, u32 destinationOffset, u32 length)
		{
			EnsureValidThread();

			// Funnel through GetOrOpenBlitEncoder — it handles closing render/compute encoders,
			// acquiring the command buffer, and wrapping in an @autoreleasepool uniformly across copy
			// paths. Re-implementing the same sequence inline was the source of subtle divergence
			// (missing @autoreleasepool, copy-pasted error strings drifting apart).
			id<MTLBlitCommandEncoder> blit = GetOrOpenBlitEncoder();
			if (blit == nil)
				return;

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

			id<MTLBlitCommandEncoder> blit = GetOrOpenBlitEncoder();
			if (blit == nil)
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

			const TextureProperties& props = destination->GetProperties();
			const u32 mipWidth = std::max(1u, props.Width >> mipLevel);
			const u32 mipHeight = std::max(1u, props.Height >> mipLevel);
			const u32 mipDepth = props.Type == TEX_TYPE_3D ? std::max(1u, props.Depth >> mipLevel) : 1u;

			const u32 rowPitch = MetalUtility::GetTextureRowPitch(props.Format, mipWidth);
			const u32 slicePitch = MetalUtility::GetTextureSlicePitch(props.Format, mipWidth, mipHeight);

			[blit copyFromBuffer:src
				sourceOffset:bufferOffset
				sourceBytesPerRow:rowPitch
				sourceBytesPerImage:slicePitch
				sourceSize:MTLSizeMake(mipWidth, mipHeight, mipDepth)
				toTexture:dst
				destinationSlice:arrayLayer
				destinationLevel:mipLevel
				destinationOrigin:MTLOriginMake(0, 0, 0)];
		}

		void MetalGpuCommandBuffer::CopyTextureToBuffer(const TShared<Texture>& source, const TShared<GpuBuffer>& destination, u32 mipLevel, u32 arrayLayer, u32 bufferOffset)
		{
			EnsureValidThread();
			if (!source || !destination)
				return;

			id<MTLBlitCommandEncoder> blit = GetOrOpenBlitEncoder();
			if (blit == nil)
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

			const TextureProperties& props = source->GetProperties();
			const u32 mipWidth = std::max(1u, props.Width >> mipLevel);
			const u32 mipHeight = std::max(1u, props.Height >> mipLevel);
			const u32 mipDepth = props.Type == TEX_TYPE_3D ? std::max(1u, props.Depth >> mipLevel) : 1u;

			const u32 rowPitch = MetalUtility::GetTextureRowPitch(props.Format, mipWidth);
			const u32 slicePitch = MetalUtility::GetTextureSlicePitch(props.Format, mipWidth, mipHeight);

			[blit copyFromTexture:src
				sourceSlice:arrayLayer
				sourceLevel:mipLevel
				sourceOrigin:MTLOriginMake(0, 0, 0)
				sourceSize:MTLSizeMake(mipWidth, mipHeight, mipDepth)
				toBuffer:dst
				destinationOffset:bufferOffset
				destinationBytesPerRow:rowPitch
				destinationBytesPerImage:slicePitch];
		}

		void MetalGpuCommandBuffer::SetPendingVisibilityPool(const TShared<GpuQueryPool>& queryPool)
		{
			EnsureValidThread();

			if (!queryPool)
			{
				mPendingVisibilityPool.reset();
				return;
			}

			if (queryPool->GetQueryType() != GpuQueryType::Occlusion)
			{
				B3D_LOG(Error, LogRenderBackend,
					"SetPendingVisibilityPool: pool type is not Occlusion; ignoring.");
				mPendingVisibilityPool.reset();
				return;
			}

			mPendingVisibilityPool = std::static_pointer_cast<MetalGpuQueryPool>(queryPool);
		}

		void MetalGpuCommandBuffer::WriteTimestamp(GpuQueryId query, const TShared<GpuQueryPool>& queryPool)
		{
			EnsureValidThread();

			if (!queryPool || !query.IsValid())
				return;

			auto metalPool = std::static_pointer_cast<MetalGpuQueryPool>(queryPool);
			id<MTLCounterSampleBuffer> counterBuffer = metalPool->GetCounterBuffer();
			if (counterBuffer == nil)
				return;

			// Metal samples timestamps via the active encoder. The caller is expected to issue WriteTimestamp
			// while an encoder (render / compute / blit) is open; otherwise the sample is dropped. This is
			// a phase-2 limitation — bracketing a timestamp around a pass boundary requires sample-buffer
			// attachments on the render-pass descriptor, which is not wired here.
			//
			// Each encoder variant additionally requires the matching per-encoder sampling point to be
			// advertised by the device (Draw / Dispatch / Blit boundary). Apple Silicon typically only
			// advertises the stage boundary, so the render-encoder branch must be skipped even though
			// RSC_TIMER_QUERIES is set. Fall through to the next supported encoder; if none qualify,
			// drop the sample with a warn-once log.
			const bool renderOk = mImpl->RenderEncoder != nil && mGpuDevice.SupportsRenderEncoderTimestamps();
			const bool computeOk = mImpl->ComputeEncoder != nil && mGpuDevice.SupportsComputeEncoderTimestamps();
			const bool blitOk = mImpl->BlitEncoder != nil && mGpuDevice.SupportsBlitEncoderTimestamps();

			if (renderOk)
				[mImpl->RenderEncoder sampleCountersInBuffer:counterBuffer atSampleIndex:query.Id withBarrier:YES];
			else if (computeOk)
				[mImpl->ComputeEncoder sampleCountersInBuffer:counterBuffer atSampleIndex:query.Id withBarrier:YES];
			else if (blitOk)
				[mImpl->BlitEncoder sampleCountersInBuffer:counterBuffer atSampleIndex:query.Id withBarrier:YES];
			else
			{
				// @c std::atomic<bool>::exchange ensures only the first thread / fiber to observe
				// false wins the race and emits the log. Command buffers are thread-affine per the
				// engine's threading model, but multiple command buffers across worker fibers may
				// reach this branch concurrently on different devices.
				static std::atomic<bool> sWarnedOnce{false};
				if (!sWarnedOnce.exchange(true, std::memory_order_relaxed))
				{
					const bool hasAnyEncoder = mImpl->RenderEncoder != nil || mImpl->ComputeEncoder != nil || mImpl->BlitEncoder != nil;
					if (hasAnyEncoder)
					{
						B3D_LOG(Warning, LogRenderBackend,
							"MetalGpuCommandBuffer::WriteTimestamp: the active encoder's sampling point "
							"(Draw / Dispatch / Blit boundary) is not supported by this device. Timestamps "
							"inside such encoders are dropped. Subsequent samples will be silently ignored.");
					}
					else
					{
						B3D_LOG(Warning, LogRenderBackend,
							"MetalGpuCommandBuffer::WriteTimestamp called with no active encoder; sample will be missed.");
					}
				}
				return;
			}

			AddUniqueUsedQueryPool(metalPool.get());
		}

		void MetalGpuCommandBuffer::BeginQuery(GpuQueryId query, const TShared<GpuQueryPool>& queryPool, GpuQueryFlags flags)
		{
			EnsureValidThread();

			if (!queryPool || !query.IsValid() || mImpl->RenderEncoder == nil)
				return;

			auto metalPool = std::static_pointer_cast<MetalGpuQueryPool>(queryPool);

			// Counting mode returns the exact sample count per query; Boolean returns 0 or 1. We use the
			// precise flag to drive the distinction. Either way, the visibility buffer must have been
			// attached to the pass via SetPendingVisibilityPool before the render pass started — otherwise
			// the encoder has nowhere to write the result.
			const MTLVisibilityResultMode mode = flags.IsSet(GpuQueryFlag::PreciseOcclusion)
				? MTLVisibilityResultModeCounting
				: MTLVisibilityResultModeBoolean;
			[mImpl->RenderEncoder setVisibilityResultMode:mode offset:metalPool->GetQueryOffset(query)];

			AddUniqueUsedQueryPool(metalPool.get());
		}

		void MetalGpuCommandBuffer::EndQuery(GpuQueryId query, const TShared<GpuQueryPool>& queryPool)
		{
			EnsureValidThread();
			(void)query;
			(void)queryPool;

			if (mImpl->RenderEncoder == nil)
				return;

			[mImpl->RenderEncoder setVisibilityResultMode:MTLVisibilityResultModeDisabled offset:0];
		}

		void MetalGpuCommandBuffer::ResetQueries(const TShared<GpuQueryPool>& queryPool)
		{
			EnsureValidThread();

			if (!queryPool)
				return;

			auto metalPool = std::static_pointer_cast<MetalGpuQueryPool>(queryPool);
			if (id<MTLBuffer> visibilityBuffer = metalPool->GetVisibilityBuffer())
			{
				// The visibility buffer is MTLStorageModeShared and thus CPU/GPU-coherent on Apple
				// Silicon unified memory. Callers are expected to have drained any in-flight writes
				// (via WaitUntilIdle or a pool-level TryResolve(wait=true)) before reset; if they
				// didn't, memsetting the buffer while the GPU is still writing to it is a data race
				// that corrupts visibility results. TryResolve(false) reports whether every recorded
				// submission has signaled — if it returns false, the caller violated the contract and
				// proceeding with the memset would introduce silent corruption. Log and bail out.
				//
				// TODO: replace this CPU memset with a GPU-side @c fillBuffer:range:value:0 blit. A
				// blit-encoded clear is naturally ordered against subsequent writes via the command
				// stream and would remove the caller-side synchronization requirement entirely.
				if (!metalPool->TryResolve(false))
				{
					// Atomic exchange so concurrent ResetQueries callers on different worker fibers
					// can't both win the race and log twice. See matching pattern in WriteTimestamp.
					static std::atomic<bool> sWarnedOnce{false};
					if (!sWarnedOnce.exchange(true, std::memory_order_relaxed))
					{
						B3D_LOG(Error, LogRenderBackend,
							"ResetQueries called while the visibility buffer may still be written by "
							"the GPU. Skipping the CPU memset to avoid corrupting in-flight results; "
							"the query pool's contents are now undefined until the caller drains prior "
							"submissions and re-issues ResetQueries. Subsequent violations will be silent.");
					}
					return;
				}
				memset([visibilityBuffer contents], 0, [visibilityBuffer length]);
			}

			// The allocator lives CPU-side, so resetting is a plain counter bump — no need to gate on GPU
			// completion. Callers are expected to have waited for in-flight reads before calling reset.
			metalPool->ResetNextQueryId();
		}

		void MetalGpuCommandBuffer::BeginLabel(const StringView& name)
		{
			if (!mImpl->CommandBuffer || name.empty())
				return;

			NSString* nsName = [[NSString alloc] initWithBytes:name.data() length:name.size() encoding:NSUTF8StringEncoding];
			// Prefer the currently-open encoder so the debug group nests inside the encoder's scope
			// in GPU capture tools; fall back to the command buffer when no encoder is live.
			id<MTLCommandEncoder> encoder = GetActiveEncoder();
			if (encoder != nil)
				[encoder pushDebugGroup:nsName];
			else
				[mImpl->CommandBuffer pushDebugGroup:nsName];
#if !__has_feature(objc_arc)
			[nsName release];
#endif
		}

		void MetalGpuCommandBuffer::EndLabel()
		{
			// Symmetric with @c BeginLabel: pop on the encoder if one is open, else on the command
			// buffer. Extra guard on the command buffer matches the original behavior (no-op before
			// the buffer is acquired).
			id<MTLCommandEncoder> encoder = GetActiveEncoder();
			if (encoder != nil)
				[encoder popDebugGroup];
			else if (mImpl->CommandBuffer)
				[mImpl->CommandBuffer popDebugGroup];
		}

		void MetalGpuCommandBuffer::InsertLabel(const StringView& name)
		{
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
		}

		void MetalGpuCommandBuffer::End()
		{
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

			// Metal's hazard tracker handles dependencies between tracked resources automatically, but
			// the resources that live inside an argument buffer are bound via @c useResource: and are
			// considered untracked. For those, the application has to emit an explicit memory barrier
			// when switching between producer and consumer dispatches. The engine's @c GpuBarriers
			// descriptor does not yet carry enough information to scope the barrier to a specific stage
			// or resource set, so issue a full between-stages barrier on the active encoder. A future
			// refinement should consult the barrier descriptor and emit only the scopes required.
			(void)barriers;
			// TODO: phase-2 review S14 + S37 — DEFERRED.
			// The full plan called for an engine-side resource tracker (B3DMetalResourceTracker) +
			// barrier helper (B3DMetalBarrierHelper) modeled on the Vulkan backend, plus flipping every
			// MTLTexture / MTLBuffer to @c MTLHazardTrackingModeUntracked. That combination would:
			//   (a) let us emit finer-scoped memory barriers than the current @c Buffers|Textures,
			//       @c Fragment→Vertex fallback (this function), and
			//   (b) unlock the Untracked-mode perf win (driver stops bookkeeping per bind).
			// Not required for correctness today: every MTLTexture / MTLBuffer currently uses the
			// default @c hazardTrackingMode (→ Tracked on macOS), and @c useResource: ties argument-
			// buffer-referenced resources into that auto-tracking path. So the phase-2 backend is
			// already safe; S14 + S37 buy perf + scope, not correctness.
			// Revisit once the macOS smoke test is green and we have profiling data that justifies
			// the ~1000-LOC subsystem. See S14 and S37 in the phase-2 review plan.

			if (mImpl->RenderEncoder != nil)
			{
				[mImpl->RenderEncoder memoryBarrierWithScope:MTLBarrierScopeBuffers | MTLBarrierScopeTextures
					afterStages:MTLRenderStageFragment
					beforeStages:MTLRenderStageVertex];
			}
			else if (mImpl->ComputeEncoder != nil)
			{
				[mImpl->ComputeEncoder memoryBarrierWithScope:MTLBarrierScopeBuffers | MTLBarrierScopeTextures];
			}
		}

		u64 MetalGpuCommandBuffer::EncodeQueueSyncAndSignal(id<MTLCommandBuffer> cmdBuffer, MetalGpuQueue& submitQueue, GpuQueueMask syncMask)
		{
			// --- A7: ordering note for @c encodeWaitForEvent: / @c encodeSignalEvent: at commit time ---
			// Both calls below are command-buffer-level (not encoder-level) APIs. Metal documents that
			// they insert a sync point at the current position in the recorded command stream and
			// requires no encoder to be active at the call site. The caller in @c CommitInternal closed
			// every render/compute/blit encoder via @c EnsureEncoderKind before invoking this helper;
			// the empty-command-buffer path never opened one. The sync points are ordered against the
			// previously encoded work in-buffer: waits stall the GPU before any subsequent command
			// executes, and signals fire when the GPU has finished every preceding command in this
			// buffer. Ordering is identical to encoding them at the start of recording — the command
			// buffer is a FIFO. Metal 4's @c MTL4CommandQueue offers separate @c waitForEvent/signalEvent
			// APIs with the same semantics and would be adopted alongside the Metal-4 migration.
			// -----------------------------------------------------------------------------------------

			// Encode cross-queue waits: for every queue in the sync mask that isn't the submitting one,
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
			// The commit path creates several autoreleased Obj-C objects (completion-handler block,
			// NSArray of encoded waits, label strings). Draining locally guarantees they don't
			// accumulate across the fiber-scheduled frames that never hit a runloop.
			@autoreleasepool
			{
			// A'8: close any still-open encoder without touching @c mState. The public @c End()
			// would flip the buffer into @c RecordingDone, but we intentionally do not route through
			// it — the submit path is the transition authority for @c Executing → (handler) @c Done
			// and must not observe an intermediate @c RecordingDone flip of its own making. Note
			// that the caller usually has already invoked @c End() on this path; calling
			// @c EnsureEncoderKind a second time is a cheap no-op when every encoder is already
			// closed.
			EnsureEncoderKind(EncoderKind::None);

			// A'9: the empty path still has to go through the queue's event so cross-queue waits
			// the engine accumulated via @c AddQueueSyncMask are honored. Acquire a throwaway
			// MTLCommandBuffer from the submit queue, encode the waits + signal into it, install
			// the same completion handler as the real path, and commit. If queue acquisition fails
			// (queue torn down), fall back to the legacy "post completion via message queue"
			// behavior so listeners are still released.
			if (mImpl->CommandBuffer == nil)
			{
				// No real work was recorded, so any query-pool tracking is stale; drop it without
				// notifying so TryResolve continues to see the pool as unsubmitted.
				mUsedQueryPools.clear();

				id<MTLCommandQueue> mtlQueue = submitQueue.GetMetalQueue();
				id<MTLCommandBuffer> emptyCmdBuffer = mtlQueue ? [mtlQueue commandBuffer] : nil;
				if (emptyCmdBuffer == nil)
				{
					// A6: reset the buffer-level sync mask even on this fallback path — the caller's
					// AddQueueSyncMask accumulation is considered consumed by this Submit/Commit cycle,
					// matching the Vulkan cleanup semantics.
					mQueueSyncMask = GpuQueueMask();

					TShared<GpuCommandBuffer> selfShared = GetShared();
					mPool.GetMessageQueue().PostCommand([selfShared]()
					{
						auto* metalSelf = static_cast<MetalGpuCommandBuffer*>(selfShared.get());
						metalSelf->mState = GpuCommandBufferState::Done;
						metalSelf->mPool.NotifyCommandBufferReady(metalSelf->mId);
						metalSelf->OnDidComplete();
					}, "MetalGpuCommandBuffer empty completion");
					return;
				}

				// Reuse the shared wait+signal encoder so the empty path observes the exact same
				// cross-queue sync contract as the real path. Without this, any consumer queue that
				// waited on this queue's next signaled value would deadlock when the producer's
				// recording happened to be empty.
				const u64 signalValue = EncodeQueueSyncAndSignal(emptyCmdBuffer, submitQueue, syncMask);

				// Encode any user-provided timeline fence signals on the same throwaway buffer so the
				// device-level fence and any caller-supplied fences advance even
				// when no real work was recorded.
				EncodeUserFenceSignals(emptyCmdBuffer, signalFences);

				mState = GpuCommandBufferState::Executing;

				TShared<GpuCommandBuffer> selfShared = GetShared();
				[emptyCmdBuffer addCompletedHandler:^(id<MTLCommandBuffer>)
				{
					auto* metalSelf = static_cast<MetalGpuCommandBuffer*>(selfShared.get());
					metalSelf->mPool.GetMessageQueue().PostCommand([selfShared]()
					{
						auto* owner = static_cast<MetalGpuCommandBuffer*>(selfShared.get());
						owner->mState = GpuCommandBufferState::Done;
						owner->mPool.NotifyCommandBufferReady(owner->mId);
						owner->OnDidComplete();
					}, "MetalGpuCommandBuffer empty completion");
				}];

				[emptyCmdBuffer commit];

				// A8: publish the committed value after commit returns — same ordering rules as the
				// real path. See comment in the non-empty branch.
				submitQueue.NotifySubmissionCommitted(signalValue);

				// A6: reset the buffer-level sync mask now that it has been consumed.
				mQueueSyncMask = GpuQueueMask();
				return;
			}

			id<MTLCommandBuffer> cmdBuffer = mImpl->CommandBuffer;

			// A'9: cross-queue wait encoding and signal reservation shared with the empty path.
			const u64 signalValue = EncodeQueueSyncAndSignal(cmdBuffer, submitQueue, syncMask);

			// User-provided timeline fence signals are appended after the queue's own signal so they
			// fire only once all of this command buffer's GPU work has retired (FIFO on the cmd buffer).
			EncodeUserFenceSignals(cmdBuffer, signalFences);

			// Transition through Executing; the completion handler flips to Done on GPU finish. The
			// handler captures a strong pointer so the command buffer stays alive until completion, and
			// posts the state transition + OnDidComplete back to the pool's owner thread so callers see
			// these notifications on the same thread they recorded the buffer on. This matches the
			// bsfVulkanGpuBackend backend's message-queue-back pattern.
			mState = GpuCommandBufferState::Executing;

			TShared<GpuCommandBuffer> selfShared = GetShared();
			[cmdBuffer addCompletedHandler:^(id<MTLCommandBuffer>)
			{
				auto* metalSelf = static_cast<MetalGpuCommandBuffer*>(selfShared.get());
				metalSelf->mPool.GetMessageQueue().PostCommand([selfShared]()
				{
					auto* owner = static_cast<MetalGpuCommandBuffer*>(selfShared.get());
					owner->mState = GpuCommandBufferState::Done;
					owner->mPool.NotifyCommandBufferReady(owner->mId);
					owner->OnDidComplete();
				}, "MetalGpuCommandBuffer completion");
			}];

			[cmdBuffer commit];
			mImpl->CommandBuffer = nil;

			// A8: publish the committed value to the queue so cross-queue waits on this queue observe
			// a race-free high-water mark. Must happen after @c [cmdBuffer commit] returns — before
			// that, a reader would see a value that might never actually be signaled if this thread
			// is preempted between reservation and commit. Uses release-store semantics so subsequent
			// acquire-loads on other queues see the committed value at the right time.
			submitQueue.NotifySubmissionCommitted(signalValue);

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
			for (MetalGpuQueryPool* pool : mUsedQueryPools)
				pool->MarkSubmitted(submitQueue, signalValue);
			mUsedQueryPools.clear();
			} // @autoreleasepool
		}

		void MetalGpuCommandBuffer::AddUniqueUsedQueryPool(MetalGpuQueryPool* pool)
		{
			// B10: linear scan. Typical cardinality is <= 4 (one visibility pool plus one or two
			// timestamp pools), so the scan is cheaper than the hash-set allocation the old path paid
			// on every BeginQuery / WriteTimestamp.
			for (MetalGpuQueryPool* existing : mUsedQueryPools)
			{
				if (existing == pool)
					return;
			}
			mUsedQueryPools.Add(pool);
		}
	} // namespace render
} // namespace b3d
