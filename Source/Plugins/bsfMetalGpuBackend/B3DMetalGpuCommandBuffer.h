//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DMetalPrerequisites.h"
#include "B3DMetalGpuPipelineState.h"
#include "B3DMetalResourceTracker.h"
#include "B3DMetalBarrierHelper.h"
#include "B3DMetalShaderABI.h"
#include "GpuBackend/B3DGpuCommandBuffer.h"
#include "GpuBackend/B3DGpuPushConstants.h"
#include "GpuBackend/B3DGpuTimelineFence.h"
#include "GpuBackend/B3DRenderTarget.h"

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
		class MetalGpuBuffer;
		class MetalBuffer;

		/** @addtogroup MetalGpuBackend
		 *  @{
		 */

		/**
		 * Metal implementation of a GPU command buffer.
		 *
		 * Owns a single @c MTLCommandBuffer acquired from the owning queue. At most one encoder (render,
		 * compute, or blit) is active at a time.
		 */
		class MetalGpuCommandBuffer final : public GpuCommandBuffer
		{
		public:
			MetalGpuCommandBuffer(MetalGpuDevice& device, MetalGpuCommandBufferPool& pool, u32 id, ThreadId ownerThread, GpuQueueType queueType, const GpuCommandBufferCreateInformation& createInformation);
			~MetalGpuCommandBuffer() override;

			/** Returns a unique identifier of this command buffer. */
			u32 GetId() const { return mId; }

			/**
			 * Called on the owner thread just before the command buffer is handed to the submit thread. Releases the
			 * recording state the submit thread must not touch.
			 */
			void NotifyWillQueueForSubmit();

			/**
			 * Notifies the command buffer that the pool it was allocated from was reset, returning a finished command
			 * buffer to the ready state. The pool may only be reset once all of its command buffers have finished executing.
			 */
			void NotifyParentPoolReset();

			/**
			 * Commits the recorded commands to @p submitQueue. Waits for the other queues in @p syncMask are encoded before
			 * the recorded work, and the queue's own event and @p signalFences are signaled after it.
			 *
			 * @note	Submit thread only. The owner thread must have called NotifyWillQueueForSubmit() first.
			 */
			void ExecuteSubmitOnSubmitThread(MetalGpuQueue& submitQueue, GpuQueueMask syncMask, TArrayView<const GpuTimelineFenceAndValue> signalFences = {});

			/** Returns the underlying MTLCommandBuffer, acquiring it on first use. */
			id<MTLCommandBuffer> GetOrAcquireMetalCommandBuffer();

			/**
			 * Closes any open render or compute encoder and returns a blit encoder, opening one if needed. Lets code outside
			 * the command buffer (e.g. window surface readback) append blits in order with the recorded commands.
			 */
			id<MTLBlitCommandEncoder> GetOrOpenBlitEncoder();

			/** Encodes an event signal at the current position in the recorded commands. */
			bool EncodeSignalEvent(id<MTLSharedEvent> event, u64 value);

			/**
			 * @name GpuCommandBuffer Interface
			 *  @{
			 */

			void SetName(const StringView& name) override;

			void SetGpuParameterSet(const TShared<GpuParameterSet>& parameters) override;
			void SetDynamicBufferOffset(u32 set, u32 bufferIndex, u32 offset) override;
			void SetPushConstants(u32 offsetInBytes, u32 sizeInBytes, const void* data) override;
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
			bool IsInRenderPass() const override { return mRenderEncoder != nil; }
			void SetViewport(const Area2& area) override;
			void ClearRenderTarget(RenderSurfaceMask mask) override;
			void ClearViewport(RenderSurfaceMask mask) override;
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
			void ClearRecordingState() override;
			void Destroy() override;

			/** @} */

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

			/**
			 * Remembers which parameter set, at which generation, last had its resources made resident on the open encoder,
			 * so rebinding an unchanged set can skip the useResources: calls. The pointer is only compared, never
			 * dereferenced. Reset whenever the encoder closes.
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

			void SetState(GpuCommandBufferState state) { mState = state; }

			/** Returns true if the command buffer is currently recording (with or without an open render pass). */
			bool IsRecording() const { return mState == GpuCommandBufferState::Recording || mState == GpuCommandBufferState::RecordingRenderPass; }

			/** Tracks graphics resources when bindings change and compute resources before each dispatch. */
			bool TrackShaderResources(bool compute);

			/**
			 * Binds @p parameters at the slot of its set index. Returns false if the set index collides with the reserved
			 * vertex buffer slots.
			 */
			bool BindParameterSet(const TShared<GpuParameterSet>& parameters);

			/** Resets the dynamic offset overrides of the parameter set bound at @p setIndex, so every buffer uses the offset it was bound with. */
			void ResetDynamicOffsetOverrides(u32 setIndex);

			/** Adds @p pool to mUsedQueryPools unless it is already present. */
			void AddUniqueUsedQueryPool(const TShared<MetalGpuQueryPool>& pool);

			/**
			 * Clears the render encoder's residency caches, so every parameter set bound afterwards makes its resources
			 * resident again. Must be called whenever the render encoder closes.
			 */
			void ResetRenderResidencyCaches();

			/** Compute encoder counterpart of ResetRenderResidencyCaches(). */
			void ResetComputeResidencyCaches();

			/** Converts the stored normalized viewport to this pass's pixel units and applies it to the open render encoder. */
			void ApplyViewportToRenderEncoder();

			/** Closes the open encoder unless it is of @p targetKind, resetting that encoder's residency caches. */
			void EnsureEncoderKind(EncoderKind targetKind);

			/** Returns the open encoder of any kind, or nil if none is open. */
			id<MTLCommandEncoder> GetActiveEncoder() const;

			/** Ends every open encoder. Does not reset the residency caches, callers do that when it matters to them. */
			void CloseAllEncoders();

#if B3D_METAL_USE_EXPLICIT_RESOURCE_SYNCHRONIZATION
			/** Signals the resource fence once @p encoder's work completes, so the next encoder can wait on it. */
			void UpdateResourceFence(id<MTLRenderCommandEncoder> encoder);
			void UpdateResourceFence(id<MTLComputeCommandEncoder> encoder);
			void UpdateResourceFence(id<MTLBlitCommandEncoder> encoder);

			/** Makes @p encoder wait on the resource fence if an earlier encoder signaled it. */
			void WaitForResourceFence(id<MTLRenderCommandEncoder> encoder);
			void WaitForResourceFence(id<MTLComputeCommandEncoder> encoder);
			void WaitForResourceFence(id<MTLBlitCommandEncoder> encoder);
#endif

			/**
			 * Encodes waits for the last committed submission of every queue in @p syncMask. @p submitQueue itself is only
			 * waited on with explicit resource synchronization.
			 */
			void EncodeQueueWaits(id<MTLCommandBuffer> commandBuffer, MetalGpuQueue& submitQueue, GpuQueueMask syncMask);

			/**
			 * Encodes the waits of the frame fence (see GpuSubmitThread::ConsumeFrameFence()): one per queue with a non-zero
			 * value in @p frameFenceValues, @p submitQueue included. Skips other queues in @p syncMask, which
			 * EncodeQueueWaits() already waits on at their latest committed value.
			 */
			void EncodeFrameFenceWaits(id<MTLCommandBuffer> commandBuffer, MetalGpuQueue& submitQueue, GpuQueueMask syncMask,
				TArrayView<const u64> frameFenceValues);

			/**
			 * Encodes a signal of @p submitQueue's event and returns the signaled value. MetalGpuQueue::NotifySubmissionCommitted()
			 * must still be called once the command buffer is committed.
			 */
			u64 EncodeQueueSignal(id<MTLCommandBuffer> commandBuffer, MetalGpuQueue& submitQueue);

			/** Resolves pending tracker barriers against the open encoder. */
			bool ExecutePendingBarriers();

			/** Ends and reopens the current render pass with load actions that preserve its attachments. */
			bool RestartRenderPassForBarrier();

			/** Opens a continuation render encoder and restores the state the previous encoder held. */
			bool ResumeRenderPass(MTLRenderPassDescriptor* descriptor);

			/**
			 * Makes the render encoder write visibility results into @p queryPool. Metal fixes the visibility buffer when an
			 * encoder is created, so changing pools requires a new encoder.
			 */
			bool ActivateOcclusionQueryPool(const TShared<MetalGpuQueryPool>& queryPool);

			/** Encodes the event signals that were deferred until the render encoder ends. */
			void EncodePendingEventSignals();

			/**
			 * Resolves the bound graphics pipeline's vertex input against the bound vertex description, and binds the null
			 * vertex buffer if the resolved layout needs one. Returns null if the pipeline has no vertex input.
			 *
			 * @param	outSkipDraw		Set to true when the draw must be skipped, because the pipeline has vertex inputs
			 *							but no vertex description is bound, or the description cannot be expressed on Metal.
			 */
			TShared<MetalVertexInput> ResolveVertexInputForDraw(bool& outSkipDraw);

			/**
			 * Binds the vertex buffers that differ from what the render encoder holds. Called before barriers are executed,
			 * so a render pass restarted for a barrier rebinds the current buffers.
			 */
			void ApplyVertexBuffersToRenderEncoder();

			/** Uploads the push constants to the open render or compute encoder. */
			void BindPushConstants(bool isGraphics);

			/**
			 * Binds the dynamic offset uniform buffers of every bound parameter set directly in the open encoder's argument
			 * table, applying any SetDynamicBufferOffset() overrides. Only bindings that changed are encoded.
			 */
			void BindDynamicUniformBuffers(bool isGraphics);

			/**
			 * Applies all bound state to the render encoder and executes pending barriers ahead of a draw. Returns false if
			 * the draw must be skipped.
			 *
			 * @param	indexBuffer		Index buffer the draw reads, tracked with the other resources. Null for non-indexed draws.
			 */
			bool PrepareDraw(MetalBuffer* indexBuffer);

			/**
			 * Attaches the bound parameter sets' argument buffers to the open render or compute encoder and makes their
			 * resources resident, skipping sets the encoder already holds at their current generation.
			 */
			void AttachParameterSetsToEncoder(bool isGraphics);

			/** Forgets what the encoders' argument tables hold, forcing a full rebind on the next draw or dispatch. */
			void ResetArgumentTableBindings();

			MetalGpuDevice& mGpuDevice;
			MetalGpuCommandBufferPool& mPool;
			u32 mId;

			struct PendingEventSignal
			{
				id<MTLSharedEvent> Event = nil;
				u64 Value = 0;
			};

			struct VertexBufferBinding
			{
				id<MTLBuffer> Buffer = nil;
				NSUInteger Offset = 0;
				NSUInteger Index = 0;
			};

			/** Buffer and offset last handed to an encoder's argument table at one dynamic uniform-buffer index. Buffer is compared by address only, never dereferenced. */
			struct ArgumentTableBinding
			{
				__unsafe_unretained id<MTLBuffer> Buffer = nil;
				NSUInteger Offset = 0;
			};

			id<MTLCommandBuffer> mCommandBuffer = nil;
			id<MTLRenderCommandEncoder> mRenderEncoder = nil;
			id<MTLComputeCommandEncoder> mComputeEncoder = nil;
			id<MTLBlitCommandEncoder> mBlitEncoder = nil;

			MTLRenderPassDescriptor* mRestartRenderPassDescriptor = nil;

			Vector<PendingEventSignal> mPendingEventSignals;
			Vector<VertexBufferBinding> mVertexBufferBindings;

			/** Scratch list of the resources passed to a single useResources: call, reused to avoid per-draw allocations. */
			Vector<__unsafe_unretained id<MTLResource>> mResidencyResources;

			/** Argument-table contents of the vertex, fragment and compute stages, relative to kMetalDynamicUniformBufferIndexBase. */
			Array<ArgumentTableBinding, kMetalDynamicUniformBufferCount> mVertexArgumentTable;
			Array<ArgumentTableBinding, kMetalDynamicUniformBufferCount> mFragmentArgumentTable;
			Array<ArgumentTableBinding, kMetalDynamicUniformBufferCount> mComputeArgumentTable;

			MTLViewport mViewport = {};
			Area2 mNormalizedViewport = Area2(0.0f, 0.0f, 1.0f, 1.0f); /**< Viewport in normalized [0, 1] units, converted to pixels per render pass. */
			MTLScissorRect mScissor = {};
			bool mHasViewport = false;
			bool mHasScissor = false;

			MTLVisibilityResultMode mVisibilityMode = MTLVisibilityResultModeDisabled;
			NSUInteger mVisibilityOffset = 0;

			u32 mDebugGroupDepth = 0;

#if B3D_METAL_USE_EXPLICIT_RESOURCE_SYNCHRONIZATION
			id<MTLFence> mResourceFence = nil;
			bool mFenceNeedsWait = false;
#endif

			/** Tracks every resource used by the recorded commands, deducing barriers and resource usage notifications. */
			MetalResourceTracker mResourceTracker;
			MetalBarrierHelper mBarrierHelper;

			TShared<MetalGpuGraphicsPipelineState> mBoundGraphicsPipeline;
			TShared<GpuComputePipelineState> mBoundComputePipeline;
			GpuPushConstantPayload mPushConstants;
			bool mGraphicsPushConstantsRequireBind = false;
			bool mComputePushConstantsRequireBind = false;
			TShared<GpuBuffer> mBoundIndexBuffer;
			TShared<VertexDescription> mBoundVertexDescription;
			DrawOperationType mDrawOperation = DOT_TRIANGLE_LIST;
			u32 mStencilReference = 0;

			/**
			 * Indexed by stream index, null slots have no buffer bound. Holds the engine buffers rather than native handles
			 * because a buffer's backing can be replaced between the bind and the draw (e.g. GpuBufferUtility::Write()
			 * discarding a bound buffer).
			 */
			TInlineArray<TShared<MetalGpuBuffer>, 4> mBoundVertexBuffers;

			/** Indexed by GpuParameterSet::GetSet(), null slots have no set bound. */
			TInlineArray<TShared<GpuParameterSet>, 4> mBoundParameterSets;

			/**
			 * Dynamic offset overrides applied through SetDynamicBufferOffset(), per bound parameter set slot and indexed by
			 * that set layout's dynamic offset index. ~0u means the offset the buffer was bound with applies. Reset whenever
			 * a set is bound at the slot.
			 */
			TInlineArray<TInlineArray<u32, 4>, 4> mDynamicOffsetOverridesPerSet;

			/** Set when graphics parameter resources must be registered with the tracker again. */
			bool mGraphicsResourcesRequireTracking = true;

			/** Residency caches per parameter set slot, indexed by GpuParameterSet::GetSet(). */
			TInlineArray<ParameterSetResidencyCache, 4> mRenderResidencyCaches;
			TInlineArray<ParameterSetResidencyCache, 4> mComputeResidencyCaches;

			/** Pipeline variant key fields fixed by the current render pass's attachments. TopologyClass is set per draw. */
			MetalPipelineVariantKey mRenderPassPipelineKey;
			IMetalRenderWindowSurface* mAcquiredWindowSurface = nullptr;
			bool mRenderPassTrackingActive = false;
			u32 mRenderPassWidth = 0;
			u32 mRenderPassHeight = 0;

			/** Clear values of the bound target's surfaces, used by both the pass load actions and explicit clears. */
			RenderTargetClearValues mRenderPassClearValues;

			/** Occlusion query pool whose visibility buffer is attached to the render encoder. */
			TShared<MetalGpuQueryPool> mActiveOcclusionQueryPool;

			/** Query pools used while recording. Usually only a few, so they're searched linearly. */
			TInlineArray<TShared<MetalGpuQueryPool>, 4> mUsedQueryPools;
			TInlineArray<TShared<MetalGpuQueryPool>, 4> mSubmittedQueryPools;

			/** Set once mUsedQueryPools were notified of the pending submission, so a failure is reported as a failed submission. */
			bool mQueryPoolsQueuedForSubmission = false;

			/** Queue the command buffer was last submitted on, used to route the tracker's completion notification. */
			GpuQueueId mSubmittedQueueId;

			/** True after resource tracking has been promoted from bound to submitted use. */
			bool mResourcesSubmitted = false;

			/** Prevents further encoding or native submission after an unrecoverable recording failure. */
			bool mRecordingFailed = false;
		};

		/** @} */
	} // namespace render
} // namespace b3d
