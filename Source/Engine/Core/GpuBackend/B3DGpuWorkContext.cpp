//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DGpuWorkContext.h"
#include "B3DGpuDevice.h"
#include "B3DGpuCommandBuffer.h"
#include "B3DGpuCommandBufferPoolRing.h"
#include "B3DGpuParameterSetPool.h"
#include "GpuBackend/Allocators/B3DGpuResource.h"

using namespace b3d;

namespace
{
	constexpr GpuQueueType kTransferQueueType = GQT_GRAPHICS;
	constexpr u32 kTransferQueueIndex = 0;

	TUnique<GpuParameterSetPool> CreateParameterSetPool(GpuDevice& device, bool isWorkerContext)
	{
		GpuParameterSetPoolCreateInformation createInformation;
		createInformation.Mode = GpuParameterSetPoolMode::Persistent;

		// Worker contexts are short-lived, single-operation contexts that allocate at most a handful of
		// parameter sets, so they start small. The backend pool grows on demand if exhausted.
		if (isWorkerContext)
		{
			createInformation.MaxSets = 512;
			createInformation.MaxSampledImages = 256;
			createInformation.MaxStorageImages = 256;
			createInformation.MaxSampledBuffers = 256;
			createInformation.MaxStorageBuffers = 256;
			createInformation.MaxUniformBuffers = 256;
			createInformation.MaxSamplers = 256;
			createInformation.MaxCombinedImageSamplers = 256;
			createInformation.MaxUniformBuffersDynamic = 128;
			createInformation.MaxStorageBuffersDynamic = 128;
		}

		return device.CreateParameterSetPool(createInformation);
	}
}

GpuWorkContext::GpuWorkContext(PrivatelyConstruct, GpuDevice& device)
	: mDevice(device), mTracker(nullptr), mOwnedTracker(B3DMakeUnique<GpuFenceCompletionTracker>(device.CreateTimelineFence()))
	, mParameterSetPool(CreateParameterSetPool(device, true))
{
	mTracker = mOwnedTracker.get();
}

GpuWorkContext::GpuWorkContext(PrivatelyConstruct, GpuDevice& device, IGpuCompletionTracker& tracker)
	: mDevice(device), mTracker(&tracker), mParameterSetPool(CreateParameterSetPool(device, false))
{
}

TShared<GpuWorkContext> GpuWorkContext::Create(GpuDevice& device)
{
	return B3DMakeShared<GpuWorkContext>(PrivatelyConstruct(), device);
}

TShared<GpuWorkContext> GpuWorkContext::Create(GpuDevice& device, IGpuCompletionTracker& tracker)
{
	return B3DMakeShared<GpuWorkContext>(PrivatelyConstruct(), device, tracker);
}

IGpuAllocator* GpuWorkContext::TryGetOrCreateTransientAllocator(u32 memoryType)
{
	TUnique<IGpuAllocator>& slot = mTransientAllocators[memoryType];
	if (slot == nullptr)
		slot = mDevice.CreateTransientAllocator(memoryType, *mTracker);

	return slot.get();
}

IGpuAllocator& GpuWorkContext::GetOrCreateTransientAllocator(u32 memoryType)
{
	IGpuAllocator* allocator = TryGetOrCreateTransientAllocator(memoryType);

	B3D_ASSERT(allocator != nullptr && "Backend does not support context transient allocation.");
	return *allocator;
}

IGpuWorkContextLocal* GpuWorkContext::GetLocal(const void* key) const
{
	const auto findIterator = mLocalObjects.find(key);
	return findIterator != mLocalObjects.end() ? findIterator->second.get() : nullptr;
}

IGpuWorkContextLocal& GpuWorkContext::AddLocal(const void* key, TUnique<IGpuWorkContextLocal> object)
{
	B3D_ASSERT(object != nullptr);

	TUnique<IGpuWorkContextLocal>& slot = mLocalObjects[key];
	B3D_ASSERT(slot == nullptr && "A context-local object is already registered under this key.");

	slot = std::move(object);
	return *slot;
}

TShared<render::GpuBuffer> GpuWorkContext::CreateTransientGpuBuffer(const GpuBufferCreateInformation& createInformation)
{
	// A buffer's memory type is a pure function of its create information, so its transient allocator
	// can be resolved once, up front, and carried by the buffer for life.
	const u32 memoryType = mDevice.PickBufferMemoryType(createInformation);
	IGpuAllocator* allocator = TryGetOrCreateTransientAllocator(memoryType);

	// Backends without context transient allocation fall back to a persistent allocation.
	if (allocator == nullptr)
		return mDevice.CreateGpuBuffer(createInformation, GpuObjectCreateFlag::None);

	return mDevice.CreateGpuBuffer(createInformation, *allocator, GpuObjectCreateFlag::None);
}


GpuWorkContext::~GpuWorkContext()
{
	// Contexts owning a fence tracker (workers) settle their own GPU work: wait for it and reclaim the
	// transient memory before tearing down. Contexts borrowing an external tracker are frame-driven;
	// their owner destroys them only once their GPU work is known complete (e.g. after a device idle at
	// shutdown), so no wait is performed (or possible) here.
	if (mOwnedTracker != nullptr)
		WaitAndReclaim();

	B3D_DEBUG_ONLY(AssertNoOutstandingTransientAllocations());

	// Context-local objects are normally destroyed by WaitAndReclaim() (after the GPU drain, before the
	// transient memory is reclaimed) - both the worker path above and the borrowed-tracker owner's explicit
	// drain go through it, so this is a no-op in the normal path. It remains as a defensive teardown for a
	// borrowed-tracker context destroyed without its owner having called WaitAndReclaim(); the pools below
	// (declared after mLocalObjects) are still alive, so any parameter sets the locals hold free cleanly.
	mLocalObjects.clear();

	if (!mTransferPoolRing)
		return;

	if (mTransferCommandBuffer != nullptr)
	{
		mTransferCommandBuffer->End();
		mTransferCommandBuffer = nullptr;
	}

	mTransferPoolRing->Destroy();
	mTransferPoolRing = nullptr;
}

const TShared<render::GpuCommandBuffer>& GpuWorkContext::GetTransferCommandBuffer()
{
	if (!mTransferPoolRing)
	{
		render::GpuCommandBufferPoolCreateInformation poolCreateInformation;
		poolCreateInformation.Thread = B3D_CURRENT_THREAD_ID;
		poolCreateInformation.Type = kTransferQueueType;
		poolCreateInformation.UsePoolReset = true;

		mTransferPoolRing = B3DMakeUnique<render::GpuCommandBufferPoolRing>(mDevice, poolCreateInformation);
	}

	if (mTransferCommandBuffer == nullptr)
	{
		render::GpuCommandBufferCreateInformation commandBufferCreateInformation;
		commandBufferCreateInformation.Name = "Transfer";

		mTransferCommandBuffer = mTransferPoolRing->GetCurrentPool().FindOrCreate(commandBufferCreateInformation);
	}

	return mTransferCommandBuffer;
}

void GpuWorkContext::SubmitCommandBuffer(const GpuSubmissionInformation& information, u32 queueIndex)
{
	// NOTE: Transfer and work are submitted to the same queue (graphics 0), so same-queue submission
	//		 order preserves the transfer-before-work dependency with no explicit semaphore. If the work
	//		 command buffer ever targets a different queue, the caller must add an explicit sync mask for
	//		 the transfer queue - same-queue ordering no longer guarantees transfer visibility.

	// Flush this context's pending transfers first (non-blocking) so they are visible to the work below.
	SubmitTransferCommandBuffers(false);

	if (!B3D_ENSURE(information.CommandBuffer))
		return;

	const GpuQueueType queueType = information.CommandBuffer->GetQueueType();
	const u32 queueCount = mDevice.GetQueueCount(queueType);
	if (!B3D_ENSURE(queueIndex < queueCount))
		return;

	const TShared<GpuQueue> queue = mDevice.GetQueue(queueType, queueIndex);
	if (!B3D_ENSURE(queue))
		return;

	if (mOwnedTracker == nullptr)
	{
		queue->SubmitCommandBuffer(information);
		return;
	}

	// Contexts owning a fence tracker tag every submission with their fence, so the tracker observes GPU
	// progress: transient pages retire against these markers and WaitAndReclaim() blocks on the last one.
	GpuSubmissionInformation taggedInformation = information;
	taggedInformation.SignalFences.Add(mOwnedTracker->NotifyWillSubmit());

	queue->SubmitCommandBuffer(taggedInformation);
}

void GpuWorkContext::SubmitCommandBuffer(const TShared<render::GpuCommandBuffer>& commandBuffer, GpuQueueMask syncMask, u32 queueIndex)
{
	GpuSubmissionInformation information;
	information.CommandBuffer = commandBuffer;
	information.SyncMask = syncMask;

	SubmitCommandBuffer(information, queueIndex);
}

void GpuWorkContext::SubmitTransferCommandBuffers(bool wait)
{
	TShared<render::GpuCommandBuffer> commandBufferToSubmit = mTransferCommandBuffer;
	mTransferCommandBuffer = nullptr;

	if (commandBufferToSubmit == nullptr && !wait)
		return;

	TShared<GpuQueue> queue = mDevice.GetQueue(kTransferQueueType, kTransferQueueIndex);
	if (queue == nullptr)
		return;

	if (commandBufferToSubmit != nullptr)
	{
		commandBufferToSubmit->End();

		GpuSubmissionInformation submissionInfo;
		submissionInfo.CommandBuffer = commandBufferToSubmit;
		submissionInfo.SyncMask = GpuQueueMask::kAll;

		// See SubmitCommandBuffer() - fence-tracked contexts tag every submission with their fence.
		if (mOwnedTracker != nullptr)
			submissionInfo.SignalFences.Add(mOwnedTracker->NotifyWillSubmit());

		queue->SubmitCommandBuffer(submissionInfo);
	}

	if (wait)
		queue->WaitUntilIdle();
}

void GpuWorkContext::AdvanceFrame()
{
	if (mTransferPoolRing)
		mTransferPoolRing->AdvanceFrame();

	mTransferCommandBuffer = nullptr;

	// Reclaim transient memory at the frame boundary: retire each linear allocator's open page, then
	// drain any pages whose completion marker has signaled. No-op for backends with no transient
	// allocators (the map stays empty).
	for (auto& entry : mTransientAllocators)
	{
		if (entry.second == nullptr)
			continue;

		entry.second->FreeAll();
		entry.second->ReclaimUnused(false);
	}
}

void GpuWorkContext::WaitAndReclaim()
{
	// Flush pending transfers so everything this context recorded is actually submitted (and, for
	// fence-tracked contexts, tagged with the fence value waited on below).
	SubmitTransferCommandBuffers(false);

	const bool hasSubmittedWork = mOwnedTracker != nullptr && mOwnedTracker->GetLastSubmittedMarker() != 0;

	// Drain the GPU before tearing anything down, so destroying the context-local objects below releases
	// their GPU resources safely. A context that never submitted work and never allocated transient memory
	// has nothing to drain.
	if (hasSubmittedWork || !mTransientAllocators.empty())
	{
		// Block (yieldably) until the GPU drains this context's last submission.
		if (mOwnedTracker != nullptr)
			mOwnedTracker->WaitUntilComplete();

		// A signaled fence does not mean the work's completion callbacks have run - those are delivered
		// through the command pools' message queues once the submit thread processes the finished
		// submissions. The queue-level wait forces that processing and blocks until every completion
		// callback has been consumed on its pool's owning thread, so callbacks holding transient buffers
		// have released them by the time the pages are drained below.
		// TODO: This waits on the whole queue, serializing concurrent contexts at teardown. Replace it with
		//		 a targeted completion-refresh + pump of this context's pools once contexts run concurrently.
		const TShared<GpuQueue> queue = mDevice.GetQueue(kTransferQueueType, kTransferQueueIndex);
		if (queue != nullptr)
			queue->WaitUntilIdle();
	}

	// Destroy context-local objects now that the GPU is idle. They may hold context-allocated resources -
	// parameter sets, and the transient buffers those parameter sets reference - which must be released
	// before the transient allocations are asserted clean and reclaimed below. Borrowed-tracker contexts
	// rely on their owner having drained the GPU before calling this (see the destructor contract).
	mLocalObjects.clear();

	// With the locals gone, a context with no submitted work and no transient memory has nothing left to do.
	if (!hasSubmittedWork && mTransientAllocators.empty())
		return;

	B3D_DEBUG_ONLY(AssertNoOutstandingTransientAllocations());

	// Retire the open pages and force-drain everything. Safe because the GPU work consuming the pages
	// finished above; the drained pages return to the device's shared page pool for reuse.
	for (auto& entry : mTransientAllocators)
	{
		if (entry.second == nullptr)
			continue;

		entry.second->FreeAll();
		entry.second->ReclaimUnused(true);
	}
}

#if B3D_DEBUG
void GpuWorkContext::AssertNoOutstandingTransientAllocations() const
{
	for (const auto& entry : mTransientAllocators)
	{
		if (entry.second == nullptr)
			continue;

		B3D_ASSERT(entry.second->GetOutstandingAllocationCount() == 0 &&
			"A transient allocation outlived its work context. Release all transient buffers before the context is reclaimed or destroyed.");
	}
}
#endif
