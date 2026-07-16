//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DMetalPrerequisites.h"
#include "B3DMetalGpuPipelineState.h"
#include "B3DMetalResourceTracker.h"
#include "B3DMetalBarrierHelper.h"
#include "GpuBackend/B3DGpuCommandBuffer.h"
#include "GpuBackend/B3DGpuTimelineFence.h"

namespace b3d
{
	namespace render
	{
		class MetalGpuDevice;
		class MetalGpuQueue;
		class MetalGpuCommandBufferPool;
		class MetalGpuParameters;
		class MetalGpuQueryPool;
		class IMetalRenderWindowSurface;
		class MetalVertexInput;
		class MetalImage;

		/** @addtogroup MetalGpuBackend
		 *  @{
		 */

		/**
		 * Metal implementation of a GPU command buffer.
		 *
		 * Owns a single @c MTLCommandBuffer acquired from the owning queue. At most one encoder (render,
		 * compute, or blit) is active at a time; @c EnsureEncoderKind implicitly closes any mismatched
		 * encoder when transitioning to a new operation.
		 */
		class MetalGpuCommandBuffer final : public GpuCommandBuffer
		{
		public:
			MetalGpuCommandBuffer(MetalGpuDevice& device, MetalGpuCommandBufferPool& pool, u32 id, ThreadId ownerThread, GpuQueueType queueType, const GpuCommandBufferCreateInformation& createInformation);
			~MetalGpuCommandBuffer() override;

			void SetName(const StringView& name) override;

			void SetGpuParameterSet(const TShared<GpuParameterSet>& parameters) override;
			void SetDynamicBufferOffset(u32 set, u32 bufferIndex, u32 offset) override;
			void SetGpuGraphicsPipelineState(const TShared<GpuGraphicsPipelineState>& pipelineState) override;
			void SetGpuComputePipelineState(const TShared<GpuComputePipelineState>& pipelineState) override;
			void SetVertexBuffers(u32 index, TShared<GpuBuffer>* buffers, u32 bufferCount) override;
			void SetIndexBuffer(const TShared<GpuBuffer>& buffer) override;
			void SetVertexDescription(const TShared<VertexDescription>& vertexDescription) override;
			void SetDrawOperation(DrawOperationType operation) override;
			void Draw(u32 vertexOffset, u32 vertexCount, u32 instanceCount, u32 firstInstance) override;
			void DrawIndexed(u32 startIndex, u32 indexCount, u32 vertexOffset, u32 vertexCount, u32 instanceCount, u32 firstInstance) override;
			void DispatchCompute(u32 groupCountX, u32 groupCountY, u32 groupCountZ) override;
			void BeginRenderPass(const RenderPassCreateInformation& createInformation) override;
			void EndRenderPass() override;
			bool IsInRenderPass() const override;
			void SetViewport(const Area2& area) override;
			void ClearRenderTarget(RenderSurfaceMask mask, const Color& color, float depth, u16 stencil) override;
			void ClearViewport(RenderSurfaceMask mask, const Color& color, float depth, u16 stencil) override;
			void EnableScissorTest(u32 left, u32 top, u32 right, u32 bottom) override;
			void DisableScissorTest() override;
			void SetStencilReferenceValue(u32 value) override;
			void CopyBufferToBuffer(const TShared<GpuBuffer>& source, const TShared<GpuBuffer>& destination, u32 sourceOffset, u32 destinationOffset, u32 length) override;
			void CopyBufferToTexture(const TShared<GpuBuffer>& source, const TShared<Texture>& destination, u32 bufferOffset, u32 mipLevel, u32 arrayLayer) override;
			void CopyTextureToBuffer(const TShared<Texture>& source, const TShared<GpuBuffer>& destination, u32 mipLevel, u32 arrayLayer, u32 bufferOffset) override;
			bool CopyTexture(const TShared<Texture>& source, const TShared<Texture>& destination, const TextureCopyInformation& copyInformation) override;
			bool BlitTexture(const TShared<Texture>& source, const TShared<Texture>& destination, const TextureBlitInformation& blitInformation) override;
			void WriteTimestamp(GpuQueryId query, const TShared<GpuQueryPool>& queryPool) override;
			void BeginQuery(GpuQueryId query, const TShared<GpuQueryPool>& queryPool, GpuQueryFlags flags) override;
			void EndQuery(GpuQueryId query, const TShared<GpuQueryPool>& queryPool) override;
			void ResetQueries(const TShared<GpuQueryPool>& queryPool) override;
			void BeginLabel(const StringView& name) override;
			void EndLabel() override;
			void InsertLabel(const StringView& name) override;
			void End() override;
			void IssueBarriers(const GpuBarriers& barriers) override;

			/** Returns a unique identifier of this command buffer. */
			u32 GetId() const { return mId; }

			/**
			 * Called on the owner thread just before the command buffer is queued for submission on
			 * the submit thread (via IGpuSubmitThreadBackend::NotifyWillQueueForSubmit). Releases
			 * cached recording state that must not be touched from the submit thread — the submit
			 * thread only needs the recorded MTLCommandBuffer and the used-query-pool list. Mirrors
			 * VulkanGpuCommandBuffer::NotifyWillQueueForSubmit.
			 */
			void NotifyWillQueueForSubmit();

			/**
			 * Notifies the command buffer that the pool it was allocated from has been reset
			 * (GpuCommandBufferPool::Reset). The underlying MTLCommandBuffer is one-shot and already
			 * released at commit time, so this is pure bookkeeping: buffers in @c Done state are
			 * cleaned up and returned to @c Ready; buffers already @c Ready are left alone. Any other
			 * state is a caller contract violation (the pool must only be reset once all of its
			 * buffers have finished executing) and is reported via B3D_ENSURE.
			 */
			void NotifyParentPoolReset();

			void Cleanup() override;
			void Destroy() override;

#ifdef __OBJC__
			/**
			 * Commits the MTLCommandBuffer (if any) onto the provided queue.
			 *
			 * Before submission the command buffer is marked so its completion handler transitions the
			 * state machine to @c Executing/@c Done and notifies @c OnDidComplete listeners.
			 *
			 * The @p syncMask encodes cross-queue waits: for every queue in the mask other than the
			 * submitting one, a wait on that queue's shared-event value is prepended, and this queue's
			 * event value is signaled at the end of the command buffer.
			 *
			 * @p signalFences are user-provided timeline fences whose MTLSharedEvent is signaled with
			 * the requested value once this command buffer's GPU work completes. The signals are encoded
			 * after the queue's own signal so they observe the same FIFO ordering as cross-queue sync.
			 *
			 * @note	Submit thread only. Invoked via GpuSubmitThread -> MetalGpuDevice::ExecuteSubmit.
			 *			The owner thread must already have released its recording state through
			 *			NotifyWillQueueForSubmit() and transitioned the buffer to @c Executing
			 *			(MetalGpuQueue::SubmitCommandBuffer does both via the submit-thread hand-off).
			 */
			void CommitInternal(MetalGpuQueue& submitQueue, GpuQueueMask syncMask, TArrayView<const GpuTimelineFenceAndValue> signalFences = {});

			/** Returns the underlying MTLCommandBuffer, acquiring it lazily if needed. */
			id<MTLCommandBuffer> GetOrAcquireMetalCommandBuffer();

			/**
			 * Closes any open render / compute encoder and returns a blit encoder bound to this command
			 * buffer, opening one if needed. Used by external helpers (notably @c MetalRenderWindowSurface
			 * for @c ReadAsync) to append blit work onto the same command buffer the caller is recording
			 * into, keeping ordering intact with the rest of the recorded commands.
			 */
			id<MTLBlitCommandEncoder> GetOrOpenBlitEncoder();

			/** Encodes an event signal at the current command-stream position without violating encoder scope rules. */
			bool EncodeSignalEvent(id<MTLSharedEvent> event, u64 value);
#endif

		private:
			friend class MetalGpuCommandBufferPool;
			friend class MetalGpuQueue;

			enum class EncoderKind
			{
				None,
				Render,
				Compute,
				Blit
			};

			/** Sets the command buffer state. Only accessible by friends (pool and queue). */
			void SetState(GpuCommandBufferState state) { mState = state; }

#ifdef __OBJC__
			/**
			 * Closes any currently-active encoder whose kind does not match @p targetKind and resets
			 * every per-slot residency cache entry for that encoder via @c ResetRenderResidencyCaches
			 * / @c ResetComputeResidencyCaches. An encoder already matching @p targetKind is left
			 * alone (idempotent). Used to funnel the "transition to a different encoder kind" logic
			 * through a single place so residency-cache invalidation stays in lockstep with
			 * @c endEncoding.
			 */
			void EnsureEncoderKind(EncoderKind targetKind);

			/**
			 * Returns the currently-active @c id<MTLCommandEncoder>, or @c nil when no encoder is
			 * open. Used by the debug-label helpers so a single branch over the three encoder slots
			 * is not duplicated across @c BeginLabel / @c EndLabel / @c InsertLabel.
			 */
			id<MTLCommandEncoder> GetActiveEncoder() const;

			/**
			 * Encodes waits from @p syncMask followed by this queue's signal, returning the reserved
			 * signal value. The non-empty submission path passes an empty mask because its waits run
			 * in a prologue command buffer; the empty submission path safely encodes waits here before
			 * the signal.
			 *
			 * Must be called before @c [cmdBuffer commit]; ordering of the encoded waits / signal
			 * against any other encoded work in @p cmdBuffer is the command buffer's FIFO order,
			 * so waits stall the GPU before subsequent commands execute and the signal fires after
			 * every preceding command completes. @c NotifySubmissionCommitted must still be called
			 * on @p submitQueue after @c commit returns — this helper only encodes the sync points.
			 */
			u64 EncodeQueueSyncAndSignal(id<MTLCommandBuffer> cmdBuffer, MetalGpuQueue& submitQueue, GpuQueueMask syncMask);

			/** Encodes queue-event waits at the current position in @p cmdBuffer without reserving a signal. */
			void EncodeQueueWaits(id<MTLCommandBuffer> cmdBuffer, MetalGpuQueue& submitQueue, GpuQueueMask syncMask);

			/** Resolves pending tracker barriers against the currently active Metal encoder. */
			bool ExecutePendingBarriers();

			/** Ends and reopens the current render pass with load actions that preserve its attachments. */
			bool RestartRenderPassForBarrier();

			/** Opens a continuation render encoder and restores state invalidated by the preceding encoder boundary. */
			bool ResumeRenderPass(MTLRenderPassDescriptor* descriptor);

			/**
			 * Ensures the current render encoder writes visibility results into @p queryPool. Metal fixes the
			 * visibility buffer when an encoder is created, so changing pools requires an encoder boundary.
			 */
			bool ActivateOcclusionQueryPool(const TShared<MetalGpuQueryPool>& queryPool);

			/** Encodes and releases event signals deferred until the current render encoder ends. */
			void EncodePendingEventSignals();

			/**
			 * Resolves the vertex input for the currently bound graphics pipeline against the bound
			 * vertex-buffer VertexDescription (via MetalVertexInputManager) and binds the device's
			 * zero-filled null vertex buffer at the resolved null-stream slot when the layout reserves
			 * one. Returns the resolved vertex input (whose id feeds the pipeline variant key), or null
			 * when the pipeline consumes no vertex input, when no vertex description is bound (the draw
			 * should be skipped), or when the input could not be expressed on Metal (skip the draw).
			 *
			 * @param[out]	outSkipDraw		Set to true when the caller must skip the draw: a pipeline that
			 *								declares vertex inputs but has no bound vertex description, or a
			 *								vertex description that could not be resolved on Metal.
			 */
			TShared<MetalVertexInput> ResolveVertexInputForDraw(bool& outSkipDraw);
#endif

			struct Impl;

			MetalGpuDevice& mGpuDevice;
			MetalGpuCommandBufferPool& mPool;
			TUnique<Impl> mImpl;
			u32 mId;

			/** Tracks every resource recorded on this command buffer; drives barrier deduction and Notify* fan-out. */
			MetalResourceTracker mResourceTracker;

			/** Accumulates and emits the native barriers the tracker requests. */
			MetalBarrierHelper mBarrierHelper;

			// Cached pipeline + input state; applied to the render encoder at bind time.
			TShared<MetalGpuGraphicsPipelineState> mBoundGraphicsPipeline;
			TShared<GpuComputePipelineState> mBoundComputePipeline;
			TShared<GpuBuffer> mBoundIndexBuffer;
			TShared<VertexDescription> mBoundVertexDescription;
			/**
			 * A'3: parameter sets indexed by @c GpuParameterSet::GetSet(). Replaces the former single
			 * @c mBoundParameterSet slot which silently overwrote itself when a pipeline had @c >1
			 * set. Sized @c 4 inline — matches @c RenderPassCreateInformation::Parameters (base
			 * header) and the Vulkan backend's @c mBoundGpuParameterSets. Null slots are legal and
			 * mean "no set bound at this index"; the @c Draw / @c DrawIndexed / @c DispatchCompute
			 * loops skip them.
			 */
			TInlineArray<TShared<GpuParameterSet>, 4> mBoundParameterSets;
			DrawOperationType mDrawOperation = DOT_TRIANGLE_LIST;
			u32 mStencilReference = 0;

			// Cached render-pass state. Rebuilt in BeginRenderPass and consumed at draw time to produce
			// the pipeline variant key. TopologyClass is overwritten per-draw; everything else is fixed
			// by the attachment layout of the current render pass.
			MetalPipelineVariantKey mRenderPassPipelineKey;
			IMetalRenderWindowSurface* mAcquiredWindowSurface = nullptr;
			u32 mRenderPassWidth = 0;
			u32 mRenderPassHeight = 0;
			TInlineArray<MetalImage*, B3D_MAXIMUM_RENDER_TARGET_COUNT + 1> mRenderPassAttachmentImages;

			/** Occlusion pool whose visibility buffer is attached to the current native render encoder. */
			TShared<MetalGpuQueryPool> mActiveOcclusionQueryPool;

			// Id of the queue this command buffer was last submitted on. Recorded in CommitInternal at
			// the same point the resource tracker is notified of use, and consumed by Cleanup() to route
			// the tracker's completion notification (NotifyDone) to the correct queue. Mirrors
			// VulkanGpuCommandBuffer::mSubmittedQueueId.
			GpuQueueId mSubmittedQueueId;

			/** True after resource tracking has been promoted from bound to submitted use. */
			bool mResourcesSubmitted = false;

			/** Prevents further encoding or native submission after an unrecoverable recording failure. */
			bool mRecordingFailed = false;

			/**
			 * B10: query pools referenced during this command buffer's recording. Typical @p N is @<= 4
			 * (one visibility pool plus one or two timestamp pools), so a @c TInlineArray + linear
			 * @c AddUniqueUsedQueryPool beats the hash-set allocation overhead every recording. Cleared
			 * in @c CommitInternal after @c MarkSubmitted is fanned out.
			 */
			TInlineArray<TShared<MetalGpuQueryPool>, 4> mUsedQueryPools;
			TInlineArray<TShared<MetalGpuQueryPool>, 4> mSubmittedQueryPools;
			bool mQueryPoolsQueuedForSubmission = false;

			/** Linear-scan AddUnique for @c mUsedQueryPools. See the field's note above. */
			void AddUniqueUsedQueryPool(const TShared<MetalGpuQueryPool>& pool);

			/**
			 * B3: residency-elision cache entry. A hit means @c [encoder useResources:…] already ran
			 * against the currently-open encoder for this exact set at this generation, so the
			 * bucket loop in @c EmitResidencyFor{Render,Compute}Encoder can be skipped.
			 *
			 * @c LastBoundSet is a raw pointer and never dereferenced on the skip path; it is only
			 * compared against an incoming @c MetalGpuParameters* to decide whether a re-bind of the
			 * same set is happening inside the same encoder scope. A'3: caches are keyed per bound
			 * slot so that multiple parameter sets on the same encoder don't thrash a single cache
			 * entry. Caches are invalidated at every encoder close (BeginRenderPass / EndRenderPass
			 * / EnsureEncoderKind / End), so they cannot outlive an encoder scope.
			 */
			struct ParameterSetResidencyCache
			{
				const MetalGpuParameters* LastBoundSet = nullptr;
				u64 LastBoundGeneration = 0;

				void Reset()
				{
					LastBoundSet = nullptr;
					LastBoundGeneration = 0;
				}
			};
			/**
			 * A'3: per-bound-slot residency cache, grown alongside @c mBoundParameterSets. Indexed by
			 * @c GpuParameterSet::GetSet(). Reset on encoder close via @c ResetResidencyCaches.
			 */
			TInlineArray<ParameterSetResidencyCache, 4> mRenderResidencyCaches;
			TInlineArray<ParameterSetResidencyCache, 4> mComputeResidencyCaches;

			/** Resets every slot of @c mRenderResidencyCaches to an empty state. */
			void ResetRenderResidencyCaches();

			/** Resets every slot of @c mComputeResidencyCaches to an empty state. */
			void ResetComputeResidencyCaches();
		};

		/** @} */
	} // namespace render
} // namespace b3d
