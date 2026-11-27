//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

// Template method definitions for TGpuPagePool. Not a translation unit of its own — included at the end of B3DGpuPagePool.h.

#include "GpuBackend/Allocators/B3DGpuPagePool.h"

namespace b3d
{
	template <typename HeapBackend>
	TGpuPagePool<HeapBackend>::TGpuPagePool(HeapBackend* backend, const Configuration& configuration)
		: mBackend(backend), mConfiguration(configuration)
	{
		B3D_ASSERT(backend != nullptr);
		B3D_ASSERT(configuration.CompletionTracker != nullptr);
	}

	template <typename HeapBackend>
	TGpuPagePool<HeapBackend>::~TGpuPagePool()
	{
		for(const Heap& heap : mHeaps)
		{
			B3D_ASSERT(heap.FreePageCount + heap.PendingFreePageCount == heap.PageCount && "Page pool destroyed while pages are still allocated.");
			mBackend->DestroyHeap(heap.Handle);
		}
	}

	template <typename HeapBackend>
	bool TGpuPagePool<HeapBackend>::TryAllocate(u32 pageCount, TArray<GpuPageAllocation>& outAllocations)
	{
		if(pageCount == 0)
			return true;

		Lock lock(mMutex);
		const u64 currentFrame = mConfiguration.CompletionTracker->GetCurrentMarker();

		// The smallest free run that holds all pages keeps the longer runs for larger requests
		// TODO - Linear in the free runs of all heaps, as are TakeFreePages(), AddFreePages() and the sort below. Index the runs by size
		// class with bitmaps (TLSF-like) for constant time fits, and binary search the sorted runs of a heap.
		u32 bestHeapIndex = ~0u;
		GpuPageRange bestRun;
		for(u32 heapIndex = 0; heapIndex < (u32)mHeaps.Size(); heapIndex++)
		{
			for(const GpuPageRange& run : mHeaps[heapIndex].FreeRuns)
			{
				if(run.PageCount >= pageCount && (bestHeapIndex == ~0u || run.PageCount < bestRun.PageCount))
				{
					bestHeapIndex = heapIndex;
					bestRun = run;
				}
			}
		}

		if(bestHeapIndex != ~0u)
		{
			Heap& heap = mHeaps[bestHeapIndex];
			const GpuPageRange pages(bestRun.FirstPageIndex, pageCount);

			TakeFreePages(heap, pages);
			heap.LastUsedFrame = currentFrame;
			outAllocations.Add(GpuPageAllocation(heap.Handle, pages));
			return true;
		}

		// Heaps are added for the pages the free runs lack. A new heap is one run, so it is taken first.
		u32 freePageCount = 0;
		for(const Heap& heap : mHeaps)
			freePageCount += heap.FreePageCount;

		const u64 heapSize = (u64)gGpuVirtualMemoryPageHeapSize.Get() * 1024 * 1024;
		const u32 heapPageCount = (u32)std::max((heapSize + kGpuPageSize - 1) / kGpuPageSize, (u64)1);

		u32 createdHeapCount = 0;
		while(freePageCount < pageCount)
		{
			const HeapHandle handle = mBackend->CreateHeap((u64)heapPageCount * kGpuPageSize, mConfiguration.HeapCreateInformation);
			if(handle == nullptr)
			{
				// All or nothing, so the heaps created for the request go too
				for(u32 heapIndex = 0; heapIndex < createdHeapCount; heapIndex++)
				{
					mBackend->DestroyHeap(mHeaps.Back().Handle);
					mHeaps.Pop();
				}

				return false;
			}

			Heap heap;
			heap.Handle = handle;
			heap.PageCount = heapPageCount;
			heap.FreePageCount = heapPageCount;
			heap.LastUsedFrame = currentFrame;
			heap.FreeRuns.Add(GpuPageRange(0, heapPageCount));

			mHeaps.Add(std::move(heap));
			createdHeapCount++;
			freePageCount += heapPageCount;
		}

		// Free run of a heap, referenced by heap index because taking pages changes the runs
		struct FreeRun
		{
			FreeRun(u32 heapIndex, const GpuPageRange& pages)
				: HeapIndex(heapIndex), Pages(pages)
			{ }

			u32 HeapIndex;
			GpuPageRange Pages;
		};

		FrameAllocatorScope frameScope;
		FrameVector<FreeRun> freeRuns;
		for(u32 heapIndex = 0; heapIndex < (u32)mHeaps.Size(); heapIndex++)
		{
			for(const GpuPageRange& run : mHeaps[heapIndex].FreeRuns)
				freeRuns.push_back(FreeRun(heapIndex, run));
		}

		// Runs of the same length are taken in heap and page order, so the result doesn't depend on the sort
		std::sort(freeRuns.begin(), freeRuns.end(), [](const FreeRun& lhs, const FreeRun& rhs)
		{
			if(lhs.Pages.PageCount != rhs.Pages.PageCount)
				return lhs.Pages.PageCount > rhs.Pages.PageCount;

			if(lhs.HeapIndex != rhs.HeapIndex)
				return lhs.HeapIndex < rhs.HeapIndex;

			return lhs.Pages.FirstPageIndex < rhs.Pages.FirstPageIndex;
		});

		u32 remainingPageCount = pageCount;
		for(const FreeRun& freeRun : freeRuns)
		{
			if(remainingPageCount == 0)
				break;

			Heap& heap = mHeaps[freeRun.HeapIndex];
			const GpuPageRange pages(freeRun.Pages.FirstPageIndex, std::min(freeRun.Pages.PageCount, remainingPageCount));

			TakeFreePages(heap, pages);
			heap.LastUsedFrame = currentFrame;
			outAllocations.Add(GpuPageAllocation(heap.Handle, pages));
			remainingPageCount -= pages.PageCount;
		}

		B3D_ASSERT(remainingPageCount == 0);
		return true;
	}

	template <typename HeapBackend>
	void TGpuPagePool<HeapBackend>::Free(const GpuPageAllocation& allocation)
	{
		if(allocation.Pages.PageCount == 0)
			return;

		Lock lock(mMutex);

		Heap& heap = FindHeap(allocation.Heap);
		B3D_ASSERT(allocation.Pages.GetEndPageIndex() <= heap.PageCount);

		heap.PendingFreePageCount += allocation.Pages.PageCount;
		mPendingFrees.Add(PendingFree(allocation, mConfiguration.CompletionTracker->GetCurrentMarker()));
	}

	template <typename HeapBackend>
	void TGpuPagePool<HeapBackend>::FreeAndReclaim(const GpuPageAllocation& allocation)
	{
		if(allocation.Pages.PageCount == 0)
			return;

		Lock lock(mMutex);

		Heap& heap = FindHeap(allocation.Heap);
		AddFreePages(heap, allocation.Pages);
		heap.LastUsedFrame = mConfiguration.CompletionTracker->GetCurrentMarker();
	}

	template <typename HeapBackend>
	void TGpuPagePool<HeapBackend>::ReclaimUnused()
	{
		Lock lock(mMutex);

		IGpuCompletionTracker& completionTracker = *mConfiguration.CompletionTracker;
		const u64 currentFrame = completionTracker.GetCurrentMarker();
		const u32 idleFrames = gGpuVirtualMemoryPageHeapIdleFrames;

		// Pending frees are in the order of their markers, but a later one may still complete while an earlier one is kept
		u32 pendingFreeCount = 0;
		for(u32 pendingFreeIndex = 0; pendingFreeIndex < (u32)mPendingFrees.Size(); pendingFreeIndex++)
		{
			const PendingFree pendingFree = mPendingFrees[pendingFreeIndex];
			if(!completionTracker.IsMarkerComplete(pendingFree.Marker))
			{
				mPendingFrees[pendingFreeCount++] = pendingFree;
				continue;
			}

			Heap& heap = FindHeap(pendingFree.Allocation.Heap);
			heap.PendingFreePageCount -= pendingFree.Allocation.Pages.PageCount;
			AddFreePages(heap, pendingFree.Allocation.Pages);
		}

		mPendingFrees.Erase(mPendingFrees.Begin() + pendingFreeCount, mPendingFrees.End());

		// Heaps whose pages all went unused for long enough are destroyed, once the GPU finished the last frame that used them. That frame
		// also covers the mappings that took the heap's memory away from resources, since FreeAndReclaim() marks the heap as used and the
		// frame completes only after the mappings queued in it execute. Up to MaxEmptyHeapCount heaps without allocated pages are kept.
		u32 emptyHeapCount = 0;
		for(u32 heapIndex = 0; heapIndex < (u32)mHeaps.Size();)
		{
			Heap& heap = mHeaps[heapIndex];
			if(heap.FreePageCount < heap.PageCount)
				heap.LastUsedFrame = currentFrame;
			else if(emptyHeapCount < mConfiguration.MaxEmptyHeapCount)
				emptyHeapCount++;
			else if(currentFrame - heap.LastUsedFrame >= idleFrames && completionTracker.IsMarkerComplete(heap.LastUsedFrame))
			{
				mBackend->DestroyHeap(heap.Handle);
				mHeaps.Remove(heapIndex);
				continue;
			}

			heapIndex++;
		}
	}

	template <typename HeapBackend>
	GpuPagePoolStatistics TGpuPagePool<HeapBackend>::GetStatistics() const
	{
		Lock lock(mMutex);

		GpuPagePoolStatistics output;
		for(const Heap& heap : mHeaps)
		{
			output.HeapCount++;
			output.PageCount += heap.PageCount;
			output.AllocatedPageCount += heap.PageCount - heap.FreePageCount;
		}

		return output;
	}

	template <typename HeapBackend>
	typename TGpuPagePool<HeapBackend>::Heap& TGpuPagePool<HeapBackend>::FindHeap(IGpuHeap* handle)
	{
		B3D_ASSERT(handle != nullptr);

		Heap* output = nullptr;
		for(Heap& heap : mHeaps)
		{
			if(heap.Handle == handle)
				output = &heap;
		}

		B3D_ASSERT(output != nullptr && "Heap does not belong to this page pool.");
		return *output;
	}

	template <typename HeapBackend>
	void TGpuPagePool<HeapBackend>::TakeFreePages(Heap& heap, const GpuPageRange& pages)
	{
		for(u32 runIndex = 0; runIndex < (u32)heap.FreeRuns.Size(); runIndex++)
		{
			GpuPageRange& run = heap.FreeRuns[runIndex];
			if(run.FirstPageIndex != pages.FirstPageIndex)
				continue;

			B3D_ASSERT(pages.PageCount <= run.PageCount);
			if(pages.PageCount == run.PageCount)
				heap.FreeRuns.Remove(runIndex);
			else
			{
				run.FirstPageIndex += pages.PageCount;
				run.PageCount -= pages.PageCount;
			}

			heap.FreePageCount -= pages.PageCount;
			return;
		}

		B3D_ASSERT(false && "Pages are not at the start of a free run.");
	}

	template <typename HeapBackend>
	void TGpuPagePool<HeapBackend>::AddFreePages(Heap& heap, const GpuPageRange& pages)
	{
		B3D_ASSERT(pages.GetEndPageIndex() <= heap.PageCount);

		u32 nextRunIndex = 0;
		while(nextRunIndex < (u32)heap.FreeRuns.Size() && heap.FreeRuns[nextRunIndex].FirstPageIndex < pages.FirstPageIndex)
			nextRunIndex++;

		const bool hasPreviousRun = nextRunIndex > 0;
		const bool hasNextRun = nextRunIndex < (u32)heap.FreeRuns.Size();
		B3D_ASSERT((!hasPreviousRun || heap.FreeRuns[nextRunIndex - 1].GetEndPageIndex() <= pages.FirstPageIndex) && "Pages are freed twice.");
		B3D_ASSERT((!hasNextRun || pages.GetEndPageIndex() <= heap.FreeRuns[nextRunIndex].FirstPageIndex) && "Pages are freed twice.");

		const bool mergesPrevious = hasPreviousRun && heap.FreeRuns[nextRunIndex - 1].GetEndPageIndex() == pages.FirstPageIndex;
		const bool mergesNext = hasNextRun && pages.GetEndPageIndex() == heap.FreeRuns[nextRunIndex].FirstPageIndex;

		if(mergesPrevious && mergesNext)
		{
			heap.FreeRuns[nextRunIndex - 1].PageCount += pages.PageCount + heap.FreeRuns[nextRunIndex].PageCount;
			heap.FreeRuns.Remove(nextRunIndex);
		}
		else if(mergesPrevious)
			heap.FreeRuns[nextRunIndex - 1].PageCount += pages.PageCount;
		else if(mergesNext)
		{
			heap.FreeRuns[nextRunIndex].FirstPageIndex = pages.FirstPageIndex;
			heap.FreeRuns[nextRunIndex].PageCount += pages.PageCount;
		}
		else
			heap.FreeRuns.Insert(heap.FreeRuns.Begin() + nextRunIndex, pages);

		heap.FreePageCount += pages.PageCount;
	}
} // namespace b3d
