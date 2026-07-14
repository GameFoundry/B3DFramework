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
	 * All of the backend-independent work lives here: converting resource usage to pipeline stages, the WAW/RAW/WAR
	 * hazard analysis that derives the source access from the tracked state, subdividing a subresource range into the
	 * tracked blocks it overlaps, accumulating the per-barrier tracking info, and running the post-barrier tracker
	 * callbacks. Only the actual native barrier accumulation and emission is backend-specific.
	 *
	 * Implemented with CRTP - a backend derives as `class XBarrierHelper : public TGpuBarrierHelper<XBarrierHelper>`
	 * and provides:
	 *  - @c RecordBufferBarrier / @c RecordSubresourceBarrier - accumulate the native barrier for one
	 *    buffer/image. Called by the shared low-level path (befriend this template so they can stay private).
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
		/** Information needed to update hazard tracking after barrier execution. Either Buffer or Image is set. */
		struct BarrierTrackingInfo
		{
			IGpuBufferResource* Buffer = nullptr;
			IGpuImageResource* Image = nullptr;
			GpuTextureSubresourceRange ImageSubresourceRange{};
			GpuHazardStageAndAccess StageAndAccess;
		};

		/**
		 * Constructs a barrier helper associated with the provided resource tracker.
		 *
		 * @param	resourceTracker		Object responsible for tracking all resource usages on a command buffer. Used for
		 *								determining current object state, and notified with new state when barriers and
		 *								layout transitions are executed.
		 */
		TGpuBarrierHelper(TGpuResourceTracker<TDerived>* resourceTracker);

		/**
		 * Adds a memory barrier for a buffer resource.
		 *
		 * @param buffer				Buffer to add barrier for.
		 * @param sourceUsage			How the buffer was used before the barrier.
		 * @param sourceAccess			Type of access (read/write) before the barrier.
		 * @param destinationUsage		How the buffer will be used after the barrier.
		 * @param destinationAccess		Type of access (read/write) after the barrier.
		 * @return						Information about a barrier that was queued, or null if none was queued. Only valid until next call to Add/Execute/Clear.
		 */
		const BarrierTrackingInfo* AddBufferBarrier(IGpuBufferResource* buffer, GpuResourceUseFlags sourceUsage, GpuAccessFlags sourceAccess, GpuResourceUseFlags destinationUsage, GpuAccessFlags destinationAccess);

		/** Adds a memory barrier for a buffer resource. Automatically deduces source usage/access from current tracked state. */
		const BarrierTrackingInfo* AddBufferBarrier(IGpuBufferResource* buffer, GpuResourceUseFlags destinationUsage, GpuAccessFlags destinationAccess);

		/** Adds a memory barrier for a buffer resource. Automatically deduces source usage/access from provided tracked state. */
		const BarrierTrackingInfo* AddBufferBarrier(IGpuBufferResource* buffer, const GpuBufferTrackingState& bufferTrackingState, GpuResourceUseFlags destinationUsage, GpuAccessFlags destinationAccess);

		/**
		 * Adds a memory barrier for an image resource.
		 *
		 * @param image						Image to add barrier for.
		 * @param subresourceRange			Subresource range of the image to barrier.
		 * @param sourceUsage				How the image was used before the barrier.
		 * @param sourceAccessFlags			Type of access (read/write) before the barrier.
		 * @param destinationUsage			How the image will be used after the barrier.
		 * @param destinationAccessFlags	Type of access (read/write) after the barrier.
		 * @param oldLayout					Current layout of the image before the barrier.
		 * @param newLayout					Layout the image will be transitioned to after the barrier.
		 * @return							Information about a barrier that was queued, or null if none was queued. Only valid until next call to Add/Execute/Clear.
		 */
		const BarrierTrackingInfo* AddImageBarrier(IGpuImageResource* image, const GpuTextureSubresourceRange& subresourceRange, GpuResourceUseFlags sourceUsage, GpuAccessFlags sourceAccessFlags, GpuResourceUseFlags destinationUsage, GpuAccessFlags destinationAccessFlags, GpuImageLayout oldLayout, GpuImageLayout newLayout);

		/** Adds a memory barrier for an image resource. Automatically deduces source usage/access and layout from current tracked state. */
		const BarrierTrackingInfo* AddImageBarrier(IGpuImageResource* image, const GpuTextureSubresourceRange& subresourceRange, GpuResourceUseFlags destinationUsage, GpuAccessFlags destinationAccess, GpuImageLayout newLayout);

		/** Adds a memory barrier for an existing subresource of an image resource. Automatically deduces source usage/access and layout from provided tracked state. */
		const BarrierTrackingInfo* AddSubresourceBarrier(IGpuImageResource* image, const GpuImageSubresourceTrackingState& subresourceTrackingState, GpuResourceUseFlags destinationUsage, GpuAccessFlags destinationAccess, GpuImageLayout newLayout);

	protected:
		/** Information needed to update layout after barrier execution. */
		struct LayoutTrackingInfo
		{
			IGpuImageResource* Image = nullptr;
			GpuTextureSubresourceRange SubresourceRange{};
			GpuImageLayout OldLayout = GpuImageLayout::Undefined;
			GpuImageLayout NewLayout = GpuImageLayout::Undefined;
		};

		/**
		 * Shared low-level buffer barrier path on explicit stage masks. Asks the derived backend to accumulate the native
		 * barrier (RecordBufferBarrier), then records the bookkeeping needed for the post-barrier tracker updates.
		 */
		const BarrierTrackingInfo* AddBufferBarrier(IGpuBufferResource* buffer, const GpuHazardStageAndAccess& stageAndAccess);

		/**
		 * Shared low-level image subresource barrier path on explicit stage masks. Asks the derived backend to accumulate
		 * the native barrier (RecordSubresourceBarrier; it may reconcile @p oldLayout from an already-merged
		 * barrier), then records the layout transition and bookkeeping needed for the post-barrier tracker updates.
		 */
		const BarrierTrackingInfo* AddSubresourceBarrier(IGpuImageResource* image, const GpuTextureSubresourceRange& subresourceRange, const GpuHazardStageAndAccess& stageAndAccess, GpuImageLayout oldLayout, GpuImageLayout newLayout);

		/**
		 * Runs the post-barrier tracker callbacks for everything accumulated since the last Clear: advances the tracked
		 * layout for each recorded transition, then marks each source->destination pair safe to access. The derived
		 * Execute must call this after emitting the native barriers (and before CommitPendingHazardRegistrations).
		 */
		void ApplyPostBarrierTracking();

		/** Clears the shared accumulated tracking. The derived Clear must call this after resetting its native state. */
		void Clear();

		TGpuResourceTracker<TDerived>* mResourceTracker;

		TInlineArray<LayoutTrackingInfo, 4> mImageLayoutTracking;
		TInlineArray<BarrierTrackingInfo, 8> mBarrierTracking;
	};

	/** @} */
}
