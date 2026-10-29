//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DD3D12Prerequisites.h"
#include "B3DD3D12GpuDevice.h"
#include "B3DD3D12Resource.h"
#include "Utility/B3DD3D12BarrierBatch.h"
#include "GpuBackend/B3DGpuResourceTracker.h"
#include "GpuBackend/B3DGpuCommandBuffer.h"
#include "GpuBackend/B3DGpuPushConstants.h"
#include "Math/B3DArea2.h"
#include "GpuBackend/B3DRenderTarget.h"

namespace b3d
{
	namespace render
	{
		/** @addtogroup D3D12GpuBackend
		 *  @{
		 */

		/** Command lists required for one D3D12 command-buffer submission. */
		struct D3D12GpuCommandBufferSubmitInformation
		{
			/**
			 * Brings images the primary command buffer uses to rest (see IGpuImageResource::RequestRestingTransition()), in a prologue executed on
			 * the destination queue.
			 */
			TShared<D3D12GpuCommandBuffer> TransitionCommandBuffer;

			/**
			 * Brings images to rest whose resting layout the destination queue cannot transition into. Executed on the graphics queue, and the
			 * submission waits for it.
			 */
			TShared<D3D12GpuCommandBuffer> GraphicsTransitionCommandBuffer;

			TShared<D3D12GpuCommandBuffer> PrimaryCommandBuffer; /**< User-recorded command buffer. */
		};

		/** DirectX 12 implementation of GpuCommandBufferPool. */
		class D3D12GpuCommandBufferPool : public GpuCommandBufferPool
		{
			using Base = GpuCommandBufferPool;
		public:
			/**
			 * Creates a command-buffer pool. A pool that uses pool reset backs all its command buffers with one D3D12 command allocator,
			 * otherwise each command buffer has an allocator of its own.
			 */
			D3D12GpuCommandBufferPool(D3D12GpuDevice& device, const GpuCommandBufferPoolCreateInformation& createInformation);
			~D3D12GpuCommandBufferPool() override;

			/**
			 * @name GpuCommandBufferPool Interface
			 * @{
			 */

			TShared<GpuCommandBuffer> Create(const GpuCommandBufferCreateInformation& createInformation) override;
			TShared<GpuCommandBuffer> FindOrCreate(const GpuCommandBufferCreateInformation& createInformation) override;
			void Reset() override;
			void Destroy() override;

			/** @} */

			/** Returns the D3D12 command allocator shared by the command buffers. Null if the pool does not use pool reset. */
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

			/**
			 * @name GpuCommandBuffer Interface
			 * @{
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
			bool IsInRenderPass() const override { return mState == GpuCommandBufferState::RecordingRenderPass; }
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
			D3D12GpuCommandBufferSubmitInformation PrepareForSubmitOnSubmitThread(GpuQueueType queueType);

			/**
			 * Called on the owning thread just before the command buffer is queued for submission on the submit
			 * thread. Releases any state that must not be touched from the submit thread.
			 */
			void NotifyWillQueueForSubmit(GpuQueueId queueId, GpuQueueMask syncMask) override;

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

			/** Releases the current recording and returns the command buffer to the Ready state. */
			void Reset();

			/************************************************************************/
			/* 								COPY COMMANDS                     		*/
			/************************************************************************/

			/**
			 * Copies the contents of the source texture to the destination buffer at the ID3D12Resource level.
			 *
			 * @param	source				Source texture to copy from.
			 * @param	destination			Destination buffer to copy to.
			 * @param	layout				Footprint layout describing the buffer data organization.
			 * @param	subresourceIndex	Source texture subresource index.
			 */
			void CopyTextureToBuffer(ID3D12Resource* source, ID3D12Resource* destination, const D3D12_PLACED_SUBRESOURCE_FOOTPRINT& layout, u32 subresourceIndex);

			/**
			 * Copies the top-level subresource of an image into a buffer, including the required resource tracking and
			 * state transitions. Used for reading back images that have no owning texture (e.g. swap chain back buffers).
			 *
			 * @param	source			Image to copy from. Only subresource 0 (first face, top mip) is copied.
			 * @param	destination		Buffer to copy into, at offset 0.
			 * @param	width			Width of the image, in pixels.
			 * @param	height			Height of the image, in pixels.
			 * @param	rowPitchBytes	Byte pitch between rows in the buffer. Must be a multiple of
			 *							D3D12_TEXTURE_DATA_PITCH_ALIGNMENT.
			 */
			void CopyImageToBuffer(D3D12Image* source, D3D12Buffer* destination, u32 width, u32 height, u32 rowPitchBytes);

		private:
			friend class D3D12GpuCommandBufferPool;
			friend class D3D12GpuQueue;

			bool RecordBarriers(const GpuBarriers& barriers, GpuBarrierPhase phase, GpuSplitBarrier* split) override;

			/** Creates a command buffer wrapping @p commandList. */
			D3D12GpuCommandBuffer(D3D12GpuDevice& device, D3D12GpuCommandBufferPool& pool, u32 id, ID3D12GraphicsCommandList7* commandList, ID3D12CommandAllocator* commandAllocator, GpuQueueType queueType, const GpuCommandBufferCreateInformation& createInformation);

			/** Returns the pool the command buffer was allocated from. */
			D3D12GpuCommandBufferPool& GetPool() const { return mPool; }

			/** Makes the command buffer ready to start recording commands. */
			void Begin();

			/** Checks if all the prerequisites for rendering have been made. */
			bool IsReadyForRender() const;

			/** Marks the command buffer as submitted on a queue. */
			void SetIsSubmitted() { mState = GpuCommandBufferState::Executing; }

			/** Binds the current graphics pipeline to the command buffer. Returns true if bind was successful. */
			bool BindGraphicsPipeline();

			/** Binds any dynamic states to the pipeline, as required. */
			void BindDynamicStates(bool forceAll);

			/** Binds vertex and index buffers to the pipeline, if dirty. */
			void BindVertexInputs();

			/**
			 * Binds the currently stored GPU parameter sets, if dirty, and tracks compute accesses on every call.
			 * @p isGraphics selects whether the sets are bound to the graphics or the compute root signature;
			 * both pipeline types can be bound on the command buffer simultaneously, so the bind point cannot
			 * be inferred from the command buffer's state alone.
			 * Returns false if the pipeline layout is unavailable or resource usage is invalid.
			 */
			bool BindGpuParameterSets(bool isGraphics);

			/** Uploads the cached push-constant block for the selected pipeline bind point. */
			void BindPushConstants(bool isGraphics);

			/**
			 * Returns the framebuffer of the active swap chain image of @p renderWindow, and registers the swap chain with the command
			 * buffer. Returns null if the window has no surface.
			 */
			D3D12Framebuffer* AcquireWindowFramebuffer(RenderWindow& renderWindow);

			/** Clears the masked surfaces of the currently bound render target within @p area, to @p clearValues. */
			void ClearViewportArea(const Area2I& area, RenderSurfaceMask mask, const RenderTargetClearValues& clearValues);

			/** Remembers a query pool written to during the current recording so its results are resolved on End(). */
			void TrackQueryPool(const TShared<GpuQueryPool>& queryPool);

			/** Returns the current viewport area in pixels. */
			Area2I GetViewportArea() const;

			/** Returns the current area of the render pass in pixels. */
			Area2I GetRenderPassArea() const;

			/** Returns the owner GPU device, cast as a D3D12GpuDevice. */
			D3D12GpuDevice& GetD3D12GpuDevice() const { return static_cast<D3D12GpuDevice&>(mGpuDevice); }

			u32 mId;
			ComPtr<ID3D12GraphicsCommandList7> mCommandList;
			ComPtr<ID3D12CommandAllocator> mCommandAllocator; /**< Allocator of this command buffer only. Null if the pool shares one. */
			D3D12GpuCommandBufferPool& mPool;
			ComPtr<ID3D12Fence> mFence;
			u64 mFenceValue = 0;

			/** Tracks every resource used on the command buffer: lifetime (bound/use counts) and render pass attachments. */
			GpuResourceTracker mResourceTracker;

			/** Queue the command buffer was submitted on; identifies the queue for NotifyDone. */
			GpuQueueId mSubmittedQueueId;

			// Render state
			D3D12Framebuffer* mFramebuffer = nullptr;
			RenderSurfaceMask mRenderTargetReadOnlyMask = RT_NONE;

#if B3D_BUILD_TYPE_DEVELOPMENT
			/** Surfaces EndRenderPass() clears, and their values. See GpuBackendUtility::GetRenderPassDiscardClears(). */
			RenderSurfaceMask mRenderPassDiscardClearMask = RT_NONE;
			RenderTargetClearValues mRenderPassDiscardClearValues;
#endif

			TShared<D3D12GpuGraphicsPipelineState> mGraphicsPipeline;
			TShared<D3D12GpuComputePipelineState> mComputePipeline;
			TShared<VertexDescription> mVertexDescription;
			TShared<D3D12GpuBuffer> mIndexBuffer;
			Vector<TShared<D3D12GpuBuffer>> mVertexBuffers;
			Area2 mNormalizedViewportArea{ 0.0f, 0.0f, 1.0f, 1.0f };
			Area2I mScissor{ 0, 0, 0, 0 };
			bool mIsScissorTestEnabled = false;
			u32 mStencilReferenceValue = 0;
			DrawOperationType mDrawOperation = DOT_TRIANGLE_LIST;
			u32 mRequiredVertexBufferBindingCount = 0;
			D3D12Pipeline* mLastBoundGraphicsPipeline = nullptr; /**< Pipeline variant currently set on the command list. */

			bool mGraphicsPipelineRequiresBind : 1;
			bool mGraphicsRootSignatureRequiresBind : 1;
			bool mComputePipelineRequiresBind : 1;
			bool mPrimitiveTopologyRequiresBind : 1;
			bool mViewportRequiresBind : 1;
			bool mStencilReferenceValueRequiresBind : 1;
			bool mScissorRequiresBind : 1;
			bool mGraphicsParametersRequireBind : 1;
			bool mComputeParametersRequireBind : 1;
			bool mGraphicsPushConstantsRequireBind : 1;
			bool mComputePushConstantsRequireBind : 1;
			bool mVertexInputsDirty : 1;

			/** Shared values retained across partial writes and graphics/compute pipeline changes. */
			GpuPushConstantPayload mPushConstants;

			Vector<TShared<D3D12GpuParameters>> mBoundParameterSets; /**< Bound parameter sets, indexed by set. */
			TShared<RenderTarget> mRenderTarget;

			/**
			 * Per-set dynamic offset overrides (keyed by dynamic-offset index, see GpuPipelineParameterSetLayout::GetDynamicOffsetIndex), 
			 * applied to root CBV binds on top of the bound parameter sets' own offsets. A set's overrides are cleared when new parameters are bound on it.
			 */
			Vector<UnorderedMap<u32, u32>> mDynamicOffsetOverridesPerSet;

			/**
			 * Query pools written to during the current recording. On End() each pool's allocated queries are resolved
			 * into its readback buffer. The references also keep the pools alive until the command buffer is reset.
			 */
			Vector<TShared<GpuQueryPool>> mUsedQueryPools;
		};

		/** @} */
	} // namespace render
} // namespace b3d
