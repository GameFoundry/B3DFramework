//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsCommandBuffer.h"
#include "BsCommandBufferManager.h"

namespace BansheeEngine
{
	void CommandSyncMask::addDependency(const SPtr<CommandBuffer>& buffer)
	{
		if (buffer == nullptr)
			return;

		mMask |= getGlobalQueueMask(buffer->getType(), buffer->getQueueIdx());
	}

	UINT32 CommandSyncMask::getGlobalQueueMask(GpuQueueType type, UINT32 queueIdx)
	{
		UINT32 bitShift = 0;
		switch (type)
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

	UINT32 CommandSyncMask::getGlobalQueueIdx(GpuQueueType type, UINT32 queueIdx)
	{
		switch (type)
		{
		case GQT_COMPUTE:
			return 8 + queueIdx;
		case GQT_UPLOAD:
			return 16 + queueIdx;
		default:
			return queueIdx;
		}
	}

	UINT32 CommandSyncMask::getQueueIdxAndType(UINT32 globalQueueIdx, GpuQueueType& type)
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

	CommandBuffer::CommandBuffer(GpuQueueType type, UINT32 deviceIdx, UINT32 queueIdx, bool secondary)
		:mType(type), mDeviceIdx(deviceIdx), mQueueIdx(queueIdx), mIsSecondary(secondary)
	{

	}

	SPtr<CommandBuffer> CommandBuffer::create(GpuQueueType type, UINT32 deviceIdx, UINT32 queueIdx,
		bool secondary)
	{
		return CommandBufferManager::instance().create(type, deviceIdx, queueIdx, secondary);
	}
}