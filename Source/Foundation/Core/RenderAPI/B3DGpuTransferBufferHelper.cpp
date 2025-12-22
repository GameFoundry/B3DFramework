//************************************ B3D Framework - Copyright 2023 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DGpuTransferBufferHelper.h"
#include "B3DGpuCommandBuffer.h"
#include "B3DGpuDevice.h"

using namespace b3d;

namespace
{
	thread_local GpuTransferBufferHelper::ThreadData* gPerThreadData = nullptr;
	thread_local GpuTransferBufferHelper* gPerThreadActiveHelper = nullptr;
}

GpuTransferBufferHelper::GpuTransferBufferHelper(GpuDevice& device)
	: mGpuDevice(device)
{
}

GpuTransferBufferHelper::~GpuTransferBufferHelper()
{
	WaitGroup waitGroup;

	Lock lock(mRegistryMutex);
	for(auto& threadData : mThreadRegistry)
	{
		for(auto& queueEntry : threadData->QueueData)
		{
			QueueData& queueData = queueEntry.second;
			if(!queueData.PoolRing)
				continue;

			// Get scheduler thread from the first pool in the ring (all pools have the same owning thread)
			render::GpuCommandBufferPool& firstPool = queueData.PoolRing->GetCurrentPool();

			waitGroup.Increment();

			// It's important we queue the destroy on the thread the command buffer pool was created on, as command buffers are bound to a single thread. We don't use the command buffer pool message queue directly
			// as the destroy operation may wait on the queue to complete, which would result in a deadlock.
			const SPtr<SchedulerThread> ownerSchedulerThread = firstPool.GetMessageQueue().GetSchedulerThread();
			if(B3D_ENSURE(ownerSchedulerThread))
			{
				ownerSchedulerThread->Post(SchedulerTask([&waitGroup, poolRing = queueData.PoolRing.get()]
				{
					poolRing->Destroy();
					waitGroup.NotifyDone();
				}, "Destroy GpuCommandBufferPoolRing"));
			}
		}
	}

	lock.unlock();
	waitGroup.Wait();
}

GpuTransferBufferHelper::ThreadData* GpuTransferBufferHelper::GetCurrentThreadData()
{
	// Fast path: check if TLS is already set for this helper
	if(gPerThreadActiveHelper == this && gPerThreadData != nullptr)
		return gPerThreadData;

	return nullptr;
}

GpuTransferBufferHelper::ThreadData* GpuTransferBufferHelper::RegisterCurrentThreadIfNeeded()
{
	// Fast path: already registered with this helper
	if(gPerThreadActiveHelper == this && gPerThreadData != nullptr)
		return gPerThreadData;

	// Slow path: need to register
	Lock lock(mRegistryMutex);

	// Check if this thread is already in the registry (possible if multiple helpers per thread in multi-device case)
	ThreadId currentThreadId = B3D_CURRENT_THREAD_ID;
	for(auto& threadData : mThreadRegistry)
	{
		if(threadData->OwnerThread == currentThreadId)
		{
			// Update TLS
			gPerThreadData = threadData.get();
			gPerThreadActiveHelper = this;

			return gPerThreadData;
		}
	}

	// Create new thread data
	UPtr<ThreadData> newThreadData = B3DMakeUnique<ThreadData>();
	newThreadData->OwnerThread = currentThreadId;

	ThreadData* threadDataPtr = newThreadData.get();
	mThreadRegistry.push_back(std::move(newThreadData));

	// Update TLS
	gPerThreadData = threadDataPtr;
	gPerThreadActiveHelper = this;

	return threadDataPtr;
}

SPtr<render::GpuCommandBuffer> GpuTransferBufferHelper::GetOrCreateTransferCommandBuffer(GpuQueue& queue)
{
	ThreadData* threadData = RegisterCurrentThreadIfNeeded();

	const u32 queueId = queue.GetId().Id;
	QueueData& queueData = threadData->QueueData[queueId];

	// Initialize pool ring if needed
	if(!queueData.PoolRing)
	{
		render::GpuCommandBufferPoolCreateInformation poolCreateInformation;
		poolCreateInformation.Thread = B3D_CURRENT_THREAD_ID;
		poolCreateInformation.Type = queue.GetType();
		poolCreateInformation.UsePoolReset = true;

		queueData.PoolRing = B3DMakeUnique<render::GpuCommandBufferPoolRing>(mGpuDevice, poolCreateInformation);
	}

	// Create command buffer if needed
	if(queueData.CurrentCommandBuffer == nullptr)
	{
		render::GpuCommandBufferCreateInformation commandBufferCreateInformation;
		commandBufferCreateInformation.Name = "Transfer";

		queueData.CurrentCommandBuffer = queueData.PoolRing->GetCurrentPool().FindOrCreate(commandBufferCreateInformation);
	}

	return queueData.CurrentCommandBuffer;
}

void GpuTransferBufferHelper::SubmitTransferCommandBuffer(GpuQueue& queue, bool wait)
{
	ThreadData* threadData = GetCurrentThreadData();
	if(threadData == nullptr)
		return;

	u32 queueId = queue.GetId().Id;
	auto found = threadData->QueueData.find(queueId);
	if(found == threadData->QueueData.end())
		return;

	QueueData& queueData = found->second;
	SPtr<render::GpuCommandBuffer> commandBufferToSubmit = queueData.CurrentCommandBuffer;
	queueData.CurrentCommandBuffer = nullptr;

	if(commandBufferToSubmit != nullptr)
	{
		commandBufferToSubmit->End();
		queue.SubmitCommandBuffer(commandBufferToSubmit, 0xFFFFFFFF);
	}

	if(wait)
		queue.WaitUntilIdle();
}

void GpuTransferBufferHelper::SubmitAllTransferCommandBuffers()
{
	ThreadData* threadData = GetCurrentThreadData();
	if(threadData == nullptr)
		return;

	for(auto& queueEntry : threadData->QueueData)
	{
		QueueData& queueData = queueEntry.second;
		SPtr<render::GpuCommandBuffer> commandBufferToSubmit = queueData.CurrentCommandBuffer;
		queueData.CurrentCommandBuffer = nullptr;

		if(commandBufferToSubmit != nullptr)
		{
			commandBufferToSubmit->End();

			// Get the queue for this queue ID
			const GpuQueueId queueId(queueEntry.first);
			SPtr<GpuQueue> queue = mGpuDevice.GetQueue(queueId.GetType(), queueId.GetIndex());
			if(queue)
				queue->SubmitCommandBuffer(commandBufferToSubmit, 0xFFFFFFFF);
		}
	}
}

void GpuTransferBufferHelper::EndFrame()
{
	Lock lock(mRegistryMutex);

	for(auto& threadData : mThreadRegistry)
	{
		for(auto& queueEntry : threadData->QueueData)
		{
			QueueData& queueData = queueEntry.second;
			if(!queueData.PoolRing)
				continue;

			queueData.PoolRing->AdvanceFrame();
			queueData.CurrentCommandBuffer = nullptr;
		}
	}
}
