//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsVulkanGpuQueue.h"
#include "BsVulkanGpuCommandBuffer.h"
#include "BsVulkanOcclusionQuery.h"
#include "BsVulkanSubmitThread.h"
#include "BsVulkanSwapChain.h"
#include "BsVulkanTimerQuery.h"
#include "Profiling/BsRenderStats.h"

#if B3D_PLATFORM == B3D_PLATFORM_ID_WIN32
#include "Win32/BsWin32RenderWindow.h"
#endif

using namespace bs;
using namespace bs::ct;

VulkanGpuQueue::VulkanGpuQueue(VulkanGpuDevice& device, GpuQueueUsage usage, u32 index, VkQueue vulkanQueue)
	: GpuQueue(device, usage, index), mQueue(vulkanQueue)
{
	for(u32 i = 0; i < BS_MAX_UNIQUE_QUEUES; i++)
		mSubmitDstWaitMask[i] = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;
}

void VulkanGpuQueue::SubmitCommandBuffer(const SPtr<GpuCommandBuffer>& commandBuffer, u32 syncMask, bool flushTransferCommandBuffer)
{
	if(flushTransferCommandBuffer)
		mGpuDevice.SubmitTransferCommandBuffers();

	if (!B3D_ENSURE(commandBuffer))
		return;

	VulkanGpuCommandBuffer& vulkanCommandBuffer = static_cast<VulkanGpuCommandBuffer&>(*commandBuffer);
	if (!B3D_ENSURE(vulkanCommandBuffer.GetUsage() == mUsage))
		return;

	if (vulkanCommandBuffer.GetState() == CommandBufferState::Executing)
	{
		B3D_LOG(Error, RenderBackend, "Cannot submit a command buffer that's still executing.");
		return;
	}

	if (vulkanCommandBuffer.IsInRenderPass())
		vulkanCommandBuffer.EndRenderPass();

	// Execute any queued layout transitions that weren't already handled by the render pass
	vulkanCommandBuffer.ExecuteLayoutTransitions();

	// Interrupt any in-progress queries (no in-progress queries allowed during command buffer submit)
	Vector<VulkanTimerQuery*> timerQueries;
	Vector<VulkanOcclusionQuery*> occlusionQueries;
	vulkanCommandBuffer.GetInProgressQueries(timerQueries, occlusionQueries);

	if (!timerQueries.empty() || !occlusionQueries.empty())
	{
		B3D_LOG(Warning, RenderBackend, "Submitting a command buffer with {0} timer queries "
			"and {1} occlusion queries that are still open. The queries will be closed automatically.",
			timerQueries.size(), occlusionQueries.size());

		for (auto& query : timerQueries)
			query->Interrupt(vulkanCommandBuffer);

		for (auto& query : occlusionQueries)
			query->Interrupt(vulkanCommandBuffer);
	}

	if (vulkanCommandBuffer.IsRecording())
		vulkanCommandBuffer.End();

	vulkanCommandBuffer.SetIsSubmitted();
	GetVulkanSubmitThread().QueueSubmit(std::static_pointer_cast<VulkanGpuCommandBuffer>(commandBuffer), *this, syncMask);
	vulkanCommandBuffer.mIsSubmitted = true;
}

void VulkanGpuQueue::PresentRenderWindow(const SPtr<RenderWindow>& renderWindow, u32 syncMask)
{
	if(renderWindow == nullptr)
		return;

	VulkanRenderWindowSurface* renderWindowSurface = static_cast<VulkanRenderWindowSurface*>(renderWindow->GetRenderWindowSurface().get());
	if(renderWindowSurface == nullptr)
		return;

	// Retrieve the swap chain before command buffer submit, as the submit might internally rebuild the swap chain.
	VulkanSwapChain* const swapChain = renderWindowSurface->GetSwapChain();
	renderWindow->SwapBuffers();

	GetVulkanSubmitThread().QueuePresent(*this, *swapChain, syncMask);

	// Ensure the acquire operation we queued the previous frame has finished. This also means the old image was presented.
	swapChain->WaitUntilFirstImageAcquired();

	GetVulkanSubmitThread().QueueImageAcquire(*swapChain);

	B3D_INCREMENT_RENDER_STATISTIC(NumPresents);
}

bool VulkanGpuQueue::IsExecuting() const
{
	AssertIfNotVulkanSubmitThread();

	if(mLastSubmittedCommandBuffer == nullptr)
		return false;

	return mLastSubmittedCommandBuffer->IsSubmitted() || mLastSubmittedCommandBuffer->IsDone();
}

VkResult VulkanGpuQueue::Present(VulkanSwapChain* swapChain, u32 swapChainImageIndex, TArrayView<VulkanSemaphore*> waitSemaphores)
{
	AssertIfNotVulkanSubmitThread();

	B3D_ENSURE(mWaitSemaphoreHandleBuffer.Empty());
	RegisterSemaphoresAndGetHandles(waitSemaphores, mWaitSemaphoreHandleBuffer);

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
		presentInfo.waitSemaphoreCount = (u32)mWaitSemaphoreHandleBuffer.Size();
	}
	else
	{
		presentInfo.pWaitSemaphores = nullptr;
		presentInfo.waitSemaphoreCount = 0;
	}

	VkResult result = vkQueuePresentKHR(mQueue, &presentInfo);
	B3D_ASSERT(result == VK_SUCCESS || result == VK_SUBOPTIMAL_KHR || result == VK_ERROR_OUT_OF_DATE_KHR);

	mActiveSubmissions.push_back(QueueSubmissionInformation(swapChain, mNextSubmitIndex++, 1));
	mActiveCommandBuffers.push(QueueSubmissionEntryInformation(nullptr, (u32)mWaitSemaphoreHandleBuffer.Size()));

	mWaitSemaphoreHandleBuffer.Clear();
	return result;
}

void VulkanGpuQueue::WaitUntilIdle()
{
	GetVulkanSubmitThread().WaitUntilIdle(*this);
}

VkSubmitInfo VulkanGpuQueue::RegisterSubmissionAndGenerateSubmitInfo(const SPtr<VulkanGpuCommandBuffer>& commandBuffer, const TArrayView<VulkanSemaphore*>& waitSemaphores)
{
	TInlineArray<SPtr<VulkanGpuCommandBuffer>, 1> commandBuffers = { commandBuffer };
	return RegisterSubmissionAndGenerateSubmitInfo(commandBuffers, waitSemaphores);
}

VkSubmitInfo VulkanGpuQueue::RegisterSubmissionAndGenerateSubmitInfo(const TArrayView<SPtr<VulkanGpuCommandBuffer>>& commandBuffers, const TArrayView<VulkanSemaphore*>& waitSemaphores)
{
	const u32 commandBufferOffset = (u32)mCommandBufferHandleBuffer.Size();
	const u32 waitSemaphoreOffset = (u32)mWaitSemaphoreHandleBuffer.Size();
	const u32 signalSemaphoreOffset = (u32)mSignalSemaphoreHandleBuffer.Size();

	RegisterSemaphoresAndGetHandles(waitSemaphores, mWaitSemaphoreHandleBuffer);
	const u32 waitSemaphoreCount = ((u32)mWaitSemaphoreHandleBuffer.Size()) - waitSemaphoreOffset;

	u32 commandBufferCount = 0;
	for (const auto& entry : commandBuffers)
	{
		if (!B3D_ENSURE(entry))
			continue;

		entry->SetIsSubmitted();
		mCommandBufferHandleBuffer.Add(entry->GetVulkanHandle());
		mActiveCommandBuffers.push(QueueSubmissionEntryInformation(entry, waitSemaphoreCount));
		commandBufferCount++;
	}

	u32 signalSemaphoreCount = 0;
	if (!commandBuffers.IsEmpty())
	{
		signalSemaphoreCount = commandBuffers.back()->AllocateSignalSemaphores(mSignalSemaphoreHandleBuffer);

		mLastSubmittedCommandBuffer = commandBuffers.back(); // Needs to be set because GetSubmitInfo depends on it
		mLastCBSemaphoreUsed = false;
	}

	VkSubmitInfo vkSubmitInfo;
	vkSubmitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	vkSubmitInfo.pNext = nullptr;
	vkSubmitInfo.commandBufferCount = commandBufferCount;
	vkSubmitInfo.pCommandBuffers = commandBufferCount > 0 ? &mCommandBufferHandleBuffer[commandBufferOffset] : nullptr;
	vkSubmitInfo.signalSemaphoreCount = signalSemaphoreCount;
	vkSubmitInfo.pSignalSemaphores = signalSemaphoreCount > 0 ? &mSignalSemaphoreHandleBuffer[signalSemaphoreOffset] : nullptr;
	vkSubmitInfo.waitSemaphoreCount = waitSemaphoreCount;

	if(waitSemaphoreCount != 0)
	{
		vkSubmitInfo.pWaitSemaphores = &mWaitSemaphoreHandleBuffer[waitSemaphoreOffset];
		vkSubmitInfo.pWaitDstStageMask = mSubmitDstWaitMask;
	}
	else
	{
		vkSubmitInfo.pWaitSemaphores = nullptr;
		vkSubmitInfo.pWaitDstStageMask = nullptr;
	}

	return vkSubmitInfo;
}

void VulkanGpuQueue::ExecuteSubmitOnSubmitThread(const GpuCommandBufferSubmitInformation& submitInformation, u32 syncMask)
{
	AssertIfNotVulkanSubmitThread();

	if (!B3D_ENSURE(submitInformation.PrimaryCommandBuffer))
		return;

	VulkanGpuDevice& device = static_cast<VulkanGpuDevice&>(mGpuDevice);

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

		GpuCommandBufferSubmitInformation transitionSubmitInformation;
		transitionSubmitInformation.PrimaryCommandBuffer = submitInformation.SourceQueueTransitionCommandBuffer[queueUsageIndex];

		transitionQueue->ExecuteSubmitOnSubmitThread(transitionSubmitInformation, 0);
	}

	B3D_ENSURE(mWaitSemaphoreBuffer.Empty());
	mWaitSemaphoreBuffer.Append(submitInformation.Semaphores.begin(), submitInformation.Semaphores.end());

	device.GetSyncSemaphores(syncMask, mWaitSemaphoreBuffer);

	B3D_ENSURE(mSignalSemaphoreHandleBuffer.Empty());
	B3D_ENSURE(mWaitSemaphoreHandleBuffer.Empty());
	B3D_ENSURE(mCommandBufferHandleBuffer.Empty());

	TInlineArray<VkSubmitInfo, 3> submitInfos;

	if (submitInformation.QueryResetCommandBuffer != nullptr)
		submitInfos.Add(RegisterSubmissionAndGenerateSubmitInfo(submitInformation.QueryResetCommandBuffer, {}));

	if (submitInformation.DestinationQueueTransitionCommandBuffer != nullptr)
	{
		submitInfos.Add(RegisterSubmissionAndGenerateSubmitInfo(submitInformation.DestinationQueueTransitionCommandBuffer, mWaitSemaphoreBuffer));
		mWaitSemaphoreBuffer.Clear(); // No need to wait on these again with later submissions
	}

	submitInfos.Add(RegisterSubmissionAndGenerateSubmitInfo(submitInformation.PrimaryCommandBuffer , mWaitSemaphoreBuffer));
	mWaitSemaphoreBuffer.Clear();

	mActiveSubmissions.push_back(QueueSubmissionInformation(mLastSubmittedCommandBuffer, mNextSubmitIndex++, (u32)submitInfos.Size()));

	VkResult result = vkQueueSubmit(mQueue, (u32)submitInfos.Size(), submitInfos.Data(), mLastSubmittedCommandBuffer->GetFence());
	B3D_ASSERT(result == VK_SUCCESS);

	mSignalSemaphoreHandleBuffer.Clear();
	mWaitSemaphoreHandleBuffer.Clear();
	mCommandBufferHandleBuffer.Clear();
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
			SingleConsumerQueue& messageBackQueue = isPresentCall ? it->PresentOperationSwapChain->GetMessageQueue() : queueSubmissionInformation.CommandBuffer->GetPool().GetMessageQueue();

			TInlineArray<VulkanSemaphore*, 8> semaphoresToRelease;
			for (u32 semaphoreIndex = 0; semaphoreIndex < queueSubmissionInformation.SemaphoreCount; semaphoreIndex++)
			{
				VulkanSemaphore* const semaphore = mActiveSemaphores.front();
				mActiveSemaphores.pop();

				semaphoresToRelease.Add(semaphore);
			}

			messageBackQueue.PostCommand([semaphoresToRelease]()
			{
				for (const auto& semaphore : semaphoresToRelease)
					semaphore->NotifyDone(0, VulkanAccessFlag::Read | VulkanAccessFlag::Write);
			});

			if (isPresentCall)
				messageBackQueue.PostCommand([swapChain = it->PresentOperationSwapChain] { swapChain->NotifyUnbound(); });
			else
			{
				messageBackQueue.PostCommand([commandBuffer = queueSubmissionInformation.CommandBuffer]()
				{
					commandBuffer->mState = VulkanGpuCommandBuffer::State::Done;
					commandBuffer->OnDidComplete();
					commandBuffer->Reset();
				});
			}

			if(mLastSubmittedCommandBuffer == queueSubmissionInformation.CommandBuffer)
				mLastSubmittedCommandBuffer = nullptr;
		}

		it = mActiveSubmissions.erase(it);
	}
}

u32 VulkanGpuQueue::RegisterSemaphoresAndGetHandles(const TArrayView<VulkanSemaphore*>& inSemaphores, TInlineArray<VkSemaphore, 8>& outSemaphores)
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
