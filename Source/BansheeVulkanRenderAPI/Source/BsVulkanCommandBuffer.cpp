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
		, mFramebuffer(nullptr), mPresentSemaphore(VK_NULL_HANDLE), mRenderTargetWidth(0), mRenderTargetHeight(0)
		, mRenderTargetDepthReadOnly(false), mGlobalQueueIdx(-1), mViewport(0.0f, 0.0f, 1.0f, 1.0f), mScissor(0, 0, 0, 0)
		, mStencilRef(0), mDrawOp(DOT_TRIANGLE_LIST), mNumBoundDescriptorSets(0), mGfxPipelineRequiresBind(true)
		, mCmpPipelineRequiresBind(true), mViewportRequiresBind(true), mStencilRefRequiresBind(true)
		, mScissorRequiresBind(true), mVertexBuffersTemp(), mVertexBufferOffsetsTemp()
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

		VkRenderPassBeginInfo renderPassBeginInfo;
		renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassBeginInfo.pNext = nullptr;
		renderPassBeginInfo.framebuffer = mFramebuffer->getFramebuffer();
		renderPassBeginInfo.renderPass = mFramebuffer->getRenderPass();
		renderPassBeginInfo.renderArea.offset.x = 0;
		renderPassBeginInfo.renderArea.offset.y = 0;
		renderPassBeginInfo.renderArea.extent.width = mRenderTargetWidth;
		renderPassBeginInfo.renderArea.extent.height = mRenderTargetHeight;

		// TODO: Handle clears (if provided) here. See VulkanRenderAPI::clearRenderTarget.
		//  - Potential problem is that we might need different framebuffers depending on whether we use load or clear
		//    ops during render pass start.
		renderPassBeginInfo.clearValueCount = 0; // TODO
		renderPassBeginInfo.pClearValues = nullptr; // TODO

		vkCmdBeginRenderPass(mCmdBuffer, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

		mState = State::RecordingRenderPass;
	}

	void VulkanCmdBuffer::endRenderPass()
	{
		assert(mState == State::RecordingRenderPass);

		vkCmdEndRenderPass(mCmdBuffer);

		mState = State::Recording;
	}

	void VulkanCmdBuffer::submit(VulkanQueue* queue, UINT32 queueIdx, UINT32 syncMask)
	{
		assert(isReadyForSubmit());

		// Issue pipeline barriers for queue transitions (need to happen on original queue first, then on new queue)
		for (auto& entry : mBuffers)
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

		for (auto& entry : mImages)
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
				if (device.getQueueFamily((GpuQueueType)i) != entryQueueFamily)
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

				otherQueueType = (GpuQueueType)i;
				break;
			}

			syncMask |= CommandSyncMask::getGlobalQueueMask(otherQueueType, otherQueueIdx);

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

		// Wait on present (i.e. until the back buffer becomes available), if we're rendering to a window
		if (mPresentSemaphore != VK_NULL_HANDLE)
		{
			mSemaphoresTemp[numSemaphores] = mPresentSemaphore;
			numSemaphores++;
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

			vkCmdPipelineBarrier(vkCmdBuffer,
								 VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, // Note: VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT might be more correct here, according to the spec
								 VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, 
								 0, 0, nullptr,
								 numBufferBarriers, barriers.bufferBarriers.data(),
								 numImgBarriers, barriers.imageBarriers.data());

			cmdBuffer->end();

			queue->submit(cmdBuffer, mSemaphoresTemp, numSemaphores);
			numSemaphores = 0; // Semaphores are only needed the first time, since we're adding the buffers on the same queue
		}

		queue->submit(this, mSemaphoresTemp, numSemaphores);

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
			ResourceUseHandle& useHandle = entry.second.useHandle;
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

		mGraphicsPipeline = nullptr;
		mComputePipeline = nullptr;
		mGfxPipelineRequiresBind = true;
		mCmpPipelineRequiresBind = true;
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

					entry.first->notifyDone(mGlobalQueueIdx, useHandle.flags);
				}

				for (auto& entry : mImages)
				{
					ResourceUseHandle& useHandle = entry.second.useHandle;
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
			}
		}
		else
			assert(!signaled); // We reset the fence along with mState so this shouldn't be possible
	}

	void VulkanCmdBuffer::setRenderTarget(const SPtr<RenderTargetCore>& rt, bool readOnlyDepthStencil)
	{
		assert(mState != State::RecordingRenderPass && mState != State::Submitted);

		if(rt == nullptr)
		{
			mFramebuffer = nullptr;
			mPresentSemaphore = VK_NULL_HANDLE;
			mRenderTargetWidth = 0;
			mRenderTargetHeight = 0;
			mRenderTargetDepthReadOnly = false;
		}
		else
		{
			rt->getCustomAttribute("FB", &mFramebuffer);
			
			if (rt->getProperties().isWindow())
				rt->getCustomAttribute("PS", &mPresentSemaphore);
			else
				mPresentSemaphore = VK_NULL_HANDLE;

			mRenderTargetWidth = rt->getProperties().getWidth();
			mRenderTargetHeight = rt->getProperties().getHeight();
			mRenderTargetDepthReadOnly = readOnlyDepthStencil;

			registerResource(mFramebuffer, VulkanUseFlag::Write);
		}

		mGfxPipelineRequiresBind = true;
	}

	void VulkanCmdBuffer::clearViewport(const Rect2I& area, UINT32 buffers, const Color& color, float depth, UINT16 stencil,
					   UINT8 targetMask)
	{
		if (buffers == 0 || mFramebuffer == nullptr)
			return;

		VkClearAttachment attachments[BS_MAX_MULTIPLE_RENDER_TARGETS + 1];
		UINT32 baseLayer = 0;

		UINT32 attachmentIdx = 0;
		if ((buffers & FBT_COLOR) != 0)
		{
			UINT32 numColorAttachments = mFramebuffer->getNumColorAttachments();
			for (UINT32 i = 0; i < numColorAttachments; i++)
			{
				if (((1 << i) & targetMask) == 0)
					continue;

				attachments[attachmentIdx].aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
				attachments[attachmentIdx].colorAttachment = i;

				VkClearColorValue& colorValue = attachments[attachmentIdx].clearValue.color;
				colorValue.float32[0] = color.r;
				colorValue.float32[1] = color.g;
				colorValue.float32[2] = color.b;
				colorValue.float32[3] = color.a;

				UINT32 curBaseLayer = mFramebuffer->getColorBaseLayer(i);
				if (attachmentIdx == 0)
					baseLayer = curBaseLayer;
				else
				{
					
					if(baseLayer != curBaseLayer)
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
				if ((buffers & FBT_DEPTH) != 0)
				{
					attachments[attachmentIdx].aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
					attachments[attachmentIdx].clearValue.depthStencil.depth = depth;
				}

				if ((buffers & FBT_STENCIL) != 0)
				{
					attachments[attachmentIdx].aspectMask = VK_IMAGE_ASPECT_STENCIL_BIT;
					attachments[attachmentIdx].clearValue.depthStencil.stencil = stencil;
				}

				attachments[attachmentIdx].colorAttachment = 0;

				UINT32 curBaseLayer = mFramebuffer->getDepthStencilBaseLayer();
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

		VkClearRect clearRect;
		clearRect.baseArrayLayer = baseLayer;
		clearRect.layerCount = mFramebuffer->getNumLayers();
		clearRect.rect.offset.x = area.x;
		clearRect.rect.offset.y = area.y;
		clearRect.rect.extent.width = area.width;
		clearRect.rect.extent.height = area.height;

		UINT32 numAttachments = attachmentIdx;
		vkCmdClearAttachments(mCmdBuffer, numAttachments, attachments, 1, &clearRect);
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
		SPtr<VulkanGpuParams> vulkanGpuParams = std::static_pointer_cast<VulkanGpuParams>(gpuParams);

		if(vulkanGpuParams != nullptr)
		{
			mNumBoundDescriptorSets = vulkanGpuParams->getNumSets();
			vulkanGpuParams->prepareForBind(*this, mDescriptorSetsTemp);
		}
		else
		{
			mNumBoundDescriptorSets = 0;
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

	void VulkanCmdBuffer::draw(UINT32 vertexOffset, UINT32 vertexCount, UINT32 instanceCount)
	{
		if (!isReadyForRender())
			return;

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
			UINT32 deviceIdx = mDevice.getIndex();
			VkPipelineLayout pipelineLayout = mGraphicsPipeline->getPipelineLayout(deviceIdx);

			vkCmdBindDescriptorSets(mCmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0,
									mNumBoundDescriptorSets, mDescriptorSetsTemp, 0, nullptr);

			mDescriptorSetsBindState.unset(DescriptorSetBindFlag::Graphics);
		}

		vkCmdDraw(mCmdBuffer, vertexCount, instanceCount, vertexOffset, 0);
	}

	void VulkanCmdBuffer::drawIndexed(UINT32 startIndex, UINT32 indexCount, UINT32 vertexOffset, UINT32 instanceCount)
	{
		if (!isReadyForRender())
			return;

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
			UINT32 deviceIdx = mDevice.getIndex();
			VkPipelineLayout pipelineLayout = mGraphicsPipeline->getPipelineLayout(deviceIdx);

			vkCmdBindDescriptorSets(mCmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0,
									mNumBoundDescriptorSets, mDescriptorSetsTemp, 0, nullptr);

			mDescriptorSetsBindState.unset(DescriptorSetBindFlag::Graphics);
		}

		vkCmdDrawIndexed(mCmdBuffer, indexCount, instanceCount, startIndex, vertexOffset, 0);
	}

	void VulkanCmdBuffer::dispatch(UINT32 numGroupsX, UINT32 numGroupsY, UINT32 numGroupsZ)
	{
		if (mComputePipeline == nullptr)
			return;

		if (isInRenderPass())
			endRenderPass();

		UINT32 deviceIdx = mDevice.getIndex();
		if(mCmpPipelineRequiresBind)
		{
			VulkanPipeline* pipeline = mComputePipeline->getPipeline(deviceIdx);
			if (pipeline == nullptr)
				return;

			registerResource(pipeline, VulkanUseFlag::Read);

			vkCmdBindPipeline(mCmdBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipeline->getHandle());
			mCmpPipelineRequiresBind = false;
		}

		if(mDescriptorSetsBindState.isSet(DescriptorSetBindFlag::Compute))
		{
			VkPipelineLayout pipelineLayout = mComputePipeline->getPipelineLayout(deviceIdx);
			vkCmdBindDescriptorSets(mCmdBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipelineLayout, 0,
				mNumBoundDescriptorSets, mDescriptorSetsTemp, 0, nullptr);

			mDescriptorSetsBindState.unset(DescriptorSetBindFlag::Compute);
		}

		vkCmdDispatch(mCmdBuffer, numGroupsX, numGroupsY, numGroupsZ);
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

		mBuffer->submit(mQueue, mQueueIdx, syncMask);

		gVulkanCBManager().refreshStates(mDeviceIdx);
		acquireNewBuffer();
	}
}