//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsGLCommandBufferManager.h"
#include "BsGLCommandBuffer.h"

namespace BansheeEngine
{
	SPtr<CommandBuffer> GLCommandBufferManager::create(CommandBufferType type, UINT32 deviceIdx, UINT32 syncMask,
		bool secondary)
	{
		CommandBuffer* buffer = new (bs_alloc<GLCommandBuffer>()) GLCommandBuffer(type, deviceIdx, syncMask, secondary);
		return bs_shared_ptr(buffer);
	}
}