//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsVulkanCommandBuffer.h"
#include "Managers/BsVulkanCommandBufferManager.h"
#include "BsVulkanUtility.h"
#include "BsVulkanDevice.h"
#include "BsVulkanGpuParams.h"
#include "BsVulkanQueue.h"
#include "BsVulkanTexture.h"
#include "BsVulkanIndexBuffer.h"
#include "BsVulkanVertexBuffer.h"
#include "BsVulkanHardwareBuffer.h"
#include "BsVulkanFramebuffer.h"
#include "Managers/BsVulkanVertexInputManager.h"
#include "BsVulkanEventQuery.h"
#include "Managers/BsVulkanQueryManager.h"
#include "BsVulkanSwapChain.h"
#include "BsVulkanTimerQuery.h"
#include "BsVulkanOcclusionQuery.h"

#if BS_PLATFORM == BS_PLATFORM_WIN32
#include "Win32/BsWin32RenderWindow.h"
#elif BS_PLATFORM == BS_PLATFORM_LINUX
#include "Linux/BsLinuxRenderWindow.h"
#else
static_assert(false, "Other platforms go here");
#endif

namespace bs { namespace ct
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
		:mDevice(device)
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

	/** Returns a set of pipeline stages that can are allowed to be used for the specified set of access flags. */
	VkPipelineStageFlags getPipelineStageFlags(VkAccessFlags accessFlags)
	{
		VkPipelineStageFlags flags = 0;

		if ((accessFlags & VK_ACCESS_INDIRECT_COMMAND_READ_BIT) != 0)
			flags |= VK_PIPELINE_STAGE_DRAW_INDIRECT_BIT;

		if ((accessFlags & (VK_ACCESS_INDEX_READ_BIT | VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT)) != 0)
			flags |= VK_PIPELINE_STAGE_VERTEX_INPUT_BIT;

		if ((accessFlags & (VK_ACCESS_UNIFORM_READ_BIT | VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT)) != 0)
		{
			flags |= VK_PIPELINE_STAGE_VERTEX_SHADER_BIT | VK_PIPELINE_STAGE_TESSELLATION_CONTROL_SHADER_BIT;
			flags |= VK_PIPELINE_STAGE_TESSELLATION_EVALUATION_SHADER_BIT | VK_PIPELINE_STAGE_GEOMETRY_SHADER_BIT;
			flags |= VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT | VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;
		}

		if ((accessFlags & VK_ACCESS_INPUT_ATTACHMENT_READ_BIT) != 0)
			flags |= VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;

		if ((accessFlags & (VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT)) != 0)
			flags |= VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

		if ((accessFlags & (VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT)) != 0)
			flags |= VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;

		if ((accessFlags & (VK_ACCESS_TRANSFER_READ_BIT | VK_ACCESS_TRANSFER_WRITE_BIT)) != 0)
			flags |= VK_PIPELINE_STAGE_TRANSFER_BIT;

		if ((accessFlags & (VK_ACCESS_HOST_READ_BIT | VK_ACCESS_HOST_WRITE_BIT)) != 0)
			flags |= VK_PIPELINE_STAGE_HOST_BIT;

		if (flags == 0)
			flags = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;

		return flags;
	}

	template<class T>
	void getPipelineStageFlags(const Vector<T>& barriers, VkPipelineStageFlags& src, VkPipelineStageFlags& dst)
	{
		for(auto& entry : barriers)
		{
			src |= getPipelineStageFlags(entry.srcAccessMask);
			dst |= getPipelineStageFlags(entry.dstAccessMask);
		}

		if (src == 0)
			src = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;

		if (dst == 0)
			dst = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
	}

	VulkanCmdBuffer::VulkanCmdBuffer(VulkanDevice& device, UINT32 id, VkCommandPool pool, UINT32 queueFamily, bool secondary)
		: mId(id), mQueueFamily(queueFamily), mDevice(device), mPool(pool)
		, mNeedsWARMemoryBarrier(false), mNeedsRAWMemoryBarrier(false), mGfxPipelineRequiresBind(true)
		, mCmpPipelineRequiresBind(true), mViewportRequiresBind(true), mStencilRefRequiresBind(true)
		, mScissorRequiresBind(true), mBoundParamsDirty(false), mVertexInputsDirty(false)
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
				assert(!useHandle.used);

				entry.first->notifyUnbound();
			}

			for (auto& entry : mImages)
			{
				UINT32 imageInfoIdx = entry.second;
				ImageInfo& imageInfo = mImageInfos[imageInfoIdx];

				ResourceUseHandle& useHandle = imageInfo.useHandle;
				assert(!useHandle.used);

				entry.first->notifyUnbound();
			}

			for (auto& entry : mBuffers)
			{
				ResourceUseHandle& useHandle = entry.second.useHandle;
				assert(!useHandle.used);

				entry.first->notifyUnbound();
			}

			// Must be done after images & framebuffer because swap chain does error checking if those were freed
			for (auto& entry : mSwapChains)
			{
				ResourceUseHandle& useHandle = entry.second;
				assert(!useHandle.used);

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
			Rect2I rtArea(0, 0, mFramebuffer->getWidth(), mFramebuffer->getHeight());
			if (mClearArea != rtArea)
				executeClearPass();
		}

		executeWriteHazardBarrier();
		executeLayoutTransitions();

		RenderSurfaceMask readMask = getFBReadMask();

		VkRenderPassBeginInfo renderPassBeginInfo;
		renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassBeginInfo.pNext = nullptr;
		renderPassBeginInfo.framebuffer = mFramebuffer->getFramebuffer(mRenderTargetLoadMask, readMask, mClearMask);
		renderPassBeginInfo.renderPass = mFramebuffer->getRenderPass(mRenderTargetLoadMask, readMask, mClearMask);
		renderPassBeginInfo.renderArea.offset.x = 0;
		renderPassBeginInfo.renderArea.offset.y = 0;
		renderPassBeginInfo.renderArea.extent.width = mFramebuffer->getWidth();
		renderPassBeginInfo.renderArea.extent.height = mFramebuffer->getHeight();
		renderPassBeginInfo.clearValueCount = mFramebuffer->getNumClearEntries(mClearMask);
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

		// Remove any shader use flags on images. Note this relies on the fact that we re-bind all parameters on every
		// dispatch call and render pass, so they can reset this flags. Otherwise clearing the flags is wrong if the
		// images remain to be used in subsequent calls).
		for(auto& entry : mShaderBoundSubresourceInfos)
		{
			ImageSubresourceInfo& subresourceInfo = mSubresourceInfoStorage[entry];
			subresourceInfo.useFlags.unset(ImageUseFlagBits::Shader);
			subresourceInfo.shaderUse.access = VulkanAccessFlag::None;
			subresourceInfo.shaderUse.stages = 0;
		}

		mShaderBoundSubresourceInfos.clear();

		updateFinalLayouts();

		mState = State::Recording;

		// In case the same GPU params from last pass get used, this makes sure the states we reset above, get re-applied
		mBoundParamsDirty = true;
	}

	void VulkanCmdBuffer::allocateSemaphores(VkSemaphore* semaphores)
	{
		if (mIntraQueueSemaphore != nullptr)
			mIntraQueueSemaphore->destroy();

		mIntraQueueSemaphore = mDevice.getResourceManager().create<VulkanSemaphore>();
		semaphores[0] = mIntraQueueSemaphore->getHandle();

		for (UINT32 i = 0; i < BS_MAX_VULKAN_CB_DEPENDENCIES; i++)
		{
			if (mInterQueueSemaphores[i] != nullptr)
				mInterQueueSemaphores[i]->destroy();

			mInterQueueSemaphores[i] = mDevice.getResourceManager().create<VulkanSemaphore>();
			semaphores[i + 1] = mInterQueueSemaphores[i]->getHandle();
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
			if (currentQueueFamily != (UINT32)-1 && currentQueueFamily != mQueueFamily)
			{
				Vector<VkBufferMemoryBarrier>& barriers = mTransitionInfoTemp[currentQueueFamily].bufferBarriers;

				barriers.push_back(VkBufferMemoryBarrier());
				VkBufferMemoryBarrier& barrier = barriers.back();
				barrier.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
				barrier.pNext = nullptr;
				barrier.srcAccessMask = 0;
				barrier.dstAccessMask = 0;
				barrier.srcQueueFamilyIndex = currentQueueFamily;
				barrier.dstQueueFamilyIndex = mQueueFamily;
				barrier.buffer = resource->getHandle();
				barrier.offset = 0;
				barrier.size = VK_WHOLE_SIZE;
			}
		}

		// For images issue queue transitions, as above. Also issue layout transitions to their inital layouts.
		Vector<VkImageMemoryBarrier>& localBarriers = mTransitionInfoTemp[mQueueFamily].imageBarriers;
		for (auto& entry : mImages)
		{
			VulkanImage* resource = static_cast<VulkanImage*>(entry.first);
			ImageInfo& imageInfo = mImageInfos[entry.second];

			UINT32 currentQueueFamily = resource->getQueueFamily();
			bool queueMismatch = resource->isExclusive() && currentQueueFamily != (UINT32)-1
				&& currentQueueFamily != mQueueFamily;

			ImageSubresourceInfo* subresourceInfos = &mSubresourceInfoStorage[imageInfo.subresourceInfoIdx];
			if (queueMismatch)
			{
				Vector<VkImageMemoryBarrier>& barriers = mTransitionInfoTemp[currentQueueFamily].imageBarriers;

				for (UINT32 i = 0; i < imageInfo.numSubresourceInfos; i++)
				{
					ImageSubresourceInfo& subresourceInfo = subresourceInfos[i];

					UINT32 startIdx = (UINT32)barriers.size();
					resource->getBarriers(subresourceInfo.range, barriers);

					for(UINT32 j = startIdx; j < (UINT32)barriers.size(); j++)
					{
						VkImageMemoryBarrier& barrier = barriers[j];

						barrier.dstAccessMask = 0;
						barrier.newLayout = barrier.oldLayout;
						barrier.srcQueueFamilyIndex = currentQueueFamily;
						barrier.dstQueueFamilyIndex = mQueueFamily;
					}
				}
			}

			for (UINT32 i = 0; i < imageInfo.numSubresourceInfos; i++)
			{
				ImageSubresourceInfo& subresourceInfo = subresourceInfos[i];

				const VkImageSubresourceRange& range = subresourceInfo.range;
				UINT32 mipEnd = range.baseMipLevel + range.levelCount;
				UINT32 faceEnd = range.baseArrayLayer + range.layerCount;

				VkImageLayout initialLayout = subresourceInfo.initialLayout;
				if (initialLayout != VK_IMAGE_LAYOUT_UNDEFINED)
				{
					bool layoutMismatch = false;
					for (UINT32 mip = range.baseMipLevel; mip < mipEnd; mip++)
					{
						for (UINT32 face = range.baseArrayLayer; face < faceEnd; face++)
						{
							VulkanImageSubresource* subresource = resource->getSubresource(face, mip);
							if (subresource->getLayout() != initialLayout)
							{
								layoutMismatch = true;
								break;
							}
						}

						if (layoutMismatch)
							break;
					}

					if (layoutMismatch)
					{
						UINT32 startIdx = (UINT32)localBarriers.size();
						resource->getBarriers(subresourceInfo.range, localBarriers);

						for (UINT32 j = startIdx; j < (UINT32)localBarriers.size(); j++)
						{
							VkImageMemoryBarrier& barrier = localBarriers[j];

							barrier.dstAccessMask = resource->getAccessFlags(initialLayout, subresourceInfo.initialReadOnly);
							barrier.newLayout = initialLayout;
						}
					}
				}

				for (UINT32 mip = range.baseMipLevel; mip < mipEnd; mip++)
				{
					for (UINT32 face = range.baseArrayLayer; face < faceEnd; face++)
					{
						VulkanImageSubresource* subresource = resource->getSubresource(face, mip);
						subresource->setLayout(subresourceInfo.currentLayout);
					}
				}
			}
		}

		for (auto& entry : mTransitionInfoTemp)
		{
			bool empty = entry.second.imageBarriers.empty() && entry.second.bufferBarriers.empty();
			if (empty)
				continue;

			UINT32 entryQueueFamily = entry.first;

			// No queue transition needed for entries on this queue (this entry is most likely an image layout transition)
			if (entryQueueFamily == (UINT32)-1 || entryQueueFamily == mQueueFamily)
				continue;

			VulkanCmdBuffer* cmdBuffer = device.getCmdBufferPool().getBuffer(entryQueueFamily, false);
			VkCommandBuffer vkCmdBuffer = cmdBuffer->getHandle();

			TransitionInfo& barriers = entry.second;
			UINT32 numImgBarriers = (UINT32)barriers.imageBarriers.size();
			UINT32 numBufferBarriers = (UINT32)barriers.bufferBarriers.size();

			VkPipelineStageFlags srcStage = 0;
			VkPipelineStageFlags dstStage = 0;
			getPipelineStageFlags(barriers.imageBarriers, srcStage, dstStage);

			vkCmdPipelineBarrier(vkCmdBuffer,
								 srcStage, dstStage,
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

			// Note: If I switch back to doing layout transitions here, I need to wait on present semaphore
			otherQueue->submit(cmdBuffer, nullptr, 0);
		}

		UINT32 deviceIdx = device.getIndex();
		VulkanCommandBufferManager& cbm = static_cast<VulkanCommandBufferManager&>(CommandBufferManager::instance());

		UINT32 numSemaphores;
		cbm.getSyncSemaphores(deviceIdx, syncMask, mSemaphoresTemp.data(), numSemaphores);

		// Wait on present (i.e. until the back buffer becomes available) for any swap chains
		for(auto& entry : mActiveSwapChains)
		{
			const SwapChainSurface& surface = entry->getBackBuffer();
			if (surface.needsWait)
			{
				VulkanSemaphore* semaphore = entry->getBackBuffer().sync;

				if (numSemaphores >= (UINT32)mSemaphoresTemp.size())
					mSemaphoresTemp.push_back(semaphore);
				else
					mSemaphoresTemp[numSemaphores] = semaphore;

				numSemaphores++;

				entry->notifyBackBufferWaitIssued();
			}
		}

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

			VkPipelineStageFlags srcStage = 0;
			VkPipelineStageFlags dstStage = 0;
			getPipelineStageFlags(barriers.imageBarriers, srcStage, dstStage);

			vkCmdPipelineBarrier(vkCmdBuffer,
								 srcStage, dstStage,
								 0, 0, nullptr,
								 numBufferBarriers, barriers.bufferBarriers.data(),
								 numImgBarriers, barriers.imageBarriers.data());

			cmdBuffer->end();
			queue->queueSubmit(cmdBuffer, mSemaphoresTemp.data(), numSemaphores);

			numSemaphores = 0; // Semaphores are only needed the first time, since we're adding the buffers on the same queue
		}

		queue->queueSubmit(this, mSemaphoresTemp.data(), numSemaphores);
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

		for (auto& entry : mSwapChains)
		{
			ResourceUseHandle& useHandle = entry.second;
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
		mIndexBuffer = nullptr;
		mVertexBuffers.clear();
		mVertexInputsDirty = true;
		mActiveSwapChains.clear();
	}

	bool VulkanCmdBuffer::checkFenceStatus(bool block) const
	{
		VkResult result = vkWaitForFences(mDevice.getLogical(), 1, &mFence, true, block ? 1'000'000'000 : 0);
		assert(result == VK_SUCCESS || result == VK_TIMEOUT);

		return result == VK_SUCCESS;
	}

	void VulkanCmdBuffer::reset()
	{
		bool wasSubmitted = mState == State::Submitted;

		mState = State::Ready;
		vkResetCommandBuffer(mCmdBuffer, VK_COMMAND_BUFFER_RESET_RELEASE_RESOURCES_BIT); // Note: Maybe better not to release resources?

		if (wasSubmitted)
		{
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

			// Must be done after images & framebuffer because swap chain does error checking if those were freed
			for (auto& entry : mSwapChains)
			{
				ResourceUseHandle& useHandle = entry.second;
				assert(useHandle.used);

				entry.first->notifyDone(mGlobalQueueIdx, useHandle.flags);
			}
		}
		else
		{
			for (auto& entry : mResources)
				entry.first->notifyUnbound();

			for (auto& entry : mImages)
				entry.first->notifyUnbound();

			for (auto& entry : mBuffers)
				entry.first->notifyUnbound();

			// Must be done after images & framebuffer because swap chain does error checking if those were freed
			for (auto& entry : mSwapChains)
				entry.first->notifyUnbound();
		}

		mResources.clear();
		mImages.clear();
		mBuffers.clear();
		mSwapChains.clear();
		mOcclusionQueries.clear();
		mTimerQueries.clear();
		mImageInfos.clear();
		mSubresourceInfoStorage.clear();
		mShaderBoundSubresourceInfos.clear();
		mNeedsRAWMemoryBarrier = false;
		mNeedsWARMemoryBarrier = false;
		mMemoryBarrierDstAccess = 0;
		mMemoryBarrierSrcAccess = 0;
		mMemoryBarrierDstStages = 0;
		mMemoryBarrierSrcStages = 0;
	}

	void VulkanCmdBuffer::setRenderTarget(const SPtr<RenderTarget>& rt, UINT32 readOnlyFlags, RenderSurfaceMask loadMask)
	{
		assert(mState != State::Submitted);

		VulkanFramebuffer* newFB;
		VulkanSwapChain* swapChain = nullptr;
		if(rt != nullptr)
		{
			if (rt->getProperties().isWindow)
			{
#if BS_PLATFORM == BS_PLATFORM_WIN32
				Win32RenderWindow* window = static_cast<Win32RenderWindow*>(rt.get());
#elif BS_PLATFORM == BS_PLATFORM_LINUX
				LinuxRenderWindow* window = static_cast<LinuxRenderWindow*>(rt.get());
#endif
				window->acquireBackBuffer();

				rt->getCustomAttribute("SC", &swapChain);
				mActiveSwapChains.insert(swapChain);
			}

			rt->getCustomAttribute("FB", &newFB);
		}
		else
		{
			newFB = nullptr;
		}

		// Warn if invalid load mask
		if (loadMask.isSet(RT_DEPTH) && !loadMask.isSet(RT_STENCIL))
		{
			LOGWRN("setRenderTarget() invalid load mask, depth enabled but stencil disabled. This is not supported. Both \
				will be loaded.");

			loadMask.set(RT_STENCIL);
		}

		if (!loadMask.isSet(RT_DEPTH) && loadMask.isSet(RT_STENCIL))
		{
			LOGWRN("setRenderTarget() invalid load mask, stencil enabled but depth disabled. This is not supported. Both \
				will be loaded.");

			loadMask.set(RT_DEPTH);
		}

		if (mFramebuffer == newFB && mRenderTargetReadOnlyFlags == readOnlyFlags && mRenderTargetLoadMask == loadMask)
			return;

		if (isInRenderPass())
			endRenderPass();
		else
		{
			// If a clear is queued for previous FB, execute the render pass with no additional instructions
			if (mClearMask)
				executeClearPass();
		}

		// Reset isFBAttachment flags for subresources from the old framebuffer
		if(mFramebuffer != nullptr)
		{
			UINT32 numColorAttachments = mFramebuffer->getNumColorAttachments();
			for(UINT32 i = 0; i < numColorAttachments; i++)
			{
				const VulkanFramebufferAttachment& fbAttachment = mFramebuffer->getColorAttachment(i);
				UINT32 imageInfoIdx = mImages[fbAttachment.image];
				ImageInfo& imageInfo = mImageInfos[imageInfoIdx];

				ImageSubresourceInfo* subresourceInfos = &mSubresourceInfoStorage[imageInfo.subresourceInfoIdx];
				for(UINT32 j = 0; j < imageInfo.numSubresourceInfos; j++)
				{
					ImageSubresourceInfo& entry = subresourceInfos[j];
					entry.useFlags.unset(ImageUseFlagBits::Framebuffer);
					entry.fbUse.access = VulkanAccessFlag::None;
					entry.fbUse.stages = 0;
				}
			}

			if(mFramebuffer->hasDepthAttachment())
			{
				const VulkanFramebufferAttachment& fbAttachment = mFramebuffer->getDepthStencilAttachment();
				UINT32 imageInfoIdx = mImages[fbAttachment.image];
				ImageInfo& imageInfo = mImageInfos[imageInfoIdx];

				ImageSubresourceInfo* subresourceInfos = &mSubresourceInfoStorage[imageInfo.subresourceInfoIdx];
				for(UINT32 j = 0; j < imageInfo.numSubresourceInfos; j++)
				{
					ImageSubresourceInfo& entry = subresourceInfos[j];
					entry.useFlags.unset(ImageUseFlagBits::Framebuffer);
					entry.fbUse.access = VulkanAccessFlag::None;
					entry.fbUse.stages = 0;
				}
			}
		}

		if(newFB == nullptr)
		{
			mFramebuffer = nullptr;
			mRenderTargetReadOnlyFlags = 0;
			mRenderTargetLoadMask = RT_NONE;
		}
		else
		{
			mFramebuffer = newFB;
			mRenderTargetReadOnlyFlags = readOnlyFlags;
			mRenderTargetLoadMask = loadMask;
		}

		// Re-set the params as they will need to be re-bound
		setGpuParams(mBoundParams);

		if (mFramebuffer)
		{
			registerResource(mFramebuffer, loadMask, readOnlyFlags);

			if(swapChain)
				registerResource(swapChain);
		}

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
		Rect2I area(0, 0, mFramebuffer->getWidth(), mFramebuffer->getHeight());
		clearViewport(area, buffers, color, depth, stencil, targetMask);
	}

	void VulkanCmdBuffer::clearViewport(UINT32 buffers, const Color& color, float depth, UINT16 stencil, UINT8 targetMask)
	{
		Rect2I area;
		area.x = (UINT32)(mViewport.x * mFramebuffer->getWidth());
		area.y = (UINT32)(mViewport.y * mFramebuffer->getHeight());
		area.width = (UINT32)(mViewport.width * mFramebuffer->getWidth());
		area.height = (UINT32)(mViewport.height * mFramebuffer->getHeight());

		clearViewport(area, buffers, color, depth, stencil, targetMask);
	}

	void VulkanCmdBuffer::setPipelineState(const SPtr<GraphicsPipelineState>& state)
	{
		if (mGraphicsPipeline == state)
			return;

		mGraphicsPipeline = std::static_pointer_cast<VulkanGraphicsPipelineState>(state);
		mGfxPipelineRequiresBind = true; 
	}

	void VulkanCmdBuffer::setPipelineState(const SPtr<ComputePipelineState>& state)
	{
		if (mComputePipeline == state)
			return;

		mComputePipeline = std::static_pointer_cast<VulkanComputePipelineState>(state);
		mCmpPipelineRequiresBind = true;
	}

	void VulkanCmdBuffer::setGpuParams(const SPtr<GpuParams>& gpuParams)
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

	void VulkanCmdBuffer::setVertexBuffers(UINT32 index, SPtr<VertexBuffer>* buffers, UINT32 numBuffers)
	{
		if (numBuffers == 0)
			return;

		UINT32 endIdx = index + numBuffers;
		if(mVertexBuffers.size() < endIdx)
			mVertexBuffers.resize(endIdx);

		for(UINT32 i = index; i < endIdx; i++)
			mVertexBuffers[i] = std::static_pointer_cast<VulkanVertexBuffer>(buffers[i]);

		mVertexInputsDirty = true;
	}

	void VulkanCmdBuffer::setIndexBuffer(const SPtr<IndexBuffer>& buffer)
	{
		mIndexBuffer = std::static_pointer_cast<VulkanIndexBuffer>(buffer);

		mVertexInputsDirty = true;
	}

	void VulkanCmdBuffer::setVertexDeclaration(const SPtr<VertexDeclaration>& decl)
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

		SPtr<VertexDeclaration> inputDecl = mGraphicsPipeline->getInputDeclaration();
		if (inputDecl == nullptr)
			return false;

		return mFramebuffer != nullptr && mVertexDecl != nullptr;
	}

	bool VulkanCmdBuffer::bindGraphicsPipeline()
	{
		SPtr<VertexDeclaration> inputDecl = mGraphicsPipeline->getInputDeclaration();
		SPtr<VulkanVertexInput> vertexInput = VulkanVertexInputManager::instance().getVertexInfo(mVertexDecl, inputDecl);

		VulkanPipeline* pipeline = mGraphicsPipeline->getPipeline(mDevice.getIndex(), mFramebuffer,
			mRenderTargetReadOnlyFlags, mDrawOp, vertexInput);

		if (pipeline == nullptr)
			return false;

		// Check that pipeline matches the read-only state of any framebuffer attachments
		UINT32 numColorAttachments = mFramebuffer->getNumColorAttachments();
		for (UINT32 i = 0; i < numColorAttachments; i++)
		{
			const VulkanFramebufferAttachment& fbAttachment = mFramebuffer->getColorAttachment(i);
			ImageSubresourceInfo& subresourceInfo = findSubresourceInfo(fbAttachment.image, fbAttachment.surface.face, 
				fbAttachment.surface.mipLevel);

			if (subresourceInfo.useFlags.isSet(ImageUseFlagBits::Shader) && !pipeline->isColorReadOnly(i))
			{
				LOGWRN("Framebuffer attachment also used as a shader input, but color writes aren't disabled. This will"
					" result in undefined behavior.");
			}
		}

		if (mFramebuffer->hasDepthAttachment())
		{
			const VulkanFramebufferAttachment& fbAttachment = mFramebuffer->getDepthStencilAttachment();
			ImageSubresourceInfo& subresourceInfo = findSubresourceInfo(fbAttachment.image, fbAttachment.surface.face,
				fbAttachment.surface.mipLevel);

			if (subresourceInfo.useFlags.isSet(ImageUseFlagBits::Shader) && !pipeline->isDepthReadOnly())
			{
				LOGWRN("Framebuffer attachment also used as a shader input, but depth/stencil writes aren't disabled. "
					"This will result in undefined behavior.");
			}
		}

		mGraphicsPipeline->registerPipelineResources(this);
		registerResource(pipeline, VulkanAccessFlag::Read);

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
			viewport.x = mViewport.x * mFramebuffer->getWidth();
			viewport.y = mViewport.y * mFramebuffer->getHeight();
			viewport.width = mViewport.width * mFramebuffer->getWidth();
			viewport.height = mViewport.height * mFramebuffer->getHeight();
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
				scissorRect.extent.width = mFramebuffer->getWidth();
				scissorRect.extent.height = mFramebuffer->getHeight();
			}

			vkCmdSetScissor(mCmdBuffer, 0, 1, &scissorRect);

			mScissorRequiresBind = false;
		}
	}

	void VulkanCmdBuffer::bindVertexInputs()
	{
		if (!mVertexBuffers.empty())
		{
			UINT32 lastValidIdx = (UINT32)-1;
			UINT32 curIdx = 0;
			for(auto& vertexBuffer : mVertexBuffers)
			{
				bool validBuffer = false;
				if (vertexBuffer != nullptr)
				{
					VulkanBuffer* resource = vertexBuffer->getResource(mDevice.getIndex());
					if (resource != nullptr)
					{
						mVertexBuffersTemp[curIdx] = resource->getHandle();

						registerBuffer(resource, BufferUseFlagBits::Vertex, VulkanAccessFlag::Read);

						if(lastValidIdx == (UINT32)-1)
							lastValidIdx = curIdx;

						validBuffer = true;
					}
				}

				if(!validBuffer && lastValidIdx != (UINT32)-1)
				{
					UINT32 count = curIdx - lastValidIdx;
					if(count > 0)
					{
						vkCmdBindVertexBuffers(mCmdBuffer, lastValidIdx, count, mVertexBuffersTemp, 
							mVertexBufferOffsetsTemp);

						lastValidIdx = (UINT32)-1;
					}
				}

				curIdx++;
			}

			if (lastValidIdx != (UINT32)-1)
			{
				UINT32 count = curIdx - lastValidIdx;
				if (count > 0)
				{
					vkCmdBindVertexBuffers(mCmdBuffer, lastValidIdx, count, mVertexBuffersTemp,
						mVertexBufferOffsetsTemp);
				}
			}
		}

		VkBuffer vkBuffer = VK_NULL_HANDLE;
		VkIndexType indexType = VK_INDEX_TYPE_UINT32;
		if (mIndexBuffer != nullptr)
		{
			VulkanBuffer* resource = mIndexBuffer->getResource(mDevice.getIndex());
			if (resource != nullptr)
			{
				vkBuffer = resource->getHandle();
				indexType = VulkanUtility::getIndexType(mIndexBuffer->getProperties().getType());

				registerBuffer(resource, BufferUseFlagBits::Index, VulkanAccessFlag::Read);
			}
		}

		vkCmdBindIndexBuffer(mCmdBuffer, vkBuffer, 0, indexType);
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
			ImageSubresourceInfo* subresourceInfos = &mSubresourceInfoStorage[imageInfo.subresourceInfoIdx];
			for (UINT32 i = 0; i < imageInfo.numSubresourceInfos; i++)
			{
				ImageSubresourceInfo& subresourceInfo = subresourceInfos[i];

				if (subresourceInfo.requiredLayout == VK_IMAGE_LAYOUT_UNDEFINED || 
					subresourceInfo.currentLayout == subresourceInfo.requiredLayout)
					continue;

				const bool isReadOnly = 
					!subresourceInfo.fbUse.access.isSet(VulkanAccessFlag::Write) &&
					!subresourceInfo.shaderUse.access.isSet(VulkanAccessFlag::Write);

				mLayoutTransitionBarriersTemp.push_back(VkImageMemoryBarrier());
				VkImageMemoryBarrier& barrier = mLayoutTransitionBarriersTemp.back();
				barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
				barrier.pNext = nullptr;
				barrier.srcAccessMask = 0; // Not relevant for layout transition
				barrier.dstAccessMask = image->getAccessFlags(subresourceInfo.requiredLayout, isReadOnly);
				barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
				barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
				barrier.oldLayout = subresourceInfo.currentLayout;
				barrier.newLayout = subresourceInfo.requiredLayout;
				barrier.image = image->getHandle();
				barrier.subresourceRange = subresourceInfo.range;

				subresourceInfo.currentLayout = subresourceInfo.requiredLayout;
			}
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

		VkPipelineStageFlags srcStage = 0;
		VkPipelineStageFlags dstStage = 0;
		getPipelineStageFlags(mLayoutTransitionBarriersTemp, srcStage, dstStage);

		if(!mLayoutTransitionBarriersTemp.empty())
		{
			vkCmdPipelineBarrier(
				mCmdBuffer,
				srcStage, dstStage,
				0, 0, nullptr,
				0, nullptr,
				(UINT32) mLayoutTransitionBarriersTemp.size(), mLayoutTransitionBarriersTemp.data());
		}

		mQueuedLayoutTransitions.clear();
		mLayoutTransitionBarriersTemp.clear();
	}

	void VulkanCmdBuffer::executeWriteHazardBarrier()
	{
		if(!mNeedsRAWMemoryBarrier && !mNeedsWARMemoryBarrier)
			return;

		// If read-after-write we need an actual memory barrier, otherwise we just need an execution dependency
		if(mNeedsRAWMemoryBarrier)
		{
			VkMemoryBarrier barrier;
			barrier.sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER;
			barrier.pNext = nullptr;

			barrier.srcAccessMask = mMemoryBarrierSrcAccess;
			barrier.dstAccessMask = mMemoryBarrierDstAccess;

			vkCmdPipelineBarrier(getHandle(),
				mMemoryBarrierSrcStages,
				mMemoryBarrierDstStages,
				0, 1, &barrier,
				0, nullptr,
				0, nullptr);
		}
		else
		{
			vkCmdPipelineBarrier(getHandle(),
				mMemoryBarrierSrcStages,
				mMemoryBarrierDstStages,
				0, 0, nullptr,
				0, nullptr,
				0, nullptr);
		}

		mNeedsRAWMemoryBarrier = false;
		mNeedsWARMemoryBarrier = false;
		mMemoryBarrierSrcStages = 0;
		mMemoryBarrierDstStages = 0;
		mMemoryBarrierSrcAccess = 0;
		mMemoryBarrierDstAccess = 0;

		for(auto& entry : mShaderBoundSubresourceInfos)
		{
			ImageSubresourceInfo& subresourceInfo = mSubresourceInfoStorage[entry];
			subresourceInfo.writeHazardUse.access = VulkanAccessFlag::None;
			subresourceInfo.writeHazardUse.stages = 0;
		}

		// Note: I should keep track of any buffers modified and only clear those, to prevent excess iteration
		for(auto& entry : mBuffers)
		{
			BufferInfo& bufferInfo = entry.second;
			bufferInfo.writeHazardUse.access = VulkanAccessFlag::None;
			bufferInfo.writeHazardUse.stages = 0;
		}
	}

	void VulkanCmdBuffer::updateFinalLayouts()
	{
		if (mFramebuffer == nullptr)
			return;

		UINT32 numColorAttachments = mFramebuffer->getNumColorAttachments();
		for (UINT32 i = 0; i < numColorAttachments; i++)
		{
			const VulkanFramebufferAttachment& fbAttachment = mFramebuffer->getColorAttachment(i);
			ImageSubresourceInfo& subresourceInfo = findSubresourceInfo(fbAttachment.image, fbAttachment.surface.face,
																		fbAttachment.surface.mipLevel);

			subresourceInfo.currentLayout = subresourceInfo.renderPassLayout;
			subresourceInfo.requiredLayout = subresourceInfo.renderPassLayout;
		}

		if (mFramebuffer->hasDepthAttachment())
		{
			const VulkanFramebufferAttachment& fbAttachment = mFramebuffer->getDepthStencilAttachment();
			ImageSubresourceInfo& subresourceInfo = findSubresourceInfo(fbAttachment.image, fbAttachment.surface.face,
																		fbAttachment.surface.mipLevel);

			subresourceInfo.currentLayout = subresourceInfo.renderPassLayout;
			subresourceInfo.requiredLayout = subresourceInfo.renderPassLayout;
		}
	}

	void VulkanCmdBuffer::executeClearPass()
	{
		assert(mState == State::Recording);

		executeWriteHazardBarrier();
		executeLayoutTransitions();

		VkRenderPassBeginInfo renderPassBeginInfo;
		renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassBeginInfo.pNext = nullptr;
		renderPassBeginInfo.framebuffer = mFramebuffer->getFramebuffer(mRenderTargetLoadMask, RT_NONE, mClearMask);
		renderPassBeginInfo.renderPass = mFramebuffer->getRenderPass(mRenderTargetLoadMask, RT_NONE, mClearMask);
		renderPassBeginInfo.renderArea.offset.x = mClearArea.x;
		renderPassBeginInfo.renderArea.offset.y = mClearArea.y;
		renderPassBeginInfo.renderArea.extent.width = mClearArea.width;
		renderPassBeginInfo.renderArea.extent.height = mClearArea.height;
		renderPassBeginInfo.clearValueCount = mFramebuffer->getNumClearEntries(mClearMask);
		renderPassBeginInfo.pClearValues = mClearValues.data();

		vkCmdBeginRenderPass(mCmdBuffer, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
		vkCmdEndRenderPass(mCmdBuffer);

		updateFinalLayouts();

		mClearMask = CLEAR_NONE;
	}

	void VulkanCmdBuffer::draw(UINT32 vertexOffset, UINT32 vertexCount, UINT32 instanceCount)
	{
		if (!isReadyForRender())
			return;

		// Need to bind gpu params before starting render pass, in order to make sure any layout transitions execute
		bindGpuParams();

		if (!isInRenderPass())
			beginRenderPass();

		if(mVertexInputsDirty)
		{
			bindVertexInputs();
			mVertexInputsDirty = false;
		}

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

		if (instanceCount <= 0)
			instanceCount = 1;

		vkCmdDraw(mCmdBuffer, vertexCount, instanceCount, vertexOffset, 0);
	}

	void VulkanCmdBuffer::drawIndexed(UINT32 startIndex, UINT32 indexCount, UINT32 vertexOffset, UINT32 instanceCount)
	{
		if (!isReadyForRender())
			return;

		// Need to bind gpu params before starting render pass, in order to make sure any layout transitions execute
		bindGpuParams();

		if (!isInRenderPass())
			beginRenderPass();

		if(mVertexInputsDirty)
		{
			bindVertexInputs();
			mVertexInputsDirty = false;
		}

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

		if (instanceCount <= 0)
			instanceCount = 1;

		vkCmdDrawIndexed(mCmdBuffer, indexCount, instanceCount, startIndex, vertexOffset, 0);
	}

	void VulkanCmdBuffer::dispatch(UINT32 numGroupsX, UINT32 numGroupsY, UINT32 numGroupsZ)
	{
		if (mComputePipeline == nullptr)
			return;

		if (isInRenderPass())
			endRenderPass();

		// Note: Should I restore the render target after? Note that this is only being done is framebuffer subresources
		// have their "isFBAttachment" flag reset, potentially I can just clear/restore those
		setRenderTarget(nullptr, 0, RT_ALL);

		// Need to bind gpu params before starting render pass, in order to make sure any layout transitions execute
		bindGpuParams();
		executeWriteHazardBarrier();
		executeLayoutTransitions();

		UINT32 deviceIdx = mDevice.getIndex();
		if(mCmpPipelineRequiresBind)
		{
			VulkanPipeline* pipeline = mComputePipeline->getPipeline(deviceIdx);
			if (pipeline == nullptr)
				return;

			registerResource(pipeline, VulkanAccessFlag::Read);
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

		// Remove any shader use flags on images. Note this relies on the fact that we re-bind all parameters on every
		// dispatch call and render pass, so they can reset this flags. Otherwise clearing the flags is wrong if the
		// images remain to be used in subsequent calls).
		for(auto& entry : mShaderBoundSubresourceInfos)
		{
			ImageSubresourceInfo& subresourceInfo = mSubresourceInfoStorage[entry];
			subresourceInfo.useFlags.unset(ImageUseFlagBits::Shader);
			subresourceInfo.shaderUse.access = VulkanAccessFlag::None;
			subresourceInfo.shaderUse.stages = 0;
		}

		mShaderBoundSubresourceInfos.clear();
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

	void VulkanCmdBuffer::memoryBarrier(VkBuffer buffer, VkAccessFlags srcAccessFlags, VkAccessFlags dstAccessFlags,
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

		vkCmdPipelineBarrier(getHandle(),
							 srcStage,
							 dstStage,
							 0, 0, nullptr,
							 1, &barrier,
							 0, nullptr);
	}

	void VulkanCmdBuffer::setLayout(VkImage image, VkAccessFlags srcAccessFlags, VkAccessFlags dstAccessFlags,
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

		VkPipelineStageFlags srcStage = getPipelineStageFlags(srcAccessFlags);
		VkPipelineStageFlags dstStage = getPipelineStageFlags(dstAccessFlags);
		
		vkCmdPipelineBarrier(getHandle(),
							 srcStage,
							 dstStage,
							 0, 0, nullptr,
							 0, nullptr,
							 1, &barrier);
	}

	VkImageLayout VulkanCmdBuffer::getCurrentLayout(VulkanImage* image, const VkImageSubresourceRange& range, 
		bool inRenderPass)
	{
		UINT32 face = range.baseArrayLayer;
		UINT32 mip = range.baseMipLevel;

		// The assumption is that all the subresources in the range will have the same layout, as this should be handled
		// by registerResource(), or by external code (in the case of transfers). So we only check the first subresource.
		VulkanImageSubresource* subresource = image->getSubresource(face, mip);

		auto iterFind = mImages.find(image);
		if (iterFind == mImages.end())
			return subresource->getLayout();

		UINT32 imageInfoIdx = iterFind->second;
		ImageInfo& imageInfo = mImageInfos[imageInfoIdx];

		ImageSubresourceInfo* subresourceInfos = &mSubresourceInfoStorage[imageInfo.subresourceInfoIdx];
		for(UINT32 i = 0; i < imageInfo.numSubresourceInfos; i++)
		{
			ImageSubresourceInfo& entry = subresourceInfos[i];
			if(face >= entry.range.baseArrayLayer && face < (entry.range.baseArrayLayer + entry.range.layerCount) &&
			   mip >= entry.range.baseMipLevel && mip < (entry.range.baseMipLevel + entry.range.levelCount))
			{
				// If it's a FB attachment, retrieve its layout after the render pass begins
				if(entry.useFlags.isSet(ImageUseFlagBits::Framebuffer) && inRenderPass && mFramebuffer)
				{
					RenderSurfaceMask readMask = getFBReadMask();

					// Is it a depth-stencil attachment?
					if(mFramebuffer->hasDepthAttachment() && mFramebuffer->getDepthStencilAttachment().image == image)
					{
						if (readMask.isSet(RT_DEPTH))
						{
							if (readMask.isSet(RT_STENCIL))
								return VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
							else // Depth readable but stencil isn't
								return VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_STENCIL_ATTACHMENT_OPTIMAL_KHR;
						}
						else
						{
							if (readMask.isSet(RT_STENCIL)) // Stencil readable but depth isn't
								return VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_STENCIL_READ_ONLY_OPTIMAL_KHR;
							else 
								return VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
							
						}
					}
					else // It is a color attachment
					{
						UINT32 numColorAttachments = mFramebuffer->getNumColorAttachments();
						for (UINT32 j = 0; j < numColorAttachments; j++)
						{
							const VulkanFramebufferAttachment& attachment = mFramebuffer->getColorAttachment(j);

							if (attachment.image == image)
							{
								if (readMask.isSet((RenderSurfaceMaskBits)(1 << attachment.index)))
									return VK_IMAGE_LAYOUT_GENERAL;
								else
									return VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
							}
						}
					}
				}

				return entry.requiredLayout;
			}
		}

		return subresource->getLayout();
	}

	void VulkanCmdBuffer::registerResource(VulkanResource* res, VulkanAccessFlags flags)
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

	void VulkanCmdBuffer::registerImageShader(VulkanImage* image, const VkImageSubresourceRange& range, 
		VkImageLayout layout, VulkanAccessFlags access, VkPipelineStageFlags stages)
	{
		assert(layout != VK_IMAGE_LAYOUT_UNDEFINED);
		registerResource(image, range, ImageUseFlagBits::Shader, layout, layout, access, stages);
	}

	void VulkanCmdBuffer::registerImageFramebuffer(VulkanImage* image, const VkImageSubresourceRange& range, 
		VkImageLayout layout, VkImageLayout finalLayout, VulkanAccessFlags access, VkPipelineStageFlags stages)
	{
		registerResource(image, range, ImageUseFlagBits::Framebuffer, layout, finalLayout, access, stages);
	}

	void VulkanCmdBuffer::registerImageTransfer(VulkanImage* image, const VkImageSubresourceRange& range, 
		VkImageLayout layout, VulkanAccessFlags access)
	{
		assert(layout != VK_IMAGE_LAYOUT_UNDEFINED);
		registerResource(image, range, ImageUseFlagBits::Transfer, layout, layout, access, VK_PIPELINE_STAGE_TRANSFER_BIT);
	}

	void VulkanCmdBuffer::registerResource(VulkanImage* image, const VkImageSubresourceRange& range, ImageUseFlagBits use, 
		VkImageLayout layout, VkImageLayout finalLayout, VulkanAccessFlags access, VkPipelineStageFlags stages)
	{
		// This function either registers a brand new image resource that was never been used on this command buffer, or
		// if the resource has been used previously then it calculates the overlapping subresource sets and calls a relevant
		// function and further determines if any layout transitions and/or memory/execution barriers are necessary.

		UINT32 nextImageInfoIdx = (UINT32)mImageInfos.size();
		auto registerSubresourceInfo = [&](const VkImageSubresourceRange& subresourceRange)
		{
			mSubresourceInfoStorage.push_back(ImageSubresourceInfo());
			ImageSubresourceInfo& subresourceInfo = mSubresourceInfoStorage.back();
			subresourceInfo.currentLayout = layout;
			subresourceInfo.initialLayout = layout;
			subresourceInfo.initialReadOnly = !access.isSet(VulkanAccessFlag::Write);
			subresourceInfo.requiredLayout = layout;
			subresourceInfo.renderPassLayout = finalLayout;
			subresourceInfo.range = subresourceRange;

			switch(use)
			{
			default:
			case ImageUseFlagBits::Shader: 
				subresourceInfo.shaderUse.access = access;
				subresourceInfo.shaderUse.stages = stages;
				subresourceInfo.writeHazardUse.access = access;
				subresourceInfo.writeHazardUse.stages = stages;
				break;
			case ImageUseFlagBits::Framebuffer: 
				subresourceInfo.fbUse.access = access;
				subresourceInfo.fbUse.stages = stages;
				break;
			case ImageUseFlagBits::Transfer: 
				subresourceInfo.transferUse.access = access;
				subresourceInfo.transferUse.stages = stages;
				break;
			}

			subresourceInfo.useFlags = use;

			if (use == ImageUseFlagBits::Shader)
				mShaderBoundSubresourceInfos.insert((UINT32)mSubresourceInfoStorage.size() - 1);
		};

		auto insertResult = mImages.insert(std::make_pair(image, nextImageInfoIdx));
		if (insertResult.second) // New element
		{
			UINT32 imageInfoIdx = insertResult.first->second;
			mImageInfos.push_back(ImageInfo());

			ImageInfo& imageInfo = mImageInfos[imageInfoIdx];
			imageInfo.subresourceInfoIdx = (UINT32)mSubresourceInfoStorage.size();
			imageInfo.numSubresourceInfos = 1;

			imageInfo.useHandle.used = false;
			imageInfo.useHandle.flags = access;

			registerSubresourceInfo(range);

			image->notifyBound();
		}
		else // Existing element
		{
			UINT32 imageInfoIdx = insertResult.first->second;
			ImageInfo& imageInfo = mImageInfos[imageInfoIdx];

			assert(!imageInfo.useHandle.used);
			imageInfo.useHandle.flags |= access;

			// See if there is an overlap between existing ranges and the new range. And if so break them up accordingly.
			//// First test for the simplest and most common case (same range or no overlap) to avoid more complex
			//// computations.
			ImageSubresourceInfo* subresources = &mSubresourceInfoStorage[imageInfo.subresourceInfoIdx];

			bool foundRange = false;
			for(UINT32 i = 0; i < imageInfo.numSubresourceInfos; i++)
			{
				if(VulkanUtility::rangeOverlaps(subresources[i].range, range))
				{
					if (subresources[i].range.layerCount == range.layerCount &&
						subresources[i].range.levelCount == range.levelCount &&
						subresources[i].range.baseArrayLayer == range.baseArrayLayer &&
						subresources[i].range.baseMipLevel == range.baseMipLevel)
					{
						// Just update existing range
						switch(use)
						{
						default:
						case ImageUseFlagBits::Shader: 
							updateShaderSubresource(image, imageInfoIdx, subresources[i], layout, access, stages);
							break;
						case ImageUseFlagBits::Framebuffer: 
							updateFramebufferSubresource(image, imageInfoIdx, subresources[i], layout, finalLayout, access, 
								stages);
							break;
						case ImageUseFlagBits::Transfer: 
							updateTransferSubresource(image, imageInfoIdx, subresources[i], layout, access, stages);
							break;
						}

						if (use == ImageUseFlagBits::Shader)
							mShaderBoundSubresourceInfos.insert(imageInfo.subresourceInfoIdx + i);

						foundRange = true;
						break;
					}

					break;
				}
			}

			//// We'll need to update subresource ranges or add new ones. The hope is that this code is trigger VERY rarely
			//// (for just a few specific textures per frame).
			if (!foundRange)
			{
				std::array<VkImageSubresourceRange, 5> tempCutRanges;

				bs_frame_mark();
				{
					// We orphan previously allocated memory (we reset it after submit() anyway)
					UINT32 newSubresourceIdx = (UINT32)mSubresourceInfoStorage.size();

					FrameVector<UINT32> cutOverlappingRanges;
					for (UINT32 i = 0; i < imageInfo.numSubresourceInfos; i++)
					{
						UINT32 subresourceIdx = imageInfo.subresourceInfoIdx + i;
						ImageSubresourceInfo& subresource = mSubresourceInfoStorage[subresourceIdx];

						if (!VulkanUtility::rangeOverlaps(subresource.range, range))
						{
							// Just copy as is
							mSubresourceInfoStorage.push_back(subresource);

							if (use == ImageUseFlagBits::Shader)
								mShaderBoundSubresourceInfos.insert((UINT32)mSubresourceInfoStorage.size() - 1);
						}
						else // Need to cut
						{
							UINT32 numCutRanges;
							VulkanUtility::cutRange(subresource.range, range, tempCutRanges, numCutRanges);

							for(UINT32 j = 0; j < numCutRanges; j++)
							{
								// Create a copy of the original subresource with the new range
								ImageSubresourceInfo newInfo = mSubresourceInfoStorage[subresourceIdx];
								newInfo.range = tempCutRanges[j];

								if(VulkanUtility::rangeOverlaps(tempCutRanges[j], range))
								{
									// Update overlapping sub-resource range with new data from this range
									switch (use)
									{
									default:
									case ImageUseFlagBits::Shader:
										updateShaderSubresource(image, imageInfoIdx, newInfo, layout, access, stages);
										break;
									case ImageUseFlagBits::Framebuffer:
										updateFramebufferSubresource(image, imageInfoIdx, newInfo, layout, finalLayout, access,
											stages);
										break;
									case ImageUseFlagBits::Transfer:
										updateTransferSubresource(image, imageInfoIdx, newInfo, layout, access, stages);
										break;
									}

									// Keep track of the overlapping ranges for later
									cutOverlappingRanges.push_back((UINT32)mSubresourceInfoStorage.size());
								}

								mSubresourceInfoStorage.push_back(newInfo);

								if(use == ImageUseFlagBits::Shader)
									mShaderBoundSubresourceInfos.insert((UINT32)mSubresourceInfoStorage.size() - 1);
							}
						}
					}

					// Our range doesn't overlap with any existing ranges, so just add it
					if(cutOverlappingRanges.empty())
					{
						registerSubresourceInfo(range);
					}
					else // Search if overlapping ranges fully cover the requested range, and insert non-covered regions
					{
						FrameQueue<VkImageSubresourceRange> sourceRanges;
						sourceRanges.push(range);

						for(auto& entry : cutOverlappingRanges)
						{
							VkImageSubresourceRange& overlappingRange = mSubresourceInfoStorage[entry].range;

							UINT32 numSourceRanges = (UINT32)sourceRanges.size();
							for(UINT32 i = 0; i < numSourceRanges; i++)
							{
								VkImageSubresourceRange sourceRange = sourceRanges.front();
								sourceRanges.pop();

								UINT32 numCutRanges;
								VulkanUtility::cutRange(sourceRange, overlappingRange, tempCutRanges, numCutRanges);

								for(UINT32 j = 0; j < numCutRanges; j++)
								{
									// We only care about ranges outside of the ones we already covered
									if(!VulkanUtility::rangeOverlaps(tempCutRanges[j], overlappingRange))
										sourceRanges.push(tempCutRanges[j]);
								}
							}
						}

						// Any remaining range hasn't been covered yet
						while(!sourceRanges.empty())
						{
							registerSubresourceInfo(sourceRanges.front());
							sourceRanges.pop();
						}
					}

					imageInfo.subresourceInfoIdx = newSubresourceIdx;
					imageInfo.numSubresourceInfos = (UINT32)mSubresourceInfoStorage.size() - newSubresourceIdx;
				}
				bs_frame_clear();
			}
		}

		// Register any sub-resources
		for(UINT32 i = 0; i < range.layerCount; i++)
		{
			for(UINT32 j = 0; j < range.levelCount; j++)
			{
				UINT32 layer = range.baseArrayLayer + i;
				UINT32 mipLevel = range.baseMipLevel + j;

				registerResource(image->getSubresource(layer, mipLevel), access);
			}
		}
	}

	void VulkanCmdBuffer::registerBuffer(VulkanBuffer* res, BufferUseFlagBits useFlags, VulkanAccessFlags access, 
		VkPipelineStageFlags stages)
	{
		switch(useFlags)
		{
		case BufferUseFlagBits::Index: 
			stages = VK_ACCESS_INDEX_READ_BIT;
			break;
		case BufferUseFlagBits::Vertex: 
			stages = VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT;
			break;
		case BufferUseFlagBits::Transfer:
			stages = 0;

			if(access.isSet(VulkanAccessFlag::Read))
				stages |= VK_ACCESS_TRANSFER_READ_BIT;

			if(access.isSet(VulkanAccessFlag::Write))
				stages |= VK_ACCESS_TRANSFER_WRITE_BIT;
			break;
		default: 
			break;
		}

		auto insertResult = mBuffers.insert(std::make_pair(res, BufferInfo()));
		if (insertResult.second) // New element
		{
			BufferInfo& bufferInfo = insertResult.first->second;
			bufferInfo.useFlags = useFlags;

			bufferInfo.useHandle.used = false;
			bufferInfo.useHandle.flags = access;

			// Transfer write hazards are handled externally
			if(useFlags != BufferUseFlagBits::Transfer)
			{
				bufferInfo.writeHazardUse.access = access;
				bufferInfo.writeHazardUse.stages = stages;
			}

			res->notifyBound();
		}
		else // Existing element
		{
			BufferInfo& bufferInfo = insertResult.first->second;

			assert(!bufferInfo.useHandle.used);

			// Transfer write hazards are handled externally
			bool resetRenderPass = false;
			if(useFlags != BufferUseFlagBits::Transfer)
			{
				// If this buffer has been previously written to prevent read-after-write and write-after-read hazards
				if (access.isSetAny(VulkanAccessFlag::Read | VulkanAccessFlag::Write))
				{
					// Read-after-write (and write-after-write, as little sense does that make)
					if (bufferInfo.writeHazardUse.access.isSet(VulkanAccessFlag::Write))
					{
						mNeedsRAWMemoryBarrier = true;
						mMemoryBarrierSrcStages |= bufferInfo.writeHazardUse.stages;
						mMemoryBarrierDstStages |= stages;
						mMemoryBarrierSrcAccess |= VK_ACCESS_SHADER_WRITE_BIT;

						switch (useFlags)
						{
						case BufferUseFlagBits::Generic:
							if (access.isSet(VulkanAccessFlag::Read))
								mMemoryBarrierDstAccess |= VK_ACCESS_SHADER_READ_BIT;

							if (access.isSet(VulkanAccessFlag::Write))
								mMemoryBarrierDstAccess |= VK_ACCESS_SHADER_WRITE_BIT;
							break;
						case BufferUseFlagBits::Index:
							mMemoryBarrierDstAccess |= VK_ACCESS_INDEX_READ_BIT;
							break;
						case BufferUseFlagBits::Vertex:
							mMemoryBarrierDstAccess |= VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT;
							break;
						case BufferUseFlagBits::Parameter:
							mMemoryBarrierDstAccess |= VK_ACCESS_UNIFORM_READ_BIT;
							break;
						case BufferUseFlagBits::Transfer:
							if (access.isSet(VulkanAccessFlag::Read))
								mMemoryBarrierDstAccess |= VK_ACCESS_TRANSFER_READ_BIT;

							if (access.isSet(VulkanAccessFlag::Write))
								mMemoryBarrierDstAccess |= VK_ACCESS_TRANSFER_WRITE_BIT;
							break;
						}

						// End render pass as we perform memory barriers at the beggining a render pass/dispatch call, so this
						// will force them to execute
						resetRenderPass = true;
					}
				}

				if (access.isSet(VulkanAccessFlag::Write))
				{
					// Write-after-read
					if (bufferInfo.writeHazardUse.access.isSet(VulkanAccessFlag::Read))
					{
						mNeedsWARMemoryBarrier = true;
						mMemoryBarrierSrcStages |= bufferInfo.writeHazardUse.stages;
						mMemoryBarrierDstStages |= stages;

						// End render pass as we perform memory barriers at the beggining a render pass/dispatch call, so this
						// will force them to execute
						resetRenderPass = true;
					}
				}

				bufferInfo.writeHazardUse.access |= access;
				bufferInfo.writeHazardUse.stages |= stages;
			}

			bufferInfo.useHandle.flags |= access;
			bufferInfo.useFlags |= useFlags;

			// Need to end render pass in order to execute the barrier. Hopefully this won't trigger much since most
			// shader writes are done during compute
			if (resetRenderPass && isInRenderPass())
				endRenderPass();
		}
	}

	void VulkanCmdBuffer::registerResource(VulkanFramebuffer* res, RenderSurfaceMask loadMask, UINT32 readMask)
	{
		auto insertResult = mResources.insert(std::make_pair(res, ResourceUseHandle()));
		if (insertResult.second) // New element
		{
			ResourceUseHandle& useHandle = insertResult.first->second;
			useHandle.used = false;
			useHandle.flags = VulkanAccessFlag::Write;

			res->notifyBound();
		}
		else // Existing element
		{
			ResourceUseHandle& useHandle = insertResult.first->second;

			assert(!useHandle.used);
			useHandle.flags |= VulkanAccessFlag::Write;
		}

		// Register any sub-resources
		UINT32 numColorAttachments = res->getNumColorAttachments();
		for (UINT32 i = 0; i < numColorAttachments; i++)
		{
			const VulkanFramebufferAttachment& attachment = res->getColorAttachment(i);

			// If image is being loaded, we need to transfer it to correct layout, otherwise it doesn't matter. We're using
			// these values because that's what VulkanFramebuffer expects as initialLayout.
			VkImageLayout layout;
			if (loadMask.isSet((RenderSurfaceMaskBits)(1 << i)))
				layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
			else
				layout = VK_IMAGE_LAYOUT_UNDEFINED;

			VulkanAccessFlag access = ((readMask & FBT_COLOR) != 0) ? VulkanAccessFlag::Read : VulkanAccessFlag::Write;

			VkImageSubresourceRange range = attachment.image->getRange(attachment.surface);
			registerImageFramebuffer(attachment.image, range, layout, attachment.finalLayout, access, 
				VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT);
		}

		if(res->hasDepthAttachment())
		{
			const VulkanFramebufferAttachment& attachment = res->getDepthStencilAttachment();
			VulkanAccessFlag useFlag = VulkanAccessFlag::Write;

			// If image is being loaded, we need to transfer it to correct layout, otherwise it doesn't matter. We're using
			// these values because that's what VulkanFramebuffer expects as initialLayout.
			VkImageLayout layout;
			if (loadMask.isSet(RT_DEPTH) || loadMask.isSet(RT_STENCIL)) // Can't load one without the other
				layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
			else
				layout = VK_IMAGE_LAYOUT_UNDEFINED;

			VulkanAccessFlag access = (((readMask & FBT_DEPTH) != 0) && ((readMask & FBT_STENCIL) != 0)) ? 
				VulkanAccessFlag::Read : VulkanAccessFlag::Write;

			VkImageSubresourceRange range = attachment.image->getRange(attachment.surface);
			registerImageFramebuffer(attachment.image, range, layout, attachment.finalLayout, access,
				VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT);
		}
	}

	void VulkanCmdBuffer::registerResource(VulkanSwapChain* res)
	{
		auto insertResult = mSwapChains.insert(std::make_pair(res, ResourceUseHandle()));
		if (insertResult.second) // New element
		{
			ResourceUseHandle& useHandle = insertResult.first->second;
			useHandle.used = false;
			useHandle.flags = VulkanAccessFlag::Write;

			res->notifyBound();
		}
		else // Existing element
		{
			ResourceUseHandle& useHandle = insertResult.first->second;

			assert(!useHandle.used);
			useHandle.flags |= VulkanAccessFlag::Write;
		}
	}

	void VulkanCmdBuffer::updateShaderSubresource(VulkanImage* image, UINT32 imageInfoIdx, 
		ImageSubresourceInfo& subresourceInfo, VkImageLayout layout, VulkanAccessFlags access, VkPipelineStageFlags stages)
	{
		// New layout is valid, check for transitions (UNDEFINED signifies the caller doesn't want a layout transition)
		if (layout != VK_IMAGE_LAYOUT_UNDEFINED)
		{
			// Register the necessary layout transition, but only if the image isn't bound for framebuffer bind. If it is
			// then we are forced to use the layout that's expected by the framebuffer. 

			if(subresourceInfo.useFlags.isSet(ImageUseFlagBits::Framebuffer))
			{
				// Currently the system doesn't support image being bound to framebuffer, yet being written to by the 
				// shader. This seems like an unlikely scenario.
				assert(!access.isSet(VulkanAccessFlag::Write));
			}
			else
			{
				// Check if the image had a layout previously assigned, and if so check if multiple different layouts
				// were requested. In that case we wish to transfer the image to GENERAL layout.

				bool firstUseInRenderPass = !subresourceInfo.useFlags.isSetAny(
					ImageUseFlagBits::Shader | ImageUseFlagBits::Framebuffer);
				if (firstUseInRenderPass || subresourceInfo.requiredLayout == VK_IMAGE_LAYOUT_UNDEFINED)
					subresourceInfo.requiredLayout = layout;
				else if (subresourceInfo.requiredLayout != layout)
					subresourceInfo.requiredLayout = VK_IMAGE_LAYOUT_GENERAL;
			}
		}

		if (subresourceInfo.currentLayout != subresourceInfo.requiredLayout)
			mQueuedLayoutTransitions[image] = imageInfoIdx;

		// If a FB attachment was just bound as a shader input, we might need to restart the render pass with a FB
		// attachment that supports read-only attachments using the GENERAL or DEPTH_READ_ONLY layout
		bool resetRenderPass = false;
		if (!subresourceInfo.useFlags.isSet(ImageUseFlagBits::Shader))
		{
			if (subresourceInfo.useFlags.isSet(ImageUseFlagBits::Framebuffer))
			{
				// Special case for depth: If user has set up proper read-only flags, then the render pass will have
				// taken care of setting the valid state anyway, so no need to end the render pass
				if (subresourceInfo.requiredLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL)
				{
					resetRenderPass = ((mRenderTargetReadOnlyFlags & FBT_DEPTH) == 0 && 
						(mRenderTargetReadOnlyFlags & FBT_STENCIL) == 0);
				}
				else
					resetRenderPass = true;
			}
		}

		// If this image has been previously used prevent read-after-write and write-after-read hazards
		if(access.isSetAny(VulkanAccessFlag::Read | VulkanAccessFlag::Write))
		{
			// Read-after-write (and write-after-write, as little sense does that make)
			if(subresourceInfo.writeHazardUse.access.isSet(VulkanAccessFlag::Write))
			{
				mNeedsRAWMemoryBarrier = true;
				mMemoryBarrierSrcStages |= subresourceInfo.writeHazardUse.stages;
				mMemoryBarrierDstStages |= stages;
				mMemoryBarrierSrcAccess |= VK_ACCESS_SHADER_WRITE_BIT;

				if(access.isSet(VulkanAccessFlag::Read))
					mMemoryBarrierDstAccess |= VK_ACCESS_SHADER_READ_BIT;

				if(access.isSet(VulkanAccessFlag::Write))
					mMemoryBarrierDstAccess |= VK_ACCESS_SHADER_WRITE_BIT;

				// End render pass as we perform memory barriers at the beggining a render pass/dispatch call, so this
				// will force them to execute
				resetRenderPass = true;
			}
		}

		if(access.isSet(VulkanAccessFlag::Write))
		{
			// Write-after-read
			if(subresourceInfo.writeHazardUse.access.isSet(VulkanAccessFlag::Read))
			{
				mNeedsWARMemoryBarrier = true;
				mMemoryBarrierSrcStages |= subresourceInfo.writeHazardUse.stages;
				mMemoryBarrierDstStages |= stages;

				// End render pass as we perform memory barriers at the beggining a render pass/dispatch call, so this
				// will force them to execute
				resetRenderPass = true;
			}
		}

		subresourceInfo.shaderUse.access |= access;
		subresourceInfo.shaderUse.stages |= stages;

		subresourceInfo.writeHazardUse.access |= access;
		subresourceInfo.writeHazardUse.stages |= stages;

		subresourceInfo.useFlags |= ImageUseFlagBits::Shader;

		// If we need to switch frame-buffers or execute memory barriers, end current render pass
		if (resetRenderPass && isInRenderPass())
			endRenderPass();
	}

	void VulkanCmdBuffer::updateFramebufferSubresource(VulkanImage* image, UINT32 imageInfoIdx, 
		ImageSubresourceInfo& subresourceInfo, VkImageLayout layout, VkImageLayout finalLayout, VulkanAccessFlags access, 
		VkPipelineStageFlags stages)
	{
		// Framebuffer expects a certain layout and we must respect it. In the case when the FB attachment is also bound 
		// for shader reads, this will override the layout required for shader read (GENERAL or DEPTH_READ_ONLY), but that 
		// is fine because those transitions are handled automatically by render-pass layout transitions.
		subresourceInfo.requiredLayout = layout;
		subresourceInfo.renderPassLayout = finalLayout;

		if (subresourceInfo.currentLayout != subresourceInfo.requiredLayout)
			mQueuedLayoutTransitions[image] = imageInfoIdx;

		// If the FB attachment was previously bound as a shader input, we might need to restart the render pass with a FB
		// attachment that supports read-only attachments using the GENERAL or DEPTH_READ_ONLY layout
		bool resetRenderPass = false;
		if (!subresourceInfo.useFlags.isSet(ImageUseFlagBits::Framebuffer))
			resetRenderPass = subresourceInfo.useFlags.isSet(ImageUseFlagBits::Shader);

		// If this image has been previously written to be a shader prevent read-after-write and write-after-write hazards
		// Note: This could be handled through sub-pass dependencies instead of explicit memory barriers, but those require
		// different render pass objects depending on access/stage flags, which is probably more overhead than just 
		// executing the explicit barrier.
		if(access.isSetAny(VulkanAccessFlag::Read | VulkanAccessFlag::Write))
		{
			// Read-after-write
			if(subresourceInfo.writeHazardUse.access.isSet(VulkanAccessFlag::Write))
			{
				mNeedsRAWMemoryBarrier = true;
				mMemoryBarrierSrcStages |= subresourceInfo.writeHazardUse.stages;
				mMemoryBarrierDstStages |= stages;
				mMemoryBarrierSrcAccess |= VK_ACCESS_SHADER_WRITE_BIT;

				if(access.isSet(VulkanAccessFlag::Read))
				{
					if ((stages & VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT) != 0)
						mMemoryBarrierDstAccess |= VK_ACCESS_COLOR_ATTACHMENT_READ_BIT;

					if ((stages & VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT) != 0 ||
						(stages & VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT) != 0)
						mMemoryBarrierDstAccess |= VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT;
				}

				if(access.isSet(VulkanAccessFlag::Write))
				{
					if ((stages & VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT) != 0)
						mMemoryBarrierDstAccess |= VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

					if ((stages & VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT) != 0 ||
						(stages & VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT) != 0)
						mMemoryBarrierDstAccess |= VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
				}

				// End render pass as we perform memory barriers at the beggining a render pass/dispatch call, so this
				// will force them to execute
				resetRenderPass = true;
			}
		}

		// No need to check for write-after-read barrier as it only needs an execution dependency and that is already 
		// handled by the render pass

		subresourceInfo.fbUse.access |= access;
		subresourceInfo.fbUse.stages |= stages;

		subresourceInfo.useFlags |= ImageUseFlagBits::Framebuffer;

		// If we need to switch frame-buffers or execute memory barriers, end current render pass
		if (resetRenderPass && isInRenderPass())
			endRenderPass();
	}

	void VulkanCmdBuffer::updateTransferSubresource(VulkanImage* image, UINT32 imageInfoIdx, 
		ImageSubresourceInfo& subresourceInfo, VkImageLayout layout, VulkanAccessFlags access, VkPipelineStageFlags stages)
	{
		// Note: Currently it is assumed that all images submitted for a transfer operation will have their pre-operation
		// layout set externally. 	
		
		subresourceInfo.currentLayout = layout;
		subresourceInfo.requiredLayout = layout;

		// These are currently not used nor cleared
		subresourceInfo.transferUse.access |= access;
		subresourceInfo.transferUse.stages |= stages;

		subresourceInfo.useFlags |= ImageUseFlagBits::Transfer;
	}

	VulkanCmdBuffer::ImageSubresourceInfo& VulkanCmdBuffer::findSubresourceInfo(VulkanImage* image, UINT32 face, UINT32 mip)
	{
		UINT32 imageInfoIdx = mImages[image];
		ImageInfo& imageInfo = mImageInfos[imageInfoIdx];

		ImageSubresourceInfo* subresourceInfos = &mSubresourceInfoStorage[imageInfo.subresourceInfoIdx];
		for(UINT32 i = 0; i < imageInfo.numSubresourceInfos; i++)
		{
			ImageSubresourceInfo& entry = subresourceInfos[i];
			if(face >= entry.range.baseArrayLayer && face < (entry.range.baseArrayLayer + entry.range.layerCount) &&
			   mip >= entry.range.baseMipLevel && mip < (entry.range.baseMipLevel + entry.range.levelCount))
			{
				return entry;
			}
		}

		assert(false); // Caller should ensure the subresource actually exists, so this shouldn't happen
		return subresourceInfos[0];
	}

	void VulkanCmdBuffer::getInProgressQueries(Vector<VulkanTimerQuery*>& timer, Vector<VulkanOcclusionQuery*>& occlusion) const
	{
		for(auto& query : mTimerQueries)
		{
			if (query->_isInProgress())
				timer.push_back(query);
		}

		for (auto& query : mOcclusionQueries)
		{
			if (query->_isInProgress())
				occlusion.push_back(query);
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

	RenderSurfaceMask VulkanCmdBuffer::getFBReadMask()
	{
		// Check if any frame-buffer attachments are also used as shader inputs, in which case we make them read-only
		RenderSurfaceMask readMask = RT_NONE;

		UINT32 numColorAttachments = mFramebuffer->getNumColorAttachments();
		for(UINT32 i = 0; i < numColorAttachments; i++)
		{
			const VulkanFramebufferAttachment& fbAttachment = mFramebuffer->getColorAttachment(i);
			ImageSubresourceInfo& subresourceInfo = findSubresourceInfo(fbAttachment.image, fbAttachment.surface.face,
				fbAttachment.surface.mipLevel);

			bool readOnly = subresourceInfo.useFlags.isSet(ImageUseFlagBits::Shader);

			if(readOnly)
				readMask.set((RenderSurfaceMaskBits)(1 << i));
		}

		if (mFramebuffer->hasDepthAttachment())
		{
			const VulkanFramebufferAttachment& fbAttachment = mFramebuffer->getDepthStencilAttachment();
			ImageSubresourceInfo& subresourceInfo = findSubresourceInfo(fbAttachment.image, fbAttachment.surface.face,
				fbAttachment.surface.mipLevel);

			bool readOnly = subresourceInfo.useFlags.isSet(ImageUseFlagBits::Shader);

			if (readOnly)
				readMask.set(RT_DEPTH);

			if ((mRenderTargetReadOnlyFlags & FBT_DEPTH) != 0)
				readMask.set(RT_DEPTH);

			if ((mRenderTargetReadOnlyFlags & FBT_STENCIL) != 0)
				readMask.set(RT_STENCIL);
		}

		return readMask;
	}


	VulkanCommandBuffer::~VulkanCommandBuffer()
	{
		mBuffer->reset();
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

		// Execute any queued layout transitions that weren't already handled by the render pass
		mBuffer->executeLayoutTransitions();

		// Interrupt any in-progress queries (no in-progress queries allowed during command buffer submit)
		Vector<VulkanTimerQuery*> timerQueries;
		Vector<VulkanOcclusionQuery*> occlusionQueries;
		mBuffer->getInProgressQueries(timerQueries, occlusionQueries);

		for (auto& query : timerQueries)
			query->_interrupt(*mBuffer);

		for (auto& query : occlusionQueries)
			query->_interrupt(*mBuffer);

		if (mBuffer->isRecording())
			mBuffer->end();

		if (mBuffer->isReadyForSubmit()) // Possibly nothing was recorded in the buffer
		{
			mBuffer->submit(mQueue, mQueueIdx, syncMask);
			acquireNewBuffer();

			mDevice.refreshStates(false);
		}

		// Resume interrupted queries on the new command buffer
		for (auto& query : timerQueries)
			query->_resume(*mBuffer);

		for (auto& query : occlusionQueries)
			query->_resume(*mBuffer);
	}
}}