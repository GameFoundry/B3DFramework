//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "GpuBackend/Allocators/B3DGpuAliasingAllocator.h"

namespace b3d
{
	TConfigVariable<bool> gGpuTransientAliasing("gpu.TransientAliasing",
		"If disabled, no two transient resources of one scope share memory. Scopes still share memory with each other.", true);
	TConfigVariable<bool> gGpuTransientAliasAcrossQueues("gpu.TransientAliasAcrossQueues",
		"If disabled, memory last used by a transient resource on another GPU queue in the same scope is not reused.", true);
} // namespace b3d

using namespace b3d;

GpuAliasingAllocator::~GpuAliasingAllocator()
{
	B3D_ASSERT(mTimeline == nullptr && "Aliasing allocator destroyed while a scope is open.");
}

void GpuAliasingAllocator::BeginScope(const GpuSubmissionTimeline& timeline)
{
	B3D_ASSERT(mTimeline == nullptr && "Only one aliasing allocator scope may be open at a time.");

	mTimeline = &timeline;
	mIsAliasingEnabled = gGpuTransientAliasing;
	mIsAliasingAcrossQueuesEnabled = gGpuTransientAliasAcrossQueues;

	mUsedBytes = 0;
	mPeakUsedBytes = 0;
	mRequestedBytes = 0;
}

void GpuAliasingAllocator::EndScope()
{
	B3D_ASSERT(mTimeline != nullptr && "No aliasing allocator scope is open.");

	// The scope's work is synchronized once the scope ends, so its tags and live allocations have nothing left to order
	for(const Heap& heap : mHeaps)
		heap.Pool->ReleaseHeap(heap.Handle);

	mHeaps.Clear();
	mTags.Clear();
	mTagUses.Clear();
	mAllocations.Clear();
	mInheritedSegments.Clear();
	mFreeAllocationIndices.Clear();
	mTimeline = nullptr;
}

bool GpuAliasingAllocator::TryAllocate(IGpuTransientHeapPool& pool, u64 size, u64 alignment, u32 firstSubmission, GpuAllocation& outAllocation, render::GpuAliasAcquire& outAcquire)
{
	B3D_ASSERT(mTimeline != nullptr && "Aliased memory can only be allocated within a scope.");
	B3D_ASSERT(firstSubmission < mTimeline->GetSubmissionCount());
	B3D_ASSERT(alignment > 0 && Bitwise::IsPow2(alignment));

	const u64 granularity = pool.GetGranularity();
	const u64 allocationSize = Bitwise::AlignUp(std::max(size, (u64)1), granularity);
	const u64 allocationAlignment = std::max(alignment, granularity);

	// First fit over the pool's heaps in slot order. A heap the scope has not placed memory on yet is entirely free.
	const u32 slotCount = pool.GetSlotCount();
	u32 firstEmptySlot = kInvalidIndex;
	for(u32 slot = 0; slot < slotCount; slot++)
	{
		const u32 heapIndex = FindHeap(pool, slot);
		if(heapIndex == kInvalidIndex)
		{
			const GpuTransientHeap heap = pool.AcquireHeap(slot, allocationSize);
			if(heap.Heap != nullptr)
			{
				Place(AddHeap(pool, slot, heap), 0, 0, 0, allocationSize, firstSubmission, outAllocation, outAcquire);
				return true;
			}

			if(heap.Size == 0 && firstEmptySlot == kInvalidIndex)
				firstEmptySlot = slot;

			continue;
		}

		// Find the first run of adjacent eligible segments that fits the allocation
		const Heap& heap = mHeaps[heapIndex];
		u32 runStartIndex = kInvalidIndex;
		for(u32 segmentIndex = 0; segmentIndex < heap.FreeSegments.Size(); segmentIndex++)
		{
			const Segment& segment = heap.FreeSegments[segmentIndex];
			if(!IsEligible(segment.Tag, firstSubmission))
			{
				runStartIndex = kInvalidIndex;
				continue;
			}

			if(runStartIndex == kInvalidIndex || heap.FreeSegments[segmentIndex - 1].GetEnd() != segment.Offset)
				runStartIndex = segmentIndex;

			const u64 offset = Bitwise::AlignUp(heap.FreeSegments[runStartIndex].Offset, allocationAlignment);
			if(offset + allocationSize > segment.GetEnd())
				continue;

			// Segments that only pad the start of the run to the alignment stay free
			u32 firstSegmentIndex = runStartIndex;
			while(heap.FreeSegments[firstSegmentIndex].GetEnd() <= offset)
				firstSegmentIndex++;

			Place(heapIndex, firstSegmentIndex, segmentIndex, offset, allocationSize, firstSubmission, outAllocation, outAcquire);
			return true;
		}
	}

	// Nothing fits, so create a heap in the first empty slot, or past the last slot. Scopes that run out of memory at the same time
	// ask for the same slot, so only one of them creates a heap.
	for(u32 slot = firstEmptySlot != kInvalidIndex ? firstEmptySlot : slotCount; ; slot++)
	{
		if(FindHeap(pool, slot) != kInvalidIndex)
			continue;

		const GpuTransientHeap heap = pool.AcquireNewHeap(slot, allocationSize);
		if(heap.Heap == nullptr)
			return false;

		// Another scope created a heap too small for this allocation in the slot first
		if(heap.Size < allocationSize)
		{
			pool.ReleaseHeap(heap.Heap);
			continue;
		}

		Place(AddHeap(pool, slot, heap), 0, 0, 0, allocationSize, firstSubmission, outAllocation, outAcquire);
		return true;
	}
}

bool GpuAliasingAllocator::TryAllocateAt(IGpuTransientHeapPool& pool, const GpuAllocation& location, u32 firstSubmission, GpuAllocation& outAllocation, render::GpuAliasAcquire& outAcquire)
{
	B3D_ASSERT(mTimeline != nullptr && "Aliased memory can only be allocated within a scope.");
	B3D_ASSERT(firstSubmission < mTimeline->GetSubmissionCount());
	B3D_ASSERT(location.HasMemory());

	const u32 slot = location.AllocatorData0;
	const u64 locationEnd = location.Offset + location.Size;

	u32 heapIndex = FindHeap(pool, slot);
	if(heapIndex == kInvalidIndex)
	{
		const GpuTransientHeap heap = pool.AcquireHeap(slot, locationEnd);
		if(heap.Heap == nullptr)
			return false;

		B3D_ASSERT(heap.Heap == location.Heap && "The heap of a reused location must stay referenced.");
		heapIndex = AddHeap(pool, slot, heap);
	}

	// The location must lie in one run of adjacent eligible segments
	const Heap& heap = mHeaps[heapIndex];
	B3D_ASSERT(heap.Handle == location.Heap && "The heap of a reused location must stay referenced.");

	u32 firstSegmentIndex = kInvalidIndex;
	for(u32 segmentIndex = 0; segmentIndex < heap.FreeSegments.Size(); segmentIndex++)
	{
		const Segment& segment = heap.FreeSegments[segmentIndex];
		if(firstSegmentIndex == kInvalidIndex)
		{
			if(segment.GetEnd() <= location.Offset)
				continue;

			if(segment.Offset > location.Offset)
				return false;

			firstSegmentIndex = segmentIndex;
		}
		else if(heap.FreeSegments[segmentIndex - 1].GetEnd() != segment.Offset)
			return false;

		if(!IsEligible(segment.Tag, firstSubmission))
			return false;

		if(segment.GetEnd() >= locationEnd)
		{
			Place(heapIndex, firstSegmentIndex, segmentIndex, location.Offset, location.Size, firstSubmission, outAllocation, outAcquire);
			return true;
		}
	}

	return false;
}

void GpuAliasingAllocator::Release(const GpuAllocation& allocation, TArrayView<const GpuTransientLastUse> lastUses, IGpuResource* owner)
{
	B3D_ASSERT(mTimeline != nullptr && "Aliased memory can only be released within a scope.");

	Allocation& liveAllocation = GetAllocation(allocation);
	Heap& heap = mHeaps[liveAllocation.HeapIndex];

	// A resource that was never accessed leaves the memory with the uses it was placed over
	if(lastUses.IsEmpty())
	{
		for(u32 segmentIndex = 0; segmentIndex < liveAllocation.InheritedSegmentCount; segmentIndex++)
			InsertFreeSegment(heap, mInheritedSegments[liveAllocation.FirstInheritedSegment + segmentIndex]);
	}
	else
	{
		Tag tag;
		tag.FirstUse = (u32)mTagUses.Size();

#if B3D_BUILD_TYPE_DEVELOPMENT
		tag.Owner = owner;
#endif

		for(const GpuTransientLastUse& lastUse : lastUses)
		{
			B3D_ASSERT(lastUse.Submission < mTimeline->GetSubmissionCount());

			const GpuQueueId queue = mTimeline->GetQueue(lastUse.Submission);
			TagUse* existingUse = nullptr;
			for(u32 useIndex = tag.FirstUse; useIndex < mTagUses.Size(); useIndex++)
			{
				if(mTagUses[useIndex].Queue.Id == queue.Id)
					existingUse = &mTagUses[useIndex];
			}

			if(existingUse == nullptr)
				mTagUses.Add(TagUse(queue, lastUse.Submission, lastUse.Access));
			else
			{
				existingUse->LastSubmission = std::max(existingUse->LastSubmission, lastUse.Submission);
				existingUse->Access.Add(lastUse.Access);
			}
		}

		tag.UseCount = (u32)mTagUses.Size() - tag.FirstUse;

		const u32 tagIndex = (u32)mTags.Size();
		mTags.Add(tag);

		InsertFreeSegment(heap, Segment(liveAllocation.Offset, liveAllocation.Size, tagIndex));
	}

	(void)owner;

	mUsedBytes -= liveAllocation.Size;
	liveAllocation.HeapIndex = kInvalidIndex;
	liveAllocation.InheritedSegmentCount = 0;
	mFreeAllocationIndices.Add(allocation.AllocatorData1);
}

GpuTransientScopeStatistics GpuAliasingAllocator::GetStatistics() const
{
	GpuTransientScopeStatistics output;
	output.PeakUsed = mPeakUsedBytes;
	output.RequestedBytes = mRequestedBytes;

	return output;
}

u32 GpuAliasingAllocator::FindHeap(const IGpuTransientHeapPool& pool, u32 slot) const
{
	for(u32 heapIndex = 0; heapIndex < mHeaps.Size(); heapIndex++)
	{
		if(mHeaps[heapIndex].Pool == &pool && mHeaps[heapIndex].Slot == slot)
			return heapIndex;
	}

	return kInvalidIndex;
}

u32 GpuAliasingAllocator::AddHeap(IGpuTransientHeapPool& pool, u32 slot, const GpuTransientHeap& heap)
{
	const u32 heapIndex = (u32)mHeaps.Size();
	mHeaps.Add(Heap(&pool, slot, heap));
	mHeaps.Back().FreeSegments.Add(Segment(0, heap.Size, kNoTag));

	return heapIndex;
}

bool GpuAliasingAllocator::IsEligible(u32 tagIndex, u32 firstSubmission) const
{
	if(tagIndex == kNoTag)
		return true;

	if(!mIsAliasingEnabled)
		return false;

	// Every last use must be ordered before the first use. The acquire's barrier then orders the first use after the last use on
	// its own queue, and a queue wait orders it after the last uses on other queues.
	const GpuQueueId firstQueue = mTimeline->GetQueue(firstSubmission);
	const Tag& tag = mTags[tagIndex];
	for(u32 useIndex = tag.FirstUse; useIndex < tag.FirstUse + tag.UseCount; useIndex++)
	{
		const TagUse& use = mTagUses[useIndex];
		if(use.Queue.Id != firstQueue.Id && !mIsAliasingAcrossQueuesEnabled)
			return false;

		if(!mTimeline->IsOrderedBefore(use.LastSubmission, firstSubmission))
			return false;
	}

	return true;
}

void GpuAliasingAllocator::Place(u32 heapIndex, u32 firstSegmentIndex, u32 lastSegmentIndex, u64 offset, u64 size, u32 firstSubmission, GpuAllocation& outAllocation,
	render::GpuAliasAcquire& outAcquire)
{
	Heap& heap = mHeaps[heapIndex];

	u32 allocationIndex;
	if(!mFreeAllocationIndices.Empty())
	{
		allocationIndex = mFreeAllocationIndices.Back();
		mFreeAllocationIndices.Pop();
	}
	else
	{
		allocationIndex = (u32)mAllocations.Size();
		mAllocations.Add(Allocation());
	}

	Allocation& allocation = mAllocations[allocationIndex];
	allocation.HeapIndex = heapIndex;
	allocation.Offset = offset;
	allocation.Size = size;
	allocation.FirstInheritedSegment = (u32)mInheritedSegments.Size();
	allocation.InheritedSegmentCount = lastSegmentIndex - firstSegmentIndex + 1;

	// Order the first use after the last uses of the earlier resources on the same queue
	outAcquire = render::GpuAliasAcquire();

#if B3D_BUILD_TYPE_DEVELOPMENT
	mPredecessors.Clear();
#endif

	const GpuQueueId firstQueue = mTimeline->GetQueue(firstSubmission);
	const u64 allocationEnd = offset + size;
	for(u32 segmentIndex = firstSegmentIndex; segmentIndex <= lastSegmentIndex; segmentIndex++)
	{
		const Segment& segment = heap.FreeSegments[segmentIndex];
		const u64 inheritedOffset = std::max(segment.Offset, offset);
		const u64 inheritedEnd = std::min(segment.GetEnd(), allocationEnd);
		mInheritedSegments.Add(Segment(inheritedOffset, inheritedEnd - inheritedOffset, segment.Tag));

		if(segment.Tag == kNoTag)
			continue;

		const Tag& tag = mTags[segment.Tag];
		for(u32 useIndex = tag.FirstUse; useIndex < tag.FirstUse + tag.UseCount; useIndex++)
		{
			const TagUse& use = mTagUses[useIndex];
			if(use.Queue.Id == firstQueue.Id)
				outAcquire.Source.Add(use.Access);
		}

#if B3D_BUILD_TYPE_DEVELOPMENT
		if(tag.Owner != nullptr && !mPredecessors.Contains(tag.Owner))
			mPredecessors.Add(tag.Owner);
#endif
	}

#if B3D_BUILD_TYPE_DEVELOPMENT
	outAcquire.Predecessors = TArrayView<IGpuResource* const>(mPredecessors.Data(), mPredecessors.Size());
#endif

	// Replace the spanned segments with the parts the allocation does not cover
	const Segment firstSegment = heap.FreeSegments[firstSegmentIndex];
	const Segment lastSegment = heap.FreeSegments[lastSegmentIndex];
	heap.FreeSegments.Erase(heap.FreeSegments.Begin() + firstSegmentIndex, heap.FreeSegments.Begin() + lastSegmentIndex + 1);

	u32 insertIndex = firstSegmentIndex;
	if(firstSegment.Offset < offset)
		heap.FreeSegments.Insert(heap.FreeSegments.Begin() + insertIndex++, Segment(firstSegment.Offset, offset - firstSegment.Offset, firstSegment.Tag));

	if(lastSegment.GetEnd() > allocationEnd)
		heap.FreeSegments.Insert(heap.FreeSegments.Begin() + insertIndex, Segment(allocationEnd, lastSegment.GetEnd() - allocationEnd, lastSegment.Tag));

	mUsedBytes += size;
	mPeakUsedBytes = std::max(mPeakUsedBytes, mUsedBytes);
	mRequestedBytes += size;

	outAllocation.Reset();
	outAllocation.Heap = heap.Handle;
	outAllocation.Offset = offset;
	outAllocation.Size = size;
	outAllocation.AllocatorData0 = heap.Slot;
	outAllocation.AllocatorData1 = allocationIndex;
}

GpuAliasingAllocator::Allocation& GpuAliasingAllocator::GetAllocation(const GpuAllocation& allocation)
{
	B3D_ASSERT(allocation.AllocatorData1 < mAllocations.Size());

	Allocation& liveAllocation = mAllocations[allocation.AllocatorData1];
	B3D_ASSERT(liveAllocation.HeapIndex != kInvalidIndex && mHeaps[liveAllocation.HeapIndex].Handle == allocation.Heap &&
		liveAllocation.Offset == allocation.Offset && "Allocation was not made in this scope, or was already released.");

	return liveAllocation;
}

void GpuAliasingAllocator::InsertFreeSegment(Heap& heap, const Segment& segment)
{
	u32 insertIndex = 0;
	while(insertIndex < heap.FreeSegments.Size() && heap.FreeSegments[insertIndex].Offset < segment.Offset)
		insertIndex++;

	const bool mergesWithPrevious = insertIndex > 0 && heap.FreeSegments[insertIndex - 1].GetEnd() == segment.Offset && heap.FreeSegments[insertIndex - 1].Tag == segment.Tag;
	const bool mergesWithNext = insertIndex < heap.FreeSegments.Size() && segment.GetEnd() == heap.FreeSegments[insertIndex].Offset && heap.FreeSegments[insertIndex].Tag == segment.Tag;

	if(mergesWithPrevious && mergesWithNext)
	{
		heap.FreeSegments[insertIndex - 1].Size += segment.Size + heap.FreeSegments[insertIndex].Size;
		heap.FreeSegments.Erase(heap.FreeSegments.Begin() + insertIndex);
	}
	else if(mergesWithPrevious)
		heap.FreeSegments[insertIndex - 1].Size += segment.Size;
	else if(mergesWithNext)
	{
		heap.FreeSegments[insertIndex].Offset = segment.Offset;
		heap.FreeSegments[insertIndex].Size += segment.Size;
	}
	else
		heap.FreeSegments.Insert(heap.FreeSegments.Begin() + insertIndex, segment);
}
