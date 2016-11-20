//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsCommandBufferManager.h"

namespace bs
{
	SPtr<CommandBuffer> CommandBufferManager::create(GpuQueueType type, UINT32 deviceIdx, UINT32 queueIdx,
		bool secondary)
	{
		assert(deviceIdx < BS_MAX_DEVICES);

		return createInternal(type, deviceIdx, queueIdx, secondary);
	}
}