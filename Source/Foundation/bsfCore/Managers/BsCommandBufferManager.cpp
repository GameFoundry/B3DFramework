//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Managers/BsCommandBufferManager.h"

using namespace bs;
using namespace bs::ct;

SPtr<CommandBuffer> CommandBufferManager::Create(GpuQueueType queueType)
{
	return CreateInternal(queueType);
}
