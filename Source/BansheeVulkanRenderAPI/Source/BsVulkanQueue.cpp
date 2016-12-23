//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsVulkanQueue.h"
#include "BsVulkanCommandBuffer.h"
#include "BsVulkanSwapChain.h"

namespace bs
{
	VulkanQueue::VulkanQueue(VulkanDevice& device, VkQueue queue, GpuQueueType type, UINT32 index)
		:mDevice(device), mQueue(queue), mType(type), mIndex(index), mLastCommandBuffer(nullptr), mNextSubmitIdx(1)
	{
		for (UINT32 i = 0; i < BS_MAX_UNIQUE_QUEUES; i++)
			mSubmitDstWaitMask[i] = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	}

	bool VulkanQueue::isExecuting() const
	{
		if (mLastCommandBuffer == nullptr)
			return false;

		return mLastCommandBuffer->isSubmitted();
	}

	void VulkanQueue::submit(VulkanCmdBuffer* cmdBuffer, VulkanSemaphore** waitSemaphores, UINT32 semaphoresCount)
	{
		VkCommandBuffer vkCmdBuffer = cmdBuffer->getHandle();
		VulkanSemaphore* semaphore = cmdBuffer->allocateSemaphore();

		VkSemaphore vkSemaphore = semaphore->getHandle();

		VkSubmitInfo submitInfo;
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.pNext = nullptr;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &vkCmdBuffer;
		submitInfo.signalSemaphoreCount = 1;
		submitInfo.pSignalSemaphores = &vkSemaphore;
		submitInfo.waitSemaphoreCount = semaphoresCount;

		if (semaphoresCount > 0)
		{
			for (UINT32 i = 0; i < semaphoresCount; i++)
				mSemaphoresTemp[i] = waitSemaphores[i]->getHandle();

			submitInfo.pWaitSemaphores = mSemaphoresTemp;
			submitInfo.pWaitDstStageMask = mSubmitDstWaitMask;
		}
		else
		{
			submitInfo.pWaitSemaphores = nullptr;
			submitInfo.pWaitDstStageMask = nullptr;
		}

		VkResult result = vkQueueSubmit(mQueue, 1, &submitInfo, cmdBuffer->getFence());
		assert(result == VK_SUCCESS);

		cmdBuffer->setIsSubmitted();
		mLastCommandBuffer = cmdBuffer;

		for (UINT32 i = 0; i < semaphoresCount; i++)
		{
			waitSemaphores[i]->notifyBound();
			waitSemaphores[i]->notifyUsed(0, 0, VulkanUseFlag::Read | VulkanUseFlag::Write);

			mActiveSemaphores.push(waitSemaphores[i]);
		}

		mActiveBuffers.push_back(SubmitInfo(cmdBuffer, mNextSubmitIdx++, semaphoresCount));
	}

	void VulkanQueue::present(VulkanSwapChain* swapChain, VulkanSemaphore** waitSemaphores, UINT32 semaphoresCount)
	{
		UINT32 backBufferIdx;
		if (!swapChain->prepareForPresent(backBufferIdx))
			return; // Nothing to present (back buffer wasn't even acquired)

		VkSwapchainKHR vkSwapChain = swapChain->getHandle();

		VkPresentInfoKHR presentInfo;
		presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
		presentInfo.pNext = nullptr;
		presentInfo.swapchainCount = 1;
		presentInfo.pSwapchains = &vkSwapChain;
		presentInfo.pImageIndices = &backBufferIdx;
		presentInfo.pResults = nullptr;

		// Wait before presenting, if required
		if (semaphoresCount > 0)
		{
			for (UINT32 i = 0; i < semaphoresCount; i++)
				mSemaphoresTemp[i] = waitSemaphores[i]->getHandle();

			presentInfo.pWaitSemaphores = mSemaphoresTemp;
			presentInfo.waitSemaphoreCount = semaphoresCount;
		}
		else
		{
			presentInfo.pWaitSemaphores = nullptr;
			presentInfo.waitSemaphoreCount = 0;
		}

		VkResult result = vkQueuePresentKHR(mQueue, &presentInfo);
		assert(result == VK_SUCCESS || result == VK_SUBOPTIMAL_KHR);

		for (UINT32 i = 0; i < semaphoresCount; i++)
		{
			waitSemaphores[i]->notifyBound();
			waitSemaphores[i]->notifyUsed(0, 0, VulkanUseFlag::Read | VulkanUseFlag::Write);

			mActiveSemaphores.push(waitSemaphores[i]);
		}

		mActiveBuffers.push_back(SubmitInfo(nullptr, mNextSubmitIdx++, semaphoresCount));
	}

	void VulkanQueue::waitIdle() const
	{
		VkResult result = vkQueueWaitIdle(mQueue);
		assert(result == VK_SUCCESS);
	}

	void VulkanQueue::refreshStates(bool queueEmpty)
	{
		UINT32 lastFinishedSubmission = 0;

		auto iter = mActiveBuffers.begin();
		while (iter != mActiveBuffers.end())
		{
			VulkanCmdBuffer* cmdBuffer = iter->cmdBuffer;
			if (cmdBuffer == nullptr)
			{
				++iter;
				continue;
			}

			cmdBuffer->refreshFenceStatus();
			if (cmdBuffer->isSubmitted())
				break; // No chance of any later CBs of being done either
				
			lastFinishedSubmission = iter->submitIdx;
			++iter;
		}

		// If last submission was a present() call, it won't be freed until a command buffer after it is done. However on
		// shutdown there might not be a CB following it. So we instead check this special flag and free everything when its
		// true.
		if (queueEmpty)
			lastFinishedSubmission = mNextSubmitIdx - 1;

		iter = mActiveBuffers.begin();
		while (iter != mActiveBuffers.end())
		{
			if (iter->submitIdx > lastFinishedSubmission)
				break;

			for (UINT32 i = 0; i < iter->numSemaphores; i++)
			{
				VulkanSemaphore* semaphore = mActiveSemaphores.front();
				mActiveSemaphores.pop();

				semaphore->notifyDone(0, VulkanUseFlag::Read | VulkanUseFlag::Write);
			}

			iter = mActiveBuffers.erase(iter);
		}
	}
}