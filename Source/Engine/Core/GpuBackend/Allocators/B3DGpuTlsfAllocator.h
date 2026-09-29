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
	 * TryAllocate, Free, FreeAndReclaim, ReclaimUnused, Defrag, SetAllocationOwner and the diagnostic accessors —
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

		bool TryAllocateImpl(u64 size, u32 alignment, GpuResourceKind kind, IGpuResource* owner, GpuAllocation& out);
		void FreeImpl(GpuAllocation& allocation);
		void FreeAndReclaimImpl(u32 heapIndex, u32 nodeIndex);

		/**
		 * Stamps an IGpuResource owner onto a previously-allocated slot. Used by callers that can't
		 * pass the owner at TryAllocate time (e.g. backends where the allocation is performed before
		 * the IGpuResource wrapper exists, then the wrapper registers itself post-construction).
		 * Pass nullptr to clear the owner — the slot remains live but becomes ineligible for defrag.
		 */
		void SetAllocationOwner(const GpuAllocation& allocation, IGpuResource* owner);

		/** TLSF tracks per-allocation owners and relocates allocations during Defrag. */
		bool SupportsDefragmentation() const override { return true; }

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

		/** @name Defragmentation.
		 *  @{
		 */

		/** Per-call budget for Defrag. Soft caps that stop the walk early when exceeded. */
		struct DefragmentationInfo
		{
			/** Maximum number of bytes copied per call. 0 = unlimited. */
			u64 MaxBytesPerCall = 32ull * 1024 * 1024;

			/** Maximum number of moves per call. 0 = unlimited. */
			u32 MaxAllocationsPerCall = 0;
		};

		/** Counters reported by Defrag. */
		struct DefragmentationStats
		{
			/** Number of candidate slots that passed eligibility filtering and where a move was attempted. */
			u32 MovesAttempted = 0;

			/** Number of moves where a destination slot was successfully reserved. */
			u32 MovesCompleted = 0;

			/** Total bytes covered by completed moves. */
			u64 BytesMoved = 0;

			/** True if either of the DefragmentationInfo budgets aborted the walk early. */
			bool BudgetExhausted = false;
		};

		/**
		 * Compacts live allocations by moving them into lower-offset / lower-index slots.
		 * Every tracked allocation (non-null owner) is a candidate.
		 *
		 * @param commandBuffer	Command buffer the consumer's recreate-and-copy path records into.
		 * @param info			Soft per-call budgets.
		 * @return				Counters for moves attempted / completed and budget-exhausted state.
		 */
		DefragmentationStats Defrag(render::GpuCommandBuffer& commandBuffer, const DefragmentationInfo& info = {});

		/** @} */

	private:
		using InternalAllocation = typename InternalAllocator::Allocation;
		using Heap = typename InternalAllocator::Heap;

		/** (heap, node) pair tracked across a Defrag() pass for end-of-pass DefragDestination flag clear. */
		struct DefragDestinationKey
		{
			u32 HeapIndex;
			u32 NodeIndex;
		};

		/**
		 * Reserves a destination slot for the live allocation at @p sourceNodeIndex, dispatches the
		 * consumer's MoveAllocation, and (under FreeDeferralMode::FrameTracker) retires the source
		 * allocation against the current frame index. Under FreeDeferralMode::ResourceLifecycle the
		 * source slot is left untracked and freed by the consumer's destructor. Returns true on a
		 * successful move and writes the chosen destination heap and node indices to the out
		 * parameters; false if no destination slot was available within @p sourceHeapIndex inclusive.
		 */
		bool TryMoveAllocation(u32 sourceNodeIndex, u32 sourceHeapIndex, render::GpuCommandBuffer& commandBuffer, u32& outDestinationHeapIndex, u32& outDestinationNodeIndex);

		InternalAllocator mInternal;
		GpuAllocatorFreeDeferralMode mDeferralMode;
	};


	/** @} */
} // namespace b3d

#include "GpuBackend/Allocators/B3DGpuTlsfAllocator.inl"
