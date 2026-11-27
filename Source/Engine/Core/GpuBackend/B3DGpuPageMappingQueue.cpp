//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "GpuBackend/B3DGpuPageMappingQueue.h"
#include "GpuBackend/B3DGpuDevice.h"
#include "GpuBackend/B3DGpuPageTable.h"
#include "GpuBackend/B3DGpuSubmitThread.h"
#include "GpuBackend/Allocators/B3DGpuResource.h"

using namespace b3d;

GpuPageMappingQueue::GpuPageMappingQueue(GpuDevice& device, const IGpuCompletionTracker& frameCompletionTracker)
	: mDevice(device), mFrameCompletionTracker(frameCompletionTracker)
{ }

GpuPageMappingQueue::~GpuPageMappingQueue()
{
	B3D_ASSERT(mQueuedBatch.Mappings.Empty() && "The backend must clear the page mapping queue before its resource manager.");
	B3D_ASSERT(mTablesWithPendingRemovals.Empty() && "Page tables must be destroyed before their device.");
}

void GpuPageMappingQueue::ExecuteOrQueueAdds(TArrayView<const GpuPageMapping> mappings, GpuPageTableFlags flags)
{
	if(mappings.IsEmpty())
		return;

	Lock lock(mMutex);

	// A queued removal that hasn't executed may unmap the same pages these mappings map, since its table treats the pages as unmapped
	// once the removal is queued. Executing these mappings first would let the removal undo them.
	if(mUnexecutedRemovalCount.load(std::memory_order_acquire) == 0 && mDevice.TryExecutePageMappingsImmediately(mappings))
		return;

	AddToQueuedBatch(mappings, flags);
}

void GpuPageMappingQueue::QueueRemovals(TArrayView<const GpuPageMapping> mappings, GpuPageTableFlags flags)
{
	if(mappings.IsEmpty())
		return;

	Lock lock(mMutex);

	mQueuedBatch.RemovalCount += (u32)mappings.size();
	mUnexecutedRemovalCount.fetch_add((u32)mappings.size(), std::memory_order_acq_rel);
	AddToQueuedBatch(mappings, flags);
}

void GpuPageMappingQueue::AddToQueuedBatch(TArrayView<const GpuPageMapping> mappings, GpuPageTableFlags flags)
{
	for(const GpuPageMapping& mapping : mappings)
	{
		// The resource must exist until the batch executes, even if its owner destroys it right away
		mapping.Resource->NotifyPinned();
		mQueuedBatch.Mappings.Add(mapping);
	}

	if(flags.IsSet(GpuPageTableFlag::Bindless))
		mQueuedBatch.ChangesBindlessResources = true;
}

void GpuPageMappingQueue::Flush()
{
	// Handed over under the lock, so batches reach the submit thread in the order they were queued
	Lock lock(mMutex);
	if(mQueuedBatch.Mappings.Empty())
		return;

	GpuPageMappingBatch batch = std::move(mQueuedBatch);
	mQueuedBatch = GpuPageMappingBatch();

	mDevice.GetSubmitThread().QueuePageMappings(std::move(batch));
}

bool GpuPageMappingQueue::HasQueuedMappings() const
{
	Lock lock(mMutex);
	return !mQueuedBatch.Mappings.Empty();
}

void GpuPageMappingQueue::RegisterPendingRemovals(GpuPageTable& table)
{
	Lock lock(mTableMutex);
	if(!mTablesWithPendingRemovals.Contains(&table))
		mTablesWithPendingRemovals.Add(&table);
}

void GpuPageMappingQueue::UnregisterTable(GpuPageTable& table)
{
	Lock lock(mTableMutex);
	for(u32 tableIndex = 0; tableIndex < (u32)mTablesWithPendingRemovals.Size(); tableIndex++)
	{
		if(mTablesWithPendingRemovals[tableIndex] == &table)
		{
			mTablesWithPendingRemovals.Remove(tableIndex);
			return;
		}
	}
}

void GpuPageMappingQueue::QueueCompletedRemovals()
{
	// Held under the whole loop, so a table can't be destroyed while it releases its mappings
	Lock lock(mTableMutex);

	for(u32 tableIndex = 0; tableIndex < (u32)mTablesWithPendingRemovals.Size();)
	{
		if(mTablesWithPendingRemovals[tableIndex]->QueueCompletedRemovals(mFrameCompletionTracker))
			tableIndex++;
		else
			mTablesWithPendingRemovals.Remove(tableIndex);
	}
}

void GpuPageMappingQueue::NotifyBatchExecuted(const GpuPageMappingBatch& batch)
{
	// Not under the lock, since the submit thread may run a batch inline, on a thread that holds it in Flush()
	mUnexecutedRemovalCount.fetch_sub(batch.RemovalCount, std::memory_order_acq_rel);

	for(const GpuPageMapping& mapping : batch.Mappings)
		mapping.Resource->NotifyUnpinned();
}

void GpuPageMappingQueue::Clear()
{
	GpuPageMappingBatch batch;
	{
		Lock lock(mMutex);
		batch = std::move(mQueuedBatch);
		mQueuedBatch = GpuPageMappingBatch();
	}

	// Unpinning may destroy the resources, so it runs without the lock held
	NotifyBatchExecuted(batch);
}
