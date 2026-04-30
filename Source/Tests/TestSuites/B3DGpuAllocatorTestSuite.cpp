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
#include "GpuBackend/Allocators/B3DGpuResource.h"

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

		u64 LatestSubmissionIndex() const { return mLatest; }
		u64 CompletedSubmissionIndex() const { return mCompleted; }

	private:
		u64 mLatest = 0;
		u64 mCompleted = 0;
	};

	using MockLocation = TGpuResourceLocation<MockHeapBackend>;

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
