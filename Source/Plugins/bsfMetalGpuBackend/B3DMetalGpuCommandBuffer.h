//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DMetalPrerequisites.h"
#include "GpuBackend/Allocators/B3DGpuResource.h"
#include "B3DMetalGpuPipelineState.h"
#include "B3DMetalStageBarrier.h"
#include "GpuBackend/B3DGpuResourceTracker.h"
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
		class MetalImage;

		/** @addtogroup MetalGpuBackend
		 *  @{
		 */

		/**
		 * Metal implementation of a GPU command buffer.
		 *
		 * Records into a reusable @c MTL4CommandBuffer backed by the command buffer's own @c MTL4CommandAllocator. At most
		 * one encoder is open at a time: a render encoder within render passes, and a compute encoder for dispatches and
		 * transfers outside of them.
		 *
		 * Resources are untracked, so the command buffer orders all GPU work itself, through Metal 4 stage barriers:
		 *  - Work in earlier encoders, or committed earlier on the queue, is waited on through a consumer barrier
		 *    (@c barrierAfterQueueStages:) in the encoder that depends on it. Barriers recorded while no encoder is open are
		 *    encoded at the start of the next encoder.
		 *  - Work earlier in the open encoder is waited on through an intra-encoder barrier. Render encoders cannot wait for
		 *    their own fragment work, so a dependency on it ends the encoder and continues the render pass in a new one.
		 *  - Dependencies on command buffers submitted earlier to the same queue are resolved at submission, by committing a
		 *    command buffer with a producer barrier (@c barrierAfterStages:beforeQueueStages:) ahead of the recorded work.
		 *  - Dependencies on other queues are resolved by queue waits on their submission events.
		 *
		 * Metal 4 command buffers do not retain the objects they use, so the command buffer keeps alive every object it
		 * encodes that the resource tracker does not, until the GPU finishes executing it.
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
			void NotifyWillQueueForSubmit(GpuQueueId queueId, GpuQueueMask syncMask) override;

			/**
			 * Notifies the command buffer that the pool it was allocated from was reset, returning a finished or never
			 * submitted command buffer to the ready state. The pool may only be reset once none of its command buffers are
			 * executing.
			 */
			void NotifyParentPoolReset();

			/**
			 * Commits the recorded commands to @p submitQueue. Waits for the other queues in @p syncMask are issued before
			 * the recorded work, and the queue's own event and @p signalFences are signaled after it.
			 *
			 * @note	Submit thread only. The owner thread must have called NotifyWillQueueForSubmit() first.
			 */
			void ExecuteSubmitOnSubmitThread(MetalGpuQueue& submitQueue, GpuQueueMask syncMask, TArrayView<const GpuTimelineFenceAndValue> signalFences = {});

			/**
			 * Makes the queue signal @p event with @p value once the GPU finishes executing the command buffer. Metal 4
			 * command buffers cannot signal events from within. Returns false if recording failed.
			 */
			bool QueueEventSignal(id<MTLSharedEvent> event, u64 value);

			/** Keeps @p object alive until the GPU finishes executing the command buffer. */
			void RetainUntilComplete(id object);

			/** Keeps a window surface image alive until the GPU finishes executing the command buffer. */
			void TrackSurfaceImage(MetalImage& image, GpuAccessFlags access);

			/**
			 * Makes @p drawable resident for the command buffer, through the residency set of the layer that owns it. Must be
			 * called while the command buffer is recording, before any command that accesses the drawable's texture is
			 * committed. Does nothing if @p drawable is nil.
			 */
			void UseDrawableResidency(id<CAMetalDrawable> drawable);

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
			void CopyWindowToBuffer(const TShared<RenderWindow>& source, const TShared<GpuBuffer>& destination) override;
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
			void ClearRecordingState() override;
			void Destroy() override;

			/** @} */

		private:
			friend class MetalGpuCommandBufferPool;
			friend class MetalGpuQueue;

			bool RecordExplicitBarriers(const GpuExplicitBarriers& barriers, GpuBarrierPhase phase, GpuSplitBarrier* split) override;

			/** Number of buffer slots of the argument tables, the maximum Metal supports. */
			static constexpr u32 kArgumentTableBufferCount = 31;

			/** Argument table of one shader stage, together with the addresses last bound to it. */
			struct ArgumentTable
			{
				id<MTL4ArgumentTable> Table = nil;
				Array<MTLGPUAddress, kArgumentTableBufferCount> Addresses = {};

				/** Binds @p address at @p index, unless it is already bound there. */
				void SetAddress(MTLGPUAddress address, u32 index);
			};

			/** Shared buffer that small per-draw data, such as push constants, is uploaded to. */
			struct UploadChunk
			{
				id<MTLBuffer> Buffer = nil;
				GpuAllocation Allocation;
				u64 Cursor = 0;
			};

			struct PendingEventSignal
			{
				id<MTLSharedEvent> Event = nil;
				u64 Value = 0;
			};

			void SetState(GpuCommandBufferState state) { mState = state; }

			/** Returns true if the command buffer is currently recording (with or without an open render pass). */
			bool IsRecording() const { return mState == GpuCommandBufferState::Recording || mState == GpuCommandBufferState::RecordingRenderPass; }

			/**
			 * Begins recording into the native command buffer, unless already recording. Returns false if recording
			 * failed.
			 */
			bool BeginNativeCommandBuffer();

			/** Opens a compute encoder, or returns the open one. Closes an open render encoder. */
			id<MTL4ComputeCommandEncoder> OpenComputeEncoder();

			/**
			 * Opens a render encoder for @p descriptor and binds the argument tables. Encodes the barriers recorded while no
			 * encoder was open. Returns false on failure.
			 */
			bool OpenRenderEncoder(MTL4RenderPassDescriptor* descriptor);

			/** Ends the open encoder, if any. */
			void CloseEncoder();

			/**
			 * Encodes @p barrier so that it orders the rest of the open encoder and every later encoder on the queue. Without an
			 * open encoder the barrier is encoded at the start of the next one.
			 */
			void EmitBarrier(const MetalStageBarrier& barrier);

			/** Encodes the barriers recorded while no encoder was open at the start of the just opened encoder. */
			void EmitPendingBarrier();

			/** Tracks graphics resources when bindings change and compute resources before each dispatch. */
			bool TrackShaderResources(bool compute);

			/** Resets the dynamic offset overrides of the parameter set bound at @p setIndex, so every buffer uses the offset it was bound with. */
			void ResetDynamicOffsetOverrides(u32 setIndex);

			/** Adds @p pool to mUsedQueryPools unless it is already present. */
			void AddUniqueUsedQueryPool(const TShared<MetalGpuQueryPool>& pool);

			/** Converts the stored normalized viewport to this pass's pixel units and applies it to the open render encoder. */
			void ApplyViewportToRenderEncoder();

			/** Ends the open render encoder and continues the render pass in a new one, which loads what the previous one stored. */
			bool RestartRenderPass();

			/** Opens a render encoder continuing the current pass, and restores the state the previous encoder held. */
			bool ResumeRenderPass(MTL4RenderPassDescriptor* descriptor);

			/**
			 * Makes the render encoder write visibility results into @p queryPool. Metal fixes the visibility buffer when an
			 * encoder is created, so changing pools requires a new encoder.
			 */
			bool ActivateOcclusionQueryPool(const TShared<MetalGpuQueryPool>& queryPool);

			/**
			 * Resolves the bound graphics pipeline's vertex input against the bound vertex description, and binds the null
			 * vertex buffer if the resolved layout needs one. Returns null if the pipeline has no vertex input.
			 *
			 * @param	outSkipDraw		Set to true when the draw must be skipped, because the pipeline has vertex inputs
			 *							but no vertex description is bound, or the description cannot be expressed on Metal.
			 */
			TShared<MetalVertexInput> ResolveVertexInputForDraw(bool& outSkipDraw);

			/** Tracks and binds the bound vertex buffers. */
			void BindVertexBuffers();

			/**
			 * Copies @p size bytes from @p data to upload memory that stays valid until the GPU finishes executing the
			 * command buffer, and returns its GPU address, aligned to 256 bytes. Returns 0 on failure.
			 */
			MTLGPUAddress Upload(const void* data, u32 size);

			/** Uploads the push constants and binds them to the stages that use them. */
			void BindPushConstants(bool isGraphics);

			/**
			 * Binds the dynamic offset uniform buffers of every bound parameter set directly in the argument tables, applying
			 * any SetDynamicBufferOffset() overrides.
			 */
			void BindDynamicUniformBuffers(bool isGraphics);

			/**
			 * Applies all bound state to the render encoder and executes pending barriers ahead of a draw. Returns false if
			 * the draw must be skipped.
			 *
			 * @param	indexBuffer		Index buffer the draw reads, tracked with the other resources. Null for non-indexed draws.
			 */
			bool PrepareDraw(MetalBuffer* indexBuffer);

			/** Records that a draw was encoded in the open render encoder. */
			void NotifyDrawEncoded();

			/** Binds the bound parameter sets' argument buffers to the argument tables of the stages that use them. */
			void BindParameterSets(bool isGraphics);

			/** Sets @p pipeline on the open render encoder, unless it is already set. */
			void SetRenderPipelineState(id<MTLRenderPipelineState> pipeline);

			/** Sets @p state on the open render encoder, unless it is already set. */
			void SetDepthStencilState(id<MTLDepthStencilState> state);

			/** Sets @p value as the stencil reference of the open render encoder, unless it is already set. */
			void SetEncoderStencilReference(u32 value);

			MetalGpuDevice& mGpuDevice;
			MetalGpuCommandBufferPool& mPool;
			u32 mId;

			id<MTL4CommandBuffer> mCommandBuffer = nil;
			id<MTL4CommandAllocator> mCommandAllocator = nil;
			bool mIsNativeRecording = false;

			/** True once the native command buffer holds recorded commands that were never reset. */
			bool mHasNativeCommands = false;

			id<MTL4RenderCommandEncoder> mRenderEncoder = nil;
			id<MTL4ComputeCommandEncoder> mComputeEncoder = nil;

			/** True if the open encoder encoded work that later work in the same encoder may depend on. */
			bool mEncoderHasWork = false;

			/** True if fragment shaders of draws in the open render encoder may write storage resources. */
			bool mRenderEncoderHasFragmentWrites = false;

			/** Barrier recorded while no encoder was open, encoded at the start of the next encoder. */
			MetalStageBarrier mPendingBarrier;

			ArgumentTable mVertexArgumentTable;
			ArgumentTable mFragmentArgumentTable;
			ArgumentTable mComputeArgumentTable;

			Vector<UploadChunk> mUploadChunks;
			u32 mUploadChunkIndex = 0;

			/** Objects encoded by the command buffer that nothing else keeps alive until it finishes executing. */
			Vector<id> mRetainedObjects;

			/** Native pipeline states last set on the open render encoder, compared to skip redundant binds. */
			__unsafe_unretained id<MTLRenderPipelineState> mCurrentRenderPipeline = nil;
			__unsafe_unretained id<MTLDepthStencilState> mCurrentDepthStencilState = nil;
			__unsafe_unretained id<MTLComputePipelineState> mCurrentComputePipeline = nil;
			u32 mCurrentStencilReference = 0;

			/** Drawables rendered to by the command buffer. The queue must wait for them before committing it. */
			Vector<id<CAMetalDrawable>> mDrawables;

			/** Residency sets of the layers whose drawables the command buffer accesses, attached through UseDrawableResidency(). */
			Vector<id<MTLResidencySet>> mDrawableResidencySets;

			MTL4RenderPassDescriptor* mRestartRenderPassDescriptor = nil;

			/** Signals of event queries, issued by the queue once the command buffer finishes executing. */
			Vector<PendingEventSignal> mPendingEventSignals;

			MTLViewport mViewport = {};
			Area2 mNormalizedViewport = Area2(0.0f, 0.0f, 1.0f, 1.0f); /**< Viewport in normalized [0, 1] units, converted to pixels per render pass. */
			MTLScissorRect mScissor = {};
			bool mHasViewport = false;
			bool mHasScissor = false;

			MTLVisibilityResultMode mVisibilityMode = MTLVisibilityResultModeDisabled;
			NSUInteger mVisibilityOffset = 0;

			/** Labels pushed and not popped yet, re-pushed on the native command buffer if recording restarts. */
			u32 mDebugGroupDepth = 0;

			/** Tracks every resource used by the recorded commands: lifetime (bound/use counts) and render pass attachments. */
			GpuResourceTracker mResourceTracker;

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
