//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsVulkanCommandBuffer.h"
#include "BsVulkanUtility.h"
#include "BsVulkanDevice.h"

namespace BansheeEngine
{
	VulkanCommandBuffer::VulkanCommandBuffer(const SPtr<VulkanDevice>& device, CommandBufferType type, UINT32 syncMask, 
		bool secondary)
		: CommandBuffer(type, syncMask, secondary)
	{
		VulkanQueueType queueType = VulkanUtility::getQueueType(type);
		mDevice = device->getLogical();
		mPool = device->getCommandPool(queueType);

		if(mPool == VK_NULL_HANDLE)
			mPool = device->getCommandPool(VQT_GRAPHICS); // Graphics queue is guaranteed to exist

		VkCommandBufferAllocateInfo bufferInfo = {};
		bufferInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		bufferInfo.commandPool = mPool;
		bufferInfo.level = secondary ? VK_COMMAND_BUFFER_LEVEL_SECONDARY : VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		bufferInfo.commandBufferCount = 1;

		VkResult result = vkAllocateCommandBuffers(mDevice, &bufferInfo, &mBuffer);
		assert(result == VK_SUCCESS);
	}

	VulkanCommandBuffer::~VulkanCommandBuffer()
	{
		vkFreeCommandBuffers(mDevice, mPool, 1, &mBuffer);
	}
}