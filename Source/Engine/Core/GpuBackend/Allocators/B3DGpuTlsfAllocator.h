//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DPrerequisites.h"
#include "GpuBackend/Allocators/B3DGpuAllocator.h"
#include "GpuBackend/B3DGpuTimelineFence.h"
#include "Allocators/B3DTlsfAllocator.h"
#include "Utility/B3DBitwise.h"
#include "Utility/B3DFlags.h"

namespace b3d
{
	/** @addtogroup GpuBackend-Internal
	 *  @{
	 */

	// The TLSF heap machinery in detail::tlsf is kind-agnostic (TlsfAllocationKind); GpuResourceKind maps
	// onto it by value. Guarded here so the casts below stay provably value-preserving.
	static_assert((u8)GpuResourceKind::Linear == (u8)TlsfAllocationKind::Linear && (u8)GpuResourceKind::NonLinear == (u8)TlsfAllocationKind::NonLinear,
		"GpuResourceKind values must match TlsfAllocationKind — the TLSF allocator casts between them.");

	/** Maps a GPU resource kind onto the equivalent TLSF allocation kind. */
	inline TlsfAllocationKind ToTlsfKind(GpuResourceKind kind) { return (TlsfAllocationKind)(u8)kind; }

	/**
	 * Two-Level Segregated Fit GPU memory allocator. O(1) bitmap-driven bucket lookup, leading-padding
	 * split for alignment, full coalescing on free, multi-heap growable. One allocator instance manages
	 * a list of backend heaps; allocations report back to the consumer via GpuAllocation, with
	 * the heap index and pool node index stored in the allocation's two strategy-private slots.
	 *
	 * **Threading.** When ThreadPolicy is ThreadSafe (the default), every public entry point — including
	 * TryAllocate, Free, FreeAndReclaim, ReclaimUnused, PlanDefragmentation, SetAllocationOwner and the diagnostic accessors —
	 * acquires the allocator-wide mutex inherited from TGpuAllocator. When ThreadPolicy is ThreadUnsafe,
	 * the locking compiles out and the caller is responsible for external synchronization.
	 *
	 * **Buffer-image granularity.** A single allocator instance can host mixed linear (buffer / linear image)
	 * and non-linear (optimally-tiled image) allocations safely; pass the appropriate GpuResourceKind to
	 * TryAllocate. The configured Granularity (the backend's buffer-image granularity) drives the mandatory padding between conflicting
	 * neighbors. When the configured granularity is at or below GranularityDisableThreshold the tracker
	 * is fully inert and adds zero per-allocation overhead.
	 *
	 * @tparam HeapBackend	Backend trait satisfying the GpuHeapBackend contract.
	 * @tparam ThreadPolicy	Compile-time thread-safety policy. ThreadSafe (default) wraps state with a
	 * 						RecursiveMutex; ThreadUnsafe compiles out all locking.
	 *
	 * @see TGpuAllocator
	 */
	template <typename HeapBackend, ThreadSafetyPolicy ThreadPolicy = ThreadSafe>
	class TGpuTlsfAllocator : public TGpuAllocator<TGpuTlsfAllocator<HeapBackend, ThreadPolicy>, HeapBackend, ThreadPolicy>
	{
	public:
		using Base = TGpuAllocator<TGpuTlsfAllocator<HeapBackend, ThreadPolicy>, HeapBackend, ThreadPolicy>;
		using HeapHandle = typename HeapBackend::HeapHandle;

		/**
		 * General-purpose TLSF orchestrator this allocator composes. Always ThreadUnsafe — every path into it
		 * already holds this allocator's own mutex (see the Threading note in the class documentation).
		 */
		using InternalAllocator = TTlsfAllocator<HeapBackend, ThreadUnsafe>;

		/**
		 * Runtime configuration for the allocator. Extends the general TLSF configuration (heap sizing, growth,
		 * granularity) with the GPU-only options. For GPU heaps Granularity is the backend's buffer-image
		 * granularity, e.g. Vulkan's VkPhysicalDeviceLimits::bufferImageGranularity.
		 */
		struct Configuration : InternalAllocator::Configuration
		{
			/**
			 * Controls how the allocator tracks when allocations are no longer used on the GPU, so it may safely
			 * free them.
			 */
			GpuAllocatorFreeDeferralMode DeferralMode = GpuAllocatorFreeDeferralMode::FrameTracker;
		};

		TGpuTlsfAllocator(HeapBackend* backend, IGpuCompletionTracker* completionTracker, const Configuration& configuration);
		~TGpuTlsfAllocator();

		// Non-copyable — node pool and heap state are not safe to duplicate.
		TGpuTlsfAllocator(const TGpuTlsfAllocator&) = delete;
		TGpuTlsfAllocator& operator=(const TGpuTlsfAllocator&) = delete;

		/** @name TGpuAllocator CRTP surface.
		 *  @{
		 */

		bool TryAllocateImpl(u64 size, u32 alignment, GpuResourceKind kind, GpuAllocation& out);
		void FreeImpl(GpuAllocation& allocation);
		void FreeAndReclaimImpl(u32 heapIndex, u32 nodeIndex);

		/** @copydoc IGpuAllocator::SetAllocationOwner */
		void SetAllocationOwner(const GpuAllocation& allocation, GpuAllocationOwner owner) override;

		/** @copydoc IGpuAllocator::SupportsDefragmentation */
		bool SupportsDefragmentation() const override { return true; }

		/**
		 * @copydoc IGpuAllocator::PlanDefragmentation
		 *
		 * Heaps drain from the highest index, each from its highest offset. An allocation moves to a lower offset in its own
		 * heap or into a lower-index heap, with the alignment it was allocated with.
		 */
		void PlanDefragmentation(const GpuDefragmentationBudget& budget, const Function<bool(GpuAllocationOwner)>& fnIsMovable,
			TArray<GpuAllocationMove>& outMoves) override;

		/** @} */

		/** @name Diagnostics.
		 *  @{
		 */

		/** Total number of bytes allocated by all underlying heaps. */
		u64 GetCommittedBytes() const { typename Base::ScopedLock lock(this->GetMutex()); return mInternal.GetCommittedBytes(); }

		/** Total number of bytes currently held by live (non-retired, non-freed) allocations. */
		u64 GetUsedBytes() const { typename Base::ScopedLock lock(this->GetMutex()); return mInternal.GetUsedBytes(); }

		/** Number of populated heap slots (vacated slots are not counted). */
		u32 GetHeapCount() const { typename Base::ScopedLock lock(this->GetMutex()); return mInternal.GetHeapCount(); }

		/** Number of fully-empty heaps currently retained as spares. */
		u32 GetEmptyHeapCount() const { typename Base::ScopedLock lock(this->GetMutex()); return mInternal.GetEmptyHeapCount(); }

		/** @} */

	private:
		using InternalAllocation = typename InternalAllocator::Allocation;
		using Heap = typename InternalAllocator::Heap;

		/** (heap, node) pair of a destination reserved in a PlanDefragmentation() pass, flagged until the pass ends. */
		struct DefragDestinationKey
		{
			u32 HeapIndex;
			u32 NodeIndex;
		};

		InternalAllocator mInternal;
		GpuAllocatorFreeDeferralMode mDeferralMode;
	};


	/** @} */
} // namespace b3d

#include "GpuBackend/Allocators/B3DGpuTlsfAllocator.inl"
