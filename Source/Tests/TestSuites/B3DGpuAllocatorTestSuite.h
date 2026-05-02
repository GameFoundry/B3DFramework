//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "Testing/B3DTestSuite.h"

namespace b3d
{
	/**
	 * Core-layer GPU allocator + timeline-fence tests. The allocator-contract / deferred-delete cases
	 * exercise the framework-level @c TGpuAllocator surface against a mock backend, while the fence
	 * cases run directly against the active @c GpuDevice. Backend-private heap-creation self-tests
	 * live in sibling plugin test DLLs (e.g. @c bsfVulkanGpuBackendTests.dll).
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
		 * unconditionally, and the public Deallocate path snapshots slot identity into the queue
		 * then resets the caller's location.
		 */
		void TestGpuAllocatorDeferredDelete();

		/**
		 * Newly-constructed device reports a zero submission counter and the predicate accepts
		 * IsSubmissionComplete(0) trivially.
		 */
		void TestSubmissionFence_InitialState();

		/**
		 * Submitting an empty transfer command buffer advances the device's submission counter
		 * and the resulting index is reported complete after the device idles.
		 */
		void TestSubmissionFence_AdvancesAfterSubmit();

		/**
		 * A user-created fence carries explicit caller-supplied values: submitting with an
		 * info.SignalFences entry signals the fence at the requested value, and the value is
		 * observable via IsSignaled once the GPU has caught up.
		 */
		void TestUserCreatedFence_ExplicitSignal();

		/** Compiles the TLSF allocator against the mock backend, asserts trait validation, instantiates with a fresh heap. */
		void TestTlsf_ContractAndInitialState();

		/** Single allocate / deallocate round-trip with deferred-fence drain. */
		void TestTlsf_SingleAllocateDeallocate();

		/** Multiple allocations land at non-overlapping offsets aligned to the requested alignment. */
		void TestTlsf_NonOverlappingAlignedOffsets();

		/** Three adjacent allocations free in different orders all coalesce to a single trailing free range. */
		void TestTlsf_CoalesceAllOrders();

		/** Large alignment forces leading-padding split — the allocator must remain consistent across alloc/free cycles. */
		void TestTlsf_LargeAlignmentSplitsLeadingPadding();

		/** Heap grows when an allocation doesn't fit; freed empties beyond the spare budget are returned to the backend. */
		void TestTlsf_HeapGrowthAndEmptyRelease();

		/** Allocations exceeding @c MaxHeapSize land in dedicated heaps sized to fit them. */
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

		/** OnAllocationMoved is invoked with the upcoming submission index and a populated new Location identifying a live destination slot. */
		void TestTlsf_Defrag_OnAllocationMovedReceivesContext();
	};
} // namespace b3d
