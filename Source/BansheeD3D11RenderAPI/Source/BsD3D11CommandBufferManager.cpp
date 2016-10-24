//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsD3D11CommandBufferManager.h"
#include "BsD3D11CommandBuffer.h"

namespace BansheeEngine
{
	SPtr<CommandBuffer> D3D11CommandBufferManager::create(CommandBufferType type, UINT32 deviceIdx, UINT32 syncMask,
		bool secondary)
	{
		CommandBuffer* buffer = new (bs_alloc<D3D11CommandBuffer>()) D3D11CommandBuffer(type, deviceIdx, syncMask, secondary);
		return bs_shared_ptr(buffer);
	}
}