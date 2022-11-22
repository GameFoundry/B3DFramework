//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "RenderAPI/BsCommandBuffer.h"
#include "Managers/BsCommandBufferManager.h"

using namespace bs;

namespace bs { namespace ct
{
void CommandSyncMask::AddDependency(const SPtr<CommandBuffer>& buffer)
{
	if(buffer == nullptr)
		return;

	mMask |= GetGlobalQueueMask(buffer->GetType(), buffer->GetQueueIdx());
}

u32 CommandSyncMask::GetGlobalQueueMask(GpuQueueType type, u32 queueIdx)
{
	u32 bitShift = 0;
	switch(type)
	{
	case GQT_GRAPHICS:
		break;
	case GQT_COMPUTE:
		bitShift = 8;
		break;
	case GQT_UPLOAD:
		bitShift = 16;
		break;
	default:
		break;
	}

	return (1 << queueIdx) << bitShift;
}

u32 CommandSyncMask::GetGlobalQueueIdx(GpuQueueType type, u32 queueIdx)
{
	switch(type)
	{
	case GQT_COMPUTE:
		return 8 + queueIdx;
	case GQT_UPLOAD:
		return 16 + queueIdx;
	default:
		return queueIdx;
	}
}

u32 CommandSyncMask::GetQueueIdxAndType(u32 globalQueueIdx, GpuQueueType& type)
{
	if(globalQueueIdx >= 16)
	{
		type = GQT_UPLOAD;
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

CommandBuffer::CommandBuffer(GpuQueueType type, u32 deviceIdx, u32 queueIdx, bool secondary)
	: mType(type), mDeviceIdx(deviceIdx), mQueueIdx(queueIdx), mIsSecondary(secondary)
{
}

CommandBuffer::~CommandBuffer()
{
	OnDestroyed(mIsSubmitted);
}

SPtr<CommandBuffer> CommandBuffer::Create(GpuQueueType type, u32 deviceIdx, u32 queueIdx, bool secondary)
{
	return CommandBufferManager::Instance().Create(type, deviceIdx, queueIdx, secondary);
}
}}
