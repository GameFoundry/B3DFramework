//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

// Template method definitions for TTlsfAllocator. Not a translation unit of its own — included at the end of
// B3DTlsfAllocator.h.

#include "Allocators/B3DTlsfAllocator.h"

namespace b3d
{
	namespace detail::tlsf
	{
		//----------------------------------------------------------------------------------------
		// GranularityTracker definitions
		//----------------------------------------------------------------------------------------

		inline GranularityTracker::~GranularityTracker()
		{
			Destroy();
		}

		inline bool GranularityTracker::IsConflict(PageCategory a, PageCategory b)
		{
			if (a == PageCategory::Free || b == PageCategory::Free)
				return false;

			return a != b;
		}

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
				mPages[pageIndex].Category = PageCategory::Free;
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

		inline void GranularityTracker::MarkPages(u64 offset, u64 size, TlsfAllocationKind kind)
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

		inline void GranularityTracker::UnmarkPages(u64 offset, u64 size)
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

		inline bool GranularityTracker::CheckAndAlignUp(u64& inOutOffset, u64 size, TlsfAllocationKind kind, u64 blockEnd) const
		{
			if (mPages == nullptr)
				return true;

			if (inOutOffset + size > blockEnd)
				return false;

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
		inline void GranularityTracker::AssertEmpty() const
		{
			if (mPages == nullptr)
				return;

			for (u32 pageIndex = 0; pageIndex < mPageCount; pageIndex++)
				B3D_ASSERT(mPages[pageIndex].LiveCount == 0);
		}
#endif

		//----------------------------------------------------------------------------------------
		// NodePool definitions
		//----------------------------------------------------------------------------------------

		inline u32 NodePool::Allocate()
		{
			if (mFreeHead != Utility::kInvalidIndex)
			{
				const u32 index = mFreeHead;
				mFreeHead = mNodes[index].NextFree;
				return index;
			}

			const u32 index = (u32)mNodes.size();
			mNodes.push_back(Node{});
			return index;
		}

		inline void NodePool::Release(u32 nodeIndex)
		{
			Node& node = mNodes[nodeIndex];
			node.Flags = NodeFlags{};
			node.NextFree = mFreeHead;
			mFreeHead = nodeIndex;
		}

		//----------------------------------------------------------------------------------------
		// Heap definitions
		//----------------------------------------------------------------------------------------

		template <typename HeapHandleType>
		Heap<HeapHandleType>::Heap(HeapHandle handle, u64 size, u64 granularity, u64 granularityDisableThreshold, u64 minAllocationSize)
			: mHandle(handle), mTotalSize(size), mFreeSize(size), mMinAllocationSize(minAllocationSize)
		{
			for (u32 listIndex = 0; listIndex < Utility::kFreeListCount; listIndex++)
				mFreeListHead[listIndex] = Utility::kInvalidIndex;

			for (u32 firstLevel = 0; firstLevel < Utility::kFirstLevelClassCount; firstLevel++)
				mSecondLevelFreeBitmask[firstLevel] = 0;

			mGranularity.Initialize(size, granularity, granularityDisableThreshold);

			// Trailing null block — covers the entire heap initially. Excluded from the free-list bitmaps;
			// FindFreeNode falls through to it after the bitmap walk fails.
			mNullNodeIndex = mPool.Allocate();
			Node& nullNode = mPool[mNullNodeIndex];
			nullNode.Offset = 0;
			nullNode.Size = size;
			nullNode.PrevPhysical = Utility::kInvalidIndex;
			nullNode.NextPhysical = Utility::kInvalidIndex;
			nullNode.PrevFree = Utility::kInvalidIndex;
			nullNode.NextFree = Utility::kInvalidIndex;
			nullNode.Flags = NodeFlags(NodeFlag::Free) | NodeFlag::NullNode;
			nullNode.Owner = nullptr;

			mPhysicalListHead = mNullNodeIndex;
		}

		template <typename HeapHandleType>
		bool Heap<HeapHandleType>::TryAllocate(u64 size, u32 alignment, TlsfAllocationKind kind, u32& outNodeIndex)
		{
			// Cheap fast-fail: a heap whose total free size is less than the bare request can never fit.
			// Don't include alignment slack here — the natural-bucket walk in FindFreeNode rejects misaligned
			// candidates, and we don't want to skip a heap that has the bytes but might need alignment slack.
			if (mFreeSize < size)
				return false;

			u64 alignedOffset = 0;
			const u32 candidateNodeIndex = FindFreeNode(size, alignment, kind, alignedOffset);
			if (candidateNodeIndex == Utility::kInvalidIndex)
				return false;

			const u32 allocatedNodeIndex = CarveAllocation(candidateNodeIndex, alignedOffset, size);
			Node& allocated = mPool[allocatedNodeIndex];
			if (kind == TlsfAllocationKind::NonLinear)
				allocated.Flags |= NodeFlag::NonLinear;

			// Reset the opaque owner tag — recycled pool slots may carry a stale one. Live nodes are
			// untracked until the orchestrator explicitly stamps an owner.
			allocated.Owner = nullptr;

			mGranularity.MarkPages(allocated.Offset, allocated.Size, kind);

			mFreeSize -= allocated.Size;
			mLiveAllocCount++;

			outNodeIndex = allocatedNodeIndex;
			return true;
		}

		template <typename HeapHandleType>
		void Heap<HeapHandleType>::FreeNode(u32 nodeIndex)
		{
			Node& node = mPool[nodeIndex];
			B3D_ASSERT(!node.IsFree());

			mGranularity.UnmarkPages(node.Offset, node.Size);

			mFreeSize += node.Size;
			mLiveAllocCount--;
			node.Owner = nullptr;

			// Coalesce with the previous physical neighbor when it's free and not the null block.
			u32 mergedNodeIndex = nodeIndex;
			if (node.PrevPhysical != Utility::kInvalidIndex)
			{
				Node& previousNode = mPool[node.PrevPhysical];
				if (previousNode.IsFree() && !previousNode.IsNullNode())
				{
					RemoveFromFreeList(node.PrevPhysical);
					previousNode.Size += node.Size;
					previousNode.NextPhysical = node.NextPhysical;
					if (node.NextPhysical != Utility::kInvalidIndex)
						mPool[node.NextPhysical].PrevPhysical = node.PrevPhysical;

					mergedNodeIndex = node.PrevPhysical;
					mPool.Release(nodeIndex);
				}
			}

			// Coalesce with the next physical neighbor.
			Node& mergedNode = mPool[mergedNodeIndex];
			if (mergedNode.NextPhysical != Utility::kInvalidIndex)
			{
				const u32 nextNodeIndex = mergedNode.NextPhysical;
				Node& nextNode = mPool[nextNodeIndex];

				if (nextNode.IsNullNode())
				{
					// Fold our newly-freed range into the trailing null block. The merged node (if it isn't
					// the null block itself) is released back to the pool; the null block keeps its identity.
					nextNode.Offset = mergedNode.Offset;
					nextNode.Size += mergedNode.Size;
					nextNode.PrevPhysical = mergedNode.PrevPhysical;

					if (mergedNode.PrevPhysical != Utility::kInvalidIndex)
						mPool[mergedNode.PrevPhysical].NextPhysical = nextNodeIndex;
					else
						mPhysicalListHead = nextNodeIndex;

					mPool.Release(mergedNodeIndex);
					mergedNodeIndex = nextNodeIndex;
				}
				else if (nextNode.IsFree())
				{
					RemoveFromFreeList(nextNodeIndex);
					mergedNode.Size += nextNode.Size;
					mergedNode.NextPhysical = nextNode.NextPhysical;
					if (nextNode.NextPhysical != Utility::kInvalidIndex)
						mPool[nextNode.NextPhysical].PrevPhysical = mergedNodeIndex;

					mPool.Release(nextNodeIndex);
				}
			}

			// Insert the resulting node into its bucket. The null block does not participate in the free lists.
			Node& finalNode = mPool[mergedNodeIndex];
			finalNode.Flags |= NodeFlag::Free;
			if (!finalNode.IsNullNode())
				InsertIntoFreeList(mergedNodeIndex);

			if (mLiveAllocCount == 0)
			{
				B3D_DEBUG_ONLY(mGranularity.AssertEmpty());
			}
		}

		template <typename HeapHandleType>
		void Heap<HeapHandleType>::InsertIntoFreeList(u32 nodeIndex)
		{
			Node& node = mPool[nodeIndex];
			B3D_ASSERT(node.IsFree());
			B3D_ASSERT(!node.IsNullNode());

			u32 firstLevel = 0;
			u32 secondLevel = 0;
			Utility::SizeToBucket(node.Size, firstLevel, secondLevel);
			B3D_ASSERT(firstLevel < Utility::kFirstLevelClassCount);

			const u32 listIndex = Utility::GetListIndex(firstLevel, secondLevel);
			node.PrevFree = Utility::kInvalidIndex;
			node.NextFree = mFreeListHead[listIndex];
			if (mFreeListHead[listIndex] != Utility::kInvalidIndex)
				mPool[mFreeListHead[listIndex]].PrevFree = nodeIndex;
			mFreeListHead[listIndex] = nodeIndex;

			mSecondLevelFreeBitmask[firstLevel] |= (1u << secondLevel);
			mFirstLevelFreeBitmask |= (1u << firstLevel);
		}

		template <typename HeapHandleType>
		void Heap<HeapHandleType>::RemoveFromFreeList(u32 nodeIndex)
		{
			Node& node = mPool[nodeIndex];
			B3D_ASSERT(node.IsFree());
			B3D_ASSERT(!node.IsNullNode());

			u32 firstLevel = 0;
			u32 secondLevel = 0;
			Utility::SizeToBucket(node.Size, firstLevel, secondLevel);
			const u32 listIndex = Utility::GetListIndex(firstLevel, secondLevel);

			if (node.PrevFree != Utility::kInvalidIndex)
				mPool[node.PrevFree].NextFree = node.NextFree;
			else
				mFreeListHead[listIndex] = node.NextFree;

			if (node.NextFree != Utility::kInvalidIndex)
				mPool[node.NextFree].PrevFree = node.PrevFree;

			if (mFreeListHead[listIndex] == Utility::kInvalidIndex)
			{
				mSecondLevelFreeBitmask[firstLevel] &= ~(1u << secondLevel);

				if (mSecondLevelFreeBitmask[firstLevel] == 0)
					mFirstLevelFreeBitmask &= ~(1u << firstLevel);
			}
		}

		template <typename HeapHandleType>
		u32 Heap<HeapHandleType>::FindFreeNode(u64 size, u32 alignment, TlsfAllocationKind kind, u64& outAlignedOffset) const
		{
			u32 firstLevel = 0;
			u32 secondLevel = 0;
			Utility::SizeToBucket(size, firstLevel, secondLevel);

			// 1. Walk every non-empty bucket at-or-after (firstLevel, secondLevel). The natural bucket
			// (firstLevel, secondLevel) may or may not contain a fitting node depending on alignment +
			// granularity slack; strictly larger buckets would always fit absent that slack but a node
			// can still be rejected by it, so the same per-node check is applied throughout. Shifts use
			// 1ull to dodge UB at the firstLevel == 32 / secondLevel == kSecondLevelCount boundaries.
			// The second-level floor only applies on the natural first-level; higher first-levels walk
			// every set second-level bit (their nodes are strictly larger by construction).
			const u32 startFirstLevel = firstLevel;
			const u32 startSecondLevelFloor = (u32)(~((1ull << secondLevel) - 1ull));
			u32 firstLevelBitmask = mFirstLevelFreeBitmask & (u32)(~((1ull << startFirstLevel) - 1ull));

			while (firstLevelBitmask != 0)
			{
				const u32 chosenFirstLevel = (u32)Bitwise::LeastSignificantBit(firstLevelBitmask);
				const u32 secondLevelMask = (chosenFirstLevel == startFirstLevel) ? startSecondLevelFloor : ~0u;
				u32 secondLevelBitmask = mSecondLevelFreeBitmask[chosenFirstLevel] & secondLevelMask;
				while (secondLevelBitmask != 0)
				{
					const u32 chosenSecondLevel = (u32)Bitwise::LeastSignificantBit(secondLevelBitmask);
					const u32 listIndex = Utility::GetListIndex(chosenFirstLevel, chosenSecondLevel);
					const u32 candidateNodeIndex = WalkBucketForFit(listIndex, size, alignment, kind, outAlignedOffset);
					if (candidateNodeIndex != Utility::kInvalidIndex)
						return candidateNodeIndex;

					secondLevelBitmask &= ~(1u << chosenSecondLevel);
				}

				firstLevelBitmask &= ~(1u << chosenFirstLevel);
			}

			// 2. Fall back to the trailing null block. It is excluded from the bitmaps but is always free.
			if (mNullNodeIndex != Utility::kInvalidIndex)
			{
				const Node& nullBlock = mPool[mNullNodeIndex];
				u64 alignedOffset = Utility::AlignUp(nullBlock.Offset, alignment);
				if (mGranularity.CheckAndAlignUp(alignedOffset, size, kind, nullBlock.Offset + nullBlock.Size) && alignedOffset + size <= nullBlock.Offset + nullBlock.Size)
				{
					outAlignedOffset = alignedOffset;
					return mNullNodeIndex;
				}
			}

			return Utility::kInvalidIndex;
		}

		template <typename HeapHandleType>
		u32 Heap<HeapHandleType>::WalkBucketForFit(u32 listIndex, u64 size, u32 alignment, TlsfAllocationKind kind, u64& outAlignedOffset) const
		{
			u32 cursor = mFreeListHead[listIndex];
			while (cursor != Utility::kInvalidIndex)
			{
				const Node& node = mPool[cursor];
				u64 alignedOffset = Utility::AlignUp(node.Offset, alignment);

				// Buffer image granularity: adjust the offset if the start page holds a conflicting allocation. Reject the
				// candidate when the inflated range would overrun the block or end-page conflict can't be avoided.
				if (mGranularity.CheckAndAlignUp(alignedOffset, size, kind, node.Offset + node.Size) && alignedOffset + size <= node.Offset + node.Size)
				{
					outAlignedOffset = alignedOffset;
					return cursor;
				}

				cursor = node.NextFree;
			}
			return Utility::kInvalidIndex;
		}

		template <typename HeapHandleType>
		u32 Heap<HeapHandleType>::CarveAllocation(u32 candidateIndex, u64 alignedOffset, u64 size)
		{
			Node* candidateNode = &mPool[candidateIndex];
			B3D_ASSERT(candidateNode->IsFree());
			B3D_ASSERT(alignedOffset >= candidateNode->Offset);
			B3D_ASSERT(alignedOffset + size <= candidateNode->Offset + candidateNode->Size);

			const bool wasNullNode = candidateNode->IsNullNode();
			const u64 leadingPadding = alignedOffset - candidateNode->Offset;

			if (!wasNullNode)
				RemoveFromFreeList(candidateIndex);

			// Leading padding split. If the previous physical neighbor is free, fold the padding into it. Otherwise carve a fresh free node for it.
			if (leadingPadding > 0)
			{
				const u32 prevPhysicalIndex = candidateNode->PrevPhysical;
				if (prevPhysicalIndex != Utility::kInvalidIndex && mPool[prevPhysicalIndex].IsFree() && !mPool[prevPhysicalIndex].IsNullNode())
				{
					RemoveFromFreeList(prevPhysicalIndex);
					mPool[prevPhysicalIndex].Size += leadingPadding;
					InsertIntoFreeList(prevPhysicalIndex);
				}
				else
				{
					const u32 leadingPaddingNodeIndex = mPool.Allocate();
					candidateNode = &mPool[candidateIndex]; // Pool.Allocate may have invalidated references.

					Node& leadingPaddingNode = mPool[leadingPaddingNodeIndex];
					leadingPaddingNode.Offset = candidateNode->Offset;
					leadingPaddingNode.Size = leadingPadding;
					leadingPaddingNode.PrevPhysical = candidateNode->PrevPhysical;
					leadingPaddingNode.NextPhysical = candidateIndex;
					leadingPaddingNode.PrevFree = Utility::kInvalidIndex;
					leadingPaddingNode.NextFree = Utility::kInvalidIndex;
					leadingPaddingNode.Flags = NodeFlag::Free;

					if (candidateNode->PrevPhysical != Utility::kInvalidIndex)
						mPool[candidateNode->PrevPhysical].NextPhysical = leadingPaddingNodeIndex;
					else
						mPhysicalListHead = leadingPaddingNodeIndex;

					candidateNode->PrevPhysical = leadingPaddingNodeIndex;

					InsertIntoFreeList(leadingPaddingNodeIndex);
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
				allocatedIndex = mPool.Allocate();
				candidateNode = &mPool[candidateIndex]; // Pool.Allocate may have invalidated references.

				Node& allocatedNode = mPool[allocatedIndex];
				allocatedNode.Offset = candidateNode->Offset;
				allocatedNode.Size = size;
				allocatedNode.PrevPhysical = candidateNode->PrevPhysical;
				allocatedNode.NextPhysical = candidateIndex;
				allocatedNode.PrevFree = Utility::kInvalidIndex;
				allocatedNode.NextFree = Utility::kInvalidIndex;
				allocatedNode.Flags = NodeFlags{}; // Not free, not null block.

				if (candidateNode->PrevPhysical != Utility::kInvalidIndex)
					mPool[candidateNode->PrevPhysical].NextPhysical = allocatedIndex;
				else
					mPhysicalListHead = allocatedIndex;

				candidateNode->PrevPhysical = allocatedIndex;

				candidateNode->Offset += size;
				candidateNode->Size -= size;
			}
			else
			{
				// Trailing remainder either splits off as a free node, or absorbs into the allocation if
				// it would be smaller than MinAllocationSize.
				if (remainder >= mMinAllocationSize)
				{
					const u32 trailingIndex = mPool.Allocate();
					candidateNode = &mPool[candidateIndex]; // Pool.Allocate may have invalidated references.

					Node& trailingNode = mPool[trailingIndex];
					trailingNode.Offset = candidateNode->Offset + size;
					trailingNode.Size = remainder;
					trailingNode.PrevPhysical = candidateIndex;
					trailingNode.NextPhysical = candidateNode->NextPhysical;
					trailingNode.PrevFree = Utility::kInvalidIndex;
					trailingNode.NextFree = Utility::kInvalidIndex;
					trailingNode.Flags = NodeFlag::Free;

					if (candidateNode->NextPhysical != Utility::kInvalidIndex)
						mPool[candidateNode->NextPhysical].PrevPhysical = trailingIndex;

					candidateNode->NextPhysical = trailingIndex;
					candidateNode->Size = size;

					InsertIntoFreeList(trailingIndex);
				}

				candidateNode->Flags = NodeFlags{}; // Not free, not null block.
				allocatedIndex = candidateIndex;
			}

			return allocatedIndex;
		}
	} // namespace detail::tlsf

	template <typename HeapBackend, ThreadSafetyPolicy ThreadPolicy>
	TTlsfAllocator<HeapBackend, ThreadPolicy>::TTlsfAllocator(HeapBackend* backend, const Configuration& configuration)
		: mBackend(backend), mConfig(configuration), mNextHeapSize(configuration.InitialHeapSize)
	{
		B3D_ASSERT(mBackend != nullptr);
		B3D_ASSERT(mConfig.GrowthFactor >= 1);
		B3D_ASSERT(mConfig.InitialHeapSize > 0);
		B3D_ASSERT(mConfig.MaxHeapSize >= mConfig.InitialHeapSize);
		B3D_ASSERT(mConfig.MinAllocationSize > 0);
		B3D_ASSERT(mConfig.Granularity == 1 || Bitwise::IsPow2(mConfig.Granularity));
		// Guards the bitmap-width constraint — sizes whose MSB exceeds this cap can't be bucketed.
		B3D_ASSERT(mConfig.MaxHeapSize < (1ull << (detail::tlsf::Utility::kFirstLevelClassCount + detail::tlsf::Utility::kMemoryClassShift)));
	}

	template <typename HeapBackend, ThreadSafetyPolicy ThreadPolicy>
	TTlsfAllocator<HeapBackend, ThreadPolicy>::~TTlsfAllocator()
	{
		for (u32 heapIndex = 0; heapIndex < (u32)mHeaps.size(); heapIndex++)
		{
			if (mHeaps[heapIndex] != nullptr)
			{
				mBackend->DestroyHeap(mHeaps[heapIndex]->Handle());
				B3DDelete(mHeaps[heapIndex]);
				mHeaps[heapIndex] = nullptr;
			}
		}
	}

	template <typename HeapBackend, ThreadSafetyPolicy ThreadPolicy>
	bool TTlsfAllocator<HeapBackend, ThreadPolicy>::TryAllocate(u64 size, u32 alignment, Allocation& out, TlsfAllocationKind kind)
	{
		B3D_ASSERT(alignment > 0);

		ScopedLock<kThreadSafe> lock(mLockPolicy);

		const u64 requestedSize = std::max(size, mConfig.MinAllocationSize);

		// Try existing heaps oldest-first so empty-spare slots drain before any new heap is created.
		for (u32 heapIndex = 0; heapIndex < (u32)mHeaps.size(); heapIndex++)
		{
			if (TryAllocateFromHeap(heapIndex, requestedSize, alignment, kind, out))
				return true;
		}

		// All existing heaps full — grow.
		const u64 newHeapSize = std::max(requestedSize, mNextHeapSize);
		const u32 newHeapIndex = CreateNewHeap(newHeapSize);
		if (newHeapIndex == detail::tlsf::Utility::kInvalidIndex)
			return false;

		const bool ok = TryAllocateFromHeap(newHeapIndex, requestedSize, alignment, kind, out);
		B3D_ASSERT(ok); // A fresh heap big enough for the request must satisfy it.
		return ok;
	}

	template <typename HeapBackend, ThreadSafetyPolicy ThreadPolicy>
	void TTlsfAllocator<HeapBackend, ThreadPolicy>::Free(const Allocation& allocation)
	{
		ScopedLock<kThreadSafe> lock(mLockPolicy);

		B3D_ASSERT(allocation.IsValid());
		B3D_ASSERT(allocation.HeapIndex < (u32)mHeaps.size());
		Heap* heap = mHeaps[allocation.HeapIndex];
		B3D_ASSERT(heap != nullptr);

		heap->FreeNode(allocation.NodeIndex);

		// Release a fully-empty heap if we're already over the spare budget.
		if (heap->LiveAllocCount() == 0)
		{
			if (mEmptyHeapCount < mConfig.MaxEmptyHeapCount)
				mEmptyHeapCount++;
			else
				DestroyHeap(allocation.HeapIndex);
		}
	}

	template <typename HeapBackend, ThreadSafetyPolicy ThreadPolicy>
	u64 TTlsfAllocator<HeapBackend, ThreadPolicy>::GetCommittedBytes() const
	{
		ScopedLock<kThreadSafe> lock(mLockPolicy);
		u64 total = 0;
		for (Heap* heap : mHeaps)
		{
			if (heap != nullptr)
				total += heap->TotalSize();
		}

		return total;
	}

	template <typename HeapBackend, ThreadSafetyPolicy ThreadPolicy>
	u64 TTlsfAllocator<HeapBackend, ThreadPolicy>::GetUsedBytes() const
	{
		ScopedLock<kThreadSafe> lock(mLockPolicy);
		u64 used = 0;
		for (Heap* heap : mHeaps)
		{
			if (heap != nullptr)
				used += heap->TotalSize() - heap->FreeSize();
		}

		return used;
	}

	template <typename HeapBackend, ThreadSafetyPolicy ThreadPolicy>
	u32 TTlsfAllocator<HeapBackend, ThreadPolicy>::GetHeapCount() const
	{
		ScopedLock<kThreadSafe> lock(mLockPolicy);
		u32 count = 0;
		for (Heap* heap : mHeaps)
		{
			if (heap != nullptr)
				count++;
		}

		return count;
	}

	template <typename HeapBackend, ThreadSafetyPolicy ThreadPolicy>
	u32 TTlsfAllocator<HeapBackend, ThreadPolicy>::GetEmptyHeapCount() const
	{
		ScopedLock<kThreadSafe> lock(mLockPolicy);
		return mEmptyHeapCount;
	}

	template <typename HeapBackend, ThreadSafetyPolicy ThreadPolicy>
	void TTlsfAllocator<HeapBackend, ThreadPolicy>::SetAllocationOwner(const Allocation& allocation, void* owner)
	{
		ScopedLock<kThreadSafe> lock(mLockPolicy);

		B3D_ASSERT(allocation.IsValid());
		B3D_ASSERT(allocation.HeapIndex < (u32)mHeaps.size());
		Heap* heap = mHeaps[allocation.HeapIndex];
		B3D_ASSERT(heap != nullptr);

		heap->SetNodeOwner(allocation.NodeIndex, owner);
	}

	template <typename HeapBackend, ThreadSafetyPolicy ThreadPolicy>
	bool TTlsfAllocator<HeapBackend, ThreadPolicy>::TryAllocateInHeapsAtMost(u64 size, u32 alignment, u32 maxHeapIndexInclusive, Allocation& out, TlsfAllocationKind kind)
	{
		ScopedLock<kThreadSafe> lock(mLockPolicy);

		if (mHeaps.empty())
			return false;

		const u64 requestedSize = std::max(size, mConfig.MinAllocationSize);

		// Walk heaps oldest-first (matches TryAllocate's order), but bounded and without growing.
		const u32 maxHeapIndex = std::min(maxHeapIndexInclusive, (u32)mHeaps.size() - 1);
		for (u32 heapIndex = 0; heapIndex <= maxHeapIndex; heapIndex++)
		{
			if (TryAllocateFromHeap(heapIndex, requestedSize, alignment, kind, out))
				return true;
		}

		return false;
	}

	template <typename HeapBackend, ThreadSafetyPolicy ThreadPolicy>
	u32 TTlsfAllocator<HeapBackend, ThreadPolicy>::CreateNewHeap(u64 sizeInBytes)
	{
		const HeapHandle handle = mBackend->CreateHeap(sizeInBytes, mConfig.HeapCreateInfo);
		if (handle == nullptr)
		{
			return detail::tlsf::Utility::kInvalidIndex;
		}

		Heap* heap = B3DNew<Heap>(handle, sizeInBytes,
			mConfig.Granularity, mConfig.GranularityDisableThreshold, mConfig.MinAllocationSize);

		// Empty heap counts as a "spare" against the warm-spare budget the moment it's created — it
		// already has zero live allocations.
		mEmptyHeapCount++;
		mNextHeapSize = std::min(mNextHeapSize * mConfig.GrowthFactor, mConfig.MaxHeapSize);

		// Reuse a vacated slot if one exists; otherwise grow the vector.
		for (u32 heapIndex = 0; heapIndex < (u32)mHeaps.size(); heapIndex++)
		{
			if (mHeaps[heapIndex] == nullptr)
			{
				mHeaps[heapIndex] = heap;
				return heapIndex;
			}
		}

		const u32 newIndex = (u32)mHeaps.size();
		mHeaps.push_back(heap);
		return newIndex;
	}

	template <typename HeapBackend, ThreadSafetyPolicy ThreadPolicy>
	void TTlsfAllocator<HeapBackend, ThreadPolicy>::DestroyHeap(u32 heapIndex)
	{
		Heap* heap = mHeaps[heapIndex];
		B3D_ASSERT(heap != nullptr);
		B3D_ASSERT(heap->LiveAllocCount() == 0);

		mBackend->DestroyHeap(heap->Handle());
		B3DDelete(heap);
		mHeaps[heapIndex] = nullptr;
	}

	template <typename HeapBackend, ThreadSafetyPolicy ThreadPolicy>
	bool TTlsfAllocator<HeapBackend, ThreadPolicy>::TryAllocateFromHeap(u32 heapIndex, u64 requestedSize, u32 alignment, TlsfAllocationKind kind, Allocation& out)
	{
		Heap* heap = mHeaps[heapIndex];
		if (heap == nullptr)
			return false;

		const bool heapWasEmpty = (heap->LiveAllocCount() == 0);
		u32 nodeIndex = detail::tlsf::Utility::kInvalidIndex;
		if (!heap->TryAllocate(requestedSize, alignment, kind, nodeIndex))
			return false;

		if (heapWasEmpty && mEmptyHeapCount > 0)
			mEmptyHeapCount--;

		const detail::tlsf::Node& allocatedNode = heap->GetNode(nodeIndex);
		out.Heap = heap->Handle();
		out.Offset = allocatedNode.Offset;
		out.Size = allocatedNode.Size;
		out.HeapIndex = heapIndex;
		out.NodeIndex = nodeIndex;
		return true;
	}
} // namespace b3d
