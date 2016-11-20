//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsGLCommandBufferManager.h"
#include "BsGLCommandBuffer.h"

namespace bs
{
	SPtr<CommandBuffer> GLCommandBufferManager::createInternal(GpuQueueType type, UINT32 deviceIdx,
		UINT32 queueIdx, bool secondary)
	{
		CommandBuffer* buffer = new (bs_alloc<GLCommandBuffer>()) GLCommandBuffer(type, deviceIdx, queueIdx, secondary);
		return bs_shared_ptr(buffer);
	}
}