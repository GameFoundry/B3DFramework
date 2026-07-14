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
#include "GpuBackend/Allocators/B3DGpuLinearAllocator.h"
#include "GpuBackend/Allocators/B3DGpuTlsfAllocator.h"
#include "GpuBackend/Allocators/B3DGpuResource.h"
#include "Renderer/B3DRenderer.h"

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
	B3D_ADD_TEST(GpuAllocatorTestSuite::TestTlsf_FailsSoftOnHeapCreationFailure)
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
	B3D_ADD_TEST(GpuAllocatorTestSuite::TestLinear_ContractAndInitialState)
	B3D_ADD_TEST(GpuAllocatorTestSuite::TestLinear_BumpPointerAlignedOffsets)
	B3D_ADD_TEST(GpuAllocatorTestSuite::TestLinear_FailsSoftOnHeapCreationFailure)
	B3D_ADD_TEST(GpuAllocatorTestSuite::TestLinear_OverflowRotatesPage)
	B3D_ADD_TEST(GpuAllocatorTestSuite::TestLinear_MultiPageWithinFrame)
	B3D_ADD_TEST(GpuAllocatorTestSuite::TestLinear_PageRecycledOnFenceComplete)
	B3D_ADD_TEST(GpuAllocatorTestSuite::TestLinear_OversizeBypassesPagePool)
	B3D_ADD_TEST(GpuAllocatorTestSuite::TestLinear_ResetRetiresActivePage)
	B3D_ADD_TEST(GpuAllocatorTestSuite::TestLinear_SparePageCap)
	B3D_ADD_TEST(GpuAllocatorTestSuite::TestLinear_FreeIsNoop)
	B3D_ADD_TEST(GpuAllocatorTestSuite::TestLinear_FreeImmediateOnSharedPageIsNoop)
	B3D_ADD_TEST(GpuAllocatorTestSuite::TestLinear_SharedPoolReusedAcrossAllocators)
	B3D_ADD_TEST(GpuAllocatorTestSuite::TestLinear_SharedPoolRespectsBound)
	B3D_ADD_TEST(GpuAllocatorTestSuite::TestLinear_SharedPoolDrainsOnlyAfterMarkerComplete)
	B3D_ADD_TEST(GpuAllocatorTestSuite::TestLinear_SharedPoolForceDrainReturnsPages)
	B3D_ADD_TEST(GpuAllocatorTestSuite::TestAllocatorIdentity_FreeRoutesByCarriedAllocator)
	B3D_ADD_TEST(GpuAllocatorTestSuite::TestAllocatorIdentity_DefraggedAllocationFreesThroughCarriedAllocator)
}

namespace
{
	/**
	 * Returns the first device exposed by the active backend, or nullptr when the backend has
	 * none (e.g. headless CI without a usable GPU). Tests bail out gracefully on nullptr to keep
	 * the suite usable on machines where the GPU plugin couldn't bring a device up.
	 */
	TShared<GpuDevice> GetActiveDevice()
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

	/** Backend-owned mock heap with a stable address. Carries the index used to find its host storage. */
	struct MockGpuHeap : IGpuHeap
	{
		u32 Id = 0;
		u64 Size = 0;
	};

	/**
	 * In-process implementation of the GpuHeapBackend trait used by the allocator unit tests.
	 * Backs each "heap" with a host-side Vector<u8> so that allocator-driven offsets can be
	 * exercised end-to-end without a real device.
	 */
	class MockHeapBackend
	{
	public:
		using HeapHandle = IGpuHeap*;

		struct HeapCreateInformation
		{
			u64 Alignment = 16;
			bool MapPersistently = true;
		};

		MockHeapBackend() = default;

		~MockHeapBackend()
		{
			// Heap objects have stable addresses for the backend's lifetime (DestroyHeap only frees the
			// host storage, never the object) so location identity-by-pointer stays valid across the test.
			for (MockGpuHeap* heap : mHeapObjects)
				B3DDelete(heap);
		}

		IGpuHeap* CreateHeap(u64 sizeInBytes, const HeapCreateInformation&)
		{
			MockGpuHeap* heap = B3DNew<MockGpuHeap>();
			heap->Id = (u32)mHeaps.size();
			heap->Size = sizeInBytes;
			mHeapObjects.push_back(heap);

			Storage storage;
			storage.Live = true;
			storage.Bytes.resize((size_t)sizeInBytes);
			mHeaps.push_back(std::move(storage));
			mLiveCount++;

			return heap;
		}

		void DestroyHeap(IGpuHeap* handle)
		{
			if (handle == nullptr)
				return;

			MockGpuHeap* heap = static_cast<MockGpuHeap*>(handle);
			B3D_ASSERT(heap->Id < mHeaps.size());

			Storage& storage = mHeaps[heap->Id];
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
		Vector<MockGpuHeap*> mHeapObjects;
		u32 mLiveCount = 0;
		u32 mDestroyCount = 0;
	};

	/**
	 * Heap backend whose CreateHeap always fails, simulating a fail-soft backend memory-exhaustion
	 * path that returns a null heap handle.
	 */
	class OutOfMemoryHeapBackend
	{
	public:
		using HeapHandle = IGpuHeap*;

		struct HeapCreateInformation
		{
		};

		HeapHandle CreateHeap(u64 /*sizeInBytes*/, const HeapCreateInformation&)
		{
			CreateCallCount++;
			return nullptr;
		}

		void DestroyHeap(HeapHandle handle)
		{
			if (handle != nullptr)
				DestroyCallCount++;
		}

		u32 CreateCallCount = 0;
		u32 DestroyCallCount = 0;
	};

	/**
	 * Standalone implementation of IGpuCompletionTracker for the allocator unit tests. Models the
	 * device-wide frame counter as two monotonic 64-bit values — mCurrent (the frame currently being
	 * recorded) and mCompleted (the highest frame the simulated GPU has fully drained). Lets tests
	 * exercise the deferred-delete contract end-to-end without a real GpuDevice.
	 */
	class MockGpuCompletionTracker : public IGpuCompletionTracker
	{
	public:
		u64 GetCurrentMarker() const override { return mCurrent; }
		bool IsMarkerComplete(u64 marker) const override { return mAnyComplete && marker <= mCompleted; }

		/**
		 * Simulates the device advancing to the next frame: bumps the current frame counter by one
		 * and returns the new value. Allocations retired immediately after this call observe the new
		 * frame index via GetCurrentMarker.
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

	using MockLocation = GpuResourceLocation;
	using TlsfAllocator = TGpuTlsfAllocator<MockHeapBackend>;

	/**
	 * Helper: advances the frame, frees, marks complete, flushes — drives a single retire entry
	 * to completion. Only meaningful when the allocator is configured with @c FrameTracker.
	 */
	void FreeAndDrain(TlsfAllocator& allocator, MockGpuCompletionTracker& tracker, MockLocation& location)
	{
		tracker.AdvanceFrame();
		allocator.Free(location);
		tracker.MarkAllFramesComplete();
		allocator.ReclaimUnused();
	}

	/**
	 * Default TLSF configuration: 1 MB heaps, capped at 4 MB, no warm-spare retention beyond 1.
	 * The tests override individual fields as needed. @c DeferralMode defaults to @c FrameTracker so
	 * existing tests that drive @c Free → @c AdvanceFrame → @c MarkFrameComplete → @c ReclaimUnused keep
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

	using LinearAllocator = TGpuLinearAllocator<MockHeapBackend>;
	using LinearPagePool = TGpuLinearPagePool<MockHeapBackend>;

	/** Default linear allocator configuration. Tests that need a different page size or spare cap pass overrides. */
	LinearAllocator::Configuration MakeDefaultLinearConfig(u64 pageSize = 64 * 1024, u32 maxRetained = 2)
	{
		LinearAllocator::Configuration configuration;
		configuration.PageSize = pageSize;
		configuration.MaxRetainedPages = maxRetained;
		return configuration;
	}

	/** Default shared page-pool configuration. Page size must match the allocators drawing from it. */
	LinearPagePool::Configuration MakeDefaultPagePoolConfig(u64 pageSize = 4 * 1024, u32 maxRetained = 2)
	{
		LinearPagePool::Configuration configuration;
		configuration.PageSize = pageSize;
		configuration.MaxRetainedPages = maxRetained;
		return configuration;
	}

	/** Helper: advances frame, marks all complete, flushes — drains every retire entry stamped at or before the call. */
	template <typename Allocator>
	void AdvanceCompleteAndFlush(Allocator& allocator, MockGpuCompletionTracker& tracker)
	{
		tracker.AdvanceFrame();
		tracker.MarkAllFramesComplete();
		allocator.ReclaimUnused();
	}

	/** Identifier the deferred-free queue forwards to a strategy's FreeAndReclaimImpl. */
	struct FreedSlot
	{
		u32 AllocatorData0;
		u32 AllocatorData1;
	};

	/**
	 * Minimal CRTP-derived allocator used by the contract and deferred-delete tests. Records every
	 * FreeAndReclaimImpl call in FreedSlots so tests can assert which retired slots were actually drained.
	 * TryAllocateImpl / FreeImpl exist only to prove the public surface compiles and links.
	 */
	class MockAllocator : public TGpuAllocator<MockAllocator, MockHeapBackend>
	{
	public:
		using Base = TGpuAllocator<MockAllocator, MockHeapBackend>;

		// Surface the protected retire hook so the contract test can drive it directly without
		// having to round-trip through the public Free path (which auto-resets the location).
		using Base::RetireAllocation;

		MockAllocator(MockHeapBackend* backend, MockGpuCompletionTracker* tracker)
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

		void FreeAndReclaimImpl(u32 allocatorData0, u32 allocatorData1)
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
	static_assert(std::is_standard_layout<MockLocation>::value, "GpuResourceLocation must remain standard-layout.");
	static_assert(std::is_trivially_copyable<MockLocation>::value, "GpuResourceLocation must remain trivially copyable.");

	MockHeapBackend backend;
	MockGpuCompletionTracker tracker;
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

	allocator.ReclaimUnused();
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
		MockGpuCompletionTracker tracker;
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
		allocator.ReclaimUnused();

		B3D_TEST_ASSERT(allocator.FreedSlots.size() == 1)
		B3D_TEST_ASSERT(allocator.FreedSlots[0].AllocatorData0 == 1)
		B3D_TEST_ASSERT(allocator.FreedSlots[0].AllocatorData1 == 10)

		// Case 2: Subsequent advance drains the rest in original order.
		tracker.MarkFrameComplete(indexC);
		allocator.ReclaimUnused();

		B3D_TEST_ASSERT(allocator.FreedSlots.size() == 3)
		B3D_TEST_ASSERT(allocator.FreedSlots[1].AllocatorData0 == 2)
		B3D_TEST_ASSERT(allocator.FreedSlots[1].AllocatorData1 == 20)
		B3D_TEST_ASSERT(allocator.FreedSlots[2].AllocatorData0 == 3)
		B3D_TEST_ASSERT(allocator.FreedSlots[2].AllocatorData1 == 30)
	}

	// Case 3: ReclaimUnused(forceReclaimAll=true) drains everything regardless of submission state.
	{
		MockHeapBackend backend;
		MockGpuCompletionTracker tracker;
		MockAllocator allocator(&backend, &tracker);

		MockLocation locationA, locationB, locationC;
		locationA.AllocatorData0 = 1; locationA.AllocatorData1 = 10;
		locationB.AllocatorData0 = 2; locationB.AllocatorData1 = 20;
		locationC.AllocatorData0 = 3; locationC.AllocatorData1 = 30;

		tracker.AdvanceFrame(); allocator.RetireAllocation(locationA);
		tracker.AdvanceFrame(); allocator.RetireAllocation(locationB);
		tracker.AdvanceFrame(); allocator.RetireAllocation(locationC);

		// No Signal() call — submissions remain incomplete.
		allocator.ReclaimUnused(true);

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
		MockGpuCompletionTracker tracker;
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
		allocator.ReclaimUnused();

		B3D_TEST_ASSERT(allocator.FreedSlots.size() == 1)
		B3D_TEST_ASSERT(allocator.FreedSlots[0].AllocatorData0 == 42)
		B3D_TEST_ASSERT(allocator.FreedSlots[0].AllocatorData1 == 99)
	}
}

void GpuAllocatorTestSuite::TestFrameTracker_InitialState()
{
	TShared<GpuDevice> device = GetActiveDevice();
	if (device == nullptr)
		return;

	// The current frame index is "whatever has been advanced so far". Other engine subsystems
	// (renderer warm-up, asset import bring-up) may have ticked it before this test runs,
	// so the value is non-deterministic — what matters is that the predicate is monotone: a frame
	// reported complete now stays complete, and the current frame is never reported complete (it
	// is by definition still being recorded).
	IGpuCompletionTracker& tracker = render::GetRenderer()->GetFrameCompletionTracker();
	const u64 frame = tracker.GetCurrentMarker();
	B3D_TEST_ASSERT(!tracker.IsMarkerComplete(frame))

	if (frame >= RenderThread::kMaximumFramesInFlight)
		B3D_TEST_ASSERT(tracker.IsMarkerComplete(frame - RenderThread::kMaximumFramesInFlight))
}

void GpuAllocatorTestSuite::TestFrameTracker_AdvancesOnEndFrame()
{
	TShared<GpuDevice> device = GetActiveDevice();
	if (device == nullptr || !IsRealBackend(*device))
		return;

	GpuFrameCompletionTracker& tracker = render::GetRenderer()->GetFrameCompletionTracker();
	const u64 indexBefore = tracker.GetCurrentMarker();

	// BeginGpuFrame / EndGpuFrame have a render-thread-only contract — they advance render-thread-owned
	// work-context and command-buffer pool state without internal synchronization. Drive them from the
	// render thread.
	const auto fnTickFrame = []()
	{
		const TShared<render::Renderer> renderer = render::GetRenderer();
		renderer->BeginFrame();
		renderer->EndFrame();
	};

	GetRenderThread().PostCommand(fnTickFrame, "TestFrameTracker_AdvancesOnEndFrame::Tick", true);

	const u64 indexAfter = tracker.GetCurrentMarker();
	B3D_TEST_ASSERT(indexAfter == indexBefore + 1)

	// After kMaximumFramesInFlight further frame ticks, the original frame must be reported complete.
	for (u32 i = 0; i < RenderThread::kMaximumFramesInFlight; i++)
		GetRenderThread().PostCommand(fnTickFrame, "TestFrameTracker_AdvancesOnEndFrame::Tick", true);

	device->WaitUntilIdle();
	B3D_TEST_ASSERT(tracker.IsMarkerComplete(indexBefore))
}

void GpuAllocatorTestSuite::TestUserCreatedFence_ExplicitSignal()
{
	TShared<GpuDevice> device = GetActiveDevice();
	if (device == nullptr || !IsRealBackend(*device))
		return;

	TShared<GpuTimelineFence> fence = device->CreateTimelineFence();
	if (fence == nullptr)
		return; // Backend has not yet implemented user-created fences (e.g. D3D12 today).

	// Skip when the fence is in a degraded mode that lacks per-submit visibility (e.g. Vulkan on
	// hardware/drivers without VK_KHR_timeline_semaphore). The explicit-value round-trip is
	// meaningless on those paths because GetCompletedValue cannot observe per-value signals.
	if (fence->IsSignaled(7))
		return;

	B3D_TEST_ASSERT(fence->GetCompletedValue() == 0)
	B3D_TEST_ASSERT(!fence->IsSignaled(7))

	// Allocate a fresh command buffer outside the transfer machinery so that resubmitting it during a
	// later EndFrame doesn't break.
	GpuCommandBufferPoolCreateInformation poolCreateInfo = GpuCommandBufferPoolCreateInformation::CreateForThisThread(GQT_GRAPHICS);
	TShared<GpuCommandBufferPool> pool = device->CreateGpuCommandBufferPool(poolCreateInfo);
	TShared<GpuCommandBuffer> commandBuffer = pool->Create(GpuCommandBufferCreateInformation::Create("UserFenceTestCB"));
	// Pool::Create returns a CB already in the Recording state; SubmitCommandBuffer auto-ends.

	GpuSubmissionInformation info;
	info.CommandBuffer = commandBuffer;
	info.SignalFences.Add(GpuTimelineFenceAndValue{ fence, 7 });

	// Submit through a worker context owned by this thread; the render-thread primary context stays untouched.
	TShared<GpuWorkContext> gpuContext = GpuWorkContext::Create(*device);
	gpuContext->SubmitCommandBuffer(info);

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
	MockGpuCompletionTracker tracker;

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
	MockGpuCompletionTracker tracker;
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
	allocator.ReclaimUnused();
	B3D_TEST_ASSERT(allocator.GetUsedBytes() == 0)
}

void GpuAllocatorTestSuite::TestTlsf_FailsSoftOnHeapCreationFailure()
{
	// A TLSF allocator over a backend that cannot create heaps must fail the allocation gracefully
	// (TryAllocate returns false, the location stays invalid) rather than constructing a heap wrapper
	// around a null handle.
	OutOfMemoryHeapBackend backend;

	TGpuTlsfAllocator<OutOfMemoryHeapBackend>::Configuration configuration;
	configuration.InitialHeapSize = 4096;
	configuration.MaxHeapSize = 65536;
	configuration.DeferralMode = GpuAllocatorFreeDeferralMode::ResourceLifecycle;

	TGpuTlsfAllocator<OutOfMemoryHeapBackend> allocator(&backend, nullptr, configuration);

	GpuResourceLocation location;
	const bool allocated = allocator.TryAllocate(256, 16, GpuResourceKind::Linear, nullptr, location);

	B3D_TEST_ASSERT(allocated == false)
	B3D_TEST_ASSERT(location.IsValid() == false)
	B3D_TEST_ASSERT(backend.CreateCallCount == 1)
	B3D_TEST_ASSERT(backend.DestroyCallCount == 0)
	B3D_TEST_ASSERT(allocator.GetHeapCount() == 0)
}

void GpuAllocatorTestSuite::TestTlsf_NonOverlappingAlignedOffsets()
{
	MockHeapBackend backend;
	MockGpuCompletionTracker tracker;
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
			if (allocs[outerIndex].Location.Heap != allocs[innerIndex].Location.Heap)
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
	allocator.ReclaimUnused();
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
		MockGpuCompletionTracker tracker;

		TlsfAllocator::Configuration configuration = MakeDefaultTlsfConfig();
		configuration.InitialHeapSize = 64 * 1024;
		configuration.MaxHeapSize = 64 * 1024;
		TlsfAllocator allocator(&backend, &tracker, configuration);

		const u64 blockSize = 16 * 1024;
		MockLocation locations[3];
		for (u32 blockIndex = 0; blockIndex < 3; blockIndex++)
			B3D_TEST_ASSERT(allocator.TryAllocate(blockSize, 16, locations[blockIndex]))

		// All three live in the same heap (heap is 64KB, allocations total 48KB).
		B3D_TEST_ASSERT(locations[0].Heap == locations[1].Heap)
		B3D_TEST_ASSERT(locations[0].Heap == locations[2].Heap)

		tracker.AdvanceFrame();
		for (u32 freeIndex = 0; freeIndex < 3; freeIndex++)
			allocator.Free(locations[freeOrder[freeIndex]]);
		tracker.MarkAllFramesComplete();
		allocator.ReclaimUnused();
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
	MockGpuCompletionTracker tracker;

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
	allocator.ReclaimUnused();
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
	MockGpuCompletionTracker tracker;

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
	allocator.ReclaimUnused();

	// MaxEmptyHeapCount = 1, so all but one heap must be returned to the backend.
	B3D_TEST_ASSERT(allocator.GetHeapCount() == 1)
	B3D_TEST_ASSERT(backend.LiveHeapCount() == 1)
	B3D_TEST_ASSERT(backend.DestroyCount() == heapsAfterFill - 1)
}

void GpuAllocatorTestSuite::TestTlsf_OversizedAllocationGetsDedicatedHeap()
{
	MockHeapBackend backend;
	MockGpuCompletionTracker tracker;

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
	MockGpuCompletionTracker tracker;

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
					if (other.Location.Heap != record.Location.Heap)
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
			allocator.ReclaimUnused();
		}
	}

	// Drain the rest.
	tracker.AdvanceFrame();
	for (Live& record : live)
		allocator.Free(record.Location);
	tracker.MarkAllFramesComplete();
	allocator.ReclaimUnused();

	B3D_TEST_ASSERT(allocator.GetUsedBytes() == 0)
}

void GpuAllocatorTestSuite::TestTlsf_GranularityDisabled()
{
	// Default config: BufferImageGranularity = 1 → tracker is fully inert. Linear / NonLinear
	// allocations should pack contiguously without any BIG-driven padding.
	MockHeapBackend backend;
	MockGpuCompletionTracker tracker;
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
	MockGpuCompletionTracker tracker;
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
	MockGpuCompletionTracker tracker;
	TlsfAllocator allocator(&backend, &tracker, MakeTlsfConfigWithGranularity(4096));

	MockLocation linearLocation;
	B3D_TEST_ASSERT(allocator.TryAllocate(1000, 16, GpuResourceKind::Linear, linearLocation))
	B3D_TEST_ASSERT(linearLocation.Offset == 0)

	MockLocation nonLinearLocation;
	B3D_TEST_ASSERT(allocator.TryAllocate(1000, 16, GpuResourceKind::NonLinear, nonLinearLocation))
	B3D_TEST_ASSERT(nonLinearLocation.Offset == 4096)

	// Sanity: a Linear-Linear sequence in the same starting state would *not* be bumped.
	MockHeapBackend baselineBackend;
	MockGpuCompletionTracker baselineTracker;
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
	MockGpuCompletionTracker tracker;
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
	MockGpuCompletionTracker tracker;
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
	MockGpuCompletionTracker tracker;

	TlsfAllocator::Configuration configuration = MakeDefaultTlsfConfig();
	configuration.InitialHeapSize = 64 * 1024;
	configuration.MaxHeapSize = 64 * 1024;
	configuration.GrowthFactor = 1;
	configuration.MaxEmptyHeapCount = 0;
	TlsfAllocator allocator(&backend, &tracker, configuration);

	struct Holder { MockResource Resource; MockLocation Location; };
	const u32 kAllocCount = 6;
	const u64 kAllocSize = 16 * 1024;

	Vector<TUnique<Holder>> holders;
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
	allocator.ReclaimUnused(false);

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
	allocator.ReclaimUnused(false);

	B3D_TEST_ASSERT(allocator.GetHeapCount() == 1)
}

void GpuAllocatorTestSuite::TestTlsf_Defrag_SingleHeapWithinHeapCompaction()
{
	// Single-heap configuration: a sawtooth pattern frees every other allocation, leaving holes
	// at low offsets. Defrag must compact higher-offset survivors down into those holes within
	// the same heap. This proves the NodeFlag::DefragDestination marker mechanism (destination
	// lands in the heap being walked) and that single-heap setups can defrag at all.
	MockHeapBackend backend;
	MockGpuCompletionTracker tracker;

	TlsfAllocator::Configuration configuration = MakeDefaultTlsfConfig();
	configuration.InitialHeapSize = 1 * 1024 * 1024;
	configuration.MaxHeapSize = 1 * 1024 * 1024;
	TlsfAllocator allocator(&backend, &tracker, configuration);

	struct Holder { MockResource Resource; MockLocation Location; u64 OriginalOffset; };
	const u32 kAllocCount = 16;
	const u64 kAllocSize = 16 * 1024;

	Vector<TUnique<Holder>> holders;
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
	allocator.ReclaimUnused(false);

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
	MockGpuCompletionTracker tracker;

	TlsfAllocator::Configuration configuration = MakeDefaultTlsfConfig();
	configuration.InitialHeapSize = 1 * 1024 * 1024;
	configuration.MaxHeapSize = 1 * 1024 * 1024;
	TlsfAllocator allocator(&backend, &tracker, configuration);

	struct Holder { MockResource Resource; MockLocation Location; };
	const u32 kAllocCount = 8;
	const u64 kAllocSize = 16 * 1024;

	Vector<TUnique<Holder>> holders;
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
	allocator.ReclaimUnused(false);

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
	MockGpuCompletionTracker tracker;

	TlsfAllocator::Configuration configuration = MakeDefaultTlsfConfig();
	configuration.InitialHeapSize = 1 * 1024 * 1024;
	configuration.MaxHeapSize = 1 * 1024 * 1024;
	TlsfAllocator allocator(&backend, &tracker, configuration);

	struct Holder { MockResource Resource; MockLocation Location; bool Tracked; };
	const u32 kAllocCount = 8;
	const u64 kAllocSize = 16 * 1024;

	Vector<TUnique<Holder>> holders;
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
	allocator.ReclaimUnused(false);

	const TlsfAllocator::DefragmentationStats stats = allocator.Defrag(NullCommandBuffer());

	// Only the surviving tracked allocations were eligible for movement. The untracked ones could
	// not have been touched even if iteration reached them.
	B3D_TEST_ASSERT(stats.MovesCompleted > 0)
	for (TUnique<Holder>& holder : holders)
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
	MockGpuCompletionTracker tracker;

	TlsfAllocator::Configuration configuration = MakeDefaultTlsfConfig();
	configuration.InitialHeapSize = 1 * 1024 * 1024;
	configuration.MaxHeapSize = 1 * 1024 * 1024;
	TlsfAllocator allocator(&backend, &tracker, configuration);

	struct Holder { MockResource Resource; MockLocation Location; };
	const u32 kAllocCount = 4;
	const u64 kAllocSize = 16 * 1024;

	Vector<TUnique<Holder>> holders;
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
	allocator.ReclaimUnused(false);

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
	MockGpuCompletionTracker tracker;

	TlsfAllocator::Configuration configuration = MakeDefaultTlsfConfig();
	configuration.InitialHeapSize = 1 * 1024 * 1024;
	configuration.MaxHeapSize = 1 * 1024 * 1024;
	TlsfAllocator allocator(&backend, &tracker, configuration);

	struct Holder { MockResource Resource; MockLocation Location; };
	const u32 kAllocCount = 4;
	const u64 kAllocSize = 16 * 1024;

	Vector<TUnique<Holder>> holders;
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
	allocator.ReclaimUnused(false);

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
	// Under ResourceLifecycle, Free routes straight to FreeAndReclaim without going through the
	// deferred-free queue. The expectation: bytes go to zero the moment Free returns, regardless of
	// frame-tracker state. The frame tracker is intentionally not advanced or signaled here — if the
	// implementation took the FrameTracker path, the slot would still be queued.
	MockHeapBackend backend;
	MockGpuCompletionTracker tracker;

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
	MockGpuCompletionTracker tracker;

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
	allocator.ReclaimUnused(false);
	B3D_TEST_ASSERT(allocator.GetUsedBytes() == usedAfterAlloc)

	// Even after kMaximumFramesInFlight ticks the slot is still held — the mock tracker tracks
	// completion independently of the current frame, mirroring the production contract that
	// "frame-complete" is a GPU-drained predicate, not a tick predicate.
	for (u32 tick = 0; tick < RenderThread::kMaximumFramesInFlight; tick++)
		tracker.AdvanceFrame();
	allocator.ReclaimUnused(false);
	B3D_TEST_ASSERT(allocator.GetUsedBytes() == usedAfterAlloc)

	// Marking the retire frame complete is what flips IsMarkerComplete to true. Flush now drains.
	tracker.MarkFrameComplete(retireFrame);
	allocator.ReclaimUnused(false);
	B3D_TEST_ASSERT(allocator.GetUsedBytes() == 0)
}

void GpuAllocatorTestSuite::TestTlsf_Defrag_LifecycleAllowsSwap()
{
	// Under ResourceLifecycle, MoveAllocation may return a different IGpuResource (wrapper-swap).
	// The destination slot is stamped with the returned pointer so subsequent defrag passes see
	// the destination as tracked-by-the-new-owner. The source slot is released by the "old wrapper
	// destructor" path, modelled here via an explicit FreeAndReclaim against the source-side
	// location the original holder still holds.
	MockHeapBackend backend;
	MockGpuCompletionTracker tracker;

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

	Vector<TUnique<Holder>> holders;
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
		// retire it. Drive the "old wrapper destructor" by calling FreeAndReclaim against the
		// still-held source location.
		B3D_TEST_ASSERT(holders[holderIndex]->Location.AllocatorData0 == heap1Slot)
		allocator.FreeAndReclaim(holders[holderIndex]->Location);
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
	MockGpuCompletionTracker tracker;

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

	Vector<TUnique<Slot>> slots;
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
	for (TUnique<Slot>& slot : slots)
	{
		if (slot->InUse.load(std::memory_order_relaxed))
			allocator.Free(slot->Location);

		for (MockLocation& sourceLocation : slot->Resource.SourceLocations)
			allocator.FreeAndReclaim(sourceLocation);
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

void GpuAllocatorTestSuite::TestLinear_ContractAndInitialState()
{
	B3D_STATIC_ASSERT_HEAP_BACKEND_IS_VALID(MockHeapBackend);

	MockHeapBackend backend;
	MockGpuCompletionTracker tracker;

	// No heap creation on construction — pages are created lazily on first allocation.
	LinearAllocator allocator(&backend, &tracker, MakeDefaultLinearConfig());
	B3D_TEST_ASSERT(allocator.GetLivePageCount() == 0)
	B3D_TEST_ASSERT(allocator.GetCommittedBytes() == 0)
	B3D_TEST_ASSERT(allocator.GetUsedBytes() == 0)
	B3D_TEST_ASSERT(allocator.GetSparePageCount() == 0)
	B3D_TEST_ASSERT(backend.LiveHeapCount() == 0)

	// First TryAllocate creates the initial page.
	MockLocation location;
	B3D_TEST_ASSERT(allocator.TryAllocate(1024, 16, location))
	B3D_TEST_ASSERT(allocator.GetLivePageCount() == 1)
	B3D_TEST_ASSERT(allocator.GetCommittedBytes() == 64 * 1024)
	B3D_TEST_ASSERT(backend.LiveHeapCount() == 1)
	B3D_TEST_ASSERT(location.IsValid())
	B3D_TEST_ASSERT(location.Allocator == &allocator)
	B3D_TEST_ASSERT(location.Size == 1024)
	B3D_TEST_ASSERT((location.Offset & 15) == 0)
}

void GpuAllocatorTestSuite::TestLinear_BumpPointerAlignedOffsets()
{
	MockHeapBackend backend;
	MockGpuCompletionTracker tracker;
	LinearAllocator allocator(&backend, &tracker, MakeDefaultLinearConfig());

	struct Alloc { MockLocation Location; u64 Begin; u64 End; };
	Vector<Alloc> allocs;

	const u32 allocCount = 8;
	const u64 allocSize = 1024;
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

	// All allocations land in the same page (8 * 1024 << 64KB) and don't overlap.
	for (u32 outerIndex = 0; outerIndex < allocs.size(); outerIndex++)
	{
		B3D_TEST_ASSERT(allocs[outerIndex].Location.Heap == allocs[0].Location.Heap)
		for (u32 innerIndex = outerIndex + 1; innerIndex < allocs.size(); innerIndex++)
		{
			const bool disjoint = allocs[outerIndex].End <= allocs[innerIndex].Begin
				|| allocs[innerIndex].End <= allocs[outerIndex].Begin;
			B3D_TEST_ASSERT(disjoint)
		}
	}

	B3D_TEST_ASSERT(allocator.GetLivePageCount() == 1)
}

void GpuAllocatorTestSuite::TestLinear_FailsSoftOnHeapCreationFailure()
{
	OutOfMemoryHeapBackend backend;
	MockGpuCompletionTracker tracker;

	using OutOfMemoryLinearAllocator = TGpuLinearAllocator<OutOfMemoryHeapBackend>;
	OutOfMemoryLinearAllocator::Configuration configuration;
	configuration.PageSize = 4 * 1024;

	OutOfMemoryLinearAllocator allocator(&backend, &tracker, configuration);

	GpuResourceLocation regularLocation;
	B3D_TEST_ASSERT(!allocator.TryAllocate(256, 16, GpuResourceKind::Linear, nullptr, regularLocation))
	B3D_TEST_ASSERT(!regularLocation.IsValid())
	B3D_TEST_ASSERT(allocator.GetOutstandingAllocationCount() == 0)

	GpuResourceLocation oversizeLocation;
	B3D_TEST_ASSERT(!allocator.TryAllocate(8 * 1024, 16, GpuResourceKind::Linear, nullptr, oversizeLocation))
	B3D_TEST_ASSERT(!oversizeLocation.IsValid())
	B3D_TEST_ASSERT(allocator.GetOutstandingAllocationCount() == 0)

	B3D_TEST_ASSERT(backend.CreateCallCount == 2)
	B3D_TEST_ASSERT(backend.DestroyCallCount == 0)
}

void GpuAllocatorTestSuite::TestLinear_OverflowRotatesPage()
{
	MockHeapBackend backend;
	MockGpuCompletionTracker tracker;
	// 8 KB page with no spare retention so the destroy-on-drain path is also exercised.
	LinearAllocator allocator(&backend, &tracker, MakeDefaultLinearConfig(/*pageSize*/ 8 * 1024, /*maxRetained*/ 0));

	MockLocation first;
	B3D_TEST_ASSERT(allocator.TryAllocate(5 * 1024, 16, first))

	// 5KB used — a 4KB request can't fit in the remaining 3KB, must rotate.
	MockLocation second;
	B3D_TEST_ASSERT(allocator.TryAllocate(4 * 1024, 16, second))
	B3D_TEST_ASSERT(second.Heap != first.Heap)
	B3D_TEST_ASSERT(allocator.GetLivePageCount() == 2) // First retired-pending-drain, second active.

	// First allocation's location is still valid — backend heap is alive until fence drains.
	B3D_TEST_ASSERT(backend.LiveHeapCount() == 2)

	// Drain — first page goes through FreeAndReclaimImpl. With MaxRetainedPages=0, it destructs.
	AdvanceCompleteAndFlush(allocator, tracker);
	B3D_TEST_ASSERT(allocator.GetSparePageCount() == 0)
	B3D_TEST_ASSERT(allocator.GetLivePageCount() == 1) // Only the active second page remains.
	B3D_TEST_ASSERT(backend.LiveHeapCount() == 1)
	B3D_TEST_ASSERT(backend.DestroyCount() == 1)
}

void GpuAllocatorTestSuite::TestLinear_MultiPageWithinFrame()
{
	MockHeapBackend backend;
	MockGpuCompletionTracker tracker;
	LinearAllocator allocator(&backend, &tracker, MakeDefaultLinearConfig(/*pageSize*/ 4 * 1024, /*maxRetained*/ 4));

	// Force three page rotations (no AdvanceFrame between them) — three retired pages plus one active.
	MockLocation a, b, c, d;
	B3D_TEST_ASSERT(allocator.TryAllocate(3 * 1024, 16, a))
	B3D_TEST_ASSERT(allocator.TryAllocate(3 * 1024, 16, b)) // overflow → page 2
	B3D_TEST_ASSERT(allocator.TryAllocate(3 * 1024, 16, c)) // overflow → page 3
	B3D_TEST_ASSERT(allocator.TryAllocate(3 * 1024, 16, d)) // overflow → page 4

	B3D_TEST_ASSERT(a.Heap != b.Heap)
	B3D_TEST_ASSERT(b.Heap != c.Heap)
	B3D_TEST_ASSERT(c.Heap != d.Heap)

	B3D_TEST_ASSERT(allocator.GetLivePageCount() == 4)
	B3D_TEST_ASSERT(backend.LiveHeapCount() == 4)
	B3D_TEST_ASSERT(allocator.GetSparePageCount() == 0)

	// All three retired pages share the same retire-frame index. After completion + flush they
	// drain in one pass and land on the spare list (cap = 4).
	AdvanceCompleteAndFlush(allocator, tracker);
	B3D_TEST_ASSERT(allocator.GetSparePageCount() == 3)
	B3D_TEST_ASSERT(allocator.GetLivePageCount() == 4) // 3 spares + active page.
	B3D_TEST_ASSERT(backend.DestroyCount() == 0)
}

void GpuAllocatorTestSuite::TestLinear_PageRecycledOnFenceComplete()
{
	MockHeapBackend backend;
	MockGpuCompletionTracker tracker;
	LinearAllocator allocator(&backend, &tracker, MakeDefaultLinearConfig(/*pageSize*/ 4 * 1024, /*maxRetained*/ 1));

	MockLocation first;
	B3D_TEST_ASSERT(allocator.TryAllocate(3 * 1024, 16, first))

	// Trigger overflow → page 2.
	MockLocation second;
	B3D_TEST_ASSERT(allocator.TryAllocate(3 * 1024, 16, second))
	B3D_TEST_ASSERT(backend.CreateCount() == 2)

	// Drain the retired page 1 onto spares.
	AdvanceCompleteAndFlush(allocator, tracker);
	B3D_TEST_ASSERT(allocator.GetSparePageCount() == 1)
	B3D_TEST_ASSERT(backend.CreateCount() == 2) // No new heaps created during drain.
	B3D_TEST_ASSERT(backend.DestroyCount() == 0)

	// Trigger another overflow on page 2 — should pull from spares, not call CreateHeap again.
	MockLocation third;
	B3D_TEST_ASSERT(allocator.TryAllocate(3 * 1024, 16, third))
	B3D_TEST_ASSERT(backend.CreateCount() == 2) // Reused the spare.
	B3D_TEST_ASSERT(allocator.GetSparePageCount() == 0)
}

void GpuAllocatorTestSuite::TestLinear_OversizeBypassesPagePool()
{
	MockHeapBackend backend;
	MockGpuCompletionTracker tracker;
	LinearAllocator allocator(&backend, &tracker, MakeDefaultLinearConfig(/*pageSize*/ 8 * 1024, /*maxRetained*/ 4));

	// First populate the active page with a normal allocation, so we can verify the oversize path
	// doesn't disturb the active page state.
	MockLocation small;
	B3D_TEST_ASSERT(allocator.TryAllocate(1024, 16, small))
	const u64 usedBeforeOversize = allocator.GetUsedBytes();
	const IGpuHeap* activeHeapId = small.Heap;

	// Oversize request: PageSize is 8KB; ask for 32KB. Lands in a dedicated heap and is retired
	// immediately — heap stays alive in the deferred-free queue until the fence completes.
	MockLocation oversize;
	B3D_TEST_ASSERT(allocator.TryAllocate(32 * 1024, 16, oversize))
	B3D_TEST_ASSERT(oversize.Size == 32 * 1024)
	B3D_TEST_ASSERT(oversize.Offset == 0)
	B3D_TEST_ASSERT(oversize.Heap != activeHeapId)
	B3D_TEST_ASSERT(allocator.GetLivePageCount() == 2) // active + oversize-pending-drain.

	// Active page was untouched.
	B3D_TEST_ASSERT(allocator.GetUsedBytes() == usedBeforeOversize)

	// Active page is alive; oversize page is alive but retired.
	B3D_TEST_ASSERT(backend.LiveHeapCount() == 2)

	// Drain. Oversize never goes to spares — it destructs.
	AdvanceCompleteAndFlush(allocator, tracker);
	B3D_TEST_ASSERT(allocator.GetSparePageCount() == 0)
	B3D_TEST_ASSERT(backend.DestroyCount() == 1)
	B3D_TEST_ASSERT(backend.LiveHeapCount() == 1)
	B3D_TEST_ASSERT(allocator.GetLivePageCount() == 1) // Only the active page remains.
}

void GpuAllocatorTestSuite::TestLinear_ResetRetiresActivePage()
{
	MockHeapBackend backend;
	MockGpuCompletionTracker tracker;
	LinearAllocator allocator(&backend, &tracker, MakeDefaultLinearConfig());

	MockLocation first;
	B3D_TEST_ASSERT(allocator.TryAllocate(1024, 16, first))
	const IGpuHeap* activeHeapId = first.Heap;

	allocator.FreeAll();
	B3D_TEST_ASSERT(allocator.GetLivePageCount() == 1)        // Page is retired-pending-drain, not destroyed.
	B3D_TEST_ASSERT(allocator.GetUsedBytes() == 0)            // No active page → bump offset reads as 0.
	B3D_TEST_ASSERT(allocator.GetSparePageCount() == 0)

	// Drain — the page lands on the spare list (default MaxRetainedPages = 2).
	AdvanceCompleteAndFlush(allocator, tracker);
	B3D_TEST_ASSERT(allocator.GetSparePageCount() == 1)
	B3D_TEST_ASSERT(backend.DestroyCount() == 0)

	// Next allocate pulls from spares — same page slot, same backend heap.
	MockLocation second;
	B3D_TEST_ASSERT(allocator.TryAllocate(1024, 16, second))
	B3D_TEST_ASSERT(second.Heap == activeHeapId)
	B3D_TEST_ASSERT(allocator.GetSparePageCount() == 0)
	B3D_TEST_ASSERT(backend.CreateCount() == 1) // No new heap created.

	// Reset on an empty allocator (no active page) is a no-op.
	allocator.FreeAll();
	allocator.FreeAll();
	AdvanceCompleteAndFlush(allocator, tracker);
}

void GpuAllocatorTestSuite::TestLinear_SparePageCap()
{
	MockHeapBackend backend;
	MockGpuCompletionTracker tracker;
	LinearAllocator allocator(&backend, &tracker, MakeDefaultLinearConfig(/*pageSize*/ 4 * 1024, /*maxRetained*/ 1));

	// Force three pages alive concurrently within one frame: first two retire on overflow, third stays active.
	MockLocation a, b, c;
	B3D_TEST_ASSERT(allocator.TryAllocate(3 * 1024, 16, a))
	B3D_TEST_ASSERT(allocator.TryAllocate(3 * 1024, 16, b))
	B3D_TEST_ASSERT(allocator.TryAllocate(3 * 1024, 16, c))
	B3D_TEST_ASSERT(allocator.GetLivePageCount() == 3)
	B3D_TEST_ASSERT(backend.CreateCount() == 3)

	// Drain. Two pages retire; cap is 1, so the second drained page destructs.
	AdvanceCompleteAndFlush(allocator, tracker);
	B3D_TEST_ASSERT(allocator.GetSparePageCount() == 1)
	B3D_TEST_ASSERT(backend.DestroyCount() == 1)
	B3D_TEST_ASSERT(allocator.GetLivePageCount() == 2) // 1 spare + active.
}

void GpuAllocatorTestSuite::TestLinear_FreeIsNoop()
{
	MockHeapBackend backend;
	MockGpuCompletionTracker tracker;
	LinearAllocator allocator(&backend, &tracker, MakeDefaultLinearConfig());

	MockLocation first;
	B3D_TEST_ASSERT(allocator.TryAllocate(1024, 16, first))
	const IGpuHeap* originalHeapId = first.Heap;
	const u64 usedBefore = allocator.GetUsedBytes();
	const u32 livePagesBefore = allocator.GetLivePageCount();

	// Free is a no-op for the linear allocator: the location is reset by the base, but the active
	// page's bump offset is unchanged and no retire entry is queued.
	allocator.Free(first);
	B3D_TEST_ASSERT(!first.IsValid())
	B3D_TEST_ASSERT(allocator.GetUsedBytes() == usedBefore)
	B3D_TEST_ASSERT(allocator.GetLivePageCount() == livePagesBefore)

	// Subsequent allocate lands AFTER the freed range and on the same active page — proves no recycling happened.
	MockLocation second;
	B3D_TEST_ASSERT(allocator.TryAllocate(1024, 16, second))
	B3D_TEST_ASSERT(second.Offset >= usedBefore)
	B3D_TEST_ASSERT(second.Heap == originalHeapId)
	B3D_TEST_ASSERT(allocator.GetLivePageCount() == 1)

	// Flush should drain nothing — the queue was never touched by Free.
	AdvanceCompleteAndFlush(allocator, tracker);
	B3D_TEST_ASSERT(allocator.GetSparePageCount() == 0)
	B3D_TEST_ASSERT(backend.DestroyCount() == 0)
}

void GpuAllocatorTestSuite::TestLinear_FreeImmediateOnSharedPageIsNoop()
{
	MockHeapBackend backend;
	MockGpuCompletionTracker tracker;
	LinearAllocator allocator(&backend, &tracker, MakeDefaultLinearConfig());

	// Three allocations share one active page. With a naive implementation, FreeAndReclaim(a) would
	// recycle/destroy the page — invalidating b and c. The linear allocator must treat per-allocation
	// FreeAndReclaim as a no-op so peers remain valid.
	MockLocation a, b, c;
	B3D_TEST_ASSERT(allocator.TryAllocate(1024, 16, a))
	B3D_TEST_ASSERT(allocator.TryAllocate(1024, 16, b))
	B3D_TEST_ASSERT(allocator.TryAllocate(1024, 16, c))
	B3D_TEST_ASSERT(a.Heap == b.Heap && b.Heap == c.Heap)

	const u64 usedBefore = allocator.GetUsedBytes();
	const u32 livePagesBefore = allocator.GetLivePageCount();
	const u32 destroyCountBefore = backend.DestroyCount();

	// FreeAndReclaim one of the three. The page must NOT be recycled — peers are still live.
	allocator.FreeAndReclaim(a);
	B3D_TEST_ASSERT(!a.IsValid())
	B3D_TEST_ASSERT(b.IsValid())
	B3D_TEST_ASSERT(c.IsValid())
	B3D_TEST_ASSERT(allocator.GetUsedBytes() == usedBefore)        // Bump offset unchanged.
	B3D_TEST_ASSERT(allocator.GetLivePageCount() == livePagesBefore)
	B3D_TEST_ASSERT(allocator.GetSparePageCount() == 0)              // No spurious spare entry.
	B3D_TEST_ASSERT(backend.DestroyCount() == destroyCountBefore)    // No spurious destroy.

	// Subsequent allocate keeps bumping past c's range — proves the page is still being used as the active page.
	MockLocation d;
	B3D_TEST_ASSERT(allocator.TryAllocate(1024, 16, d))
	B3D_TEST_ASSERT(d.Heap == b.Heap)
	B3D_TEST_ASSERT(d.Offset >= c.Offset + c.Size)

	// Free the rest with FreeAndReclaim — still all no-ops; page is reclaimed only via FreeAll + drain.
	allocator.FreeAndReclaim(b);
	allocator.FreeAndReclaim(c);
	allocator.FreeAndReclaim(d);
	B3D_TEST_ASSERT(allocator.GetLivePageCount() == livePagesBefore)
	B3D_TEST_ASSERT(allocator.GetSparePageCount() == 0)
	B3D_TEST_ASSERT(backend.DestroyCount() == destroyCountBefore)
}

void GpuAllocatorTestSuite::TestLinear_SharedPoolReusedAcrossAllocators()
{
	MockHeapBackend backend;
	MockGpuCompletionTracker tracker;

	// One device-owned pool shared by two independent ThreadUnsafe allocators (each models a separate
	// context/operation). The pool page size must match the allocators' page size.
	LinearPagePool pool(&backend, MakeDefaultPagePoolConfig(/*pageSize*/ 4 * 1024, /*maxRetained*/ 4));

	// Allocator-local MaxRetainedPages is ignored when a pool is supplied — the pool owns the bound.
	LinearAllocator a(&backend, &tracker, MakeDefaultLinearConfig(/*pageSize*/ 4 * 1024, /*maxRetained*/ 0), &pool);
	LinearAllocator b(&backend, &tracker, MakeDefaultLinearConfig(/*pageSize*/ 4 * 1024, /*maxRetained*/ 0), &pool);

	// Each allocator fills then overflows its first page, retiring one page apiece. Four heaps total
	// (two retired-pending-drain + two active); nothing has reached the pool yet.
	MockLocation a0, a1, b0, b1;
	B3D_TEST_ASSERT(a.TryAllocate(3 * 1024, 16, a0))
	B3D_TEST_ASSERT(a.TryAllocate(3 * 1024, 16, a1)) // overflow → retire a's first page
	B3D_TEST_ASSERT(b.TryAllocate(3 * 1024, 16, b0))
	B3D_TEST_ASSERT(b.TryAllocate(3 * 1024, 16, b1)) // overflow → retire b's first page
	B3D_TEST_ASSERT(backend.CreateCount() == 4)
	B3D_TEST_ASSERT(pool.GetSparePageCount() == 0)

	// Both retired pages were stamped at marker 0; completing the marker and flushing each allocator
	// returns those pages to the shared pool (pooled, not destroyed).
	AdvanceCompleteAndFlush(a, tracker);
	AdvanceCompleteAndFlush(b, tracker);
	B3D_TEST_ASSERT(pool.GetSparePageCount() == 2)
	B3D_TEST_ASSERT(pool.GetCommittedBytes() == 2ull * 4 * 1024)
	B3D_TEST_ASSERT(backend.DestroyCount() == 0)

	// A fresh overflow now draws from the shared pool instead of calling CreateHeap. Both allocators
	// pull a pooled page — including pages the *other* allocator drained — proving the pool is shared.
	MockLocation a2, b2;
	B3D_TEST_ASSERT(a.TryAllocate(3 * 1024, 16, a2)) // overflow → pull from pool
	B3D_TEST_ASSERT(b.TryAllocate(3 * 1024, 16, b2)) // overflow → pull from pool
	B3D_TEST_ASSERT(backend.CreateCount() == 4)      // both reused pooled pages, no new heaps
	B3D_TEST_ASSERT(pool.GetSparePageCount() == 0)
}

void GpuAllocatorTestSuite::TestLinear_SharedPoolRespectsBound()
{
	MockHeapBackend backend;
	MockGpuCompletionTracker tracker;

	// Pool retains at most two warm pages regardless of how many drain into it.
	LinearPagePool pool(&backend, MakeDefaultPagePoolConfig(/*pageSize*/ 4 * 1024, /*maxRetained*/ 2));
	LinearAllocator allocator(&backend, &tracker, MakeDefaultLinearConfig(/*pageSize*/ 4 * 1024, /*maxRetained*/ 0), &pool);

	// Four pages alive within one frame: three overflow-retired + the active fourth.
	MockLocation a, b, c, d;
	B3D_TEST_ASSERT(allocator.TryAllocate(3 * 1024, 16, a))
	B3D_TEST_ASSERT(allocator.TryAllocate(3 * 1024, 16, b))
	B3D_TEST_ASSERT(allocator.TryAllocate(3 * 1024, 16, c))
	B3D_TEST_ASSERT(allocator.TryAllocate(3 * 1024, 16, d))
	B3D_TEST_ASSERT(backend.CreateCount() == 4)

	// Retire the active page too, so all four are pending drain against the same marker.
	allocator.FreeAll();

	// Drain: the pool keeps two warm spares; the remaining two exceed the bound and are destroyed.
	AdvanceCompleteAndFlush(allocator, tracker);
	B3D_TEST_ASSERT(pool.GetSparePageCount() == 2)
	B3D_TEST_ASSERT(backend.DestroyCount() == 2)
	B3D_TEST_ASSERT(allocator.GetLivePageCount() == 0) // every page handed back to the pool or destroyed
}

void GpuAllocatorTestSuite::TestLinear_SharedPoolDrainsOnlyAfterMarkerComplete()
{
	MockHeapBackend backend;
	MockGpuCompletionTracker tracker;
	LinearPagePool pool(&backend, MakeDefaultPagePoolConfig(/*pageSize*/ 4 * 1024, /*maxRetained*/ 4));
	LinearAllocator allocator(&backend, &tracker, MakeDefaultLinearConfig(/*pageSize*/ 4 * 1024, /*maxRetained*/ 0), &pool);

	// Advance the device marker so the retire below is stamped at a known, non-zero value — the value
	// the operation's *next* submit would signal, i.e. GetCurrentMarker() at retire time.
	tracker.AdvanceFrame(); // marker → 1
	tracker.AdvanceFrame(); // marker → 2
	const u64 retireMarker = tracker.GetCurrentMarker();
	B3D_TEST_ASSERT(retireMarker == 2)

	// Allocate then overflow: the rotated-out page is stamped with retireMarker.
	MockLocation first, second;
	B3D_TEST_ASSERT(allocator.TryAllocate(3 * 1024, 16, first))
	B3D_TEST_ASSERT(allocator.TryAllocate(3 * 1024, 16, second))

	// Completing every marker strictly below the stamp must NOT drain the page (off-by-one guard): the
	// page is still in flight until its own marker signals.
	tracker.MarkFrameComplete(retireMarker - 1);
	allocator.ReclaimUnused(); // drains completed retired entries
	B3D_TEST_ASSERT(pool.GetSparePageCount() == 0)

	// Completing exactly the stamped marker drains it to the pool.
	tracker.MarkFrameComplete(retireMarker);
	allocator.ReclaimUnused();
	B3D_TEST_ASSERT(pool.GetSparePageCount() == 1)
	B3D_TEST_ASSERT(backend.DestroyCount() == 0)
}

void GpuAllocatorTestSuite::TestLinear_SharedPoolForceDrainReturnsPages()
{
	MockHeapBackend backend;
	MockGpuCompletionTracker tracker;
	LinearPagePool pool(&backend, MakeDefaultPagePoolConfig(/*pageSize*/ 4 * 1024, /*maxRetained*/ 4));

	{
		LinearAllocator allocator(&backend, &tracker, MakeDefaultLinearConfig(/*pageSize*/ 4 * 1024, /*maxRetained*/ 0), &pool);

		// Retire a page but never complete its marker — models a page whose submit never signals (e.g.
		// allocate-without-submit, or teardown reached before the fence catches up).
		MockLocation first, second;
		B3D_TEST_ASSERT(allocator.TryAllocate(3 * 1024, 16, first))
		B3D_TEST_ASSERT(allocator.TryAllocate(3 * 1024, 16, second))

		// Non-blocking reclaim drains nothing — the marker never completed.
		allocator.ReclaimUnused(); // drains completed retired entries
		B3D_TEST_ASSERT(pool.GetSparePageCount() == 0)

		// Force drain (what a blocking WaitAndReclaim / the destructor performs) reclaims the retired
		// page regardless of fence and returns it to the shared pool. The guard that used to destroy
		// force-drained pages was removed — destroying still-in-flight memory is UB either way, and the
		// blocking-reclaim contract already requires the caller to have waited for in-flight work.
		allocator.ReclaimUnused(true);
		B3D_TEST_ASSERT(pool.GetSparePageCount() == 1) // retired page returned to the pool
		B3D_TEST_ASSERT(backend.DestroyCount() == 0)
	}

	// The active page was never retired, so the destructor destroys it directly rather than pooling it;
	// only the force-drained retired page made it back to the pool.
	B3D_TEST_ASSERT(pool.GetSparePageCount() == 1)
	B3D_TEST_ASSERT(backend.DestroyCount() == 1)
}

void GpuAllocatorTestSuite::TestAllocatorIdentity_FreeRoutesByCarriedAllocator()
{
	// 1A.2 made the allocation carry its own free handle: GpuResourceLocation::Allocator is an
	// IGpuAllocator* stamped at TryAllocate. This test proves a caller can free purely through that
	// carried base pointer — with no static knowledge of the concrete strategy — and the call
	// dispatches to the right allocator: a TLSF free reclaims the slot, a linear free is a per-
	// allocation no-op. Separate backends keep each allocator's byte/page accounting independent.
	MockHeapBackend tlsfBackend;
	MockHeapBackend linearBackend;
	MockGpuCompletionTracker tracker;

	// TLSF in ResourceLifecycle so FreeAndReclaim reclaims synchronously and used-bytes is observable
	// without a frame-tracker dance.
	TlsfAllocator::Configuration tlsfConfig = MakeDefaultTlsfConfig();
	tlsfConfig.DeferralMode = GpuAllocatorFreeDeferralMode::ResourceLifecycle;
	TlsfAllocator tlsfAllocator(&tlsfBackend, &tracker, tlsfConfig);

	LinearAllocator linearAllocator(&linearBackend, &tracker, MakeDefaultLinearConfig());

	MockLocation tlsfLocation;
	B3D_TEST_ASSERT(tlsfAllocator.TryAllocate(2048, 64, tlsfLocation))

	// Two linear allocations share one active page so the no-op free can be observed against a peer.
	MockLocation linearFirst, linearSecond;
	B3D_TEST_ASSERT(linearAllocator.TryAllocate(1024, 16, linearFirst))
	B3D_TEST_ASSERT(linearAllocator.TryAllocate(1024, 16, linearSecond))
	B3D_TEST_ASSERT(linearFirst.Heap == linearSecond.Heap)

	// Identity: each location points back at the concrete allocator that produced it, upcast to the
	// shared IGpuAllocator interface. The two carried handles are distinct.
	B3D_TEST_ASSERT(tlsfLocation.Allocator == static_cast<IGpuAllocator*>(&tlsfAllocator))
	B3D_TEST_ASSERT(linearFirst.Allocator == static_cast<IGpuAllocator*>(&linearAllocator))
	B3D_TEST_ASSERT(linearSecond.Allocator == static_cast<IGpuAllocator*>(&linearAllocator))
	B3D_TEST_ASSERT(tlsfLocation.Allocator != linearFirst.Allocator)

	const u64 tlsfUsedBeforeFree = tlsfAllocator.GetUsedBytes();
	B3D_TEST_ASSERT(tlsfUsedBeforeFree >= 2048)

	// Free the TLSF slot through ONLY the carried base pointer — no static knowledge of the concrete
	// type at the call site. Dispatch reaches the TLSF strategy and (ResourceLifecycle) reclaims it.
	IGpuAllocator* tlsfHandle = tlsfLocation.Allocator;
	tlsfHandle->FreeAndReclaim(tlsfLocation);
	B3D_TEST_ASSERT(!tlsfLocation.IsValid())
	B3D_TEST_ASSERT(tlsfAllocator.GetUsedBytes() == 0)

	// Free one linear slot through its carried base pointer. Dispatch reaches the linear strategy,
	// whose per-allocation FreeAndReclaim is a no-op: the shared page is not recycled or destroyed and
	// the peer slot stays valid.
	const u32 linearLivePagesBefore = linearAllocator.GetLivePageCount();
	const u32 linearDestroyBefore = linearBackend.DestroyCount();
	IGpuAllocator* linearHandle = linearFirst.Allocator;
	linearHandle->FreeAndReclaim(linearFirst);
	B3D_TEST_ASSERT(!linearFirst.IsValid())
	B3D_TEST_ASSERT(linearSecond.IsValid())
	B3D_TEST_ASSERT(linearAllocator.GetLivePageCount() == linearLivePagesBefore)
	B3D_TEST_ASSERT(linearBackend.DestroyCount() == linearDestroyBefore)

	// A subsequent linear allocation still bumps past the peer on the same page, confirming the no-op
	// free left the active page untouched.
	MockLocation linearThird;
	B3D_TEST_ASSERT(linearAllocator.TryAllocate(1024, 16, linearThird))
	B3D_TEST_ASSERT(linearThird.Heap == linearSecond.Heap)
	B3D_TEST_ASSERT(linearThird.Offset >= linearSecond.Offset + linearSecond.Size)
}

void GpuAllocatorTestSuite::TestAllocatorIdentity_DefraggedAllocationFreesThroughCarriedAllocator()
{
	// After defragmentation relocates an allocation, the replacement Location the allocator supplies to
	// MoveAllocation must itself carry the producing allocator (Location.Allocator), so a moved resource
	// can still be freed through its (new) carried handle. Mirrors the multi-heap drain setup: 6 holders
	// fill heap 0 then spill to heap 1, two heap-0 slots are vacated, and Defrag migrates the heap-1
	// survivors down into heap 0.
	MockHeapBackend backend;
	MockGpuCompletionTracker tracker;

	TlsfAllocator::Configuration configuration = MakeDefaultTlsfConfig();
	configuration.InitialHeapSize = 64 * 1024;
	configuration.MaxHeapSize = 64 * 1024;
	configuration.GrowthFactor = 1;
	configuration.MaxEmptyHeapCount = 0;
	TlsfAllocator allocator(&backend, &tracker, configuration);

	struct Holder { MockResource Resource; MockLocation Location; };
	const u32 kAllocCount = 6;
	const u64 kAllocSize = 16 * 1024;

	Vector<TUnique<Holder>> holders;
	for (u32 holderIndex = 0; holderIndex < kAllocCount; holderIndex++)
	{
		auto holder = B3DMakeUnique<Holder>();
		holder->Resource.LocationPtr = &holder->Location;
		B3D_TEST_ASSERT(allocator.TryAllocate(kAllocSize, 16, GpuResourceKind::Linear, &holder->Resource, holder->Location))
		// Every fresh allocation carries the producing allocator.
		B3D_TEST_ASSERT(holder->Location.Allocator == static_cast<IGpuAllocator*>(&allocator))
		holders.push_back(std::move(holder));
	}

	B3D_TEST_ASSERT(allocator.GetHeapCount() == 2)
	const u32 heap0Slot = holders[0]->Location.AllocatorData0;
	const u32 heap1Slot = holders[4]->Location.AllocatorData0;
	B3D_TEST_ASSERT(heap0Slot != heap1Slot)

	// Vacate room in heap 0 so heap 1's survivors can migrate down.
	tracker.AdvanceFrame();
	allocator.Free(holders[0]->Location);
	allocator.Free(holders[1]->Location);
	tracker.MarkAllFramesComplete();
	allocator.ReclaimUnused(false);

	const TlsfAllocator::DefragmentationStats stats = allocator.Defrag(NullCommandBuffer());
	B3D_TEST_ASSERT(stats.MovesCompleted == 2)

	// The two relocated survivors now live in heap 0, and their replacement locations still carry the
	// same producing allocator — the relocation didn't orphan the free handle.
	for (u32 holderIndex = 4; holderIndex < kAllocCount; holderIndex++)
	{
		B3D_TEST_ASSERT(holders[holderIndex]->Location.AllocatorData0 == heap0Slot)
		B3D_TEST_ASSERT(holders[holderIndex]->Location.Allocator == static_cast<IGpuAllocator*>(&allocator))
	}

	// Drain the defrag-retired heap-1 source slots; with MaxEmptyHeapCount=0 the emptied heap 1 is released.
	tracker.AdvanceFrame();
	tracker.MarkAllFramesComplete();
	allocator.ReclaimUnused(false);
	B3D_TEST_ASSERT(allocator.GetHeapCount() == 1)

	// Free every still-live allocation (indices 2..5, including the two relocated by defrag) through ONLY
	// its carried Location.Allocator handle, then drain. Routing through the carried pointer must reclaim
	// every slot — used bytes returns to zero.
	tracker.AdvanceFrame();
	for (u32 holderIndex = 2; holderIndex < kAllocCount; holderIndex++)
	{
		MockLocation& location = holders[holderIndex]->Location;
		B3D_TEST_ASSERT(location.IsValid())
		location.Allocator->Free(location);
	}
	tracker.MarkAllFramesComplete();
	allocator.ReclaimUnused(false);
	B3D_TEST_ASSERT(allocator.GetUsedBytes() == 0)
}

