//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

// Template method definitions for TGpuTransientHeapPool. Not a translation unit of its own — included at the end of
// B3DGpuTransientHeapPool.h.

#include "GpuBackend/Allocators/B3DGpuTransientHeapPool.h"

namespace b3d
{
	template <typename HeapBackend>
	TGpuTransientHeapPool<HeapBackend>::TGpuTransientHeapPool(HeapBackend* backend, const Configuration& configuration)
		: mBackend(backend), mConfiguration(configuration)
	{
		B3D_ASSERT(backend != nullptr);
		B3D_ASSERT(configuration.CompletionTracker != nullptr);
		B3D_ASSERT(configuration.Granularity > 0 && Bitwise::IsPow2(configuration.Granularity));
	}

	template <typename HeapBackend>
	TGpuTransientHeapPool<HeapBackend>::~TGpuTransientHeapPool()
	{
		for(const Slot& slot : mSlots)
		{
			if(slot.Heap == nullptr)
				continue;

			B3D_ASSERT(slot.ReferenceCount == 0 && "Transient heap pool destroyed while a heap is still referenced.");
			mBackend->DestroyHeap(slot.Heap);
		}

		for(const ReleasedHeap& releasedHeap : mReleasedHeaps)
			mBackend->DestroyHeap(releasedHeap.Heap);
	}

	template <typename HeapBackend>
	u32 TGpuTransientHeapPool<HeapBackend>::GetSlotCount() const
	{
		Lock lock(mMutex);
		return (u32)mSlots.Size();
	}

	template <typename HeapBackend>
	GpuTransientHeap TGpuTransientHeapPool<HeapBackend>::AcquireHeap(u32 slotIndex, u64 minimumSize)
	{
		Lock lock(mMutex);
		if(slotIndex >= mSlots.Size())
			return GpuTransientHeap();

		Slot& slot = mSlots[slotIndex];
		if(slot.Heap == nullptr || slot.Size < minimumSize)
			return GpuTransientHeap(nullptr, slot.Size);

		slot.ReferenceCount++;
		slot.LastUsedFrame = mConfiguration.CompletionTracker->GetCurrentMarker();
		return GpuTransientHeap(slot.Heap, slot.Size);
	}

	template <typename HeapBackend>
	GpuTransientHeap TGpuTransientHeapPool<HeapBackend>::AcquireNewHeap(u32 slotIndex, u64 size)
	{
		Lock lock(mMutex);
		if(slotIndex >= mSlots.Size())
			mSlots.Resize(slotIndex + 1);

		// Another scope that ran out of memory at the same time already created the heap
		Slot& slot = mSlots[slotIndex];
		if(slot.Heap == nullptr)
		{
			const u64 minimumHeapSize = (u64)gGpuTransientHeapMinimumSize.Get() * 1024 * 1024;
			const u64 heapSize = Bitwise::AlignUp(std::max(minimumHeapSize, size), mConfiguration.Granularity);

			const HeapHandle heap = mBackend->CreateHeap(heapSize, mConfiguration.HeapCreateInformation);
			if(heap == nullptr)
				return GpuTransientHeap();

			slot.Heap = heap;
			slot.Size = heapSize;
			slot.ReferenceCount = 0;
		}

		slot.ReferenceCount++;
		slot.LastUsedFrame = mConfiguration.CompletionTracker->GetCurrentMarker();
		return GpuTransientHeap(slot.Heap, slot.Size);
	}

	template <typename HeapBackend>
	void TGpuTransientHeapPool<HeapBackend>::ReleaseHeap(IGpuHeap* heap)
	{
		Lock lock(mMutex);

		Slot& slot = FindSlot(heap);
		B3D_ASSERT(slot.ReferenceCount > 0);
		slot.ReferenceCount--;
		slot.LastUsedFrame = mConfiguration.CompletionTracker->GetCurrentMarker();
	}

	template <typename HeapBackend>
	void TGpuTransientHeapPool<HeapBackend>::AddHeapReference(IGpuHeap* heap)
	{
		Lock lock(mMutex);

		Slot& slot = FindSlot(heap);
		B3D_ASSERT(slot.ReferenceCount > 0 && "A heap reference can only be added by a caller that already references the heap.");
		slot.ReferenceCount++;
		slot.LastUsedFrame = mConfiguration.CompletionTracker->GetCurrentMarker();
	}

	template <typename HeapBackend>
	void TGpuTransientHeapPool<HeapBackend>::ReclaimUnused()
	{
		Lock lock(mMutex);

		IGpuCompletionTracker& completionTracker = *mConfiguration.CompletionTracker;
		const u64 currentFrame = completionTracker.GetCurrentMarker();
		const u32 idleFrames = gGpuTransientIdleFrames;

		// A referenced heap counts as used, so only heaps without references for long enough are released
		for(Slot& slot : mSlots)
		{
			if(slot.Heap == nullptr)
				continue;

			if(slot.ReferenceCount > 0)
				slot.LastUsedFrame = currentFrame;
			else if(currentFrame - slot.LastUsedFrame >= idleFrames)
			{
				// No GPU work uses the heap after the frame it was last used in
				mReleasedHeaps.Add(ReleasedHeap(slot.Heap, slot.LastUsedFrame));
				slot = Slot();
			}
		}

		// Trailing empty slots are dropped, so the slot count shrinks once the last heaps are released
		while(!mSlots.Empty() && mSlots.Back().Heap == nullptr)
			mSlots.Pop();

		// Destroy the released heaps the GPU has finished using
		u32 pendingHeapCount = 0;
		for(u32 releasedHeapIndex = 0; releasedHeapIndex < mReleasedHeaps.Size(); releasedHeapIndex++)
		{
			const ReleasedHeap releasedHeap = mReleasedHeaps[releasedHeapIndex];
			if(completionTracker.IsMarkerComplete(releasedHeap.Marker))
				mBackend->DestroyHeap(releasedHeap.Heap);
			else
				mReleasedHeaps[pendingHeapCount++] = releasedHeap;
		}

		mReleasedHeaps.Erase(mReleasedHeaps.Begin() + pendingHeapCount, mReleasedHeaps.End());
	}

	template <typename HeapBackend>
	GpuTransientHeapPoolStatistics TGpuTransientHeapPool<HeapBackend>::GetStatistics() const
	{
		Lock lock(mMutex);

		GpuTransientHeapPoolStatistics output;
		for(const Slot& slot : mSlots)
		{
			if(slot.Heap == nullptr)
				continue;

			output.HeapCount++;
			output.Capacity += slot.Size;
		}

		return output;
	}

	template <typename HeapBackend>
	typename TGpuTransientHeapPool<HeapBackend>::Slot& TGpuTransientHeapPool<HeapBackend>::FindSlot(IGpuHeap* heap)
	{
		B3D_ASSERT(heap != nullptr);

		Slot* output = nullptr;
		for(Slot& slot : mSlots)
		{
			if(slot.Heap == heap)
				output = &slot;
		}

		B3D_ASSERT(output != nullptr && "Heap does not belong to this transient heap pool.");
		return *output;
	}
} // namespace b3d
