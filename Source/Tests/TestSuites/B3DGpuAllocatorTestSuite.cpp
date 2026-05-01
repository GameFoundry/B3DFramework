//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DGpuAllocatorTestSuite.h"
#include "GpuBackend/B3DGpuBackend.h"
#include "GpuBackend/B3DGpuDevice.h"
#include "GpuBackend/B3DGpuDeviceCapabilities.h"
#include "GpuBackend/B3DGpuTimelineFence.h"
#include "GpuBackend/B3DGpuCommandBuffer.h"
#include "GpuBackend/B3DGpuCommandBufferPoolRing.h"
#include "GpuBackend/Allocators/B3DGpuAllocator.h"
#include "GpuBackend/Allocators/B3DGpuTlsfAllocator.h"
#include "GpuBackend/Allocators/B3DGpuResource.h"

#include <random>

using namespace b3d;
using namespace b3d::render;

GpuAllocatorTestSuite::GpuAllocatorTestSuite()
	: TestSuite("GpuAllocatorTestSuite")
{
	B3D_ADD_TEST(GpuAllocatorTestSuite::TestGpuAllocatorContract)
	B3D_ADD_TEST(GpuAllocatorTestSuite::TestGpuAllocatorDeferredDelete)
	B3D_ADD_TEST(GpuAllocatorTestSuite::TestSubmissionFence_InitialState)
	B3D_ADD_TEST(GpuAllocatorTestSuite::TestSubmissionFence_AdvancesAfterSubmit)
	B3D_ADD_TEST(GpuAllocatorTestSuite::TestUserCreatedFence_ExplicitSignal)
	B3D_ADD_TEST(GpuAllocatorTestSuite::TestTlsf_ContractAndInitialState)
	B3D_ADD_TEST(GpuAllocatorTestSuite::TestTlsf_SingleAllocateDeallocate)
	B3D_ADD_TEST(GpuAllocatorTestSuite::TestTlsf_NonOverlappingAlignedOffsets)
	B3D_ADD_TEST(GpuAllocatorTestSuite::TestTlsf_CoalesceAllOrders)
	B3D_ADD_TEST(GpuAllocatorTestSuite::TestTlsf_LargeAlignmentSplitsLeadingPadding)
	B3D_ADD_TEST(GpuAllocatorTestSuite::TestTlsf_HeapGrowthAndEmptyRelease)
	B3D_ADD_TEST(GpuAllocatorTestSuite::TestTlsf_OversizedAllocationGetsDedicatedHeap)
	B3D_ADD_TEST(GpuAllocatorTestSuite::TestTlsf_RandomStressNoLeak)
}

namespace
{
	/**
	 * Returns the first device exposed by the active backend, or nullptr when the backend has
	 * none (e.g. headless CI without a usable GPU). Tests bail out gracefully on nullptr to keep
	 * the suite usable on machines where the GPU plugin couldn't bring a device up.
	 */
	SPtr<GpuDevice> GetActiveDevice()
	{
		GpuBackend& backend = GpuBackend::Instance();
		if (backend.GetDeviceCount() == 0)
			return nullptr;

		return backend.GetDevice(0);
	}

	/**
	 * Returns true when the active backend is a real GPU (Vulkan / Metal / D3D12) rather
	 * than the NullGpuBackend. The null backend's fence is intentionally always-signaled so
	 * deferred-delete drains immediately, which means the "value never reached" / "submit advances
	 * counter" cases below would not exercise meaningful behaviour against it. Skipping keeps the
	 * suite green on null-backend builds (e.g. headless test runs) while still asserting the
	 * contract on real backends.
	 */
	bool IsRealBackend(const GpuDevice& device)
	{
		return device.GetCapabilities().BackendName != "Null";
	}

	/**
	 * In-process implementation of the @c GpuHeapBackend trait used by the allocator unit tests.
	 * Backs each "heap" with a host-side Vector<u8> so that allocator-driven offsets can be
	 * exercised end-to-end without a real device.
	 */
	class MockHeapBackend
	{
	public:
		struct HeapHandle
		{
			u32 Id = 0;
			u64 Size = 0;
		};

		struct HeapCreateInformation
		{
			u64 Alignment = 16;
			bool MapPersistently = true;
		};

		MockHeapBackend() = default;

		HeapHandle CreateHeap(u64 sizeInBytes, const HeapCreateInformation&)
		{
			HeapHandle handle;
			handle.Id = (u32)mHeaps.size();
			handle.Size = sizeInBytes;

			Storage storage;
			storage.Live = true;
			storage.Bytes.resize((size_t)sizeInBytes);
			mHeaps.push_back(std::move(storage));
			mLiveCount++;

			return handle;
		}

		void DestroyHeap(HeapHandle handle)
		{
			B3D_ASSERT(handle.Id < mHeaps.size());

			Storage& storage = mHeaps[handle.Id];
			if (!storage.Live)
				return;

			storage.Live = false;
			storage.Bytes.clear();
			storage.Bytes.shrink_to_fit();
			mDestroyCount++;
			mLiveCount--;
		}

		u32 DestroyCount() const { return mDestroyCount; }
		u32 LiveHeapCount() const { return mLiveCount; }
		u32 CreateCount() const { return (u32)mHeaps.size(); }

	private:
		struct Storage
		{
			Vector<u8> Bytes;
			bool Live = false;
		};

		Vector<Storage> mHeaps;
		u32 mLiveCount = 0;
		u32 mDestroyCount = 0;
	};

	/**
	 * Standalone implementation of IGpuSubmissionTracker for the allocator unit tests. Models the
	 * device-wide submission counter as two monotonic 64-bit values — mLatest (the index assigned
	 * to the most recent simulated submit) and mCompleted (the index the simulated GPU has
	 * caught up to). Lets tests exercise the deferred-delete contract end-to-end without a real
	 * GpuDevice.
	 */
	class MockGpuSubmissionTracker : public IGpuSubmissionTracker
	{
	public:
		u64 GetLatestSubmissionIndex() const override { return mLatest; }
		bool IsSubmissionComplete(u64 index) const override { return index <= mCompleted; }

		/**
		 * Simulates one GPU submit: advances the latest-assigned counter and returns the newly
		 * assigned value. Allocations retired immediately *after* this call observe the new value
		 * via GetLatestSubmissionIndex.
		 */
		u64 Submit()
		{
			mLatest++;
			return mLatest;
		}

		/** Simulates the GPU completing all work up to (and including) @p index. */
		void Signal(u64 index)
		{
			B3D_ASSERT(index >= mCompleted);
			mCompleted = index;
		}

		/** Simulates the GPU catching up to the most recently assigned submission. */
		void SignalAll() { mCompleted = mLatest; }

		u64 LatestSubmissionIndex() const { return mLatest; }
		u64 CompletedSubmissionIndex() const { return mCompleted; }

	private:
		u64 mLatest = 0;
		u64 mCompleted = 0;
	};

	using MockLocation = TGpuResourceLocation<MockHeapBackend>;
	using TlsfAllocator = TGpuTlsfAllocator<MockHeapBackend>;

	/** Helper: simulates a submit, deallocates, signals, flushes — drives a single retire entry to completion. */
	void DeallocateAndDrain(TlsfAllocator& allocator, MockGpuSubmissionTracker& tracker, MockLocation& location)
	{
		tracker.Submit();
		allocator.Deallocate(location);
		tracker.SignalAll();
		allocator.Flush();
	}

	/**
	 * Default TLSF configuration: 1 MB heaps, capped at 4 MB, no warm-spare retention beyond 1.
	 * The tests override individual fields as needed.
	 */
	TlsfAllocator::Configuration MakeDefaultTlsfConfig(u64 initial = 1 * 1024 * 1024, u64 maxHeap = 4 * 1024 * 1024)
	{
		TlsfAllocator::Configuration configuration;
		configuration.InitialHeapSize = initial;
		configuration.MaxHeapSize = maxHeap;
		configuration.GrowthFactor = 2;
		configuration.MaxEmptyHeapCount = 1;
		configuration.MinAllocationSize = 16;
		return configuration;
	}

	/** Identifier the deferred-free queue forwards to a strategy's FreeImmediateImpl. */
	struct FreedSlot
	{
		u32 AllocatorData0;
		u32 AllocatorData1;
	};

	/**
	 * Minimal CRTP-derived allocator used by the contract and deferred-delete tests. Records every
	 * FreeImmediateImpl call in FreedSlots so tests can assert which retired slots were actually drained.
	 * TryAllocateImpl / DeallocateImpl exist only to prove the public surface compiles and links.
	 */
	class MockAllocator : public TGpuAllocator<MockAllocator, MockHeapBackend>
	{
	public:
		using Base = TGpuAllocator<MockAllocator, MockHeapBackend>;

		// Surface the protected strategy hooks so the contract tests can drive them directly without
		// having to round-trip through the public Deallocate path (which auto-resets the location).
		using Base::RetireAllocation;
		using Base::NotifyOwnerMoved;

		MockAllocator(MockHeapBackend* backend, MockGpuSubmissionTracker* tracker)
			: Base(backend, tracker)
		{}

		bool TryAllocateImpl(u64 /*size*/, u32 /*alignment*/, MockLocation& /*out*/)
		{
			return false;
		}

		void DeallocateImpl(MockLocation& allocation)
		{
			RetireAllocation(allocation);
		}

		void FreeImmediateImpl(u32 allocatorData0, u32 allocatorData1)
		{
			FreedSlots.push_back({ allocatorData0, allocatorData1 });
		}

		Vector<FreedSlot> FreedSlots;
	};

	/** Minimal @c IGpuResource implementation used to verify the migration-callback dispatch path. */
	class MockResource : public IGpuResource
	{
	public:
		u32 GetBoundCount(u32 subresourceIdx = 0) const override { (void)subresourceIdx; return 0; }
		u32 GetUseCount(u32 subresourceIdx = 0) const override { (void)subresourceIdx; return 0; }
		void OnAllocationMoved() override { MovedCount++; }

		u32 MovedCount = 0;
	};
}

void GpuAllocatorTestSuite::TestGpuAllocatorContract()
{
	// Compile-time proof: the trait-check macro accepts a valid backend.
	B3D_STATIC_ASSERT_HEAP_BACKEND_IS_VALID(MockHeapBackend);

	// Counter-example for the macro is intentionally left commented out — uncommenting it should produce
	// six focused diagnostics (one per missing requirement) rather than a wall of template-expansion errors:
	//   struct BrokenBackend {};
	//   B3D_STATIC_ASSERT_HEAP_BACKEND_IS_VALID(BrokenBackend);

	// The location must remain a POD so render proxies can copy/move it without ceremony. If a future
	// change introduces a non-trivial member, these asserts catch it before consumer code regresses.
	static_assert(std::is_standard_layout<MockLocation>::value, "TGpuResourceLocation must remain standard-layout.");
	static_assert(std::is_trivially_copyable<MockLocation>::value, "TGpuResourceLocation must remain trivially copyable.");

	MockHeapBackend backend;
	MockGpuSubmissionTracker tracker;
	MockAllocator allocator(&backend, &tracker);

	// Exercise the full public surface so the linker resolves every entry point.
	MockLocation location;
	location.Allocator = &allocator;
	location.Size = 256;
	location.AllocatorData0 = 5;
	location.AllocatorData1 = 7;

	B3D_TEST_ASSERT(location.IsValid())

	// Real-world ordering with the "stamp with latest" pattern: a touching submit advances the device
	// counter first, then the deallocate stamps the retire entry with the current latest value. Mirror
	// that here — submit, then retire.
	const u64 submittedIndex = tracker.Submit();
	allocator.RetireAllocation(location);

	tracker.Signal(submittedIndex);

	allocator.Flush();
	B3D_TEST_ASSERT(allocator.FreedSlots.size() == 1)
	B3D_TEST_ASSERT(allocator.FreedSlots[0].AllocatorData0 == 5)
	B3D_TEST_ASSERT(allocator.FreedSlots[0].AllocatorData1 == 7)

	// IGpuResource dispatch: the allocator should fire OnAllocationMoved exactly once when an owner is
	// registered, and skip the call entirely otherwise.
	MockResource owner;
	location.Owner = &owner;
	allocator.NotifyOwnerMoved(location);
	B3D_TEST_ASSERT(owner.MovedCount == 1)

	location.Owner = nullptr;
	allocator.NotifyOwnerMoved(location);
	B3D_TEST_ASSERT(owner.MovedCount == 1)

	location.Reset();
	B3D_TEST_ASSERT(!location.IsValid())
	B3D_TEST_ASSERT(location.AllocatorData0 == 0)
	B3D_TEST_ASSERT(location.AllocatorData1 == 0)
}

void GpuAllocatorTestSuite::TestGpuAllocatorDeferredDelete()
{
	// Case 1: FIFO drain stops at the first incomplete entry.
	{
		MockHeapBackend backend;
		MockGpuSubmissionTracker tracker;
		MockAllocator allocator(&backend, &tracker);

		// Each location carries a distinct slot identity so the test can match drained entries back to the
		// retire calls without relying on pointer equality.
		MockLocation locationA, locationB, locationC;
		locationA.AllocatorData0 = 1; locationA.AllocatorData1 = 10;
		locationB.AllocatorData0 = 2; locationB.AllocatorData1 = 20;
		locationC.AllocatorData0 = 3; locationC.AllocatorData1 = 30;

		// Real-world ordering with the "stamp with latest" pattern: a touching submit advances the device
		// counter first, then the retire stamps with the new latest value. The three submits assign indices
		// 1, 2, 3 and the retires inherit them.
		const u64 indexA = tracker.Submit(); allocator.RetireAllocation(locationA);
		const u64 indexB = tracker.Submit(); allocator.RetireAllocation(locationB);
		const u64 indexC = tracker.Submit(); allocator.RetireAllocation(locationC);

		B3D_TEST_ASSERT(indexA == 1)
		B3D_TEST_ASSERT(indexB == 2)
		B3D_TEST_ASSERT(indexC == 3)

		// Signal only past the first entry. The drain must release exactly that one and stop.
		tracker.Signal(indexA);
		allocator.Flush();

		B3D_TEST_ASSERT(allocator.FreedSlots.size() == 1)
		B3D_TEST_ASSERT(allocator.FreedSlots[0].AllocatorData0 == 1)
		B3D_TEST_ASSERT(allocator.FreedSlots[0].AllocatorData1 == 10)

		// Case 2: Subsequent advance drains the rest in original order.
		tracker.Signal(indexC);
		allocator.Flush();

		B3D_TEST_ASSERT(allocator.FreedSlots.size() == 3)
		B3D_TEST_ASSERT(allocator.FreedSlots[1].AllocatorData0 == 2)
		B3D_TEST_ASSERT(allocator.FreedSlots[1].AllocatorData1 == 20)
		B3D_TEST_ASSERT(allocator.FreedSlots[2].AllocatorData0 == 3)
		B3D_TEST_ASSERT(allocator.FreedSlots[2].AllocatorData1 == 30)
	}

	// Case 3: Flush(forceComplete=true) drains everything regardless of submission state.
	{
		MockHeapBackend backend;
		MockGpuSubmissionTracker tracker;
		MockAllocator allocator(&backend, &tracker);

		MockLocation locationA, locationB, locationC;
		locationA.AllocatorData0 = 1; locationA.AllocatorData1 = 10;
		locationB.AllocatorData0 = 2; locationB.AllocatorData1 = 20;
		locationC.AllocatorData0 = 3; locationC.AllocatorData1 = 30;

		tracker.Submit(); allocator.RetireAllocation(locationA);
		tracker.Submit(); allocator.RetireAllocation(locationB);
		tracker.Submit(); allocator.RetireAllocation(locationC);

		// No Signal() call — submissions remain incomplete.
		allocator.Flush(3, true);

		B3D_TEST_ASSERT(allocator.FreedSlots.size() == 3)
		B3D_TEST_ASSERT(allocator.FreedSlots[0].AllocatorData0 == 1)
		B3D_TEST_ASSERT(allocator.FreedSlots[1].AllocatorData0 == 2)
		B3D_TEST_ASSERT(allocator.FreedSlots[2].AllocatorData0 == 3)
	}

	// Case 4: Public Deallocate path — captures the slot identity by value, resets the caller's location,
	// and proves the queued snapshot is independent of the caller's storage. This is the property that
	// makes the deferred-delete queue safe against the resource being destroyed before its submission
	// completes.
	{
		MockHeapBackend backend;
		MockGpuSubmissionTracker tracker;
		MockAllocator allocator(&backend, &tracker);

		MockLocation location;
		location.Allocator = &allocator;
		location.AllocatorData0 = 42;
		location.AllocatorData1 = 99;

		const u64 retireIndex = tracker.Submit();
		allocator.Deallocate(location);

		// Auto-Reset on the caller's location: the resource sees an invalid handle the moment Deallocate
		// returns, even though the queue still holds a snapshot of the slot.
		B3D_TEST_ASSERT(!location.IsValid())
		B3D_TEST_ASSERT(location.AllocatorData0 == 0)
		B3D_TEST_ASSERT(location.AllocatorData1 == 0)

		// Mutate the caller's storage post-Deallocate. The retired-queue snapshot must remain unaffected,
		// which is what would let a resource destructor run between Deallocate and the submission signal.
		location.AllocatorData0 = 7;
		location.AllocatorData1 = 8;

		tracker.Signal(retireIndex);
		allocator.Flush();

		B3D_TEST_ASSERT(allocator.FreedSlots.size() == 1)
		B3D_TEST_ASSERT(allocator.FreedSlots[0].AllocatorData0 == 42)
		B3D_TEST_ASSERT(allocator.FreedSlots[0].AllocatorData1 == 99)
	}
}

void GpuAllocatorTestSuite::TestSubmissionFence_InitialState()
{
	SPtr<GpuDevice> device = GetActiveDevice();
	if (device == nullptr)
		return;

	// The latest submission index is "whatever has been assigned so far on this device". Other
	// engine subsystems (renderer warm-up, transfer pool init) may have submitted before this test
	// runs, so the count is non-deterministic — the contract that matters is that any submit at-or-
	// below the current latest is reported complete (after a synchronous wait), and zero is always
	// trivially complete because no submit ever takes that index.
	B3D_TEST_ASSERT(device->IsSubmissionComplete(0))

	const u64 latest = device->GetLatestSubmissionIndex();
	device->WaitUntilIdle();
	B3D_TEST_ASSERT(device->IsSubmissionComplete(latest))
}

void GpuAllocatorTestSuite::TestSubmissionFence_AdvancesAfterSubmit()
{
	SPtr<GpuDevice> device = GetActiveDevice();
	if (device == nullptr || !IsRealBackend(*device))
		return;

	const u64 indexBefore = device->GetLatestSubmissionIndex();

	GpuCommandBufferPoolCreateInformation poolCreateInfo = GpuCommandBufferPoolCreateInformation::CreateForThisThread(GQT_GRAPHICS);
	SPtr<GpuCommandBufferPool> pool = device->CreateGpuCommandBufferPool(poolCreateInfo);
	SPtr<GpuCommandBuffer> commandBuffer = pool->Create(GpuCommandBufferCreateInformation::Create("AdvancesAfterSubmitCB"));

	GpuSubmissionInformation info;
	info.CommandBuffer = commandBuffer;

	const u64 assignedIndex = device->SubmitCommandBuffer(info);
	B3D_TEST_ASSERT(assignedIndex > indexBefore)

	device->WaitUntilIdle();
	B3D_TEST_ASSERT(device->IsSubmissionComplete(assignedIndex))
}

void GpuAllocatorTestSuite::TestUserCreatedFence_ExplicitSignal()
{
	SPtr<GpuDevice> device = GetActiveDevice();
	if (device == nullptr || !IsRealBackend(*device))
		return;

	SPtr<GpuTimelineFence> fence = device->CreateTimelineFence();
	if (fence == nullptr)
		return; // Backend has not yet implemented user-created fences (e.g. D3D12 today).

	// Skip when the fence is in a degraded mode that lacks per-submit visibility (e.g. Vulkan on
	// hardware/drivers without VK_KHR_timeline_semaphore). The explicit-value round-trip is
	// meaningless on those paths because GetCompletedValue cannot observe per-value signals.
	if (fence->IsSignaled(7))
		return;

	B3D_TEST_ASSERT(fence->GetCompletedValue() == 0)
	B3D_TEST_ASSERT(!fence->IsSignaled(7))

	// Allocate a fresh command buffer outside the transfer-helper's machinery so that resubmitting
	// it during a later EndFrame doesn't break — the helper's thread-data slot would otherwise
	// retain a pointer to the CB after this test submits it.
	GpuCommandBufferPoolCreateInformation poolCreateInfo = GpuCommandBufferPoolCreateInformation::CreateForThisThread(GQT_GRAPHICS);
	SPtr<GpuCommandBufferPool> pool = device->CreateGpuCommandBufferPool(poolCreateInfo);
	SPtr<GpuCommandBuffer> commandBuffer = pool->Create(GpuCommandBufferCreateInformation::Create("UserFenceTestCB"));
	// Pool::Create returns a CB already in the Recording state; SubmitCommandBuffer auto-ends.

	GpuSubmissionInformation info;
	info.CommandBuffer = commandBuffer;
	info.SignalFences.Add(GpuTimelineFenceAndValue{ fence, 7 });

	const u64 assignedIndex = device->SubmitCommandBuffer(info);
	(void)assignedIndex;

	// Drain pending GPU work. After WaitUntilIdle the GPU has retired the (effectively empty)
	// submit, so the explicit value-7 signal must be observable via IsSignaled.
	device->WaitUntilIdle();
	B3D_TEST_ASSERT(fence->IsSignaled(7))
}

void GpuAllocatorTestSuite::TestTlsf_ContractAndInitialState()
{
	// Compile-time proof: the trait-check macro accepts the mock backend.
	B3D_STATIC_ASSERT_HEAP_BACKEND_IS_VALID(MockHeapBackend);

	MockHeapBackend backend;
	MockGpuSubmissionTracker tracker;

	// No heap creation on construction — heaps are created lazily on first allocation.
	TlsfAllocator allocator(&backend, &tracker, MakeDefaultTlsfConfig());
	B3D_TEST_ASSERT(allocator.GetHeapCount() == 0)
	B3D_TEST_ASSERT(allocator.GetCommittedBytes() == 0)
	B3D_TEST_ASSERT(allocator.GetUsedBytes() == 0)
	B3D_TEST_ASSERT(backend.LiveHeapCount() == 0)

	// First TryAllocate creates the initial heap.
	MockLocation location;
	const bool ok = allocator.TryAllocate(1024, 16, location);
	B3D_TEST_ASSERT(ok)
	B3D_TEST_ASSERT(allocator.GetHeapCount() == 1)
	B3D_TEST_ASSERT(allocator.GetCommittedBytes() == 1 * 1024 * 1024)
	B3D_TEST_ASSERT(backend.LiveHeapCount() == 1)
	B3D_TEST_ASSERT(location.IsValid())
	B3D_TEST_ASSERT(location.Size >= 1024)
	B3D_TEST_ASSERT((location.Offset & 15) == 0)

	DeallocateAndDrain(allocator, tracker, location);
}

void GpuAllocatorTestSuite::TestTlsf_SingleAllocateDeallocate()
{
	MockHeapBackend backend;
	MockGpuSubmissionTracker tracker;
	TlsfAllocator allocator(&backend, &tracker, MakeDefaultTlsfConfig());

	MockLocation location;
	B3D_TEST_ASSERT(allocator.TryAllocate(2048, 64, location))
	B3D_TEST_ASSERT(location.Allocator == &allocator)
	B3D_TEST_ASSERT((location.Offset & 63) == 0)

	const u64 usedAfterAlloc = allocator.GetUsedBytes();
	B3D_TEST_ASSERT(usedAfterAlloc >= 2048)

	// Deferred drain: Deallocate stamps the retire entry but doesn't actually return memory until
	// the submission has been signaled and Flush() runs.
	const u64 retireSubmission = tracker.Submit();
	allocator.Deallocate(location);
	B3D_TEST_ASSERT(!location.IsValid())
	B3D_TEST_ASSERT(allocator.GetUsedBytes() == usedAfterAlloc) // Still accounted for — fence pending.

	tracker.Signal(retireSubmission);
	allocator.Flush();
	B3D_TEST_ASSERT(allocator.GetUsedBytes() == 0)
}

void GpuAllocatorTestSuite::TestTlsf_NonOverlappingAlignedOffsets()
{
	MockHeapBackend backend;
	MockGpuSubmissionTracker tracker;
	TlsfAllocator allocator(&backend, &tracker, MakeDefaultTlsfConfig());

	struct Alloc { MockLocation Location; u64 Begin; u64 End; };
	Vector<Alloc> allocs;

	const u32 allocCount = 64;
	const u64 allocSize = 4096;
	const u32 alignment = 256;

	for (u32 allocIndex = 0; allocIndex < allocCount; allocIndex++)
	{
		Alloc record;
		B3D_TEST_ASSERT(allocator.TryAllocate(allocSize, alignment, record.Location))
		B3D_TEST_ASSERT((record.Location.Offset & (alignment - 1)) == 0)
		record.Begin = record.Location.Offset;
		record.End = record.Location.Offset + record.Location.Size;
		allocs.push_back(record);
	}

	// Verify all (Heap, Begin, End) ranges are non-overlapping.
	for (u32 outerIndex = 0; outerIndex < allocs.size(); outerIndex++)
	{
		for (u32 innerIndex = outerIndex + 1; innerIndex < allocs.size(); innerIndex++)
		{
			if (allocs[outerIndex].Location.Heap.Id != allocs[innerIndex].Location.Heap.Id)
				continue;
			const bool disjoint = allocs[outerIndex].End <= allocs[innerIndex].Begin
				|| allocs[innerIndex].End <= allocs[outerIndex].Begin;
			B3D_TEST_ASSERT(disjoint)
		}
	}

	// Drain everything.
	tracker.Submit();
	for (Alloc& record : allocs)
		allocator.Deallocate(record.Location);
	tracker.SignalAll();
	allocator.Flush();
	B3D_TEST_ASSERT(allocator.GetUsedBytes() == 0)
}

void GpuAllocatorTestSuite::TestTlsf_CoalesceAllOrders()
{
	// Allocate three sequential blocks; free in different permutations and assert each ends up
	// fully coalesced. The "fully coalesced" assertion is indirect: after each pass we re-allocate
	// the entire heap as one block, which can only succeed if coalescing actually merged everything.
	const u32 forwardOrder[3] = { 0, 1, 2 };
	const u32 reverseOrder[3] = { 2, 1, 0 };
	const u32 middleFirstOrder[3] = { 1, 0, 2 };
	const u32 middleLastOrder[3] = { 0, 2, 1 };

	const u32* patterns[4] = { forwardOrder, reverseOrder, middleFirstOrder, middleLastOrder };

	for (u32 patternIndex = 0; patternIndex < 4; patternIndex++)
	{
		const u32* freeOrder = patterns[patternIndex];

		MockHeapBackend backend;
		MockGpuSubmissionTracker tracker;

		TlsfAllocator::Configuration configuration = MakeDefaultTlsfConfig();
		configuration.InitialHeapSize = 64 * 1024;
		configuration.MaxHeapSize = 64 * 1024;
		TlsfAllocator allocator(&backend, &tracker, configuration);

		const u64 blockSize = 16 * 1024;
		MockLocation locations[3];
		for (u32 blockIndex = 0; blockIndex < 3; blockIndex++)
			B3D_TEST_ASSERT(allocator.TryAllocate(blockSize, 16, locations[blockIndex]))

		// All three live in the same heap (heap is 64KB, allocations total 48KB).
		B3D_TEST_ASSERT(locations[0].Heap.Id == locations[1].Heap.Id)
		B3D_TEST_ASSERT(locations[0].Heap.Id == locations[2].Heap.Id)

		tracker.Submit();
		for (u32 freeIndex = 0; freeIndex < 3; freeIndex++)
			allocator.Deallocate(locations[freeOrder[freeIndex]]);
		tracker.SignalAll();
		allocator.Flush();
		B3D_TEST_ASSERT(allocator.GetUsedBytes() == 0)

		// Coalescing proof: a single allocation equal to the entire usable heap must succeed. If any
		// of the original three blocks didn't merge with neighbors, a 48 KB allocation would fragment
		// across two free ranges and fail.
		MockLocation reuse;
		B3D_TEST_ASSERT(allocator.TryAllocate(blockSize * 3, 16, reuse))
		B3D_TEST_ASSERT(allocator.GetHeapCount() == 1)
		DeallocateAndDrain(allocator, tracker, reuse);
	}
}

void GpuAllocatorTestSuite::TestTlsf_LargeAlignmentSplitsLeadingPadding()
{
	MockHeapBackend backend;
	MockGpuSubmissionTracker tracker;

	TlsfAllocator::Configuration configuration = MakeDefaultTlsfConfig();
	configuration.InitialHeapSize = 1 * 1024 * 1024;
	configuration.MaxHeapSize = 1 * 1024 * 1024;
	TlsfAllocator allocator(&backend, &tracker, configuration);

	// Pin the start of the heap with a small allocation so the next allocation's natural offset is
	// non-zero — this forces leading-padding handling when alignment is large.
	MockLocation pin;
	B3D_TEST_ASSERT(allocator.TryAllocate(64, 16, pin))

	MockLocation aligned;
	B3D_TEST_ASSERT(allocator.TryAllocate(8192, 4096, aligned))
	B3D_TEST_ASSERT((aligned.Offset & 4095) == 0)
	B3D_TEST_ASSERT(aligned.Offset >= pin.Offset + pin.Size)

	// Free in reverse order — exercises both the leading-padding-was-folded path and the natural
	// coalesce-on-free.
	tracker.Submit();
	allocator.Deallocate(aligned);
	allocator.Deallocate(pin);
	tracker.SignalAll();
	allocator.Flush();
	B3D_TEST_ASSERT(allocator.GetUsedBytes() == 0)

	// After draining, the allocator should once again be able to fit a single allocation that
	// occupies the entire usable heap.
	MockLocation full;
	B3D_TEST_ASSERT(allocator.TryAllocate(configuration.InitialHeapSize - 1024, 16, full))
	B3D_TEST_ASSERT(allocator.GetHeapCount() == 1)
	DeallocateAndDrain(allocator, tracker, full);
}

void GpuAllocatorTestSuite::TestTlsf_HeapGrowthAndEmptyRelease()
{
	MockHeapBackend backend;
	MockGpuSubmissionTracker tracker;

	TlsfAllocator::Configuration configuration = MakeDefaultTlsfConfig();
	configuration.InitialHeapSize = 64 * 1024;
	configuration.MaxHeapSize = 64 * 1024;
	configuration.GrowthFactor = 1; // Subsequent heaps stay the same size for predictability.
	configuration.MaxEmptyHeapCount = 1;
	TlsfAllocator allocator(&backend, &tracker, configuration);

	// Each allocation is 32 KB; the 64 KB heap fits two before grow.
	const u64 allocSize = 32 * 1024;
	Vector<MockLocation> allocs;

	for (u32 allocIndex = 0; allocIndex < 6; allocIndex++)
	{
		MockLocation location;
		B3D_TEST_ASSERT(allocator.TryAllocate(allocSize, 16, location))
		allocs.push_back(location);
	}

	// 6 allocations / 2-per-heap = 3 heaps minimum. Single-allocation heaps may have been used too.
	const u32 heapsAfterFill = allocator.GetHeapCount();
	B3D_TEST_ASSERT(heapsAfterFill >= 3)
	B3D_TEST_ASSERT(backend.CreateCount() == heapsAfterFill)

	// Free everything.
	tracker.Submit();
	for (MockLocation& location : allocs)
		allocator.Deallocate(location);
	tracker.SignalAll();
	allocator.Flush();

	// MaxEmptyHeapCount = 1, so all but one heap must be returned to the backend.
	B3D_TEST_ASSERT(allocator.GetHeapCount() == 1)
	B3D_TEST_ASSERT(backend.LiveHeapCount() == 1)
	B3D_TEST_ASSERT(backend.DestroyCount() == heapsAfterFill - 1)
}

void GpuAllocatorTestSuite::TestTlsf_OversizedAllocationGetsDedicatedHeap()
{
	MockHeapBackend backend;
	MockGpuSubmissionTracker tracker;

	TlsfAllocator::Configuration configuration = MakeDefaultTlsfConfig();
	configuration.InitialHeapSize = 64 * 1024;
	configuration.MaxHeapSize = 64 * 1024;
	TlsfAllocator allocator(&backend, &tracker, configuration);

	// Request a 128 KB block — twice the configured max heap size. The allocator must create a
	// dedicated heap of at least the requested size (rather than failing) so that single-resource
	// allocations larger than the typical heap budget still succeed.
	MockLocation oversized;
	B3D_TEST_ASSERT(allocator.TryAllocate(128 * 1024, 16, oversized))
	B3D_TEST_ASSERT(oversized.Size >= 128 * 1024)
	B3D_TEST_ASSERT(allocator.GetCommittedBytes() >= 128 * 1024)

	DeallocateAndDrain(allocator, tracker, oversized);
}

void GpuAllocatorTestSuite::TestTlsf_RandomStressNoLeak()
{
	MockHeapBackend backend;
	MockGpuSubmissionTracker tracker;

	TlsfAllocator::Configuration configuration = MakeDefaultTlsfConfig();
	configuration.InitialHeapSize = 4 * 1024 * 1024;
	configuration.MaxHeapSize = 16 * 1024 * 1024;
	TlsfAllocator allocator(&backend, &tracker, configuration);

	std::mt19937 rng(0xB3D7B5Fu);
	std::uniform_int_distribution<u32> sizeDistribution(16, 64 * 1024);
	std::uniform_int_distribution<u32> alignmentBitDistribution(4, 12); // 16..4096
	std::uniform_int_distribution<u32> opDistribution(0, 100);

	struct Live { MockLocation Location; u64 Begin; u64 End; };
	Vector<Live> live;
	const u32 iterationCount = 4000;

	for (u32 iteration = 0; iteration < iterationCount; iteration++)
	{
		const bool wantAlloc = live.empty() || opDistribution(rng) < 60;
		if (wantAlloc)
		{
			Live record;
			const u64 size = sizeDistribution(rng);
			const u32 alignment = 1u << alignmentBitDistribution(rng);
			if (allocator.TryAllocate(size, alignment, record.Location))
			{
				B3D_TEST_ASSERT((record.Location.Offset & (alignment - 1)) == 0)
				record.Begin = record.Location.Offset;
				record.End = record.Location.Offset + record.Location.Size;

				// Verify non-overlap against every live entry on the same heap. The O(N) cost is
				// negligible at the 4000-iteration / sub-1KB-live workload of this stress test.
				for (const Live& other : live)
				{
					if (other.Location.Heap.Id != record.Location.Heap.Id)
						continue;
					const bool disjoint = record.End <= other.Begin || other.End <= record.Begin;
					B3D_TEST_ASSERT(disjoint)
				}
				live.push_back(record);
			}
		}
		else
		{
			std::uniform_int_distribution<u32> indexDistribution(0, (u32)live.size() - 1);
			const u32 victimIndex = indexDistribution(rng);
			tracker.Submit();
			allocator.Deallocate(live[victimIndex].Location);
			live[victimIndex] = live.back();
			live.pop_back();
		}

		// Periodically advance the submission counter so retire queue drains.
		if ((iteration % 32) == 0)
		{
			tracker.SignalAll();
			allocator.Flush();
		}
	}

	// Drain the rest.
	tracker.Submit();
	for (Live& record : live)
		allocator.Deallocate(record.Location);
	tracker.SignalAll();
	allocator.Flush();

	B3D_TEST_ASSERT(allocator.GetUsedBytes() == 0)
}
