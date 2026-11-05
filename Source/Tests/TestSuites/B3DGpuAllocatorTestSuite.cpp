//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DGpuAllocatorTestSuite.h"
#include "CoreObject/B3DRenderThread.h"
#include "GpuBackend/B3DGpuBackend.h"
#include "GpuBackend/B3DGpuDevice.h"
#include "GpuBackend/B3DGpuDeviceCapabilities.h"
#include "GpuBackend/B3DGpuTimelineFence.h"
#include "GpuBackend/B3DGpuCompletionTracker.h"
#include "GpuBackend/B3DGpuCommandBuffer.h"
#include "GpuBackend/B3DGpuCommandBufferPoolRing.h"
#include "GpuBackend/Allocators/B3DGpuAllocator.h"
#include "GpuBackend/Allocators/B3DGpuLinearAllocator.h"
#include "GpuBackend/Allocators/B3DGpuTlsfAllocator.h"
#include "GpuBackend/Allocators/B3DGpuAliasingAllocator.h"
#include "GpuBackend/B3DGpuSubmissionTimeline.h"
#include "GpuBackend/Allocators/B3DGpuResource.h"
#include "Renderer/B3DRenderer.h"
#include "Utility/B3DGpuBarrierTestUtility.h"

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
	B3D_ADD_TEST(GpuAllocatorTestSuite::TestFenceTracker_PerQueueFences)
	B3D_ADD_TEST(GpuAllocatorTestSuite::TestTlsf_ContractAndInitialState)
	B3D_ADD_TEST(GpuAllocatorTestSuite::TestTlsf_SingleAllocateFree)
	B3D_ADD_TEST(GpuAllocatorTestSuite::TestTlsf_FailsSoftOnHeapCreationFailure)
	B3D_ADD_TEST(GpuAllocatorTestSuite::TestTlsf_NonOverlappingAlignedOffsets)
	B3D_ADD_TEST(GpuAllocatorTestSuite::TestTlsf_CoalesceAllOrders)
	B3D_ADD_TEST(GpuAllocatorTestSuite::TestTlsf_LargeAlignmentSplitsLeadingPadding)
	B3D_ADD_TEST(GpuAllocatorTestSuite::TestTlsf_HeapGrowthAndEmptyRelease)
	B3D_ADD_TEST(GpuAllocatorTestSuite::TestTlsf_OversizedAllocationGetsDedicatedHeap)
	B3D_ADD_TEST(GpuAllocatorTestSuite::TestTlsf_RandomStressNoLeak)
	B3D_ADD_TEST(GpuAllocatorTestSuite::TestTlsf_Defrag_DrainsHighestHeap)
	B3D_ADD_TEST(GpuAllocatorTestSuite::TestTlsf_Defrag_SingleHeapWithinHeapCompaction)
	B3D_ADD_TEST(GpuAllocatorTestSuite::TestTlsf_Defrag_RespectsBudget)
	B3D_ADD_TEST(GpuAllocatorTestSuite::TestTlsf_Defrag_SkipsUnownedAndUnmovable)
	B3D_ADD_TEST(GpuAllocatorTestSuite::TestTlsf_Defrag_CancelReleasesDestinations)
	B3D_ADD_TEST(GpuAllocatorTestSuite::TestTlsf_Defrag_PlanDescribesMoves)
	B3D_ADD_TEST(GpuAllocatorTestSuite::TestTlsf_Defrag_KeepsAlignment)
	B3D_ADD_TEST(GpuAllocatorTestSuite::TestTlsf_ResourceLifecyclePolicy_FreesImmediately)
	B3D_ADD_TEST(GpuAllocatorTestSuite::TestTlsf_FrameTrackerPolicy_DefersAcrossFrames)
	B3D_ADD_TEST(GpuAllocatorTestSuite::TestTlsf_Defrag_ResourceLifecycleReleasesSources)
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
	B3D_ADD_TEST(GpuAllocatorTestSuite::TestSubmissionTimeline)
	B3D_ADD_TEST(GpuAllocatorTestSuite::TestAliasing_SameQueueReuse)
	B3D_ADD_TEST(GpuAllocatorTestSuite::TestAliasing_CrossQueueEligibility)
	B3D_ADD_TEST(GpuAllocatorTestSuite::TestAliasing_InheritedTags)
	B3D_ADD_TEST(GpuAllocatorTestSuite::TestAliasing_Passes)
	B3D_ADD_TEST(GpuAllocatorTestSuite::TestAliasing_ConcurrentScopes)
	B3D_ADD_TEST(GpuAllocatorTestSuite::TestAliasing_ExactClaims)
	B3D_ADD_TEST(GpuAllocatorTestSuite::TestAliasing_SegmentsAndGranularity)
	B3D_ADD_TEST(GpuAllocatorTestSuite::TestTransientHeapPool_ReleaseAndStatistics)
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
			// host storage, never the object) so allocation identity-by-pointer stays valid across the test.
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

	using MockAllocation = GpuAllocation;
	using TlsfAllocator = TGpuTlsfAllocator<MockHeapBackend>;

	/**
	 * Helper: advances the frame, frees, marks complete, flushes — drives a single retire entry
	 * to completion. Only meaningful when the allocator is configured with @c FrameTracker.
	 */
	void FreeAndDrain(TlsfAllocator& allocator, MockGpuCompletionTracker& tracker, MockAllocation& allocation)
	{
		tracker.AdvanceFrame();
		allocator.Free(allocation);
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
		// having to round-trip through the public Free path (which auto-resets the allocation).
		using Base::RetireAllocation;

		MockAllocator(MockHeapBackend* backend, MockGpuCompletionTracker* tracker)
			: Base(backend, tracker)
		{}

		bool TryAllocateImpl(u64 /*size*/, u32 /*alignment*/, MockAllocation& /*out*/)
		{
			return false;
		}

		void FreeImpl(MockAllocation& allocation)
		{
			RetireAllocation(allocation);
		}

		void FreeAndReclaimImpl(u32 allocatorData0, u32 allocatorData1)
		{
			FreedSlots.push_back({ allocatorData0, allocatorData1 });
		}

		Vector<FreedSlot> FreedSlots;
	};

	/** Allocation registered with an owner, the way a texture or buffer holds its memory. The holder's address is the owner. */
	struct OwnedAllocation
	{
		MockAllocation Allocation;

		/** Returns the owner the holder registers with the allocator. */
		GpuAllocationOwner GetOwner() { return GpuAllocationOwner::FromOpaque(this); }
	};

	/** Allocates @p size bytes for @p holder and registers the holder as the owner of the allocation. */
	bool AllocateOwned(IGpuAllocator& allocator, u64 size, u32 alignment, OwnedAllocation& holder)
	{
		if (!allocator.TryAllocate(size, alignment, holder.Allocation))
			return false;

		allocator.SetAllocationOwner(holder.Allocation, holder.GetOwner());
		return true;
	}

	/** Unregisters the owner of @p holder's allocation and frees the allocation. */
	void FreeOwned(IGpuAllocator& allocator, OwnedAllocation& holder)
	{
		allocator.SetAllocationOwner(holder.Allocation, GpuAllocationOwner());
		allocator.Free(holder.Allocation);
	}

	/** Plans moves of the owned allocations that @p fnIsMovable accepts (all by default), within @p budget. */
	TArray<GpuAllocationMove> PlanMoves(IGpuAllocator& allocator, const GpuDefragmentationBudget& budget = GpuDefragmentationBudget(),
		const Function<bool(GpuAllocationOwner)>& fnIsMovable = [](GpuAllocationOwner) { return true; })
	{
		TArray<GpuAllocationMove> moves;
		allocator.PlanDefragmentation(budget, fnIsMovable, moves);
		return moves;
	}

	/** Commits @p moves the way GpuMemoryDefragmenter does: the owner moves to the destination and the source is freed. */
	void CommitMoves(IGpuAllocator& allocator, const TArray<GpuAllocationMove>& moves)
	{
		for (const GpuAllocationMove& move : moves)
		{
			OwnedAllocation& holder = *static_cast<OwnedAllocation*>(move.Owner.ToOpaque());
			FreeOwned(allocator, holder);

			holder.Allocation = move.Destination;
			allocator.SetAllocationOwner(holder.Allocation, holder.GetOwner());
		}
	}

	/** Cancels @p moves the way GpuMemoryDefragmenter does, by freeing their destinations. */
	void CancelMoves(IGpuAllocator& allocator, const TArray<GpuAllocationMove>& moves)
	{
		for (const GpuAllocationMove& move : moves)
		{
			GpuAllocation destination = move.Destination;
			allocator.Free(destination);
		}
	}
}

void GpuAllocatorTestSuite::TestGpuAllocatorContract()
{
	// Compile-time proof: the trait-check macro accepts a valid backend.
	B3D_STATIC_ASSERT_HEAP_BACKEND_IS_VALID(MockHeapBackend);

	// Counter-example for the macro is intentionally left commented out — uncommenting it should produce
	// six focused diagnostics (one per missing requirement) rather than a wall of template-expansion errors:
	//   struct BrokenBackend {};
	//   B3D_STATIC_ASSERT_HEAP_BACKEND_IS_VALID(BrokenBackend);

	// The allocation must remain a POD so render proxies can copy/move it without ceremony. If a future
	// change introduces a non-trivial member, these asserts catch it before consumer code regresses.
	static_assert(std::is_standard_layout<MockAllocation>::value, "GpuAllocation must remain standard-layout.");
	static_assert(std::is_trivially_copyable<MockAllocation>::value, "GpuAllocation must remain trivially copyable.");

	MockHeapBackend backend;
	MockGpuCompletionTracker tracker;
	MockAllocator allocator(&backend, &tracker);

	// Exercise the full public surface so the linker resolves every entry point.
	MockAllocation allocation = MockAllocation::CreatePending(allocator);
	B3D_TEST_ASSERT(allocation.IsPending())
	B3D_TEST_ASSERT(!allocation.HasMemory())

	allocation.Heap = backend.CreateHeap(256, MockHeapBackend::HeapCreateInformation());
	allocation.Size = 256;
	allocation.AllocatorData0 = 5;
	allocation.AllocatorData1 = 7;

	B3D_TEST_ASSERT(allocation.IsOwned())
	B3D_TEST_ASSERT(allocation.HasMemory())

	// Real-world ordering with the "stamp with latest" pattern: a touching submit advances the device
	// counter first, then the deallocate stamps the retire entry with the current latest value. Mirror
	// that here — submit, then retire.
	const u64 submittedIndex = tracker.AdvanceFrame();
	allocator.RetireAllocation(allocation);

	tracker.MarkFrameComplete(submittedIndex);

	allocator.ReclaimUnused();
	B3D_TEST_ASSERT(allocator.FreedSlots.size() == 1)
	B3D_TEST_ASSERT(allocator.FreedSlots[0].AllocatorData0 == 5)
	B3D_TEST_ASSERT(allocator.FreedSlots[0].AllocatorData1 == 7)

	allocation.Reset();
	B3D_TEST_ASSERT(!allocation.IsOwned())
	B3D_TEST_ASSERT(allocation.AllocatorData0 == 0)
	B3D_TEST_ASSERT(allocation.AllocatorData1 == 0)
}

void GpuAllocatorTestSuite::TestGpuAllocatorDeferredDelete()
{
	// Case 1: FIFO drain stops at the first incomplete entry.
	{
		MockHeapBackend backend;
		MockGpuCompletionTracker tracker;
		MockAllocator allocator(&backend, &tracker);

		// Each allocation carries a distinct slot identity so the test can match drained entries back to the
		// retire calls without relying on pointer equality.
		MockAllocation allocationA, allocationB, allocationC;
		allocationA.AllocatorData0 = 1; allocationA.AllocatorData1 = 10;
		allocationB.AllocatorData0 = 2; allocationB.AllocatorData1 = 20;
		allocationC.AllocatorData0 = 3; allocationC.AllocatorData1 = 30;

		// Real-world ordering with the "stamp with latest" pattern: a touching submit advances the device
		// counter first, then the retire stamps with the new latest value. The three submits assign indices
		// 1, 2, 3 and the retires inherit them.
		const u64 indexA = tracker.AdvanceFrame(); allocator.RetireAllocation(allocationA);
		const u64 indexB = tracker.AdvanceFrame(); allocator.RetireAllocation(allocationB);
		const u64 indexC = tracker.AdvanceFrame(); allocator.RetireAllocation(allocationC);

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

		MockAllocation allocationA, allocationB, allocationC;
		allocationA.AllocatorData0 = 1; allocationA.AllocatorData1 = 10;
		allocationB.AllocatorData0 = 2; allocationB.AllocatorData1 = 20;
		allocationC.AllocatorData0 = 3; allocationC.AllocatorData1 = 30;

		tracker.AdvanceFrame(); allocator.RetireAllocation(allocationA);
		tracker.AdvanceFrame(); allocator.RetireAllocation(allocationB);
		tracker.AdvanceFrame(); allocator.RetireAllocation(allocationC);

		// No Signal() call — submissions remain incomplete.
		allocator.ReclaimUnused(true);

		B3D_TEST_ASSERT(allocator.FreedSlots.size() == 3)
		B3D_TEST_ASSERT(allocator.FreedSlots[0].AllocatorData0 == 1)
		B3D_TEST_ASSERT(allocator.FreedSlots[1].AllocatorData0 == 2)
		B3D_TEST_ASSERT(allocator.FreedSlots[2].AllocatorData0 == 3)
	}

	// Case 4: Public Free path — captures the slot identity by value, resets the caller's allocation,
	// and proves the queued snapshot is independent of the caller's storage. This is the property that
	// makes the deferred-delete queue safe against the resource being destroyed before its submission
	// completes.
	{
		MockHeapBackend backend;
		MockGpuCompletionTracker tracker;
		MockAllocator allocator(&backend, &tracker);

		MockAllocation allocation;
		allocation.Allocator = &allocator;
		allocation.AllocatorData0 = 42;
		allocation.AllocatorData1 = 99;

		const u64 retireIndex = tracker.AdvanceFrame();
		allocator.Free(allocation);

		// Auto-Reset on the caller's allocation: the resource sees an invalid handle the moment Free
		// returns, even though the queue still holds a snapshot of the slot.
		B3D_TEST_ASSERT(!allocation.IsOwned())
		B3D_TEST_ASSERT(allocation.AllocatorData0 == 0)
		B3D_TEST_ASSERT(allocation.AllocatorData1 == 0)

		// Mutate the caller's storage post-Free. The retired-queue snapshot must remain unaffected,
		// which is what would let a resource destructor run between Free and the submission signal.
		allocation.AllocatorData0 = 7;
		allocation.AllocatorData1 = 8;

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
	TShared<GpuCommandBufferPool> pool = GpuBarrierTestUtility::CreateCommandBufferPool(*device);
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

void GpuAllocatorTestSuite::TestFenceTracker_PerQueueFences()
{
	TShared<GpuDevice> device = GetActiveDevice();
	if (device == nullptr || !IsRealBackend(*device) || device->GetQueueCount(GQT_GRAPHICS) == 0)
		return;

	// Same backend-support and degraded-mode skips as TestUserCreatedFence_ExplicitSignal
	const TShared<GpuTimelineFence> probeFence = device->CreateTimelineFence();
	if (probeFence == nullptr || probeFence->IsSignaled(7))
		return;

	GpuFenceCompletionTracker tracker(*device);
	B3D_TEST_ASSERT(tracker.GetCurrentMarker() == 1)
	B3D_TEST_ASSERT(tracker.GetLastSubmittedMarker() == 0)
	B3D_TEST_ASSERT(tracker.IsMarkerComplete(1)) // nothing pending

	// Submit through a context that borrows the tracker, so the submissions are tagged only by this test.
	const TShared<GpuWorkContext> gpuContext = GpuWorkContext::Create(*device, tracker);

	TInlineArray<TShared<GpuCommandBufferPool>, GQT_COUNT + 1> pools;
	TInlineArray<GpuTimelineFenceAndValue, GQT_COUNT + 1> submits;
	const auto fnSubmitEmpty = [&device, &tracker, &gpuContext, &pools, &submits](GpuQueueType queueType)
	{
		const TShared<GpuCommandBufferPool> pool = GpuBarrierTestUtility::CreateCommandBufferPool(*device, queueType);
		pools.Add(pool);

		GpuSubmissionInformation information;
		information.CommandBuffer = pool->Create(GpuCommandBufferCreateInformation::Create("FenceTrackerTestCB"));
		information.SyncMask = GpuQueueMask::kNone;
		information.SignalFences.Add(tracker.NotifyWillSubmit(GpuQueueId(queueType, 0)));
		submits.Add(information.SignalFences.Back());

		gpuContext->SubmitCommandBuffer(information);
	};

	// One submission per queue type the device exposes, then a second one on graphics
	for (u32 queueTypeIndex = 0; queueTypeIndex < GQT_COUNT; queueTypeIndex++)
	{
		if (device->GetQueueCount((GpuQueueType)queueTypeIndex) > 0)
			fnSubmitEmpty((GpuQueueType)queueTypeIndex);
	}

	const u64 queueSubmitCount = submits.Size();
	fnSubmitEmpty(GQT_GRAPHICS);

	// Markers are shared and strictly increasing; fences are distinct per queue and reused on resubmit
	for (u64 submitIndex = 0; submitIndex < submits.Size(); submitIndex++)
	{
		B3D_TEST_ASSERT(submits[submitIndex].Value == submitIndex + 1)
		B3D_TEST_ASSERT(submits[submitIndex].Fence != nullptr)
	}

	for (u64 submitIndex = 1; submitIndex < queueSubmitCount; submitIndex++)
	{
		for (u64 otherIndex = 0; otherIndex < submitIndex; otherIndex++)
			B3D_TEST_ASSERT(submits[submitIndex].Fence != submits[otherIndex].Fence)
	}

	B3D_TEST_ASSERT(submits.Back().Fence == submits[0].Fence)
	B3D_TEST_ASSERT(tracker.GetLastSubmittedMarker() == submits.Size())
	B3D_TEST_ASSERT(tracker.GetCurrentMarker() == submits.Size() + 1)

	tracker.WaitUntilComplete();
	B3D_TEST_ASSERT(tracker.IsMarkerComplete(tracker.GetLastSubmittedMarker()))
	for (const GpuTimelineFenceAndValue& submit : submits)
		B3D_TEST_ASSERT(submit.Fence->IsSignaled(submit.Value))

	// A borrowed-tracker context must only be destroyed once its GPU work is known complete
	device->WaitUntilIdle();
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
	MockAllocation allocation;
	const bool ok = allocator.TryAllocate(1024, 16, allocation);
	B3D_TEST_ASSERT(ok)
	B3D_TEST_ASSERT(allocator.GetHeapCount() == 1)
	B3D_TEST_ASSERT(allocator.GetCommittedBytes() == 1 * 1024 * 1024)
	B3D_TEST_ASSERT(backend.LiveHeapCount() == 1)
	B3D_TEST_ASSERT(allocation.IsOwned())
	B3D_TEST_ASSERT(allocation.Size >= 1024)
	B3D_TEST_ASSERT((allocation.Offset & 15) == 0)

	// Allocations support non-power-of-two alignments such as a 12-byte stride combined with a 512-byte copy footprint.
	MockAllocation arbitraryAlignmentAllocation;
	B3D_TEST_ASSERT(allocator.TryAllocate(1024, 1536, arbitraryAlignmentAllocation))
	B3D_TEST_ASSERT(arbitraryAlignmentAllocation.Offset % 1536 == 0)

	FreeAndDrain(allocator, tracker, allocation);
	FreeAndDrain(allocator, tracker, arbitraryAlignmentAllocation);
}

void GpuAllocatorTestSuite::TestTlsf_SingleAllocateFree()
{
	MockHeapBackend backend;
	MockGpuCompletionTracker tracker;
	TlsfAllocator allocator(&backend, &tracker, MakeDefaultTlsfConfig());

	MockAllocation allocation;
	B3D_TEST_ASSERT(allocator.TryAllocate(2048, 64, allocation))
	B3D_TEST_ASSERT(allocation.Allocator == &allocator)
	B3D_TEST_ASSERT((allocation.Offset & 63) == 0)

	const u64 usedAfterAlloc = allocator.GetUsedBytes();
	B3D_TEST_ASSERT(usedAfterAlloc >= 2048)

	// Deferred drain: Free stamps the retire entry but doesn't actually return memory until
	// the submission has been signaled and Flush() runs.
	const u64 retireSubmission = tracker.AdvanceFrame();
	allocator.Free(allocation);
	B3D_TEST_ASSERT(!allocation.IsOwned())
	B3D_TEST_ASSERT(allocator.GetUsedBytes() == usedAfterAlloc) // Still accounted for — fence pending.

	tracker.MarkFrameComplete(retireSubmission);
	allocator.ReclaimUnused();
	B3D_TEST_ASSERT(allocator.GetUsedBytes() == 0)
}

void GpuAllocatorTestSuite::TestTlsf_FailsSoftOnHeapCreationFailure()
{
	// A TLSF allocator over a backend that cannot create heaps must fail the allocation gracefully
	// (TryAllocate returns false, the allocation stays invalid) rather than constructing a heap wrapper
	// around a null handle.
	OutOfMemoryHeapBackend backend;

	TGpuTlsfAllocator<OutOfMemoryHeapBackend>::Configuration configuration;
	configuration.InitialHeapSize = 4096;
	configuration.MaxHeapSize = 65536;
	configuration.DeferralMode = GpuAllocatorFreeDeferralMode::ResourceLifecycle;

	TGpuTlsfAllocator<OutOfMemoryHeapBackend> allocator(&backend, nullptr, configuration);

	GpuAllocation allocation;
	const bool allocated = allocator.TryAllocate(256, 16, allocation);

	B3D_TEST_ASSERT(allocated == false)
	B3D_TEST_ASSERT(allocation.IsOwned() == false)
	B3D_TEST_ASSERT(backend.CreateCallCount == 1)
	B3D_TEST_ASSERT(backend.DestroyCallCount == 0)
	B3D_TEST_ASSERT(allocator.GetHeapCount() == 0)
}

void GpuAllocatorTestSuite::TestTlsf_NonOverlappingAlignedOffsets()
{
	MockHeapBackend backend;
	MockGpuCompletionTracker tracker;
	TlsfAllocator allocator(&backend, &tracker, MakeDefaultTlsfConfig());

	struct Alloc { MockAllocation Allocation; u64 Begin; u64 End; };
	Vector<Alloc> allocs;

	const u32 allocCount = 64;
	const u64 allocSize = 4096;
	const u32 alignment = 256;

	for (u32 allocIndex = 0; allocIndex < allocCount; allocIndex++)
	{
		Alloc record;
		B3D_TEST_ASSERT(allocator.TryAllocate(allocSize, alignment, record.Allocation))
		B3D_TEST_ASSERT((record.Allocation.Offset & (alignment - 1)) == 0)
		record.Begin = record.Allocation.Offset;
		record.End = record.Allocation.Offset + record.Allocation.Size;
		allocs.push_back(record);
	}

	// Verify all (Heap, Begin, End) ranges are non-overlapping.
	for (u32 outerIndex = 0; outerIndex < allocs.size(); outerIndex++)
	{
		for (u32 innerIndex = outerIndex + 1; innerIndex < allocs.size(); innerIndex++)
		{
			if (allocs[outerIndex].Allocation.Heap != allocs[innerIndex].Allocation.Heap)
				continue;
			const bool disjoint = allocs[outerIndex].End <= allocs[innerIndex].Begin
				|| allocs[innerIndex].End <= allocs[outerIndex].Begin;
			B3D_TEST_ASSERT(disjoint)
		}
	}

	// Drain everything.
	tracker.AdvanceFrame();
	for (Alloc& record : allocs)
		allocator.Free(record.Allocation);
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
		MockAllocation allocations[3];
		for (u32 blockIndex = 0; blockIndex < 3; blockIndex++)
			B3D_TEST_ASSERT(allocator.TryAllocate(blockSize, 16, allocations[blockIndex]))

		// All three live in the same heap (heap is 64KB, allocations total 48KB).
		B3D_TEST_ASSERT(allocations[0].Heap == allocations[1].Heap)
		B3D_TEST_ASSERT(allocations[0].Heap == allocations[2].Heap)

		tracker.AdvanceFrame();
		for (u32 freeIndex = 0; freeIndex < 3; freeIndex++)
			allocator.Free(allocations[freeOrder[freeIndex]]);
		tracker.MarkAllFramesComplete();
		allocator.ReclaimUnused();
		B3D_TEST_ASSERT(allocator.GetUsedBytes() == 0)

		// Coalescing proof: a single allocation equal to the entire usable heap must succeed. If any
		// of the original three blocks didn't merge with neighbors, a 48 KB allocation would fragment
		// across two free ranges and fail.
		MockAllocation reuse;
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
	MockAllocation pin;
	B3D_TEST_ASSERT(allocator.TryAllocate(64, 16, pin))

	MockAllocation aligned;
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
	MockAllocation full;
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
	Vector<MockAllocation> allocs;

	for (u32 allocIndex = 0; allocIndex < 6; allocIndex++)
	{
		MockAllocation allocation;
		B3D_TEST_ASSERT(allocator.TryAllocate(allocSize, 16, allocation))
		allocs.push_back(allocation);
	}

	// 6 allocations / 2-per-heap = 3 heaps minimum. Single-allocation heaps may have been used too.
	const u32 heapsAfterFill = allocator.GetHeapCount();
	B3D_TEST_ASSERT(heapsAfterFill >= 3)
	B3D_TEST_ASSERT(backend.CreateCount() == heapsAfterFill)

	// Free everything.
	tracker.AdvanceFrame();
	for (MockAllocation& allocation : allocs)
		allocator.Free(allocation);
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
	MockAllocation oversized;
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

	struct Live { MockAllocation Allocation; u64 Begin; u64 End; };
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
			if (allocator.TryAllocate(size, alignment, record.Allocation))
			{
				B3D_TEST_ASSERT((record.Allocation.Offset & (alignment - 1)) == 0)
				record.Begin = record.Allocation.Offset;
				record.End = record.Allocation.Offset + record.Allocation.Size;

				// Verify non-overlap against every live entry on the same heap. The O(N) cost is
				// negligible at the 4000-iteration / sub-1KB-live workload of this stress test.
				for (const Live& other : live)
				{
					if (other.Allocation.Heap != record.Allocation.Heap)
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
			allocator.Free(live[victimIndex].Allocation);
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
		allocator.Free(record.Allocation);
	tracker.MarkAllFramesComplete();
	allocator.ReclaimUnused();

	B3D_TEST_ASSERT(allocator.GetUsedBytes() == 0)
}

namespace
{
	/** Creates a 1 MB single-heap allocator setup used by the within-heap planning tests. */
	TlsfAllocator::Configuration MakeSingleHeapConfig()
	{
		TlsfAllocator::Configuration configuration = MakeDefaultTlsfConfig();
		configuration.InitialHeapSize = 1 * 1024 * 1024;
		configuration.MaxHeapSize = 1 * 1024 * 1024;
		return configuration;
	}

	/** Allocates @p count owned allocations of @p size bytes, then frees every other one starting with the first, leaving holes below the rest. */
	Vector<TUnique<OwnedAllocation>> AllocateSawtooth(TlsfAllocator& allocator, MockGpuCompletionTracker& tracker, u32 count, u64 size, u32 alignment = 16)
	{
		Vector<TUnique<OwnedAllocation>> holders;
		for (u32 holderIndex = 0; holderIndex < count; holderIndex++)
		{
			holders.push_back(B3DMakeUnique<OwnedAllocation>());
			const bool allocated = AllocateOwned(allocator, size, alignment, *holders.back());
			B3D_ASSERT(allocated);
			(void)allocated;
		}

		tracker.AdvanceFrame();
		for (u32 holderIndex = 0; holderIndex < count; holderIndex += 2)
			FreeOwned(allocator, *holders[holderIndex]);
		tracker.MarkAllFramesComplete();
		allocator.ReclaimUnused(false);

		return holders;
	}
}

void GpuAllocatorTestSuite::TestTlsf_Defrag_DrainsHighestHeap()
{
	// 64 KB heaps hold four 16 KB allocations each. Six allocations fill heap 0 and spill two into heap 1. Freeing two allocations
	// in heap 0 makes room for heap 1's, so both are planned to move and heap 1 is released once the sources are freed.
	MockHeapBackend backend;
	MockGpuCompletionTracker tracker;

	TlsfAllocator::Configuration configuration = MakeDefaultTlsfConfig();
	configuration.InitialHeapSize = 64 * 1024;
	configuration.MaxHeapSize = 64 * 1024;
	configuration.GrowthFactor = 1;
	configuration.MaxEmptyHeapCount = 0;
	TlsfAllocator allocator(&backend, &tracker, configuration);

	const u32 kAllocCount = 6;
	const u64 kAllocSize = 16 * 1024;

	Vector<TUnique<OwnedAllocation>> holders;
	for (u32 holderIndex = 0; holderIndex < kAllocCount; holderIndex++)
	{
		holders.push_back(B3DMakeUnique<OwnedAllocation>());
		B3D_TEST_ASSERT(AllocateOwned(allocator, kAllocSize, 16, *holders.back()))
	}

	B3D_TEST_ASSERT(allocator.GetHeapCount() == 2)
	const u32 heap0Slot = holders[0]->Allocation.AllocatorData0;
	const u32 heap1Slot = holders[4]->Allocation.AllocatorData0;
	B3D_TEST_ASSERT(heap0Slot != heap1Slot)
	B3D_TEST_ASSERT(holders[5]->Allocation.AllocatorData0 == heap1Slot)

	tracker.AdvanceFrame();
	FreeOwned(allocator, *holders[0]);
	FreeOwned(allocator, *holders[1]);
	tracker.MarkAllFramesComplete();
	allocator.ReclaimUnused(false);

	const TArray<GpuAllocationMove> moves = PlanMoves(allocator);
	B3D_TEST_ASSERT(moves.Size() == 2)
	CommitMoves(allocator, moves);

	B3D_TEST_ASSERT(holders[4]->Allocation.AllocatorData0 == heap0Slot)
	B3D_TEST_ASSERT(holders[5]->Allocation.AllocatorData0 == heap0Slot)

	// The sources were freed against the current frame. Once it completes, the empty heap 1 is released.
	tracker.AdvanceFrame();
	tracker.MarkAllFramesComplete();
	allocator.ReclaimUnused(false);

	B3D_TEST_ASSERT(allocator.GetHeapCount() == 1)
}

void GpuAllocatorTestSuite::TestTlsf_Defrag_SingleHeapWithinHeapCompaction()
{
	// With a single heap, allocations above the holes left by freeing every other allocation move down into them
	MockHeapBackend backend;
	MockGpuCompletionTracker tracker;
	TlsfAllocator allocator(&backend, &tracker, MakeSingleHeapConfig());

	const u32 kAllocCount = 16;
	Vector<TUnique<OwnedAllocation>> holders = AllocateSawtooth(allocator, tracker, kAllocCount, 16 * 1024);

	const TArray<GpuAllocationMove> moves = PlanMoves(allocator);
	B3D_TEST_ASSERT(!moves.Empty())

	for (const GpuAllocationMove& move : moves)
	{
		B3D_TEST_ASSERT(move.Destination.AllocatorData0 == move.Source.AllocatorData0)
		B3D_TEST_ASSERT(move.Destination.Offset < move.Source.Offset)
	}

	CommitMoves(allocator, moves);
	B3D_TEST_ASSERT(allocator.GetHeapCount() == 1)
}

void GpuAllocatorTestSuite::TestTlsf_Defrag_RespectsBudget()
{
	// A large allocation sits above a hole it fits in, and small allocations sit above smaller holes. A byte budget smaller than
	// the large allocation skips it without stopping the pass, and a move count budget limits the number of moves.
	MockHeapBackend backend;
	MockGpuCompletionTracker tracker;
	TlsfAllocator allocator(&backend, &tracker, MakeSingleHeapConfig());

	const u64 kSmallSize = 16 * 1024;
	const u64 kLargeSize = 64 * 1024;

	OwnedAllocation bottom;
	B3D_TEST_ASSERT(AllocateOwned(allocator, kLargeSize, 16, bottom))

	Vector<TUnique<OwnedAllocation>> holders = AllocateSawtooth(allocator, tracker, 8, kSmallSize);

	OwnedAllocation large;
	B3D_TEST_ASSERT(AllocateOwned(allocator, kLargeSize, 16, large))

	tracker.AdvanceFrame();
	FreeOwned(allocator, bottom);
	tracker.MarkAllFramesComplete();
	allocator.ReclaimUnused(false);

	// Without a budget the large allocation moves into the hole at the bottom
	TArray<GpuAllocationMove> moves = PlanMoves(allocator);
	bool isLargePlanned = false;
	for (const GpuAllocationMove& move : moves)
	{
		if (move.Owner.ToOpaque() == &large)
			isLargePlanned = true;
	}

	B3D_TEST_ASSERT(isLargePlanned)
	CancelMoves(allocator, moves);

	GpuDefragmentationBudget byteBudget;
	byteBudget.MaximumByteCount = kSmallSize;
	moves = PlanMoves(allocator, byteBudget);
	B3D_TEST_ASSERT(moves.Size() == 1)
	B3D_TEST_ASSERT(moves[0].Owner.ToOpaque() != &large)
	B3D_TEST_ASSERT(moves[0].Source.Size <= kSmallSize)
	CancelMoves(allocator, moves);

	GpuDefragmentationBudget moveBudget;
	moveBudget.MaximumMoveCount = 2;
	moves = PlanMoves(allocator, moveBudget);
	B3D_TEST_ASSERT(moves.Size() == 2)
	CancelMoves(allocator, moves);
}

void GpuAllocatorTestSuite::TestTlsf_Defrag_SkipsUnownedAndUnmovable()
{
	// Even allocations are owned and odd ones have no owner. Allocations without an owner, and owned allocations the predicate
	// rejects, are never planned to move.
	MockHeapBackend backend;
	MockGpuCompletionTracker tracker;
	TlsfAllocator allocator(&backend, &tracker, MakeSingleHeapConfig());

	const u32 kAllocCount = 10;
	const u64 kAllocSize = 16 * 1024;

	Vector<TUnique<OwnedAllocation>> holders;
	for (u32 holderIndex = 0; holderIndex < kAllocCount; holderIndex++)
	{
		holders.push_back(B3DMakeUnique<OwnedAllocation>());
		if ((holderIndex & 1) == 0)
			B3D_TEST_ASSERT(AllocateOwned(allocator, kAllocSize, 16, *holders.back()))
		else
			B3D_TEST_ASSERT(allocator.TryAllocate(kAllocSize, 16, holders.back()->Allocation))
	}

	tracker.AdvanceFrame();
	FreeOwned(allocator, *holders[2]);
	FreeOwned(allocator, *holders[4]);
	tracker.MarkAllFramesComplete();
	allocator.ReclaimUnused(false);

	OwnedAllocation* const unmovable = holders[6].get();
	const TArray<GpuAllocationMove> moves = PlanMoves(allocator, GpuDefragmentationBudget(),
		[unmovable](GpuAllocationOwner owner) { return owner.ToOpaque() != unmovable; });

	B3D_TEST_ASSERT(!moves.Empty())
	for (const GpuAllocationMove& move : moves)
		B3D_TEST_ASSERT(move.Owner.ToOpaque() == holders[8].get())

	CancelMoves(allocator, moves);
}

void GpuAllocatorTestSuite::TestTlsf_Defrag_CancelReleasesDestinations()
{
	// Planning reserves the destinations. Cancelling the moves frees them and leaves the owners where they were, so a later pass
	// plans moves of them again.
	MockHeapBackend backend;
	MockGpuCompletionTracker tracker;
	TlsfAllocator allocator(&backend, &tracker, MakeSingleHeapConfig());

	Vector<TUnique<OwnedAllocation>> holders = AllocateSawtooth(allocator, tracker, 8, 16 * 1024);

	Vector<u64> offsets;
	for (const TUnique<OwnedAllocation>& holder : holders)
		offsets.push_back(holder->Allocation.Offset);

	const u64 usedBytes = allocator.GetUsedBytes();

	const TArray<GpuAllocationMove> moves = PlanMoves(allocator);
	B3D_TEST_ASSERT(!moves.Empty())
	B3D_TEST_ASSERT(allocator.GetUsedBytes() > usedBytes)

	CancelMoves(allocator, moves);
	AdvanceCompleteAndFlush(allocator, tracker);
	B3D_TEST_ASSERT(allocator.GetUsedBytes() == usedBytes)

	for (u32 holderIndex = 0; holderIndex < (u32)holders.size(); holderIndex++)
		B3D_TEST_ASSERT(holders[holderIndex]->Allocation.Offset == offsets[holderIndex])

	const TArray<GpuAllocationMove> nextMoves = PlanMoves(allocator);
	B3D_TEST_ASSERT(!nextMoves.Empty())
	CancelMoves(allocator, nextMoves);
}

void GpuAllocatorTestSuite::TestTlsf_Defrag_PlanDescribesMoves()
{
	// Every planned move names the owner, the owner's current allocation as the source, and a reserved destination of the same
	// size in a lower location
	MockHeapBackend backend;
	MockGpuCompletionTracker tracker;
	TlsfAllocator allocator(&backend, &tracker, MakeSingleHeapConfig());

	const u64 kAllocSize = 16 * 1024;
	Vector<TUnique<OwnedAllocation>> holders = AllocateSawtooth(allocator, tracker, 8, kAllocSize);

	const TArray<GpuAllocationMove> moves = PlanMoves(allocator);
	B3D_TEST_ASSERT(!moves.Empty())

	for (const GpuAllocationMove& move : moves)
	{
		const OwnedAllocation& holder = *static_cast<const OwnedAllocation*>(move.Owner.ToOpaque());
		B3D_TEST_ASSERT(move.Source.Allocator == static_cast<IGpuAllocator*>(&allocator))
		B3D_TEST_ASSERT(move.Source.Heap == holder.Allocation.Heap)
		B3D_TEST_ASSERT(move.Source.Offset == holder.Allocation.Offset)
		B3D_TEST_ASSERT(move.Source.AllocatorData0 == holder.Allocation.AllocatorData0)
		B3D_TEST_ASSERT(move.Source.AllocatorData1 == holder.Allocation.AllocatorData1)

		B3D_TEST_ASSERT(move.Destination.IsOwned())
		B3D_TEST_ASSERT(move.Destination.Allocator == static_cast<IGpuAllocator*>(&allocator))
		B3D_TEST_ASSERT(move.Destination.Size == move.Source.Size)
		B3D_TEST_ASSERT(move.Destination.Size >= kAllocSize)
		B3D_TEST_ASSERT(move.Destination.Offset < move.Source.Offset)
	}

	CommitMoves(allocator, moves);

	// Committed destinations have the owners, so a second pass only plans moves of the owners' new allocations
	const TArray<GpuAllocationMove> nextMoves = PlanMoves(allocator);
	for (const GpuAllocationMove& move : nextMoves)
	{
		const OwnedAllocation& holder = *static_cast<const OwnedAllocation*>(move.Owner.ToOpaque());
		B3D_TEST_ASSERT(move.Source.Offset == holder.Allocation.Offset)
	}

	CancelMoves(allocator, nextMoves);
}

void GpuAllocatorTestSuite::TestTlsf_Defrag_KeepsAlignment()
{
	// Allocations of a size that is not a multiple of their alignment leave padding next to them, so the holes they leave start
	// at unaligned offsets once the padding coalesces. Destinations must still have the alignment of the original allocation.
	MockHeapBackend backend;
	MockGpuCompletionTracker tracker;
	TlsfAllocator allocator(&backend, &tracker, MakeSingleHeapConfig());

	const u32 kAlignment = 256;
	Vector<TUnique<OwnedAllocation>> holders = AllocateSawtooth(allocator, tracker, 16, 1000, kAlignment);

	for (const TUnique<OwnedAllocation>& holder : holders)
	{
		if (holder->Allocation.IsOwned())
			B3D_TEST_ASSERT(holder->Allocation.Offset % kAlignment == 0)
	}

	const TArray<GpuAllocationMove> moves = PlanMoves(allocator);
	B3D_TEST_ASSERT(!moves.Empty())

	for (const GpuAllocationMove& move : moves)
		B3D_TEST_ASSERT(move.Destination.Offset % kAlignment == 0)

	CommitMoves(allocator, moves);
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

	MockAllocation allocation;
	B3D_TEST_ASSERT(allocator.TryAllocate(2048, 64, allocation))
	const u64 usedAfterAlloc = allocator.GetUsedBytes();
	B3D_TEST_ASSERT(usedAfterAlloc >= 2048)

	allocator.Free(allocation);

	// Synchronous release — no Flush needed, no frame-tracker tick required.
	B3D_TEST_ASSERT(allocator.GetUsedBytes() == 0)
	B3D_TEST_ASSERT(!allocation.IsOwned())
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

	MockAllocation allocation;
	B3D_TEST_ASSERT(allocator.TryAllocate(2048, 64, allocation))
	const u64 usedAfterAlloc = allocator.GetUsedBytes();
	B3D_TEST_ASSERT(usedAfterAlloc >= 2048)

	// Free stamps the entry against the current frame index. With no tick the queue stays held.
	const u64 retireFrame = tracker.CurrentFrameIndex();
	allocator.Free(allocation);
	B3D_TEST_ASSERT(!allocation.IsOwned())
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

void GpuAllocatorTestSuite::TestTlsf_Defrag_ResourceLifecycleReleasesSources()
{
	// Under ResourceLifecycle, freeing the sources of committed moves releases them immediately, so the drained heap is released
	// as soon as the last move is committed
	MockHeapBackend backend;
	MockGpuCompletionTracker tracker;

	TlsfAllocator::Configuration configuration = MakeDefaultTlsfConfig();
	configuration.InitialHeapSize = 64 * 1024;
	configuration.MaxHeapSize = 64 * 1024;
	configuration.GrowthFactor = 1;
	configuration.MaxEmptyHeapCount = 0;
	configuration.DeferralMode = GpuAllocatorFreeDeferralMode::ResourceLifecycle;
	TlsfAllocator allocator(&backend, &tracker, configuration);

	const u32 kAllocCount = 6;
	const u64 kAllocSize = 16 * 1024;

	Vector<TUnique<OwnedAllocation>> holders;
	for (u32 holderIndex = 0; holderIndex < kAllocCount; holderIndex++)
	{
		holders.push_back(B3DMakeUnique<OwnedAllocation>());
		B3D_TEST_ASSERT(AllocateOwned(allocator, kAllocSize, 16, *holders.back()))
	}

	B3D_TEST_ASSERT(allocator.GetHeapCount() == 2)
	const u32 heap0Slot = holders[0]->Allocation.AllocatorData0;

	FreeOwned(allocator, *holders[0]);
	FreeOwned(allocator, *holders[1]);

	const TArray<GpuAllocationMove> moves = PlanMoves(allocator);
	B3D_TEST_ASSERT(moves.Size() == 2)
	CommitMoves(allocator, moves);

	for (u32 holderIndex = 4; holderIndex < kAllocCount; holderIndex++)
		B3D_TEST_ASSERT(holders[holderIndex]->Allocation.AllocatorData0 == heap0Slot)

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

		Vector<MockAllocation> live;
		live.reserve(8);

		for (u32 iteration = 0; iteration < kIterationsPerWorker; iteration++)
		{
			const bool wantAllocate = live.empty() || ((u32)live.size() < liveCountDistribution(rng));
			if (wantAllocate)
			{
				MockAllocation loc;
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
		for (MockAllocation& loc : live)
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
	// Worker threads churn allocations without owners while a planning thread repeatedly plans moves of its own owned allocations,
	// and commits or cancels them. The allocator lock must keep the heaps consistent, so every byte is free once all threads are done.
	MockHeapBackend backend;

	TlsfAllocator::Configuration configuration = MakeDefaultTlsfConfig();
	configuration.InitialHeapSize = 256 * 1024;
	configuration.MaxHeapSize = 1024 * 1024;
	configuration.DeferralMode = GpuAllocatorFreeDeferralMode::ResourceLifecycle;
	TlsfAllocator allocator(&backend, nullptr, configuration);

	constexpr u32 kOwnedCount = 32;
	constexpr u32 kIterationsPerWorker = 400;
	constexpr u32 kWorkerCount = 4;

	// Owned allocations are only touched by the planning thread, like textures and buffers are only moved by the render thread
	Vector<TUnique<OwnedAllocation>> holders;
	for (u32 holderIndex = 0; holderIndex < kOwnedCount; holderIndex++)
	{
		holders.push_back(B3DMakeUnique<OwnedAllocation>());
		B3D_TEST_ASSERT(AllocateOwned(allocator, 4096, 16, *holders.back()))
	}

	std::atomic<bool> stop{ false };

	auto fnWorker = [&](u32 seed)
	{
		std::mt19937 rng(seed);
		std::uniform_int_distribution<u32> sizeDistribution(256, 8 * 1024);
		std::uniform_int_distribution<u32> liveCountDistribution(1, 8);

		Vector<MockAllocation> live;
		for (u32 iteration = 0; iteration < kIterationsPerWorker; iteration++)
		{
			if (live.empty() || (u32)live.size() < liveCountDistribution(rng))
			{
				MockAllocation allocation;
				if (allocator.TryAllocate(sizeDistribution(rng), 16, allocation))
					live.push_back(allocation);
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

		for (MockAllocation& allocation : live)
			allocator.Free(allocation);
	};

	auto fnPlanWorker = [&]()
	{
		// Free some owned allocations first, so the rest have somewhere to move
		for (u32 holderIndex = 0; holderIndex < kOwnedCount; holderIndex += 2)
			FreeOwned(allocator, *holders[holderIndex]);

		bool commit = false;
		while (!stop.load(std::memory_order_relaxed))
		{
			const TArray<GpuAllocationMove> moves = PlanMoves(allocator);
			if (commit)
				CommitMoves(allocator, moves);
			else
				CancelMoves(allocator, moves);

			commit = !commit;
		}
	};

	Vector<std::thread> workers;
	for (u32 workerIndex = 0; workerIndex < kWorkerCount; workerIndex++)
		workers.emplace_back(fnWorker, 0xC0FFEEu + workerIndex);
	std::thread planThread(fnPlanWorker);

	for (std::thread& worker : workers)
		worker.join();
	stop.store(true, std::memory_order_release);
	planThread.join();

	for (TUnique<OwnedAllocation>& holder : holders)
	{
		if (holder->Allocation.IsOwned())
			FreeOwned(allocator, *holder);
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

	MockAllocation locA;
	MockAllocation locB;
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
	MockAllocation allocation;
	B3D_TEST_ASSERT(allocator.TryAllocate(1024, 16, allocation))
	B3D_TEST_ASSERT(allocator.GetLivePageCount() == 1)
	B3D_TEST_ASSERT(allocator.GetCommittedBytes() == 64 * 1024)
	B3D_TEST_ASSERT(backend.LiveHeapCount() == 1)
	B3D_TEST_ASSERT(allocation.IsOwned())
	B3D_TEST_ASSERT(allocation.Allocator == &allocator)
	B3D_TEST_ASSERT(allocation.Size == 1024)
	B3D_TEST_ASSERT((allocation.Offset & 15) == 0)
}

void GpuAllocatorTestSuite::TestLinear_BumpPointerAlignedOffsets()
{
	MockHeapBackend backend;
	MockGpuCompletionTracker tracker;
	LinearAllocator allocator(&backend, &tracker, MakeDefaultLinearConfig());

	struct Alloc { MockAllocation Allocation; u64 Begin; u64 End; };
	Vector<Alloc> allocs;

	const u32 allocCount = 8;
	const u64 allocSize = 1024;
	const u32 alignment = 256;

	for (u32 allocIndex = 0; allocIndex < allocCount; allocIndex++)
	{
		Alloc record;
		B3D_TEST_ASSERT(allocator.TryAllocate(allocSize, alignment, record.Allocation))
		B3D_TEST_ASSERT((record.Allocation.Offset & (alignment - 1)) == 0)
		record.Begin = record.Allocation.Offset;
		record.End = record.Allocation.Offset + record.Allocation.Size;
		allocs.push_back(record);
	}

	// All allocations land in the same page (8 * 1024 << 64KB) and don't overlap.
	for (u32 outerIndex = 0; outerIndex < allocs.size(); outerIndex++)
	{
		B3D_TEST_ASSERT(allocs[outerIndex].Allocation.Heap == allocs[0].Allocation.Heap)
		for (u32 innerIndex = outerIndex + 1; innerIndex < allocs.size(); innerIndex++)
		{
			const bool disjoint = allocs[outerIndex].End <= allocs[innerIndex].Begin
				|| allocs[innerIndex].End <= allocs[outerIndex].Begin;
			B3D_TEST_ASSERT(disjoint)
		}
	}

	MockAllocation nonPowerOfTwoAligned;
	B3D_TEST_ASSERT(allocator.TryAllocate(512, 1536, nonPowerOfTwoAligned))
	B3D_TEST_ASSERT(nonPowerOfTwoAligned.Offset % 1536 == 0)

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

	GpuAllocation regularAllocation;
	B3D_TEST_ASSERT(!allocator.TryAllocate(256, 16, regularAllocation))
	B3D_TEST_ASSERT(!regularAllocation.IsOwned())
	B3D_TEST_ASSERT(allocator.GetOutstandingAllocationCount() == 0)

	GpuAllocation oversizeAllocation;
	B3D_TEST_ASSERT(!allocator.TryAllocate(8 * 1024, 16, oversizeAllocation))
	B3D_TEST_ASSERT(!oversizeAllocation.IsOwned())
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

	MockAllocation first;
	B3D_TEST_ASSERT(allocator.TryAllocate(5 * 1024, 16, first))

	// 5KB used — a 4KB request can't fit in the remaining 3KB, must rotate.
	MockAllocation second;
	B3D_TEST_ASSERT(allocator.TryAllocate(4 * 1024, 16, second))
	B3D_TEST_ASSERT(second.Heap != first.Heap)
	B3D_TEST_ASSERT(allocator.GetLivePageCount() == 2) // First retired-pending-drain, second active.

	// First allocation's allocation is still valid — backend heap is alive until fence drains.
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
	MockAllocation a, b, c, d;
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

	MockAllocation first;
	B3D_TEST_ASSERT(allocator.TryAllocate(3 * 1024, 16, first))

	// Trigger overflow → page 2.
	MockAllocation second;
	B3D_TEST_ASSERT(allocator.TryAllocate(3 * 1024, 16, second))
	B3D_TEST_ASSERT(backend.CreateCount() == 2)

	// Drain the retired page 1 onto spares.
	AdvanceCompleteAndFlush(allocator, tracker);
	B3D_TEST_ASSERT(allocator.GetSparePageCount() == 1)
	B3D_TEST_ASSERT(backend.CreateCount() == 2) // No new heaps created during drain.
	B3D_TEST_ASSERT(backend.DestroyCount() == 0)

	// Trigger another overflow on page 2 — should pull from spares, not call CreateHeap again.
	MockAllocation third;
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
	MockAllocation small;
	B3D_TEST_ASSERT(allocator.TryAllocate(1024, 16, small))
	const u64 usedBeforeOversize = allocator.GetUsedBytes();
	const IGpuHeap* activeHeapId = small.Heap;

	// Oversize request: PageSize is 8KB; ask for 32KB. Lands in a dedicated heap and is retired
	// immediately — heap stays alive in the deferred-free queue until the fence completes.
	MockAllocation oversize;
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

	MockAllocation first;
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
	MockAllocation second;
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
	MockAllocation a, b, c;
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

	MockAllocation first;
	B3D_TEST_ASSERT(allocator.TryAllocate(1024, 16, first))
	const IGpuHeap* originalHeapId = first.Heap;
	const u64 usedBefore = allocator.GetUsedBytes();
	const u32 livePagesBefore = allocator.GetLivePageCount();

	// Free is a no-op for the linear allocator: the allocation is reset by the base, but the active
	// page's bump offset is unchanged and no retire entry is queued.
	allocator.Free(first);
	B3D_TEST_ASSERT(!first.IsOwned())
	B3D_TEST_ASSERT(allocator.GetUsedBytes() == usedBefore)
	B3D_TEST_ASSERT(allocator.GetLivePageCount() == livePagesBefore)

	// Subsequent allocate lands AFTER the freed range and on the same active page — proves no recycling happened.
	MockAllocation second;
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
	MockAllocation a, b, c;
	B3D_TEST_ASSERT(allocator.TryAllocate(1024, 16, a))
	B3D_TEST_ASSERT(allocator.TryAllocate(1024, 16, b))
	B3D_TEST_ASSERT(allocator.TryAllocate(1024, 16, c))
	B3D_TEST_ASSERT(a.Heap == b.Heap && b.Heap == c.Heap)

	const u64 usedBefore = allocator.GetUsedBytes();
	const u32 livePagesBefore = allocator.GetLivePageCount();
	const u32 destroyCountBefore = backend.DestroyCount();

	// FreeAndReclaim one of the three. The page must NOT be recycled — peers are still live.
	allocator.FreeAndReclaim(a);
	B3D_TEST_ASSERT(!a.IsOwned())
	B3D_TEST_ASSERT(b.IsOwned())
	B3D_TEST_ASSERT(c.IsOwned())
	B3D_TEST_ASSERT(allocator.GetUsedBytes() == usedBefore)        // Bump offset unchanged.
	B3D_TEST_ASSERT(allocator.GetLivePageCount() == livePagesBefore)
	B3D_TEST_ASSERT(allocator.GetSparePageCount() == 0)              // No spurious spare entry.
	B3D_TEST_ASSERT(backend.DestroyCount() == destroyCountBefore)    // No spurious destroy.

	// Subsequent allocate keeps bumping past c's range — proves the page is still being used as the active page.
	MockAllocation d;
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
	MockAllocation a0, a1, b0, b1;
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
	MockAllocation a2, b2;
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
	MockAllocation a, b, c, d;
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
	MockAllocation first, second;
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
		MockAllocation first, second;
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
	// 1A.2 made the allocation carry its own free handle: GpuAllocation::Allocator is an
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

	MockAllocation tlsfAllocation;
	B3D_TEST_ASSERT(tlsfAllocator.TryAllocate(2048, 64, tlsfAllocation))

	// Two linear allocations share one active page so the no-op free can be observed against a peer.
	MockAllocation linearFirst, linearSecond;
	B3D_TEST_ASSERT(linearAllocator.TryAllocate(1024, 16, linearFirst))
	B3D_TEST_ASSERT(linearAllocator.TryAllocate(1024, 16, linearSecond))
	B3D_TEST_ASSERT(linearFirst.Heap == linearSecond.Heap)

	// Identity: each allocation points back at the concrete allocator that produced it, upcast to the
	// shared IGpuAllocator interface. The two carried handles are distinct.
	B3D_TEST_ASSERT(tlsfAllocation.Allocator == static_cast<IGpuAllocator*>(&tlsfAllocator))
	B3D_TEST_ASSERT(linearFirst.Allocator == static_cast<IGpuAllocator*>(&linearAllocator))
	B3D_TEST_ASSERT(linearSecond.Allocator == static_cast<IGpuAllocator*>(&linearAllocator))
	B3D_TEST_ASSERT(tlsfAllocation.Allocator != linearFirst.Allocator)

	const u64 tlsfUsedBeforeFree = tlsfAllocator.GetUsedBytes();
	B3D_TEST_ASSERT(tlsfUsedBeforeFree >= 2048)

	// Free the TLSF slot through ONLY the carried base pointer — no static knowledge of the concrete
	// type at the call site. Dispatch reaches the TLSF strategy and (ResourceLifecycle) reclaims it.
	IGpuAllocator* tlsfHandle = tlsfAllocation.Allocator;
	tlsfHandle->FreeAndReclaim(tlsfAllocation);
	B3D_TEST_ASSERT(!tlsfAllocation.IsOwned())
	B3D_TEST_ASSERT(tlsfAllocator.GetUsedBytes() == 0)

	// Free one linear slot through its carried base pointer. Dispatch reaches the linear strategy,
	// whose per-allocation FreeAndReclaim is a no-op: the shared page is not recycled or destroyed and
	// the peer slot stays valid.
	const u32 linearLivePagesBefore = linearAllocator.GetLivePageCount();
	const u32 linearDestroyBefore = linearBackend.DestroyCount();
	IGpuAllocator* linearHandle = linearFirst.Allocator;
	linearHandle->FreeAndReclaim(linearFirst);
	B3D_TEST_ASSERT(!linearFirst.IsOwned())
	B3D_TEST_ASSERT(linearSecond.IsOwned())
	B3D_TEST_ASSERT(linearAllocator.GetLivePageCount() == linearLivePagesBefore)
	B3D_TEST_ASSERT(linearBackend.DestroyCount() == linearDestroyBefore)

	// A subsequent linear allocation still bumps past the peer on the same page, confirming the no-op
	// free left the active page untouched.
	MockAllocation linearThird;
	B3D_TEST_ASSERT(linearAllocator.TryAllocate(1024, 16, linearThird))
	B3D_TEST_ASSERT(linearThird.Heap == linearSecond.Heap)
	B3D_TEST_ASSERT(linearThird.Offset >= linearSecond.Offset + linearSecond.Size)
}

void GpuAllocatorTestSuite::TestAllocatorIdentity_DefraggedAllocationFreesThroughCarriedAllocator()
{
	// The destination of a planned move carries the producing allocator, so an allocation moved into it can be freed through the
	// carried handle. Six allocations fill heap 0 and spill into heap 1, two heap 0 allocations are freed, and the heap 1
	// allocations move down into heap 0.
	MockHeapBackend backend;
	MockGpuCompletionTracker tracker;

	TlsfAllocator::Configuration configuration = MakeDefaultTlsfConfig();
	configuration.InitialHeapSize = 64 * 1024;
	configuration.MaxHeapSize = 64 * 1024;
	configuration.GrowthFactor = 1;
	configuration.MaxEmptyHeapCount = 0;
	TlsfAllocator allocator(&backend, &tracker, configuration);

	const u32 kAllocCount = 6;
	const u64 kAllocSize = 16 * 1024;

	Vector<TUnique<OwnedAllocation>> holders;
	for (u32 holderIndex = 0; holderIndex < kAllocCount; holderIndex++)
	{
		holders.push_back(B3DMakeUnique<OwnedAllocation>());
		B3D_TEST_ASSERT(AllocateOwned(allocator, kAllocSize, 16, *holders.back()))
		B3D_TEST_ASSERT(holders.back()->Allocation.Allocator == static_cast<IGpuAllocator*>(&allocator))
	}

	B3D_TEST_ASSERT(allocator.GetHeapCount() == 2)
	const u32 heap0Slot = holders[0]->Allocation.AllocatorData0;

	tracker.AdvanceFrame();
	FreeOwned(allocator, *holders[0]);
	FreeOwned(allocator, *holders[1]);
	tracker.MarkAllFramesComplete();
	allocator.ReclaimUnused(false);

	const TArray<GpuAllocationMove> moves = PlanMoves(allocator);
	B3D_TEST_ASSERT(moves.Size() == 2)
	CommitMoves(allocator, moves);

	for (u32 holderIndex = 4; holderIndex < kAllocCount; holderIndex++)
	{
		B3D_TEST_ASSERT(holders[holderIndex]->Allocation.AllocatorData0 == heap0Slot)
		B3D_TEST_ASSERT(holders[holderIndex]->Allocation.Allocator == static_cast<IGpuAllocator*>(&allocator))
	}

	// Free the remaining allocations, including the moved ones, only through their carried allocator
	tracker.AdvanceFrame();
	for (u32 holderIndex = 2; holderIndex < kAllocCount; holderIndex++)
	{
		MockAllocation& allocation = holders[holderIndex]->Allocation;
		B3D_TEST_ASSERT(allocation.IsOwned())
		allocation.Allocator->SetAllocationOwner(allocation, GpuAllocationOwner());
		allocation.Allocator->Free(allocation);
	}
	tracker.MarkAllFramesComplete();
	allocator.ReclaimUnused(false);
	B3D_TEST_ASSERT(allocator.GetUsedBytes() == 0)
}

void GpuAllocatorTestSuite::TestSubmissionTimeline()
{
	const GpuQueueId graphics(GQT_GRAPHICS, 0);
	const GpuQueueId compute(GQT_COMPUTE, 0);
	const GpuQueueId transfer(GQT_TRANSFER, 0);

	// Graphics G1, compute A1 waiting on graphics, graphics G2, then graphics G3 waiting on compute
	GpuSubmissionTimeline timeline;
	const u32 g1 = timeline.AddSubmission(graphics, GpuQueueMask::kNone);
	const u32 a1 = timeline.AddSubmission(compute, GpuQueueMask(graphics));
	const u32 g2 = timeline.AddSubmission(graphics, GpuQueueMask::kNone);
	const u32 g3 = timeline.AddSubmission(graphics, GpuQueueMask(compute));

	B3D_TEST_ASSERT(timeline.GetSubmissionCount() == 4)
	B3D_TEST_ASSERT(timeline.GetQueue(a1).Id == compute.Id)

	// A queue executes in submission order
	B3D_TEST_ASSERT(timeline.IsOrderedBefore(g1, g2))
	B3D_TEST_ASSERT(timeline.IsOrderedBefore(g2, g2))
	B3D_TEST_ASSERT(!timeline.IsOrderedBefore(g2, g1))

	// A wait covers the latest submission of the queue at the time, and nothing submitted to it later
	B3D_TEST_ASSERT(timeline.IsOrderedBefore(g1, a1))
	B3D_TEST_ASSERT(!timeline.IsOrderedBefore(g2, a1))
	B3D_TEST_ASSERT(timeline.IsOrderedBefore(a1, g3))
	B3D_TEST_ASSERT(!timeline.IsOrderedBefore(a1, g2))
	B3D_TEST_ASSERT(!timeline.IsOrderedBefore(a1, g1))

	// A wait on a queue without a submission in the timeline orders nothing in the timeline
	timeline.Clear();
	B3D_TEST_ASSERT(timeline.GetSubmissionCount() == 0)

	const u32 waitsOnCompute = timeline.AddSubmission(graphics, GpuQueueMask(compute));
	const u32 c1 = timeline.AddSubmission(compute, GpuQueueMask::kNone);
	B3D_TEST_ASSERT(!timeline.IsOrderedBefore(c1, waitsOnCompute))

	// Waits chain, and carry the coverage of earlier work along
	const u32 g4 = timeline.AddSubmission(graphics, GpuQueueMask(compute));
	const u32 t1 = timeline.AddSubmission(transfer, GpuQueueMask(graphics));
	B3D_TEST_ASSERT(timeline.IsOrderedBefore(c1, t1))
	B3D_TEST_ASSERT(timeline.IsOrderedBefore(g4, t1))
	B3D_TEST_ASSERT(timeline.IsOrderedBefore(waitsOnCompute, t1))
	B3D_TEST_ASSERT(!timeline.IsOrderedBefore(t1, g4))
}

namespace
{
	using TransientHeapPool = TGpuTransientHeapPool<MockHeapBackend>;

	/** Size of a transient heap in the tests, in megabytes. */
	constexpr u32 kTransientHeapSizeMegabytes = 1;

	/** Size of a transient heap in the tests, in bytes. */
	constexpr u64 kTransientHeapSize = (u64)kTransientHeapSizeMegabytes * 1024 * 1024;

	/** Sets the transient heap configuration variables, and restores their earlier values when destroyed. */
	class TransientHeapConfigurationScope
	{
	public:
		TransientHeapConfigurationScope(u32 heapMinimumSize, u32 idleFrames)
			: mHeapMinimumSize(gGpuTransientHeapMinimumSize), mIdleFrames(gGpuTransientIdleFrames)
		{
			gGpuTransientHeapMinimumSize.Set(heapMinimumSize);
			gGpuTransientIdleFrames.Set(idleFrames);
		}

		~TransientHeapConfigurationScope()
		{
			gGpuTransientHeapMinimumSize.Set(mHeapMinimumSize);
			gGpuTransientIdleFrames.Set(mIdleFrames);
		}

	private:
		u32 mHeapMinimumSize;
		u32 mIdleFrames;
	};

	/** Transient heap pool configuration over @p tracker, with a granularity of one byte. */
	TransientHeapPool::Configuration MakeTransientHeapPoolConfiguration(MockGpuCompletionTracker& tracker)
	{
		TransientHeapPool::Configuration configuration;
		configuration.CompletionTracker = &tracker;
		return configuration;
	}

	/** Returns an access scope with @p access in @p stages. */
	GpuAccessScope MakeAccess(GpuStageFlags stages, GpuAccessFlags access)
	{
		GpuAccessScope output;
		output.Add(stages, access);
		return output;
	}

	/** Resource standing in for the occupant of aliased memory. */
	class MockAliasedResource : public IGpuResource
	{
	public:
		MockAliasedResource() = default;
	};
}

void GpuAllocatorTestSuite::TestAliasing_SameQueueReuse()
{
	const TransientHeapConfigurationScope configurationScope(kTransientHeapSizeMegabytes, 1);
	MockHeapBackend backend;
	MockGpuCompletionTracker tracker;
	TransientHeapPool pool(&backend, MakeTransientHeapPoolConfiguration(tracker));
	GpuAliasingAllocator allocator;

	const GpuQueueId graphics(GQT_GRAPHICS, 0);
	GpuSubmissionTimeline timeline;
	const u32 first = timeline.AddSubmission(graphics, GpuQueueMask::kNone);
	const u32 second = timeline.AddSubmission(graphics, GpuQueueMask::kNone);

	MockAliasedResource firstOwner;
	allocator.BeginScope(timeline);

	GpuAllocation firstAllocation;
	GpuAliasAcquire firstAcquire;
	B3D_TEST_ASSERT(allocator.TryAllocate(pool, 256, 16, first, firstAllocation, firstAcquire))
	B3D_TEST_ASSERT(firstAllocation.HasMemory() && !firstAllocation.IsOwned())
	B3D_TEST_ASSERT(firstAllocation.Offset == 0 && firstAllocation.Size == 256)
	B3D_TEST_ASSERT(!firstAcquire.Source.IsValid() && firstAcquire.HasUnknownSourceAccess)
	B3D_TEST_ASSERT(backend.CreateCount() == 1)

	const GpuTransientLastUse firstLastUse(first, MakeAccess(GpuStageFlag::FragmentShaderNonUniform, GpuAccessFlag::Read));
	allocator.Release(firstAllocation, TArrayView<const GpuTransientLastUse>(&firstLastUse, 1), &firstOwner);

	// A later allocation on the same queue reuses the memory, ordered after the read
	GpuAllocation secondAllocation;
	GpuAliasAcquire secondAcquire;
	B3D_TEST_ASSERT(allocator.TryAllocate(pool, 128, 16, second, secondAllocation, secondAcquire))
	B3D_TEST_ASSERT(secondAllocation.Heap == firstAllocation.Heap && secondAllocation.Offset == 0)
	B3D_TEST_ASSERT(secondAcquire.Source.ReadStages == GpuStageFlag::FragmentShaderNonUniform)
	B3D_TEST_ASSERT(secondAcquire.Source.WriteStages == GpuStageFlag::None)
	B3D_TEST_ASSERT(!secondAcquire.HasUnknownSourceAccess)

#if B3D_BUILD_TYPE_DEVELOPMENT
	B3D_TEST_ASSERT(secondAcquire.Predecessors.Size() == 1 && secondAcquire.Predecessors[0] == &firstOwner)
#endif

	// The rest of the released memory keeps its accesses
	GpuAllocation thirdAllocation;
	GpuAliasAcquire thirdAcquire;
	B3D_TEST_ASSERT(allocator.TryAllocate(pool, 128, 16, second, thirdAllocation, thirdAcquire))
	B3D_TEST_ASSERT(thirdAllocation.Offset == 128)
	B3D_TEST_ASSERT(thirdAcquire.Source.ReadStages == GpuStageFlag::FragmentShaderNonUniform && !thirdAcquire.HasUnknownSourceAccess)

	// Memory not used before in the scope has nothing of the scope to order after, but earlier scopes may have used it
	GpuAllocation fourthAllocation;
	GpuAliasAcquire fourthAcquire;
	B3D_TEST_ASSERT(allocator.TryAllocate(pool, 256, 16, first, fourthAllocation, fourthAcquire))
	B3D_TEST_ASSERT(fourthAllocation.Offset == 256)
	B3D_TEST_ASSERT(!fourthAcquire.Source.IsValid() && fourthAcquire.HasUnknownSourceAccess)

#if B3D_BUILD_TYPE_DEVELOPMENT
	B3D_TEST_ASSERT(fourthAcquire.Predecessors.IsEmpty())
#endif

	// Memory released after a later submission on the queue is not reused by an earlier one
	const GpuTransientLastUse secondLastUse(second, MakeAccess(GpuStageFlag::ColorAttachment, GpuAccessFlag::Write));
	allocator.Release(secondAllocation, TArrayView<const GpuTransientLastUse>(&secondLastUse, 1), nullptr);

	GpuAllocation fifthAllocation;
	GpuAliasAcquire fifthAcquire;
	B3D_TEST_ASSERT(allocator.TryAllocate(pool, 128, 16, first, fifthAllocation, fifthAcquire))
	B3D_TEST_ASSERT(fifthAllocation.Offset == 512)

	GpuAllocation sixthAllocation;
	GpuAliasAcquire sixthAcquire;
	B3D_TEST_ASSERT(allocator.TryAllocate(pool, 128, 16, second, sixthAllocation, sixthAcquire))
	B3D_TEST_ASSERT(sixthAllocation.Offset == 0)
	B3D_TEST_ASSERT(sixthAcquire.Source.WriteStages == GpuStageFlag::ColorAttachment && !sixthAcquire.HasUnknownSourceAccess)

	// Memory spanning released memory and memory not used in the scope orders after the release, and is marked for the rest
	const GpuTransientLastUse fifthLastUse(first, MakeAccess(GpuStageFlag::Transfer, GpuAccessFlag::Write));
	allocator.Release(fifthAllocation, TArrayView<const GpuTransientLastUse>(&fifthLastUse, 1), nullptr);

	GpuAllocation seventhAllocation;
	GpuAliasAcquire seventhAcquire;
	B3D_TEST_ASSERT(allocator.TryAllocate(pool, 256, 16, second, seventhAllocation, seventhAcquire))
	B3D_TEST_ASSERT(seventhAllocation.Offset == 512)
	B3D_TEST_ASSERT(seventhAcquire.Source.WriteStages == GpuStageFlag::Transfer && seventhAcquire.HasUnknownSourceAccess)

	// Allocations still live at the end of the scope are dropped with it
	allocator.EndScope();
	B3D_TEST_ASSERT(backend.CreateCount() == 1)
}

void GpuAllocatorTestSuite::TestAliasing_CrossQueueEligibility()
{
	const TransientHeapConfigurationScope configurationScope(kTransientHeapSizeMegabytes, 1);
	MockHeapBackend backend;
	MockGpuCompletionTracker tracker;
	TransientHeapPool pool(&backend, MakeTransientHeapPoolConfiguration(tracker));
	GpuAliasingAllocator allocator;

	const GpuQueueId graphics(GQT_GRAPHICS, 0);
	const GpuQueueId compute(GQT_COMPUTE, 0);

	// Graphics, compute without a wait, then graphics waiting on compute
	GpuSubmissionTimeline timeline;
	const u32 graphicsSubmission = timeline.AddSubmission(graphics, GpuQueueMask::kNone);
	const u32 computeSubmission = timeline.AddSubmission(compute, GpuQueueMask::kNone);
	const u32 joinSubmission = timeline.AddSubmission(graphics, GpuQueueMask(compute));

	const GpuTransientLastUse graphicsLastUse(graphicsSubmission, MakeAccess(GpuStageFlag::ColorAttachment, GpuAccessFlag::Write));
	const GpuTransientLastUse computeLastUse(computeSubmission, MakeAccess(GpuStageFlag::ComputeShaderNonUniform, GpuAccessFlag::Write));

	MockAliasedResource graphicsOwner;
	MockAliasedResource computeOwner;
	{
		allocator.BeginScope(timeline);

		GpuAllocation graphicsAllocation;
		GpuAliasAcquire graphicsAcquire;
		B3D_TEST_ASSERT(allocator.TryAllocate(pool, 256, 16, graphicsSubmission, graphicsAllocation, graphicsAcquire))
		B3D_TEST_ASSERT(graphicsAllocation.Offset == 0)
		allocator.Release(graphicsAllocation, TArrayView<const GpuTransientLastUse>(&graphicsLastUse, 1), &graphicsOwner);

		// Compute does not wait on graphics, so it cannot reuse memory graphics used
		GpuAllocation computeAllocation;
		GpuAliasAcquire computeAcquire;
		B3D_TEST_ASSERT(allocator.TryAllocate(pool, 256, 16, computeSubmission, computeAllocation, computeAcquire))
		B3D_TEST_ASSERT(computeAllocation.Offset == 256)
		B3D_TEST_ASSERT(!computeAcquire.Source.IsValid())
		allocator.Release(computeAllocation, TArrayView<const GpuTransientLastUse>(&computeLastUse, 1), &computeOwner);

		// Graphics after the wait on compute reuses both. Only the graphics accesses need a barrier.
		GpuAllocation joinAllocation;
		GpuAliasAcquire joinAcquire;
		B3D_TEST_ASSERT(allocator.TryAllocate(pool, 512, 16, joinSubmission, joinAllocation, joinAcquire))
		B3D_TEST_ASSERT(joinAllocation.Offset == 0)
		B3D_TEST_ASSERT(joinAcquire.Source.WriteStages == GpuStageFlag::ColorAttachment)
		B3D_TEST_ASSERT(joinAcquire.Source.ReadStages == GpuStageFlag::None)

#if B3D_BUILD_TYPE_DEVELOPMENT
		B3D_TEST_ASSERT(joinAcquire.Predecessors.Size() == 2)
		B3D_TEST_ASSERT(joinAcquire.Predecessors.Contains(&graphicsOwner) && joinAcquire.Predecessors.Contains(&computeOwner))
#endif

		allocator.EndScope();
	}

	// Aliasing across queues can be disabled
	gGpuTransientAliasAcrossQueues.Set(false);
	{
		allocator.BeginScope(timeline);

		GpuAllocation computeAllocation;
		GpuAliasAcquire computeAcquire;
		B3D_TEST_ASSERT(allocator.TryAllocate(pool, 256, 16, computeSubmission, computeAllocation, computeAcquire))
		B3D_TEST_ASSERT(computeAllocation.Offset == 0)
		allocator.Release(computeAllocation, TArrayView<const GpuTransientLastUse>(&computeLastUse, 1), nullptr);

		GpuAllocation joinAllocation;
		GpuAliasAcquire joinAcquire;
		B3D_TEST_ASSERT(allocator.TryAllocate(pool, 256, 16, joinSubmission, joinAllocation, joinAcquire))
		B3D_TEST_ASSERT(joinAllocation.Offset == 256)

		allocator.EndScope();
	}
	gGpuTransientAliasAcrossQueues.Set(true);

	// Aliasing within a scope can be disabled
	gGpuTransientAliasing.Set(false);
	{
		allocator.BeginScope(timeline);

		GpuAllocation graphicsAllocation;
		GpuAliasAcquire graphicsAcquire;
		B3D_TEST_ASSERT(allocator.TryAllocate(pool, 256, 16, graphicsSubmission, graphicsAllocation, graphicsAcquire))
		B3D_TEST_ASSERT(graphicsAllocation.Offset == 0)
		allocator.Release(graphicsAllocation, TArrayView<const GpuTransientLastUse>(&graphicsLastUse, 1), nullptr);

		GpuAllocation joinAllocation;
		GpuAliasAcquire joinAcquire;
		B3D_TEST_ASSERT(allocator.TryAllocate(pool, 256, 16, joinSubmission, joinAllocation, joinAcquire))
		B3D_TEST_ASSERT(joinAllocation.Offset == 256)

		allocator.EndScope();
	}
	gGpuTransientAliasing.Set(true);

	B3D_TEST_ASSERT(backend.CreateCount() == 1)
}

void GpuAllocatorTestSuite::TestAliasing_InheritedTags()
{
	const TransientHeapConfigurationScope configurationScope(kTransientHeapSizeMegabytes, 1);
	MockHeapBackend backend;
	MockGpuCompletionTracker tracker;
	TransientHeapPool pool(&backend, MakeTransientHeapPoolConfiguration(tracker));
	GpuAliasingAllocator allocator;

	const GpuQueueId graphics(GQT_GRAPHICS, 0);
	GpuSubmissionTimeline timeline;
	const u32 first = timeline.AddSubmission(graphics, GpuQueueMask::kNone);
	const u32 second = timeline.AddSubmission(graphics, GpuQueueMask::kNone);

	MockAliasedResource writerOwner;
	MockAliasedResource unusedOwner;
	allocator.BeginScope(timeline);

	GpuAllocation writerAllocation;
	GpuAliasAcquire writerAcquire;
	B3D_TEST_ASSERT(allocator.TryAllocate(pool, 256, 16, first, writerAllocation, writerAcquire))

	const GpuTransientLastUse writerLastUse(first, MakeAccess(GpuStageFlag::Transfer, GpuAccessFlag::Write));
	allocator.Release(writerAllocation, TArrayView<const GpuTransientLastUse>(&writerLastUse, 1), &writerOwner);

	// A resource placed over the written memory, and never used
	GpuAllocation unusedAllocation;
	GpuAliasAcquire unusedAcquire;
	B3D_TEST_ASSERT(allocator.TryAllocate(pool, 256, 16, second, unusedAllocation, unusedAcquire))
	B3D_TEST_ASSERT(unusedAllocation.Offset == 0)
	B3D_TEST_ASSERT(unusedAcquire.Source.WriteStages == GpuStageFlag::Transfer)
	allocator.Release(unusedAllocation, TArrayView<const GpuTransientLastUse>(), &unusedOwner);

	// The next resource on the memory is still ordered after the write
	GpuAllocation readerAllocation;
	GpuAliasAcquire readerAcquire;
	B3D_TEST_ASSERT(allocator.TryAllocate(pool, 256, 16, second, readerAllocation, readerAcquire))
	B3D_TEST_ASSERT(readerAllocation.Offset == 0)
	B3D_TEST_ASSERT(readerAcquire.Source.WriteStages == GpuStageFlag::Transfer)

#if B3D_BUILD_TYPE_DEVELOPMENT
	B3D_TEST_ASSERT(readerAcquire.Predecessors.Size() == 1 && readerAcquire.Predecessors[0] == &writerOwner)
#endif

	// A resource that was used replaces the accesses it was placed over
	const GpuTransientLastUse readerLastUse(second, MakeAccess(GpuStageFlag::FragmentShaderNonUniform, GpuAccessFlag::Read));
	allocator.Release(readerAllocation, TArrayView<const GpuTransientLastUse>(&readerLastUse, 1), nullptr);

	GpuAllocation lastAllocation;
	GpuAliasAcquire lastAcquire;
	B3D_TEST_ASSERT(allocator.TryAllocate(pool, 256, 16, second, lastAllocation, lastAcquire))
	B3D_TEST_ASSERT(lastAllocation.Offset == 0)
	B3D_TEST_ASSERT(lastAcquire.Source.ReadStages == GpuStageFlag::FragmentShaderNonUniform)
	B3D_TEST_ASSERT(lastAcquire.Source.WriteStages == GpuStageFlag::None)

	allocator.EndScope();
}

void GpuAllocatorTestSuite::TestAliasing_Passes()
{
	const TransientHeapConfigurationScope configurationScope(kTransientHeapSizeMegabytes, 1);
	MockHeapBackend backend;
	MockGpuCompletionTracker tracker;
	TransientHeapPool pool(&backend, MakeTransientHeapPoolConfiguration(tracker));
	GpuAliasingAllocator allocator;

	const GpuQueueId graphics(GQT_GRAPHICS, 0);
	const GpuQueueId compute(GQT_COMPUTE, 0);

	// Graphics, compute without a wait, then graphics waiting on compute
	GpuSubmissionTimeline timeline;
	const u32 first = timeline.AddSubmission(graphics, GpuQueueMask::kNone);
	const u32 computeSubmission = timeline.AddSubmission(compute, GpuQueueMask::kNone);
	const u32 join = timeline.AddSubmission(graphics, GpuQueueMask(compute));

	const GpuAccessScope read = MakeAccess(GpuStageFlag::FragmentShaderNonUniform, GpuAccessFlag::Read);
	const GpuAccessScope write = MakeAccess(GpuStageFlag::ColorAttachment, GpuAccessFlag::Write);

	allocator.BeginScope(timeline);

	// Memory never used before has no pass
	GpuAllocation leftAllocation, rightAllocation;
	GpuAliasAcquire leftAcquire, rightAcquire;
	TOptional<u32> leftPass;
	B3D_TEST_ASSERT(allocator.TryAllocate(pool, 256, 16, first, leftAllocation, leftAcquire, &leftPass))
	B3D_TEST_ASSERT(allocator.TryAllocate(pool, 256, 16, first, rightAllocation, rightAcquire))
	B3D_TEST_ASSERT(leftAllocation.Offset == 0 && rightAllocation.Offset == 256)
	B3D_TEST_ASSERT(!leftPass.has_value())

	const GpuTransientLastUse leftLastUse(first, read, 3);
	const GpuTransientLastUse rightLastUse(first, write, 7);
	allocator.Release(leftAllocation, TArrayView<const GpuTransientLastUse>(&leftLastUse, 1), nullptr);
	allocator.Release(rightAllocation, TArrayView<const GpuTransientLastUse>(&rightLastUse, 1), nullptr);

	// A resource over both reports the latest of their passes
	GpuAllocation spanAllocation;
	GpuAliasAcquire spanAcquire;
	TOptional<u32> spanPass;
	B3D_TEST_ASSERT(allocator.TryAllocate(pool, 512, 16, join, spanAllocation, spanAcquire, &spanPass))
	B3D_TEST_ASSERT(spanAllocation.Offset == 0)
	B3D_TEST_ASSERT(spanPass == 7u)

	// Several last uses on one queue merge to the latest pass
	const GpuTransientLastUse spanLastUses[] = { GpuTransientLastUse(join, write, 12), GpuTransientLastUse(join, read, 9) };
	allocator.Release(spanAllocation, TArrayView<const GpuTransientLastUse>(spanLastUses, 2), nullptr);

	// Passes on other queues are ordered by queue waits, so they are not reported
	GpuAllocation computeAllocation;
	GpuAliasAcquire computeAcquire;
	B3D_TEST_ASSERT(allocator.TryAllocate(pool, 256, 16, computeSubmission, computeAllocation, computeAcquire))
	B3D_TEST_ASSERT(computeAllocation.Offset == 512)

	const GpuTransientLastUse computeLastUse(computeSubmission, MakeAccess(GpuStageFlag::ComputeShaderNonUniform, GpuAccessFlag::Write), 20);
	allocator.Release(computeAllocation, TArrayView<const GpuTransientLastUse>(&computeLastUse, 1), nullptr);

	GpuAllocation joinAllocation;
	GpuAliasAcquire joinAcquire;
	TOptional<u32> joinPass;
	B3D_TEST_ASSERT(allocator.TryAllocate(pool, 768, 16, join, joinAllocation, joinAcquire, &joinPass))
	B3D_TEST_ASSERT(joinAllocation.Offset == 0)
	B3D_TEST_ASSERT(joinPass == 12u)

	// A resource released without accesses leaves the passes it was placed over
	allocator.Release(joinAllocation, TArrayView<const GpuTransientLastUse>(), nullptr);

	GpuAllocation inheritedAllocation;
	GpuAliasAcquire inheritedAcquire;
	TOptional<u32> inheritedPass;
	B3D_TEST_ASSERT(allocator.TryAllocate(pool, 256, 16, join, inheritedAllocation, inheritedAcquire, &inheritedPass))
	B3D_TEST_ASSERT(inheritedAllocation.Offset == 0)
	B3D_TEST_ASSERT(inheritedPass == 12u)

	allocator.EndScope();
}

void GpuAllocatorTestSuite::TestAliasing_ConcurrentScopes()
{
	const TransientHeapConfigurationScope configurationScope(kTransientHeapSizeMegabytes, 1);
	MockHeapBackend backend;
	MockGpuCompletionTracker tracker;
	TransientHeapPool pool(&backend, MakeTransientHeapPoolConfiguration(tracker));

	const GpuQueueId graphics(GQT_GRAPHICS, 0);
	GpuSubmissionTimeline timeline;
	const u32 first = timeline.AddSubmission(graphics, GpuQueueMask::kNone);
	const u32 second = timeline.AddSubmission(graphics, GpuQueueMask::kNone);

	constexpr u64 kLargeSize = 768 * 1024;
	constexpr u64 kSmallSize = 512 * 1024;
	{
		GpuAliasingAllocator firstAllocator;
		GpuAliasingAllocator secondAllocator;
		firstAllocator.BeginScope(timeline);
		secondAllocator.BeginScope(timeline);

		// Each scope sees the whole heap as free, so both place at its start, with nothing to order after
		GpuAllocation firstLarge;
		GpuAliasAcquire firstLargeAcquire;
		B3D_TEST_ASSERT(firstAllocator.TryAllocate(pool, kLargeSize, 16, first, firstLarge, firstLargeAcquire))

		GpuAllocation secondLarge;
		GpuAliasAcquire secondLargeAcquire;
		B3D_TEST_ASSERT(secondAllocator.TryAllocate(pool, kLargeSize, 16, first, secondLarge, secondLargeAcquire))
		B3D_TEST_ASSERT(firstLarge.Heap == secondLarge.Heap && firstLarge.Offset == 0 && secondLarge.Offset == 0)
		B3D_TEST_ASSERT(!firstLargeAcquire.Source.IsValid() && !secondLargeAcquire.Source.IsValid())
		B3D_TEST_ASSERT(firstLargeAcquire.HasUnknownSourceAccess && secondLargeAcquire.HasUnknownSourceAccess)
		B3D_TEST_ASSERT(backend.CreateCount() == 1)

		// A scope that runs out of memory creates a heap, which the other scope then uses as well
		GpuAllocation firstSmall;
		GpuAliasAcquire firstSmallAcquire;
		B3D_TEST_ASSERT(firstAllocator.TryAllocate(pool, kSmallSize, 16, first, firstSmall, firstSmallAcquire))

		GpuAllocation secondSmall;
		GpuAliasAcquire secondSmallAcquire;
		B3D_TEST_ASSERT(secondAllocator.TryAllocate(pool, kSmallSize, 16, first, secondSmall, secondSmallAcquire))
		B3D_TEST_ASSERT(firstSmall.Heap != firstLarge.Heap && firstSmall.Heap == secondSmall.Heap && firstSmall.Offset == 0 && secondSmall.Offset == 0)
		B3D_TEST_ASSERT(backend.CreateCount() == 2)

		// Two scopes that run out of memory at the same time ask for the same slot, and only the first creates a heap
		const GpuTransientHeap firstNewHeap = pool.AcquireNewHeap(2, kSmallSize);
		const GpuTransientHeap secondNewHeap = pool.AcquireNewHeap(2, kSmallSize);
		B3D_TEST_ASSERT(firstNewHeap.Heap != nullptr && firstNewHeap.Heap == secondNewHeap.Heap)
		B3D_TEST_ASSERT(backend.CreateCount() == 3)
		pool.ReleaseHeap(firstNewHeap.Heap);
		pool.ReleaseHeap(secondNewHeap.Heap);

		// The pool holds the largest scope's memory, not the sum over scopes
		const GpuTransientHeapPoolStatistics poolStatistics = pool.GetStatistics();
		B3D_TEST_ASSERT(poolStatistics.HeapCount == 3 && poolStatistics.Capacity == 3 * kTransientHeapSize)

		const GpuTransientScopeStatistics scopeStatistics = firstAllocator.GetStatistics();
		B3D_TEST_ASSERT(scopeStatistics.PeakUsed == kLargeSize + kSmallSize && scopeStatistics.RequestedBytes == kLargeSize + kSmallSize)

		firstAllocator.EndScope();
		secondAllocator.EndScope();
	}

	// Memory released in a scope has no tag in the next scope, so the next scope's acquire has no source, and marks the memory for its
	// first use to wait for all earlier work
	GpuAliasingAllocator allocator;
	allocator.BeginScope(timeline);

	GpuAllocation writerAllocation;
	GpuAliasAcquire writerAcquire;
	B3D_TEST_ASSERT(allocator.TryAllocate(pool, 256, 16, first, writerAllocation, writerAcquire))

	const GpuTransientLastUse writerLastUse(first, MakeAccess(GpuStageFlag::Transfer, GpuAccessFlag::Write));
	allocator.Release(writerAllocation, TArrayView<const GpuTransientLastUse>(&writerLastUse, 1), nullptr);
	allocator.EndScope();

	allocator.BeginScope(timeline);

	GpuAllocation readerAllocation;
	GpuAliasAcquire readerAcquire;
	B3D_TEST_ASSERT(allocator.TryAllocate(pool, 256, 16, second, readerAllocation, readerAcquire))
	B3D_TEST_ASSERT(readerAllocation.Heap == writerAllocation.Heap && readerAllocation.Offset == 0)
	B3D_TEST_ASSERT(!readerAcquire.Source.IsValid() && readerAcquire.HasUnknownSourceAccess)

	allocator.EndScope();
}

void GpuAllocatorTestSuite::TestAliasing_ExactClaims()
{
	const TransientHeapConfigurationScope configurationScope(kTransientHeapSizeMegabytes, 1);
	MockHeapBackend backend;
	MockGpuCompletionTracker tracker;
	TransientHeapPool pool(&backend, MakeTransientHeapPoolConfiguration(tracker));
	GpuAliasingAllocator allocator;

	const GpuQueueId graphics(GQT_GRAPHICS, 0);
	const GpuQueueId compute(GQT_COMPUTE, 0);

	// Graphics, graphics, then compute without a wait
	GpuSubmissionTimeline timeline;
	const u32 first = timeline.AddSubmission(graphics, GpuQueueMask::kNone);
	const u32 second = timeline.AddSubmission(graphics, GpuQueueMask::kNone);
	const u32 computeSubmission = timeline.AddSubmission(compute, GpuQueueMask::kNone);

	GpuAllocation firstLocation;
	GpuAllocation secondLocation;
	{
		allocator.BeginScope(timeline);

		GpuAliasAcquire acquire;
		B3D_TEST_ASSERT(allocator.TryAllocate(pool, 256, 16, first, firstLocation, acquire))
		B3D_TEST_ASSERT(allocator.TryAllocate(pool, 256, 16, first, secondLocation, acquire))
		B3D_TEST_ASSERT(firstLocation.Offset == 0 && secondLocation.Offset == 256)

		const GpuTransientLastUse writerLastUse(first, MakeAccess(GpuStageFlag::Transfer, GpuAccessFlag::Write));
		allocator.Release(firstLocation, TArrayView<const GpuTransientLastUse>(&writerLastUse, 1), nullptr);

		// A claim over released memory returns the accesses to order after
		GpuAllocation claim;
		GpuAliasAcquire claimAcquire;
		B3D_TEST_ASSERT(allocator.TryAllocateAt(pool, firstLocation, second, claim, claimAcquire))
		B3D_TEST_ASSERT(claim.Heap == firstLocation.Heap && claim.Offset == 0 && claim.Size == 256)
		B3D_TEST_ASSERT(claimAcquire.Source.WriteStages == GpuStageFlag::Transfer)

		// A claim over memory in use fails
		GpuAllocation occupiedClaim;
		GpuAliasAcquire occupiedAcquire;
		B3D_TEST_ASSERT(!allocator.TryAllocateAt(pool, secondLocation, second, occupiedClaim, occupiedAcquire))

		// A claim over memory whose last use is not ordered before the first use fails
		const GpuTransientLastUse readerLastUse(second, MakeAccess(GpuStageFlag::FragmentShaderNonUniform, GpuAccessFlag::Read));
		allocator.Release(claim, TArrayView<const GpuTransientLastUse>(&readerLastUse, 1), nullptr);

		GpuAllocation unorderedClaim;
		GpuAliasAcquire unorderedAcquire;
		B3D_TEST_ASSERT(!allocator.TryAllocateAt(pool, firstLocation, computeSubmission, unorderedClaim, unorderedAcquire))

		allocator.EndScope();
	}

	// A claim over a heap the scope has not used yet has nothing to order after, and the rest of the heap stays free around it
	{
		allocator.BeginScope(timeline);

		GpuAllocation claim;
		GpuAliasAcquire claimAcquire;
		B3D_TEST_ASSERT(allocator.TryAllocateAt(pool, secondLocation, first, claim, claimAcquire))
		B3D_TEST_ASSERT(claim.Heap == secondLocation.Heap && claim.Offset == 256)
		B3D_TEST_ASSERT(!claimAcquire.Source.IsValid())

		GpuAllocation before;
		GpuAliasAcquire beforeAcquire;
		B3D_TEST_ASSERT(allocator.TryAllocate(pool, 256, 16, first, before, beforeAcquire))
		B3D_TEST_ASSERT(before.Heap == claim.Heap && before.Offset == 0)

		GpuAllocation after;
		GpuAliasAcquire afterAcquire;
		B3D_TEST_ASSERT(allocator.TryAllocate(pool, 512, 16, first, after, afterAcquire))
		B3D_TEST_ASSERT(after.Heap == claim.Heap && after.Offset == 512)

		allocator.EndScope();
	}

	B3D_TEST_ASSERT(backend.CreateCount() == 1)
}

void GpuAllocatorTestSuite::TestAliasing_SegmentsAndGranularity()
{
	const TransientHeapConfigurationScope configurationScope(kTransientHeapSizeMegabytes, 1);
	const GpuQueueId graphics(GQT_GRAPHICS, 0);
	GpuSubmissionTimeline timeline;
	const u32 submission = timeline.AddSubmission(graphics, GpuQueueMask::kNone);
	const TArrayView<const GpuTransientLastUse> noUses;

	// Alignment pads the start of a placement, and the padding stays free
	{
		MockHeapBackend backend;
		MockGpuCompletionTracker tracker;
		TransientHeapPool pool(&backend, MakeTransientHeapPoolConfiguration(tracker));
		GpuAliasingAllocator allocator;
		allocator.BeginScope(timeline);

		GpuAllocation firstAllocation;
		GpuAliasAcquire firstAcquire;
		B3D_TEST_ASSERT(allocator.TryAllocate(pool, 256, 16, submission, firstAllocation, firstAcquire))

		GpuAllocation alignedAllocation;
		GpuAliasAcquire alignedAcquire;
		B3D_TEST_ASSERT(allocator.TryAllocate(pool, 512, 512, submission, alignedAllocation, alignedAcquire))
		B3D_TEST_ASSERT(alignedAllocation.Offset == 512)

		GpuAllocation paddingAllocation;
		GpuAliasAcquire paddingAcquire;
		B3D_TEST_ASSERT(allocator.TryAllocate(pool, 256, 16, submission, paddingAllocation, paddingAcquire))
		B3D_TEST_ASSERT(paddingAllocation.Offset == 256)

		// Released neighbors coalesce, so the whole heap fits one allocation again
		allocator.Release(paddingAllocation, noUses, nullptr);
		allocator.Release(firstAllocation, noUses, nullptr);
		allocator.Release(alignedAllocation, noUses, nullptr);

		GpuAllocation wholeAllocation;
		GpuAliasAcquire wholeAcquire;
		B3D_TEST_ASSERT(allocator.TryAllocate(pool, kTransientHeapSize, 16, submission, wholeAllocation, wholeAcquire))
		B3D_TEST_ASSERT(wholeAllocation.Offset == 0 && wholeAllocation.Heap == firstAllocation.Heap)
		B3D_TEST_ASSERT(backend.CreateCount() == 1)

		// Allocations that do not fit an existing heap create a new one, sized to fit them
		GpuAllocation overflowAllocation;
		GpuAliasAcquire overflowAcquire;
		B3D_TEST_ASSERT(allocator.TryAllocate(pool, 2 * kTransientHeapSize, 16, submission, overflowAllocation, overflowAcquire))
		B3D_TEST_ASSERT(overflowAllocation.Offset == 0 && overflowAllocation.Heap != wholeAllocation.Heap)
		B3D_TEST_ASSERT(backend.CreateCount() == 2)
		B3D_TEST_ASSERT(static_cast<MockGpuHeap*>(overflowAllocation.Heap)->Size == 2 * kTransientHeapSize)

		allocator.EndScope();
	}

	// Granularity rounds offsets and sizes
	{
		MockHeapBackend backend;
		MockGpuCompletionTracker tracker;
		TransientHeapPool::Configuration configuration = MakeTransientHeapPoolConfiguration(tracker);
		configuration.Granularity = 1024;

		TransientHeapPool pool(&backend, configuration);
		GpuAliasingAllocator allocator;
		allocator.BeginScope(timeline);

		GpuAllocation firstAllocation;
		GpuAliasAcquire firstAcquire;
		B3D_TEST_ASSERT(allocator.TryAllocate(pool, 100, 16, submission, firstAllocation, firstAcquire))
		B3D_TEST_ASSERT(firstAllocation.Offset == 0 && firstAllocation.Size == 1024)

		GpuAllocation secondAllocation;
		GpuAliasAcquire secondAcquire;
		B3D_TEST_ASSERT(allocator.TryAllocate(pool, 1, 1, submission, secondAllocation, secondAcquire))
		B3D_TEST_ASSERT(secondAllocation.Offset == 1024 && secondAllocation.Size == 1024)

		allocator.EndScope();
	}

	// A backend that cannot create heaps fails the allocation
	{
		OutOfMemoryHeapBackend backend;
		MockGpuCompletionTracker tracker;

		TGpuTransientHeapPool<OutOfMemoryHeapBackend>::Configuration configuration;
		configuration.CompletionTracker = &tracker;

		TGpuTransientHeapPool<OutOfMemoryHeapBackend> pool(&backend, configuration);
		GpuAliasingAllocator allocator;
		allocator.BeginScope(timeline);

		GpuAllocation allocation;
		GpuAliasAcquire acquire;
		B3D_TEST_ASSERT(!allocator.TryAllocate(pool, 256, 16, submission, allocation, acquire))
		B3D_TEST_ASSERT(!allocation.HasMemory())
		B3D_TEST_ASSERT(backend.CreateCallCount == 1)

		allocator.EndScope();
	}
}

void GpuAllocatorTestSuite::TestTransientHeapPool_ReleaseAndStatistics()
{
	const TransientHeapConfigurationScope configurationScope(kTransientHeapSizeMegabytes, 2);
	MockHeapBackend backend;
	MockGpuCompletionTracker tracker;
	TransientHeapPool pool(&backend, MakeTransientHeapPoolConfiguration(tracker));

	// Ends a frame the way GpuDevice::EndFrame() does
	auto fnEndFrame = [&pool, &tracker]()
	{
		pool.ReclaimUnused();
		tracker.AdvanceFrame();
	};

	// A referenced heap is kept over any number of frames
	const GpuTransientHeap heap = pool.AcquireNewHeap(0, 1024);
	B3D_TEST_ASSERT(heap.Heap != nullptr && heap.Size == kTransientHeapSize)
	B3D_TEST_ASSERT(pool.GetSlotCount() == 1)

	GpuTransientHeapPoolStatistics statistics = pool.GetStatistics();
	B3D_TEST_ASSERT(statistics.HeapCount == 1 && statistics.Capacity == kTransientHeapSize)

	for(u32 frameIndex = 0; frameIndex < 4; frameIndex++)
		fnEndFrame();

	B3D_TEST_ASSERT(backend.LiveHeapCount() == 1)

	// Only a heap at least as large as asked for is acquired. A slot without a heap reports no size.
	const GpuTransientHeap tooSmall = pool.AcquireHeap(0, 2 * kTransientHeapSize);
	B3D_TEST_ASSERT(tooSmall.Heap == nullptr && tooSmall.Size == kTransientHeapSize)

	const GpuTransientHeap missing = pool.AcquireHeap(3, 1024);
	B3D_TEST_ASSERT(missing.Heap == nullptr && missing.Size == 0)

	// Every reference keeps the heap
	pool.AddHeapReference(heap.Heap);
	pool.ReleaseHeap(heap.Heap);
	for(u32 frameIndex = 0; frameIndex < 4; frameIndex++)
		fnEndFrame();

	B3D_TEST_ASSERT(pool.GetStatistics().HeapCount == 1)

	// Without references, the heap is released once it went unused for the idle frames
	pool.ReleaseHeap(heap.Heap);
	fnEndFrame();
	fnEndFrame();
	B3D_TEST_ASSERT(pool.GetStatistics().HeapCount == 1)

	pool.ReclaimUnused();
	statistics = pool.GetStatistics();
	B3D_TEST_ASSERT(statistics.HeapCount == 0 && statistics.Capacity == 0)
	B3D_TEST_ASSERT(pool.GetSlotCount() == 0)

	// The released heap is destroyed once the GPU has finished the frame that last used it
	B3D_TEST_ASSERT(backend.DestroyCount() == 0 && backend.LiveHeapCount() == 1)

	tracker.MarkAllFramesComplete();
	pool.ReclaimUnused();
	B3D_TEST_ASSERT(backend.DestroyCount() == 1 && backend.LiveHeapCount() == 0)

	// A later heap reuses the empty slot
	const GpuTransientHeap nextHeap = pool.AcquireNewHeap(0, 1024);
	B3D_TEST_ASSERT(nextHeap.Heap != nullptr && pool.GetSlotCount() == 1)
	B3D_TEST_ASSERT(backend.CreateCount() == 2)

	// A heap whose last frame of use the GPU already finished is destroyed as soon as it is released
	const u64 lastUsedFrame = tracker.CurrentFrameIndex();
	pool.ReleaseHeap(nextHeap.Heap);
	fnEndFrame();
	fnEndFrame();

	tracker.MarkFrameComplete(lastUsedFrame);
	pool.ReclaimUnused();
	B3D_TEST_ASSERT(pool.GetStatistics().HeapCount == 0)
	B3D_TEST_ASSERT(backend.DestroyCount() == 2 && backend.LiveHeapCount() == 0)
}
