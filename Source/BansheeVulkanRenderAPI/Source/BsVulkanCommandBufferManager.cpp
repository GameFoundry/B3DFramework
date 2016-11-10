//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsVulkanCommandBufferManager.h"
#include "BsVulkanCommandBuffer.h"
#include "BsVulkanRenderAPI.h"
#include "BsVulkanDevice.h"

namespace BansheeEngine
{
	VulkanTransferBufferInfo::VulkanTransferBufferInfo(UINT32 queueIdx)
		:mCB(nullptr), mSyncMask(0), mQueueIdx(queueIdx)
	{ }

	VulkanCommandBufferManager::VulkanCommandBufferManager(const VulkanRenderAPI& rapi)
		:mRapi(rapi), mDeviceData(nullptr), mNumDevices(rapi.getNumDevices())
	{
		mDeviceData = bs_newN<PerDeviceData>(mNumDevices);
		for (UINT32 i = 0; i < mNumDevices; i++)
		{
			bs_zero_out(mDeviceData[i].activeBuffers);
			bs_zero_out(mDeviceData[i].transferBuffers);
		}
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
		mDeviceData[deviceIdx].activeBuffers[idx] = buffer;
	}

	void VulkanCommandBufferManager::getSyncSemaphores(UINT32 deviceIdx, UINT32 syncMask, VkSemaphore* semaphores, 
		UINT32& count)
	{
		assert(deviceIdx < mNumDevices);
		const PerDeviceData& deviceData = mDeviceData[deviceIdx];

		UINT32 semaphoreIdx = 0;
		for (UINT32 i = 0; i < BS_MAX_UNIQUE_QUEUES; i++)
		{
			if (deviceData.activeBuffers[i] == nullptr)
				continue;

			if ((syncMask & (1 << i)) == 0) // We don't care about the command buffer
				continue;

			assert(deviceData.activeBuffers[i]->isSubmitted()); // It shouldn't be here if it wasn't submitted
			semaphores[semaphoreIdx++] = deviceData.activeBuffers[i]->getSemaphore();
		}

		count = semaphoreIdx;
	}

	void VulkanCommandBufferManager::refreshStates(UINT32 deviceIdx)
	{
		assert(deviceIdx < mNumDevices);
		PerDeviceData& deviceData = mDeviceData[deviceIdx];

		UINT32 semaphoreIdx = 0;
		for (UINT32 i = 0; i < BS_MAX_UNIQUE_QUEUES; i++)
		{
			if (deviceData.activeBuffers[i] == nullptr)
				continue;

			VulkanCmdBuffer* cmdBuffer = deviceData.activeBuffers[i];
			cmdBuffer->refreshFenceStatus();
			if (!cmdBuffer->isSubmitted())
				deviceData.activeBuffers[i] = nullptr;
		}
	}

	VulkanTransferBufferInfo* VulkanCommandBufferManager::getTransferBuffer(UINT32 deviceIdx, GpuQueueType type, 
		UINT32 queueIdx)
	{
		assert(deviceIdx < mNumDevices);

		UINT32 globalQueueIdx = CommandSyncMask::getGlobalQueueIdx(type, queueIdx);
		assert(globalQueueIdx < BS_MAX_UNIQUE_QUEUES);

		PerDeviceData& deviceData = mDeviceData[deviceIdx];
		if (deviceData.transferBuffers[globalQueueIdx].mCB == nullptr)
		{
			SPtr<VulkanDevice> device = mRapi._getDevice(deviceIdx);

			UINT32 queueFamily = device->getQueueFamily(type);
			deviceData.transferBuffers[globalQueueIdx].mCB = device->getCmdBufferPool().getBuffer(queueFamily, false);
		}

		return &deviceData.transferBuffers[globalQueueIdx];
	}

	void VulkanCommandBufferManager::flushTransferBuffers(UINT32 deviceIdx)
	{
		assert(deviceIdx < mNumDevices);

		SPtr<VulkanDevice> device = mRapi._getDevice(deviceIdx);
		PerDeviceData& deviceData = mDeviceData[deviceIdx];

		UINT32 transferBufferIdx = 0;
		for(UINT32 i = 0; i < GQT_COUNT; i++)
		{
			GpuQueueType queueType = (GpuQueueType)i;
			UINT32 numQueues = device->getNumQueues(queueType);
			if (numQueues == 0)
			{
				queueType = GQT_GRAPHICS;
				numQueues = device->getNumQueues(GQT_GRAPHICS);
			}

			for(UINT32 j = 0; j < BS_MAX_QUEUES_PER_TYPE; j++)
			{
				VulkanTransferBufferInfo& bufferInfo = deviceData.transferBuffers[transferBufferIdx];
				if (bufferInfo.mCB == nullptr)
					continue;

				UINT32 physicalQueueIdx = j % numQueues;
				VulkanQueue* queue = device->getQueue(queueType, physicalQueueIdx);

				bufferInfo.mCB->submit(queue, bufferInfo.mQueueIdx, bufferInfo.mSyncMask);
				bufferInfo.mCB = nullptr;

				transferBufferIdx++;
			}
		}
	}
}