//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsGLCommandBufferManager.h"
#include "BsGLCommandBuffer.h"

namespace bs
{
	namespace ct
	{
		SPtr<CommandBuffer> GLCommandBufferManager::CreateInternal(GpuQueueType type, u32 deviceIdx, u32 queueIdx, bool secondary)
		{
			CommandBuffer* buffer = new(bs_alloc<GLCommandBuffer>()) GLCommandBuffer(type, deviceIdx, queueIdx, secondary);
			return bs_shared_ptr(buffer);
		}
	} // namespace ct
} // namespace bs
