//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DPrerequisites.h"
#include "Utility/B3DTArray.h"
#include "GpuBackend/B3DGpuResourceTracker.h"

namespace b3d::render
{
	/** @addtogroup GpuBackend
	 *  @{
	 */

	/**
	 * Helper class for building and issuing GPU memory barriers.
	 *
	 * A barrier helper accumulates the synchronization required before a set of resources can be used on a 
	 * command buffer, then emits it as native barriers and notifies the resource tracker so it can update its 
	 * hazard/layout bookkeeping.
	 *
	 * The resource tracker resolves hazards, layouts and subresource partitions before queuing barriers here. The helper
	 * translates those resolved barriers to the native API, batches them, and runs the post-emission tracker callbacks.
	 *
	 * Implemented with CRTP - a backend derives as `class XBarrierHelper : public TGpuBarrierHelper<XBarrierHelper>`
	 * and provides:
	 *  - @c RecordNativeBufferBarrier / @c RecordNativeImageBarrier - accumulate the native barrier for one
	 *    buffer/image. Called by the shared queueing path (befriend this template so they can stay private).
	 *  - @c Execute - emit the accumulated native barriers, then call ApplyPostBarrierTracking(), the tracker's
	 *    CommitPendingHazardRegistrations() and Clear().
	 *  - @c Clear - reset the backend-specific accumulation, then call Clear().
	 *  - @c HasBarriers - whether anything has been accumulated.
	 *
	 * @tparam	TDerived	The concrete backend barrier helper (CRTP self-type).
	 */
	template<class TDerived>
	class TGpuBarrierHelper
	{
	public:
		/**
		 * Constructs a barrier helper associated with the provided resource tracker.
		 *
		 * @param	resourceTracker		Object responsible for tracking all resource usages on a command buffer. It is
		 *								notified after queued barriers and layout transitions are emitted.
		 */
		TGpuBarrierHelper(TGpuResourceTracker<TDerived>* resourceTracker);

	protected:
		/** Information needed to update hazard tracking after barrier execution. Either Buffer or Image is set. */
		struct BarrierTrackingInfo
		{
			IGpuBufferResource* Buffer = nullptr;
			IGpuImageResource* Image = nullptr;
			GpuTextureSubresourceRange ImageSubresourceRange{};
			GpuBarrierScope Barrier;
		};

		/** Information needed to update layout after barrier execution. */
		struct LayoutTrackingInfo
		{
			IGpuImageResource* Image = nullptr;
			GpuTextureSubresourceRange SubresourceRange{};
			GpuImageLayout OldLayout = GpuImageLayout::Undefined;
			GpuImageLayout NewLayout = GpuImageLayout::Undefined;
		};

		/**
		 * Runs the post-barrier tracker callbacks for everything accumulated since the last Clear: advances tracked
		 * layouts, then records every source->destination barrier. The derived Execute must call this after emitting
		 * the native barriers and before committing the pending accesses.
		 */
		void ApplyPostBarrierTracking();

		/** Clears the shared accumulated tracking. The derived Clear must call this after resetting its native state. */
		void Clear();

		TGpuResourceTracker<TDerived>* mResourceTracker;

		TInlineArray<LayoutTrackingInfo, 4> mImageLayoutTracking;
		TInlineArray<BarrierTrackingInfo, 8> mBarrierTracking;

	private:
		friend class TGpuResourceTracker<TDerived>;

		/** Queues a resolved buffer barrier and its post-emission tracker update. */
		void QueueResolvedBufferBarrier(IGpuBufferResource* buffer, const GpuBarrierScope& barrier);

		/** Queues a resolved image barrier and its post-emission layout and hazard updates. */
		void QueueResolvedImageBarrier(IGpuImageResource* image, const GpuTextureSubresourceRange& subresourceRange, const GpuBarrierScope& barrier, GpuImageLayout oldLayout, GpuImageLayout newLayout);
	};

	/** @} */
}
