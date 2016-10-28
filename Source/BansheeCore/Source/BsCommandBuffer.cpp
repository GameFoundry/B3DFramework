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

		mMask |= 1 << buffer->_getId();
	}

	CommandBuffer::CommandBuffer(UINT32 id, CommandBufferType type, UINT32 queueIdx, bool secondary)
		:mId(id), mType(type), mQueueIdx(queueIdx), mIsSecondary(secondary)
	{

	}

	CommandBuffer::~CommandBuffer()
	{
		CommandBufferManager::instance().notifyCommandBufferDestroyed(mId);
	}

	SPtr<CommandBuffer> CommandBuffer::create(CommandBufferType type, UINT32 deviceIdx, UINT32 queueIdx,
		bool secondary)
	{
		return CommandBufferManager::instance().create(type, deviceIdx, queueIdx, secondary);
	}
}