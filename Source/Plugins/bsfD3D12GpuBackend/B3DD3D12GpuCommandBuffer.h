//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DD3D12Prerequisites.h"
#include "B3DD3D12GpuDevice.h"
#include "B3DD3D12Resource.h"
#include "B3DD3D12ResourceTracker.h"
#include "Utility/B3DD3D12BarrierHelper.h"
#include "GpuBackend/B3DGpuCommandBuffer.h"
#include "Math/B3DArea2.h"

namespace b3d
{
	namespace render
	{
		/** @addtogroup D3D12GpuBackend
		 *  @{
		 */

		/**
		 * Layout compatibility release recorded in a separate ECL on the queue that previously owned an image. Access
		 * visibility is provided by the queue-fence signal and wait around this submission.
		 */
		struct D3D12SourceQueueTransition
		{
			GpuQueueId QueueId; /**< Queue that executes the release command buffer. */
			GpuQueueMask WaitMask = GpuQueueMask::kNone; /**< Queues whose work must precede the release. */
			TShared<D3D12GpuCommandBuffer> CommandBuffer; /**< Command buffer containing the release barriers. */
		};

		/** Command lists and resource-derived waits required for one D3D12 command-buffer submission. */
		struct D3D12GpuCommandBufferSubmitInformation
		{
			TInlineArray<D3D12SourceQueueTransition, 4> SourceQueueTransitions; /**< Releases executed on prior owner queues. */
			TShared<D3D12GpuCommandBuffer> TransitionCommandBuffer; /**< Prologue executed on the destination queue. */
			TShared<D3D12GpuCommandBuffer> PrimaryCommandBuffer; /**< User-recorded command buffer. */
			GpuQueueMask RequiredWaitMask = GpuQueueMask::kNone; /**< Additional queues the destination waits for. */
		};

		/** CommandBuffer implementation for DirectX 12. */
		class D3D12GpuCommandBuffer final : public GpuCommandBuffer
		{
		public:
			~D3D12GpuCommandBuffer() override;

			/**
			 * @name GpuCommandBuffer Interface
			 * @{
			 */

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
			bool IsInRenderPass() const override { return mState == GpuCommandBufferState::RecordingRenderPass; }
			void SetViewport(const Area2& area) override;
			void ClearRenderTarget(RenderSurfaceMask mask) override;
			void ClearViewport(RenderSurfaceMask mask) override;
			void EnableScissorTest(u32 left, u32 top, u32 right, u32 bottom) override;
			void DisableScissorTest() override;
			void SetStencilReferenceValue(u32 value) override;
			void IssueBarriers(const GpuBarriers& barriers) override;
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

			/** @} */

			/** Returns a unique identifier of this command buffer. */
			u32 GetId() const { return mId; }

			/** Returns the handle to the internal D3D12 command list wrapped by this object. */
			ID3D12GraphicsCommandList7* GetD3D12Handle() const { return mCommandList.Get(); }

			/** Returns the D3D12 fence associated with this command buffer. */
			ID3D12Fence* GetFence() const { return mFence.Get(); }

			/** Returns the fence value that will be signaled when this command buffer completes. */
			u64 GetFenceValue() const { return mFenceValue; }

			/** Returns true if the command buffer is currently being processed by the device. */
			bool IsSubmitted() const { return mState == GpuCommandBufferState::Executing; }

			/** Returns true if the command buffer is currently recording. */
			bool IsRecording() const { return mState == GpuCommandBufferState::Recording || mState == GpuCommandBufferState::RecordingRenderPass; }

			/** Returns true if the command buffer is done executing on the device. */
			bool IsDone() const { return mState == GpuCommandBufferState::Done; }

			/** 
			 * Resolves resource transitions against submit-thread state and prepares the command lists for submission. 
			 * 
			 * @note Submit thread only.
			 */
			D3D12GpuCommandBufferSubmitInformation PrepareForSubmitOnSubmitThread(GpuQueueType queueType, u32 queueIndex);

			/**
			 * Called on the owning thread just before the command buffer is queued for submission on the submit
			 * thread. Releases any state that must not be touched from the submit thread.
			 */
			void NotifyWillQueueForSubmit();

			/**
			 * Called on the submit thread when the command buffer is executed on a queue. Marks every tracked
			 * resource as in-flight (NotifyUsed) and remembers the queue for the matching NotifyDone on completion.
			 *
			 * @note Submit thread only.
			 */
			void NotifyWasSubmittedToQueue(GpuQueueId queueId);

			/**
			 * Checks if the command buffer still executing on the GPU.
			 *
			 * @param	block	If true, the system will block until the command buffer is done executing.
			 * @return			True if execution has finished (or was never submitted), false if still running.
			 *
			 * @note Submit thread only.
			 */
			bool UpdateExecutionStatus(bool block);

			/**
			 * Binds the currently stored GPU parameter sets, if dirty. @p isGraphics selects whether the sets are bound
			 * to the graphics or the compute root signature; both pipeline types can be bound on the command buffer
			 * simultaneously, so the bind point cannot be inferred from the command buffer's state alone.
			 */
			void BindGpuParameterSets(bool isGraphics);

			/**
			 * Registers resources from the currently bound parameter sets without rebinding their descriptors, using the
			 * active graphics or compute pipeline's per-binding stage visibility. Used for repeated compute dispatches so
			 * UAV writes remain ordered when descriptor bindings do not change.
			 */
			void TrackGpuParameterSets(bool isGraphics);