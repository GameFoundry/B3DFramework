//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsGLCommandBufferManager.h"
#include "BsGLCommandBuffer.h"

namespace BansheeEngine
{
	SPtr<CommandBuffer> GLCommandBufferManager::createInternal(UINT32 id, CommandBufferType type, UINT32 deviceIdx, 
		UINT32 queueIdx, bool secondary)
	{
		CommandBuffer* buffer = new (bs_alloc<GLCommandBuffer>()) GLCommandBuffer(id, type, deviceIdx, queueIdx, secondary);
		return bs_shared_ptr(buffer);
	}
}