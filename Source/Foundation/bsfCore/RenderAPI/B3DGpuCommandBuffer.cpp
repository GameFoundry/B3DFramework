//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "RenderAPI/BsGpuCommandBuffer.h"

using namespace b3d;

namespace b3d { namespace render
{
u32 CommandSyncMask::GetGlobalQueueMask(GpuQueueUsage type, u32 queueIdx)
{
	u32 bitShift = 0;
	switch(type)
	{
	case GQT_GRAPHICS:
		break;
	case GQT_COMPUTE:
		bitShift = 8;
		break;
	case GQT_TRANSFER:
		bitShift = 16;
		break;
	default:
		break;
	}

	return (1 << queueIdx) << bitShift;
}

u32 CommandSyncMask::GetGlobalQueueIdx(GpuQueueUsage type, u32 queueIdx)
{
	switch(type)
	{
	case GQT_COMPUTE:
		return 8 + queueIdx;
	case GQT_TRANSFER:
		return 16 + queueIdx;
	default:
		return queueIdx;
	}
}

u32 CommandSyncMask::GetQueueIdxAndType(u32 globalQueueIdx, GpuQueueUsage& type)
{
	if(globalQueueIdx >= 16)
	{
		type = GQT_TRANSFER;
		return globalQueueIdx - 16;
	}

	if(globalQueueIdx >= 8)
	{
		type = GQT_COMPUTE;
		return globalQueueIdx - 8;
	}

	type = GQT_GRAPHICS;
	return globalQueueIdx;
}

GpuCommandBufferPool::GpuCommandBufferPool(GpuDevice& gpuDevice, const GpuCommandBufferPoolCreateInformation& createInformation)
	:mGpuDevice(gpuDevice), mInformation(createInformation)
{
	// Process messages related to this command buffer pool on this thread. Mostly these are command buffer resets once they are done executing.
	Scheduler* const scheduler = Scheduler::Get();
	if (B3D_ENSURE(scheduler))
	{
		mMessageQueue.ScheduleRunUntilShutdown(*scheduler, true);
	}
}

void GpuCommandBufferPool::Destroy()
{
	if (mIsDestroyed)
		return;

	mIsDestroyed = true;
}

GpuCommandBuffer::GpuCommandBuffer(GpuDevice& gpuDevice, ThreadId ownerThread, GpuQueueUsage queueType, const GpuCommandBufferCreateInformation& createInformation)
	:mGpuDevice(gpuDevice), mUsage(queueType), mOwnerThread(ownerThread), mInformation(createInformation)
{ }


GpuCommandBuffer::~GpuCommandBuffer()
{
	OnDestroyed(mIsSubmitted);
}
}}
