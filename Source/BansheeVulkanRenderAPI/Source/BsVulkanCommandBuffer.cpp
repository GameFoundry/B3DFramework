//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsVulkanCommandBuffer.h"
#include "BsVulkanUtility.h"
#include "BsVulkanDevice.h"

namespace BansheeEngine
{
	VulkanCmdBufferPool::VulkanCmdBufferPool(VulkanDevice& device)
		:mDevice(device), mPools{}, mBuffers {}
	{
		for (UINT32 i = 0; i < VQT_COUNT; i++)
		{
			UINT32 familyIdx = device.getQueueFamily((VulkanQueueType)i);

			if (familyIdx == (UINT32)-1)
				continue;

			VkCommandPoolCreateInfo poolInfo;
			poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
			poolInfo.pNext = nullptr;
			poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
			poolInfo.queueFamilyIndex = familyIdx;

			vkCreateCommandPool(device.getLogical(), &poolInfo, gVulkanAllocator, &mPools[i]);
		}
	}

	VulkanCmdBufferPool::~VulkanCmdBufferPool()
	{
		for (UINT32 i = 0; i < VQT_COUNT; i++)
		{
			for(UINT32 j = 0; j < BS_MAX_QUEUES_PER_TYPE; j++)
			{
				LVulkanCommandBuffer** buffers = mBuffers[i][j];
				for(UINT32 k = 0; k < BS_MAX_VULKAN_COMMAND_BUFFERS_PER_QUEUE; k++)
				{
					if (buffers[k] == nullptr)
						break;

					bs_delete(buffers[k]);
				}
			}
		}

		for (UINT32 i = 0; i < VQT_COUNT; i++)
		{
			if (mPools[i] == VK_NULL_HANDLE)
				continue;

			vkDestroyCommandPool(mDevice.getLogical(), mPools[i], gVulkanAllocator);
		}
	}

	LVulkanCommandBuffer* VulkanCmdBufferPool::getBuffer(CommandBufferType type, UINT32 queueIdx, bool secondary)
	{
		assert(queueIdx < BS_MAX_QUEUES_PER_TYPE);

		VulkanQueueType queueType = VulkanUtility::getQueueType(type);
		LVulkanCommandBuffer** buffers = mBuffers[queueType][queueIdx];

		UINT32 i = 0;
		for(; i < BS_MAX_VULKAN_COMMAND_BUFFERS_PER_QUEUE; i++)
		{
			if (buffers[i] == nullptr)
				break;

			if(buffers[i]->mState == LVulkanCommandBuffer::State::Ready)
			{
				buffers[i]->begin();
				return buffers[i];
			}
		}

		assert(i < BS_MAX_VULKAN_COMMAND_BUFFERS_PER_QUEUE && 
			"Too many command buffers allocated. Increment BS_MAX_VULKAN_COMMAND_BUFFERS_PER_QUEUE to a higher value. ");

		buffers[i] = createBuffer(queueType, secondary);
		buffers[i]->begin();

		return buffers[i];
	}

	LVulkanCommandBuffer* VulkanCmdBufferPool::createBuffer(VulkanQueueType type, bool secondary)
	{
		VkCommandPool pool = getPool(type);

		return bs_new<LVulkanCommandBuffer>(mDevice, pool, secondary);
	}

	VkCommandPool VulkanCmdBufferPool::getPool(VulkanQueueType type)
	{
		VkCommandPool pool = mPools[type];
		if (pool == VK_NULL_HANDLE)
			pool = mPools[VQT_GRAPHICS]; // Graphics queue is guaranteed to exist

		return pool;
	}

	LVulkanCommandBuffer::LVulkanCommandBuffer(VulkanDevice& device, VkCommandPool pool, bool secondary)
		:mState(State::Ready), mDevice(device), mPool(pool)
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

	LVulkanCommandBuffer::~LVulkanCommandBuffer()
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
		}
		
		vkDestroyFence(device, mFence, gVulkanAllocator);
		vkDestroySemaphore(device, mSemaphore, gVulkanAllocator);
		vkFreeCommandBuffers(device, mPool, 1, &mCmdBuffer);
	}

	void LVulkanCommandBuffer::begin()
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

	void LVulkanCommandBuffer::end()
	{
		assert(mState == State::Recording);

		VkResult result = vkEndCommandBuffer(mCmdBuffer);
		assert(result == VK_SUCCESS);

		mState = State::RecordingDone;
	}

	void LVulkanCommandBuffer::beginRenderPass()
	{
		assert(mState == State::Recording);

		// TODO
		BS_EXCEPT(NotImplementedException, "Not implemented");

		mState = State::RecordingRenderPass;
	}

	void LVulkanCommandBuffer::endRenderPass()
	{
		assert(mState == State::RecordingRenderPass);

		vkCmdEndRenderPass(mCmdBuffer);

		mState = State::Recording;
	}

	void LVulkanCommandBuffer::refreshFenceStatus()
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
			}
		}
		else
			assert(!signaled); // We reset the fence along with mState so this shouldn't be possible

	}

	VulkanCommandBuffer::VulkanCommandBuffer(VulkanDevice& device, UINT32 id, CommandBufferType type, 
		UINT32 queueIdx, bool secondary)
		: CommandBuffer(id, type, queueIdx, secondary), mBuffer(nullptr), mDevice(device)
	{
		acquireNewBuffer();
	}

	void VulkanCommandBuffer::acquireNewBuffer()
	{
		VulkanCmdBufferPool& pool = mDevice.getCmdBufferPool();

		mBuffer = pool.getBuffer(mType, mQueueIdx, mIsSecondary);
	}
}