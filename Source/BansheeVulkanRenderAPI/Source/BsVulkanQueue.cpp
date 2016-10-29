//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsVulkanQueue.h"
#include "BsVulkanCommandBuffer.h"

namespace BansheeEngine
{
	VulkanQueue::VulkanQueue(VkQueue queue)
		:mQueue(queue), mFenceCounter(0), mLastCommandBufferId(-1)
	{ }

	void VulkanQueue::notifySubmit(const VulkanCommandBuffer& cmdBuffer, UINT32 fenceCounter)
	{
		mLastCommandBufferId = cmdBuffer._getId();
		mFenceCounter = fenceCounter;
	}
}