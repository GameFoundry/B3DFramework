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

bool VulkanGpuQueue::IsExecuting() const
{
	AssertIfNotVulkanSubmitThread();

	if(mLastSubmittedCommandBuffer == nullptr)
		return false;

	return mLastSubmittedCommandBuffer->IsSubmitted() || mLastSubmittedCommandBuffer->IsDone();
}

u32 VulkanGpuQueue::Submit(const SPtr<VulkanGpuCommandBuffer>& commandBuffer, const ArrayView<VulkanSemaphore*>& waitSemaphores)
{
	AssertIfNotVulkanSubmitThread();

	const u32 submitIndex = mNextSubmitIndex;

	B3D_ENSURE(mSignalSemaphoreHandleBuffer.Empty());
	commandBuffer->AllocateSemaphores(mSignalSemaphoreHandleBuffer);

	VkCommandBuffer vkCmdBuffer = commandBuffer->GetHandle();

	B3D_ENSURE(mWaitSemaphoreHandleBuffer.Empty());
	PrepareSemaphores(waitSemaphores, mWaitSemaphoreHandleBuffer);

	VkSubmitInfo submitInfo;
	GetSubmitInfo(&vkCmdBuffer, mSignalSemaphoreHandleBuffer, mWaitSemaphoreHandleBuffer, submitInfo);

	VkResult result = vkQueueSubmit(mQueue, 1, &submitInfo, commandBuffer->GetFence());
	B3D_ASSERT(result == VK_SUCCESS);

	commandBuffer->SetIsSubmitted();
	mLastSubmittedCommandBuffer = commandBuffer;
	mLastCBSemaphoreUsed = false;

	mActiveSubmissions.push_back(QueueSubmissionInformation(commandBuffer, mNextSubmitIndex++, 1));
	mActiveCommandBuffers.push(QueueSubmissionEntryInformation(commandBuffer, mWaitSemaphoreHandleBuffer.Size()));

	mSignalSemaphoreHandleBuffer.Clear();
	mWaitSemaphoreHandleBuffer.Clear();

	return submitIndex;
}

void VulkanGpuQueue::QueueSubmit(const SPtr<VulkanGpuCommandBuffer>& commandBuffer, const ArrayView<VulkanSemaphore*>& waitSemaphores)
{
	AssertIfNotVulkanSubmitThread();

	mQueuedCommandBuffers.push_back(QueueSubmissionEntryInformation(commandBuffer, waitSemaphores.Size()));

	for(const auto& semaphore : waitSemaphores)
		mQueuedSemaphores.push_back(semaphore);
}

u32 VulkanGpuQueue::SubmitQueued()
{
	AssertIfNotVulkanSubmitThread();

	u32 queuedCommandBufferCount = (u32)mQueuedCommandBuffers.size();
	if(queuedCommandBufferCount == 0)
		return ~0u;

	u8* data = (u8*)B3DStackAllocate((sizeof(VkSubmitInfo) + sizeof(VkCommandBuffer)) * queuedCommandBufferCount);
	u8* dataPtr = data;

	VkSubmitInfo* submitInfos = (VkSubmitInfo*)dataPtr;
	dataPtr += sizeof(VkSubmitInfo) * queuedCommandBufferCount;

	VkCommandBuffer* commandBuffers = (VkCommandBuffer*)dataPtr;
	dataPtr += sizeof(VkCommandBuffer) * queuedCommandBufferCount;

	B3D_ENSURE(mSignalSemaphoreHandleBuffer.Empty());
	B3D_ENSURE(mWaitSemaphoreHandleBuffer.Empty());

	u32 waitSemaphoreInputIndex = 0;
	u32 waitSemaphoreOutputIndex = 0;
	u32 signalSemaphoreIndex = 0;
	for(u32 i = 0; i < queuedCommandBufferCount; i++)
	{
		QueueSubmissionEntryInformation& entry = mQueuedCommandBuffers[i];

		commandBuffers[i] = entry.CommandBuffer->GetHandle();

		const u32 allocatedSignalSemaphoreCount = entry.CommandBuffer->AllocateSemaphores(mSignalSemaphoreHandleBuffer);
		const u32 allocatedWaitSemaphoreCount = PrepareSemaphores(ArrayView(mQueuedSemaphores.data() + waitSemaphoreInputIndex, entry.SemaphoreCount), mWaitSemaphoreHandleBuffer);

		GetSubmitInfo(&commandBuffers[i], ArrayView(mSignalSemaphoreHandleBuffer.Data() + signalSemaphoreIndex, allocatedSignalSemaphoreCount), ArrayView(mWaitSemaphoreHandleBuffer.data() + waitSemaphoreOutputIndex, allocatedWaitSemaphoreCount), submitInfos[i]);

		entry.CommandBuffer->SetIsSubmitted();
		mLastSubmittedCommandBuffer = entry.CommandBuffer; // Needs to be set because GetSubmitInfo depends on it
		mLastCBSemaphoreUsed = false;

		mActiveCommandBuffers.push(entry);

		waitSemaphoreInputIndex += entry.SemaphoreCount;
		waitSemaphoreOutputIndex += allocatedWaitSemaphoreCount;
		signalSemaphoreIndex += allocatedSignalSemaphoreCount;
	}

	const u32 submitIndex = mNextSubmitIndex;

	const SPtr<VulkanGpuCommandBuffer> lastSubmittedCommandBuffer = mQueuedCommandBuffers[queuedCommandBufferCount - 1].CommandBuffer;
	mActiveSubmissions.push_back(QueueSubmissionInformation(lastSubmittedCommandBuffer, mNextSubmitIndex++, queuedCommandBufferCount));

	VkResult result = vkQueueSubmit(mQueue, queuedCommandBufferCount, submitInfos, mLastSubmittedCommandBuffer->GetFence());
	B3D_ASSERT(result == VK_SUCCESS);

	mQueuedCommandBuffers.clear();
	mQueuedSemaphores.clear();

	B3DStackFree(data);

	mSignalSemaphoreHandleBuffer.Clear();
	mWaitSemaphoreHandleBuffer.Clear();

	return submitIndex;
}

void VulkanGpuQueue::GetSubmitInfo(VkCommandBuffer* vkCommandBuffer, const ArrayView<VkSemaphore>& signalSemaphores, const ArrayView<VkSemaphore>& waitSemaphores, VkSubmitInfo& outSubmitInfo)
{
	AssertIfNotVulkanSubmitThread();

	outSubmitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	outSubmitInfo.pNext = nullptr;
	outSubmitInfo.commandBufferCount = 1;
	outSubmitInfo.pCommandBuffers = vkCommandBuffer;
	outSubmitInfo.signalSemaphoreCount = signalSemaphores.Size();
	outSubmitInfo.pSignalSemaphores = signalSemaphores.Data();
	outSubmitInfo.waitSemaphoreCount = waitSemaphores.Size();

	if(waitSemaphores.Size() > 0)
	{
		outSubmitInfo.pWaitSemaphores = waitSemaphores.Data();
		outSubmitInfo.pWaitDstStageMask = mSubmitDstWaitMask;
	}
	else
	{
		outSubmitInfo.pWaitSemaphores = nullptr;
		outSubmitInfo.pWaitDstStageMask = nullptr;
	}
}

VkResult VulkanGpuQueue::Present(VulkanSwapChain* swapChain, u32 swapChainImageIndex, ArrayView<VulkanSemaphore*> waitSemaphores)
{
	AssertIfNotVulkanSubmitThread();

	B3D_ENSURE(mWaitSemaphoreHandleBuffer.Empty());
	PrepareSemaphores(waitSemaphores, mWaitSemaphoreHandleBuffer);

	VkSwapchainKHR vkSwapChain = swapChain->GetHandle();
	VkPresentInfoKHR presentInfo;
	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	presentInfo.pNext = nullptr;
	presentInfo.swapchainCount = 1;
	presentInfo.pSwapchains = &vkSwapChain;
	presentInfo.pImageIndices = &swapChainImageIndex;
	presentInfo.pResults = nullptr;

	// Wait before presenting, if required
	if(mWaitSemaphoreHandleBuffer.Size() > 0)
	{
		presentInfo.pWaitSemaphores = mWaitSemaphoreHandleBuffer.data();
		presentInfo.waitSemaphoreCount = mWaitSemaphoreHandleBuffer.Size();
	}
	else
	{
		presentInfo.pWaitSemaphores = nullptr;
		presentInfo.waitSemaphoreCount = 0;
	}

	VkResult result = vkQueuePresentKHR(mQueue, &presentInfo);
	B3D_ASSERT(result == VK_SUCCESS || result == VK_SUBOPTIMAL_KHR || result == VK_ERROR_OUT_OF_DATE_KHR);

	mActiveSubmissions.push_back(QueueSubmissionInformation(swapChain, mNextSubmitIndex++, 1));
	mActiveCommandBuffers.push(QueueSubmissionEntryInformation(nullptr, mWaitSemaphoreHandleBuffer.Size()));

	mWaitSemaphoreHandleBuffer.Clear();
	return result;
}

void VulkanGpuQueue::WaitUntilIdle()
{
	GetVulkanSubmitThread().WaitUntilIdle(*this);
	GetVulkanSubmitThread().RefreshCommandBufferCompletionStates();
}

void VulkanGpuQueue::ExecuteSubmitOnSubmitThread(const GpuCommandBufferSubmitInformation& submitInformation, u32 syncMask)
{
	AssertIfNotVulkanSubmitThread();

	VulkanGpuDevice& device = static_cast<VulkanGpuDevice&>(mGpuDevice);

	if (submitInformation.QueryResetCommandBuffer != nullptr)
		QueueSubmit(submitInformation.QueryResetCommandBuffer, {});

	// No need to explicitly sync with any entries on the same queue
	const u32 queueMask = device.GetQueueMask(mUsage, mIndex);
	syncMask &= ~queueMask;

	B3D_ASSERT(B3DSize(submitInformation.SourceQueueTransitionCommandBuffer) == GQT_COUNT);
	for(u32 queueUsageIndex = 0; queueUsageIndex < GQT_COUNT; ++queueUsageIndex)
	{
		if (submitInformation.SourceQueueTransitionCommandBuffer[queueUsageIndex] == nullptr)
			continue;

		const GpuQueueUsage transitionQueueUsage = (GpuQueueUsage)queueUsageIndex;
		
		// Find an appropriate queue to execute on
		u32 transitionQueueIndex = 0;
		SPtr<VulkanGpuQueue> transitionQueue = nullptr;

		const u32 queueCount = device.GetQueueCount(transitionQueueUsage);
		for(u32 queueIndex = 0; queueIndex < queueCount; queueIndex++)
		{
			// Try to find a queue not currently executing
			const SPtr<VulkanGpuQueue>& curQueue = std::static_pointer_cast<VulkanGpuQueue>(device.GetQueue(transitionQueueUsage, queueIndex));
			if(!curQueue->IsExecuting())
			{
				transitionQueue = curQueue;
				transitionQueueIndex = queueIndex;
			}
		}

		// Can't find empty one, use the first one then
		if(transitionQueue == nullptr)
		{
			transitionQueue = std::static_pointer_cast<VulkanGpuQueue>(device.GetQueue(transitionQueueUsage, 0));
			transitionQueueIndex = 0;
		}

		syncMask |= CommandSyncMask::GetGlobalQueueMask(transitionQueueUsage, transitionQueueIndex);

		transitionQueue->Submit(submitInformation.SourceQueueTransitionCommandBuffer[queueUsageIndex], {});
	}

	B3D_ENSURE(mWaitSemaphoreBuffer.Empty());
	mWaitSemaphoreBuffer.Append(submitInformation.Semaphores.begin(), submitInformation.Semaphores.end());

	device.GetSyncSemaphores(syncMask, mWaitSemaphoreBuffer);

	if(submitInformation.DestinationQueueTransitionCommandBuffer != nullptr)
	{
		QueueSubmit(submitInformation.DestinationQueueTransitionCommandBuffer, mWaitSemaphoreBuffer);

		// No need to submit them with the primary command buffer if they were submitted here
		mWaitSemaphoreBuffer.Clear();
	}

	QueueSubmit(submitInformation.PrimaryCommandBuffer, mWaitSemaphoreBuffer);
	SubmitQueued();

	mWaitSemaphoreBuffer.Clear();
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
				queueSubmissionInformation.CommandBuffer->OnDidComplete();
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
		entry->OnDidComplete();
		entry->Reset();
	}

	for(const auto& entry : mPresentedSwapChainsToUnbindOnRenderThread)
		entry->NotifyUnbound();

	mSemaphoresToReleaseOnRenderThread.clear();
	mCommandBuffersToResetOnRenderThread.clear();
	mPresentedSwapChainsToUnbindOnRenderThread.clear();
}

u32 VulkanGpuQueue::PrepareSemaphores(const ArrayView<VulkanSemaphore*>& inSemaphores, SmallVector<VkSemaphore, 8>& outSemaphores)
{
	AssertIfNotVulkanSubmitThread();

	u32 count = 0;
	const u32 globalQueueIndex = CommandSyncMask::GetGlobalQueueIdx(mUsage, mIndex);

	for(const auto& semaphore : inSemaphores)
	{
		semaphore->NotifyBound();
		semaphore->NotifyUsed(globalQueueIndex, VulkanAccessFlag::Read | VulkanAccessFlag::Write);

		outSemaphores.Add(semaphore->GetHandle());
		count++;
		mActiveSemaphores.push(semaphore);
	}

	// Wait on previous CB, as we want execution to proceed in order
	if(mLastSubmittedCommandBuffer != nullptr && (mLastSubmittedCommandBuffer->IsSubmitted() || mLastSubmittedCommandBuffer->IsDone()) && !mLastCBSemaphoreUsed)
	{
		VulkanSemaphore* prevSemaphore = mLastSubmittedCommandBuffer->GetIntraQueueSemaphore();

		prevSemaphore->NotifyBound();
		prevSemaphore->NotifyUsed(globalQueueIndex, VulkanAccessFlag::Read | VulkanAccessFlag::Write);

		outSemaphores.Add(prevSemaphore->GetHandle());
		count++;
		mActiveSemaphores.push(prevSemaphore);

		// This will prevent command buffers submitted after present() to use the same semaphore. This also means that
		// there will be no intra-queue dependencies between commands for on the other ends of a present call
		// (Meaning those queue submissions could execute concurrently).
		mLastCBSemaphoreUsed = true;
	}

	return count;
}
