//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DPrerequisites.h"
#include "GpuBackend/Allocators/B3DGpuAllocator.h"
#include "GpuBackend/B3DGpuTimelineFence.h"
#include "Utility/B3DBitwise.h"

namespace b3d
{
	/** @addtogroup GpuBackend
	 *  @{
	 */

	namespace tlsf_detail
	{
		constexpr u32 kInvalidIndex			= ~0u;

		/** Sizes <= this are bucketed entirely within first-level class 0. */
		constexpr u64 kSmallBufferSize		= 256;

		/** Granule width for second-level buckets inside first-level class 0. */
		constexpr u32 kSmallBufferGranule	= 8;

		/** Number of low bits removed from MSB(size) when computing the first-level class for sizes > kSmallBufferSize. */
		constexpr u32 kMemoryClassShift		= 7;

		/** log2 of the second-level bucket count per first-level class. */
		constexpr u32 kSecondLevelIndexBits	= 5;

		/** Second-level buckets per first-level class. */
		constexpr u32 kSecondLevelCount		= 1u << kSecondLevelIndexBits;

		/** First-level class count. Capped at u32 bitmap width — covers heaps up to 2^(kFirstLevelClassCount + kMemoryClassShift) bytes. */
		constexpr u32 kFirstLevelClassCount	= 32;

		/** Total free-list bucket count per heap. */
		constexpr u32 kFreeListCount		= kFirstLevelClassCount * kSecondLevelCount;

		// FirstLevelFreeBitmask is a u32; if the first-level class count grows past 32 the bitmap type must widen.
		static_assert(kFirstLevelClassCount <= 32, "FirstLevelFreeBitmask is u32; widen the bitmap if more first-level classes are required");
		// Likewise for SecondLevelFreeBitmask[firstLevel].
		static_assert(kSecondLevelCount <= 32, "SecondLevelFreeBitmask entries are u32; widen the type if more second-level buckets are required");

		/** Bit set in TlsfBlockNode::Flags when the node is on a free list (or is the trailing null block). */
		constexpr u32 kBlockFlagFree		= 1u << 0;

		/** Bit set in TlsfBlockNode::Flags when the node is the trailing null block of its heap. */
		constexpr u32 kBlockFlagNullBlock	= 1u << 1;

		/** Bit set in TlsfBlockNode::Flags when a live allocation is non-linear (optimally-tiled image). */
		constexpr u32 kBlockFlagNonLinear	= 1u << 2;

		/** Sentinel category for an empty granularity page (no live allocations touch it). */
		constexpr u8  kCategoryFree			= 0xFF;

		/**
		 * Maps an allocation size to a (firstLevel, secondLevel) bucket. The first-level class is the MSB-derived 
		 * size order, the second-level class linearly subdivides each first-level range into kSecondLevelCount sub-buckets.
		 *
		 * For sizes in [1, kSmallBufferSize] the first-level class is forced to 0 and the second-level index is
		 * derived from kSmallBufferGranule-byte granules so small allocations stay segregated below the natural
		 * MSB-class boundaries.
		 */
		inline void SizeToBucket(u64 size, u32& firstLevel, u32& secondLevel)
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

		/** Flat free-list index for a (firstLevel, secondLevel) bucket. */
		inline u32 GetListIndex(u32 firstLevel, u32 secondLevel)
		{
			return firstLevel * kSecondLevelCount + secondLevel;
		}

		/**
		 * Pool node describing a contiguous range within one heap. Indexed by u32 so node
		 * identity fits in TGpuResourceLocation::AllocatorData1.
		 */
		struct TlsfBlockNode
		{
			u64 Offset;
			u64 Size;

			// Heap-order doubly-linked list. kInvalidIndex at the heap start / end.
			u32 PrevPhysical;
			u32 NextPhysical;

			// Free-list doubly-linked list when the node is free; unused otherwise.
			u32 PrevFree;
			u32 NextFree;

			u32 Flags;

			bool IsFree() const { return (Flags & kBlockFlagFree) != 0; }
			bool IsNullNode() const { return (Flags & kBlockFlagNullBlock) != 0; }
		};

		/**
		 * Helper that is used for ensuring that non-linear images are placed at correct alignment (granularity).
		 * Some backends have different alignment requirements if a non-linear image follows or trails 
		 * a linear memory allocation, which this object helps to track.
		 *
		 * One entry per granularity-aligned page. Only the start and end pages of each allocation are ever referenced.
		 *
		 * Default-constructed instances are inert (no allocation). Non-copyable.
		 */
		class GranularityTracker
		{
		public:
			GranularityTracker() = default;
			~GranularityTracker()										{ Destroy(); }

			GranularityTracker(const GranularityTracker&)				= delete;
			GranularityTracker& operator=(const GranularityTracker&)	= delete;

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
			struct Page
			{
				u8	Category;	// GpuResourceKind value, or kCategoryFree when empty.
				u16	LiveCount;	// refcount of allocations touching this page.
			};

			static bool IsConflict(u8 a, u8 b)
			{
				if (a == kCategoryFree || b == kCategoryFree)
					return false;

				return a != b;
			}

			Page*	mPages			= nullptr;
			u32		mPageCount		= 0;
			u64		mGranularity	= 1;
			u32		mPageShift		= 0;
		};

		/** Per-heap TLSF metadata. */
		template <typename HeapBackend>
		struct TlsfHeapState
		{
			typename HeapBackend::HeapHandle Heap{};
			u64 TotalSize			= 0;
			u64 FreeSize			= 0;
			u32 LiveAllocCount		= 0;
			u32 PhysicalListHead	= kInvalidIndex;
			u32 NullNodeIndex		= kInvalidIndex;

			u32 FreeListHead[kFreeListCount]; /**< Free-list head per (firstLevel, secondLevel) bucket. Updated alongside the bitmaps. */
			u32 FirstLevelFreeBitmask = 0; /**< Bit set if any entry in SecondLevelFreeBitmask[firstLevel] is non-zero. */
			u32 SecondLevelFreeBitmask[kFirstLevelClassCount]; /**< Bit set when FreeListHead[(firstLevel, secondLevel)] is non-empty. */

			GranularityTracker Granularity; /**< Buffer-image granularity tracker — inert when the allocator is configured with granularity <= 1 or below the threshold. */

			TlsfHeapState()
			{
				for (u32 listIndex = 0; listIndex < kFreeListCount; listIndex++)
					FreeListHead[listIndex] = kInvalidIndex;

				for (u32 firstLevel = 0; firstLevel < kFirstLevelClassCount; firstLevel++)
					SecondLevelFreeBitmask[firstLevel] = 0;
			}
		};

		inline void GranularityTracker::Initialize(u64 heapSize, u64 granularity, u64 disableThreshold)
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
				mPages[pageIndex].Category = kCategoryFree;
				mPages[pageIndex].LiveCount = 0;
			}
		}

		inline void GranularityTracker::Destroy()
		{
			if (mPages != nullptr)
				B3DFree(mPages);

			mPages = nullptr;
			mPageCount = 0;
			mGranularity = 1;
			mPageShift = 0;
		}

		inline void GranularityTracker::MarkPages(u64 offset, u64 size, GpuResourceKind kind)
		{
			if (mPages == nullptr)
				return;

			const u32 startPage = (u32)(offset >> mPageShift);
			const u32 endPage = (u32)((offset + size - 1) >> mPageShift);
			const u8 category = (u8)kind;

			if (mPages[startPage].LiveCount == 0 || mPages[startPage].Category == kCategoryFree)
				mPages[startPage].Category = category;

			mPages[startPage].LiveCount++;

			if (endPage != startPage)
			{
				if (mPages[endPage].LiveCount == 0 || mPages[endPage].Category == kCategoryFree)
					mPages[endPage].Category = category;

				mPages[endPage].LiveCount++;
			}
		}

		inline void GranularityTracker::UnmarkPages(u64 offset, u64 size)
		{
			if (mPages == nullptr)
				return;

			const u32 startPage = (u32)(offset >> mPageShift);
			const u32 endPage = (u32)((offset + size - 1) >> mPageShift);

			B3D_ASSERT(mPages[startPage].LiveCount > 0);
			if (--mPages[startPage].LiveCount == 0)
				mPages[startPage].Category = kCategoryFree;

			if (endPage != startPage)
			{
				B3D_ASSERT(mPages[endPage].LiveCount > 0);
				if (--mPages[endPage].LiveCount == 0)
					mPages[endPage].Category = kCategoryFree;
			}
		}

		inline bool GranularityTracker::CheckAndAlignUp(u64& inOutOffset, u64 size, GpuResourceKind kind, u64 blockEnd) const
		{
			if (mPages == nullptr)
				return true;

			const u8 category = (u8)kind;
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
		inline void GranularityTracker::AssertEmpty() const
		{
			if (mPages == nullptr)
				return;

			for (u32 pageIndex = 0; pageIndex < mPageCount; pageIndex++)
				B3D_ASSERT(mPages[pageIndex].LiveCount == 0);
		}
#endif
	} // namespace tlsf_detail

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
	 * TODO:
	 * **Defragmentation.** Not exposed in this first pass. The data structures (per-heap physical
	 * doubly-linked list, @c Owner / @c OnAllocationMoved hooks in the location) are designed not to
	 * preclude a future @c Defrag pass.
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

			/** Backend create-info forwarded verbatim to  HeapBackend::CreateHeap on each grow. */
			typename HeapBackend::HeapCreateInformation HeapCreateInfo{};
		};

		TGpuTlsfAllocator(HeapBackend* backend, IGpuSubmissionTracker* submissionTracker, const Configuration& configuration)
			: Base(backend, submissionTracker) , mConfig(configuration) , mNextHeapSize(configuration.InitialHeapSize)
		{
			B3D_ASSERT(mConfig.GrowthFactor >= 1);
			B3D_ASSERT(mConfig.InitialHeapSize > 0);
			B3D_ASSERT(mConfig.MaxHeapSize >= mConfig.InitialHeapSize);
			B3D_ASSERT(mConfig.MinAllocationSize > 0);
			B3D_ASSERT(mConfig.BufferImageGranularity == 1 || Bitwise::IsPow2(mConfig.BufferImageGranularity));
			// Guards the bitmap-width constraint — sizes whose MSB exceeds this cap can't be bucketed.
			B3D_ASSERT(mConfig.MaxHeapSize < (1ull << (tlsf_detail::kFirstLevelClassCount + tlsf_detail::kMemoryClassShift)));
		}

		~TGpuTlsfAllocator()
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

		// Non-copyable — node pool and heap state are not safe to duplicate.
		TGpuTlsfAllocator(const TGpuTlsfAllocator&) = delete;
		TGpuTlsfAllocator& operator=(const TGpuTlsfAllocator&) = delete;

		/** @name TGpuAllocator CRTP surface.
		 *  @{
		 */

		bool TryAllocateImpl(u64 size, u32 alignment, GpuResourceKind kind, Location& out)
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

				if (TryAllocateInHeap(*heap, heapIndex, requestedSize, alignment, kind, out))
					return true;
			}

			// All existing heaps full — grow.
			const u64 newHeapSize = std::max(requestedSize, mNextHeapSize);
			const u32 newHeapIndex = CreateNewHeap(newHeapSize);
			if (newHeapIndex == tlsf_detail::kInvalidIndex)
				return false;

			HeapState& fresh = *mHeaps[newHeapIndex];
			const bool ok = TryAllocateInHeap(fresh, newHeapIndex, requestedSize, alignment, kind, out);
			B3D_ASSERT(ok); // A fresh heap big enough for the request must satisfy it.

			return ok;
		}

		void DeallocateImpl(Location& allocation)
		{
			B3D_ASSERT(allocation.Allocator == this);
			B3D_ASSERT(allocation.AllocatorData0 < (u32)mHeaps.size());
			Base::RetireAllocation(allocation);
		}

		void FreeImmediateImpl(u32 heapIndex, u32 nodeIndex)
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

			// Coalesce with the previous physical neighbor when it's free and not the null block.
			u32 mergedNodeIndex = nodeIndex;
			if (node.PrevPhysical != tlsf_detail::kInvalidIndex)
			{
				Node& previousNode = mNodes[node.PrevPhysical];
				if (previousNode.IsFree() && !previousNode.IsNullNode())
				{
					RemoveFromFreeList(*heap, node.PrevPhysical);
					previousNode.Size += node.Size;
					previousNode.NextPhysical = node.NextPhysical;
					if (node.NextPhysical != tlsf_detail::kInvalidIndex)
						mNodes[node.NextPhysical].PrevPhysical = node.PrevPhysical;

					mergedNodeIndex = node.PrevPhysical;
					ReleaseNode(nodeIndex);
				}
			}

			// Coalesce with the next physical neighbor.
			Node& mergedNode = mNodes[mergedNodeIndex];
			if (mergedNode.NextPhysical != tlsf_detail::kInvalidIndex)
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

					if (mergedNode.PrevPhysical != tlsf_detail::kInvalidIndex)
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
					if (nextNode.NextPhysical != tlsf_detail::kInvalidIndex)
						mNodes[nextNode.NextPhysical].PrevPhysical = mergedNodeIndex;

					ReleaseNode(nextNodeIndex);
				}
			}

			// Insert the resulting node into its bucket. The null block does not participate in the free lists.
			Node& finalNode = mNodes[mergedNodeIndex];
			finalNode.Flags |= tlsf_detail::kBlockFlagFree;
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

		/** @} */

		/** @name Diagnostics.
		 *  @{
		 */

		/** Total number of bytes allocated by all underlying heaps. */
		u64 GetCommittedBytes() const
		{
			u64 total = 0;
			for (HeapState* heap : mHeaps)
			{
				if (heap != nullptr)
					total += heap->TotalSize;
			}

			return total;
		}

		/** Total number of bytes currently held by live (non-retired, non-freed) allocations. */
		u64 GetUsedBytes() const
		{
			u64 used = 0;
			{
				for (HeapState* heap : mHeaps)
					if (heap != nullptr)
						used += heap->TotalSize - heap->FreeSize;
			}

			return used;
		}

		/** Number of populated heap slots (vacated slots are not counted). */
		u32 GetHeapCount() const
		{
			u32 count = 0;
			for (HeapState* heap : mHeaps)
			{
				if (heap != nullptr)
					count++;
			}

			return count;
		}

		/** Number of fully-empty heaps currently retained as spares. */
		u32 GetEmptyHeapCount() const
		{
			return mEmptyHeapCount;
		}

		/** @} */

	private:
		using Node			= tlsf_detail::TlsfBlockNode;
		using HeapState		= tlsf_detail::TlsfHeapState<HeapBackend>;

		/** Acquire a free node-pool slot, growing the pool if necessary. */
		u32 AllocateNode()
		{
			if (mNodeFreeHead != tlsf_detail::kInvalidIndex)
			{
				const u32 index = mNodeFreeHead;
				mNodeFreeHead = mNodes[index].NextFree;
				return index;
			}

			const u32 index = (u32)mNodes.size();
			mNodes.push_back(Node{});
			return index;
		}

		/** Return a node-pool slot to the free list. */
		void ReleaseNode(u32 nodeIndex)
		{
			Node& node = mNodes[nodeIndex];
			node.Flags = 0;
			node.NextFree = mNodeFreeHead;
			mNodeFreeHead = nodeIndex;
		}

		/** Insert @p nodeIndex into the appropriate (firstLevel, secondLevel) bucket of @p heap and update bitmaps. */
		void InsertIntoFreeList(HeapState& heap, u32 nodeIndex)
		{
			Node& node = mNodes[nodeIndex];
			B3D_ASSERT(node.IsFree());
			B3D_ASSERT(!node.IsNullNode());

			u32 firstLevel = 0;
			u32 secondLevel = 0;
			tlsf_detail::SizeToBucket(node.Size, firstLevel, secondLevel);
			B3D_ASSERT(firstLevel < tlsf_detail::kFirstLevelClassCount);

			const u32 listIndex = tlsf_detail::GetListIndex(firstLevel, secondLevel);
			node.PrevFree = tlsf_detail::kInvalidIndex;
			node.NextFree = heap.FreeListHead[listIndex];
			if (heap.FreeListHead[listIndex] != tlsf_detail::kInvalidIndex)
				mNodes[heap.FreeListHead[listIndex]].PrevFree = nodeIndex;
			heap.FreeListHead[listIndex] = nodeIndex;

			heap.SecondLevelFreeBitmask[firstLevel] |= (1u << secondLevel);
			heap.FirstLevelFreeBitmask |= (1u << firstLevel);
		}

		/** Splice @p nodeIndex out of its free list and clear bitmap bits if its bucket is now empty. */
		void RemoveFromFreeList(HeapState& heap, u32 nodeIndex)
		{
			Node& node = mNodes[nodeIndex];
			B3D_ASSERT(node.IsFree());
			B3D_ASSERT(!node.IsNullNode());

			u32 firstLevel = 0;
			u32 secondLevel = 0;
			tlsf_detail::SizeToBucket(node.Size, firstLevel, secondLevel);
			const u32 listIndex = tlsf_detail::GetListIndex(firstLevel, secondLevel);

			if (node.PrevFree != tlsf_detail::kInvalidIndex)
				mNodes[node.PrevFree].NextFree = node.NextFree;
			else
				heap.FreeListHead[listIndex] = node.NextFree;

			if (node.NextFree != tlsf_detail::kInvalidIndex)
				mNodes[node.NextFree].PrevFree = node.PrevFree;

			if (heap.FreeListHead[listIndex] == tlsf_detail::kInvalidIndex)
			{
				heap.SecondLevelFreeBitmask[firstLevel] &= ~(1u << secondLevel);

				if (heap.SecondLevelFreeBitmask[firstLevel] == 0)
					heap.FirstLevelFreeBitmask &= ~(1u << firstLevel);
			}
		}

		/**
		 * Find a free node in @p heap that can satisfy a (size, alignment, kind) request. Searches the
		 * natural bucket first (best-fit candidates live there) and walks larger buckets via the bitmaps
		 * if needed. The returned @p outAlignedOffset folds in both natural alignment and any buffer image granularity
		 * inflation, so the carver doesn't have to recompute either. Returns kInvalidIndex on miss.
		 */
		u32 FindFreeNode(const HeapState& heap, u64 size, u32 alignment, GpuResourceKind kind, u64& outAlignedOffset) const
		{
			u32 firstLevel = 0;
			u32 secondLevel = 0;
			tlsf_detail::SizeToBucket(size, firstLevel, secondLevel);

			// 1. Walk the natural bucket. Best-fit candidates live here when one fits.
			if ((heap.SecondLevelFreeBitmask[firstLevel] & (1u << secondLevel)) != 0)
			{
				const u32 listIndex = tlsf_detail::GetListIndex(firstLevel, secondLevel);
				const u32 candidateNodeIndex = WalkBucketForFit(heap, listIndex, size, alignment, kind, outAlignedOffset);
				if (candidateNodeIndex != tlsf_detail::kInvalidIndex)
					return candidateNodeIndex;
			}

			// 2. Climb buckets via the bitmaps. The first non-empty bucket whose nodes are large enough
			// when alignment slack is applied wins. The above check is not enough due to alignment slack,
			// as it might make the allocation not fit in certain cases.
			u32 nextFirstLevel = firstLevel;
			u32 nextSecondLevel = secondLevel + 1u;
			if (nextSecondLevel >= tlsf_detail::kSecondLevelCount)
			{
				nextFirstLevel++;
				nextSecondLevel = 0;
			}

			if (nextFirstLevel < tlsf_detail::kFirstLevelClassCount)
			{
				u32 secondLevelBitmask = heap.SecondLevelFreeBitmask[nextFirstLevel] & ~((1u << nextSecondLevel) - 1u);
				while (secondLevelBitmask != 0)
				{
					const u32 chosenSecondLevel = (u32)Bitwise::LeastSignificantBit(secondLevelBitmask);
					const u32 listIndex = tlsf_detail::GetListIndex(nextFirstLevel, chosenSecondLevel);
					const u32 candidateNodeIndex = WalkBucketForFit(heap, listIndex, size, alignment, kind, outAlignedOffset);
					if (candidateNodeIndex != tlsf_detail::kInvalidIndex)
						return candidateNodeIndex;

					secondLevelBitmask &= ~(1u << chosenSecondLevel);
				}
			}

			// 3. Climb beyond the current first-level class via the outer bitmap. `1u << 32` is undefined
			// behavior, so guard the shift when the boundary is at the cap. Walk every set second-level
			// bit in each chosen first-level class — the same alignment-vs-size argument applies.
			u32 firstLevelBitmask = 0;
			if (nextFirstLevel + 1u < tlsf_detail::kFirstLevelClassCount)
				firstLevelBitmask = heap.FirstLevelFreeBitmask & ~((1u << (nextFirstLevel + 1u)) - 1u);

			while (firstLevelBitmask != 0)
			{
				const u32 chosenFirstLevel = (u32)Bitwise::LeastSignificantBit(firstLevelBitmask);
				u32 secondLevelBitmask = heap.SecondLevelFreeBitmask[chosenFirstLevel];
				B3D_ASSERT(secondLevelBitmask != 0);
				while (secondLevelBitmask != 0)
				{
					const u32 chosenSecondLevel = (u32)Bitwise::LeastSignificantBit(secondLevelBitmask);
					const u32 listIndex = tlsf_detail::GetListIndex(chosenFirstLevel, chosenSecondLevel);
					const u32 candidate = WalkBucketForFit(heap, listIndex, size, alignment, kind, outAlignedOffset);
					if (candidate != tlsf_detail::kInvalidIndex)
						return candidate;

					secondLevelBitmask &= ~(1u << chosenSecondLevel);
				}

				firstLevelBitmask &= ~(1u << chosenFirstLevel);
			}

			// 4. Fall back to the trailing null block. It is excluded from the bitmaps but is always free.
			if (heap.NullNodeIndex != tlsf_detail::kInvalidIndex)
			{
				const Node& nullBlock = mNodes[heap.NullNodeIndex];
				u64 alignedOffset = AlignUp(nullBlock.Offset, alignment);
				if (heap.Granularity.CheckAndAlignUp(alignedOffset, size, kind, nullBlock.Offset + nullBlock.Size) && alignedOffset + size <= nullBlock.Offset + nullBlock.Size)
				{
					outAlignedOffset = alignedOffset;
					return heap.NullNodeIndex;
				}
			}

			return tlsf_detail::kInvalidIndex;
		}

		/**
		 * Walk a bucket's free list and return the first node large enough to satisfy (size, alignment, kind).
		 * The returned @p outAlignedOffset contains any buffer image granularity past natural alignment.
		 */
		u32 WalkBucketForFit(const HeapState& heap, u32 listIndex, u64 size, u32 alignment, GpuResourceKind kind, u64& outAlignedOffset) const
		{
			u32 cursor = heap.FreeListHead[listIndex];
			while (cursor != tlsf_detail::kInvalidIndex)
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
			return tlsf_detail::kInvalidIndex;
		}

		/**
		 * Carve a @p size byte allocation starting at @p alignedOffset out of the candidate node, splitting
		 * any leading padding and trailing remainder into separate free nodes. Returns the node-index of the allocated block.
		 */
		u32 CarveAllocation(HeapState& heap, u32 candidateIndex, u64 alignedOffset, u64 size)
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
				if (prevPhysicalIndex != tlsf_detail::kInvalidIndex && mNodes[prevPhysicalIndex].IsFree() && !mNodes[prevPhysicalIndex].IsNullNode())
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
					leadingPaddingNode.PrevFree = tlsf_detail::kInvalidIndex;
					leadingPaddingNode.NextFree = tlsf_detail::kInvalidIndex;
					leadingPaddingNode.Flags = tlsf_detail::kBlockFlagFree;

					if (candidateNode->PrevPhysical != tlsf_detail::kInvalidIndex)
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
				allocatedNode.PrevFree = tlsf_detail::kInvalidIndex;
				allocatedNode.NextFree = tlsf_detail::kInvalidIndex;
				allocatedNode.Flags = 0; // Not free, not null block.

				if (candidateNode->PrevPhysical != tlsf_detail::kInvalidIndex)
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
					trailingNode.PrevFree = tlsf_detail::kInvalidIndex;
					trailingNode.NextFree = tlsf_detail::kInvalidIndex;
					trailingNode.Flags = tlsf_detail::kBlockFlagFree;

					if (candidateNode->NextPhysical != tlsf_detail::kInvalidIndex)
						mNodes[candidateNode->NextPhysical].PrevPhysical = trailingIndex;

					candidateNode->NextPhysical = trailingIndex;
					candidateNode->Size = size;

					InsertIntoFreeList(heap, trailingIndex);
				}

				candidateNode->Flags = 0; // Not free, not null block.
				allocatedIndex = candidateIndex;
			}

			return allocatedIndex;
		}

		/** Search-and-carve combined: returns true on hit and populates @p out, false on miss. */
		bool TryAllocateInHeap(HeapState& heap, u32 heapIndex, u64 size, u32 alignment, GpuResourceKind kind, Location& out)
		{
			// Cheap fast-fail: a heap whose total free size is less than the bare request can never fit.
			// Don't include alignment slack here — the natural-bucket walk in FindFreeNode rejects misaligned
			// candidates, and we don't want to skip a heap that has the bytes but might need alignment slack.
			if (heap.FreeSize < size)
				return false;

			u64 alignedOffset = 0;
			const u32 candidateNodeIndex = FindFreeNode(heap, size, alignment, kind, alignedOffset);
			if (candidateNodeIndex == tlsf_detail::kInvalidIndex)
				return false;

			const bool heapWasEmpty = (heap.LiveAllocCount == 0);
			const u32 allocatedNodeIndex = CarveAllocation(heap, candidateNodeIndex, alignedOffset, size);
			Node& allocated = mNodes[allocatedNodeIndex];
			if (kind == GpuResourceKind::NonLinear)
				allocated.Flags |= tlsf_detail::kBlockFlagNonLinear;

			heap.Granularity.MarkPages(allocated.Offset, allocated.Size, kind);

			heap.FreeSize -= allocated.Size;
			heap.LiveAllocCount++;

			if (heapWasEmpty && mEmptyHeapCount > 0)
				mEmptyHeapCount--;

			out.Heap = heap.Heap;
			out.Offset = allocated.Offset;
			out.Size = allocated.Size;
			out.Allocator = this;
			out.AllocatorData0 = heapIndex;
			out.AllocatorData1 = allocatedNodeIndex;

			return true;
		}

		/** Create a fresh heap and install it into @c mHeaps, reusing a vacated slot if one is available. */
		u32 CreateNewHeap(u64 sizeInBytes)
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
			nullNode.PrevPhysical = tlsf_detail::kInvalidIndex;
			nullNode.NextPhysical = tlsf_detail::kInvalidIndex;
			nullNode.PrevFree = tlsf_detail::kInvalidIndex;
			nullNode.NextFree = tlsf_detail::kInvalidIndex;
			nullNode.Flags = tlsf_detail::kBlockFlagFree | tlsf_detail::kBlockFlagNullBlock;

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

		/** Destroy heap @p heapIndex and vacate its slot. Caller has verified LiveAllocCount == 0. */
		void DestroyHeap(u32 heapIndex)
		{
			HeapState* heap = mHeaps[heapIndex];
			B3D_ASSERT(heap != nullptr);
			B3D_ASSERT(heap->LiveAllocCount == 0);

			ReleaseNode(heap->NullNodeIndex);
			Base::mBackend->DestroyHeap(heap->Heap);
			B3DDelete(heap);
			mHeaps[heapIndex] = nullptr;
		}

		static u64 AlignUp(u64 value, u32 alignment)
		{
			const u64 mask = (u64)alignment - 1;
			return (value + mask) & ~mask;
		}

		Configuration mConfig;
		Vector<HeapState*>	mHeaps;
		Vector<Node> mNodes;
		u32 mNodeFreeHead = tlsf_detail::kInvalidIndex;
		u32 mEmptyHeapCount = 0;
		u64 mNextHeapSize = 0;
	};

	/** @} */
} // namespace b3d
