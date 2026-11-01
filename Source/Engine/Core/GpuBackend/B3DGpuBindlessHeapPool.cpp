//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "GpuBackend/B3DGpuBindlessHeapPool.h"
#include "GpuBackend/B3DGpuResourceTracker.h"

using namespace b3d;

GpuBindlessHeap::GpuBindlessHeap(GpuResourceManager* owner, u32 capacity)
	: IGpuResource(owner, "Bindless heap"), mCapacity(capacity)
{
	mDirtySlots.Resize(capacity);
}

GpuBindlessHeapPool::GpuBindlessHeapPool(IGpuBindlessHeapPoolBackend& backend, u32 maximumHeapCount)
	: mBackend(backend), mMaximumHeapCount(std::max(maximumHeapCount, 1u))
{ }

GpuBindlessHeapPool::~GpuBindlessHeapPool()
{
	Lock lock(mMutex);

	if(mCurrent != nullptr)
		mCurrent->Destroy();

	for(GpuBindlessHeap* heap : mRetiringHeaps)
		heap->Destroy();

	for(GpuBindlessHeap* heap : mPooledHeaps)
		heap->Destroy();
}

bool GpuBindlessHeapPool::Initialize(u32 capacity)
{
	B3D_ASSERT(mCurrent == nullptr);

	GpuBindlessHeap* heap = mBackend.CreateHeap(capacity);
	if(heap == nullptr)
		return false;

	heap->mVersion = mNextVersion++;
	heap->mIsCurrent = true;

	Lock lock(mMutex);
	mCurrent = heap;
	return true;
}

GpuBindlessHeap* GpuBindlessHeapPool::GetAndTrack(render::GpuResourceTracker& resourceTracker)
{
	// Tracking under the lock keeps the heap from being reused between the capture and the tracking
	Lock lock(mMutex);

	resourceTracker.TrackResourceUsage(mCurrent, GpuAccessFlag::Read);
	return mCurrent;
}

void GpuBindlessHeapPool::CopyLiveSlots(GpuBindlessHeap& destination, const TBitfield<>& slots, const TBitfield<>& liveSlots)
{
	TArray<TIndexRange<u32>> ranges;
	slots.ForEachSetRange(liveSlots, std::min(destination.mCapacity, mCurrent->mCapacity), [&ranges](const TIndexRange<u64>& range)
	{
		ranges.Add(TIndexRange<u32>((u32)range.First, (u32)range.Count));
	});

	if(!ranges.Empty())
		mBackend.CopySlots(destination, *mCurrent, ranges);
}

void GpuBindlessHeapPool::MakeCurrent(GpuBindlessHeap* heap)
{
	Lock lock(mMutex);

	mCurrent->mIsCurrent = false;
	mRetiringHeaps.Add(mCurrent);

	heap->mIsCurrent = true;
	mCurrent = heap;
}

bool GpuBindlessHeapPool::ApplyUpdates(TArrayView<const GpuBindlessSlotUpdate> updates, const TBitfield<>& liveSlots)
{
	const u32 capacity = mCurrent->mCapacity;

	// A pooled heap only lacks the slots changed since it was last current. A new one lacks every slot.
	GpuBindlessHeap* heap = nullptr;
	{
		Lock lock(mMutex);
		if(!mPooledHeaps.Empty())
		{
			heap = mPooledHeaps.Back();
			mPooledHeaps.Pop();
		}
	}

	if(heap != nullptr)
		CopyLiveSlots(*heap, heap->mDirtySlots, liveSlots);
	else
	{
		heap = mBackend.CreateHeap(capacity);
		if(heap == nullptr)
			return false;

		CopyLiveSlots(*heap, liveSlots, liveSlots);
	}

	heap->mDirtySlots.Reset(false);
	heap->mVersion = mNextVersion++;

	mBackend.WriteUpdates(*heap, updates);

	// Every other heap now lacks the updated slots
	{
		Lock lock(mMutex);

		const auto fnMarkDirty = [&updates](GpuBindlessHeap& otherHeap)
		{
			for(const GpuBindlessSlotUpdate& update : updates)
			{
				if(update.SlotIndex < otherHeap.mCapacity)
					otherHeap.mDirtySlots[update.SlotIndex] = true;
			}
		};

		fnMarkDirty(*mCurrent);
		for(GpuBindlessHeap* otherHeap : mRetiringHeaps)
			fnMarkDirty(*otherHeap);

		for(GpuBindlessHeap* otherHeap : mPooledHeaps)
			fnMarkDirty(*otherHeap);
	}

	MakeCurrent(heap);
	return true;
}

bool GpuBindlessHeapPool::Grow(u32 capacity, const TBitfield<>& liveSlots)
{
	B3D_ASSERT(capacity > mCurrent->mCapacity);

	GpuBindlessHeap* heap = mBackend.CreateHeap(capacity);
	if(heap == nullptr)
		return false;

	CopyLiveSlots(*heap, liveSlots, liveSlots);
	heap->mVersion = mNextVersion++;

	MakeCurrent(heap);

	// Pooled heaps are too small to be reused
	Lock lock(mMutex);
	for(GpuBindlessHeap* pooledHeap : mPooledHeaps)
		pooledHeap->Destroy();

	mPooledHeaps.Clear();
	return true;
}

void GpuBindlessHeapPool::MarkSlotWritten(u32 slotIndex)
{
	Lock lock(mMutex);

	// Retiring heaps are only read by work recorded before the write, but may be pooled later
	for(GpuBindlessHeap* heap : mRetiringHeaps)
	{
		if(slotIndex < heap->mCapacity)
			heap->mDirtySlots[slotIndex] = true;
	}

	for(GpuBindlessHeap* heap : mPooledHeaps)
		heap->mDirtySlots[slotIndex] = true;
}

void GpuBindlessHeapPool::ProcessRetiredHeaps()
{
	Lock lock(mMutex);

	for(u32 heapIndex = 0; heapIndex < mRetiringHeaps.Size();)
	{
		GpuBindlessHeap* heap = mRetiringHeaps[heapIndex];
		if(heap->IsBound())
		{
			heapIndex++;
			continue;
		}

		mRetiringHeaps.Remove(heapIndex);

		const u32 aliveHeapCount = 1 + (u32)mRetiringHeaps.Size() + (u32)mPooledHeaps.Size();
		if(heap->mCapacity == mCurrent->mCapacity && aliveHeapCount < mMaximumHeapCount)
			mPooledHeaps.Add(heap);
		else
			heap->Destroy();
	}
}
