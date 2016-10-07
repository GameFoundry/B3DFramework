//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsCommandBuffer.h"
#include "BsCommandBufferManager.h"

namespace BansheeEngine
{
	CommandBuffer::CommandBuffer(CommandBufferType type, UINT32 syncMask, bool secondary)
		:mType(type), mSyncMask(syncMask), mIsSecondary(secondary)
	{

	}

	SPtr<CommandBuffer> CommandBuffer::create(CommandBufferType type, UINT32 deviceIdx, UINT32 syncMask,
		bool secondary)
	{
		return CommandBufferManager::instance().create(type, deviceIdx, syncMask, secondary);
	}
}