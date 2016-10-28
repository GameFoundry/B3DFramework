//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsCommandBufferManager.h"

namespace BansheeEngine
{
	SPtr<CommandBuffer> CommandBufferManager::create(CommandBufferType type, UINT32 deviceIdx, UINT32 queueIdx,
		bool secondary)
	{
		UINT32 id = -1;
		for(UINT32 i = 0; i < BS_MAX_COMMAND_BUFFERS; i++)
		{
			if (!mActiveCommandBuffers[i])
			{
				id = i;
				break;
			}
		}

		if(id == -1)
		{
			LOGERR("Attempting to allocate more than 32 command buffers. This is not supported. ");
			return nullptr;
		}

		mActiveCommandBuffers[id] = true;
		return createInternal(id, type, deviceIdx, queueIdx, secondary);
	}

	void CommandBufferManager::notifyCommandBufferDestroyed(UINT32 id)
	{
		mActiveCommandBuffers[id] = false;
	}
}