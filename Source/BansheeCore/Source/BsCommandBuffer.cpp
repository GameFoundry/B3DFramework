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

		mMask |= getGlobalQueueIdx(buffer->getType(), buffer->getQueueIdx());
	}

	UINT32 CommandSyncMask::getGlobalQueueIdx(GpuQueueType type, UINT32 queueIdx)
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

	CommandBuffer::CommandBuffer(UINT32 id, GpuQueueType type, UINT32 deviceIdx, UINT32 queueIdx, bool secondary)
		:mId(id), mType(type), mDeviceIdx(deviceIdx), mQueueIdx(queueIdx), mIsSecondary(secondary)
	{

	}

	CommandBuffer::~CommandBuffer()
	{
		CommandBufferManager::instance().notifyCommandBufferDestroyed(mDeviceIdx, mId);
	}

	SPtr<CommandBuffer> CommandBuffer::create(GpuQueueType type, UINT32 deviceIdx, UINT32 queueIdx,
		bool secondary)
	{
		return CommandBufferManager::instance().create(type, deviceIdx, queueIdx, secondary);
	}
}