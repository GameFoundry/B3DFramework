//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DNullPrerequisites.h"
#include "RenderAPI/B3DGpuCommandBuffer.h"

namespace b3d
{
	namespace render
	{
		class NullGpuDevice;
		class NullGpuCommandBufferPool;

		/** @addtogroup Null
		 *  @{
		 */

		/**
		 * Command buffer implementation for Null backend.
		 *
		 * Accepts all rendering commands but does not execute them. State tracking is implemented
		 * to allow proper command buffer lifecycle management (Ready -> Done transitions).
		 */
		class NullGpuCommandBuffer final : public GpuCommandBuffer
		{
		public:
			NullGpuCommandBuffer(NullGpuDevice& device, NullGpuCommandBufferPool& pool, u32 id, ThreadId ownerThread, GpuQueueUsage queueType, const GpuCommandBufferCreateInformation& createInformation);
			~NullGpuCommandBuffer() override = default;

			void SetName(const StringView& name) override { mName = name; }
			CommandBufferState GetState() const override { return mState; }

			void SetGpuParameterSet(const SPtr<GpuParameterSet>& parameters) override {}
			void SetDynamicBufferOffset(u32 set, u32 bufferIndex, u32 offset) override {}
			void SetGpuGraphicsPipelineState(const SPtr<GpuGraphicsPipelineState>& pipelineState) override {}
			void SetGpuComputePipelineState(const SPtr<GpuComputePipelineState>& pipelineState) override {}
			void SetVertexBuffers(u32 index, SPtr<GpuBuffer>* buffers, u32 bufferCount) override {}
			void SetIndexBuffer(const SPtr<GpuBuffer>& buffer) override {}
			void SetVertexDescription(const SPtr<VertexDescription>& vertexDescription) override {}
			void SetDrawOperation(DrawOperationType operation) override {}
			void Draw(u32 vertexOffset, u32 vertexCount, u32 instanceCount, u32 firstInstance) override {}
			void DrawIndexed(u32 startIndex, u32 indexCount, u32 vertexOffset, u32 vertexCount, u32 instanceCount, u32 firstInstance) override {}
			void DispatchCompute(u32 groupCountX, u32 groupCountY, u32 groupCountZ) override {}
			void BeginRenderPass(const SPtr<RenderTarget>& target, u32 readOnlyFlags, RenderSurfaceMask loadMask) override {}
			void EndRenderPass() override {}
			bool IsInRenderPass() const override { return false; }
			void SetViewport(const Area2& area) override {}
			void ClearRenderTarget(u32 buffers, const Color& color, float depth, u16 stencil, u8 targetMask) override {}
			void ClearViewport(u32 buffers, const Color& color, float depth, u16 stencil, u8 targetMask) override {}
			void EnableScissorTest(u32 left, u32 top, u32 right, u32 bottom) override {}
			void DisableScissorTest() override {}
			void SetStencilReferenceValue(u32 value) override {}
			void CopyBufferToBuffer(const SPtr<GpuBuffer>& source, const SPtr<GpuBuffer>& destination, u32 sourceOffset, u32 destinationOffset, u32 length) override {}
			void WriteTimestamp(GpuQueryId query, const SPtr<GpuQueryPool>& queryPool) override {}
			void BeginQuery(GpuQueryId query, const SPtr<GpuQueryPool>& queryPool, GpuQueryFlags flags) override {}
			void EndQuery(GpuQueryId query, const SPtr<GpuQueryPool>& queryPool) override {}
			void ResetQueries(const SPtr<GpuQueryPool>& queryPool) override {}
			void BeginLabel(const StringView& name) override {}
			void EndLabel() override {}
			void InsertLabel(const StringView& name) override {}
			void End() override;
			void TransitionTextureLayout(const SPtr<Texture>& texture, GpuTextureLayout layout, const GpuTextureSubresourceRange& subresourceRange) override {}
			void IssueBarriers(const GpuBarriers& barriers) override {}

			/** Returns an unique identifier of this command buffer. */
			u32 GetId() const { return mId; }

		private:
			friend class NullGpuCommandBufferPool;
			friend class NullGpuQueue;

			/** Sets the command buffer state. Only accessible by friends (pool and queue). */
			void SetState(CommandBufferState state) { mState = state; }

			u32 mId;
			String mName;
			CommandBufferState mState = CommandBufferState::Ready;
		};

		/** @} */
	} // namespace render
} // namespace b3d
