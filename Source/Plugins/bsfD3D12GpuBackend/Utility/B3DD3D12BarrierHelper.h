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
		void RecordNativeBufferBarrier(IGpuBufferResource* buffer, const GpuBarrierScope& barrier);

		/** Adds a native transition for one logical image range. */
		void RecordNativeImageBarrier(IGpuImageResource* image, const GpuTextureSubresourceRange& subresourceRange, const GpuBarrierScope& barrier, GpuImageLayout& oldLayout, GpuImageLayout newLayout);

		/** Returns the destination stages of the preceding barrier recorded for @p buffer. */
		GpuStageFlags GetPrecedingBarrierDestinationStages(IGpuBufferResource* buffer) const;

		/** Returns the combined destination stages of preceding barriers for tracked image partitions overlapping @p subresourceRange. */
		GpuStageFlags GetPrecedingBarrierDestinationStages(IGpuImageResource* image, const GpuTextureSubresourceRange& subresourceRange) const;

		/** Returns the destination stages of the preceding physical barrier recorded for @p page. */
		GpuStageFlags GetPrecedingBarrierDestinationStages(D3D12BufferPage& page) const;

		/** Associates a queued native barrier with the page barrier chain it advances. */
		struct PendingBufferPageBarrier
		{
			PendingBufferPageBarrier(D3D12BufferPage* page, const GpuBarrierScope& barrier)
				: Page(page), Barrier(barrier)
			{ }

			D3D12BufferPage* Page = nullptr; /**< Physical page receiving the barrier. */
			GpuBarrierScope Barrier; /**< Barrier forming the page's latest chain link. */
		};

		D3D12BarrierBatch mBarriers;
		TInlineArray<PendingBufferPageBarrier, 8> mPendingBufferPageBarriers;
		GpuQueueType mQueueType;
	};

	extern template class TGpuBarrierHelper<D3D12BarrierHelper>;
} // namespace b3d::render
