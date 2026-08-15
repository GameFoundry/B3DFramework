//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DD3D12Prerequisites.h"
#include "B3DD3D12BarrierBatch.h"
#include "GpuBackend/B3DGpuBarrierHelper.h"

namespace b3d::render
{
	class D3D12ResourceTracker;

	/** Accumulates and emits D3D12 enhanced barriers for one command buffer. */
	class D3D12BarrierHelper : public TGpuBarrierHelper<D3D12BarrierHelper>
	{
	public:
		/** Creates a barrier collector for command buffers recorded for @p queueType. */
		D3D12BarrierHelper(D3D12ResourceTracker* resourceTracker, GpuQueueType queueType);

		/** Emits pending native barriers and commits their logical tracking updates. */
		void Execute(D3D12GpuCommandBuffer& commandBuffer);

		/** Discards barriers and per-command accesses that have not been emitted. */
		void Clear();

		/** Returns whether native barriers are pending. */
		bool HasBarriers() const { return !mBarriers.IsEmpty(); }

	private:
		friend class TGpuBarrierHelper<D3D12BarrierHelper>;
		friend class D3D12ResourceTracker;

		/** Adds a native barrier for the resource backing @p buffer. */
		void RecordBufferBarrier(IGpuBufferResource* buffer, const GpuBarrierScope& barrier);

		/** Adds a native barrier for a pooled physical buffer page. */
		void RecordBufferPageBarrier(D3D12BufferPage& page, const GpuBarrierScope& barrier);

		/** Adds a native transition for one logical image range. */
		void RecordSubresourceBarrier(IGpuImageResource* image, const GpuTextureSubresourceRange& subresourceRange, const GpuBarrierScope& barrier, GpuImageLayout& oldLayout, GpuImageLayout newLayout);

		/** Returns the preceding barrier recorded for @p buffer, if any. */
		GpuBarrierScope GetLastBarrier(IGpuBufferResource* buffer) const;

		/** Returns the combined preceding barrier for tracked image partitions overlapping @p subresourceRange. */
		GpuBarrierScope GetLastBarrier(IGpuImageResource* image, const GpuTextureSubresourceRange& subresourceRange) const;

		/** Returns the preceding physical barrier recorded for @p page, if any. */
		GpuBarrierScope GetLastBarrier(D3D12BufferPage& page) const;

		D3D12BarrierBatch mBarriers;
		GpuQueueType mQueueType;
	};

	extern template class TGpuBarrierHelper<D3D12BarrierHelper>;
} // namespace b3d::render
