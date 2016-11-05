//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsVulkanCommandBufferManager.h"
#include "BsVulkanCommandBuffer.h"
#include "BsVulkanRenderAPI.h"
#include "BsVulkanDevice.h"

namespace BansheeEngine
{
	VulkanCommandBufferManager::VulkanCommandBufferManager(const VulkanRenderAPI& rapi)
		:mRapi(rapi), mDeviceData(nullptr), mNumDevices(rapi.getNumDevices())
	{
		mDeviceData = bs_newN<PerDeviceData>(mNumDevices);
		for(UINT32 i = 0; i < mNumDevices; i++)
			memset(mDeviceData[i].buffers, 0, BS_MAX_COMMAND_BUFFERS * sizeof(VulkanCmdBuffer*));
	}

	VulkanCommandBufferManager::~VulkanCommandBufferManager()
	{
		bs_deleteN(mDeviceData, mNumDevices);
	}

	SPtr<CommandBuffer> VulkanCommandBufferManager::createInternal(GpuQueueType type, UINT32 deviceIdx,
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
			new (bs_alloc<VulkanCommandBuffer>()) VulkanCommandBuffer(*device, type, deviceIdx, queueIdx, secondary);

		return bs_shared_ptr(buffer);
	}

	void VulkanCommandBufferManager::setActiveBuffer(GpuQueueType type, UINT32 deviceIdx, UINT32 queueIdx, 
		VulkanCmdBuffer* buffer)
	{
		assert(deviceIdx < mNumDevices);
		assert(buffer->isSubmitted());

		UINT32 idx = CommandSyncMask::getGlobalQueueIdx(type, queueIdx);
		mDeviceData[deviceIdx].buffers[idx] = buffer;
	}

	void VulkanCommandBufferManager::getSyncSemaphores(UINT32 deviceIdx, UINT32 syncMask, 
		VkSemaphore(&semaphores)[BS_MAX_COMMAND_BUFFERS], UINT32& count)
	{
		assert(deviceIdx < mNumDevices);
		const PerDeviceData& deviceData = mDeviceData[deviceIdx];

		UINT32 semaphoreIdx = 0;
		for (UINT32 i = 0; i < BS_MAX_COMMAND_BUFFERS; i++)
		{
			if (deviceData.buffers[i] == nullptr)
				continue;

			if ((syncMask & (1 << i)) == 0) // We don't care about the command buffer
				continue;

			assert(deviceData.buffers[i]->isSubmitted()); // It shouldn't be here if it wasn't submitted
			semaphores[semaphoreIdx++] = deviceData.buffers[i]->getSemaphore();
		}

		count = semaphoreIdx;
	}

	void VulkanCommandBufferManager::refreshStates(UINT32 deviceIdx)
	{
		assert(deviceIdx < mNumDevices);
		PerDeviceData& deviceData = mDeviceData[deviceIdx];

		UINT32 semaphoreIdx = 0;
		for (UINT32 i = 0; i < BS_MAX_COMMAND_BUFFERS; i++)
		{
			if (deviceData.buffers[i] == nullptr)
				continue;

			VulkanCmdBuffer* cmdBuffer = deviceData.buffers[i];
			cmdBuffer->refreshFenceStatus();
			if (!cmdBuffer->isSubmitted())
				deviceData.buffers[i] = nullptr;
		}
	}
}