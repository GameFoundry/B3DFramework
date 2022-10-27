//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsD3D11CommandBufferManager.h"
#include "BsD3D11CommandBuffer.h"

using namespace bs;
using namespace bs::ct;

SPtr<CommandBuffer> D3D11CommandBufferManager::CreateInternal(GpuQueueType type, u32 deviceIdx, u32 queueIdx, bool secondary)
{
	CommandBuffer* buffer = new(bs_alloc<D3D11CommandBuffer>()) D3D11CommandBuffer(type, deviceIdx, queueIdx, secondary);
	return bs_shared_ptr(buffer);
}
