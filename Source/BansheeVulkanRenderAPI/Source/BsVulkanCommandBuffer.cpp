//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsVulkanCommandBuffer.h"
#include "BsVulkanCommandBufferManager.h"
#include "BsVulkanUtility.h"
#include "BsVulkanDevice.h"
#include "BsVulkanGpuParams.h"
#include "BsVulkanQueue.h"
#include "BsVulkanTexture.h"
#include "BsVulkanHardwareBuffer.h"

namespace BansheeEngine
{
	VulkanCmdBufferPool::VulkanCmdBufferPool(VulkanDevice& device)
		:mDevice(device), mNextId(1)
	{
		for (UINT32 i = 0; i < GQT_COUNT; i++)
		{
			UINT32 familyIdx = device.getQueueFamily((GpuQueueType)i);

			if (familyIdx == (UINT32)-1)
				continue;

			VkCommandPoolCreateInfo poolCI;
			poolCI.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
			poolCI.pNext = nullptr;
			poolCI.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
			poolCI.queueFamilyIndex = familyIdx;

			PoolInfo& poolInfo = mPools[familyIdx];
			poolInfo.queueFamily = familyIdx;
			memset(poolInfo.buffers, 0, sizeof(poolInfo.buffers));

			vkCreateCommandPool(device.getLogical(), &poolCI, gVulkanAllocator, &poolInfo.pool);
		}
	}

	VulkanCmdBufferPool::~VulkanCmdBufferPool()
	{
		// Note: Shutdown should be the only place command buffers are destroyed at, as the system relies on the fact that
		// they won't be destroyed during normal operation.

		for(auto& entry : mPools)
		{
			PoolInfo& poolInfo = entry.second;
			for (UINT32 i = 0; i < BS_MAX_VULKAN_CB_PER_QUEUE_FAMILY; i++)
			{
				VulkanCmdBuffer* buffer = poolInfo.buffers[i];
				if (buffer == nullptr)
					break;

				bs_delete(buffer);
			}

			vkDestroyCommandPool(mDevice.getLogical(), poolInfo.pool, gVulkanAllocator);
		}
	}

	VulkanCmdBuffer* VulkanCmdBufferPool::getBuffer(UINT32 queueFamily, bool secondary)
	{
		auto iterFind = mPools.find(queueFamily);
		if (iterFind != mPools.end())
			return nullptr;

		VulkanCmdBuffer** buffers = iterFind->second.buffers;

		UINT32 i = 0;
		for(; i < BS_MAX_VULKAN_CB_PER_QUEUE_FAMILY; i++)
		{
			if (buffers[i] == nullptr)
				break;

			if(buffers[i]->mState == VulkanCmdBuffer::State::Ready)
			{
				buffers[i]->begin();
				return buffers[i];
			}
		}

		assert(i < BS_MAX_VULKAN_CB_PER_QUEUE_FAMILY &&
			"Too many command buffers allocated. Increment BS_MAX_VULKAN_CB_PER_QUEUE_FAMILY to a higher value. ");

		buffers[i] = createBuffer(queueFamily, secondary);
		buffers[i]->begin();

		return buffers[i];
	}

	VulkanCmdBuffer* VulkanCmdBufferPool::createBuffer(UINT32 queueFamily, bool secondary)
	{
		auto iterFind = mPools.find(queueFamily);
		if (iterFind != mPools.end())
			return nullptr;

		const PoolInfo& poolInfo = iterFind->second;

		return bs_new<VulkanCmdBuffer>(mDevice, mNextId++, poolInfo.pool, poolInfo.queueFamily, secondary);
	}

	VulkanCmdBuffer::VulkanCmdBuffer(VulkanDevice& device, UINT32 id, VkCommandPool pool, UINT32 queueFamily, bool secondary)
		: mId(id), mQueueFamily(queueFamily), mState(State::Ready), mDevice(device), mPool(pool), mFenceCounter(0)
	{
		VkCommandBufferAllocateInfo cmdBufferAllocInfo;
		cmdBufferAllocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		cmdBufferAllocInfo.pNext = nullptr;
		cmdBufferAllocInfo.commandPool = pool;
		cmdBufferAllocInfo.level = secondary ? VK_COMMAND_BUFFER_LEVEL_SECONDARY : VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		cmdBufferAllocInfo.commandBufferCount = 1;

		VkResult result = vkAllocateCommandBuffers(mDevice.getLogical(), &cmdBufferAllocInfo, &mCmdBuffer);
		assert(result == VK_SUCCESS);

		VkFenceCreateInfo fenceCI;
		fenceCI.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		fenceCI.pNext = nullptr;
		fenceCI.flags = 0;

		result = vkCreateFence(mDevice.getLogical(), &fenceCI, gVulkanAllocator, &mFence);
		assert(result == VK_SUCCESS);

		VkSemaphoreCreateInfo semaphoreCI;
		semaphoreCI.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
		semaphoreCI.pNext = nullptr;
		semaphoreCI.flags = 0;

		result = vkCreateSemaphore(mDevice.getLogical(), &semaphoreCI, gVulkanAllocator, &mSemaphore);
		assert(result == VK_SUCCESS);
	}

	VulkanCmdBuffer::~VulkanCmdBuffer()
	{
		VkDevice device = mDevice.getLogical();

		if(mState == State::Submitted)
		{
			// Wait 1s
			UINT64 waitTime = 1000 * 1000 * 1000;
			VkResult result = vkWaitForFences(device, 1, &mFence, true, waitTime);
			assert(result == VK_SUCCESS || result == VK_TIMEOUT);

			if (result == VK_TIMEOUT)
				LOGWRN("Freeing a command buffer before done executing because fence wait expired!");

			// Resources have been marked as used, make sure to notify them we're done with them
			refreshFenceStatus();
		}
		else if(mState != State::Ready) 
		{
			// Notify any resources that they are no longer bound
			for (auto& entry : mResources)
			{
				ResourceUseHandle& useHandle = entry.second;
				assert(useHandle.used);

				entry.first->notifyUnbound();
			}

			for (auto& entry : mImages)
			{
				ResourceUseHandle& useHandle = entry.second.useHandle;
				assert(useHandle.used);

				entry.first->notifyUnbound();
			}

			for (auto& entry : mBuffers)
			{
				ResourceUseHandle& useHandle = entry.second.useHandle;
				assert(useHandle.used);

				entry.first->notifyUnbound();
			}
		}
		
		vkDestroyFence(device, mFence, gVulkanAllocator);
		vkDestroySemaphore(device, mSemaphore, gVulkanAllocator);
		vkFreeCommandBuffers(device, mPool, 1, &mCmdBuffer);
	}

	UINT32 VulkanCmdBuffer::getDeviceIdx() const
	{
		return mDevice.getIndex();
	}

	void VulkanCmdBuffer::begin()
	{
		assert(mState == State::Ready);

		VkCommandBufferBeginInfo beginInfo;
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		beginInfo.pNext = nullptr;
		beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
		beginInfo.pInheritanceInfo = nullptr;

		VkResult result = vkBeginCommandBuffer(mCmdBuffer, &beginInfo);
		assert(result == VK_SUCCESS);

		mState = State::Recording;
	}

	void VulkanCmdBuffer::end()
	{
		assert(mState == State::Recording);

		VkResult result = vkEndCommandBuffer(mCmdBuffer);
		assert(result == VK_SUCCESS);

		mState = State::RecordingDone;
	}

	void VulkanCmdBuffer::beginRenderPass()
	{
		assert(mState == State::Recording);

		// TODO
		BS_EXCEPT(NotImplementedException, "Not implemented");

		mState = State::RecordingRenderPass;
	}

	void VulkanCmdBuffer::endRenderPass()
	{
		assert(mState == State::RecordingRenderPass);

		vkCmdEndRenderPass(mCmdBuffer);

		mState = State::Recording;
	}

	void VulkanCmdBuffer::refreshFenceStatus()
	{
		VkResult result = vkGetFenceStatus(mDevice.getLogical(), mFence);
		assert(result == VK_SUCCESS || result == VK_NOT_READY);

		bool signaled = result == VK_SUCCESS;

		if (mState == State::Submitted)
		{
			if(signaled)
			{
				mState = State::Ready;
				vkResetCommandBuffer(mCmdBuffer, VK_COMMAND_BUFFER_RESET_RELEASE_RESOURCES_BIT); // Note: Maybe better not to release resources?

				result = vkResetFences(mDevice.getLogical(), 1, &mFence);
				assert(result == VK_SUCCESS);

				mFenceCounter++;

				for (auto& entry : mResources)
				{
					ResourceUseHandle& useHandle = entry.second;
					assert(useHandle.used);

					entry.first->notifyDone();
				}

				for (auto& entry : mImages)
				{
					ResourceUseHandle& useHandle = entry.second.useHandle;
					assert(useHandle.used);

					entry.first->notifyDone();
				}

				for (auto& entry : mBuffers)
				{
					ResourceUseHandle& useHandle = entry.second.useHandle;
					assert(useHandle.used);

					entry.first->notifyDone();
				}

				mResources.clear();
				mImages.clear();
				mBuffers.clear();
			}
		}
		else
			assert(!signaled); // We reset the fence along with mState so this shouldn't be possible
	}

	void VulkanCmdBuffer::registerResource(VulkanResource* res, VulkanUseFlags flags)
	{
		auto insertResult = mResources.insert(std::make_pair(res, ResourceUseHandle()));
		if(insertResult.second) // New element
		{
			ResourceUseHandle& useHandle = insertResult.first->second;
			useHandle.used = false;
			useHandle.flags = flags;

			res->notifyBound();
		}
		else // Existing element
		{
			ResourceUseHandle& useHandle = insertResult.first->second;

			assert(!useHandle.used);
			useHandle.flags |= flags;
		}
	}

	void VulkanCmdBuffer::registerResource(VulkanImage* res, VkAccessFlags accessFlags, VkImageLayout layout, 
		const VkImageSubresourceRange& range, VulkanUseFlags flags)
	{
		auto insertResult = mImages.insert(std::make_pair(res, ImageInfo()));
		if (insertResult.second) // New element
		{
			ImageInfo& imageInfo = insertResult.first->second;
			imageInfo.accessFlags = accessFlags;
			imageInfo.layout = layout;
			imageInfo.range = range;

			imageInfo.useHandle.used = false;
			imageInfo.useHandle.flags = flags;

			res->notifyBound();
		}
		else // Existing element
		{
			ImageInfo& imageInfo = insertResult.first->second;

			assert(!imageInfo.useHandle.used);
			imageInfo.useHandle.flags |= flags;

			assert(imageInfo.layout == layout && "Cannot bind the same image with two different layouts on the same command buffer.");
			imageInfo.accessFlags |= accessFlags;
			imageInfo.range = range;
		}
	}

	void VulkanCmdBuffer::registerResource(VulkanBuffer* res, VkAccessFlags accessFlags, VulkanUseFlags flags)
	{
		auto insertResult = mBuffers.insert(std::make_pair(res, BufferInfo()));
		if (insertResult.second) // New element
		{
			BufferInfo& bufferInfo = insertResult.first->second;
			bufferInfo.accessFlags = accessFlags;

			bufferInfo.useHandle.used = false;
			bufferInfo.useHandle.flags = flags;

			res->notifyBound();
		}
		else // Existing element
		{
			BufferInfo& bufferInfo = insertResult.first->second;

			assert(!bufferInfo.useHandle.used);
			bufferInfo.useHandle.flags |= flags;
			bufferInfo.accessFlags |= accessFlags;
		}
	}

	void VulkanCmdBuffer::submit(VulkanQueue* queue, UINT32 queueIdx, UINT32 syncMask)
	{
		assert(isReadyForSubmit());

		// Issue pipeline barriers for queue transitions (need to happen on original queue first, then on new queue)
		for(auto& entry : mBuffers)
		{
			VulkanBuffer* resource = static_cast<VulkanBuffer*>(entry.first);

			if (!resource->isExclusive())
				continue;

			UINT32 currentQueueFamily = resource->getQueueFamily();
			if (currentQueueFamily != mQueueFamily)
			{
				Vector<VkBufferMemoryBarrier>& barriers = mTransitionInfoTemp[currentQueueFamily].bufferBarriers;

				barriers.push_back(VkBufferMemoryBarrier());
				VkBufferMemoryBarrier& barrier = barriers.back();
				barrier.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
				barrier.pNext = nullptr;
				barrier.srcAccessMask = entry.second.accessFlags;
				barrier.dstAccessMask = entry.second.accessFlags;
				barrier.srcQueueFamilyIndex = currentQueueFamily;
				barrier.dstQueueFamilyIndex = mQueueFamily;
				barrier.buffer = resource->getHandle();
				barrier.offset = 0;
				barrier.size = VK_WHOLE_SIZE;
			}
		}

		for(auto& entry : mImages)
		{
			VulkanImage* resource = static_cast<VulkanImage*>(entry.first);

			UINT32 currentQueueFamily = resource->getQueueFamily();
			bool queueMismatch = resource->isExclusive() && currentQueueFamily != mQueueFamily;

			if (queueMismatch || resource->getLayout() != entry.second.layout)
			{
				Vector<VkImageMemoryBarrier>& barriers = mTransitionInfoTemp[currentQueueFamily].imageBarriers;

				barriers.push_back(VkImageMemoryBarrier());
				VkImageMemoryBarrier& barrier = barriers.back();
				barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
				barrier.pNext = nullptr;
				barrier.srcAccessMask = entry.second.accessFlags;
				barrier.dstAccessMask = entry.second.accessFlags;
				barrier.srcQueueFamilyIndex = currentQueueFamily;
				barrier.dstQueueFamilyIndex = mQueueFamily;
				barrier.oldLayout = resource->getLayout();
				barrier.newLayout = entry.second.layout;
				barrier.image = resource->getHandle();
				barrier.subresourceRange = entry.second.range;

				resource->setLayout(entry.second.layout);
			}
		}

		VulkanDevice& device = queue->getDevice();
		for (auto& entry : mTransitionInfoTemp)
		{
			bool empty = entry.second.imageBarriers.size() == 0 && entry.second.bufferBarriers.size() == 0;
			if (empty)
				continue;

			UINT32 entryQueueFamily = entry.first;

			// No queue transition needed for entries on this queue (this entry is most likely an in image layout transition)
			if (entryQueueFamily == mQueueFamily)
				continue;

			VulkanCmdBuffer* cmdBuffer = device.getCmdBufferPool().getBuffer(entryQueueFamily, false);
			VkCommandBuffer vkCmdBuffer = cmdBuffer->getHandle();

			TransitionInfo& barriers = entry.second;
			UINT32 numImgBarriers = (UINT32)barriers.imageBarriers.size();
			UINT32 numBufferBarriers = (UINT32)barriers.bufferBarriers.size();

			vkCmdPipelineBarrier(vkCmdBuffer,
								 VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
								 VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
								 0, 0, nullptr,
								 numBufferBarriers, barriers.bufferBarriers.data(),
								 numImgBarriers, barriers.imageBarriers.data());

			// Find an appropriate queue to execute on
			UINT32 otherQueueIdx = 0;
			VulkanQueue* otherQueue = nullptr;
			GpuQueueType otherQueueType = GQT_GRAPHICS;
			for (UINT32 i = 0; i < GQT_COUNT; i++)
			{
				if (device.getQueueFamily((GpuQueueType)i) != entryQueueFamily)
					continue;

				UINT32 numQueues = device.getNumQueues(otherQueueType);
				for(UINT32 j = 0; j < numQueues; j++)
				{
					// Try to find a queue not currently executing
					VulkanQueue* curQueue = device.getQueue(otherQueueType, j);
					if(!curQueue->isExecuting())
					{
						otherQueue = curQueue;
						otherQueueIdx = j;
					}
				}

				// Can't find empty one, use the first one then
				if(otherQueue == nullptr)
				{
					otherQueue = device.getQueue(otherQueueType, 0);
					otherQueueIdx = 0;
				}

				otherQueueType = (GpuQueueType)i;
				break;
			}

			UINT32 otherGlobalQueueIdx = CommandSyncMask::getGlobalQueueIdx(otherQueueType, otherQueueIdx);
			syncMask |= otherGlobalQueueIdx;

			cmdBuffer->end();
			cmdBuffer->submit(otherQueue, otherQueueIdx, 0);

			// If there are any layout transitions, reset them as we don't need them for the second pipeline barrier
			for (auto& barrierEntry : barriers.imageBarriers)
				barrierEntry.oldLayout = barrierEntry.newLayout;
		}

		UINT32 deviceIdx = device.getIndex();
		VulkanCommandBufferManager& cbm = static_cast<VulkanCommandBufferManager&>(CommandBufferManager::instance());

		UINT32 numSemaphores;
		cbm.getSyncSemaphores(deviceIdx, syncMask, mSemaphoresTemp, numSemaphores);

		// Issue second part of transition pipeline barriers (on this queue)
		for (auto& entry : mTransitionInfoTemp)
		{
			bool empty = entry.second.imageBarriers.size() == 0 && entry.second.bufferBarriers.size() == 0;
			if (empty)
				continue;

			VulkanCmdBuffer* cmdBuffer = device.getCmdBufferPool().getBuffer(mQueueFamily, false);
			VkCommandBuffer vkCmdBuffer = cmdBuffer->getHandle();

			TransitionInfo& barriers = entry.second;
			UINT32 numImgBarriers = (UINT32)barriers.imageBarriers.size();
			UINT32 numBufferBarriers = (UINT32)barriers.bufferBarriers.size();

			vkCmdPipelineBarrier(vkCmdBuffer,
								 VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
								 VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
								 0, 0, nullptr,
								 numBufferBarriers, barriers.bufferBarriers.data(),
								 numImgBarriers, barriers.imageBarriers.data());

			cmdBuffer->end();

			queue->submit(cmdBuffer, mSemaphoresTemp, numSemaphores);
			numSemaphores = 0; // Semaphores are only needed the first time, since we're adding the buffers on the same queue
		}

		queue->submit(this, mSemaphoresTemp, numSemaphores);

		for (auto& entry : mResources)
		{
			ResourceUseHandle& useHandle = entry.second;
			assert(!useHandle.used);

			useHandle.used = true;
			entry.first->notifyUsed(this, useHandle.flags);
		}

		for (auto& entry : mImages)
		{
			ResourceUseHandle& useHandle = entry.second.useHandle;
			assert(!useHandle.used);

			useHandle.used = true;
			entry.first->notifyUsed(this, useHandle.flags);
		}

		for (auto& entry : mBuffers)
		{
			ResourceUseHandle& useHandle = entry.second.useHandle;
			assert(!useHandle.used);

			useHandle.used = true;
			entry.first->notifyUsed(this, useHandle.flags);
		}

		cbm.refreshStates(deviceIdx);

		// Note: Uncommented for debugging only, prevents any device concurrency issues.
		// vkQueueWaitIdle(queue->getHandle());

		mState = State::Submitted;
		cbm.setActiveBuffer(queue->getType(), deviceIdx, queueIdx, this);

		// Clear vectors but don't clear the actual map, as we want to re-use the memory since we expect queue family
		// indices to be the same
		for (auto& entry : mTransitionInfoTemp)
		{
			entry.second.imageBarriers.clear();
			entry.second.bufferBarriers.clear();
		}
	}

	VulkanCommandBuffer::VulkanCommandBuffer(VulkanDevice& device, GpuQueueType type, UINT32 deviceIdx,
		UINT32 queueIdx, bool secondary)
		: CommandBuffer(type, deviceIdx, queueIdx, secondary), mBuffer(nullptr)
		, mDevice(device), mQueue(nullptr), mIdMask(0)
	{
		UINT32 numQueues = device.getNumQueues(mType);
		if (numQueues == 0) // Fall back to graphics queue
		{
			mType = GQT_GRAPHICS;
			numQueues = device.getNumQueues(GQT_GRAPHICS);
		}

		mQueue = device.getQueue(mType, mQueueIdx % numQueues);
		mIdMask = device.getQueueMask(mType, mQueueIdx);

		acquireNewBuffer();
	}

	void VulkanCommandBuffer::acquireNewBuffer()
	{
		VulkanCmdBufferPool& pool = mDevice.getCmdBufferPool();

		if (mBuffer != nullptr)
			assert(mBuffer->isSubmitted());

		UINT32 queueFamily = mDevice.getQueueFamily(mType);
		mBuffer = pool.getBuffer(queueFamily, mIsSecondary);
	}

	void VulkanCommandBuffer::submit(UINT32 syncMask)
	{
		// Ignore myself
		syncMask &= ~mIdMask;

		mBuffer->submit(mQueue, mQueueIdx, syncMask);
		acquireNewBuffer();
	}
}