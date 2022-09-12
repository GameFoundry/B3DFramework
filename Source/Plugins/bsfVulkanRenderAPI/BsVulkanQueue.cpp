//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsVulkanQueue.h"
#include "BsVulkanCommandBuffer.h"
#include "BsVulkanSwapChain.h"

namespace bs { namespace ct
{
	VulkanQueue::VulkanQueue(VulkanDevice& device, VkQueue queue, GpuQueueType type, UINT32 index)
		: MDevice(device), mQueue(queue), mType(type), mIndex(index)
	{
		for (UINT32 i = 0; i < BS_MAX_UNIQUE_QUEUES; i++)
			mSubmitDstWaitMask[i] = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;
	}

	bool VulkanQueue::IsExecuting() const
	{
		if (mLastCommandBuffer == nullptr)
			return false;

		return mLastCommandBuffer->IsSubmitted();
	}

	void VulkanQueue::Submit(VulkanCmdBuffer* cmdBuffer, VulkanSemaphore** waitSemaphores, UINT32 semaphoresCount)
	{
		VkSemaphore signalSemaphores[BS_MAX_VULKAN_CB_DEPENDENCIES + 1];
		cmdBuffer->AllocateSemaphores(signalSemaphores);

		VkCommandBuffer vkCmdBuffer = cmdBuffer->GetHandle();

		mSemaphoresTemp.Resize(semaphoresCount + 1); // +1 for self semaphore
		prepareSemaphores(waitSemaphores, mSemaphoresTemp.Data(), semaphoresCount);
		
		VkSubmitInfo submitInfo;
		getSubmitInfo(&vkCmdBuffer, signalSemaphores, BS_MAX_VULKAN_CB_DEPENDENCIES + 1,
					  mSemaphoresTemp.Data(), semaphoresCount, submitInfo);

		VkResult result = vkQueueSubmit(mQueue, 1, &submitInfo, cmdBuffer->GetFence());
		assert(result == VK_SUCCESS);

		cmdBuffer->SetIsSubmitted();
		mLastCommandBuffer = cmdBuffer;
		mLastCBSemaphoreUsed = false;

		mActiveSubmissions.push_back(SubmitInfo(cmdBuffer, mNextSubmitIdx++, semaphoresCount, 1));
		mActiveBuffers.Push(cmdBuffer);
	}

	void VulkanQueue::QueueSubmit(VulkanCmdBuffer* cmdBuffer, VulkanSemaphore** waitSemaphores, UINT32 semaphoresCount)
	{
		mQueuedBuffers.push_back(SubmitInfo(cmdBuffer, 0, semaphoresCount, 1));

		for (UINT32 i = 0; i < semaphoresCount; i++)
			mQueuedSemaphores.push_back(waitSemaphores[i]);
	}

	void VulkanQueue::SubmitQueued()
	{
		UINT32 numCBs = (UINT32)mQueuedBuffers.Size();
		if (numCBs == 0)
			return;

		UINT32 totalNumWaitSemaphores = (UINT32)mQueuedSemaphores.Size() + numCBs;
		UINT32 signalSemaphoresPerCB = (BS_MAX_VULKAN_CB_DEPENDENCIES + 1);

		UINT8* data = (UINT8*)bs_stack_alloc((sizeof(VkSubmitInfo) + sizeof(VkCommandBuffer)) *
			numCBs + sizeof(VkSemaphore) * signalSemaphoresPerCB * numCBs + sizeof(VkSemaphore) * totalNumWaitSemaphores);
		UINT8* dataPtr = data;

		VkSubmitInfo* submitInfos = (VkSubmitInfo*)dataPtr;
		dataPtr += sizeof(VkSubmitInfo) * numCBs;

		VkCommandBuffer* commandBuffers = (VkCommandBuffer*)dataPtr;
		dataPtr += sizeof(VkCommandBuffer) * numCBs;

		VkSemaphore* signalSemaphores = (VkSemaphore*)dataPtr;
		dataPtr += sizeof(VkSemaphore) * signalSemaphoresPerCB * numCBs;

		VkSemaphore* waitSemaphores = (VkSemaphore*)dataPtr;
		dataPtr += sizeof(VkSemaphore) * totalNumWaitSemaphores;

		UINT32 readSemaphoreIdx = 0;
		UINT32 writeSemaphoreIdx = 0;
		UINT32 signalSemaphoreIdx = 0;
		for(UINT32 i = 0; i < numCBs; i++)
		{
			const SubmitInfo& entry = mQueuedBuffers[i];

			commandBuffers[i] = entry.cmdBuffer->GetHandle();
			entry.cmdBuffer->AllocateSemaphores(&signalSemaphores[signalSemaphoreIdx]);

			UINT32 semaphoresCount = entry.numSemaphores;
			prepareSemaphores(mQueuedSemaphores.Data() + readSemaphoreIdx, &waitSemaphores[writeSemaphoreIdx], semaphoresCount);

			getSubmitInfo(&commandBuffers[i], &signalSemaphores[signalSemaphoreIdx], signalSemaphoresPerCB,
						  &waitSemaphores[writeSemaphoreIdx], semaphoresCount, submitInfos[i]);

			entry.cmdBuffer->SetIsSubmitted();
			mLastCommandBuffer = entry.cmdBuffer; // Needs to be set because getSubmitInfo depends on it
			mLastCBSemaphoreUsed = false;

			mActiveBuffers.Push(entry.cmdBuffer);

			readSemaphoreIdx += entry.numSemaphores;
			writeSemaphoreIdx += semaphoresCount;
			signalSemaphoreIdx += signalSemaphoresPerCB;
		}

		VulkanCmdBuffer* lastCB = mQueuedBuffers[numCBs - 1].cmdBuffer;
		UINT32 totalNumSemaphores = writeSemaphoreIdx;
		mActiveSubmissions.push_back(SubmitInfo(lastCB, mNextSubmitIdx++, totalNumSemaphores, numCBs));

		VkResult result = vkQueueSubmit(mQueue, numCBs, submitInfos, mLastCommandBuffer->GetFence());
		assert(result == VK_SUCCESS);

		mQueuedBuffers.Clear();
		mQueuedSemaphores.Clear();

		bs_stack_free(data);
	}

	void VulkanQueue::getSubmitInfo(VkCommandBuffer* cmdBuffer, VkSemaphore* signalSemaphores, UINT32 numSignalSemaphores,
									VkSemaphore* waitSemaphores, UINT32 numWaitSemaphores, VkSubmitInfo& submitInfo)
	{
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.pNext = nullptr;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = cmdBuffer;
		submitInfo.signalSemaphoreCount = numSignalSemaphores;
		submitInfo.pSignalSemaphores = signalSemaphores;
		submitInfo.waitSemaphoreCount = numWaitSemaphores;

		if (numWaitSemaphores > 0)
		{
			submitInfo.pWaitSemaphores = waitSemaphores;
			submitInfo.pWaitDstStageMask = mSubmitDstWaitMask;
		}
		else
		{
			submitInfo.pWaitSemaphores = nullptr;
			submitInfo.pWaitDstStageMask = nullptr;
		}
	}

	VkResult VulkanQueue::Present(VulkanSwapChain* swapChain, VulkanSemaphore** waitSemaphores, UINT32 semaphoresCount)
	{
		UINT32 backBufferIdx;
		if (!swapChain->PrepareForPresent(backBufferIdx))
			return VK_SUCCESS; // Nothing to present (back buffer wasn't even acquired)

		mSemaphoresTemp.Resize(semaphoresCount + 1); // +1 for self semaphore
		prepareSemaphores(waitSemaphores, mSemaphoresTemp.Data(), semaphoresCount);

		VkSwapchainKHR vkSwapChain = swapChain->GetHandle();
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
			presentInfo.pWaitSemaphores = mSemaphoresTemp.Data();
			presentInfo.waitSemaphoreCount = semaphoresCount;
		}
		else
		{
			presentInfo.pWaitSemaphores = nullptr;
			presentInfo.waitSemaphoreCount = 0;
		}

		VkResult result = vkQueuePresentKHR(mQueue, &presentInfo);
		assert(result == VK_SUCCESS || result == VK_SUBOPTIMAL_KHR || result == VK_ERROR_OUT_OF_DATE_KHR);

		mActiveSubmissions.push_back(SubmitInfo(nullptr, mNextSubmitIdx++, semaphoresCount, 0));
		return result;
	}

	void VulkanQueue::WaitIdle() const
	{
		VkResult result = vkQueueWaitIdle(mQueue);
		assert(result == VK_SUCCESS);
	}

	void VulkanQueue::RefreshStates(bool forceWait, bool queueEmpty)
	{
		UINT32 lastFinishedSubmission = 0;

		auto iter = mActiveSubmissions.Begin();
		while (iter != mActiveSubmissions.End())
		{
			VulkanCmdBuffer* cmdBuffer = iter->cmdBuffer;
			if (cmdBuffer == nullptr)
			{
				++iter;
				continue;
			}

			if (!cmdBuffer->CheckFenceStatus(forceWait))
			{
				assert(!forceWait);
				break; // No chance of any later CBs of being done either
			}

			lastFinishedSubmission = iter->submitIdx;
			++iter;
		}

		// If last submission was a present() call, it won't be freed until a command buffer after it is done. However on
		// shutdown there might not be a CB following it. So we instead check this special flag and free everything when its
		// true.
		if (queueEmpty)
			lastFinishedSubmission = mNextSubmitIdx - 1;

		iter = mActiveSubmissions.Begin();
		while (iter != mActiveSubmissions.End())
		{
			if (iter->submitIdx > lastFinishedSubmission)
				break;

			for (UINT32 i = 0; i < iter->numSemaphores; i++)
			{
				VulkanSemaphore* semaphore = mActiveSemaphores.Front();
				mActiveSemaphores.Pop();

				semaphore->NotifyDone(0, VulkanAccessFlag::Read | VulkanAccessFlag::Write);
			}

			for(UINT32 i = 0; i < iter->numCommandBuffers; i++)
			{
				VulkanCmdBuffer* cb = mActiveBuffers.Front();
				mActiveBuffers.Pop();

				cb->Reset();
			}

			iter = mActiveSubmissions.Erase(iter);
		}
	}

	void VulkanQueue::PrepareSemaphores(VulkanSemaphore** inSemaphores, VkSemaphore* outSemaphores, UINT32& semaphoresCount)
	{
		UINT32 semaphoreIdx = 0;
		for (UINT32 i = 0; i < semaphoresCount; i++)
		{
			VulkanSemaphore* semaphore = inSemaphores[i];

			semaphore->NotifyBound();
			semaphore->NotifyUsed(0, 0, VulkanAccessFlag::Read | VulkanAccessFlag::Write);

			outSemaphores[semaphoreIdx++] = semaphore->GetHandle();
			mActiveSemaphores.Push(semaphore);
		}

		// Wait on previous CB, as we want execution to proceed in order
		if (mLastCommandBuffer != nullptr && mLastCommandBuffer->IsSubmitted() && !mLastCBSemaphoreUsed)
		{
			VulkanSemaphore* prevSemaphore = mLastCommandBuffer->GetIntraQueueSemaphore();

			prevSemaphore->NotifyBound();
			prevSemaphore->NotifyUsed(0, 0, VulkanAccessFlag::Read | VulkanAccessFlag::Write);

			outSemaphores[semaphoreIdx++] = prevSemaphore->GetHandle();
			mActiveSemaphores.Push(prevSemaphore);

			// This will prevent command buffers submitted after present() to use the same semaphore. This also means that
			// there will be no intra-queue dependencies between commands for on the other ends of a present call
			// (Meaning those queue submissions could execute concurrently).
			mLastCBSemaphoreUsed = true;
		}

		semaphoresCount = semaphoreIdx;
	}
}}
