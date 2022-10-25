//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsVulkanQueue.h"
#include "BsVulkanCommandBuffer.h"
#include "BsVulkanSwapChain.h"

namespace bs
{
	namespace ct
	{
		VulkanQueue::VulkanQueue(VulkanDevice& device, VkQueue queue, GpuQueueType type, u32 index)
			: mDevice(device), mQueue(queue), mType(type), mIndex(index)
		{
			for(u32 i = 0; i < BS_MAX_UNIQUE_QUEUES; i++)
				mSubmitDstWaitMask[i] = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;
		}

		bool VulkanQueue::IsExecuting() const
		{
			if(mLastCommandBuffer == nullptr)
				return false;

			return mLastCommandBuffer->IsSubmitted();
		}

		void VulkanQueue::Submit(VulkanCmdBuffer* cmdBuffer, VulkanSemaphore** waitSemaphores, u32 semaphoresCount)
		{
			VkSemaphore signalSemaphores[BS_MAX_VULKAN_CB_DEPENDENCIES + 1];
			cmdBuffer->AllocateSemaphores(signalSemaphores);

			VkCommandBuffer vkCmdBuffer = cmdBuffer->GetHandle();

			mSemaphoresTemp.resize(semaphoresCount + 1); // +1 for self semaphore
			PrepareSemaphores(waitSemaphores, mSemaphoresTemp.data(), semaphoresCount);

			VkSubmitInfo submitInfo;
			GetSubmitInfo(&vkCmdBuffer, signalSemaphores, BS_MAX_VULKAN_CB_DEPENDENCIES + 1, mSemaphoresTemp.data(), semaphoresCount, submitInfo);

			VkResult result = vkQueueSubmit(mQueue, 1, &submitInfo, cmdBuffer->GetFence());
			assert(result == VK_SUCCESS);

			cmdBuffer->SetIsSubmitted();
			mLastCommandBuffer = cmdBuffer;
			mLastCBSemaphoreUsed = false;

			mActiveSubmissions.push_back(SubmitInfo(cmdBuffer, mNextSubmitIdx++, semaphoresCount, 1));
			mActiveBuffers.push(cmdBuffer);
		}

		void VulkanQueue::QueueSubmit(VulkanCmdBuffer* cmdBuffer, VulkanSemaphore** waitSemaphores, u32 semaphoresCount)
		{
			mQueuedBuffers.push_back(SubmitInfo(cmdBuffer, 0, semaphoresCount, 1));

			for(u32 i = 0; i < semaphoresCount; i++)
				mQueuedSemaphores.push_back(waitSemaphores[i]);
		}

		void VulkanQueue::SubmitQueued()
		{
			u32 numCBs = (u32)mQueuedBuffers.size();
			if(numCBs == 0)
				return;

			u32 totalNumWaitSemaphores = (u32)mQueuedSemaphores.size() + numCBs;
			u32 signalSemaphoresPerCB = (BS_MAX_VULKAN_CB_DEPENDENCIES + 1);

			u8* data = (u8*)bs_stack_alloc((sizeof(VkSubmitInfo) + sizeof(VkCommandBuffer)) * numCBs + sizeof(VkSemaphore) * signalSemaphoresPerCB * numCBs + sizeof(VkSemaphore) * totalNumWaitSemaphores);
			u8* dataPtr = data;

			VkSubmitInfo* submitInfos = (VkSubmitInfo*)dataPtr;
			dataPtr += sizeof(VkSubmitInfo) * numCBs;

			VkCommandBuffer* commandBuffers = (VkCommandBuffer*)dataPtr;
			dataPtr += sizeof(VkCommandBuffer) * numCBs;

			VkSemaphore* signalSemaphores = (VkSemaphore*)dataPtr;
			dataPtr += sizeof(VkSemaphore) * signalSemaphoresPerCB * numCBs;

			VkSemaphore* waitSemaphores = (VkSemaphore*)dataPtr;
			dataPtr += sizeof(VkSemaphore) * totalNumWaitSemaphores;

			u32 readSemaphoreIdx = 0;
			u32 writeSemaphoreIdx = 0;
			u32 signalSemaphoreIdx = 0;
			for(u32 i = 0; i < numCBs; i++)
			{
				const SubmitInfo& entry = mQueuedBuffers[i];

				commandBuffers[i] = entry.CmdBuffer->GetHandle();
				entry.CmdBuffer->AllocateSemaphores(&signalSemaphores[signalSemaphoreIdx]);

				u32 semaphoresCount = entry.NumSemaphores;
				PrepareSemaphores(mQueuedSemaphores.data() + readSemaphoreIdx, &waitSemaphores[writeSemaphoreIdx], semaphoresCount);

				GetSubmitInfo(&commandBuffers[i], &signalSemaphores[signalSemaphoreIdx], signalSemaphoresPerCB, &waitSemaphores[writeSemaphoreIdx], semaphoresCount, submitInfos[i]);

				entry.CmdBuffer->SetIsSubmitted();
				mLastCommandBuffer = entry.CmdBuffer; // Needs to be set because getSubmitInfo depends on it
				mLastCBSemaphoreUsed = false;

				mActiveBuffers.push(entry.CmdBuffer);

				readSemaphoreIdx += entry.NumSemaphores;
				writeSemaphoreIdx += semaphoresCount;
				signalSemaphoreIdx += signalSemaphoresPerCB;
			}

			VulkanCmdBuffer* lastCB = mQueuedBuffers[numCBs - 1].CmdBuffer;
			u32 totalNumSemaphores = writeSemaphoreIdx;
			mActiveSubmissions.push_back(SubmitInfo(lastCB, mNextSubmitIdx++, totalNumSemaphores, numCBs));

			VkResult result = vkQueueSubmit(mQueue, numCBs, submitInfos, mLastCommandBuffer->GetFence());
			assert(result == VK_SUCCESS);

			mQueuedBuffers.clear();
			mQueuedSemaphores.clear();

			bs_stack_free(data);
		}

		void VulkanQueue::GetSubmitInfo(VkCommandBuffer* cmdBuffer, VkSemaphore* signalSemaphores, u32 numSignalSemaphores, VkSemaphore* waitSemaphores, u32 numWaitSemaphores, VkSubmitInfo& submitInfo)
		{
			submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
			submitInfo.pNext = nullptr;
			submitInfo.commandBufferCount = 1;
			submitInfo.pCommandBuffers = cmdBuffer;
			submitInfo.signalSemaphoreCount = numSignalSemaphores;
			submitInfo.pSignalSemaphores = signalSemaphores;
			submitInfo.waitSemaphoreCount = numWaitSemaphores;

			if(numWaitSemaphores > 0)
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

		VkResult VulkanQueue::Present(VulkanSwapChain* swapChain, VulkanSemaphore** waitSemaphores, u32 semaphoresCount)
		{
			u32 backBufferIdx;
			if(!swapChain->PrepareForPresent(backBufferIdx))
				return VK_SUCCESS; // Nothing to present (back buffer wasn't even acquired)

			mSemaphoresTemp.resize(semaphoresCount + 1); // +1 for self semaphore
			PrepareSemaphores(waitSemaphores, mSemaphoresTemp.data(), semaphoresCount);

			VkSwapchainKHR vkSwapChain = swapChain->GetHandle();
			VkPresentInfoKHR presentInfo;
			presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
			presentInfo.pNext = nullptr;
			presentInfo.swapchainCount = 1;
			presentInfo.pSwapchains = &vkSwapChain;
			presentInfo.pImageIndices = &backBufferIdx;
			presentInfo.pResults = nullptr;

			// Wait before presenting, if required
			if(semaphoresCount > 0)
			{
				presentInfo.pWaitSemaphores = mSemaphoresTemp.data();
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
			u32 lastFinishedSubmission = 0;

			auto iter = mActiveSubmissions.begin();
			while(iter != mActiveSubmissions.end())
			{
				VulkanCmdBuffer* cmdBuffer = iter->CmdBuffer;
				if(cmdBuffer == nullptr)
				{
					++iter;
					continue;
				}

				if(!cmdBuffer->CheckFenceStatus(forceWait))
				{
					assert(!forceWait);
					break; // No chance of any later CBs of being done either
				}

				lastFinishedSubmission = iter->SubmitIdx;
				++iter;
			}

			// If last submission was a present() call, it won't be freed until a command buffer after it is done. However on
			// shutdown there might not be a CB following it. So we instead check this special flag and free everything when its
			// true.
			if(queueEmpty)
				lastFinishedSubmission = mNextSubmitIdx - 1;

			iter = mActiveSubmissions.begin();
			while(iter != mActiveSubmissions.end())
			{
				if(iter->SubmitIdx > lastFinishedSubmission)
					break;

				for(u32 i = 0; i < iter->NumSemaphores; i++)
				{
					VulkanSemaphore* semaphore = mActiveSemaphores.front();
					mActiveSemaphores.pop();

					semaphore->NotifyDone(0, VulkanAccessFlag::Read | VulkanAccessFlag::Write);
				}

				for(u32 i = 0; i < iter->NumCommandBuffers; i++)
				{
					VulkanCmdBuffer* cb = mActiveBuffers.front();
					mActiveBuffers.pop();

					cb->Reset();
				}

				iter = mActiveSubmissions.erase(iter);
			}
		}

		void VulkanQueue::PrepareSemaphores(VulkanSemaphore** inSemaphores, VkSemaphore* outSemaphores, u32& semaphoresCount)
		{
			u32 semaphoreIdx = 0;
			for(u32 i = 0; i < semaphoresCount; i++)
			{
				VulkanSemaphore* semaphore = inSemaphores[i];

				semaphore->NotifyBound();
				semaphore->NotifyUsed(0, 0, VulkanAccessFlag::Read | VulkanAccessFlag::Write);

				outSemaphores[semaphoreIdx++] = semaphore->GetHandle();
				mActiveSemaphores.push(semaphore);
			}

			// Wait on previous CB, as we want execution to proceed in order
			if(mLastCommandBuffer != nullptr && mLastCommandBuffer->IsSubmitted() && !mLastCBSemaphoreUsed)
			{
				VulkanSemaphore* prevSemaphore = mLastCommandBuffer->GetIntraQueueSemaphore();

				prevSemaphore->NotifyBound();
				prevSemaphore->NotifyUsed(0, 0, VulkanAccessFlag::Read | VulkanAccessFlag::Write);

				outSemaphores[semaphoreIdx++] = prevSemaphore->GetHandle();
				mActiveSemaphores.push(prevSemaphore);

				// This will prevent command buffers submitted after present() to use the same semaphore. This also means that
				// there will be no intra-queue dependencies between commands for on the other ends of a present call
				// (Meaning those queue submissions could execute concurrently).
				mLastCBSemaphoreUsed = true;
			}

			semaphoresCount = semaphoreIdx;
		}
	} // namespace ct
} // namespace bs
