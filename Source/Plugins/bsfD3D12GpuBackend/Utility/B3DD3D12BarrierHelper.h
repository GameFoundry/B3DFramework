//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DD3D12Prerequisites.h"
#include "GpuBackend/B3DGpuCommandBuffer.h"
#include "GpuBackend/B3DGpuBarrierHelper.h"
#include "Utility/B3DTArray.h"

namespace b3d::render
{
	class D3D12ResourceTracker;

	/** @addtogroup D3D12GpuBackend
	 *  @{
	 */

	/**
	 * Accumulates the synchronization required before a set of resources can be used on a D3D12 command buffer, and
	 * emits it as native resource barriers.
	 *
	 * D3D12 fuses image layout and access into a single per-(sub)resource D3D12_RESOURCE_STATES value, and a
	 * transition barrier both moves the state and synchronizes the accesses on either side. Consequently:
	 *  - Image barriers become per-subresource transitions from the state stored on the D3D12ImageSubresource (which
	 *    is guaranteed to match what was last emitted) to the state derived from the destination layout/access.
	 *  - Same-state write hazards (UAV -> UAV) become D3D12_RESOURCE_BARRIER_TYPE_UAV barriers.
	 *  - Buffer state changes are driven by RequireBufferState() from D3D12ResourceTracker::TrackBufferUsage, because
	 *    buffers carry no layout in the core model and read->read state changes (e.g. COPY_SOURCE -> SRV) produce no
	 *    hazard callback from the shared tracker. The RecordBufferBarrier hook therefore only contributes UAV
	 *    barriers; the transition emitted by RequireBufferState provides the execution/memory synchronization.
	 */
	class D3D12BarrierHelper : public TGpuBarrierHelper<D3D12BarrierHelper>
	{
	public:
		/**
		 * Constructs a barrier helper associated with the provided resource tracker.
		 *
		 * @param	resourceTracker		Tracker that owns the per-command-buffer resource state. Used to look up current
		 *								state and notified with new state once the barriers are emitted.
		 */
		D3D12BarrierHelper(D3D12ResourceTracker* resourceTracker);

		/**
		 * Emits all accumulated native barriers on the provided command buffer, then runs the core hazard/layout
		 * bookkeeping callbacks and clears the helper. No-op (apart from committing deferred hazard registrations)
		 * if nothing was queued.
		 */
		void Execute(D3D12GpuCommandBuffer& commandBuffer);

		/** Clears all accumulated state without emitting anything. */
		void Clear();

		/** Returns true if any barrier has been accumulated and is ready to emit. */
		bool HasBarriers() const { return !mNativeBarriers.Empty(); }

		/**
		 * Ensures the buffer's native state equals @p state, appending a transition if it does not. UPLOAD-heap
		 * buffers (permanently GENERIC_READ) and READBACK-heap buffers (permanently COPY_DEST) are skipped. The
		 * buffer's tracked state is advanced immediately (record order).
		 */
		void RequireBufferState(D3D12Buffer* buffer, D3D12_RESOURCE_STATES state);

		/**
		 * Ensures the image subresource's native state equals @p state, appending a transition if it does not. The
		 * subresource's tracked state is advanced immediately (record order). Used for the first use of a subresource
		 * on a command buffer, which seeds the shared tracker's layout without invoking the barrier hooks (see
		 * D3D12ResourceTracker::TrackImageUsage).
		 */
		void RequireSubresourceState(D3D12Image* image, u32 face, u32 mipLevel, D3D12_RESOURCE_STATES state);

	private:
		friend class TGpuBarrierHelper<D3D12BarrierHelper>;

		/**
		 * CRTP hook: same-state write hazards on buffers become UAV barriers. State-changing transitions are handled
		 * by RequireBufferState (see class doc). Called by the shared low-level path.
		 */
		void RecordBufferBarrier(IGpuBufferResource* buffer, const GpuHazardStageAndAccess& barrier);

		/**
		 * CRTP hook: accumulates per-subresource native transitions from each subresource's stored state to the state
		 * derived from @p newLayout + destination access (or from the destination stages when the layout is
		 * Undefined). Same-state write hazards become UAV barriers. Called by the shared low-level path.
		 */
		void RecordSubresourceBarrier(IGpuImageResource* image, const GpuTextureSubresourceRange& subresourceRange,
			const GpuHazardStageAndAccess& barrier, GpuImageLayout& oldLayout, GpuImageLayout newLayout);

		/** Appends a transition barrier for a single (sub)resource. */
		void AppendTransition(ID3D12Resource* resource, u32 nativeSubresource, D3D12_RESOURCE_STATES before, D3D12_RESOURCE_STATES after);

		/** Appends a UAV barrier for the resource, deduplicated against those already queued this batch. */
		void AppendUavBarrier(ID3D12Resource* resource);

		TInlineArray<D3D12_RESOURCE_BARRIER, 8> mNativeBarriers;

		/** Resources that already have a UAV barrier queued this batch (deduplication). */
		TInlineArray<ID3D12Resource*, 4> mUavBarrierResources;
	};

	extern template class TGpuBarrierHelper<D3D12BarrierHelper>;

	/** @} */
} // namespace b3d::render
