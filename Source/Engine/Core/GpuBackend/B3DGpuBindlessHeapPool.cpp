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

GpuBindlessHeapPool::GpuBindlessHeapPool(u32 maximumHeapCount)
	: mMaximumHeapCount(std::max(maximumHeapCount, 1u))
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

void GpuBindlessHeapPool::Initialize(GpuBindlessHeap* heap)
{
	B3D_ASSERT(mCurrent == nullptr && heap != nullptr);

	heap->mVersion = mNextVersion++;
	heap->mIsCurrent = true;

	Lock lock(mMutex);
	mCurrent = heap;
}

GpuBindlessHeap* GpuBindlessHeapPool::GetAndTrack(render::GpuResourceTracker& resourceTracker)
{
	// Tracking under the lock keeps the native heap from being reused between the capture and the tracking
	Lock lock(mMutex);

	resourceTracker.TrackResourceUsage(mCurrent, GpuAccessFlag::Read);
	return mCurrent;
}

GpuBindlessHeap* GpuBindlessHeapPool::TakePooledHeap()
{
	Lock lock(mMutex);
	if(mPooledHeaps.Empty())
		return nullptr;

	GpuBindlessHeap* heap = mPooledHeaps.Back();
	mPooledHeaps.Pop();
	return heap;
}

void GpuBindlessHeapPool::MakeCurrent(GpuBindlessHeap* heap, TArrayView<const GpuBindlessSlotUpdate> updates)
{
	heap->mDirtySlots.Reset(false);
	heap->mVersion = mNextVersion++;

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

	mCurrent->mIsCurrent = false;
	mRetiringHeaps.Add(mCurrent);

	heap->mIsCurrent = true;
	mCurrent = heap;

	// Pooled native heaps are too small to be reused once the resource heap grows
	for(u32 heapIndex = 0; heapIndex < mPooledHeaps.Size();)
	{
		if(mPooledHeaps[heapIndex]->mCapacity == heap->mCapacity)
		{
			heapIndex++;
			continue;
		}

		mPooledHeaps[heapIndex]->Destroy();
		mPooledHeaps.Remove(heapIndex);
	}
}

void GpuBindlessHeapPool::MarkSlotWritten(u32 slotIndex)
{
	Lock lock(mMutex);

	// Retiring native heaps are only read by work recorded before the write, but may be pooled later
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
