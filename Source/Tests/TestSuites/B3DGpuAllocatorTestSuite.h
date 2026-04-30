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
	};
} // namespace b3d
