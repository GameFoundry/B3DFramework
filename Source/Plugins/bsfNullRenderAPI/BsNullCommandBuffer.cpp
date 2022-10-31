//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsNullCommandBuffer.h"

using namespace bs;
using namespace bs::ct;

SPtr<CommandBuffer> NullCommandBufferManager::CreateInternal(GpuQueueType type, u32 deviceIdx, u32 queueIdx, bool secondary)
{
	CommandBuffer* buffer = new(B3DAllocate<NullCommandBuffer>()) NullCommandBuffer(type, deviceIdx, queueIdx, secondary);
	return B3DMakeSharedFromExisting(buffer);
}
