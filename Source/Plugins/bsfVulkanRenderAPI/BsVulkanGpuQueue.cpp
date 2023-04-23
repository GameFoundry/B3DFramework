//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsVulkanGpuQueue.h"
#include "BsVulkanGpuCommandBuffer.h"
#include "BsVulkanSubmitThread.h"
#include "BsVulkanSwapChain.h"

using namespace bs;
using namespace bs::ct;

VulkanGpuQueue::VulkanGpuQueue(VulkanGpuDevice& device, GpuQueueUsage usage, u32 index, VkQueue vulkanQueue)
	: GpuQueue(device, usage, index), mQueue(vulkanQueue)
{
	for(u32 i = 0; i < BS_MAX_UNIQUE_QUEUES; i++)
		mSubmitDstWaitMask[i] = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;
}

void VulkanGpuQueue::SubmitCommandBuffer(const SPtr<GpuCommandBuffer>& commandBuffer, u32 syncMask)
{
	static_cast<VulkanGpuCommandBuffer&>(*commandBuffer).Submit(*this, syncMask);
}

void VulkanGpuQueue::SubmitCommandBuffers(const ArrayView<SPtr<GpuCommandBuffer>>& commandBuffers, u32 syncMask)
{
	// TODO - Add a way to bulk-submit these
	for(const auto& entry : commandBuffers)
		static_cast<VulkanGpuCommandBuffer&>(*entry).Submit(*this, syncMask);
}

bool VulkanGpuQueue::IsExecuting() const
{
	AssertIfNotVulkanSubmitThread();

	if(mLastSubmittedCommandBuffer == nullptr)
		return false;

	return mLastSubmittedCommandBuffer->IsSubmitted() || mLastSubmittedCommandBuffer->IsDone();
}

u32 VulkanGpuQueue::Submit(const SPtr<VulkanGpuCommandBuffer>& commandBuffer, VulkanSemaphore** waitSemaphores, u32 semaphoresCount)
{
	AssertIfNotVulkanSubmitThread();

	const u32 submitIndex = mNextSubmitIndex;

	VkSemaphore signalSemaphores[BS_MAX_VULKAN_CB_DEPENDENCIES + 1];
	commandBuffer->AllocateSemaphores(signalSemaphores);

	VkCommandBuffer vkCmdBuffer = commandBuffer->GetHandle();

	mSemaphoresTemp.resize(semaphoresCount + 1); // +1 for self semaphore
	PrepareSemaphores(waitSemaphores, mSemaphoresTemp.data(), semaphoresCount);

	VkSubmitInfo submitInfo;
	GetSubmitInfo(&vkCmdBuffer, signalSemaphores, BS_MAX_VULKAN_CB_DEPENDENCIES + 1, mSemaphoresTemp.data(), semaphoresCount, submitInfo);

	VkResult result = vkQueueSubmit(mQueue, 1, &submitInfo, commandBuffer->GetFence());
	B3D_ASSERT(result == VK_SUCCESS);

	commandBuffer->SetIsSubmitted();
	mLastSubmittedCommandBuffer = commandBuffer;
	mLastCBSemaphoreUsed = false;

	mActiveSubmissions.push_back(QueueSubmissionInformation(commandBuffer, mNextSubmitIndex++, 1));
	mActiveCommandBuffers.push(QueueSubmissionEntryInformation(commandBuffer, semaphoresCount));

	return submitIndex;
}

void VulkanGpuQueue::QueueSubmit(const SPtr<VulkanGpuCommandBuffer>& commandBuffer, VulkanSemaphore** waitSemaphores, u32 semaphoresCount)
{
	AssertIfNotVulkanSubmitThread();

	mQueuedCommandBuffers.push_back(QueueSubmissionEntryInformation(commandBuffer, semaphoresCount));

	for(u32 i = 0; i < semaphoresCount; i++)
		mQueuedSemaphores.push_back(waitSemaphores[i]);
}

u32 VulkanGpuQueue::SubmitQueued()
{
	AssertIfNotVulkanSubmitThread();

	u32 queuedCommandBufferCount = (u32)mQueuedCommandBuffers.size();
	if(queuedCommandBufferCount == 0)
		return ~0u;

	u32 totalNumWaitSemaphores = (u32)mQueuedSemaphores.size() + queuedCommandBufferCount;
	u32 signalSemaphoresPerCB = (BS_MAX_VULKAN_CB_DEPENDENCIES + 1);

	u8* data = (u8*)B3DStackAllocate((sizeof(VkSubmitInfo) + sizeof(VkCommandBuffer)) * queuedCommandBufferCount + sizeof(VkSemaphore) * signalSemaphoresPerCB * queuedCommandBufferCount + sizeof(VkSemaphore) * totalNumWaitSemaphores);
	u8* dataPtr = data;

	VkSubmitInfo* submitInfos = (VkSubmitInfo*)dataPtr;
	dataPtr += sizeof(VkSubmitInfo) * queuedCommandBufferCount;

	VkCommandBuffer* commandBuffers = (VkCommandBuffer*)dataPtr;
	dataPtr += sizeof(VkCommandBuffer) * queuedCommandBufferCount;

	VkSemaphore* signalSemaphores = (VkSemaphore*)dataPtr;
	dataPtr += sizeof(VkSemaphore) * signalSemaphoresPerCB * queuedCommandBufferCount;

	VkSemaphore* waitSemaphores = (VkSemaphore*)dataPtr;
	dataPtr += sizeof(VkSemaphore) * totalNumWaitSemaphores;

	u32 readSemaphoreIdx = 0;
	u32 writeSemaphoreIdx = 0;
	u32 signalSemaphoreIdx = 0;
	for(u32 i = 0; i < queuedCommandBufferCount; i++)
	{
		QueueSubmissionEntryInformation& entry = mQueuedCommandBuffers[i];

		commandBuffers[i] = entry.CommandBuffer->GetHandle();
		entry.CommandBuffer->AllocateSemaphores(&signalSemaphores[signalSemaphoreIdx]);

		const u32 initialSemaphoresCount = entry.SemaphoreCount;
		PrepareSemaphores(mQueuedSemaphores.data() + readSemaphoreIdx, &waitSemaphores[writeSemaphoreIdx], entry.SemaphoreCount);

		GetSubmitInfo(&commandBuffers[i], &signalSemaphores[signalSemaphoreIdx], signalSemaphoresPerCB, &waitSemaphores[writeSemaphoreIdx], entry.SemaphoreCount, submitInfos[i]);

		entry.CommandBuffer->SetIsSubmitted();
		mLastSubmittedCommandBuffer = entry.CommandBuffer; // Needs to be set because GetSubmitInfo depends on it
		mLastCBSemaphoreUsed = false;

		mActiveCommandBuffers.push(entry);

		readSemaphoreIdx += initialSemaphoresCount;
		writeSemaphoreIdx += entry.SemaphoreCount;
		signalSemaphoreIdx += signalSemaphoresPerCB;
	}

	const u32 submitIndex = mNextSubmitIndex;

	const SPtr<VulkanGpuCommandBuffer> lastSubmittedCommandBuffer = mQueuedCommandBuffers[queuedCommandBufferCount - 1].CommandBuffer;
	mActiveSubmissions.push_back(QueueSubmissionInformation(lastSubmittedCommandBuffer, mNextSubmitIndex++, queuedCommandBufferCount));

	VkResult result = vkQueueSubmit(mQueue, queuedCommandBufferCount, submitInfos, mLastSubmittedCommandBuffer->GetFence());
	B3D_ASSERT(result == VK_SUCCESS);

	mQueuedCommandBuffers.clear();
	mQueuedSemaphores.clear();

	B3DStackFree(data);
	return submitIndex;
}

void VulkanGpuQueue::GetSubmitInfo(VkCommandBuffer* vkCommandBuffer, VkSemaphore* signalSemaphores, u32 signalSemaphoreCount, VkSemaphore* waitSemaphores, u32 waitSemaphoreCount, VkSubmitInfo& submitInfo)
{
	AssertIfNotVulkanSubmitThread();

	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.pNext = nullptr;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = vkCommandBuffer;
	submitInfo.signalSemaphoreCount = signalSemaphoreCount;
	submitInfo.pSignalSemaphores = signalSemaphores;
	submitInfo.waitSemaphoreCount = waitSemaphoreCount;

	if(waitSemaphoreCount > 0)
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

VkResult VulkanGpuQueue::Present(VulkanSwapChain* swapChain, u32 swapChainImageIndex, VulkanSemaphore** waitSemaphores, u32 semaphoresCount)
{
	AssertIfNotVulkanSubmitThread();

	mSemaphoresTemp.resize(semaphoresCount + 1); // +1 for self semaphore
	PrepareSemaphores(waitSemaphores, mSemaphoresTemp.data(), semaphoresCount);

	VkSwapchainKHR vkSwapChain = swapChain->GetHandle();
	VkPresentInfoKHR presentInfo;
	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	presentInfo.pNext = nullptr;
	presentInfo.swapchainCount = 1;
	presentInfo.pSwapchains = &vkSwapChain;
	presentInfo.pImageIndices = &swapChainImageIndex;
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
	B3D_ASSERT(result == VK_SUCCESS || result == VK_SUBOPTIMAL_KHR || result == VK_ERROR_OUT_OF_DATE_KHR);

	mActiveSubmissions.push_back(QueueSubmissionInformation(swapChain, mNextSubmitIndex++, 1));
	mActiveCommandBuffers.push(QueueSubmissionEntryInformation(nullptr, semaphoresCount));

	return result;
}

void VulkanGpuQueue::WaitUntilIdle()
{
	GetVulkanSubmitThread().WaitUntilIdle(*this);
	GetVulkanSubmitThread().RefreshCommandBufferCompletionStates();
}

void VulkanGpuQueue::RefreshCompletionStateOnSubmitThread(bool forceWait, bool queueEmpty, u32 lastSubmitIndex)
{
	AssertIfNotVulkanSubmitThread();

	u32 lastFinishedSubmission = 0;

	auto it = mActiveSubmissions.begin();
	while(it != mActiveSubmissions.end())
	{
		const SPtr<VulkanGpuCommandBuffer> cmdBuffer = it->LastSubmittedCommandBuffer;
		if(cmdBuffer == nullptr)
		{
			++it;
			continue;
		}

		if(lastSubmitIndex != ~0u && it->SubmitIndex > lastSubmitIndex)
			break;

		if(!cmdBuffer->UpdateExecutionStatus(forceWait))
		{
			B3D_ASSERT(!forceWait);
			break; // No chance of any later CBs of being done either
		}

		lastFinishedSubmission = it->SubmitIndex;
		++it;
	}

	// If last submission was a Present() call, it won't be freed until a command buffer after it is done. However on
	// shutdown there might not be a CB following it. So we instead check this special flag and free everything when its
	// true.
	if(queueEmpty)
		lastFinishedSubmission = mNextSubmitIndex - 1;

	Lock lock(mMutex);
	it = mActiveSubmissions.begin();
	while(it != mActiveSubmissions.end())
	{
		if(it->SubmitIndex > lastFinishedSubmission)
			break;

		for(u32 commandBufferIndex = 0; commandBufferIndex < it->CommandBufferCount; commandBufferIndex++)
		{
			const QueueSubmissionEntryInformation queueSubmissionInformation = mActiveCommandBuffers.front();
			mActiveCommandBuffers.pop();

			const bool isPresentCall = queueSubmissionInformation.CommandBuffer == nullptr;
			const bool isOwnedBySubmitThread = isPresentCall || queueSubmissionInformation.CommandBuffer->GetOwnerThread() == B3D_CURRENT_THREAD_ID;

			for(u32 semaphoreIndex = 0; semaphoreIndex < queueSubmissionInformation.SemaphoreCount; semaphoreIndex++)
			{
				VulkanSemaphore* const semaphore = mActiveSemaphores.front();
				mActiveSemaphores.pop();

				if(isOwnedBySubmitThread)
					semaphore->NotifyDone(0, VulkanAccessFlag::Read | VulkanAccessFlag::Write);
				else
					mSemaphoresToReleaseOnRenderThread.push_back(semaphore);
			}

			if(isPresentCall)
			{
				B3D_ASSERT(it->PresentOperationSwapChain != nullptr);
				mPresentedSwapChainsToUnbindOnRenderThread.push_back(it->PresentOperationSwapChain);
			}

			if(queueSubmissionInformation.CommandBuffer == nullptr)
				continue;

			if(isOwnedBySubmitThread)
			{
				queueSubmissionInformation.CommandBuffer->mState = VulkanGpuCommandBuffer::State::Done;
				queueSubmissionInformation.CommandBuffer->Reset();
			}
			else
				mCommandBuffersToResetOnRenderThread.push_back(queueSubmissionInformation.CommandBuffer);

			if(mLastSubmittedCommandBuffer == queueSubmissionInformation.CommandBuffer)
				mLastSubmittedCommandBuffer = nullptr;
		}

		it = mActiveSubmissions.erase(it);
	}
}

void VulkanGpuQueue::RefreshCompletionStateOnRenderThread()
{
	Lock lock(mMutex);
	for(const auto& entry : mSemaphoresToReleaseOnRenderThread)
		entry->NotifyDone(0, VulkanAccessFlag::Read | VulkanAccessFlag::Write);

	for(const auto& entry : mCommandBuffersToResetOnRenderThread)
	{
		entry->mState = VulkanGpuCommandBuffer::State::Done;
		entry->Reset();
	}

	for(const auto& entry : mPresentedSwapChainsToUnbindOnRenderThread)
		entry->NotifyUnbound();

	mSemaphoresToReleaseOnRenderThread.clear();
	mCommandBuffersToResetOnRenderThread.clear();
	mPresentedSwapChainsToUnbindOnRenderThread.clear();
}

void VulkanGpuQueue::PrepareSemaphores(VulkanSemaphore** inSemaphores, VkSemaphore* outSemaphores, u32& semaphoresCount)
{
	AssertIfNotVulkanSubmitThread();

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
	if(mLastSubmittedCommandBuffer != nullptr && (mLastSubmittedCommandBuffer->IsSubmitted() || mLastSubmittedCommandBuffer->IsDone()) && !mLastCBSemaphoreUsed)
	{
		VulkanSemaphore* prevSemaphore = mLastSubmittedCommandBuffer->GetIntraQueueSemaphore();

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
