//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DGpuAllocatorTestSuite.h"
#include "CoreObject/B3DRenderThread.h"
#include "GpuBackend/B3DGpuBackend.h"
#include "GpuBackend/B3DGpuDevice.h"
#include "GpuBackend/B3DGpuDeviceCapabilities.h"
#include "GpuBackend/B3DGpuTimelineFence.h"
#include "GpuBackend/B3DGpuCommandBuffer.h"
#include "GpuBackend/B3DGpuCommandBufferPoolRing.h"
#include "GpuBackend/Allocators/B3DGpuAllocator.h"
#include "GpuBackend/Allocators/B3DGpuTlsfAllocator.h"
#include "GpuBackend/Allocators/B3DGpuResource.h"

#include <atomic>
#include <chrono>
#include <random>
#include <thread>

using namespace b3d;
using namespace b3d::render;

GpuAllocatorTestSuite::GpuAllocatorTestSuite()
	: TestSuite("GpuAllocatorTestSuite")
{
	B3D_ADD_TEST(GpuAllocatorTestSuite::TestGpuAllocatorContract)
	B3D_ADD_TEST(GpuAllocatorTestSuite::TestGpuAllocatorDeferredDelete)
	B3D_ADD_TEST(GpuAllocatorTestSuite::TestFrameTracker_InitialState)
	B3D_ADD_TEST(GpuAllocatorTestSuite::TestFrameTracker_AdvancesOnEndFrame)
	B3D_ADD_TEST(GpuAllocatorTestSuite::TestUserCreatedFence_ExplicitSignal)
	B3D_ADD_TEST(GpuAllocatorTestSuite::TestTlsf_ContractAndInitialState)
	B3D_ADD_TEST(GpuAllocatorTestSuite::TestTlsf_SingleAllocateFree)
	B3D_ADD_TEST(GpuAllocatorTestSuite::TestTlsf_NonOverlappingAlignedOffsets)
	B3D_ADD_TEST(GpuAllocatorTestSuite::TestTlsf_CoalesceAllOrders)
	B3D_ADD_TEST(GpuAllocatorTestSuite::TestTlsf_LargeAlignmentSplitsLeadingPadding)
	B3D_ADD_TEST(GpuAllocatorTestSuite::TestTlsf_HeapGrowthAndEmptyRelease)
	B3D_ADD_TEST(GpuAllocatorTestSuite::TestTlsf_OversizedAllocationGetsDedicatedHeap)
	B3D_ADD_TEST(GpuAllocatorTestSuite::TestTlsf_RandomStressNoLeak)
	B3D_ADD_TEST(GpuAllocatorTestSuite::TestTlsf_GranularityDisabled)
	B3D_ADD_TEST(GpuAllocatorTestSuite::TestTlsf_GranularityHomogeneousNoPadding)
	B3D_ADD_TEST(GpuAllocatorTestSuite::TestTlsf_GranularityLinearVsNonLinearInflatesPadding)
	B3D_ADD_TEST(GpuAllocatorTestSuite::TestTlsf_GranularityRejectAndRetryAcrossHeaps)
	B3D_ADD_TEST(GpuAllocatorTestSuite::TestTlsf_GranularityFreeReleasesRegion)
	B3D_ADD_TEST(GpuAllocatorTestSuite::TestTlsf_Defrag_DrainsHighestHeap)
	B3D_ADD_TEST(GpuAllocatorTestSuite::TestTlsf_Defrag_SingleHeapWithinHeapCompaction)
	B3D_ADD_TEST(GpuAllocatorTestSuite::TestTlsf_Defrag_RespectsBudget)
	B3D_ADD_TEST(GpuAllocatorTestSuite::TestTlsf_Defrag_OnlySkipsUntrackedSlots)
	B3D_ADD_TEST(GpuAllocatorTestSuite::TestTlsf_Defrag_MovesInFlightResource)
	B3D_ADD_TEST(GpuAllocatorTestSuite::TestTlsf_Defrag_MoveAllocationReceivesContext)
	B3D_ADD_TEST(GpuAllocatorTestSuite::TestTlsf_ResourceLifecyclePolicy_FreesImmediately)
	B3D_ADD_TEST(GpuAllocatorTestSuite::TestTlsf_FrameTrackerPolicy_DefersAcrossFrames)
	B3D_ADD_TEST(GpuAllocatorTestSuite::TestTlsf_Defrag_LifecycleAllowsSwap)
	B3D_ADD_TEST(GpuAllocatorTestSuite::TestTlsf_ConcurrentAllocateAndFree)
	B3D_ADD_TEST(GpuAllocatorTestSuite::TestTlsf_ConcurrentDefragWithAllocateAndFree)
	B3D_ADD_TEST(GpuAllocatorTestSuite::TestTlsf_ThreadUnsafePolicyOptOut)
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
	 * In-process implementation of the GpuHeapBackend trait used by the allocator unit tests.
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
	 * Standalone implementation of IGpuFrameTracker for the allocator unit tests. Models the
	 * device-wide frame counter as two monotonic 64-bit values — mCurrent (the frame currently being
	 * recorded) and mCompleted (the highest frame the simulated GPU has fully drained). Lets tests
	 * exercise the deferred-delete contract end-to-end without a real GpuDevice.
	 */
	class MockGpuFrameTracker : public IGpuFrameTracker
	{
	public:
		u64 GetCurrentFrameIndex() const override { return mCurrent; }
		bool IsFrameComplete(u64 index) const override { return mAnyComplete && index <= mCompleted; }

		/**
		 * Simulates the device advancing to the next frame: bumps the current frame counter by one
		 * and returns the new value. Allocations retired immediately after this call observe the new
		 * frame index via GetCurrentFrameIndex.
		 */
		u64 AdvanceFrame()
		{
			mCurrent++;
			return mCurrent;
		}

		/** Simulates the GPU completing all work up to (and including) frame @p index. */
		void MarkFrameComplete(u64 index)
		{
			B3D_ASSERT(!mAnyComplete || index >= mCompleted);
			mCompleted = index;
			mAnyComplete = true;
		}

		/** Simulates the GPU catching up to the most recently advanced frame. */
		void MarkAllFramesComplete()
		{
			mCompleted = mCurrent;
			mAnyComplete = true;
		}

		u64 CurrentFrameIndex() const { return mCurrent; }
		u64 CompletedFrameIndex() const { return mCompleted; }

	private:
		u64 mCurrent = 0;
		u64 mCompleted = 0;
		bool mAnyComplete = false;
	};

	using MockLocation = TGpuResourceLocation<MockHeapBackend>;
	using TlsfAllocator = TGpuTlsfAllocator<MockHeapBackend>;

	/**
	 * Helper: advances the frame, frees, marks complete, flushes — drives a single retire entry
	 * to completion. Only meaningful when the allocator is configured with @c FrameTracker.
	 */
	void FreeAndDrain(TlsfAllocator& allocator, MockGpuFrameTracker& tracker, MockLocation& location)
	{
		tracker.AdvanceFrame();
		allocator.Free(location);
		tracker.MarkAllFramesComplete();
		allocator.Flush();
	}

	/**
	 * Default TLSF configuration: 1 MB heaps, capped at 4 MB, no warm-spare retention beyond 1.
	 * The tests override individual fields as needed. @c DeferralMode defaults to @c FrameTracker so
	 * existing tests that drive @c Free → @c AdvanceFrame → @c MarkFrameComplete → @c Flush keep
	 * exercising the queued path.
	 */
	TlsfAllocator::Configuration MakeDefaultTlsfConfig(u64 initial = 1 * 1024 * 1024, u64 maxHeap = 4 * 1024 * 1024)
	{
		TlsfAllocator::Configuration configuration;
		configuration.InitialHeapSize = initial;
		configuration.MaxHeapSize = maxHeap;
		configuration.GrowthFactor = 2;
		configuration.MaxEmptyHeapCount = 1;
		configuration.MinAllocationSize = 16;
		configuration.DeferralMode = GpuAllocatorFreeDeferralMode::FrameTracker;
		return configuration;
	}

	/** TLSF configuration with BIG enabled — disables the "small granularity" early-out so the tracker is always live. */
	TlsfAllocator::Configuration MakeTlsfConfigWithGranularity(u64 granularity)
	{
		TlsfAllocator::Configuration configuration = MakeDefaultTlsfConfig();
		configuration.BufferImageGranularity = granularity;
		configuration.GranularityDisableThreshold = 0;
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
	 * TryAllocateImpl / FreeImpl exist only to prove the public surface compiles and links.
	 */
	class MockAllocator : public TGpuAllocator<MockAllocator, MockHeapBackend>
	{
	public:
		using Base = TGpuAllocator<MockAllocator, MockHeapBackend>;

		// Surface the protected retire hook so the contract test can drive it directly without
		// having to round-trip through the public Free path (which auto-resets the location).
		using Base::RetireAllocation;

		MockAllocator(MockHeapBackend* backend, MockGpuFrameTracker* tracker)
			: Base(backend, tracker)
		{}

		bool TryAllocateImpl(u64 /*size*/, u32 /*alignment*/, GpuResourceKind /*kind*/, IGpuResource* /*owner*/, MockLocation& /*out*/)
		{
			return false;
		}

		void FreeImpl(MockLocation& allocation)
		{
			RetireAllocation(allocation);
		}

		void FreeImmediateImpl(u32 allocatorData0, u32 allocatorData1)
		{
			FreedSlots.push_back({ allocatorData0, allocatorData1 });
		}

		Vector<FreedSlot> FreedSlots;
	};

	/**
	 * Minimal IGpuResource implementation used to verify the migration-callback dispatch path.
	 * Records the source offset captured at entry plus the new Location supplied by the
	 * allocator, then assigns it onto the externally-held MockLocation pointed to by LocationPtr
	 * to model the consumer-owned location-replacement contract.
	 */
	class MockResource : public IGpuResource
	{
	public:
		u32 GetBoundCount() const override { return BoundCount; }
		u32 GetUseCount() const override { return UseCount; }
		IGpuResource* MoveAllocation(render::GpuCommandBuffer& /*cb*/, const GpuResourceLocation& newLocation) override
		{
			MovedCount++;

			// Capture the source range from the still-intact location before we overwrite it. This
			// mirrors what production consumers do: they read the source heap / offset / size off
			// their own location to record the GPU copy.
			if (LocationPtr != nullptr)
				LastSourceOffset = LocationPtr->Offset;

			const auto& typedNewLocation = static_cast<const MockLocation&>(newLocation);
			LastNewLocation = typedNewLocation;

			// Mirror the production consumer contract: the consumer is the sole writer of its own
			// location and replaces it wholesale with the supplied newLocation.
			if (LocationPtr != nullptr)
				*LocationPtr = typedNewLocation;

			// Stable identity — this mock keeps the same IGpuResource across moves.
			return this;
		}

		u32 MovedCount = 0;
		u32 UseCount = 0;
		u32 BoundCount = 0;
		u64 LastSourceOffset = 0;
		MockLocation LastNewLocation{};
		MockLocation* LocationPtr = nullptr;
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
	MockGpuFrameTracker tracker;
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
	const u64 submittedIndex = tracker.AdvanceFrame();
	allocator.RetireAllocation(location);

	tracker.MarkFrameComplete(submittedIndex);

	allocator.Flush();
	B3D_TEST_ASSERT(allocator.FreedSlots.size() == 1)
	B3D_TEST_ASSERT(allocator.FreedSlots[0].AllocatorData0 == 5)
	B3D_TEST_ASSERT(allocator.FreedSlots[0].AllocatorData1 == 7)

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
		MockGpuFrameTracker tracker;
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
		const u64 indexA = tracker.AdvanceFrame(); allocator.RetireAllocation(locationA);
		const u64 indexB = tracker.AdvanceFrame(); allocator.RetireAllocation(locationB);
		const u64 indexC = tracker.AdvanceFrame(); allocator.RetireAllocation(locationC);

		B3D_TEST_ASSERT(indexA == 1)
		B3D_TEST_ASSERT(indexB == 2)
		B3D_TEST_ASSERT(indexC == 3)

		// Signal only past the first entry. The drain must release exactly that one and stop.
		tracker.MarkFrameComplete(indexA);
		allocator.Flush();

		B3D_TEST_ASSERT(allocator.FreedSlots.size() == 1)
		B3D_TEST_ASSERT(allocator.FreedSlots[0].AllocatorData0 == 1)
		B3D_TEST_ASSERT(allocator.FreedSlots[0].AllocatorData1 == 10)

		// Case 2: Subsequent advance drains the rest in original order.
		tracker.MarkFrameComplete(indexC);
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
		MockGpuFrameTracker tracker;
		MockAllocator allocator(&backend, &tracker);

		MockLocation locationA, locationB, locationC;
		locationA.AllocatorData0 = 1; locationA.AllocatorData1 = 10;
		locationB.AllocatorData0 = 2; locationB.AllocatorData1 = 20;
		locationC.AllocatorData0 = 3; locationC.AllocatorData1 = 30;

		tracker.AdvanceFrame(); allocator.RetireAllocation(locationA);
		tracker.AdvanceFrame(); allocator.RetireAllocation(locationB);
		tracker.AdvanceFrame(); allocator.RetireAllocation(locationC);

		// No Signal() call — submissions remain incomplete.
		allocator.Flush(3, true);

		B3D_TEST_ASSERT(allocator.FreedSlots.size() == 3)
		B3D_TEST_ASSERT(allocator.FreedSlots[0].AllocatorData0 == 1)
		B3D_TEST_ASSERT(allocator.FreedSlots[1].AllocatorData0 == 2)
		B3D_TEST_ASSERT(allocator.FreedSlots[2].AllocatorData0 == 3)
	}

	// Case 4: Public Free path — captures the slot identity by value, resets the caller's location,
	// and proves the queued snapshot is independent of the caller's storage. This is the property that
	// makes the deferred-delete queue safe against the resource being destroyed before its submission
	// completes.
	{
		MockHeapBackend backend;
		MockGpuFrameTracker tracker;
		MockAllocator allocator(&backend, &tracker);

		MockLocation location;
		location.Allocator = &allocator;
		location.AllocatorData0 = 42;
		location.AllocatorData1 = 99;

		const u64 retireIndex = tracker.AdvanceFrame();
		allocator.Free(location);

		// Auto-Reset on the caller's location: the resource sees an invalid handle the moment Free
		// returns, even though the queue still holds a snapshot of the slot.
		B3D_TEST_ASSERT(!location.IsValid())
		B3D_TEST_ASSERT(location.AllocatorData0 == 0)
		B3D_TEST_ASSERT(location.AllocatorData1 == 0)

		// Mutate the caller's storage post-Free. The retired-queue snapshot must remain unaffected,
		// which is what would let a resource destructor run between Free and the submission signal.
		location.AllocatorData0 = 7;
		location.AllocatorData1 = 8;

		tracker.MarkFrameComplete(retireIndex);
		allocator.Flush();

		B3D_TEST_ASSERT(allocator.FreedSlots.size() == 1)
		B3D_TEST_ASSERT(allocator.FreedSlots[0].AllocatorData0 == 42)
		B3D_TEST_ASSERT(allocator.FreedSlots[0].AllocatorData1 == 99)
	}
}

void GpuAllocatorTestSuite::TestFrameTracker_InitialState()
{
	SPtr<GpuDevice> device = GetActiveDevice();
	if (device == nullptr)
		return;

	// The current frame index is "whatever has been advanced so far on this device". Other engine
	// subsystems (renderer warm-up, asset import bring-up) may have ticked it before this test runs,
	// so the value is non-deterministic — what matters is that the predicate is monotone: a frame
	// reported complete now stays complete, and the current frame is never reported complete (it
	// is by definition still being recorded).
	const u64 frame = device->GetCurrentFrameIndex();
	B3D_TEST_ASSERT(!device->IsFrameComplete(frame))

	if (frame >= RenderThread::kMaximumFramesInFlight)
		B3D_TEST_ASSERT(device->IsFrameComplete(frame - RenderThread::kMaximumFramesInFlight))
}

void GpuAllocatorTestSuite::TestFrameTracker_AdvancesOnEndFrame()
{
	SPtr<GpuDevice> device = GetActiveDevice();
	if (device == nullptr || !IsRealBackend(*device))
		return;

	const u64 indexBefore = device->GetCurrentFrameIndex();

	// BeginFrame / EndFrame have a render-thread-only contract — backends advance render-thread-owned
	// command-buffer pool rings without internal synchronization. Drive them from the render thread.
	const auto fnTickFrame = [device = device.get()]()
	{
		device->BeginFrame();
		device->EndFrame();
	};

	GetRenderThread().PostCommand(fnTickFrame, "TestFrameTracker_AdvancesOnEndFrame::Tick", true);

	const u64 indexAfter = device->GetCurrentFrameIndex();
	B3D_TEST_ASSERT(indexAfter == indexBefore + 1)

	// After kMaximumFramesInFlight further EndFrame ticks, the original frame must be reported complete.
	for (u32 i = 0; i < RenderThread::kMaximumFramesInFlight; i++)
		GetRenderThread().PostCommand(fnTickFrame, "TestFrameTracker_AdvancesOnEndFrame::Tick", true);

	device->WaitUntilIdle();
	B3D_TEST_ASSERT(device->IsFrameComplete(indexBefore))
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

	device->SubmitCommandBuffer(info);

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
	MockGpuFrameTracker tracker;

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

	FreeAndDrain(allocator, tracker,location);
}

void GpuAllocatorTestSuite::TestTlsf_SingleAllocateFree()
{
	MockHeapBackend backend;
	MockGpuFrameTracker tracker;
	TlsfAllocator allocator(&backend, &tracker, MakeDefaultTlsfConfig());

	MockLocation location;
	B3D_TEST_ASSERT(allocator.TryAllocate(2048, 64, location))
	B3D_TEST_ASSERT(location.Allocator == &allocator)
	B3D_TEST_ASSERT((location.Offset & 63) == 0)

	const u64 usedAfterAlloc = allocator.GetUsedBytes();
	B3D_TEST_ASSERT(usedAfterAlloc >= 2048)

	// Deferred drain: Free stamps the retire entry but doesn't actually return memory until
	// the submission has been signaled and Flush() runs.
	const u64 retireSubmission = tracker.AdvanceFrame();
	allocator.Free(location);
	B3D_TEST_ASSERT(!location.IsValid())
	B3D_TEST_ASSERT(allocator.GetUsedBytes() == usedAfterAlloc) // Still accounted for — fence pending.

	tracker.MarkFrameComplete(retireSubmission);
	allocator.Flush();
	B3D_TEST_ASSERT(allocator.GetUsedBytes() == 0)
}

void GpuAllocatorTestSuite::TestTlsf_NonOverlappingAlignedOffsets()
{
	MockHeapBackend backend;
	MockGpuFrameTracker tracker;
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
	tracker.AdvanceFrame();
	for (Alloc& record : allocs)
		allocator.Free(record.Location);
	tracker.MarkAllFramesComplete();
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
		MockGpuFrameTracker tracker;

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

		tracker.AdvanceFrame();
		for (u32 freeIndex = 0; freeIndex < 3; freeIndex++)
			allocator.Free(locations[freeOrder[freeIndex]]);
		tracker.MarkAllFramesComplete();
		allocator.Flush();
		B3D_TEST_ASSERT(allocator.GetUsedBytes() == 0)

		// Coalescing proof: a single allocation equal to the entire usable heap must succeed. If any
		// of the original three blocks didn't merge with neighbors, a 48 KB allocation would fragment
		// across two free ranges and fail.
		MockLocation reuse;
		B3D_TEST_ASSERT(allocator.TryAllocate(blockSize * 3, 16, reuse))
		B3D_TEST_ASSERT(allocator.GetHeapCount() == 1)
		FreeAndDrain(allocator, tracker,reuse);
	}
}

void GpuAllocatorTestSuite::TestTlsf_LargeAlignmentSplitsLeadingPadding()
{
	MockHeapBackend backend;
	MockGpuFrameTracker tracker;

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
	tracker.AdvanceFrame();
	allocator.Free(aligned);
	allocator.Free(pin);
	tracker.MarkAllFramesComplete();
	allocator.Flush();
	B3D_TEST_ASSERT(allocator.GetUsedBytes() == 0)

	// After draining, the allocator should once again be able to fit a single allocation that
	// occupies the entire usable heap.
	MockLocation full;
	B3D_TEST_ASSERT(allocator.TryAllocate(configuration.InitialHeapSize - 1024, 16, full))
	B3D_TEST_ASSERT(allocator.GetHeapCount() == 1)
	FreeAndDrain(allocator, tracker,full);
}

void GpuAllocatorTestSuite::TestTlsf_HeapGrowthAndEmptyRelease()
{
	MockHeapBackend backend;
	MockGpuFrameTracker tracker;

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
	tracker.AdvanceFrame();
	for (MockLocation& location : allocs)
		allocator.Free(location);
	tracker.MarkAllFramesComplete();
	allocator.Flush();

	// MaxEmptyHeapCount = 1, so all but one heap must be returned to the backend.
	B3D_TEST_ASSERT(allocator.GetHeapCount() == 1)
	B3D_TEST_ASSERT(backend.LiveHeapCount() == 1)
	B3D_TEST_ASSERT(backend.DestroyCount() == heapsAfterFill - 1)
}

void GpuAllocatorTestSuite::TestTlsf_OversizedAllocationGetsDedicatedHeap()
{
	MockHeapBackend backend;
	MockGpuFrameTracker tracker;

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

	FreeAndDrain(allocator, tracker,oversized);
}

void GpuAllocatorTestSuite::TestTlsf_RandomStressNoLeak()
{
	MockHeapBackend backend;
	MockGpuFrameTracker tracker;

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
			tracker.AdvanceFrame();
			allocator.Free(live[victimIndex].Location);
			live[victimIndex] = live.back();
			live.pop_back();
		}

		// Periodically advance the submission counter so retire queue drains.
		if ((iteration % 32) == 0)
		{
			tracker.MarkAllFramesComplete();
			allocator.Flush();
		}
	}

	// Drain the rest.
	tracker.AdvanceFrame();
	for (Live& record : live)
		allocator.Free(record.Location);
	tracker.MarkAllFramesComplete();
	allocator.Flush();

	B3D_TEST_ASSERT(allocator.GetUsedBytes() == 0)
}

void GpuAllocatorTestSuite::TestTlsf_GranularityDisabled()
{
	// Default config: BufferImageGranularity = 1 → tracker is fully inert. Linear / NonLinear
	// allocations should pack contiguously without any BIG-driven padding.
	MockHeapBackend backend;
	MockGpuFrameTracker tracker;
	TlsfAllocator allocator(&backend, &tracker, MakeDefaultTlsfConfig());

	MockLocation linearLocation;
	B3D_TEST_ASSERT(allocator.TryAllocate(1000, 16, GpuResourceKind::Linear, linearLocation))

	MockLocation nonLinearLocation;
	B3D_TEST_ASSERT(allocator.TryAllocate(1000, 16, GpuResourceKind::NonLinear, nonLinearLocation))

	// With granularity disabled the second allocation must fall immediately after the first
	// (rounded up only by natural alignment to 16). 1000 → 16-aligned end is 1008.
	B3D_TEST_ASSERT(linearLocation.Offset == 0)
	B3D_TEST_ASSERT(nonLinearLocation.Offset == 1008)
}

void GpuAllocatorTestSuite::TestTlsf_GranularityHomogeneousNoPadding()
{
	// All-Linear workload — no conflicting neighbors anywhere, so BIG inflation never fires.
	MockHeapBackend backend;
	MockGpuFrameTracker tracker;
	TlsfAllocator allocator(&backend, &tracker, MakeTlsfConfigWithGranularity(4096));

	MockLocation a, b, c;
	B3D_TEST_ASSERT(allocator.TryAllocate(3000, 16, GpuResourceKind::Linear, a))
	B3D_TEST_ASSERT(allocator.TryAllocate(3000, 16, GpuResourceKind::Linear, b))
	B3D_TEST_ASSERT(allocator.TryAllocate(3000, 16, GpuResourceKind::Linear, c))

	// 3000 → 16-aligned end is 3008. Allocations pack tightly even though they all straddle
	// the 4 KB granularity boundary, because there's no Linear-vs-NonLinear conflict.
	B3D_TEST_ASSERT(a.Offset == 0)
	B3D_TEST_ASSERT(b.Offset == 3008)
	B3D_TEST_ASSERT(c.Offset == 6016)
}

void GpuAllocatorTestSuite::TestTlsf_GranularityLinearVsNonLinearInflatesPadding()
{
	// The first allocation occupies the start of page 0; the second is NonLinear and would
	// naturally land at offset 1008 but that's still inside page 0 (which now holds Linear),
	// so BIG inflation must bump it past the granularity boundary to offset 4096.
	MockHeapBackend backend;
	MockGpuFrameTracker tracker;
	TlsfAllocator allocator(&backend, &tracker, MakeTlsfConfigWithGranularity(4096));

	MockLocation linearLocation;
	B3D_TEST_ASSERT(allocator.TryAllocate(1000, 16, GpuResourceKind::Linear, linearLocation))
	B3D_TEST_ASSERT(linearLocation.Offset == 0)

	MockLocation nonLinearLocation;
	B3D_TEST_ASSERT(allocator.TryAllocate(1000, 16, GpuResourceKind::NonLinear, nonLinearLocation))
	B3D_TEST_ASSERT(nonLinearLocation.Offset == 4096)

	// Sanity: a Linear-Linear sequence in the same starting state would *not* be bumped.
	MockHeapBackend baselineBackend;
	MockGpuFrameTracker baselineTracker;
	TlsfAllocator baselineAllocator(&baselineBackend, &baselineTracker, MakeTlsfConfigWithGranularity(4096));

	MockLocation baselineFirst;
	MockLocation baselineSecond;
	B3D_TEST_ASSERT(baselineAllocator.TryAllocate(1000, 16, GpuResourceKind::Linear, baselineFirst))
	B3D_TEST_ASSERT(baselineAllocator.TryAllocate(1000, 16, GpuResourceKind::Linear, baselineSecond))
	B3D_TEST_ASSERT(baselineSecond.Offset == 1008)
}

void GpuAllocatorTestSuite::TestTlsf_GranularityRejectAndRetryAcrossHeaps()
{
	// Single 8 KB heap with two Linear allocations that fill all but the trailing 1 KB.
	// A NonLinear request that would land in the trailing slot is rejected by the BIG check
	// (start page holds a Linear allocation; bump-up overruns the heap), forcing the allocator
	// to fall through and create a fresh heap for the NonLinear.
	TlsfAllocator::Configuration configuration = MakeTlsfConfigWithGranularity(4096);
	configuration.InitialHeapSize = 8192;
	configuration.MaxHeapSize = 8192;

	MockHeapBackend backend;
	MockGpuFrameTracker tracker;
	TlsfAllocator allocator(&backend, &tracker, configuration);

	MockLocation firstLinear;
	MockLocation secondLinear;
	B3D_TEST_ASSERT(allocator.TryAllocate(4096, 16, GpuResourceKind::Linear, firstLinear))
	B3D_TEST_ASSERT(allocator.TryAllocate(3072, 16, GpuResourceKind::Linear, secondLinear))
	B3D_TEST_ASSERT(backend.CreateCount() == 1)

	// 1 KB free remaining at heap-tail (offset 7168) — but BIG forces the NonLinear past 8192,
	// which doesn't fit. The allocator must spin up a second heap.
	MockLocation nonLinear;
	B3D_TEST_ASSERT(allocator.TryAllocate(1024, 16, GpuResourceKind::NonLinear, nonLinear))
	B3D_TEST_ASSERT(backend.CreateCount() == 2)
	B3D_TEST_ASSERT(nonLinear.AllocatorData0 != firstLinear.AllocatorData0)
	B3D_TEST_ASSERT(nonLinear.Offset == 0)
}

void GpuAllocatorTestSuite::TestTlsf_GranularityFreeReleasesRegion()
{
	// Reserve page 0 with a Linear allocation, force a NonLinear past the boundary, then free
	// the Linear and confirm a fresh NonLinear can land at offset 0 — the page-table refcount
	// has dropped to zero so the page reverts to Free and no longer conflicts.
	MockHeapBackend backend;
	MockGpuFrameTracker tracker;
	TlsfAllocator allocator(&backend, &tracker, MakeTlsfConfigWithGranularity(4096));

	MockLocation linearLocation;
	MockLocation firstNonLinear;
	B3D_TEST_ASSERT(allocator.TryAllocate(1000, 16, GpuResourceKind::Linear, linearLocation))
	B3D_TEST_ASSERT(allocator.TryAllocate(1000, 16, GpuResourceKind::NonLinear, firstNonLinear))
	B3D_TEST_ASSERT(linearLocation.Offset == 0)
	B3D_TEST_ASSERT(firstNonLinear.Offset == 4096)

	FreeAndDrain(allocator, tracker,linearLocation);

	MockLocation freshNonLinear;
	B3D_TEST_ASSERT(allocator.TryAllocate(1000, 16, GpuResourceKind::NonLinear, freshNonLinear))
	B3D_TEST_ASSERT(freshNonLinear.Offset == 0)
}

namespace
{
	/**
	 * Forms a typed null reference to GpuCommandBuffer for unit tests that don't actually issue
	 * GPU commands. The MockResource implementations below never dereference the command buffer
	 * passed to MoveAllocation, so the underlying nullptr is never accessed; this hack lets the
	 * test reach Defrag's signature without dragging in a full command-buffer mock.
	 */
	render::GpuCommandBuffer& NullCommandBuffer()
	{
		render::GpuCommandBuffer* nullPtr = nullptr;
		return *nullPtr;
	}
}

void GpuAllocatorTestSuite::TestTlsf_Defrag_DrainsHighestHeap()
{
	// 64 KB heaps that hold 4 * 16 KB allocations each. Allocate 6 holders to force a second heap
	// (4 in heap 0, 2 in heap 1), then free 2 in heap 0 so it has 32 KB free to receive the heap 1
	// migrants. Defrag should drain heap 1 into heap 0 and the empty heap 1 should be released
	// once the deferred-free queue settles.
	MockHeapBackend backend;
	MockGpuFrameTracker tracker;

	TlsfAllocator::Configuration configuration = MakeDefaultTlsfConfig();
	configuration.InitialHeapSize = 64 * 1024;
	configuration.MaxHeapSize = 64 * 1024;
	configuration.GrowthFactor = 1;
	configuration.MaxEmptyHeapCount = 0;
	TlsfAllocator allocator(&backend, &tracker, configuration);

	struct Holder { MockResource Resource; MockLocation Location; };
	const u32 kAllocCount = 6;
	const u64 kAllocSize = 16 * 1024;

	Vector<UPtr<Holder>> holders;
	for (u32 holderIndex = 0; holderIndex < kAllocCount; holderIndex++)
	{
		auto holder = B3DMakeUnique<Holder>();
		holder->Resource.LocationPtr = &holder->Location;
		B3D_TEST_ASSERT(allocator.TryAllocate(kAllocSize, 16, GpuResourceKind::Linear, &holder->Resource, holder->Location))
		holders.push_back(std::move(holder));
	}

	// First 4 land in heap 0, last 2 spill to heap 1.
	B3D_TEST_ASSERT(allocator.GetHeapCount() == 2)
	const u32 heap0Slot = holders[0]->Location.AllocatorData0;
	const u32 heap1Slot = holders[4]->Location.AllocatorData0;
	B3D_TEST_ASSERT(heap0Slot != heap1Slot)
	B3D_TEST_ASSERT(holders[5]->Location.AllocatorData0 == heap1Slot)

	// Free 2 allocations in heap 0 so it has room to receive heap 1's migrants.
	tracker.AdvanceFrame();
	allocator.Free(holders[0]->Location);
	allocator.Free(holders[1]->Location);
	tracker.MarkAllFramesComplete();
	allocator.Flush(0, false);

	const TlsfAllocator::DefragmentationStats stats = allocator.Defrag(NullCommandBuffer());

	// The 2 holders in heap 1 should have moved to heap 0.
	B3D_TEST_ASSERT(stats.MovesCompleted == 2)
	B3D_TEST_ASSERT(holders[4]->Location.AllocatorData0 == heap0Slot)
	B3D_TEST_ASSERT(holders[5]->Location.AllocatorData0 == heap0Slot)

	// Drain the deferred-free queue: source slots in heap 1 are retired against the current frame
	// index, so advancing the frame and marking it complete drains them. With MaxEmptyHeapCount=0
	// the now-empty heap 1 is released back to the backend.
	tracker.AdvanceFrame();
	tracker.MarkAllFramesComplete();
	allocator.Flush(0, false);

	B3D_TEST_ASSERT(allocator.GetHeapCount() == 1)
}

void GpuAllocatorTestSuite::TestTlsf_Defrag_SingleHeapWithinHeapCompaction()
{
	// Single-heap configuration: a sawtooth pattern frees every other allocation, leaving holes
	// at low offsets. Defrag must compact higher-offset survivors down into those holes within
	// the same heap. This proves the NodeFlag::DefragDestination marker mechanism (destination
	// lands in the heap being walked) and that single-heap setups can defrag at all.
	MockHeapBackend backend;
	MockGpuFrameTracker tracker;

	TlsfAllocator::Configuration configuration = MakeDefaultTlsfConfig();
	configuration.InitialHeapSize = 1 * 1024 * 1024;
	configuration.MaxHeapSize = 1 * 1024 * 1024;
	TlsfAllocator allocator(&backend, &tracker, configuration);

	struct Holder { MockResource Resource; MockLocation Location; u64 OriginalOffset; };
	const u32 kAllocCount = 16;
	const u64 kAllocSize = 16 * 1024;

	Vector<UPtr<Holder>> holders;
	for (u32 holderIndex = 0; holderIndex < kAllocCount; holderIndex++)
	{
		auto holder = B3DMakeUnique<Holder>();
		holder->Resource.LocationPtr = &holder->Location;
		B3D_TEST_ASSERT(allocator.TryAllocate(kAllocSize, 16, GpuResourceKind::Linear, &holder->Resource, holder->Location))
		holder->OriginalOffset = holder->Location.Offset;
		holders.push_back(std::move(holder));
	}

	// Free every-other allocation. The remaining survivors live at original offsets that
	// alternate with newly-vacated holes.
	tracker.AdvanceFrame();
	for (u32 holderIndex = 0; holderIndex < kAllocCount; holderIndex += 2)
		allocator.Free(holders[holderIndex]->Location);
	tracker.MarkAllFramesComplete();
	allocator.Flush(0, false);

	const TlsfAllocator::DefragmentationStats stats = allocator.Defrag(NullCommandBuffer());

	// At least one survivor must have moved to a strictly lower offset. (We accept the move only
	// when destination offset < source offset within the same heap, so any completed move proves
	// productive within-heap compaction.)
	B3D_TEST_ASSERT(stats.MovesCompleted > 0)
	B3D_TEST_ASSERT(allocator.GetHeapCount() == 1) // No new heap created.

	bool sawCompaction = false;
	for (u32 holderIndex = 1; holderIndex < kAllocCount; holderIndex += 2)
	{
		if (holders[holderIndex]->Location.Offset < holders[holderIndex]->OriginalOffset)
		{
			sawCompaction = true;
			break;
		}
	}
	B3D_TEST_ASSERT(sawCompaction)
}

void GpuAllocatorTestSuite::TestTlsf_Defrag_RespectsBudget()
{
	// Single-heap setup where multiple compaction moves are possible. Budget = single-allocation
	// size aborts after the first attempt that would exceed.
	MockHeapBackend backend;
	MockGpuFrameTracker tracker;

	TlsfAllocator::Configuration configuration = MakeDefaultTlsfConfig();
	configuration.InitialHeapSize = 1 * 1024 * 1024;
	configuration.MaxHeapSize = 1 * 1024 * 1024;
	TlsfAllocator allocator(&backend, &tracker, configuration);

	struct Holder { MockResource Resource; MockLocation Location; };
	const u32 kAllocCount = 8;
	const u64 kAllocSize = 16 * 1024;

	Vector<UPtr<Holder>> holders;
	for (u32 holderIndex = 0; holderIndex < kAllocCount; holderIndex++)
	{
		auto holder = B3DMakeUnique<Holder>();
		holder->Resource.LocationPtr = &holder->Location;
		B3D_TEST_ASSERT(allocator.TryAllocate(kAllocSize, 16, GpuResourceKind::Linear, &holder->Resource, holder->Location))
		holders.push_back(std::move(holder));
	}

	tracker.AdvanceFrame();
	for (u32 holderIndex = 0; holderIndex < kAllocCount; holderIndex += 2)
		allocator.Free(holders[holderIndex]->Location);
	tracker.MarkAllFramesComplete();
	allocator.Flush(0, false);

	TlsfAllocator::DefragmentationInfo info{};
	info.MaxBytesPerCall = kAllocSize; // One allocation worth.
	info.MaxAllocationsPerCall = 0;

	const TlsfAllocator::DefragmentationStats stats = allocator.Defrag(NullCommandBuffer(), info);

	B3D_TEST_ASSERT(stats.BudgetExhausted)
	B3D_TEST_ASSERT(stats.BytesMoved <= info.MaxBytesPerCall)
}

void GpuAllocatorTestSuite::TestTlsf_Defrag_OnlySkipsUntrackedSlots()
{
	// One heap with mixed tracked and untracked allocations. Untracked slots (Owner == nullptr at
	// TryAllocate time) are the only ones Defrag should skip — they have no consumer to relocate
	// against. Tracked slots in the same workload should still be moved.
	MockHeapBackend backend;
	MockGpuFrameTracker tracker;

	TlsfAllocator::Configuration configuration = MakeDefaultTlsfConfig();
	configuration.InitialHeapSize = 1 * 1024 * 1024;
	configuration.MaxHeapSize = 1 * 1024 * 1024;
	TlsfAllocator allocator(&backend, &tracker, configuration);

	struct Holder { MockResource Resource; MockLocation Location; bool Tracked; };
	const u32 kAllocCount = 8;
	const u64 kAllocSize = 16 * 1024;

	Vector<UPtr<Holder>> holders;
	for (u32 holderIndex = 0; holderIndex < kAllocCount; holderIndex++)
	{
		auto holder = B3DMakeUnique<Holder>();
		// Even-indexed allocations are tracked, odd-indexed are untracked (owner == nullptr).
		holder->Tracked = ((holderIndex & 1) == 0);
		IGpuResource* owner = nullptr;
		if (holder->Tracked)
		{
			holder->Resource.LocationPtr = &holder->Location;
			owner = &holder->Resource;
		}
		B3D_TEST_ASSERT(allocator.TryAllocate(kAllocSize, 16, GpuResourceKind::Linear, owner, holder->Location))
		holders.push_back(std::move(holder));
	}

	// Free a couple of mid-heap allocations to create holes that downstream survivors could move
	// into. Free indices 2 and 4 — both happen to be tracked / untracked respectively, so the
	// resulting holes are arbitrary and the surviving tracked allocation at index 6 has somewhere
	// lower-offset to migrate to.
	tracker.AdvanceFrame();
	allocator.Free(holders[2]->Location);
	allocator.Free(holders[4]->Location);
	tracker.MarkAllFramesComplete();
	allocator.Flush(0, false);

	const TlsfAllocator::DefragmentationStats stats = allocator.Defrag(NullCommandBuffer());

	// Only the surviving tracked allocations were eligible for movement. The untracked ones could
	// not have been touched even if iteration reached them.
	B3D_TEST_ASSERT(stats.MovesCompleted > 0)
	for (UPtr<Holder>& holder : holders)
	{
		if (!holder->Tracked)
			B3D_TEST_ASSERT(holder->Resource.MovedCount == 0)
	}
}

void GpuAllocatorTestSuite::TestTlsf_Defrag_MovesInFlightResource()
{
	// All survivors report UseCount = 1 and BoundCount = 1 — i.e. they look "in flight" and
	// "bound to a recording command buffer". With the relaxed in-flight filter Defrag must move
	// them anyway; correctness comes from submission ordering, not from skipping the candidates.
	MockHeapBackend backend;
	MockGpuFrameTracker tracker;

	TlsfAllocator::Configuration configuration = MakeDefaultTlsfConfig();
	configuration.InitialHeapSize = 1 * 1024 * 1024;
	configuration.MaxHeapSize = 1 * 1024 * 1024;
	TlsfAllocator allocator(&backend, &tracker, configuration);

	struct Holder { MockResource Resource; MockLocation Location; };
	const u32 kAllocCount = 4;
	const u64 kAllocSize = 16 * 1024;

	Vector<UPtr<Holder>> holders;
	for (u32 holderIndex = 0; holderIndex < kAllocCount; holderIndex++)
	{
		auto holder = B3DMakeUnique<Holder>();
		holder->Resource.LocationPtr = &holder->Location;
		holder->Resource.UseCount = 1;
		holder->Resource.BoundCount = 1;
		B3D_TEST_ASSERT(allocator.TryAllocate(kAllocSize, 16, GpuResourceKind::Linear, &holder->Resource, holder->Location))
		holders.push_back(std::move(holder));
	}

	tracker.AdvanceFrame();
	for (u32 holderIndex = 0; holderIndex < kAllocCount; holderIndex += 2)
		allocator.Free(holders[holderIndex]->Location);
	tracker.MarkAllFramesComplete();
	allocator.Flush(0, false);

	const TlsfAllocator::DefragmentationStats stats = allocator.Defrag(NullCommandBuffer());

	// At least one in-flight survivor was moved — the relaxed filter no longer blocks them.
	B3D_TEST_ASSERT(stats.MovesAttempted > 0)
	B3D_TEST_ASSERT(stats.MovesCompleted > 0)

	// At least one of the in-flight survivors actually got moved.
	bool sawMove = false;
	for (u32 holderIndex = 1; holderIndex < kAllocCount; holderIndex += 2)
	{
		if (holders[holderIndex]->Resource.MovedCount > 0)
		{
			sawMove = true;
			break;
		}
	}
	B3D_TEST_ASSERT(sawMove)
}

void GpuAllocatorTestSuite::TestTlsf_Defrag_MoveAllocationReceivesContext()
{
	// Confirms the MoveAllocation arguments the consumer needs: the typed new Location that
	// identifies a live destination slot in the allocator. The mock captures the new Location at
	// MoveAllocation time and assigns it onto its externally-held location, mirroring the
	// production consumer contract (replace location wholesale, no field-by-field patching).
	MockHeapBackend backend;
	MockGpuFrameTracker tracker;

	TlsfAllocator::Configuration configuration = MakeDefaultTlsfConfig();
	configuration.InitialHeapSize = 1 * 1024 * 1024;
	configuration.MaxHeapSize = 1 * 1024 * 1024;
	TlsfAllocator allocator(&backend, &tracker, configuration);

	struct Holder { MockResource Resource; MockLocation Location; };
	const u32 kAllocCount = 4;
	const u64 kAllocSize = 16 * 1024;

	Vector<UPtr<Holder>> holders;
	for (u32 holderIndex = 0; holderIndex < kAllocCount; holderIndex++)
	{
		auto holder = B3DMakeUnique<Holder>();
		holder->Resource.LocationPtr = &holder->Location;
		B3D_TEST_ASSERT(allocator.TryAllocate(kAllocSize, 16, GpuResourceKind::Linear, &holder->Resource, holder->Location))
		holders.push_back(std::move(holder));
	}

	tracker.AdvanceFrame();
	for (u32 holderIndex = 0; holderIndex < kAllocCount; holderIndex += 2)
		allocator.Free(holders[holderIndex]->Location);
	tracker.MarkAllFramesComplete();
	allocator.Flush(0, false);

	// Capture original offsets for survivors before Defrag rewrites their locations.
	Vector<u64> originalOffsetForSurvivor;
	for (u32 holderIndex = 1; holderIndex < kAllocCount; holderIndex += 2)
		originalOffsetForSurvivor.push_back(holders[holderIndex]->Location.Offset);

	const TlsfAllocator::DefragmentationStats stats = allocator.Defrag(NullCommandBuffer());
	B3D_TEST_ASSERT(stats.MovesCompleted > 0)

	// At least one survivor's MovedCount went up. For every moved survivor, validate the recorded
	// context and confirm the consumer's location was overwritten with NewLocation.
	bool sawMove = false;
	u32 originalIndex = 0;
	for (u32 holderIndex = 1; holderIndex < kAllocCount; holderIndex += 2)
	{
		const Holder& holder = *holders[holderIndex];
		const u64 originalOffset = originalOffsetForSurvivor[originalIndex++];
		if (holder.Resource.MovedCount == 0)
			continue;

		sawMove = true;
		B3D_TEST_ASSERT(holder.Resource.LastSourceOffset == originalOffset)

		const MockLocation& newLocation = holder.Resource.LastNewLocation;
		B3D_TEST_ASSERT(newLocation.IsValid())
		B3D_TEST_ASSERT(newLocation.Offset != originalOffset)
		B3D_TEST_ASSERT(newLocation.Size == kAllocSize)

		// The mock replaces its location with NewLocation, so the externally-held location now
		// identifies the destination slot.
		B3D_TEST_ASSERT(holder.Location.Offset == newLocation.Offset)
		B3D_TEST_ASSERT(holder.Location.AllocatorData0 == newLocation.AllocatorData0)
		B3D_TEST_ASSERT(holder.Location.AllocatorData1 == newLocation.AllocatorData1)
	}
	B3D_TEST_ASSERT(sawMove)
}

void GpuAllocatorTestSuite::TestTlsf_ResourceLifecyclePolicy_FreesImmediately()
{
	// Under ResourceLifecycle, Free routes straight to FreeImmediate without going through the
	// deferred-free queue. The expectation: bytes go to zero the moment Free returns, regardless of
	// frame-tracker state. The frame tracker is intentionally not advanced or signaled here — if the
	// implementation took the FrameTracker path, the slot would still be queued.
	MockHeapBackend backend;
	MockGpuFrameTracker tracker;

	TlsfAllocator::Configuration configuration = MakeDefaultTlsfConfig();
	configuration.DeferralMode = GpuAllocatorFreeDeferralMode::ResourceLifecycle;
	TlsfAllocator allocator(&backend, &tracker, configuration);

	MockLocation location;
	B3D_TEST_ASSERT(allocator.TryAllocate(2048, 64, location))
	const u64 usedAfterAlloc = allocator.GetUsedBytes();
	B3D_TEST_ASSERT(usedAfterAlloc >= 2048)

	allocator.Free(location);

	// Synchronous release — no Flush needed, no frame-tracker tick required.
	B3D_TEST_ASSERT(allocator.GetUsedBytes() == 0)
	B3D_TEST_ASSERT(!location.IsValid())
}

void GpuAllocatorTestSuite::TestTlsf_FrameTrackerPolicy_DefersAcrossFrames()
{
	// Under FrameTracker, Free queues the slot against the current frame index. The slot is held in
	// the deferred-free queue until the frame is reported complete by the tracker.
	MockHeapBackend backend;
	MockGpuFrameTracker tracker;

	TlsfAllocator::Configuration configuration = MakeDefaultTlsfConfig();
	configuration.DeferralMode = GpuAllocatorFreeDeferralMode::FrameTracker;
	TlsfAllocator allocator(&backend, &tracker, configuration);

	MockLocation location;
	B3D_TEST_ASSERT(allocator.TryAllocate(2048, 64, location))
	const u64 usedAfterAlloc = allocator.GetUsedBytes();
	B3D_TEST_ASSERT(usedAfterAlloc >= 2048)

	// Free stamps the entry against the current frame index. With no tick the queue stays held.
	const u64 retireFrame = tracker.CurrentFrameIndex();
	allocator.Free(location);
	B3D_TEST_ASSERT(!location.IsValid())
	allocator.Flush(0, false);
	B3D_TEST_ASSERT(allocator.GetUsedBytes() == usedAfterAlloc)

	// Even after kMaximumFramesInFlight ticks the slot is still held — the mock tracker tracks
	// completion independently of the current frame, mirroring the production contract that
	// "frame-complete" is a GPU-drained predicate, not a tick predicate.
	for (u32 tick = 0; tick < RenderThread::kMaximumFramesInFlight; tick++)
		tracker.AdvanceFrame();
	allocator.Flush(0, false);
	B3D_TEST_ASSERT(allocator.GetUsedBytes() == usedAfterAlloc)

	// Marking the retire frame complete is what flips IsFrameComplete to true. Flush now drains.
	tracker.MarkFrameComplete(retireFrame);
	allocator.Flush(0, false);
	B3D_TEST_ASSERT(allocator.GetUsedBytes() == 0)
}

void GpuAllocatorTestSuite::TestTlsf_Defrag_LifecycleAllowsSwap()
{
	// Under ResourceLifecycle, MoveAllocation may return a different IGpuResource (wrapper-swap).
	// The destination slot is stamped with the returned pointer so subsequent defrag passes see
	// the destination as tracked-by-the-new-owner. The source slot is released by the "old wrapper
	// destructor" path, modelled here via an explicit FreeImmediate against the source-side
	// location the original holder still holds.
	MockHeapBackend backend;
	MockGpuFrameTracker tracker;

	TlsfAllocator::Configuration configuration = MakeDefaultTlsfConfig();
	configuration.InitialHeapSize = 64 * 1024;
	configuration.MaxHeapSize = 64 * 1024;
	configuration.GrowthFactor = 1;
	configuration.MaxEmptyHeapCount = 0;
	configuration.DeferralMode = GpuAllocatorFreeDeferralMode::ResourceLifecycle;
	TlsfAllocator allocator(&backend, &tracker, configuration);

	// MockResource override that returns a *different* IGpuResource from MoveAllocation, modelling
	// the wrapper-swap pattern only valid under ResourceLifecycle. The replacement's location is
	// patched with the new destination; the original holder's location is intentionally NOT
	// patched so the test can later release the source through the "old wrapper destructor" path.
	struct SwappingMockResource : public MockResource
	{
		MockResource* SwapTarget = nullptr;
		MockLocation* SwapTargetLocationPtr = nullptr;

		IGpuResource* MoveAllocation(render::GpuCommandBuffer& /*cb*/, const GpuResourceLocation& newLocation) override
		{
			MovedCount++;
			const auto& typedNewLocation = static_cast<const MockLocation&>(newLocation);
			LastNewLocation = typedNewLocation;

			// The replacement wrapper is now responsible for the destination slot — patch its
			// location to point at the destination. The original wrapper's location is left as
			// the still-intact source location, mirroring production: in a real swap the old
			// wrapper is then Destroy()-ed and its destructor frees the source slot via FreeMemory.
			if (SwapTargetLocationPtr != nullptr)
				*SwapTargetLocationPtr = typedNewLocation;

			return SwapTarget;
		}
	};

	const u64 kAllocSize = 16 * 1024;
	const u32 kAllocCount = 6;

	struct Holder
	{
		SwappingMockResource Resource;
		MockResource Replacement;
		MockLocation Location;
		MockLocation ReplacementLocation;
	};

	Vector<UPtr<Holder>> holders;
	holders.reserve(kAllocCount);
	for (u32 holderIndex = 0; holderIndex < kAllocCount; holderIndex++)
	{
		auto holder = B3DMakeUnique<Holder>();
		holder->Resource.LocationPtr = &holder->Location;
		holder->Resource.SwapTarget = &holder->Replacement;
		holder->Resource.SwapTargetLocationPtr = &holder->ReplacementLocation;
		holder->Replacement.LocationPtr = &holder->ReplacementLocation;
		B3D_TEST_ASSERT(allocator.TryAllocate(kAllocSize, 16, GpuResourceKind::Linear, &holder->Resource, holder->Location))
		holders.push_back(std::move(holder));
	}

	// First 4 land in heap 0, last 2 spill to heap 1.
	B3D_TEST_ASSERT(allocator.GetHeapCount() == 2)
	const u32 heap0Slot = holders[0]->Location.AllocatorData0;
	const u32 heap1Slot = holders[4]->Location.AllocatorData0;
	B3D_TEST_ASSERT(heap0Slot != heap1Slot)

	// ResourceLifecycle: Free is synchronous, no frame-tracker dance required to vacate space in
	// heap 0 to receive the heap 1 migrants.
	allocator.Free(holders[0]->Location);
	allocator.Free(holders[1]->Location);

	const TlsfAllocator::DefragmentationStats stats = allocator.Defrag(NullCommandBuffer());
	B3D_TEST_ASSERT(stats.MovesCompleted == 2)

	// Each moved holder's replacement now identifies the destination location in heap 0.
	for (u32 holderIndex = 4; holderIndex < kAllocCount; holderIndex++)
	{
		const Holder& holder = *holders[holderIndex];
		B3D_TEST_ASSERT(holder.Resource.MovedCount == 1)
		B3D_TEST_ASSERT(holder.ReplacementLocation.IsValid())
		B3D_TEST_ASSERT(holder.ReplacementLocation.AllocatorData0 == heap0Slot)
		B3D_TEST_ASSERT(holder.ReplacementLocation.Size == kAllocSize)

		// Source slot is committed but untracked under ResourceLifecycle — the allocator did NOT
		// retire it. Drive the "old wrapper destructor" by calling FreeImmediate against the
		// still-held source location.
		B3D_TEST_ASSERT(holders[holderIndex]->Location.AllocatorData0 == heap1Slot)
		allocator.FreeImmediate(holders[holderIndex]->Location);
	}

	// Heap 1 was vacated by the immediate-free path; with MaxEmptyHeapCount=0 it gets released.
	B3D_TEST_ASSERT(allocator.GetHeapCount() == 1)
}

void GpuAllocatorTestSuite::TestTlsf_ConcurrentAllocateAndFree()
{
	// Drives several worker threads through tight allocate / free loops against a single allocator.
	// Random sizes maximize contention on the free-list bitmaps and bucket heads. Final used-bytes
	// goes to zero only if every allocate/free pair was correctly serialized through the recursive
	// mutex — torn writes to the bucket heads would manifest as either an assertion failure inside
	// FreeNode (e.g. corrupted physical chain) or a non-zero used-bytes count at teardown.
	MockHeapBackend backend;
	MockGpuFrameTracker tracker;

	TlsfAllocator::Configuration configuration = MakeDefaultTlsfConfig();
	configuration.InitialHeapSize = 4 * 1024 * 1024;
	configuration.MaxHeapSize = 16 * 1024 * 1024;
	configuration.DeferralMode = GpuAllocatorFreeDeferralMode::ResourceLifecycle;
	TlsfAllocator allocator(&backend, nullptr, configuration);

	constexpr u32 kWorkerCount = 4;
	constexpr u32 kIterationsPerWorker = 500;

	auto fnWorker = [&](u32 seed)
	{
		std::mt19937 rng(seed);
		std::uniform_int_distribution<u32> sizeDistribution(64, 16 * 1024);
		std::uniform_int_distribution<u32> alignmentBitDistribution(4, 10); // 16..1024
		std::uniform_int_distribution<u32> liveCountDistribution(1, 8);

		Vector<MockLocation> live;
		live.reserve(8);

		for (u32 iteration = 0; iteration < kIterationsPerWorker; iteration++)
		{
			const bool wantAllocate = live.empty() || ((u32)live.size() < liveCountDistribution(rng));
			if (wantAllocate)
			{
				MockLocation loc;
				const u64 size = sizeDistribution(rng);
				const u32 alignment = 1u << alignmentBitDistribution(rng);
				if (allocator.TryAllocate(size, alignment, loc))
					live.push_back(loc);
			}
			else
			{
				std::uniform_int_distribution<u32> indexDistribution(0, (u32)live.size() - 1);
				const u32 victimIndex = indexDistribution(rng);
				allocator.Free(live[victimIndex]);
				live[victimIndex] = live.back();
				live.pop_back();
			}
		}

		// Drain the worker's remaining live set so the post-join used-bytes assertion is valid.
		for (MockLocation& loc : live)
			allocator.Free(loc);
	};

	Vector<std::thread> workers;
	workers.reserve(kWorkerCount);
	for (u32 workerIndex = 0; workerIndex < kWorkerCount; workerIndex++)
		workers.emplace_back(fnWorker, 0xB3D7B5Fu + workerIndex);

	for (std::thread& worker : workers)
		worker.join();

	B3D_TEST_ASSERT(allocator.GetUsedBytes() == 0)
}

void GpuAllocatorTestSuite::TestTlsf_ConcurrentDefragWithAllocateAndFree()
{
	// Several worker threads churn allocations against a fixed pool of resources while a defrag
	// thread runs Defrag() in a tight loop. Defrag's MoveAllocation callback rewrites slot.Location
	// in place; the recursive mutex serializes that write against concurrent worker access. The
	// custom mock captures every source location handed to MoveAllocation so the test can release
	// them at teardown — under ResourceLifecycle the allocator does not retire the source itself.
	MockHeapBackend backend;

	TlsfAllocator::Configuration configuration = MakeDefaultTlsfConfig();
	configuration.InitialHeapSize = 256 * 1024;
	configuration.MaxHeapSize = 1024 * 1024;
	configuration.DeferralMode = GpuAllocatorFreeDeferralMode::ResourceLifecycle;
	TlsfAllocator allocator(&backend, nullptr, configuration);

	// MoveAllocation captures the source location so the orchestrator can release it after the run.
	// Defrag is single-threaded against any given resource, so the SourceLocations vector is mutated
	// only by the defrag thread for this resource — no per-resource synchronization needed.
	struct TrackingMockResource : public MockResource
	{
		Vector<MockLocation> SourceLocations;

		IGpuResource* MoveAllocation(render::GpuCommandBuffer& /*cb*/, const GpuResourceLocation& newLocation) override
		{
			MovedCount++;
			const auto& typedNewLocation = static_cast<const MockLocation&>(newLocation);
			LastNewLocation = typedNewLocation;
			if (LocationPtr != nullptr)
			{
				SourceLocations.push_back(*LocationPtr);
				*LocationPtr = typedNewLocation;
			}
			return this;
		}
	};

	struct Slot
	{
		TrackingMockResource Resource;
		MockLocation Location;
		std::atomic<bool> InUse{ false };
	};

	constexpr u32 kSlotCount = 32;
	constexpr u32 kIterationsPerWorker = 400;
	constexpr u32 kWorkerCount = 4;

	Vector<UPtr<Slot>> slots;
	slots.reserve(kSlotCount);
	for (u32 slotIndex = 0; slotIndex < kSlotCount; slotIndex++)
	{
		auto slot = B3DMakeUnique<Slot>();
		slot->Resource.LocationPtr = &slot->Location;
		const bool ok = allocator.TryAllocate(4096, 16, GpuResourceKind::Linear, &slot->Resource, slot->Location);
		B3D_TEST_ASSERT(ok)
		slot->InUse.store(true, std::memory_order_relaxed);
		slots.push_back(std::move(slot));
	}

	std::atomic<bool> stop{ false };

	auto fnWorker = [&](u32 seed)
	{
		std::mt19937 rng(seed);
		std::uniform_int_distribution<u32> indexDistribution(0, kSlotCount - 1);
		std::uniform_int_distribution<u32> sizeDistribution(256, 8 * 1024);

		for (u32 iteration = 0; iteration < kIterationsPerWorker; iteration++)
		{
			if (stop.load(std::memory_order_relaxed))
				break;

			const u32 victimIndex = indexDistribution(rng);
			Slot& slot = *slots[victimIndex];

			// Claim the slot; only one worker mutates a given slot at a time.
			bool expected = true;
			if (!slot.InUse.compare_exchange_strong(expected, false, std::memory_order_acquire))
				continue;

			allocator.Free(slot.Location);

			const u64 newSize = sizeDistribution(rng);
			if (allocator.TryAllocate(newSize, 16, GpuResourceKind::Linear, &slot.Resource, slot.Location))
				slot.InUse.store(true, std::memory_order_release);
		}
	};

	auto fnDefragWorker = [&]()
	{
		while (!stop.load(std::memory_order_relaxed))
			allocator.Defrag(NullCommandBuffer());
	};

	Vector<std::thread> workers;
	workers.reserve(kWorkerCount);
	for (u32 workerIndex = 0; workerIndex < kWorkerCount; workerIndex++)
		workers.emplace_back(fnWorker, 0xC0FFEEu + workerIndex);
	std::thread defragThread(fnDefragWorker);

	for (std::thread& worker : workers)
		worker.join();
	stop.store(true, std::memory_order_release);
	defragThread.join();

	// Free every live destination plus every captured defrag source. ResourceLifecycle does not
	// retire source slots automatically — the consumer (this test) plays the role of the
	// destructor that would normally call FreeMemory on the old wrapper.
	for (UPtr<Slot>& slot : slots)
	{
		if (slot->InUse.load(std::memory_order_relaxed))
			allocator.Free(slot->Location);

		for (MockLocation& sourceLocation : slot->Resource.SourceLocations)
			allocator.FreeImmediate(sourceLocation);
	}

	B3D_TEST_ASSERT(allocator.GetUsedBytes() == 0)
}

void GpuAllocatorTestSuite::TestTlsf_ThreadUnsafePolicyOptOut()
{
	// Smoke-test the ThreadUnsafe specialization: GpuAllocatorMutex<false> / GpuAllocatorScopedLock<false>
	// are empty types, so the mutex member is empty-base-optimizable and every ScopedLock is a no-op
	// at the machine level. The test verifies the template instantiates and the public surface behaves
	// identically to the thread-safe variant for a single-threaded round-trip.
	using ThreadUnsafeAllocator = TGpuTlsfAllocator<MockHeapBackend, ThreadUnsafe>;

	MockHeapBackend backend;

	ThreadUnsafeAllocator::Configuration configuration;
	configuration.InitialHeapSize = 64 * 1024;
	configuration.MaxHeapSize = 256 * 1024;
	configuration.GrowthFactor = 2;
	configuration.MaxEmptyHeapCount = 1;
	configuration.MinAllocationSize = 16;
	configuration.DeferralMode = GpuAllocatorFreeDeferralMode::ResourceLifecycle;
	ThreadUnsafeAllocator allocator(&backend, nullptr, configuration);

	MockLocation locA;
	MockLocation locB;
	B3D_TEST_ASSERT(allocator.TryAllocate(1024, 16, locA))
	B3D_TEST_ASSERT(allocator.TryAllocate(2048, 16, locB))
	B3D_TEST_ASSERT(allocator.GetUsedBytes() >= 1024 + 2048)

	allocator.Free(locA);
	allocator.Free(locB);
	B3D_TEST_ASSERT(allocator.GetUsedBytes() == 0)
}
