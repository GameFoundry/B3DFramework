//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Managers/BsCommandBufferManager.h"

using namespace bs;
using namespace bs::ct;

SPtr<CommandBuffer> CommandBufferManager::Create(GpuQueueType type, u32 deviceIdx, u32 queueIdx, bool secondary)
{
	assert(deviceIdx < BS_MAX_DEVICES);

	return CreateInternal(type, deviceIdx, queueIdx, secondary);
}
