//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsVulkanCommandBuffer.h"
#include "BsVulkanCommandBufferManager.h"
#include "BsVulkanUtility.h"
#include "BsVulkanDevice.h"
#include "BsVulkanGpuParams.h"
#include "BsVulkanQueue.h"
#include "BsVulkanTexture.h"
#include "BsVulkanIndexBuffer.h"
#include "BsVulkanVertexBuffer.h"
#include "BsVulkanHardwareBuffer.h"
#include "BsVulkanFramebuffer.h"
#include "BsVulkanVertexInputManager.h"
#include "BsVulkanEventQuery.h"
#include "BsVulkanQueryManager.h"

#if BS_PLATFORM == BS_PLATFORM_WIN32
#include "Win32/BsWin32RenderWindow.h"
#else
static_assert(false, "Other platforms go here");
#endif

namespace bs
{
	VulkanSemaphore::VulkanSemaphore(VulkanResourceManager* owner)
		:VulkanResource(owner, true)
	{
		VkSemaphoreCreateInfo semaphoreCI;
		semaphoreCI.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
		semaphoreCI.pNext = nullptr;
		semaphoreCI.flags = 0;

		VkResult result = vkCreateSemaphore(owner->getDevice().getLogical(), &semaphoreCI, gVulkanAllocator, &mSemaphore);
		assert(result == VK_SUCCESS);
	}

	VulkanSemaphore::~VulkanSemaphore()
	{
		vkDestroySemaphore(mOwner->getDevice().getLogical(), mSemaphore, gVulkanAllocator);
	}

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
		if (iterFind == mPools.end())
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
		if (iterFind == mPools.end())
			return nullptr;

		const PoolInfo& poolInfo = iterFind->second;

		return bs_new<VulkanCmdBuffer>(mDevice, mNextId++, poolInfo.pool, poolInfo.queueFamily, secondary);
	}

	VulkanCmdBuffer::VulkanCmdBuffer(VulkanDevice& device, UINT32 id, VkCommandPool pool, UINT32 queueFamily, bool secondary)
		: mId(id), mQueueFamily(queueFamily), mState(State::Ready), mDevice(device), mPool(pool)
		, mIntraQueueSemaphore(nullptr), mInterQueueSemaphores(), mNumUsedInterQueueSemaphores(0)
		, mFramebuffer(nullptr), mRenderTargetWidth(0)
		, mRenderTargetHeight(0), mRenderTargetDepthReadOnly(false), mRenderTargetLoadMask(RT_NONE), mGlobalQueueIdx(-1)
		, mViewport(0.0f, 0.0f, 1.0f, 1.0f), mScissor(0, 0, 0, 0), mStencilRef(0), mDrawOp(DOT_TRIANGLE_LIST)
		, mNumBoundDescriptorSets(0), mGfxPipelineRequiresBind(true), mCmpPipelineRequiresBind(true)
		, mViewportRequiresBind(true), mStencilRefRequiresBind(true), mScissorRequiresBind(true), mBoundParamsDirty(false)
		, mClearValues(), mClearMask(), mVertexBuffersTemp(), mVertexBufferOffsetsTemp()
	{
		UINT32 maxBoundDescriptorSets = device.getDeviceProperties().limits.maxBoundDescriptorSets;
		mDescriptorSetsTemp = (VkDescriptorSet*)bs_alloc(sizeof(VkDescriptorSet) * maxBoundDescriptorSets);

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
			reset();
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
				UINT32 imageInfoIdx = entry.second;
				ImageInfo& imageInfo = mImageInfos[imageInfoIdx];

				ResourceUseHandle& useHandle = imageInfo.useHandle;
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

		if (mIntraQueueSemaphore != nullptr)
			mIntraQueueSemaphore->destroy();
		
		for(UINT32 i = 0; i < BS_MAX_VULKAN_CB_DEPENDENCIES; i++)
		{
			if (mInterQueueSemaphores[i] != nullptr)
				mInterQueueSemaphores[i]->destroy();
		}

		vkDestroyFence(device, mFence, gVulkanAllocator);
		vkFreeCommandBuffers(device, mPool, 1, &mCmdBuffer);

		bs_free(mDescriptorSetsTemp);
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

		// If a clear is queued, execute the render pass with no additional instructions
		if (mClearMask)
			executeClearPass();

		VkResult result = vkEndCommandBuffer(mCmdBuffer);
		assert(result == VK_SUCCESS);

		mState = State::RecordingDone;
	}

	void VulkanCmdBuffer::beginRenderPass()
	{
		assert(mState == State::Recording);

		if (mFramebuffer == nullptr)
		{
			LOGWRN("Attempting to begin a render pass but no render target is bound to the command buffer.");
			return;
		}

		if(mClearMask != CLEAR_NONE)
		{
			// If a previous clear is queued, but it doesn't match the rendered area, need to execute a separate pass
			// just for it
			Rect2I rtArea(0, 0, mRenderTargetWidth, mRenderTargetHeight);
			if (mClearArea != rtArea)
				executeClearPass();
		}

		executeLayoutTransitions();

		// Check if any frame-buffer attachments are also used as shader inputs, in which case we make them read-only
		RenderSurfaceMask readMask = RT_NONE;

		UINT32 numColorAttachments = mFramebuffer->getNumColorAttachments();
		for(UINT32 i = 0; i < numColorAttachments; i++)
		{
			VulkanImage* image = mFramebuffer->getColorAttachment(i).image;

			UINT32 imageInfoIdx = mImages[image];
			ImageInfo& imageInfo = mImageInfos[imageInfoIdx];

			bool readOnly = imageInfo.isShaderInput;

			if(readOnly)
				readMask.set((RenderSurfaceMaskBits)(1 << i));
		}

		if(mFramebuffer->hasDepthAttachment())
		{
			VulkanImage* image = mFramebuffer->getDepthStencilAttachment().image;

			UINT32 imageInfoIdx = mImages[image];
			ImageInfo& imageInfo = mImageInfos[imageInfoIdx];

			bool readOnly = imageInfo.isShaderInput;

			if (readOnly)
				readMask.set(RT_DEPTH);
		}

		// Reset flags that signal image usage (since those only matter for the render-pass' purposes)
		for (auto& entry : mImages)
		{
			UINT32 imageInfoIdx = entry.second;
			ImageInfo& imageInfo = mImageInfos[imageInfoIdx];

			imageInfo.isFBAttachment = false;
			imageInfo.isShaderInput = false;
		}

		VkRenderPassBeginInfo renderPassBeginInfo;
		renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassBeginInfo.pNext = nullptr;
		renderPassBeginInfo.framebuffer = mFramebuffer->getFramebuffer(mRenderTargetLoadMask, readMask, mClearMask);
		renderPassBeginInfo.renderPass = mFramebuffer->getRenderPass(mRenderTargetLoadMask, readMask, mClearMask);
		renderPassBeginInfo.renderArea.offset.x = 0;
		renderPassBeginInfo.renderArea.offset.y = 0;
		renderPassBeginInfo.renderArea.extent.width = mRenderTargetWidth;
		renderPassBeginInfo.renderArea.extent.height = mRenderTargetHeight;
		renderPassBeginInfo.clearValueCount = mFramebuffer->getNumAttachments();
		renderPassBeginInfo.pClearValues = mClearValues.data();

		vkCmdBeginRenderPass(mCmdBuffer, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

		mClearMask = CLEAR_NONE;
		mState = State::RecordingRenderPass;
	}

	void VulkanCmdBuffer::endRenderPass()
	{
		assert(mState == State::RecordingRenderPass);

		vkCmdEndRenderPass(mCmdBuffer);

		// Execute any queued events
		for(auto& entry : mQueuedEvents)
			vkCmdSetEvent(mCmdBuffer, entry->getHandle(), VK_PIPELINE_STAGE_ALL_COMMANDS_BIT);

		mQueuedEvents.clear();

		// Update any layout transitions that were performed by subpass dependencies, reset flags that signal image usage
		// and reset access flags
		for (auto& entry : mImages)
		{
			UINT32 imageInfoIdx = entry.second;
			ImageInfo& imageInfo = mImageInfos[imageInfoIdx];

			imageInfo.currentLayout = imageInfo.finalLayout;
			imageInfo.requiredLayout = imageInfo.finalLayout;

			imageInfo.isFBAttachment = false;
			imageInfo.isShaderInput = false;

			imageInfo.accessFlags = 0;
		}

		mState = State::Recording;
	}

	void VulkanCmdBuffer::allocateSemaphores()
	{
		if (mIntraQueueSemaphore != nullptr)
			mIntraQueueSemaphore->destroy();

		mIntraQueueSemaphore = mDevice.getResourceManager().create<VulkanSemaphore>();
		
		for (UINT32 i = 0; i < BS_MAX_VULKAN_CB_DEPENDENCIES; i++)
		{
			if (mInterQueueSemaphores[i] != nullptr)
				mInterQueueSemaphores[i]->destroy();

			mInterQueueSemaphores[i] = mDevice.getResourceManager().create<VulkanSemaphore>();
		}

		mNumUsedInterQueueSemaphores = 0;
	}

	VulkanSemaphore* VulkanCmdBuffer::requestInterQueueSemaphore() const
	{
		if (mNumUsedInterQueueSemaphores >= BS_MAX_VULKAN_CB_DEPENDENCIES)
			return nullptr;

		return mInterQueueSemaphores[mNumUsedInterQueueSemaphores++];
	}

	void VulkanCmdBuffer::submit(VulkanQueue* queue, UINT32 queueIdx, UINT32 syncMask)
	{
		assert(isReadyForSubmit());

		// Make sure to reset the CB fence before we submit it
		VkResult result = vkResetFences(mDevice.getLogical(), 1, &mFence);
		assert(result == VK_SUCCESS);

		// If there are any query resets needed, execute those first
		VulkanDevice& device = queue->getDevice();
		if(!mQueuedQueryResets.empty())
		{
			VulkanCmdBuffer* cmdBuffer = device.getCmdBufferPool().getBuffer(mQueueFamily, false);
			VkCommandBuffer vkCmdBuffer = cmdBuffer->getHandle();

			for (auto& entry : mQueuedQueryResets)
				entry->reset(vkCmdBuffer);

			cmdBuffer->end();
			queue->queueSubmit(cmdBuffer, nullptr, 0);

			mQueuedQueryResets.clear();
		}

		// Issue pipeline barriers for queue transitions (need to happen on original queue first, then on new queue)
		for (auto& entry : mBuffers)
		{
			VulkanBuffer* resource = static_cast<VulkanBuffer*>(entry.first);

			if (!resource->isExclusive())
				continue;

			UINT32 currentQueueFamily = resource->getQueueFamily();
			if (currentQueueFamily != -1 && currentQueueFamily != mQueueFamily)
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

		for (auto& entry : mImages)
		{
			VulkanImage* resource = static_cast<VulkanImage*>(entry.first);
			ImageInfo& imageInfo = mImageInfos[entry.second];

			UINT32 currentQueueFamily = resource->getQueueFamily();
			bool queueMismatch = resource->isExclusive() && currentQueueFamily != -1 && currentQueueFamily != mQueueFamily;

			if (queueMismatch)
			{
				Vector<VkImageMemoryBarrier>& barriers = mTransitionInfoTemp[currentQueueFamily].imageBarriers;

				barriers.push_back(VkImageMemoryBarrier());
				VkImageMemoryBarrier& barrier = barriers.back();
				barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
				barrier.pNext = nullptr;
				barrier.srcAccessMask = resource->getAccessFlags(imageInfo.currentLayout);
				barrier.dstAccessMask = imageInfo.accessFlags;
				barrier.oldLayout = imageInfo.currentLayout;
				barrier.newLayout = imageInfo.currentLayout;
				barrier.image = resource->getHandle();
				barrier.subresourceRange = imageInfo.range;
				barrier.srcQueueFamilyIndex = currentQueueFamily;
				barrier.dstQueueFamilyIndex = mQueueFamily;
			}
		}

		for (auto& entry : mTransitionInfoTemp)
		{
			bool empty = entry.second.imageBarriers.size() == 0 && entry.second.bufferBarriers.size() == 0;
			if (empty)
				continue;

			UINT32 entryQueueFamily = entry.first;

			// No queue transition needed for entries on this queue 
			if (entryQueueFamily == -1 || entryQueueFamily == mQueueFamily)
				continue;

			VulkanCmdBuffer* cmdBuffer = device.getCmdBufferPool().getBuffer(entryQueueFamily, false);
			VkCommandBuffer vkCmdBuffer = cmdBuffer->getHandle();

			TransitionInfo& barriers = entry.second;
			UINT32 numImgBarriers = (UINT32)barriers.imageBarriers.size();
			UINT32 numBufferBarriers = (UINT32)barriers.bufferBarriers.size();

			vkCmdPipelineBarrier(vkCmdBuffer,
								 VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, // Note: VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT might be more correct here, according to the spec
								 VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, //       The main idea is that the barrier executes before the semaphore triggers, no actual stage dependencies are needed.
								 0, 0, nullptr,
								 numBufferBarriers, barriers.bufferBarriers.data(),
								 numImgBarriers, barriers.imageBarriers.data());

			// Find an appropriate queue to execute on
			UINT32 otherQueueIdx = 0;
			VulkanQueue* otherQueue = nullptr;
			GpuQueueType otherQueueType = GQT_GRAPHICS;
			for (UINT32 i = 0; i < GQT_COUNT; i++)
			{
				otherQueueType = (GpuQueueType)i;
				if (device.getQueueFamily(otherQueueType) != entryQueueFamily)
					continue;

				UINT32 numQueues = device.getNumQueues(otherQueueType);
				for (UINT32 j = 0; j < numQueues; j++)
				{
					// Try to find a queue not currently executing
					VulkanQueue* curQueue = device.getQueue(otherQueueType, j);
					if (!curQueue->isExecuting())
					{
						otherQueue = curQueue;
						otherQueueIdx = j;
					}
				}

				// Can't find empty one, use the first one then
				if (otherQueue == nullptr)
				{
					otherQueue = device.getQueue(otherQueueType, 0);
					otherQueueIdx = 0;
				}

				break;
			}

			syncMask |= CommandSyncMask::getGlobalQueueMask(otherQueueType, otherQueueIdx);

			cmdBuffer->end();
			otherQueue->submit(cmdBuffer, nullptr, 0);
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
								 VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, // Note: VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT might be more correct here, according to the spec
								 VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, 
								 0, 0, nullptr,
								 numBufferBarriers, barriers.bufferBarriers.data(),
								 numImgBarriers, barriers.imageBarriers.data());

			cmdBuffer->end();
			queue->queueSubmit(cmdBuffer, mSemaphoresTemp, numSemaphores);

			numSemaphores = 0; // Semaphores are only needed the first time, since we're adding the buffers on the same queue
		}

		queue->queueSubmit(this, mSemaphoresTemp, numSemaphores);
		queue->submitQueued();

		mGlobalQueueIdx = CommandSyncMask::getGlobalQueueIdx(queue->getType(), queueIdx);
		for (auto& entry : mResources)
		{
			ResourceUseHandle& useHandle = entry.second;
			assert(!useHandle.used);

			useHandle.used = true;
			entry.first->notifyUsed(mGlobalQueueIdx, mQueueFamily, useHandle.flags);
		}

		for (auto& entry : mImages)
		{
			UINT32 imageInfoIdx = entry.second;
			ImageInfo& imageInfo = mImageInfos[imageInfoIdx];

			ResourceUseHandle& useHandle = imageInfo.useHandle;
			assert(!useHandle.used);

			useHandle.used = true;
			entry.first->notifyUsed(mGlobalQueueIdx, mQueueFamily, useHandle.flags);
		}

		for (auto& entry : mBuffers)
		{
			ResourceUseHandle& useHandle = entry.second.useHandle;
			assert(!useHandle.used);

			useHandle.used = true;
			entry.first->notifyUsed(mGlobalQueueIdx, mQueueFamily, useHandle.flags);
		}

		// Note: Uncomment for debugging only, prevents any device concurrency issues.
		// vkQueueWaitIdle(queue->getHandle());

		// Clear vectors but don't clear the actual map, as we want to re-use the memory since we expect queue family
		// indices to be the same
		for (auto& entry : mTransitionInfoTemp)
		{
			entry.second.imageBarriers.clear();
			entry.second.bufferBarriers.clear();
		}

		mGraphicsPipeline = nullptr;
		mComputePipeline = nullptr;
		mGfxPipelineRequiresBind = true;
		mCmpPipelineRequiresBind = true;
		mFramebuffer = nullptr;
		mDescriptorSetsBindState = DescriptorSetBindFlag::Graphics | DescriptorSetBindFlag::Compute;
		mQueuedLayoutTransitions.clear();
		mBoundParams = nullptr;
		mBoundParams = false;
	}

	bool VulkanCmdBuffer::checkFenceStatus() const
	{
		VkResult result = vkGetFenceStatus(mDevice.getLogical(), mFence);
		assert(result == VK_SUCCESS || result == VK_NOT_READY);

		return result == VK_SUCCESS;
	}

	void VulkanCmdBuffer::reset()
	{
		if (mState != State::Submitted)
			return;

		mState = State::Ready;
		vkResetCommandBuffer(mCmdBuffer, VK_COMMAND_BUFFER_RESET_RELEASE_RESOURCES_BIT); // Note: Maybe better not to release resources?

		for (auto& entry : mResources)
		{
			ResourceUseHandle& useHandle = entry.second;
			assert(useHandle.used);

			entry.first->notifyDone(mGlobalQueueIdx, useHandle.flags);
		}

		for (auto& entry : mImages)
		{
			UINT32 imageInfoIdx = entry.second;
			ImageInfo& imageInfo = mImageInfos[imageInfoIdx];

			ResourceUseHandle& useHandle = imageInfo.useHandle;
			assert(useHandle.used);

			entry.first->notifyDone(mGlobalQueueIdx, useHandle.flags);
		}

		for (auto& entry : mBuffers)
		{
			ResourceUseHandle& useHandle = entry.second.useHandle;
			assert(useHandle.used);

			entry.first->notifyDone(mGlobalQueueIdx, useHandle.flags);
		}

		mResources.clear();
		mImages.clear();
		mBuffers.clear();
		mImageInfos.clear();
	}

	void VulkanCmdBuffer::setRenderTarget(const SPtr<RenderTargetCore>& rt, bool readOnlyDepthStencil, 
		RenderSurfaceMask loadMask)
	{
		assert(mState != State::Submitted);

		VulkanFramebuffer* newFB;
		if(rt != nullptr)
		{
			if (rt->getProperties().isWindow())
			{
				Win32RenderWindowCore* window = static_cast<Win32RenderWindowCore*>(rt.get());
				window->acquireBackBuffer();
			}

			rt->getCustomAttribute("FB", &newFB);
		}
		else
			newFB = nullptr;

		if (mFramebuffer == newFB && mRenderTargetDepthReadOnly == readOnlyDepthStencil && mRenderTargetLoadMask == loadMask)
			return;

		if (isInRenderPass())
			endRenderPass();
		else
		{
			// If a clear is queued for previous FB, execute the render pass with no additional instructions
			if (mClearMask)
				executeClearPass();
		}

		if(newFB == nullptr)
		{
			mFramebuffer = nullptr;
			mRenderTargetWidth = 0;
			mRenderTargetHeight = 0;
			mRenderTargetDepthReadOnly = false;
			mRenderTargetLoadMask = RT_NONE;
		}
		else
		{
			mFramebuffer = newFB;
			mRenderTargetWidth = rt->getProperties().getWidth();
			mRenderTargetHeight = rt->getProperties().getHeight();
			mRenderTargetDepthReadOnly = readOnlyDepthStencil;
			mRenderTargetLoadMask = loadMask;
		}

		// Reset flags that signal image usage
		for (auto& entry : mImages)
		{
			UINT32 imageInfoIdx = entry.second;
			ImageInfo& imageInfo = mImageInfos[imageInfoIdx];

			imageInfo.isFBAttachment = false;
		}

		setGpuParams(nullptr);

		if(mFramebuffer != nullptr)
			registerResource(mFramebuffer, loadMask, VulkanUseFlag::Write);

		mGfxPipelineRequiresBind = true;
	}

	void VulkanCmdBuffer::clearViewport(const Rect2I& area, UINT32 buffers, const Color& color, float depth, UINT16 stencil,
					   UINT8 targetMask)
	{
		if (buffers == 0 || mFramebuffer == nullptr)
			return;

		// Add clear command if currently in render pass
		if (isInRenderPass())
		{
			VkClearAttachment attachments[BS_MAX_MULTIPLE_RENDER_TARGETS + 1];
			UINT32 baseLayer = 0;

			UINT32 attachmentIdx = 0;
			if ((buffers & FBT_COLOR) != 0)
			{
				UINT32 numColorAttachments = mFramebuffer->getNumColorAttachments();
				for (UINT32 i = 0; i < numColorAttachments; i++)
				{
					const VulkanFramebufferAttachment& attachment = mFramebuffer->getColorAttachment(i);

					if (((1 << attachment.index) & targetMask) == 0)
						continue;

					attachments[attachmentIdx].aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
					attachments[attachmentIdx].colorAttachment = i;

					VkClearColorValue& colorValue = attachments[attachmentIdx].clearValue.color;
					colorValue.float32[0] = color.r;
					colorValue.float32[1] = color.g;
					colorValue.float32[2] = color.b;
					colorValue.float32[3] = color.a;

					UINT32 curBaseLayer = attachment.baseLayer;
					if (attachmentIdx == 0)
						baseLayer = curBaseLayer;
					else
					{
						if (baseLayer != curBaseLayer)
						{
							// Note: This could be supported relatively easily: we would need to issue multiple separate
							// clear commands for such framebuffers. 
							LOGERR("Attempting to clear a texture that has multiple multi-layer surfaces with mismatching "
								   "starting layers. This is currently not supported.");
						}
					}

					attachmentIdx++;
				}
			}

			if ((buffers & FBT_DEPTH) != 0 || (buffers & FBT_STENCIL) != 0)
			{
				if (mFramebuffer->hasDepthAttachment())
				{
					attachments[attachmentIdx].aspectMask = 0;

					if ((buffers & FBT_DEPTH) != 0)
					{
						attachments[attachmentIdx].aspectMask |= VK_IMAGE_ASPECT_DEPTH_BIT;
						attachments[attachmentIdx].clearValue.depthStencil.depth = depth;
					}

					if ((buffers & FBT_STENCIL) != 0)
					{
						attachments[attachmentIdx].aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
						attachments[attachmentIdx].clearValue.depthStencil.stencil = stencil;
					}

					attachments[attachmentIdx].colorAttachment = 0;

					UINT32 curBaseLayer = mFramebuffer->getDepthStencilAttachment().baseLayer;
					if (attachmentIdx == 0)
						baseLayer = curBaseLayer;
					else
					{
						if (baseLayer != curBaseLayer)
						{
							// Note: This could be supported relatively easily: we would need to issue multiple separate
							// clear commands for such framebuffers. 
							LOGERR("Attempting to clear a texture that has multiple multi-layer surfaces with mismatching "
								   "starting layers. This is currently not supported.");
						}
					}

					attachmentIdx++;
				}
			}

			UINT32 numAttachments = attachmentIdx;
			if (numAttachments == 0)
				return;

			VkClearRect clearRect;
			clearRect.baseArrayLayer = baseLayer;
			clearRect.layerCount = mFramebuffer->getNumLayers();
			clearRect.rect.offset.x = area.x;
			clearRect.rect.offset.y = area.y;
			clearRect.rect.extent.width = area.width;
			clearRect.rect.extent.height = area.height;

			vkCmdClearAttachments(mCmdBuffer, numAttachments, attachments, 1, &clearRect);
		}
		// Otherwise we use a render pass that performs a clear on begin
		else
		{
			ClearMask clearMask;
			std::array<VkClearValue, BS_MAX_MULTIPLE_RENDER_TARGETS + 1> clearValues = mClearValues;

			UINT32 numColorAttachments = mFramebuffer->getNumColorAttachments();
			if ((buffers & FBT_COLOR) != 0)
			{
				for (UINT32 i = 0; i < numColorAttachments; i++)
				{
					const VulkanFramebufferAttachment& attachment = mFramebuffer->getColorAttachment(i);

					if (((1 << attachment.index) & targetMask) == 0)
						continue;

					clearMask |= (ClearMaskBits)(1 << attachment.index);

					VkClearColorValue& colorValue = clearValues[i].color;
					colorValue.float32[0] = color.r;
					colorValue.float32[1] = color.g;
					colorValue.float32[2] = color.b;
					colorValue.float32[3] = color.a;
				}
			}

			if ((buffers & FBT_DEPTH) != 0 || (buffers & FBT_STENCIL) != 0)
			{
				if (mFramebuffer->hasDepthAttachment())
				{
					UINT32 depthAttachmentIdx = numColorAttachments;

					if ((buffers & FBT_DEPTH) != 0)
					{
						clearValues[depthAttachmentIdx].depthStencil.depth = depth;
						clearMask |= CLEAR_DEPTH;
					}

					if ((buffers & FBT_STENCIL) != 0)
					{
						clearValues[depthAttachmentIdx].depthStencil.stencil = stencil;
						clearMask |= CLEAR_STENCIL;
					}
				}
			}

			if (!clearMask)
				return;

			// Some previous clear operation is already queued, execute it first
			bool previousClearNeedsToFinish = (mClearMask & clearMask) != CLEAR_NONE;
			
			if(previousClearNeedsToFinish)
				executeClearPass();

			mClearMask |= clearMask;
			mClearValues = clearValues;
			mClearArea = area;
		}
	}

	void VulkanCmdBuffer::clearRenderTarget(UINT32 buffers, const Color& color, float depth, UINT16 stencil, UINT8 targetMask)
	{
		Rect2I area(0, 0, mRenderTargetWidth, mRenderTargetHeight);
		clearViewport(area, buffers, color, depth, stencil, targetMask);
	}

	void VulkanCmdBuffer::clearViewport(UINT32 buffers, const Color& color, float depth, UINT16 stencil, UINT8 targetMask)
	{
		Rect2I area;
		area.x = (UINT32)(mViewport.x * mRenderTargetWidth);
		area.y = (UINT32)(mViewport.y * mRenderTargetHeight);
		area.width = (UINT32)(mViewport.width * mRenderTargetWidth);
		area.height = (UINT32)(mViewport.height * mRenderTargetHeight);

		clearViewport(area, buffers, color, depth, stencil, targetMask);
	}

	void VulkanCmdBuffer::setPipelineState(const SPtr<GraphicsPipelineStateCore>& state)
	{
		if (mGraphicsPipeline == state)
			return;

		mGraphicsPipeline = std::static_pointer_cast<VulkanGraphicsPipelineStateCore>(state);
		mGfxPipelineRequiresBind = true; 
	}

	void VulkanCmdBuffer::setPipelineState(const SPtr<ComputePipelineStateCore>& state)
	{
		if (mComputePipeline == state)
			return;

		mComputePipeline = std::static_pointer_cast<VulkanComputePipelineStateCore>(state);
		mCmpPipelineRequiresBind = true;
	}

	void VulkanCmdBuffer::setGpuParams(const SPtr<GpuParamsCore>& gpuParams)
	{
		// Note: We keep an internal reference to GPU params even though we shouldn't keep a reference to a core thread
		// object. But it should be fine since we expect the resource to be externally synchronized so it should never
		// be allowed to go out of scope on a non-core thread anyway.

		mBoundParams = std::static_pointer_cast<VulkanGpuParams>(gpuParams);
		
		if (mBoundParams != nullptr)
			mBoundParamsDirty = true;
		else
		{
			mNumBoundDescriptorSets = 0;
			mBoundParamsDirty = false;
		}

		mDescriptorSetsBindState = DescriptorSetBindFlag::Graphics | DescriptorSetBindFlag::Compute;
	}

	void VulkanCmdBuffer::setViewport(const Rect2& area)
	{
		if (mViewport == area)
			return;

		mViewport = area;
		mViewportRequiresBind = true;
	}

	void VulkanCmdBuffer::setScissorRect(const Rect2I& value)
	{
		if (mScissor == value)
			return;

		mScissor = value;
		mScissorRequiresBind = true;
	}

	void VulkanCmdBuffer::setStencilRef(UINT32 value)
	{
		if (mStencilRef == value)
			return;

		mStencilRef = value;
		mStencilRefRequiresBind = true;
	}

	void VulkanCmdBuffer::setDrawOp(DrawOperationType drawOp)
	{
		if (mDrawOp == drawOp)
			return;

		mDrawOp = drawOp;
		mGfxPipelineRequiresBind = true;
	}

	void VulkanCmdBuffer::setVertexBuffers(UINT32 index, SPtr<VertexBufferCore>* buffers, UINT32 numBuffers)
	{
		if (numBuffers == 0)
			return;

		for(UINT32 i = 0; i < numBuffers; i++)
		{
			VulkanVertexBufferCore* vertexBuffer = static_cast<VulkanVertexBufferCore*>(buffers[i].get());

			if (vertexBuffer != nullptr)
			{
				VulkanBuffer* resource = vertexBuffer->getResource(mDevice.getIndex());
				if (resource != nullptr)
				{
					mVertexBuffersTemp[i] = resource->getHandle();

					registerResource(resource, VulkanUseFlag::Read);
				}
				else
					mVertexBuffersTemp[i] = VK_NULL_HANDLE;
			}
			else
				mVertexBuffersTemp[i] = VK_NULL_HANDLE;
		}

		vkCmdBindVertexBuffers(mCmdBuffer, index, numBuffers, mVertexBuffersTemp, mVertexBufferOffsetsTemp);
	}

	void VulkanCmdBuffer::setIndexBuffer(const SPtr<IndexBufferCore>& buffer)
	{
		VulkanIndexBufferCore* indexBuffer = static_cast<VulkanIndexBufferCore*>(buffer.get());

		VkBuffer vkBuffer = VK_NULL_HANDLE;
		VkIndexType indexType = VK_INDEX_TYPE_UINT32;
		if (indexBuffer != nullptr)
		{
			VulkanBuffer* resource = indexBuffer->getResource(mDevice.getIndex());
			if (resource != nullptr)
			{
				vkBuffer = resource->getHandle();
				indexType = VulkanUtility::getIndexType(buffer->getProperties().getType());

				registerResource(resource, VulkanUseFlag::Read);
			}
		}

		vkCmdBindIndexBuffer(mCmdBuffer, vkBuffer, 0, indexType);
	}

	void VulkanCmdBuffer::setVertexDeclaration(const SPtr<VertexDeclarationCore>& decl)
	{
		if (mVertexDecl == decl)
			return;

		mVertexDecl = decl;
		mGfxPipelineRequiresBind = true;
	}

	bool VulkanCmdBuffer::isReadyForRender()
	{
		if (mGraphicsPipeline == nullptr)
			return false;

		SPtr<VertexDeclarationCore> inputDecl = mGraphicsPipeline->getInputDeclaration();
		if (inputDecl == nullptr)
			return false;

		return mFramebuffer != nullptr && mVertexDecl != nullptr;
	}

	bool VulkanCmdBuffer::bindGraphicsPipeline()
	{
		SPtr<VertexDeclarationCore> inputDecl = mGraphicsPipeline->getInputDeclaration();
		SPtr<VulkanVertexInput> vertexInput = VulkanVertexInputManager::instance().getVertexInfo(mVertexDecl, inputDecl);

		VulkanPipeline* pipeline = mGraphicsPipeline->getPipeline(mDevice.getIndex(), mFramebuffer,
																  mRenderTargetDepthReadOnly, mDrawOp, vertexInput);

		if (pipeline == nullptr)
			return false;

		// Check that pipeline matches the read-only state of any framebuffer attachments
		UINT32 numColorAttachments = mFramebuffer->getNumColorAttachments();
		for (UINT32 i = 0; i < numColorAttachments; i++)
		{
			VulkanImage* image = mFramebuffer->getColorAttachment(i).image;

			UINT32 imageInfoIdx = mImages[image];
			ImageInfo& imageInfo = mImageInfos[imageInfoIdx];

			if (imageInfo.isShaderInput && !pipeline->isColorReadOnly(i))
			{
				LOGWRN("Framebuffer attachment also used as a shader input, but color writes aren't disabled. This will"
					" result in undefined behavior.");
			}
		}

		if (mFramebuffer->hasDepthAttachment())
		{
			VulkanImage* image = mFramebuffer->getDepthStencilAttachment().image;

			UINT32 imageInfoIdx = mImages[image];
			ImageInfo& imageInfo = mImageInfos[imageInfoIdx];

			if (imageInfo.isShaderInput && !pipeline->isDepthStencilReadOnly())
			{
				LOGWRN("Framebuffer attachment also used as a shader input, but depth/stencil writes aren't disabled. "
					"This will result in undefined behavior.");
			}
		}

		mGraphicsPipeline->registerPipelineResources(this);
		registerResource(pipeline, VulkanUseFlag::Read);

		vkCmdBindPipeline(mCmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline->getHandle());
		bindDynamicStates(true);

		mGfxPipelineRequiresBind = false;
		return true;
	}

	void VulkanCmdBuffer::bindDynamicStates(bool forceAll)
	{
		if (mViewportRequiresBind || forceAll)
		{
			VkViewport viewport;
			viewport.x = mViewport.x * mRenderTargetWidth;
			viewport.y = mViewport.y * mRenderTargetHeight;
			viewport.width = mViewport.width * mRenderTargetWidth;
			viewport.height = mViewport.height * mRenderTargetHeight;
			viewport.minDepth = 0.0f;
			viewport.maxDepth = 1.0f;

			vkCmdSetViewport(mCmdBuffer, 0, 1, &viewport);
			mViewportRequiresBind = false;
		}

		if(mStencilRefRequiresBind || forceAll)
		{
			vkCmdSetStencilReference(mCmdBuffer, VK_STENCIL_FRONT_AND_BACK, mStencilRef);
			mStencilRefRequiresBind = false;
		}

		if(mScissorRequiresBind || forceAll)
		{
			VkRect2D scissorRect;
			if(mGraphicsPipeline->isScissorEnabled())
			{
				scissorRect.offset.x = mScissor.x;
				scissorRect.offset.y = mScissor.y;
				scissorRect.extent.width = mScissor.width;
				scissorRect.extent.height = mScissor.height;
			}
			else
			{
				scissorRect.offset.x = 0;
				scissorRect.offset.y = 0;
				scissorRect.extent.width = mRenderTargetWidth;
				scissorRect.extent.height = mRenderTargetHeight;
			}

			vkCmdSetScissor(mCmdBuffer, 0, 1, &scissorRect);

			mScissorRequiresBind = false;
		}
	}

	void VulkanCmdBuffer::bindGpuParams()
	{
		if (mBoundParamsDirty)
		{
			if (mBoundParams != nullptr)
			{
				mNumBoundDescriptorSets = mBoundParams->getNumSets();
				mBoundParams->prepareForBind(*this, mDescriptorSetsTemp);
			}
			else
				mNumBoundDescriptorSets = 0;

			mBoundParamsDirty = false;
		}
		else
		{
			mNumBoundDescriptorSets = 0;
		}
	}

	void VulkanCmdBuffer::executeLayoutTransitions()
	{
		auto createLayoutTransitionBarrier = [&](VulkanImage* image, ImageInfo& imageInfo)
		{
			mLayoutTransitionBarriersTemp.push_back(VkImageMemoryBarrier());
			VkImageMemoryBarrier& barrier = mLayoutTransitionBarriersTemp.back();
			barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
			barrier.pNext = nullptr;
			barrier.srcAccessMask = image->getAccessFlags(imageInfo.currentLayout);
			barrier.dstAccessMask = imageInfo.accessFlags;
			barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			barrier.oldLayout = imageInfo.currentLayout;
			barrier.newLayout = imageInfo.requiredLayout;
			barrier.image = image->getHandle();
			barrier.subresourceRange = imageInfo.range;

			imageInfo.currentLayout = imageInfo.requiredLayout;
			imageInfo.accessFlags = 0;
		};

		// Note: These layout transitions will contain transitions for offscreen framebuffer attachments (while they 
		// transition to shader read-only layout). This can be avoided, since they're immediately used by the render pass
		// as color attachments, making the layout change redundant.
		for (auto& entry : mQueuedLayoutTransitions)
		{
			UINT32 imageInfoIdx = entry.second;
			ImageInfo& imageInfo = mImageInfos[imageInfoIdx];

			createLayoutTransitionBarrier(entry.first, imageInfo);
		}

		mQueuedLayoutTransitions.clear();

		vkCmdPipelineBarrier(mCmdBuffer,
							 VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, // Note: VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT might be more correct here, according to the spec
							 VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
							 0, 0, nullptr,
							 0, nullptr,
							 (UINT32)mLayoutTransitionBarriersTemp.size(), mLayoutTransitionBarriersTemp.data());

		mLayoutTransitionBarriersTemp.clear();
	}

	void VulkanCmdBuffer::executeClearPass()
	{
		assert(mState == State::Recording);

		executeLayoutTransitions();

		VkRenderPassBeginInfo renderPassBeginInfo;
		renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassBeginInfo.pNext = nullptr;
		renderPassBeginInfo.framebuffer = mFramebuffer->getFramebuffer(RT_NONE, RT_NONE, mClearMask);
		renderPassBeginInfo.renderPass = mFramebuffer->getRenderPass(RT_NONE, RT_NONE, mClearMask);
		renderPassBeginInfo.renderArea.offset.x = mClearArea.x;
		renderPassBeginInfo.renderArea.offset.y = mClearArea.y;
		renderPassBeginInfo.renderArea.extent.width = mClearArea.width;
		renderPassBeginInfo.renderArea.extent.height = mClearArea.height;
		renderPassBeginInfo.clearValueCount = mFramebuffer->getNumAttachments();
		renderPassBeginInfo.pClearValues = mClearValues.data();

		vkCmdBeginRenderPass(mCmdBuffer, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
		vkCmdEndRenderPass(mCmdBuffer);

		mClearMask = CLEAR_NONE;
	}

	void VulkanCmdBuffer::draw(UINT32 vertexOffset, UINT32 vertexCount, UINT32 instanceCount)
	{
		if (!isReadyForRender())
			return;

		bindGpuParams();

		if (!isInRenderPass())
			beginRenderPass();

		if (mGfxPipelineRequiresBind)
		{
			if (!bindGraphicsPipeline())
				return;
		}
		else
			bindDynamicStates(false);

		if (mDescriptorSetsBindState.isSet(DescriptorSetBindFlag::Graphics))
		{
			if (mNumBoundDescriptorSets > 0)
			{
				UINT32 deviceIdx = mDevice.getIndex();
				VkPipelineLayout pipelineLayout = mGraphicsPipeline->getPipelineLayout(deviceIdx);

				vkCmdBindDescriptorSets(mCmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0,
										mNumBoundDescriptorSets, mDescriptorSetsTemp, 0, nullptr);
			}

			mDescriptorSetsBindState.unset(DescriptorSetBindFlag::Graphics);
		}

		vkCmdDraw(mCmdBuffer, vertexCount, instanceCount, vertexOffset, 0);
	}

	void VulkanCmdBuffer::drawIndexed(UINT32 startIndex, UINT32 indexCount, UINT32 vertexOffset, UINT32 instanceCount)
	{
		if (!isReadyForRender())
			return;

		bindGpuParams();

		if (!isInRenderPass())
			beginRenderPass();

		if (mGfxPipelineRequiresBind)
		{
			if (!bindGraphicsPipeline())
				return;
		}
		else
			bindDynamicStates(false);

		if (mDescriptorSetsBindState.isSet(DescriptorSetBindFlag::Graphics))
		{
			if (mNumBoundDescriptorSets > 0)
			{
				UINT32 deviceIdx = mDevice.getIndex();
				VkPipelineLayout pipelineLayout = mGraphicsPipeline->getPipelineLayout(deviceIdx);

				vkCmdBindDescriptorSets(mCmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0,
										mNumBoundDescriptorSets, mDescriptorSetsTemp, 0, nullptr);
			}

			mDescriptorSetsBindState.unset(DescriptorSetBindFlag::Graphics);
		}

		vkCmdDrawIndexed(mCmdBuffer, indexCount, instanceCount, startIndex, vertexOffset, 0);
	}

	void VulkanCmdBuffer::dispatch(UINT32 numGroupsX, UINT32 numGroupsY, UINT32 numGroupsZ)
	{
		if (mComputePipeline == nullptr)
			return;

		bindGpuParams();

		if (isInRenderPass())
			endRenderPass();

		UINT32 deviceIdx = mDevice.getIndex();
		if(mCmpPipelineRequiresBind)
		{
			VulkanPipeline* pipeline = mComputePipeline->getPipeline(deviceIdx);
			if (pipeline == nullptr)
				return;

			registerResource(pipeline, VulkanUseFlag::Read);
			mComputePipeline->registerPipelineResources(this);

			vkCmdBindPipeline(mCmdBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipeline->getHandle());
			mCmpPipelineRequiresBind = false;
		}

		if(mDescriptorSetsBindState.isSet(DescriptorSetBindFlag::Compute))
		{
			if (mNumBoundDescriptorSets > 0)
			{
				VkPipelineLayout pipelineLayout = mComputePipeline->getPipelineLayout(deviceIdx);
				vkCmdBindDescriptorSets(mCmdBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipelineLayout, 0,
										mNumBoundDescriptorSets, mDescriptorSetsTemp, 0, nullptr);
			}

			mDescriptorSetsBindState.unset(DescriptorSetBindFlag::Compute);
		}

		vkCmdDispatch(mCmdBuffer, numGroupsX, numGroupsY, numGroupsZ);
	}

	void VulkanCmdBuffer::setEvent(VulkanEvent* event)
	{
		if(isInRenderPass())
			mQueuedEvents.push_back(event);
		else
			vkCmdSetEvent(mCmdBuffer, event->getHandle(), VK_PIPELINE_STAGE_ALL_COMMANDS_BIT);
	}

	void VulkanCmdBuffer::resetQuery(VulkanQuery* query)
	{
		if (isInRenderPass())
			mQueuedQueryResets.push_back(query);
		else
			query->reset(mCmdBuffer);
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

	void VulkanCmdBuffer::registerResource(VulkanImage* res, VkAccessFlags accessFlags, VkImageLayout currentLayout,
			VkImageLayout newLayout, VkImageLayout finalLayout, VulkanUseFlags flags, bool isFBAttachment)
	{
		// Note: I currently always perform pipeline barriers (layout transitions and similar), over the entire image.
		//       In the case of render and storage images, the case is often that only a specific subresource requires
		//       it. However this makes grouping and tracking of current image layouts much more difficult.
		//       If this is ever requires we'll need to track image layout per-subresource instead per-image, and we
		//       might also need a smart way to group layout transitions for multiple sub-resources on the same image.

		VkImageSubresourceRange range = res->getRange();
		UINT32 nextImageInfoIdx = (UINT32)mImageInfos.size();

		auto insertResult = mImages.insert(std::make_pair(res, nextImageInfoIdx));
		if (insertResult.second) // New element
		{
			UINT32 imageInfoIdx = insertResult.first->second;
			mImageInfos.push_back(ImageInfo());

			ImageInfo& imageInfo = mImageInfos[imageInfoIdx];
			imageInfo.accessFlags = accessFlags;
			imageInfo.currentLayout = currentLayout;
			imageInfo.requiredLayout = newLayout;
			imageInfo.finalLayout = finalLayout;
			imageInfo.range = range;
			imageInfo.isFBAttachment = isFBAttachment;
			imageInfo.isShaderInput = !isFBAttachment;

			imageInfo.useHandle.used = false;
			imageInfo.useHandle.flags = flags;

			res->notifyBound();

			if (imageInfo.currentLayout != imageInfo.requiredLayout)
				mQueuedLayoutTransitions[res] = imageInfoIdx;
		}
		else // Existing element
		{
			UINT32 imageInfoIdx = insertResult.first->second;
			ImageInfo& imageInfo = mImageInfos[imageInfoIdx];

			assert(!imageInfo.useHandle.used);
			imageInfo.useHandle.flags |= flags;

			imageInfo.accessFlags |= accessFlags;

			// New layout is valid, check for transitions (UNDEFINED signifies the caller doesn't want a layout transition)
			if (newLayout != VK_IMAGE_LAYOUT_UNDEFINED)
			{
				// If layout transition was requested by framebuffer bind, respect it because render-pass will only accept a
				// specific layout (in certain cases), and we have no choice.
				// In the case when a FB attachment is also bound for shader reads, this will override the layout required for
				// shader read (GENERAL or DEPTH_READ_ONLY), but that is fine because those transitions are handled
				// automatically by render-pass layout transitions.
				// Any other texture (non FB attachment) will only even be bound in a single layout and we can keep the one it
				// was originally registered with.
				if (isFBAttachment)
					imageInfo.requiredLayout = newLayout;
				else if(!imageInfo.isFBAttachment) // Layout transition is not being done on a FB image
				{
					// Check if the image had a layout previously assigned, and if so check if multiple different layouts
					// were requested. In that case we wish to transfer the image to GENERAL layout.

					bool firstUseInRenderPass = !imageInfo.isShaderInput && !imageInfo.isFBAttachment;
					if (firstUseInRenderPass || imageInfo.requiredLayout == VK_IMAGE_LAYOUT_UNDEFINED)
						imageInfo.requiredLayout = newLayout;
					else if (imageInfo.requiredLayout != newLayout)
						imageInfo.requiredLayout = VK_IMAGE_LAYOUT_GENERAL;
				}
			}

			// If attached to FB, then the final layout is set by the FB (provided as layout param here), otherwise its
			// the same as required layout
			if(!isFBAttachment && !imageInfo.isFBAttachment)
				imageInfo.finalLayout = imageInfo.requiredLayout;
			else
			{
				if (isFBAttachment)
					imageInfo.finalLayout = finalLayout;
			}

			if (imageInfo.currentLayout != imageInfo.requiredLayout)
				mQueuedLayoutTransitions[res] = imageInfoIdx;

			// If a FB attachment was just bound as a shader input, we might need to restart the render pass with a FB
			// attachment that supports read-only attachments using the GENERAL layout
			bool requiresReadOnlyFB = false;
			if (isFBAttachment)
			{
				if (!imageInfo.isFBAttachment)
				{
					imageInfo.isFBAttachment = true;
					requiresReadOnlyFB = imageInfo.isShaderInput;
				}
			}
			else
			{
				if (!imageInfo.isShaderInput)
				{
					imageInfo.isShaderInput = true;
					requiresReadOnlyFB = imageInfo.isFBAttachment;
				}
			}

			// If we need to switch frame-buffers, end current render pass
			if (requiresReadOnlyFB && isInRenderPass())
				endRenderPass();
		}

		// Register any sub-resources
		for(UINT32 i = 0; i < range.layerCount; i++)
		{
			for(UINT32 j = 0; j < range.levelCount; j++)
			{
				UINT32 layer = range.baseArrayLayer + i;
				UINT32 mipLevel = range.baseMipLevel + j;

				registerResource(res->getSubresource(layer, mipLevel), flags);
			}
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

	void VulkanCmdBuffer::registerResource(VulkanFramebuffer* res, RenderSurfaceMask loadMask, VulkanUseFlags flags)
	{
		auto insertResult = mResources.insert(std::make_pair(res, ResourceUseHandle()));
		if (insertResult.second) // New element
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

		// Register any sub-resources
		UINT32 numColorAttachments = res->getNumColorAttachments();
		for (UINT32 i = 0; i < numColorAttachments; i++)
		{
			const VulkanFramebufferAttachment& attachment = res->getColorAttachment(i);

			VkImageLayout layout;
			VkAccessFlags accessMask;

			// If image is being loaded, we need to transfer it to correct layout, otherwise it doesn't matter
			if (loadMask.isSet((RenderSurfaceMaskBits)(1 << i)))
			{
				layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
				accessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
			}
			else
			{
				layout = attachment.image->getLayout();
				accessMask = 0;
			}

			registerResource(attachment.image, accessMask, attachment.image->getLayout(), layout, attachment.finalLayout, 
				VulkanUseFlag::Write, true);
		}

		if(res->hasDepthAttachment())
		{
			const VulkanFramebufferAttachment& attachment = res->getDepthStencilAttachment();

			VkImageLayout layout;
			VkAccessFlags accessMask;

			// If image is being loaded, we need to transfer it to correct layout, otherwise it doesn't matter
			if (loadMask.isSet(RT_DEPTH))
			{
				layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
				accessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT;
			}
			else
			{
				layout = attachment.image->getLayout();
				accessMask = 0;
			}

			registerResource(attachment.image, accessMask, attachment.image->getLayout(), layout, attachment.finalLayout, 
				VulkanUseFlag::Write, true);
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

		if (mBuffer->isInRenderPass())
			mBuffer->endRenderPass();

		if (mBuffer->isRecording())
			mBuffer->end();

		if (!mBuffer->isReadyForSubmit()) // Possibly nothing was recorded in the buffer
			return;

		mBuffer->submit(mQueue, mQueueIdx, syncMask);
		acquireNewBuffer();

		gVulkanCBManager().refreshStates(mDeviceIdx);
	}
}