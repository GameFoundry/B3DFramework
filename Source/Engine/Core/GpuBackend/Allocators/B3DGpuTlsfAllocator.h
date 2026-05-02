//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DPrerequisites.h"
#include "GpuBackend/Allocators/B3DGpuAllocator.h"
#include "GpuBackend/B3DGpuTimelineFence.h"
#include "Utility/B3DBitwise.h"
#include "Utility/B3DFlags.h"

namespace b3d
{
	/** @addtogroup GpuBackend
	 *  @{
	 */

	/**
	 * Helper used by the TLSF allocator to ensure that non-linear images are placed at correct alignment
	 * (granularity). Some backends have different alignment requirements if a non-linear image follows or
	 * trails a linear memory allocation, which this object helps to track.
	 *
	 * One entry per granularity-aligned page. Only the start and end pages of each allocation are ever referenced.
	 *
	 * Default-constructed instances are inert (no allocation). Non-copyable.
	 */
	class TlsfGranularityTracker
	{
	public:
		TlsfGranularityTracker() = default;
		~TlsfGranularityTracker()											{ Destroy(); }

		TlsfGranularityTracker(const TlsfGranularityTracker&)				= delete;
		TlsfGranularityTracker& operator=(const TlsfGranularityTracker&)	= delete;

		/**
		 * Allocate the page table sized for @p heapSize. When @p granularity is <= 1 or
		 * <= @p disableThreshold the tracker stays inert — every call below short-circuits.
		 * @p disableThreshold is useful if allocations are guaranteed to be aligned to this
		 * value regardless of buffer-image granularity.
		 */
		void Initialize(u64 heapSize, u64 granularity, u64 disableThreshold);

		/** Releases the page table. Safe to call on an inert tracker. */
		void Destroy();

		/** True when the page table is allocated and the conflict checks are live. */
		bool IsEnabled() const { return mPages != nullptr; }

		/** Bumps the refcounts for the start + end pages of @p [offset, offset+size). */
		void MarkPages(u64 offset, u64 size, GpuResourceKind kind);

		/** Decrements the refcounts for the start + end pages; resets category to Free at zero. */
		void UnmarkPages(u64 offset, u64 size);

		/**
		 * Adjust @p inOutOffset upward to clear any granularity conflict at the start page;
		 * return false if the adjusted range overruns @p blockEnd or the end page holds
		 * a conflicting allocation. Returns true (no-op) when the tracker is inert.
		 */
		bool CheckAndAlignUp(u64& inOutOffset, u64 size, GpuResourceKind kind, u64 blockEnd) const;

#if B3D_DEBUG
		/** Asserts every page has zero LiveCount — sanity check when a heap goes empty. */
		void AssertEmpty() const;
#endif

	private:
		/** Resource-kind category stored per granularity page. */
		enum class PageCategory : u8
		{
			Linear = (u8)GpuResourceKind::Linear,
			NonLinear = (u8)GpuResourceKind::NonLinear,
			Free = 0xFF, /**< Sentinel value for an empty page (no live allocations touch it). */
		};

		/** Describes one page (memory range as wide as the granularity) and its category. */
		struct Page
		{
			PageCategory Category; /**< PageCategory::Free when no live allocation touches this page. */
			u16 LiveCount; /**< Number of allocations touching this page. */
		};

		/** Returns true if two categories cannot exist in the same granularity page. */
		static bool IsConflict(PageCategory a, PageCategory b)
		{
			if (a == PageCategory::Free || b == PageCategory::Free)
				return false;

			return a != b;
		}

		Page* mPages = nullptr;
		u32 mPageCount = 0;
		u64 mGranularity = 1;
		u32 mPageShift = 0;
	};

	/**
	 * Two-Level Segregated Fit GPU memory allocator. O(1) bitmap-driven bucket lookup, leading-padding
	 * split for alignment, full coalescing on free, multi-heap growable. One allocator instance manages
	 * a list of backend heaps; allocations report back to the consumer via TGpuResourceLocation, with
	 * the heap index and pool node index stored in the location's two strategy-private slots.
	 *
	 * **Threading.** Single-threaded by contract —  Caller is responsible for external synchronization
	 * if the same instance is shared between threads.
	 *
	 * **Buffer-image granularity.** A single allocator instance can host mixed linear (buffer / linear image)
	 * and non-linear (optimally-tiled image) allocations safely; pass the appropriate GpuResourceKind to
	 * TryAllocate. The configured BufferImageGranularity  drives the mandatory padding between conflicting
	 * neighbors. When the configured granularity is at or below GranularityDisableThreshold the tracker
	 * is fully inert and adds zero per-allocation overhead.
	 *
	 * @tparam HeapBackend	Backend trait satisfying the GpuHeapBackend contract.
	 *
	 * @see TGpuAllocator
	 */
	template <typename HeapBackend>
	class TGpuTlsfAllocator : public TGpuAllocator<TGpuTlsfAllocator<HeapBackend>, HeapBackend>
	{
	public:
		using Base = TGpuAllocator<TGpuTlsfAllocator<HeapBackend>, HeapBackend>;
		using Location = typename Base::Location;
		using HeapHandle = typename HeapBackend::HeapHandle;

	private:
		// First-level class count. Capped at u32 bitmap width — covers heaps up to 2^(kFirstLevelClassCount + kMemoryClassShift) bytes.
		static constexpr u32 kFirstLevelClassCount = 32;

		// Number of low bits removed from MSB(size) when computing the first-level class for sizes > kSmallBufferSize.
		static constexpr u32 kMemoryClassShift = 7;

	public:
		/** Runtime configuration for the allocator. */
		struct Configuration
		{
			/** Size of the first heap created on demand. */
			u64 InitialHeapSize = 64ull * 1024 * 1024;

			/**
			 * Maximum size for any single heap. Single allocations larger than this are placed in dedicated heaps.
			 * Hard upper bound: 2^(kFirstLevelClassCount + kMemoryClassShift) bytes (~549 GB) — sizes beyond
			 * that exceed the bitmap's first-level class capacity.
			 */
			u64 MaxHeapSize = 256ull * 1024 * 1024;

			/** Each new heap is sized min(previousSize * GrowthFactor, MaxHeapSize). */
			u32 GrowthFactor = 2;

			/** Number of fully-empty heaps to retain as warm spares before destroying further empties. */
			u32 MaxEmptyHeapCount = 1;

			/** Allocations smaller than this are rounded up — keeps tiny allocations from over-fragmenting the small bucket. */
			u64 MinAllocationSize = 16;

			/**
			 * Buffer-image granularity in bytes (e.g. Vulkan VkPhysicalDeviceLimits). Linear and
			 * non-linear allocations sharing one heap are guaranteed to be separated by at least this
			 * many bytes. Default 1 disables buffer image granularity handling at zero cost.
			 * Must be a power of two when > 1.
			 */
			u64 BufferImageGranularity = 1;

			/**
			 * Skip the per-heap region table when BufferImageGranularity is at or below this threshold.
			 * At small granularities the natural alignment of typical buffers (>= 256 B for UBO/SSBO bindings)
			 * implicitly satisfies the constraint, so the tracker's memory cost is wasted. Set to 0 to track every granularity > 1.
			 */
			u64 GranularityDisableThreshold = 256;

			/** Backend create-info forwarded verbatim to HeapBackend::CreateHeap on each grow. */
			typename HeapBackend::HeapCreateInformation HeapCreateInfo{};
		};

		TGpuTlsfAllocator(HeapBackend* backend, IGpuSubmissionTracker* submissionTracker, const Configuration& configuration);
		~TGpuTlsfAllocator();

		// Non-copyable — node pool and heap state are not safe to duplicate.
		TGpuTlsfAllocator(const TGpuTlsfAllocator&) = delete;
		TGpuTlsfAllocator& operator=(const TGpuTlsfAllocator&) = delete;

		/** @name TGpuAllocator CRTP surface.
		 *  @{
		 */

		bool TryAllocateImpl(u64 size, u32 alignment, GpuResourceKind kind, IGpuResource* owner, Location& out);
		void DeallocateImpl(Location& allocation);
		void FreeImmediateImpl(u32 heapIndex, u32 nodeIndex);

		/** @} */

		/** @name Diagnostics.
		 *  @{
		 */

		/** Total number of bytes allocated by all underlying heaps. */
		u64 GetCommittedBytes() const;

		/** Total number of bytes currently held by live (non-retired, non-freed) allocations. */
		u64 GetUsedBytes() const;

		/** Number of populated heap slots (vacated slots are not counted). */
		u32 GetHeapCount() const;

		/** Number of fully-empty heaps currently retained as spares. */
		u32 GetEmptyHeapCount() const;

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
		 * Compacts live allocations by moving them into lower-offset / lower-index slots. The pass
		 * walks heaps high-index → low-index and within each heap follows the per-heap physical
		 * chain offset-high → offset-low. Every tracked allocation (non-null owner) is a candidate.
		 *
		 * For each successful move the consumer's IGpuResource::OnAllocationMoved is invoked with
		 * the next-not-yet-issued submission index, @p commandBuffer, and a fresh
		 * TGpuResourceLocation<HeapBackend> for the destination slot; the consumer records the
		 * GPU copy from its still-intact source location to the destination via @p commandBuffer,
		 * queues the old backend object for deferred-destroy on the supplied submission index,
		 * replaces its own TGpuResourceLocation with the new one, and recreates the placed
		 * backend object at the new memory range. The source slot is retired against the upcoming
		 * submission index and freed when the deferred-free queue drains.
		 *
		 * **Submission-ordering contract.** The caller must submit @p commandBuffer next with no
		 * intervening submissions. Pre-recorded command buffers that reference the OLD backend
		 * object must already have been submitted before Defrag() is called.
		 *
		 * @param commandBuffer	Command buffer the consumer's recreate-and-copy path records into.
		 * @param info			Soft per-call budgets.
		 * @return				Counters for moves attempted / completed and budget-exhausted state.
		 */
		DefragmentationStats Defrag(render::GpuCommandBuffer& commandBuffer, const DefragmentationInfo& info = {});

		/** @} */

	private:
		/** Sentinel index for "no node" / "end of list" — stored in physical / free-list link fields. */
		static constexpr u32 kInvalidIndex = ~0u;

		/** Sizes <= this are bucketed entirely within first-level class 0. */
		static constexpr u64 kSmallBufferSize = 256;

		/** Granule width for second-level buckets inside first-level class 0. */
		static constexpr u32 kSmallBufferGranule = 8;

		/** log2 of the second-level bucket count per first-level class. */
		static constexpr u32 kSecondLevelIndexBits = 5;

		/** Second-level buckets per first-level class. */
		static constexpr u32 kSecondLevelCount = 1u << kSecondLevelIndexBits;

		/** Total free-list bucket count per heap. */
		static constexpr u32 kFreeListCount = kFirstLevelClassCount * kSecondLevelCount;

		// FirstLevelFreeBitmask is a u32; if the first-level class count grows past 32 the bitmap type must widen.
		static_assert(kFirstLevelClassCount <= 32, "FirstLevelFreeBitmask is u32; widen the bitmap if more first-level classes are required");

		// Likewise for SecondLevelFreeBitmask[firstLevel].
		static_assert(kSecondLevelCount <= 32, "SecondLevelFreeBitmask entries are u32; widen the type if more second-level buckets are required");

		/** State bits stored on each pool node. */
		enum class NodeFlag : u32
		{
			Free				= 1u << 0, /**< Set when the node is on a free list (or is the trailing null node). */
			NullNode			= 1u << 1, /**< Set when the node is the trailing null node of its heap. */
			NonLinear			= 1u << 2, /**< Set when a live allocation is non-linear (optimally-tiled image). */
			DefragDestination	= 1u << 3, /**< Set on slots reserved as defrag destinations within the current Defrag() pass; cleared at end of Defrag(). */
		};

		using NodeFlags = Flags<NodeFlag, u32>;

		/**
		 * Pool node describing a contiguous range within one heap. Indexed by u32 so node
		 * identity fits in TGpuResourceLocation::AllocatorData1.
		 */
		struct Node
		{
			u64 Offset;
			u64 Size;

			// Heap-order doubly-linked list. kInvalidIndex at the heap start / end.
			u32 PrevPhysical;
			u32 NextPhysical;

			// Free-list doubly-linked list when the node is free; unused otherwise.
			u32 PrevFree;
			u32 NextFree;

			NodeFlags Flags;

			IGpuResource* Owner; /**< Owning resource for defragmentation. nullptr when the slot is untracked or free. */

			bool IsFree() const { return Flags.IsSet(NodeFlag::Free); }
			bool IsNullNode() const { return Flags.IsSet(NodeFlag::NullNode); }
			bool IsDefragDestination() const { return Flags.IsSet(NodeFlag::DefragDestination); }
		};

		/** Per-heap TLSF metadata. */
		struct HeapState
		{
			typename HeapBackend::HeapHandle Heap{};
			u64 TotalSize = 0;
			u64 FreeSize = 0;
			u32 LiveAllocCount = 0;
			u32 PhysicalListHead = kInvalidIndex;
			u32 NullNodeIndex = kInvalidIndex;

			u32 FreeListHead[kFreeListCount]; /**< Free-list head per (firstLevel, secondLevel) bucket. Updated alongside the bitmaps. */
			u32 FirstLevelFreeBitmask = 0; /**< Bit set if any entry in SecondLevelFreeBitmask[firstLevel] is non-zero. */
			u32 SecondLevelFreeBitmask[kFirstLevelClassCount]; /**< Bit set when FreeListHead[(firstLevel, secondLevel)] is non-empty. */

			TlsfGranularityTracker Granularity; /**< Buffer-image granularity tracker — inert when the allocator is configured with granularity <= 1 or below the threshold. */

			HeapState()
			{
				for (u32 listIndex = 0; listIndex < kFreeListCount; listIndex++)
					FreeListHead[listIndex] = kInvalidIndex;

				for (u32 firstLevel = 0; firstLevel < kFirstLevelClassCount; firstLevel++)
					SecondLevelFreeBitmask[firstLevel] = 0;
			}
		};

		/** Destination slot reserved by TryAllocateInHeapsAtMost for a single defrag move. */
		struct DefragDestinationSlot
		{
			u32 HeapIndex = kInvalidIndex;
			u32 NodeIndex = kInvalidIndex;
			u64 Offset = 0;
		};

		/**
		 * Maps an allocation size to a (firstLevel, secondLevel) bucket. The first-level class is the MSB-derived
		 * size order, the second-level class linearly subdivides each first-level range into kSecondLevelCount sub-buckets.
		 *
		 * For sizes in [1, kSmallBufferSize] the first-level class is forced to 0 and the second-level index is
		 * derived from kSmallBufferGranule-byte granules so small allocations stay segregated below the natural
		 * MSB-class boundaries.
		 */
		static void SizeToBucket(u64 size, u32& firstLevel, u32& secondLevel);

		/** Flat free-list index for a (firstLevel, secondLevel) bucket. */
		static u32 GetListIndex(u32 firstLevel, u32 secondLevel);

		/** Round @p value up to the next multiple of @p alignment (which must be a power of two). */
		static u64 AlignUp(u64 value, u32 alignment);

		/** Acquire a free node-pool slot, growing the pool if necessary. */
		u32 AllocateNode();

		/** Return a node-pool slot to the free list. */
		void ReleaseNode(u32 nodeIndex);

		/** Insert @p nodeIndex into the appropriate (firstLevel, secondLevel) bucket of @p heap and update bitmaps. */
		void InsertIntoFreeList(HeapState& heap, u32 nodeIndex);

		/** Splice @p nodeIndex out of its free list and clear bitmap bits if its bucket is now empty. */
		void RemoveFromFreeList(HeapState& heap, u32 nodeIndex);

		/**
		 * Find a free node in @p heap that can satisfy a (size, alignment, kind) request. Searches the
		 * natural bucket first (best-fit candidates live there) and walks larger buckets via the bitmaps
		 * if needed. The returned @p outAlignedOffset folds in both natural alignment and any buffer image granularity
		 * inflation, so the carver doesn't have to recompute either. Returns kInvalidIndex on miss.
		 */
		u32 FindFreeNode(const HeapState& heap, u64 size, u32 alignment, GpuResourceKind kind, u64& outAlignedOffset) const;

		/**
		 * Walk a bucket's free list and return the first node large enough to satisfy (size, alignment, kind).
		 * The returned @p outAlignedOffset contains any buffer image granularity past natural alignment.
		 */
		u32 WalkBucketForFit(const HeapState& heap, u32 listIndex, u64 size, u32 alignment, GpuResourceKind kind, u64& outAlignedOffset) const;

		/**
		 * Carve a @p size byte allocation starting at @p alignedOffset out of the candidate node, splitting
		 * any leading padding and trailing remainder into separate free nodes. Returns the node-index of the allocated block.
		 */
		u32 CarveAllocation(HeapState& heap, u32 candidateIndex, u64 alignedOffset, u64 size);

		/**
		 * Reserves a slot of @p size bytes within @p heap: fast-fail on insufficient size, walk
		 * the TLSF buckets for a fitting free node, carve, mark the granularity pages, update heap
		 * bookkeeping (FreeSize, LiveAllocCount, mEmptyHeapCount). On success writes the carved
		 * node's index to @p outNodeIndex; the caller reads mNodes[outNodeIndex] for offset and
		 * final size. Shared by the Location-writing overload below and by TryAllocateInHeapsAtMost;
		 * neither stamps additional state (Owner, Location-typed output) — the caller is responsible for that.
		 */
		bool TryAllocateInHeap(HeapState& heap, u64 size, u32 alignment, GpuResourceKind kind, u32& outNodeIndex);

		/**
		 * Search-and-carve combined: returns true on hit and populates @p out, false on miss. Stamps
		 * @p owner onto the carved node for defragmentation tracking; pass nullptr for an untracked
		 * allocation that won't participate in defrag.
		 */
		bool TryAllocateInHeap(HeapState& heap, u32 heapIndex, u64 size, u32 alignment, GpuResourceKind kind, IGpuResource* owner, Location& out);

		/** Create a fresh heap and install it into @c mHeaps, reusing a vacated slot if one is available. */
		u32 CreateNewHeap(u64 sizeInBytes);

		/** Destroy heap @p heapIndex and vacate its slot. Caller has verified LiveAllocCount == 0. */
		void DestroyHeap(u32 heapIndex);

		/**
		 * Variant of TryAllocateImpl that limits heap iteration to indices @p maxHeapIndexInclusive
		 * and below, and never grows a fresh heap. Used during defrag so destinations can land in the
		 * same heap as the source (within-heap compaction) or any lower-index heap (drain), but never
		 * cause the allocator to expand committed memory.
		 */
		bool TryAllocateInHeapsAtMost(u64 size, u32 alignment, GpuResourceKind kind, u32 maxHeapIndexInclusive, DefragDestinationSlot& out);

		/**
		 * Reserves a destination slot for the live allocation at @p sourceNodeIndex, dispatches the
		 * consumer's OnAllocationMoved with the typed move context, and retires the source slot
		 * against @p submissionIndex. Returns true on a successful move and writes the chosen
		 * destination node index to @p outDestinationNodeIndex; false if no destination slot was
		 * available within @p sourceHeapIndex inclusive.
		 */
		bool TryMoveAllocation(u32 sourceNodeIndex, u32 sourceHeapIndex, render::GpuCommandBuffer& commandBuffer, u64 submissionIndex, u32& outDestinationNodeIndex);

		Configuration mConfig;
		Vector<HeapState*> mHeaps;
		Vector<Node> mNodes;
		u32 mNodeFreeHead = kInvalidIndex;
		u32 mEmptyHeapCount = 0;
		u64 mNextHeapSize = 0;
	};

	template <typename HeapBackend>
	TGpuTlsfAllocator<HeapBackend>::TGpuTlsfAllocator(HeapBackend* backend, IGpuSubmissionTracker* submissionTracker, const Configuration& configuration)
		: Base(backend, submissionTracker), mConfig(configuration), mNextHeapSize(configuration.InitialHeapSize)
	{
		B3D_ASSERT(mConfig.GrowthFactor >= 1);
		B3D_ASSERT(mConfig.InitialHeapSize > 0);
		B3D_ASSERT(mConfig.MaxHeapSize >= mConfig.InitialHeapSize);
		B3D_ASSERT(mConfig.MinAllocationSize > 0);
		B3D_ASSERT(mConfig.BufferImageGranularity == 1 || Bitwise::IsPow2(mConfig.BufferImageGranularity));
		// Guards the bitmap-width constraint — sizes whose MSB exceeds this cap can't be bucketed.
		B3D_ASSERT(mConfig.MaxHeapSize < (1ull << (kFirstLevelClassCount + kMemoryClassShift)));
	}

	template <typename HeapBackend>
	TGpuTlsfAllocator<HeapBackend>::~TGpuTlsfAllocator()
	{
		// Drain unconditionally — any submissions still in flight at destructor time are the caller's
		// responsibility to wait for via WaitUntilIdle, matching the convention from TGpuAllocator.
		Base::Flush(0, true);

		for (u32 heapIndex = 0; heapIndex < (u32)mHeaps.size(); heapIndex++)
		{
			if (mHeaps[heapIndex] != nullptr)
			{
				Base::mBackend->DestroyHeap(mHeaps[heapIndex]->Heap);
				B3DDelete(mHeaps[heapIndex]);
				mHeaps[heapIndex] = nullptr;
			}
		}
	}

	template <typename HeapBackend>
	bool TGpuTlsfAllocator<HeapBackend>::TryAllocateImpl(u64 size, u32 alignment, GpuResourceKind kind, IGpuResource* owner, Location& out)
	{
		B3D_ASSERT(out.Allocator == nullptr);
		B3D_ASSERT(alignment > 0);
		B3D_ASSERT(Bitwise::IsPow2(alignment));

		const u64 requestedSize = std::max(size, mConfig.MinAllocationSize);

		// Try existing heaps oldest-first so empty-spare slots drain before any new heap is created.
		for (u32 heapIndex = 0; heapIndex < (u32)mHeaps.size(); heapIndex++)
		{
			HeapState* heap = mHeaps[heapIndex];
			if (heap == nullptr)
				continue;

			if (TryAllocateInHeap(*heap, heapIndex, requestedSize, alignment, kind, owner, out))
				return true;
		}

		// All existing heaps full — grow.
		const u64 newHeapSize = std::max(requestedSize, mNextHeapSize);
		const u32 newHeapIndex = CreateNewHeap(newHeapSize);
		if (newHeapIndex == kInvalidIndex)
			return false;

		HeapState& fresh = *mHeaps[newHeapIndex];
		const bool ok = TryAllocateInHeap(fresh, newHeapIndex, requestedSize, alignment, kind, owner, out);
		B3D_ASSERT(ok); // A fresh heap big enough for the request must satisfy it.

		return ok;
	}

	template <typename HeapBackend>
	void TGpuTlsfAllocator<HeapBackend>::DeallocateImpl(Location& allocation)
	{
		B3D_ASSERT(allocation.Allocator == this);
		B3D_ASSERT(allocation.AllocatorData0 < (u32)mHeaps.size());
		Base::RetireAllocation(allocation);
	}

	template <typename HeapBackend>
	void TGpuTlsfAllocator<HeapBackend>::FreeImmediateImpl(u32 heapIndex, u32 nodeIndex)
	{
		B3D_ASSERT(heapIndex < (u32)mHeaps.size());
		HeapState* heap = mHeaps[heapIndex];
		B3D_ASSERT(heap != nullptr);
		B3D_ASSERT(nodeIndex < (u32)mNodes.size());

		Node& node = mNodes[nodeIndex];
		B3D_ASSERT(!node.IsFree());

		heap->Granularity.UnmarkPages(node.Offset, node.Size);

		heap->FreeSize += node.Size;
		heap->LiveAllocCount--;
		node.Owner = nullptr;

		// Coalesce with the previous physical neighbor when it's free and not the null block.
		u32 mergedNodeIndex = nodeIndex;
		if (node.PrevPhysical != kInvalidIndex)
		{
			Node& previousNode = mNodes[node.PrevPhysical];
			if (previousNode.IsFree() && !previousNode.IsNullNode())
			{
				RemoveFromFreeList(*heap, node.PrevPhysical);
				previousNode.Size += node.Size;
				previousNode.NextPhysical = node.NextPhysical;
				if (node.NextPhysical != kInvalidIndex)
					mNodes[node.NextPhysical].PrevPhysical = node.PrevPhysical;

				mergedNodeIndex = node.PrevPhysical;
				ReleaseNode(nodeIndex);
			}
		}

		// Coalesce with the next physical neighbor.
		Node& mergedNode = mNodes[mergedNodeIndex];
		if (mergedNode.NextPhysical != kInvalidIndex)
		{
			const u32 nextNodeIndex = mergedNode.NextPhysical;
			Node& nextNode = mNodes[nextNodeIndex];

			if (nextNode.IsNullNode())
			{
				// Fold our newly-freed range into the trailing null block. The merged node (if it isn't
				// the null block itself) is released back to the pool; the null block keeps its identity.
				nextNode.Offset = mergedNode.Offset;
				nextNode.Size += mergedNode.Size;
				nextNode.PrevPhysical = mergedNode.PrevPhysical;

				if (mergedNode.PrevPhysical != kInvalidIndex)
					mNodes[mergedNode.PrevPhysical].NextPhysical = nextNodeIndex;
				else
					heap->PhysicalListHead = nextNodeIndex;

				ReleaseNode(mergedNodeIndex);
				mergedNodeIndex = nextNodeIndex;
			}
			else if (nextNode.IsFree())
			{
				RemoveFromFreeList(*heap, nextNodeIndex);
				mergedNode.Size += nextNode.Size;
				mergedNode.NextPhysical = nextNode.NextPhysical;
				if (nextNode.NextPhysical != kInvalidIndex)
					mNodes[nextNode.NextPhysical].PrevPhysical = mergedNodeIndex;

				ReleaseNode(nextNodeIndex);
			}
		}

		// Insert the resulting node into its bucket. The null block does not participate in the free lists.
		Node& finalNode = mNodes[mergedNodeIndex];
		finalNode.Flags |= NodeFlag::Free;
		if (!finalNode.IsNullNode())
			InsertIntoFreeList(*heap, mergedNodeIndex);

		// Release a fully-empty heap if we're already over the spare budget.
		if (heap->LiveAllocCount == 0)
		{
			B3D_DEBUG_ONLY(heap->Granularity.AssertEmpty());

			if (mEmptyHeapCount < mConfig.MaxEmptyHeapCount)
				mEmptyHeapCount++;
			else
				DestroyHeap(heapIndex);
		}
	}

	template <typename HeapBackend>
	u64 TGpuTlsfAllocator<HeapBackend>::GetCommittedBytes() const
	{
		u64 total = 0;
		for (HeapState* heap : mHeaps)
		{
			if (heap != nullptr)
				total += heap->TotalSize;
		}

		return total;
	}

	template <typename HeapBackend>
	u64 TGpuTlsfAllocator<HeapBackend>::GetUsedBytes() const
	{
		u64 used = 0;
		for (HeapState* heap : mHeaps)
		{
			if (heap != nullptr)
				used += heap->TotalSize - heap->FreeSize;
		}

		return used;
	}

	template <typename HeapBackend>
	u32 TGpuTlsfAllocator<HeapBackend>::GetHeapCount() const
	{
		u32 count = 0;
		for (HeapState* heap : mHeaps)
		{
			if (heap != nullptr)
				count++;
		}

		return count;
	}

	template <typename HeapBackend>
	u32 TGpuTlsfAllocator<HeapBackend>::GetEmptyHeapCount() const
	{
		return mEmptyHeapCount;
	}

	template <typename HeapBackend>
	void TGpuTlsfAllocator<HeapBackend>::SizeToBucket(u64 size, u32& firstLevel, u32& secondLevel)
	{
		if (size <= kSmallBufferSize)
		{
			firstLevel = 0;
			secondLevel = (size > 0) ? (u32)((size - 1) / kSmallBufferGranule) : 0;
			return;
		}

		firstLevel = (u32)Bitwise::MostSignificantBit(size) - kMemoryClassShift;
		const u32 shift = firstLevel + kMemoryClassShift - kSecondLevelIndexBits;
		secondLevel = (u32)((size >> shift) ^ kSecondLevelCount);
	}

	template <typename HeapBackend>
	u32 TGpuTlsfAllocator<HeapBackend>::GetListIndex(u32 firstLevel, u32 secondLevel)
	{
		return firstLevel * kSecondLevelCount + secondLevel;
	}

	template <typename HeapBackend>
	u64 TGpuTlsfAllocator<HeapBackend>::AlignUp(u64 value, u32 alignment)
	{
		const u64 mask = (u64)alignment - 1;
		return (value + mask) & ~mask;
	}

	template <typename HeapBackend>
	u32 TGpuTlsfAllocator<HeapBackend>::AllocateNode()
	{
		if (mNodeFreeHead != kInvalidIndex)
		{
			const u32 index = mNodeFreeHead;
			mNodeFreeHead = mNodes[index].NextFree;
			return index;
		}

		const u32 index = (u32)mNodes.size();
		mNodes.push_back(Node{});
		return index;
	}

	template <typename HeapBackend>
	void TGpuTlsfAllocator<HeapBackend>::ReleaseNode(u32 nodeIndex)
	{
		Node& node = mNodes[nodeIndex];
		node.Flags = NodeFlags{};
		node.NextFree = mNodeFreeHead;
		mNodeFreeHead = nodeIndex;
	}

	template <typename HeapBackend>
	void TGpuTlsfAllocator<HeapBackend>::InsertIntoFreeList(HeapState& heap, u32 nodeIndex)
	{
		Node& node = mNodes[nodeIndex];
		B3D_ASSERT(node.IsFree());
		B3D_ASSERT(!node.IsNullNode());

		u32 firstLevel = 0;
		u32 secondLevel = 0;
		SizeToBucket(node.Size, firstLevel, secondLevel);
		B3D_ASSERT(firstLevel < kFirstLevelClassCount);

		const u32 listIndex = GetListIndex(firstLevel, secondLevel);
		node.PrevFree = kInvalidIndex;
		node.NextFree = heap.FreeListHead[listIndex];
		if (heap.FreeListHead[listIndex] != kInvalidIndex)
			mNodes[heap.FreeListHead[listIndex]].PrevFree = nodeIndex;
		heap.FreeListHead[listIndex] = nodeIndex;

		heap.SecondLevelFreeBitmask[firstLevel] |= (1u << secondLevel);
		heap.FirstLevelFreeBitmask |= (1u << firstLevel);
	}

	template <typename HeapBackend>
	void TGpuTlsfAllocator<HeapBackend>::RemoveFromFreeList(HeapState& heap, u32 nodeIndex)
	{
		Node& node = mNodes[nodeIndex];
		B3D_ASSERT(node.IsFree());
		B3D_ASSERT(!node.IsNullNode());

		u32 firstLevel = 0;
		u32 secondLevel = 0;
		SizeToBucket(node.Size, firstLevel, secondLevel);
		const u32 listIndex = GetListIndex(firstLevel, secondLevel);

		if (node.PrevFree != kInvalidIndex)
			mNodes[node.PrevFree].NextFree = node.NextFree;
		else
			heap.FreeListHead[listIndex] = node.NextFree;

		if (node.NextFree != kInvalidIndex)
			mNodes[node.NextFree].PrevFree = node.PrevFree;

		if (heap.FreeListHead[listIndex] == kInvalidIndex)
		{
			heap.SecondLevelFreeBitmask[firstLevel] &= ~(1u << secondLevel);

			if (heap.SecondLevelFreeBitmask[firstLevel] == 0)
				heap.FirstLevelFreeBitmask &= ~(1u << firstLevel);
		}
	}

	template <typename HeapBackend>
	u32 TGpuTlsfAllocator<HeapBackend>::FindFreeNode(const HeapState& heap, u64 size, u32 alignment, GpuResourceKind kind, u64& outAlignedOffset) const
	{
		u32 firstLevel = 0;
		u32 secondLevel = 0;
		SizeToBucket(size, firstLevel, secondLevel);

		// 1. Walk the natural bucket. Best-fit candidates live here when one fits.
		if ((heap.SecondLevelFreeBitmask[firstLevel] & (1u << secondLevel)) != 0)
		{
			const u32 listIndex = GetListIndex(firstLevel, secondLevel);
			const u32 candidateNodeIndex = WalkBucketForFit(heap, listIndex, size, alignment, kind, outAlignedOffset);
			if (candidateNodeIndex != kInvalidIndex)
				return candidateNodeIndex;
		}

		// 2. Climb buckets via the bitmaps. The first non-empty bucket whose nodes are large enough
		// when alignment slack is applied wins. The above check is not enough due to alignment slack,
		// as it might make the allocation not fit in certain cases.
		u32 nextFirstLevel = firstLevel;
		u32 nextSecondLevel = secondLevel + 1u;
		if (nextSecondLevel >= kSecondLevelCount)
		{
			nextFirstLevel++;
			nextSecondLevel = 0;
		}

		if (nextFirstLevel < kFirstLevelClassCount)
		{
			u32 secondLevelBitmask = heap.SecondLevelFreeBitmask[nextFirstLevel] & ~((1u << nextSecondLevel) - 1u);
			while (secondLevelBitmask != 0)
			{
				const u32 chosenSecondLevel = (u32)Bitwise::LeastSignificantBit(secondLevelBitmask);
				const u32 listIndex = GetListIndex(nextFirstLevel, chosenSecondLevel);
				const u32 candidateNodeIndex = WalkBucketForFit(heap, listIndex, size, alignment, kind, outAlignedOffset);
				if (candidateNodeIndex != kInvalidIndex)
					return candidateNodeIndex;

				secondLevelBitmask &= ~(1u << chosenSecondLevel);
			}
		}

		// 3. Climb beyond the current first-level class via the outer bitmap. `1u << 32` is undefined
		// behavior, so guard the shift when the boundary is at the cap. Walk every set second-level
		// bit in each chosen first-level class — the same alignment-vs-size argument applies.
		u32 firstLevelBitmask = 0;
		if (nextFirstLevel + 1u < kFirstLevelClassCount)
			firstLevelBitmask = heap.FirstLevelFreeBitmask & ~((1u << (nextFirstLevel + 1u)) - 1u);

		while (firstLevelBitmask != 0)
		{
			const u32 chosenFirstLevel = (u32)Bitwise::LeastSignificantBit(firstLevelBitmask);
			u32 secondLevelBitmask = heap.SecondLevelFreeBitmask[chosenFirstLevel];
			B3D_ASSERT(secondLevelBitmask != 0);
			while (secondLevelBitmask != 0)
			{
				const u32 chosenSecondLevel = (u32)Bitwise::LeastSignificantBit(secondLevelBitmask);
				const u32 listIndex = GetListIndex(chosenFirstLevel, chosenSecondLevel);
				const u32 candidate = WalkBucketForFit(heap, listIndex, size, alignment, kind, outAlignedOffset);
				if (candidate != kInvalidIndex)
					return candidate;

				secondLevelBitmask &= ~(1u << chosenSecondLevel);
			}

			firstLevelBitmask &= ~(1u << chosenFirstLevel);
		}

		// 4. Fall back to the trailing null block. It is excluded from the bitmaps but is always free.
		if (heap.NullNodeIndex != kInvalidIndex)
		{
			const Node& nullBlock = mNodes[heap.NullNodeIndex];
			u64 alignedOffset = AlignUp(nullBlock.Offset, alignment);
			if (heap.Granularity.CheckAndAlignUp(alignedOffset, size, kind, nullBlock.Offset + nullBlock.Size) && alignedOffset + size <= nullBlock.Offset + nullBlock.Size)
			{
				outAlignedOffset = alignedOffset;
				return heap.NullNodeIndex;
			}
		}

		return kInvalidIndex;
	}

	template <typename HeapBackend>
	u32 TGpuTlsfAllocator<HeapBackend>::WalkBucketForFit(const HeapState& heap, u32 listIndex, u64 size, u32 alignment, GpuResourceKind kind, u64& outAlignedOffset) const
	{
		u32 cursor = heap.FreeListHead[listIndex];
		while (cursor != kInvalidIndex)
		{
			const Node& node = mNodes[cursor];
			u64 alignedOffset = AlignUp(node.Offset, alignment);

			// Buffer image granularity: adjust the offset if the start page holds a conflicting allocation. Reject the
			// candidate when the inflated range would overrun the block or end-page conflict can't be avoided.
			if (heap.Granularity.CheckAndAlignUp(alignedOffset, size, kind, node.Offset + node.Size) && alignedOffset + size <= node.Offset + node.Size)
			{
				outAlignedOffset = alignedOffset;
				return cursor;
			}

			cursor = node.NextFree;
		}
		return kInvalidIndex;
	}

	template <typename HeapBackend>
	u32 TGpuTlsfAllocator<HeapBackend>::CarveAllocation(HeapState& heap, u32 candidateIndex, u64 alignedOffset, u64 size)
	{
		Node* candidateNode = &mNodes[candidateIndex];
		B3D_ASSERT(candidateNode->IsFree());
		B3D_ASSERT(alignedOffset >= candidateNode->Offset);
		B3D_ASSERT(alignedOffset + size <= candidateNode->Offset + candidateNode->Size);

		const bool wasNullNode = candidateNode->IsNullNode();
		const u64 leadingPadding = alignedOffset - candidateNode->Offset;

		if (!wasNullNode)
			RemoveFromFreeList(heap, candidateIndex);

		// Leading padding split. If the previous physical neighbor is free, fold the padding into it. Otherwise carve a fresh free node for it.
		if (leadingPadding > 0)
		{
			const u32 prevPhysicalIndex = candidateNode->PrevPhysical;
			if (prevPhysicalIndex != kInvalidIndex && mNodes[prevPhysicalIndex].IsFree() && !mNodes[prevPhysicalIndex].IsNullNode())
			{
				RemoveFromFreeList(heap, prevPhysicalIndex);
				mNodes[prevPhysicalIndex].Size += leadingPadding;
				InsertIntoFreeList(heap, prevPhysicalIndex);
			}
			else
			{
				const u32 leadingPaddingNodeIndex = AllocateNode();
				candidateNode = &mNodes[candidateIndex]; // AllocateNode may have invalidated references.

				Node& leadingPaddingNode = mNodes[leadingPaddingNodeIndex];
				leadingPaddingNode.Offset = candidateNode->Offset;
				leadingPaddingNode.Size = leadingPadding;
				leadingPaddingNode.PrevPhysical = candidateNode->PrevPhysical;
				leadingPaddingNode.NextPhysical = candidateIndex;
				leadingPaddingNode.PrevFree = kInvalidIndex;
				leadingPaddingNode.NextFree = kInvalidIndex;
				leadingPaddingNode.Flags = NodeFlag::Free;

				if (candidateNode->PrevPhysical != kInvalidIndex)
					mNodes[candidateNode->PrevPhysical].NextPhysical = leadingPaddingNodeIndex;
				else
					heap.PhysicalListHead = leadingPaddingNodeIndex;

				candidateNode->PrevPhysical = leadingPaddingNodeIndex;

				InsertIntoFreeList(heap, leadingPaddingNodeIndex);
			}

			candidateNode->Offset = alignedOffset;
			candidateNode->Size -= leadingPadding;
		}

		// Trailing-remainder split. If the candidate is the null block, the remainder *becomes* the new
		// null block — we allocate a separate node for the carved-out front portion instead, so the heap
		// always retains a trailing null block.
		const u64 remainder = candidateNode->Size - size;

		u32 allocatedIndex;
		if (wasNullNode)
		{
			// Carve a new allocated node before the null block; shrink the null block to cover the rest.
			allocatedIndex = AllocateNode();
			candidateNode = &mNodes[candidateIndex]; // AllocateNode may have invalidated references.

			Node& allocatedNode = mNodes[allocatedIndex];
			allocatedNode.Offset = candidateNode->Offset;
			allocatedNode.Size = size;
			allocatedNode.PrevPhysical = candidateNode->PrevPhysical;
			allocatedNode.NextPhysical = candidateIndex;
			allocatedNode.PrevFree = kInvalidIndex;
			allocatedNode.NextFree = kInvalidIndex;
			allocatedNode.Flags = NodeFlags{}; // Not free, not null block.

			if (candidateNode->PrevPhysical != kInvalidIndex)
				mNodes[candidateNode->PrevPhysical].NextPhysical = allocatedIndex;
			else
				heap.PhysicalListHead = allocatedIndex;

			candidateNode->PrevPhysical = allocatedIndex;

			candidateNode->Offset += size;
			candidateNode->Size -= size;
		}
		else
		{
			// Trailing remainder either splits off as a free node, or absorbs into the allocation if
			// it would be smaller than MinAllocationSize.
			if (remainder >= mConfig.MinAllocationSize)
			{
				const u32 trailingIndex = AllocateNode();
				candidateNode = &mNodes[candidateIndex]; // AllocateNode may have invalidated references.

				Node& trailingNode = mNodes[trailingIndex];
				trailingNode.Offset = candidateNode->Offset + size;
				trailingNode.Size = remainder;
				trailingNode.PrevPhysical = candidateIndex;
				trailingNode.NextPhysical = candidateNode->NextPhysical;
				trailingNode.PrevFree = kInvalidIndex;
				trailingNode.NextFree = kInvalidIndex;
				trailingNode.Flags = NodeFlag::Free;

				if (candidateNode->NextPhysical != kInvalidIndex)
					mNodes[candidateNode->NextPhysical].PrevPhysical = trailingIndex;

				candidateNode->NextPhysical = trailingIndex;
				candidateNode->Size = size;

				InsertIntoFreeList(heap, trailingIndex);
			}

			candidateNode->Flags = NodeFlags{}; // Not free, not null block.
			allocatedIndex = candidateIndex;
		}

		return allocatedIndex;
	}

	template <typename HeapBackend>
	bool TGpuTlsfAllocator<HeapBackend>::TryAllocateInHeap(HeapState& heap, u64 size, u32 alignment, GpuResourceKind kind, u32& outNodeIndex)
	{
		// Cheap fast-fail: a heap whose total free size is less than the bare request can never fit.
		// Don't include alignment slack here — the natural-bucket walk in FindFreeNode rejects misaligned
		// candidates, and we don't want to skip a heap that has the bytes but might need alignment slack.
		if (heap.FreeSize < size)
			return false;

		u64 alignedOffset = 0;
		const u32 candidateNodeIndex = FindFreeNode(heap, size, alignment, kind, alignedOffset);
		if (candidateNodeIndex == kInvalidIndex)
			return false;

		const bool heapWasEmpty = (heap.LiveAllocCount == 0);
		const u32 allocatedNodeIndex = CarveAllocation(heap, candidateNodeIndex, alignedOffset, size);
		Node& allocated = mNodes[allocatedNodeIndex];
		if (kind == GpuResourceKind::NonLinear)
			allocated.Flags |= NodeFlag::NonLinear;

		heap.Granularity.MarkPages(allocated.Offset, allocated.Size, kind);

		heap.FreeSize -= allocated.Size;
		heap.LiveAllocCount++;

		if (heapWasEmpty && mEmptyHeapCount > 0)
			mEmptyHeapCount--;

		outNodeIndex = allocatedNodeIndex;
		return true;
	}

	template <typename HeapBackend>
	bool TGpuTlsfAllocator<HeapBackend>::TryAllocateInHeap(HeapState& heap, u32 heapIndex, u64 size, u32 alignment, GpuResourceKind kind, IGpuResource* owner, Location& out)
	{
		u32 allocatedNodeIndex = kInvalidIndex;
		if (!TryAllocateInHeap(heap, size, alignment, kind, allocatedNodeIndex))
			return false;

		// Capture the owner pointer onto the node so defragmentation can later look up the resource
		// without consulting the consumer's location. Untracked allocations (owner == nullptr) remain
		// ineligible for defrag.
		Node& allocatedNode = mNodes[allocatedNodeIndex];
		allocatedNode.Owner = owner;

		out.Heap = heap.Heap;
		out.Offset = allocatedNode.Offset;
		out.Size = allocatedNode.Size;
		out.Allocator = this;
		out.AllocatorData0 = heapIndex;
		out.AllocatorData1 = allocatedNodeIndex;

		return true;
	}

	template <typename HeapBackend>
	u32 TGpuTlsfAllocator<HeapBackend>::CreateNewHeap(u64 sizeInBytes)
	{
		const HeapHandle handle = Base::mBackend->CreateHeap(sizeInBytes, mConfig.HeapCreateInfo);

		HeapState* heapState = B3DNew<HeapState>();
		heapState->Heap = handle;
		heapState->TotalSize = sizeInBytes;
		heapState->FreeSize = sizeInBytes;
		heapState->Granularity.Initialize(sizeInBytes, mConfig.BufferImageGranularity, mConfig.GranularityDisableThreshold);

		const u32 nullNodeIndex = AllocateNode();
		Node& nullNode = mNodes[nullNodeIndex];
		nullNode.Offset = 0;
		nullNode.Size = sizeInBytes;
		nullNode.PrevPhysical = kInvalidIndex;
		nullNode.NextPhysical = kInvalidIndex;
		nullNode.PrevFree = kInvalidIndex;
		nullNode.NextFree = kInvalidIndex;
		nullNode.Flags = NodeFlags(NodeFlag::Free) | NodeFlag::NullNode;

		heapState->PhysicalListHead = nullNodeIndex;
		heapState->NullNodeIndex = nullNodeIndex;

		// Empty heap counts as a "spare" against the warm-spare budget the moment it's created — it
		// already has zero live allocations.
		mEmptyHeapCount++;
		mNextHeapSize = std::min(mNextHeapSize * mConfig.GrowthFactor, mConfig.MaxHeapSize);

		// Reuse a vacated slot if one exists; otherwise grow the vector.
		for (u32 heapIndex = 0; heapIndex < (u32)mHeaps.size(); heapIndex++)
		{
			if (mHeaps[heapIndex] == nullptr)
			{
				mHeaps[heapIndex] = heapState;
				return heapIndex;
			}
		}

		const u32 newIndex = (u32)mHeaps.size();
		mHeaps.push_back(heapState);
		return newIndex;
	}

	template <typename HeapBackend>
	void TGpuTlsfAllocator<HeapBackend>::DestroyHeap(u32 heapIndex)
	{
		HeapState* heap = mHeaps[heapIndex];
		B3D_ASSERT(heap != nullptr);
		B3D_ASSERT(heap->LiveAllocCount == 0);

		ReleaseNode(heap->NullNodeIndex);
		Base::mBackend->DestroyHeap(heap->Heap);
		B3DDelete(heap);
		mHeaps[heapIndex] = nullptr;
	}

	template <typename HeapBackend>
	bool TGpuTlsfAllocator<HeapBackend>::TryAllocateInHeapsAtMost(u64 size, u32 alignment, GpuResourceKind kind, u32 maxHeapIndexInclusive, DefragDestinationSlot& out)
	{
		const u64 requestedSize = std::max(size, mConfig.MinAllocationSize);

		// Walk heaps oldest-first (matches TryAllocateImpl's order) but bounded — defragmentation
		// must never grow committed memory.
		const u32 maxIndex = std::min(maxHeapIndexInclusive, (u32)mHeaps.size() - 1);
		for (u32 heapIndex = 0; heapIndex <= maxIndex; heapIndex++)
		{
			HeapState* heap = mHeaps[heapIndex];
			if (heap == nullptr)
				continue;

			u32 allocatedNodeIndex = kInvalidIndex;
			if (!TryAllocateInHeap(*heap, requestedSize, alignment, kind, allocatedNodeIndex))
				continue;

			// Owner is set by the caller (TryMoveAllocation) after this returns.

			out.HeapIndex = heapIndex;
			out.NodeIndex = allocatedNodeIndex;
			out.Offset = mNodes[allocatedNodeIndex].Offset;
			return true;
		}

		return false;
	}

	template <typename HeapBackend>
	bool TGpuTlsfAllocator<HeapBackend>::TryMoveAllocation(u32 sourceNodeIndex, u32 sourceHeapIndex, render::GpuCommandBuffer& commandBuffer, u64 submissionIndex, u32& outDestinationNodeIndex)
	{
		// Snapshot source state before TryAllocateInHeapsAtMost — CarveAllocation may push_back
		// onto mNodes which would invalidate any held Node& references.
		const u64 sourceOffset = mNodes[sourceNodeIndex].Offset;
		const u64 sourceSize = mNodes[sourceNodeIndex].Size;
		IGpuResource* owner = mNodes[sourceNodeIndex].Owner;
		const GpuResourceKind sourceKind = mNodes[sourceNodeIndex].Flags.IsSet(NodeFlag::NonLinear)
			? GpuResourceKind::NonLinear : GpuResourceKind::Linear;

		// 1. Reserve a destination slot in the same heap (within-heap compaction) or any lower-index
		//    heap (multi-heap drain). TryAllocateInHeapsAtMost never grows a fresh heap.
		DefragDestinationSlot destination;
		if (!TryAllocateInHeapsAtMost(sourceSize, /*alignment=*/1u, sourceKind, sourceHeapIndex, destination))
			return false;

		// 2. Reject within-heap destinations that don't compact (offset >= source's). The TLSF
		//    best-fit picks the smallest fitting free block irrespective of offset, so the
		//    destination might land at a higher offset than the source — that would be anti-defrag.
		//    Cross-heap moves (lower heap index) need no such constraint; the goal there is to
		//    drain the source heap.
		if (destination.HeapIndex == sourceHeapIndex && destination.Offset >= sourceOffset)
		{
			FreeImmediateImpl(destination.HeapIndex, destination.NodeIndex);
			return false;
		}

		// 3. Stamp the destination with the owner pointer so the slot is tracked from the moment
		//    the consumer's recreate-and-record path observes it, and so future Defrag() calls
		//    treat it as an eligible candidate.
		mNodes[destination.NodeIndex].Owner = owner;

		// 4. Build the destination Location. The consumer assigns this onto its own location field
		//    instead of patching individual hot fields. Heap handles are read out of mHeaps directly
		//    — slots are stable (DestroyHeap nulls the slot but never repacks indices).
		Location newLocation;
		newLocation.Heap = mHeaps[destination.HeapIndex]->Heap;
		newLocation.Offset = destination.Offset;
		newLocation.Size = sourceSize;
		newLocation.Allocator = this;
		newLocation.AllocatorData0 = destination.HeapIndex;
		newLocation.AllocatorData1 = destination.NodeIndex;

		// 5. Notify the owner: it records the GPU copy from its still-intact source location to
		//    the destination via the command buffer, queues the old backend object for
		//    deferred-destroy on submissionIndex, replaces its location with newLocation, and
		//    recreates the placed backend object at the new memory range. The owner downcasts the
		//    GpuResourceLocation reference back to TGpuResourceLocation<HeapBackend>.
		owner->OnAllocationMoved(submissionIndex, commandBuffer, newLocation);

		// 6. Retire the source slot against submissionIndex; the deferred-free queue releases it
		//    once that submission completes. The retire path keys on Allocator-private slot
		//    fields, so synthesise a Location snapshot identifying the source slot. Clear the
		//    source node's Owner now so any defrag pass issued before the deferred-free drains
		//    observes the slot as untracked rather than as a phantom candidate.
		Location sourceSnapshot;
		sourceSnapshot.Allocator = this;
		sourceSnapshot.AllocatorData0 = sourceHeapIndex;
		sourceSnapshot.AllocatorData1 = sourceNodeIndex;
		Base::RetireAllocation(sourceSnapshot, submissionIndex);
		mNodes[sourceNodeIndex].Owner = nullptr;

		outDestinationNodeIndex = destination.NodeIndex;
		return true;
	}

	template <typename HeapBackend>
	typename TGpuTlsfAllocator<HeapBackend>::DefragmentationStats
	TGpuTlsfAllocator<HeapBackend>::Defrag(render::GpuCommandBuffer& commandBuffer, const DefragmentationInfo& info)
	{
		DefragmentationStats stats{};

		// Tracks destination nodes stamped with NodeFlag::DefragDestination so we can clear the
		// flag at end of pass. Bounded by stats.MovesCompleted ≤ info.MaxAllocationsPerCall.
		Vector<u32> destinationNodes;
		if (info.MaxAllocationsPerCall != 0)
			destinationNodes.reserve(info.MaxAllocationsPerCall);

		// All moves recorded inside this Defrag() call ride the next-not-yet-issued submission
		// index. The caller is contractually required to submit @p commandBuffer next, with no
		// intervening submissions, so the deferred-destroy of source slots and old backend handles
		// drains in lockstep with the GPU completing the move.
		const u64 submissionIndex = Base::mSubmissionTracker->GetLatestSubmissionIndex() + 1;

		// Walk heaps high-index → low-index — newer (typically sparser) heaps drain first.
		// Destinations land in the same heap (within-heap compaction) or any lower-index heap
		// (multi-heap drain); both placements rely on NodeFlag::DefragDestination to keep the
		// destination invisible to subsequent iteration in the same pass.
		for (i32 outerIndex = (i32)mHeaps.size() - 1; outerIndex >= 0; outerIndex--)
		{
			const u32 heapIndex = (u32)outerIndex;
			if (mHeaps[heapIndex] == nullptr)
				continue;

			HeapState& heap = *mHeaps[heapIndex];
			if (heap.NullNodeIndex == kInvalidIndex)
				continue;

			// Walk the physical chain backwards (highest offset → lowest) starting just before
			// the trailing null block. Compaction is more productive draining high-offset
			// allocations into freshly-vacated low-offset slots.
			u32 nodeIndex = mNodes[heap.NullNodeIndex].PrevPhysical;
			while (nodeIndex != kInvalidIndex)
			{
				// Capture the chain link before any state change — TryMoveAllocation may modify
				// the source node's NextPhysical/PrevPhysical pointers via CarveAllocation when
				// the destination lands in the same heap.
				const u32 prevIndex = mNodes[nodeIndex].PrevPhysical;

				if (mNodes[nodeIndex].IsFree() ||
					mNodes[nodeIndex].IsNullNode() ||
					mNodes[nodeIndex].IsDefragDestination())
				{
					nodeIndex = prevIndex;
					continue;
				}

				// Untracked slots (Owner left null at TryAllocate time) cannot be relocated — there
				// is no consumer to invoke OnAllocationMoved on. Any other tracked allocation is a
				// candidate, regardless of in-flight / bound state: correctness follows from submission
				// ordering, not from filtering. The caller's contract requires submitting @p commandBuffer
				// next with no intervening submissions, which guarantees pre-recorded CBs that reference
				// the OLD backend object are submitted at index < submissionIndex (and run before the
				// move on the GPU queue), and any newly recorded CBs reference the NEW location post-patch.
				IGpuResource* owner = mNodes[nodeIndex].Owner;
				if (owner == nullptr)
				{
					nodeIndex = prevIndex;
					continue;
				}

				const u64 sourceSize = mNodes[nodeIndex].Size;

				if (info.MaxBytesPerCall != 0 && stats.BytesMoved + sourceSize > info.MaxBytesPerCall)
				{
					stats.BudgetExhausted = true;
					break;
				}
				if (info.MaxAllocationsPerCall != 0 && stats.MovesAttempted >= info.MaxAllocationsPerCall)
				{
					stats.BudgetExhausted = true;
					break;
				}

				stats.MovesAttempted++;
				u32 destinationNodeIndex = kInvalidIndex;
				if (TryMoveAllocation(nodeIndex, heapIndex, commandBuffer, submissionIndex, destinationNodeIndex))
				{
					stats.MovesCompleted++;
					stats.BytesMoved += sourceSize;
					mNodes[destinationNodeIndex].Flags |= NodeFlag::DefragDestination;
					destinationNodes.push_back(destinationNodeIndex);
				}

				nodeIndex = prevIndex;
			}

			if (stats.BudgetExhausted)
				break;
		}

		// Clear destination markers — destinations are now ordinary live allocations and become
		// valid candidates for future Defrag() calls. The marker is in effect only inside this
		// single Defrag() invocation.
		for (u32 dstIndex : destinationNodes)
			mNodes[dstIndex].Flags.Unset(NodeFlag::DefragDestination);

		return stats;
	}

	inline void TlsfGranularityTracker::Initialize(u64 heapSize, u64 granularity, u64 disableThreshold)
	{
		// Idempotent — covers the "Init twice" sanity case.
		Destroy();

		if (granularity <= 1 || granularity <= disableThreshold)
			return;

		B3D_ASSERT(Bitwise::IsPow2(granularity));
		mGranularity = granularity;
		mPageShift = (u32)Bitwise::MostSignificantBit(granularity);
		mPageCount = (u32)((heapSize + granularity - 1) >> mPageShift);
		mPages = (Page*)B3DAllocate(mPageCount * sizeof(Page));
		for (u32 pageIndex = 0; pageIndex < mPageCount; pageIndex++)
		{
			mPages[pageIndex].Category = PageCategory::Free;
			mPages[pageIndex].LiveCount = 0;
		}
	}

	inline void TlsfGranularityTracker::Destroy()
	{
		if (mPages != nullptr)
			B3DFree(mPages);

		mPages = nullptr;
		mPageCount = 0;
		mGranularity = 1;
		mPageShift = 0;
	}

	inline void TlsfGranularityTracker::MarkPages(u64 offset, u64 size, GpuResourceKind kind)
	{
		if (mPages == nullptr)
			return;

		const u32 startPage = (u32)(offset >> mPageShift);
		const u32 endPage = (u32)((offset + size - 1) >> mPageShift);
		const PageCategory category = (PageCategory)kind;

		if (mPages[startPage].LiveCount == 0 || mPages[startPage].Category == PageCategory::Free)
			mPages[startPage].Category = category;

		mPages[startPage].LiveCount++;

		if (endPage != startPage)
		{
			if (mPages[endPage].LiveCount == 0 || mPages[endPage].Category == PageCategory::Free)
				mPages[endPage].Category = category;

			mPages[endPage].LiveCount++;
		}
	}

	inline void TlsfGranularityTracker::UnmarkPages(u64 offset, u64 size)
	{
		if (mPages == nullptr)
			return;

		const u32 startPage = (u32)(offset >> mPageShift);
		const u32 endPage = (u32)((offset + size - 1) >> mPageShift);

		B3D_ASSERT(mPages[startPage].LiveCount > 0);
		if (--mPages[startPage].LiveCount == 0)
			mPages[startPage].Category = PageCategory::Free;

		if (endPage != startPage)
		{
			B3D_ASSERT(mPages[endPage].LiveCount > 0);
			if (--mPages[endPage].LiveCount == 0)
				mPages[endPage].Category = PageCategory::Free;
		}
	}

	inline bool TlsfGranularityTracker::CheckAndAlignUp(u64& inOutOffset, u64 size, GpuResourceKind kind, u64 blockEnd) const
	{
		if (mPages == nullptr)
			return true;

		const PageCategory category = (PageCategory)kind;
		u32 startPage = (u32)(inOutOffset >> mPageShift);
		if (mPages[startPage].LiveCount > 0 && IsConflict(mPages[startPage].Category, category))
		{
			inOutOffset = (inOutOffset + mGranularity - 1) & ~(mGranularity - 1);
			if (inOutOffset + size > blockEnd)
				return false;

			startPage++;
		}

		const u32 endPage = (u32)((inOutOffset + size - 1) >> mPageShift);
		if (endPage != startPage && mPages[endPage].LiveCount > 0 && IsConflict(mPages[endPage].Category, category))
			return false;

		return true;
	}

#if B3D_DEBUG
	inline void TlsfGranularityTracker::AssertEmpty() const
	{
		if (mPages == nullptr)
			return;

		for (u32 pageIndex = 0; pageIndex < mPageCount; pageIndex++)
			B3D_ASSERT(mPages[pageIndex].LiveCount == 0);
	}
#endif

	/** @} */
} // namespace b3d
