//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "GpuBackend/B3DGpuPageTable.h"
#include "GpuBackend/B3DGpuDevice.h"
#include "GpuBackend/B3DGpuPageMappingQueue.h"
#include "GpuBackend/Allocators/B3DGpuPagePool.h"

using namespace b3d;

GpuPageTable::GpuPageTable(GpuDevice& device, IGpuResource& resource, u32 pageCount, IGpuPagePool& pagePool, GpuPageTableFlags flags)
	: mDevice(device), mResource(resource), mPagePool(pagePool), mPageCount(pageCount), mFlags(flags)
{ }

GpuPageTable::GpuPageTable(GpuDevice& device, IGpuResource& resource, const GpuVirtualTextureLayout& textureLayout, IGpuPagePool& pagePool,
	GpuPageTableFlags flags)
	: mDevice(device), mResource(resource), mPagePool(pagePool), mPageCount(textureLayout.PageCount), mFlags(flags), mTextureLayout(textureLayout)
{ }

GpuPageTable::~GpuPageTable()
{
	// Before the lock, which QueueCompletedRemovals() takes after the queue's
	mDevice.GetPageMappingQueue().UnregisterTable(*this);

	// The resource is destroyed with the table, so its mappings are never changed again. Work submitted before may still use the memory.
	Lock lock(mMutex);
	for(const Run& run : mRuns)
	{
		if(run.Memory.IsOwned)
			mPagePool.Free(GpuPageAllocation(run.Memory.Heap, GpuPageRange(run.Memory.FirstHeapPageIndex, run.Pages.PageCount)));

		if(run.HasPendingRemoval && run.OldMemory.IsOwned)
			mPagePool.Free(GpuPageAllocation(run.OldMemory.Heap, GpuPageRange(run.OldMemory.FirstHeapPageIndex, run.Pages.PageCount)));
	}
}

u32 GpuPageTable::GetCommittedPageCount() const
{
	Lock lock(mMutex);
	return mCommittedPageCount;
}

bool GpuPageTable::IsCommitted(const GpuPageRange& range) const
{
	Lock lock(mMutex);

	u32 nextPageIndex = range.FirstPageIndex;
	for(u32 runIndex = FindRunIndex(range.FirstPageIndex); runIndex < (u32)mRuns.Size() && nextPageIndex < range.GetEndPageIndex(); runIndex++)
	{
		const Run& run = mRuns[runIndex];
		if(run.Pages.FirstPageIndex > nextPageIndex || run.Memory.Heap == nullptr)
			return false;

		nextPageIndex = run.Pages.GetEndPageIndex();
	}

	return nextPageIndex >= range.GetEndPageIndex();
}

bool GpuPageTable::TryCommit(const GpuPageRange& range)
{
	if(!B3D_ENSURE_LOG(range.GetEndPageIndex() <= mPageCount, "Cannot commit pages {0} to {1} of a page table with {2} pages.", range.FirstPageIndex,
		range.GetEndPageIndex(), mPageCount))
	{
		return false;
	}

	if(range.PageCount == 0)
		return true;

	Lock lock(mMutex);
	FrameAllocatorScope frameScope;

	SplitRunAt(range.FirstPageIndex);
	SplitRunAt(range.GetEndPageIndex());

	const u32 firstRunIndex = FindRunIndex(range.FirstPageIndex);
	const u32 endRunIndex = FindRunIndex(range.GetEndPageIndex());

	// Pages without a run have no memory on the GPU either, so they get new memory
	u32 gapPageCount = range.PageCount;
	bool hasPendingExternalRemoval = false;
	for(u32 runIndex = firstRunIndex; runIndex < endRunIndex; runIndex++)
	{
		const Run& run = mRuns[runIndex];
		gapPageCount -= run.Pages.PageCount;
		hasPendingExternalRemoval |= run.HasPendingRemoval && !run.OldMemory.IsOwned;
	}

	// The GPU still maps memory the table doesn't own to those pages, and pool memory would replace it before the work that uses it completes
	if(!B3D_ENSURE_LOG(!hasPendingExternalRemoval, "Cannot commit pages {0} to {1}, since memory from a GpuPageAllocation was decommitted from "
		"some of them in a frame the GPU hasn't finished.", range.FirstPageIndex, range.GetEndPageIndex()))
	{
		// Undoes the splits
		MergeRuns(firstRunIndex, endRunIndex);
		return false;
	}

	TArray<GpuPageAllocation> allocations;
	if(gapPageCount > 0 && !mPagePool.TryAllocate(gapPageCount, allocations))
	{
		// Undoes the splits
		MergeRuns(firstRunIndex, endRunIndex);
		return false;
	}

	TInlineArray<Run, 8> runs;
	FrameVector<GpuPageMapping> adds;
	u32 allocationIndex = 0;
	u32 allocationOffset = 0;
	const auto fnFillGap = [this, &allocations, &allocationIndex, &allocationOffset, &runs, &adds](u32 firstPageIndex, u32 endPageIndex)
	{
		while(firstPageIndex < endPageIndex)
		{
			const GpuPageAllocation& allocation = allocations[allocationIndex];
			const u32 pageCount = std::min(allocation.Pages.PageCount - allocationOffset, endPageIndex - firstPageIndex);

			Run run;
			run.Pages = GpuPageRange(firstPageIndex, pageCount);
			run.Memory = RunMemory(allocation.Heap, allocation.Pages.FirstPageIndex + allocationOffset, true);

			adds.push_back(GpuPageMapping(&mResource, run.Pages, run.Memory.Heap, run.Memory.FirstHeapPageIndex));
			runs.Add(run);
			mCommittedPageCount += pageCount;

			firstPageIndex += pageCount;
			allocationOffset += pageCount;
			if(allocationOffset == allocation.Pages.PageCount)
			{
				allocationIndex++;
				allocationOffset = 0;
			}
		}
	};

	u32 nextPageIndex = range.FirstPageIndex;
	for(u32 runIndex = firstRunIndex; runIndex < endRunIndex; runIndex++)
	{
		Run run = mRuns[runIndex];
		fnFillGap(nextPageIndex, run.Pages.FirstPageIndex);
		nextPageIndex = run.Pages.GetEndPageIndex();

		// The GPU still maps the old memory of decommitted pages whose removal is pending, so they get it back, contents included
		if(run.Memory.Heap == nullptr)
		{
			B3D_ASSERT(run.HasPendingRemoval);

			run.Memory = run.OldMemory;
			run.OldMemory = RunMemory();
			run.HasPendingRemoval = false;
			
			mCommittedPageCount += run.Pages.PageCount;
			mPendingRemovalPageCount -= run.Pages.PageCount;
		}

		runs.Add(run);
	}

	fnFillGap(nextPageIndex, range.GetEndPageIndex());
	ReplaceRuns(firstRunIndex, endRunIndex, TArrayView<const Run>(runs.Data(), runs.Size()));

	// Queued under the lock, so a later change of the same pages is queued after
	mDevice.GetPageMappingQueue().ExecuteOrQueueAdds(TArrayView<const GpuPageMapping>(adds.data(), adds.size()), mFlags);
	return true;
}

bool GpuPageTable::TryCommit(u32 firstPageIndex, const GpuPageAllocation& memory)
{
	const GpuPageRange range(firstPageIndex, memory.Pages.PageCount);
	if(!B3D_ENSURE_LOG(range.GetEndPageIndex() <= mPageCount, "Cannot commit pages {0} to {1} of a page table with {2} pages.", range.FirstPageIndex,
		range.GetEndPageIndex(), mPageCount))
	{
		return false;
	}

	if(!B3D_ENSURE_LOG(memory.Heap != nullptr, "Cannot commit memory without a heap. Use Decommit() to remove the memory of pages."))
		return false;

	if(range.PageCount == 0)
		return true;

	Lock lock(mMutex);

	// The GPU may still map other memory to pages with a run, and replacing it would change the memory under work submitted before
	if(HasRunsIn(range))
		return false;

	Run run;
	run.Pages = range;
	run.Memory = RunMemory(memory.Heap, memory.Pages.FirstPageIndex, false);

	const u32 runIndex = FindRunIndex(range.FirstPageIndex);
	ReplaceRuns(runIndex, runIndex, TArrayView<const Run>(&run, 1));
	mCommittedPageCount += range.PageCount;

	// Queued under the lock, so a later change of the same pages is queued after
	const GpuPageMapping add(&mResource, range, memory.Heap, memory.Pages.FirstPageIndex);
	mDevice.GetPageMappingQueue().ExecuteOrQueueAdds(TArrayView<const GpuPageMapping>(&add, 1), mFlags);
	return true;
}

void GpuPageTable::Decommit(const GpuPageRange& range)
{
	if(!B3D_ENSURE_LOG(range.GetEndPageIndex() <= mPageCount, "Cannot decommit pages {0} to {1} of a page table with {2} pages.", range.FirstPageIndex,
		range.GetEndPageIndex(), mPageCount))
	{
		return;
	}

	bool hasPendingRemovals = false;
	{
		Lock lock(mMutex);

		SplitRunAt(range.FirstPageIndex);
		SplitRunAt(range.GetEndPageIndex());

		const u32 firstRunIndex = FindRunIndex(range.FirstPageIndex);
		const u32 endRunIndex = FindRunIndex(range.GetEndPageIndex());
		const u64 currentFrame = mDevice.GetFrameIndex();
		for(u32 runIndex = firstRunIndex; runIndex < endRunIndex; runIndex++)
		{
			Run& run = mRuns[runIndex];
			if(run.HasPendingRemoval)
				continue;

			// Work submitted before may still use the memory the GPU maps, so the removal is pending until the current frame completes
			run.HasPendingRemoval = true;
			run.OldMemory = run.Memory;
			run.RemovalFrame = currentFrame;
			run.Memory = RunMemory();
			mCommittedPageCount -= run.Pages.PageCount;
			mPendingRemovalPageCount += run.Pages.PageCount;
		}

		MergeRuns(firstRunIndex, endRunIndex);
		hasPendingRemovals = HasPendingRemovals();
	}

	// Without the table locked, since the queue locks its tables after its own lock
	if(hasPendingRemovals)
		mDevice.GetPageMappingQueue().RegisterPendingRemovals(*this);
}

bool GpuPageTable::QueueCompletedRemovals(const IGpuCompletionTracker& frameCompletionTracker)
{
	Lock lock(mMutex);
	FrameAllocatorScope frameScope;

	FrameVector<GpuPageMapping> removals;
	TInlineArray<GpuPageAllocation, 4> releasedMemory;
	for(Run& run : mRuns)
	{
		if(!run.HasPendingRemoval || !frameCompletionTracker.IsMarkerComplete(run.RemovalFrame))
			continue;

		removals.push_back(GpuPageMapping(&mResource, run.Pages, nullptr, 0));
		if(run.OldMemory.IsOwned)
			releasedMemory.Add(GpuPageAllocation(run.OldMemory.Heap, GpuPageRange(run.OldMemory.FirstHeapPageIndex, run.Pages.PageCount)));

		run.HasPendingRemoval = false;
		run.OldMemory = RunMemory();

		mPendingRemovalPageCount -= run.Pages.PageCount;
	}

	if(!removals.empty())
		MergeRuns(0, (u32)mRuns.Size());

	// The removals are queued before the memory goes back to the pool, so whoever gets the memory next maps it after they execute
	mDevice.GetPageMappingQueue().QueueRemovals(TArrayView<const GpuPageMapping>(removals.data(), removals.size()), mFlags);
	for(const GpuPageAllocation& allocation : releasedMemory)
		mPagePool.FreeAndReclaim(allocation);

	return HasPendingRemovals();
}

u32 GpuPageTable::FindRunIndex(u32 pageIndex) const
{
	const auto iterFound = std::partition_point(mRuns.Begin(), mRuns.End(), [pageIndex](const Run& run) { return run.Pages.GetEndPageIndex() <= pageIndex; });
	return (u32)(iterFound - mRuns.Begin());
}

void GpuPageTable::SplitRunAt(u32 pageIndex)
{
	const u32 runIndex = FindRunIndex(pageIndex);
	if(runIndex == (u32)mRuns.Size() || mRuns[runIndex].Pages.FirstPageIndex >= pageIndex)
		return;

	const Run run = mRuns[runIndex];
	const u32 firstPageCount = pageIndex - run.Pages.FirstPageIndex;

	Run second = run;
	second.Pages = GpuPageRange(pageIndex, run.Pages.PageCount - firstPageCount);
	second.Memory = run.Memory.Offset(firstPageCount);
	second.OldMemory = run.OldMemory.Offset(firstPageCount);

	mRuns[runIndex].Pages.PageCount = firstPageCount;
	mRuns.Insert(mRuns.Begin() + runIndex + 1, second);
}

void GpuPageTable::ReplaceRuns(u32 firstRunIndex, u32 endRunIndex, TArrayView<const Run> runs)
{
	const u32 runCount = (u32)runs.Size();
	const u32 replacedRunCount = endRunIndex - firstRunIndex;
	const u32 overwrittenRunCount = std::min(runCount, replacedRunCount);
	for(u32 runIndex = 0; runIndex < overwrittenRunCount; runIndex++)
		mRuns[firstRunIndex + runIndex] = runs[runIndex];

	if(runCount > replacedRunCount)
		mRuns.Insert(mRuns.Begin() + endRunIndex, runs.Begin() + overwrittenRunCount, runs.End());
	else
		mRuns.Erase(mRuns.Begin() + firstRunIndex + runCount, mRuns.Begin() + endRunIndex);

	MergeRuns(firstRunIndex, firstRunIndex + runCount);
}

void GpuPageTable::MergeRuns(u32 firstRunIndex, u32 endRunIndex)
{
	// Memory continues other memory if both are none, or it follows it in the same heap
	const auto fnContinues = [](const RunMemory& first, u32 firstPageCount, const RunMemory& second)
	{
		if(first.Heap == nullptr || second.Heap == nullptr)
			return first.Heap == second.Heap;

		return first.Heap == second.Heap && first.IsOwned == second.IsOwned && first.FirstHeapPageIndex + firstPageCount == second.FirstHeapPageIndex;
	};

	const auto fnMerges = [&fnContinues](const Run& previous, const Run& run)
	{
		if(previous.Pages.GetEndPageIndex() != run.Pages.FirstPageIndex || previous.HasPendingRemoval != run.HasPendingRemoval ||
			!fnContinues(previous.Memory, previous.Pages.PageCount, run.Memory))
		{
			return false;
		}

		return !run.HasPendingRemoval || (previous.RemovalFrame == run.RemovalFrame && fnContinues(previous.OldMemory, previous.Pages.PageCount, run.OldMemory));
	};

	firstRunIndex = firstRunIndex > 0 ? firstRunIndex - 1 : 0;
	endRunIndex = std::min(endRunIndex + 1, (u32)mRuns.Size());

	u32 keptEndRunIndex = firstRunIndex;
	for(u32 runIndex = firstRunIndex; runIndex < endRunIndex; runIndex++)
	{
		const Run run = mRuns[runIndex];
		if(!run.HasPendingRemoval && run.Memory.Heap == nullptr)
			continue;

		if(keptEndRunIndex > firstRunIndex && fnMerges(mRuns[keptEndRunIndex - 1], run))
			mRuns[keptEndRunIndex - 1].Pages.PageCount += run.Pages.PageCount;
		else
			mRuns[keptEndRunIndex++] = run;
	}

	mRuns.Erase(mRuns.Begin() + keptEndRunIndex, mRuns.Begin() + endRunIndex);
}

bool GpuPageTable::HasRunsIn(const GpuPageRange& range) const
{
	const u32 runIndex = FindRunIndex(range.FirstPageIndex);
	return range.PageCount > 0 && runIndex < (u32)mRuns.Size() && mRuns[runIndex].Pages.FirstPageIndex < range.GetEndPageIndex();
}
