//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DNullPrerequisites.h"
#include "GpuBackend/B3DGpuCommandBuffer.h"

namespace b3d
{
	namespace render
	{
		class NullGpuDevice;
		class NullGpuCommandBufferPool;

		/** @addtogroup NullGpuBackend 
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
			NullGpuCommandBuffer(NullGpuDevice& device, NullGpuCommandBufferPool& pool, u32 id, GpuQueueType queueType, const GpuCommandBufferCreateInformation& createInformation);
			~NullGpuCommandBuffer() override = default;

			void SetName(const StringView& name) override {}

			void SetDynamicBufferOffset(u32 set, u32 bufferIndex, u32 offset) override {}
			void SetVertexDescription(const TShared<VertexDescription>& vertexDescription) override {}
			void SetDrawOperation(DrawOperationType operation) override {}
			void EndRenderPass() override {}
			bool IsInRenderPass() const override { return false; }
			void SetViewport(const Area2& area) override {}
			void ClearRenderTarget(RenderSurfaceMask mask) override {}
			void ClearViewport(RenderSurfaceMask mask) override {}
			void EnableScissorTest(u32 left, u32 top, u32 right, u32 bottom) override {}
			void DisableScissorTest() override {}
			void SetStencilReferenceValue(u32 value) override {}
			void WriteTimestamp(GpuQueryId query, const TShared<GpuQueryPool>& queryPool) override {}
			void BeginQuery(GpuQueryId query, const TShared<GpuQueryPool>& queryPool, GpuQueryFlags flags) override {}
			void EndQuery(GpuQueryId query, const TShared<GpuQueryPool>& queryPool) override {}
			void ResetQueries(const TShared<GpuQueryPool>& queryPool) override {}
			void BeginLabel(const StringView& name) override {}
			void EndLabel() override {}
			void InsertLabel(const StringView& name) override {}
			void End() override;

			using GpuCommandBuffer::CopyBufferToBuffer;
			using GpuCommandBuffer::CopyBufferToTexture;
			using GpuCommandBuffer::CopyTextureToBuffer;
			using GpuCommandBuffer::CopyTexture;

			/** Copies between the CPU memory of the buffers, so data written through staging buffers reaches the destination. */
			void CopyBufferToBuffer(IGpuBufferResource& source, IGpuBufferResource& destination, TArrayView<const GpuBufferCopyRange> ranges) override;
			void CopyBufferToTexture(IGpuBufferResource& source, IGpuImageResource& destination, const GpuImageCopyInformation& information) override {}
			void CopyTextureToBuffer(IGpuImageResource& source, IGpuBufferResource& destination, const GpuImageCopyInformation& information) override {}
			void CopyTexture(IGpuImageResource& source, IGpuImageResource& destination, const GpuImageCopyInformation& information) override {}

			/** Writes a black image, the contents of every null window. */
			void CopyWindowToBuffer(const TShared<RenderWindow>& source, const TShared<GpuBuffer>& destination) override;

			/** Returns an unique identifier of this command buffer. */
			u32 GetId() const { return mId; }

		private:
			friend class NullGpuCommandBufferPool;
			friend class NullGpuQueue;
			friend class NullGpuDevice;

			/** Accepts every barrier, since nothing executes. */
			bool RecordBarriers(const GpuBarriers& barriers, GpuBarrierPhase phase, GpuSplitBarrier* split) override { return true; }

			/** @copydoc RecordBarriers(const GpuBarriers&, GpuBarrierPhase, GpuSplitBarrier*) */
			bool RecordBarriers(const GpuResourceBarriers& barriers) override { return true; }

			/** Sets the command buffer state. Only accessible by friends (pool, queue and device). */
			void SetState(GpuCommandBufferState state) { mState = state; }

			u32 mId;
		};

		/** @} */
	} // namespace render
} // namespace b3d
