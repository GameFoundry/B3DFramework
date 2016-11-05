//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsVulkanQueue.h"
#include "BsVulkanCommandBuffer.h"

namespace BansheeEngine
{
	VulkanQueue::VulkanQueue(VulkanDevice& device, VkQueue queue, GpuQueueType type, UINT32 index)
		:mDevice(device), mQueue(queue), mType(type), mIndex(index), mLastCommandBuffer(nullptr)
	{ }

	bool VulkanQueue::isExecuting() const
	{
		if (mLastCommandBuffer == nullptr)
			return false;

		return mLastCommandBuffer->isSubmitted();
	}

	void VulkanQueue::submit(VulkanCmdBuffer* cmdBuffer, VkSemaphore* waitSemaphores, UINT32 semaphoresCount)
	{
		VkCommandBuffer vkCmdBuffer = cmdBuffer->getHandle();
		VkSemaphore semaphore = cmdBuffer->getSemaphore();

		VkSubmitInfo submitInfo;
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.pNext = nullptr;
		submitInfo.pWaitDstStageMask = 0;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &vkCmdBuffer;
		submitInfo.signalSemaphoreCount = 1;
		submitInfo.pSignalSemaphores = &semaphore;

		if (semaphoresCount > 0)
			submitInfo.pWaitSemaphores = waitSemaphores;
		else
			submitInfo.pWaitSemaphores = nullptr;

		vkQueueSubmit(mQueue, 1, &submitInfo, cmdBuffer->getFence());

		mLastCommandBuffer = cmdBuffer;
	}
}