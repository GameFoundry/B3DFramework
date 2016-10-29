//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsVulkanCommandBufferManager.h"
#include "BsVulkanCommandBuffer.h"
#include "BsVulkanRenderAPI.h"

namespace BansheeEngine
{
	VulkanCommandBufferManager::VulkanCommandBufferManager(const VulkanRenderAPI& rapi)
		:mRapi(rapi)
	{ }

	VulkanCommandBufferManager::~VulkanCommandBufferManager()
	{
		
	}

	SPtr<CommandBuffer> VulkanCommandBufferManager::createInternal(UINT32 id, CommandBufferType type, UINT32 deviceIdx, 
		UINT32 queueIdx, bool secondary)
	{
		UINT32 numDevices = mRapi._getNumDevices();
		if(deviceIdx >= numDevices)
		{
			LOGERR("Cannot create command buffer, invalid device index: " + toString(deviceIdx) + 
				". Valid range: [0, " + toString(numDevices) + ").");

			return nullptr;
		}

		SPtr<VulkanDevice> device = mRapi._getDevice(deviceIdx);

		CommandBuffer* buffer = 
			new (bs_alloc<VulkanCommandBuffer>()) VulkanCommandBuffer(*device, id, type, deviceIdx, queueIdx, secondary);

		return bs_shared_ptr(buffer);
	}

	void VulkanCommandBufferManager::getSyncSemaphores(UINT32 deviceIdx, UINT32 syncMask, 
		VkSemaphore(&semaphores)[BS_MAX_COMMAND_BUFFERS], UINT32& count)
	{
		assert(deviceIdx < BS_MAX_DEVICES);

		UINT32 semaphoreIdx = 0;
		for (UINT32 i = 0; i < BS_MAX_COMMAND_BUFFERS; i++)
		{
			if ((syncMask & (1 << i)) == 0) // We don't care about the command buffer
				continue;

			if (mActiveCommandBuffers[deviceIdx][i] == nullptr) // Command buffer doesn't exist
				continue;

			VulkanCommandBuffer* cmdBuffer = static_cast<VulkanCommandBuffer*>(mActiveCommandBuffers[deviceIdx][i]);
			VulkanCmdBuffer* lowLevelCmdBuffer = cmdBuffer->mSubmittedBuffer;

			if (lowLevelCmdBuffer == nullptr || !lowLevelCmdBuffer->isSubmitted()) // If not submitted, no need to sync with it
				continue;

			semaphores[semaphoreIdx++] = lowLevelCmdBuffer->getSemaphore();
		}

		count = semaphoreIdx;
	}

	void VulkanCommandBufferManager::refreshStates(UINT32 deviceIdx)
	{
		assert(deviceIdx < BS_MAX_DEVICES);

		UINT32 semaphoreIdx = 0;
		for (UINT32 i = 0; i < BS_MAX_COMMAND_BUFFERS; i++)
		{
			if (mActiveCommandBuffers[deviceIdx][i] == nullptr) // Command buffer doesn't exist
				continue;

			VulkanCommandBuffer* cmdBuffer = static_cast<VulkanCommandBuffer*>(mActiveCommandBuffers[deviceIdx][i]);
			cmdBuffer->refreshSubmitStatus();
		}
	}
}