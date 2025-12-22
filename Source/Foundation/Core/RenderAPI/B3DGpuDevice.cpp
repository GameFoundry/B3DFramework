//************************************ B3D Framework - Copyright 2023 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DGpuDevice.h"
#include "B3DGpuCommandBuffer.h"
#include "B3DGpuCommandBufferPoolRing.h"
#include "B3DEventQuery.h"
#include "B3DGpuFrameCapture.h"

using namespace b3d;

const GpuQueueMask GpuQueueMask::kNone = GpuQueueMask(0);
const GpuQueueMask GpuQueueMask::kAll = GpuQueueMask(~0u);

GpuQueue::GpuQueue(GpuDevice& gpuDevice, GpuQueueType type, u32 index)
	:mGpuDevice(gpuDevice), mType(type), mIndex(index)
{
	
}

GpuQueue::~GpuQueue()
{
	WaitGroup waitGroup;
	for(auto& entry : mTransferCommandBuffers)
	{
		if(!entry.second.PoolRing)
			continue;

		// Get scheduler thread from the first pool in the ring (all pools have the same owning thread)
		render::GpuCommandBufferPool& firstPool = entry.second.PoolRing->GetCurrentPool();

		waitGroup.Increment();

		// It's important we queue the destroy on the thread the command buffer pool was created on, as command buffers are bound to a single thread. We don't use the command buffer pool message queue directly
		// as the destroy operation may wait on the queue to complete, which would result in a deadlock.
		const SPtr<SchedulerThread> ownerSchedulerThread = firstPool.GetMessageQueue().GetSchedulerThread();
		if(B3D_ENSURE(ownerSchedulerThread))
		{
			ownerSchedulerThread->Post(SchedulerTask([&waitGroup, poolRing = entry.second.PoolRing.get()]
			{
				poolRing->Destroy();
				waitGroup.NotifyDone();
			}, "Destroy GpuCommandBufferPoolRing"));
		}
	}

	waitGroup.Wait();
}

SPtr<render::GpuCommandBuffer> GpuQueue::GetOrCreateTransferCommandBuffer()
{
	Lock lock(mMutex);

	PerThreadTransferCommandBufferInformation& transferCommandBufferInformation = mTransferCommandBuffers[B3D_CURRENT_THREAD_ID];
	if(!transferCommandBufferInformation.PoolRing)
	{
		render::GpuCommandBufferPoolCreateInformation poolCreateInformation;
		poolCreateInformation.Thread = B3D_CURRENT_THREAD_ID;
		poolCreateInformation.Type = mType;
		poolCreateInformation.UsePoolReset = true;

		transferCommandBufferInformation.PoolRing = B3DMakeUnique<render::GpuCommandBufferPoolRing>(mGpuDevice, poolCreateInformation);
	}

	if(transferCommandBufferInformation.CurrentTransferCommandBuffer == nullptr)
	{
		render::GpuCommandBufferCreateInformation commandBufferCreateInformation;
		commandBufferCreateInformation.Name = "Transfer";

		transferCommandBufferInformation.CurrentTransferCommandBuffer = transferCommandBufferInformation.PoolRing->GetCurrentPool().FindOrCreate(commandBufferCreateInformation);
	}

	return transferCommandBufferInformation.CurrentTransferCommandBuffer;
}

void GpuDevice::SubmitCommandBuffer(const SPtr<render::GpuCommandBuffer>& commandBuffer, GpuQueueMask syncMask, u32 queueIndex)
{
	if (!B3D_ENSURE(commandBuffer))
		return;

	const u32 queueCount = GetQueueCount(commandBuffer->GetQueueType());
	if (!B3D_ENSURE(queueIndex < queueCount))
		return;

	const SPtr<GpuQueue>& queue = GetQueue(commandBuffer->GetQueueType(), queueIndex);
	if (!B3D_ENSURE(queue))
		return;

	queue->SubmitCommandBuffer(commandBuffer, syncMask);
}

SPtr<SamplerState> GpuDevice::FindOrCreateSamplerState(const SamplerStateCreateInformation& createInformation)
{
	Lock lock(mSamplerStateMutex);

	if (auto found = mCachedSamplerStates.find(createInformation); found != mCachedSamplerStates.end())
	{
		SPtr<SamplerState> existingSamplerState = found->second;
		if (existingSamplerState != nullptr)
			return existingSamplerState;
	}

	SPtr<SamplerState> newSamplerState = CreateSamplerState(createInformation);
	mCachedSamplerStates[createInformation] = newSamplerState;

	return newSamplerState;
}

void GpuQueue::SubmitCommandBuffer(const SPtr<render::GpuCommandBuffer>& commandBuffer, GpuQueueMask syncMask)
{
	SubmitCommandBuffer(commandBuffer, syncMask, true);
}

void GpuQueue::SubmitTransferCommandBuffer(bool wait)
{
	SPtr<render::GpuCommandBuffer> commandBufferToSubmit;

	{
		Lock lock(mMutex);

		if(auto found = mTransferCommandBuffers.find(B3D_CURRENT_THREAD_ID); found != mTransferCommandBuffers.end())
		{
			commandBufferToSubmit = found->second.CurrentTransferCommandBuffer;
			found->second.CurrentTransferCommandBuffer = nullptr;
		}
	}

	if (commandBufferToSubmit != nullptr)
	{
		commandBufferToSubmit->End();
		SubmitCommandBuffer(commandBufferToSubmit, 0xFFFFFFFF, false);
	}

	if (wait)
		WaitUntilIdle();
}

void GpuQueue::EndFrame()
{
	Lock lock(mMutex);

	for(auto& entry : mTransferCommandBuffers)
	{
		if(!entry.second.PoolRing)
			continue;

		entry.second.PoolRing->AdvanceFrame();
		entry.second.CurrentTransferCommandBuffer = nullptr;
	}
}

