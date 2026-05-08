//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
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

GpuTransferBufferHelper::GpuTransferBufferHelper(GpuDevice& device, GpuQueueId targetQueue)
	: mGpuDevice(device), mTargetQueueType(targetQueue.GetType()), mTargetQueueIndex(targetQueue.GetIndex())
{
}

GpuTransferBufferHelper::~GpuTransferBufferHelper()
{
	WaitGroup waitGroup;

	Lock lock(mRegistryMutex);
	for(auto& threadData : mThreadRegistry)
	{
		if(!threadData->PoolRing)
			continue;

		// Get scheduler thread from the first pool in the ring (all pools have the same owning thread)
		render::GpuCommandBufferPool& firstPool = threadData->PoolRing->GetCurrentPool();

		waitGroup.Increment();

		// It's important we queue the destroy on the thread the command buffer pool was created on, as command buffers are bound to a single thread. We don't use the command buffer pool message queue directly
		// as the destroy operation may wait on the queue to complete, which would result in a deadlock.
		const TShared<SchedulerThread> ownerSchedulerThread = firstPool.GetMessageQueue().GetSchedulerThread();
		if(B3D_ENSURE(ownerSchedulerThread))
		{
			ownerSchedulerThread->Post(SchedulerTask([&waitGroup, poolRing = threadData->PoolRing.get(), currentCommandBuffer = threadData->CurrentCommandBuffer]() mutable
			{
				if(currentCommandBuffer != nullptr)
				{
					currentCommandBuffer->End();
					currentCommandBuffer = nullptr;
				}

				poolRing->Destroy();
				waitGroup.NotifyDone();
			}, "Destroy GpuCommandBufferPoolRing"));
		}

		threadData->CurrentCommandBuffer = nullptr;
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

const TShared<render::GpuCommandBuffer>& GpuTransferBufferHelper::GetOrCreateTransferCommandBuffer()
{
	ThreadData* threadData = RegisterCurrentThreadIfNeeded();

	// Initialize pool ring if needed
	if(!threadData->PoolRing)
	{
		render::GpuCommandBufferPoolCreateInformation poolCreateInformation;
		poolCreateInformation.Thread = B3D_CURRENT_THREAD_ID;
		poolCreateInformation.Type = mTargetQueueType;
		poolCreateInformation.UsePoolReset = true;

		threadData->PoolRing = B3DMakeUnique<render::GpuCommandBufferPoolRing>(mGpuDevice, poolCreateInformation);
	}

	// Create command buffer if needed
	if(threadData->CurrentCommandBuffer == nullptr)
	{
		render::GpuCommandBufferCreateInformation commandBufferCreateInformation;
		commandBufferCreateInformation.Name = "Transfer";

		threadData->CurrentCommandBuffer = threadData->PoolRing->GetCurrentPool().FindOrCreate(commandBufferCreateInformation);
	}

	return threadData->CurrentCommandBuffer;
}

void GpuTransferBufferHelper::SubmitTransferCommandBuffer(bool wait)
{
	ThreadData* threadData = GetCurrentThreadData();
	if(threadData == nullptr)
		return;

	TShared<render::GpuCommandBuffer> commandBufferToSubmit = threadData->CurrentCommandBuffer;
	threadData->CurrentCommandBuffer = nullptr;

	if(commandBufferToSubmit != nullptr)
	{
		commandBufferToSubmit->End();

		TShared<GpuQueue> queue = mGpuDevice.GetQueue(mTargetQueueType, mTargetQueueIndex);
		if(queue != nullptr)
		{
			GpuSubmissionInformation submissionInfo;
			submissionInfo.CommandBuffer = commandBufferToSubmit;
			submissionInfo.SyncMask = GpuQueueMask::kAll;
			queue->SubmitCommandBuffer(submissionInfo, false);
		}
	}

	if(wait)
	{
		TShared<GpuQueue> queue = mGpuDevice.GetQueue(mTargetQueueType, mTargetQueueIndex);
		if(queue)
			queue->WaitUntilIdle();
	}
}

void GpuTransferBufferHelper::EndFrame()
{
	Lock lock(mRegistryMutex);

	for(auto& threadData : mThreadRegistry)
	{
		if(!threadData->PoolRing)
			continue;

		threadData->PoolRing->AdvanceFrame();
		threadData->CurrentCommandBuffer = nullptr;
	}
}
