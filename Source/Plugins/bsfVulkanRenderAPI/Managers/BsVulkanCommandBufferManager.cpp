//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Managers/BsVulkanCommandBufferManager.h"
#include "BsVulkanCommandBuffer.h"
#include "BsVulkanRenderAPI.h"
#include "BsVulkanDevice.h"
#include "BsVulkanQueue.h"
#include "BsVulkanTexture.h"

namespace bs { namespace ct
{
	VulkanTransferBuffer::VulkanTransferBuffer(VulkanDevice* device, GpuQueueType type, UINT32 queueIdx)
		:mDevice(device), mType(type), mQueueIdx(queueIdx)
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
		mCB->memoryBarrier(buffer, srcAccessFlags, dstAccessFlags, srcStage, dstStage);
	}

	void VulkanTransferBuffer::setLayout(VkImage image, VkAccessFlags srcAccessFlags, VkAccessFlags dstAccessFlags, 
		VkImageLayout oldLayout, VkImageLayout newLayout, const VkImageSubresourceRange& range)
	{
		mCB->setLayout(image, srcAccessFlags, dstAccessFlags, oldLayout, newLayout, range);
	}

	void VulkanTransferBuffer::setLayout(VulkanImage* image, const VkImageSubresourceRange& range, 
										 VkAccessFlags newAccessMask, VkImageLayout newLayout)
	{
		image->getBarriers(range, mBarriersTemp);

		if (mBarriersTemp.size() == 0)
			return;

		INT32 count = (INT32)mBarriersTemp.size();
		for(INT32 i = 0; i < count; i++)
		{
			VkImageMemoryBarrier& barrier = mBarriersTemp[i];

			// Remove barriers that don't signify a layout change
			if(barrier.oldLayout == newLayout)
			{
				if(i < (count - 1))
					std::swap(mBarriersTemp[i], mBarriersTemp[count - 1]);

				mBarriersTemp.erase(mBarriersTemp.begin() + count - 1);
				count--;
				i--;
			}
		}

		for(auto& entry : mBarriersTemp)
		{
			entry.dstAccessMask = newAccessMask;
			entry.newLayout = newLayout;
		}

		vkCmdPipelineBarrier(mCB->getHandle(),
							 VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
							 VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
							 0, 0, nullptr,
							 0, nullptr,
							 (UINT32)mBarriersTemp.size(), mBarriersTemp.data());

		mBarriersTemp.clear();		
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
			mDevice->refreshStates(true);

			assert(!mCB->isSubmitted());
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
		bool semaphoreRequestFailed = false;
		SPtr<VulkanDevice> device = mRapi._getDevice(deviceIdx);

		UINT32 semaphoreIdx = 0;
		for (UINT32 i = 0; i < GQT_COUNT; i++)
		{
			GpuQueueType queueType = (GpuQueueType)i;

			UINT32 numQueues = device->getNumQueues(queueType);
			for (UINT32 j = 0; j < numQueues; j++)
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

				VulkanSemaphore* semaphore = lastCB->requestInterQueueSemaphore();
				if (semaphore == nullptr)
				{
					semaphoreRequestFailed = true;
					continue;
				}

				semaphores[semaphoreIdx++] = semaphore;
			}
		}

		count = semaphoreIdx;

		if (semaphoreRequestFailed)
		{
			LOGERR("Failed to allocate semaphores for a command buffer sync. This means some of the dependency requests "
				"will not be fulfilled. This happened because a command buffer has too many dependant command "
				"buffers. The maximum allowed number is " + toString(BS_MAX_VULKAN_CB_DEPENDENCIES) + " but can be "
				"increased by incrementing the value of BS_MAX_VULKAN_CB_DEPENDENCIES.");
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
}}