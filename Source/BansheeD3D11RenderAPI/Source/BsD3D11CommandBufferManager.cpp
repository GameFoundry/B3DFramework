//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsD3D11CommandBufferManager.h"
#include "BsD3D11CommandBuffer.h"

namespace BansheeEngine
{
	SPtr<CommandBuffer> D3D11CommandBufferManager::createInternal(UINT32 id, CommandBufferType type, UINT32 deviceIdx, 
		UINT32 queueIdx, bool secondary)
	{
		CommandBuffer* buffer = new (bs_alloc<D3D11CommandBuffer>()) D3D11CommandBuffer(id, type, deviceIdx, queueIdx, secondary);
		return bs_shared_ptr(buffer);
	}
}