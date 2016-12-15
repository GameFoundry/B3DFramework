//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsVulkanCommandBufferManager.h"
#include "BsVulkanCommandBuffer.h"
#include "BsVulkanRenderAPI.h"
#include "BsVulkanDevice.h"
#include "BsVulkanQueue.h"

namespace bs
{
	VulkanTransferBuffer::VulkanTransferBuffer()
		:mDevice(nullptr), mType(GQT_GRAPHICS), mQueueIdx(0), mQueue(nullptr), mCB(nullptr), mSyncMask(0), mQueueMask(0)
	{ }

	VulkanTransferBuffer::VulkanTransferBuffer(VulkanDevice* device, GpuQueueType type, UINT32 queueIdx)
		:mDevice(device), mType(type), mQueueIdx(queueIdx), mQueue(nullptr), mCB(nullptr), mSyncMask(0), mQueueMask(0)
	{
		UINT32 numQueues = device->getNumQueues(mType);
		if (numQueues == 0)
		{
			mType = GQT_GRAPHICS;
			numQueues = device->getNumQueues(GQT_GRAPHICS);
		}

		UINT32 physicalQueueIdx = queueIdx % numQueues;
		mQueue = device->getQueue(mType, physicalQueueIdx);
		mQueueMask = device->getQueueMask(mType, queueIdx);
	}

	VulkanTransferBuffer::~VulkanTransferBuffer()
	{
		if (mCB != nullptr)
			mCB->end();
	}

	void VulkanTransferBuffer::allocate()
	{
		if (mCB != nullptr)
			return;

		UINT32 queueFamily = mDevice->getQueueFamily(mType);
		mCB = mDevice->getCmdBufferPool().getBuffer(queueFamily, false);
	}

	void VulkanTransferBuffer::memoryBarrier(VkBuffer buffer, VkAccessFlags srcAccessFlags, VkAccessFlags dstAccessFlags,
					   VkPipelineStageFlags srcStage, VkPipelineStageFlags dstStage)
	{
		VkBufferMemoryBarrier barrier;
		barrier.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
		barrier.pNext = nullptr;
		barrier.srcAccessMask = srcAccessFlags;
		barrier.dstAccessMask = dstAccessFlags;
		barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.buffer = buffer;
		barrier.offset = 0;
		barrier.size = VK_WHOLE_SIZE;

		vkCmdPipelineBarrier(mCB->getHandle(),
							 srcStage,
							 dstStage,
							 0, 0, nullptr,
							 1, &barrier,
							 0, nullptr);
	}

	void VulkanTransferBuffer::setLayout(VkImage image, VkAccessFlags srcAccessFlags, VkAccessFlags dstAccessFlags, 
		VkImageLayout oldLayout, VkImageLayout newLayout, const VkImageSubresourceRange& range)
	{
		VkImageMemoryBarrier barrier;
		barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		barrier.pNext = nullptr;
		barrier.srcAccessMask = srcAccessFlags;
		barrier.dstAccessMask = dstAccessFlags;
		barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.oldLayout = oldLayout;
		barrier.newLayout = newLayout;
		barrier.image = image;
		barrier.subresourceRange = range;

		vkCmdPipelineBarrier(mCB->getHandle(),
							 VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
							 VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
							 0, 0, nullptr,
							 0, nullptr,
							 1, &barrier);
	}

	void VulkanTransferBuffer::flush(bool wait)
	{
		if (mCB == nullptr)
			return;

		UINT32 syncMask = mSyncMask & ~mQueueMask; // Don't sync with itself

		mCB->end();
		mCB->submit(mQueue, mQueueIdx, syncMask);

		if (wait)
		{
			mQueue->waitIdle();
			gVulkanCBManager().refreshStates(mDevice->getIndex());
		}

		mCB = nullptr;
	}

	VulkanCommandBufferManager::VulkanCommandBufferManager(const VulkanRenderAPI& rapi)
		:mRapi(rapi), mDeviceData(nullptr), mNumDevices(rapi.getNumDevices())
	{
		mDeviceData = bs_newN<PerDeviceData>(mNumDevices);
		for (UINT32 i = 0; i < mNumDevices; i++)
		{
			SPtr<VulkanDevice> device = rapi._getDevice(i);

			for (UINT32 j = 0; j < GQT_COUNT; j++)
			{
				GpuQueueType queueType = (GpuQueueType)j;

				for (UINT32 k = 0; k < BS_MAX_QUEUES_PER_TYPE; k++)
					mDeviceData[i].transferBuffers[j][k] = VulkanTransferBuffer(device.get(), queueType, k);
			}
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

	void VulkanCommandBufferManager::getSyncSemaphores(UINT32 deviceIdx, UINT32 syncMask, VulkanSemaphore** semaphores,
		UINT32& count)
	{
		SPtr<VulkanDevice> device = mRapi._getDevice(deviceIdx);

		UINT32 semaphoreIdx = 0;
		for(UINT32 i = 0; i < GQT_COUNT; i++)
		{
			GpuQueueType queueType = (GpuQueueType)i;

			UINT32 numQueues = device->getNumQueues(queueType);
			for(UINT32 j = 0; j < numQueues; j++)
			{
				VulkanQueue* queue = device->getQueue(queueType, j);
				VulkanCmdBuffer* lastCB = queue->getLastCommandBuffer();

				// Check if a buffer is currently executing on the queue
				if (lastCB == nullptr || !lastCB->isSubmitted())
					continue;

				// Check if we care about this specific queue
				UINT32 queueMask = device->getQueueMask(queueType, j);
				if ((syncMask & queueMask) == 0)
					continue;

				semaphores[semaphoreIdx++] = lastCB->getSemaphore();
			}
		}

		count = semaphoreIdx;
	}

	void VulkanCommandBufferManager::refreshStates(UINT32 deviceIdx)
	{
		SPtr<VulkanDevice> device = mRapi._getDevice(deviceIdx);

		for (UINT32 i = 0; i < GQT_COUNT; i++)
		{
			UINT32 numQueues = device->getNumQueues((GpuQueueType)i);
			for (UINT32 j = 0; j < numQueues; j++)
			{
				VulkanQueue* queue = device->getQueue((GpuQueueType)i, j);
				queue->refreshStates();
			}
		}
	}

	VulkanTransferBuffer* VulkanCommandBufferManager::getTransferBuffer(UINT32 deviceIdx, GpuQueueType type,
		UINT32 queueIdx)
	{
		assert(deviceIdx < mNumDevices);

		PerDeviceData& deviceData = mDeviceData[deviceIdx];

		VulkanTransferBuffer* transferBuffer = &deviceData.transferBuffers[type][queueIdx];
		transferBuffer->allocate();
		return transferBuffer;
	}

	void VulkanCommandBufferManager::flushTransferBuffers(UINT32 deviceIdx)
	{
		assert(deviceIdx < mNumDevices);

		PerDeviceData& deviceData = mDeviceData[deviceIdx];
		for (UINT32 i = 0; i < GQT_COUNT; i++)
		{
			for (UINT32 j = 0; j < BS_MAX_QUEUES_PER_TYPE; j++)
				deviceData.transferBuffers[i][j].flush(false);
		}
	}

	VulkanCommandBufferManager& gVulkanCBManager()
	{
		return static_cast<VulkanCommandBufferManager&>(CommandBufferManager::instance());
	}
}