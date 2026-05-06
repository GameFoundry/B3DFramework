//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "Testing/B3DTestSuite.h"

namespace b3d
{
	/**
	 * Core-layer GPU allocator + timeline-fence tests. The allocator-contract / deferred-delete cases
	 * exercise the framework-level TGpuAllocator surface against a mock backend, while the fence
	 * cases run directly against the active GpuDevice. Backend-private heap-creation self-tests
	 * live in sibling plugin test DLLs (e.g. bsfVulkanGpuBackendTests.dll).
	 *
	 * @see  TGpuAllocator
	 * @see  GpuTimelineFence
	 */
	class GpuAllocatorTestSuite : public TestSuite
	{
	public:
		GpuAllocatorTestSuite();

	private:
		/**
		 * Compile-time + runtime contract surface of TGpuAllocator: trait static asserts, layout
		 * guarantees on TGpuResourceLocation, retire/drain ordering and IGpuResource callback dispatch.
		 */
		void TestGpuAllocatorContract();

		/**
		 * Behavioural cases for the deferred-delete queue: FIFO drain stops at the first incomplete
		 * entry, subsequent advances drain remaining entries in order, Flush(true) drains
		 * unconditionally, and the public Free path snapshots slot identity into the queue
		 * then resets the caller's location.
		 */
		void TestGpuAllocatorDeferredDelete();

		/**
		 * Newly-observed device reports a stable current frame index. The current frame is never
		 * marked complete (it is by definition still being recorded), and any frame at least
		 * kMaximumFramesInFlight ticks behind the current one is reported complete.
		 */
		void TestFrameTracker_InitialState();

		/**
		 * Calling EndFrame advances the device's frame index by one. After kMaximumFramesInFlight
		 * further ticks the original frame is reported complete by the IGpuFrameTracker interface.
		 */
		void TestFrameTracker_AdvancesOnEndFrame();

		/**
		 * A user-created fence carries explicit caller-supplied values: submitting with an
		 * info.SignalFences entry signals the fence at the requested value, and the value is
		 * observable via IsSignaled once the GPU has caught up.
		 */
		void TestUserCreatedFence_ExplicitSignal();

		/** Compiles the TLSF allocator against the mock backend, asserts trait validation, instantiates with a fresh heap. */
		void TestTlsf_ContractAndInitialState();

		/** Single allocate / free round-trip with deferred-fence drain. */
		void TestTlsf_SingleAllocateFree();

		/** Multiple allocations land at non-overlapping offsets aligned to the requested alignment. */
		void TestTlsf_NonOverlappingAlignedOffsets();

		/** Three adjacent allocations free in different orders all coalesce to a single trailing free range. */
		void TestTlsf_CoalesceAllOrders();

		/** Large alignment forces leading-padding split — the allocator must remain consistent across alloc/free cycles. */
		void TestTlsf_LargeAlignmentSplitsLeadingPadding();

		/** Heap grows when an allocation doesn't fit; freed empties beyond the spare budget are returned to the backend. */
		void TestTlsf_HeapGrowthAndEmptyRelease();

		/** Allocations exceeding MaxHeapSize land in dedicated heaps sized to fit them. */
		void TestTlsf_OversizedAllocationGetsDedicatedHeap();

		/** Random alloc/free workload — proves no leak, no overlap, full reclaim after clear. */
		void TestTlsf_RandomStressNoLeak();

		/** With granularity disabled (= 1), Linear/NonLinear allocations interleave without padding. */
		void TestTlsf_GranularityDisabled();

		/** All-Linear allocations across granularity boundaries don't trigger BIG-driven padding. */
		void TestTlsf_GranularityHomogeneousNoPadding();

		/** A NonLinear allocation following a Linear one is bumped past the granularity boundary. */
		void TestTlsf_GranularityLinearVsNonLinearInflatesPadding();

		/** When BIG inflation would overrun the heap, the allocator falls through and creates a new heap. */
		void TestTlsf_GranularityRejectAndRetryAcrossHeaps();

		/** Freeing a Linear allocation lets a subsequent NonLinear allocation reclaim the page without padding. */
		void TestTlsf_GranularityFreeReleasesRegion();

		/** Multi-heap drain: live allocations in a higher-index heap migrate to a lower-index heap and the vacated heap is released. */
		void TestTlsf_Defrag_DrainsHighestHeap();

		/** Single-heap configuration with a sawtooth pattern compacts allocations toward lower offsets within the same heap. */
		void TestTlsf_Defrag_SingleHeapWithinHeapCompaction();

		/** Per-call byte budget aborts the walk early and reports BudgetExhausted. */
		void TestTlsf_Defrag_RespectsBudget();

		/** Allocations whose Owner was left null at TryAllocate time are skipped; tracked allocations are not. */
		void TestTlsf_Defrag_OnlySkipsUntrackedSlots();

		/** A tracked allocation whose owner reports GetUseCount > 0 / GetBoundCount > 0 is still moved. */
		void TestTlsf_Defrag_MovesInFlightResource();

		/** MoveAllocation is invoked with a populated new Location identifying a live destination slot. */
		void TestTlsf_Defrag_MoveAllocationReceivesContext();

		/**
		 * Under FreeDeferralMode::ResourceLifecycle, Free routes straight to FreeImmediate;
		 * the slot is returned to the pool synchronously without going through the deferred-free queue.
		 */
		void TestTlsf_ResourceLifecyclePolicy_FreesImmediately();

		/**
		 * Under FreeDeferralMode::FrameTracker, Free queues the slot against the current frame
		 * index. The entry is held until kMaximumFramesInFlight further AdvanceFrame ticks pass
		 * and the queue is flushed.
		 */
		void TestTlsf_FrameTrackerPolicy_DefersAcrossFrames();

		/**
		 * Under FreeDeferralMode::ResourceLifecycle, MoveAllocation may return a different
		 * IGpuResource than the one it was called on (wrapper-swap pattern). The destination slot
		 * is owned by the returned pointer; the source slot is left for the consumer's destructor to
		 * free via FreeImmediate.
		 */
		void TestTlsf_Defrag_LifecycleAllowsSwap();

		/**
		 * Multiple threads driving concurrent TryAllocate / Free against a single allocator instance
		 * complete without data races and leave the allocator in a consistent post-flush state. Asserts
		 * that the recursive-mutex protection of the public surface is sufficient under contention.
		 */
		void TestTlsf_ConcurrentAllocateAndFree();

		/**
		 * Concurrent allocate/free workers running alongside a defragmentation thread. The defrag
		 * thread invokes MoveAllocation under the recursive lock; the allocator must not deadlock,
		 * lose track of slots, or corrupt the heap when a worker re-enters the lock through a freshly
		 * allocated location. Final state is verified to be coherent after all threads join.
		 */
		void TestTlsf_ConcurrentDefragWithAllocateAndFree();

		/**
		 * Compile-time + runtime smoke test for the ThreadUnsafe policy: instantiating the allocator
		 * with ThreadSafetyPolicy::ThreadUnsafe compiles, all locking primitives optimize out, and
		 * a simple allocate / free round-trip behaves identically to the thread-safe instantiation.
		 */
		void TestTlsf_ThreadUnsafePolicyOptOut();
	};
} // namespace b3d
