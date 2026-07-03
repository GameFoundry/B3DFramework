//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DD3D12Prerequisites.h"
#include "B3DD3D12GpuDevice.h"
#include "GpuBackend/B3DGpuCommandBuffer.h"
#include "Math/B3DArea2.h"

namespace b3d
{
	namespace render
	{
		/** @addtogroup D3D12GpuBackend
		 *  @{
		 */

		/** DirectX 12 implementation of GpuCommandBufferPool. */
		class D3D12GpuCommandBufferPool : public GpuCommandBufferPool
		{
			using Base = GpuCommandBufferPool;
		public:
			D3D12GpuCommandBufferPool(D3D12GpuDevice& device, const GpuCommandBufferPoolCreateInformation& createInformation);
			~D3D12GpuCommandBufferPool() override;

			TShared<GpuCommandBuffer> Create(const GpuCommandBufferCreateInformation& createInformation) override;
			TShared<GpuCommandBuffer> FindOrCreate(const GpuCommandBufferCreateInformation& createInformation) override;
			void Reset() override;
			void Destroy() override;

			/** Returns the D3D12 command allocator. */
			ID3D12CommandAllocator* GetD3D12CommandAllocator() const { return mCommandAllocator.Get(); }

		private:
			ComPtr<ID3D12CommandAllocator> mCommandAllocator;
			u32 mNextCommandBufferId = 1;

			UnorderedMap<u32, TShared<D3D12GpuCommandBuffer>> mCommandBuffers;
		};

		/** CommandBuffer implementation for DirectX 12. */
		class D3D12GpuCommandBuffer final : public GpuCommandBuffer
		{
		public:
			~D3D12GpuCommandBuffer() override;

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
			void ClearRenderTarget(RenderSurfaceMask mask, const Color& color, float depth, u16 stencil) override;
			void ClearViewport(RenderSurfaceMask mask, const Color& color, float depth, u16 stencil) override;
			void EnableScissorTest(u32 left, u32 top, u32 right, u32 bottom) override;
			void DisableScissorTest() override;
			void SetStencilReferenceValue(u32 value) override;
			void IssueBarriers(const GpuBarriers& barriers) override;
			void CopyBufferToBuffer(const TShared<GpuBuffer>& source, const TShared<GpuBuffer>& destination, u32 sourceOffset, u32 destinationOffset, u32 length) override;
			void CopyBufferToTexture(const TShared<GpuBuffer>& source, const TShared<Texture>& destination, u32 bufferOffset, u32 mipLevel, u32 arrayLayer) override;
			void CopyTextureToBuffer(const TShared<Texture>& source, const TShared<GpuBuffer>& destination, u32 mipLevel, u32 arrayLayer, u32 bufferOffset) override;
			void WriteTimestamp(GpuQueryId query, const TShared<GpuQueryPool>& queryPool) override;
			void BeginQuery(GpuQueryId query, const TShared<GpuQueryPool>& queryPool, GpuQueryFlags flags) override;
			void EndQuery(GpuQueryId query, const TShared<GpuQueryPool>& queryPool) override;
			void ResetQueries(const TShared<GpuQueryPool>& queryPool) override;
			void BeginLabel(const StringView& name) override;
			void EndLabel() override;
			void InsertLabel(const StringView& name) override;
			void End() override;

			/** Returns an unique identifier of this command buffer. */
			u32 GetId() const { return mId; }

			/** Returns the thread that the command buffer is allowed to be used on. */
			ThreadId GetOwnerThread() const { return mOwnerThread; }

			/** Returns the handle to the internal D3D12 command list wrapped by this object. */
			ID3D12GraphicsCommandList* GetD3D12Handle() const { return mCommandList.Get(); }

			/** Returns the D3D12 fence associated with this command buffer. */
			ID3D12Fence* GetFence() const { return mFence.Get(); }

			/** Returns the fence value that will be signaled when this command buffer completes. */
			u64 GetFenceValue() const { return mFenceValue; }

			/** Returns true if the command buffer is currently being processed by the device. */
			bool IsSubmitted() const { return mState == GpuCommandBufferState::Executing; }

			/** Returns true if the command buffer is currently recording. */
			bool IsRecording() const { return mState == GpuCommandBufferState::Recording || mState == GpuCommandBufferState::RecordingRenderPass; }

			/** Returns true if the command buffer is ready to be submitted to a queue. */
			bool IsReadyForSubmit() const { return mState == GpuCommandBufferState::RecordingDone; }

			/** Returns true if the command buffer is done executing on the device. */
			bool IsDone() const { return mState == GpuCommandBufferState::Done; }

			/**
			 * Called on the owning thread just before the command buffer is queued for submission on the submit
			 * thread. Releases any state that must not be touched from the submit thread.
			 */
			void NotifyWillQueueForSubmit();

			/**
			 * Checks if the command buffer still executing on the GPU.
			 *
			 * @param	block	If true, the system will block until the command buffer is done executing.
			 * @return			True if execution has finished (or was never submitted), false if still running.
			 */
			bool UpdateExecutionStatus(bool block);

			/**
			 * Resets the command buffer back in Ready state. Should be called when command buffer is done executing on a
			 * queue.
			 */
			void Reset();

			/************************************************************************/
			/* 								COPY COMMANDS                     		*/
			/************************************************************************/

			/**
			 * Copies the contents of the source buffer to the destination buffer at the ID3D12Resource level.
			 *
			 * @param	source				Source buffer to copy from.
			 * @param	destination			Destination buffer to copy to.
			 * @param	sourceOffset		Offset into the source buffer, in bytes.
			 * @param	destinationOffset	Offset into the destination buffer, in bytes.
			 * @param	length				Size of the data to copy, in bytes.
			 */
			void CopyBufferToBufferRaw(ID3D12Resource* source, ID3D12Resource* destination, u64 sourceOffset, u64 destinationOffset, u64 length);

			/**
			 * Copies the contents of the source buffer to the destination texture at the ID3D12Resource level.
			 *
			 * @param	source				Source buffer to copy from.
			 * @param	destination			Destination texture to copy to.
			 * @param	layout				Footprint layout describing the buffer data organization.
			 * @param	subresourceIndex	Destination texture subresource index.
			 */
			void CopyBufferToTextureRaw(ID3D12Resource* source, ID3D12Resource* destination, const D3D12_PLACED_SUBRESOURCE_FOOTPRINT& layout, u32 subresourceIndex);

			/**
			 * Copies the contents of the source texture to the destination buffer at the ID3D12Resource level.
			 *
			 * @param	source				Source texture to copy from.
			 * @param	destination			Destination buffer to copy to.
			 * @param	layout				Footprint layout describing the buffer data organization.
			 * @param	subresourceIndex	Source texture subresource index.
			 */
			void CopyTextureToBufferRaw(ID3D12Resource* source, ID3D12Resource* destination, const D3D12_PLACED_SUBRESOURCE_FOOTPRINT& layout, u32 subresourceIndex);

			/**
			 * Issues a resource barrier to transition a resource state.
			 *
			 * @param	resource			Resource to transition.
			 * @param	stateBefore			Current state of the resource.
			 * @param	stateAfter			Target state for the resource.
			 * @param	subresource			Subresource index (default is all subresources).
			 */
			void TransitionResource(ID3D12Resource* resource, D3D12_RESOURCE_STATES stateBefore, D3D12_RESOURCE_STATES stateAfter, u32 subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES);

			// TODO: Add more resource tracking and transition methods similar to Vulkan implementation

		private:
			friend class D3D12GpuCommandBufferPool;
			friend class D3D12GpuQueue;

			D3D12GpuCommandBuffer(D3D12GpuDevice& device, D3D12GpuCommandBufferPool& pool, u32 id,
				ID3D12GraphicsCommandList* commandList, ThreadId ownerThread, GpuQueueType queueType,
				const GpuCommandBufferCreateInformation& createInformation);

			/** Returns the pool the command buffer was allocated from. */
			D3D12GpuCommandBufferPool& GetPool() const { return mPool; }

			/** Makes the command buffer ready to start recording commands. */
			void Begin();

			/** Checks if all the prerequisites for rendering have been made. */
			bool IsReadyForRender();

			/** Marks the command buffer as submitted on a queue. */
			void SetIsSubmitted() { mState = GpuCommandBufferState::Executing; }

			/** Binds the current graphics pipeline to the command buffer. Returns true if bind was successful. */
			bool BindGraphicsPipeline();

			/** Binds any dynamic states to the pipeline, as required. */
			void BindDynamicStates(bool forceAll);

			/** Binds vertex and index buffers to the pipeline, if dirty. */
			void BindVertexInputs();

			/** Binds the currently stored GPU parameters object, if dirty. */
			void BindGpuParams();

			/** Clears the specified area of the currently bound render target. */
			void ClearViewportArea(const Area2I& area, RenderSurfaceMask mask, const Color& color, float depth, u16 stencil);

			/**
			 * Transitions the current framebuffer's attachments into their render-pass states (color -> RENDER_TARGET,
			 * depth -> DEPTH_WRITE/DEPTH_READ, honoring the read-only mask) and updates the tracked states. Must be
			 * called before OMSetRenderTargets / clears in BeginRenderPass.
			 */
			void TransitionRenderPassAttachments();

			/**
			 * Emits a D3D12 transition for a resource whose current state is tracked externally (via @p stateHolder),
			 * skipping no-op transitions and leaving the tracked state updated. Skips UPLOAD/READBACK heap buffers,
			 * which are permanently in GENERIC_READ / COPY_DEST and must never be transitioned. Batches into the caller.
			 *
			 * @return		true if a transition barrier was appended to @p outBarriers.
			 */
			bool AppendTransition(ID3D12Resource* resource, D3D12_RESOURCE_STATES* stateHolder, D3D12_RESOURCE_STATES targetState,
				Vector<D3D12_RESOURCE_BARRIER>& outBarriers, u32 subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES);

			/**
			 * Immediately transitions a buffer into a copy state (COPY_SOURCE/COPY_DEST) and updates its tracked state.
			 * UPLOAD-heap buffers (permanently GENERIC_READ) and READBACK-heap buffers (permanently COPY_DEST) are left
			 * untouched. State is left in the copy state after the call (leave-and-track).
			 */
			void TransitionBufferForCopy(D3D12GpuBuffer* buffer, bool asDestination);

			/**
			 * Immediately transitions a texture into a copy state (COPY_SOURCE/COPY_DEST) and updates its tracked state.
			 * State is left in the copy state after the call (leave-and-track).
			 */
			void TransitionTextureForCopy(D3D12Texture* texture, bool asDestination);

			/** Returns the current viewport area in pixels. */
			Area2I GetViewportArea() const;

			/** Returns the current area of the render pass in pixels. */
			Area2I GetRenderPassArea() const;

			/** Returns the owner GPU device, cast as a D3D12GpuDevice. */
			D3D12GpuDevice& GetD3D12GpuDevice() const { return static_cast<D3D12GpuDevice&>(mGpuDevice); }

			u32 mId;
			ComPtr<ID3D12GraphicsCommandList> mCommandList;
			D3D12GpuCommandBufferPool& mPool;
			ComPtr<ID3D12Fence> mFence;
			u64 mFenceValue = 0;

			// Render state
			D3D12Framebuffer* mFramebuffer = nullptr;
			RenderSurfaceMask mRenderTargetReadOnlyMask = RT_NONE;
			RenderSurfaceMask mRenderTargetLoadMask = RT_NONE;

			TShared<D3D12GpuGraphicsPipelineState> mGraphicsPipeline;
			TShared<D3D12GpuComputePipelineState> mComputePipeline;
			TShared<VertexDescription> mVertexDescription;
			TShared<D3D12GpuBuffer> mIndexBuffer;
			Vector<TShared<D3D12GpuBuffer>> mVertexBuffers;
			Area2 mNormalizedViewportArea{ 0.0f, 0.0f, 1.0f, 1.0f };
			Area2I mScissor{ 0, 0, 0, 0 };
			bool mIsScissorTestEnabled = false;
			u32 mStencilRef = 0;
			DrawOperationType mDrawOp = DOT_TRIANGLE_LIST;
			u32 mRequiredVertexBufferBindingCount = 0;
			ID3D12PipelineState* mLastBoundGraphicsPipeline = nullptr; /**< Pipeline variant currently set on the command list. */

			bool mGfxPipelineRequiresBind : 1;
			bool mCmpPipelineRequiresBind : 1;
			bool mViewportRequiresBind : 1;
			bool mStencilRefRequiresBind : 1;
			bool mScissorRequiresBind : 1;
			bool mBoundParamsDirty : 1;
			bool mVertexInputsDirty : 1;

			TShared<D3D12GpuParameters> mBoundParams;
			TShared<RenderTarget> mRenderTarget;
		};

		/** @} */
	} // namespace render
} // namespace b3d
