//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DVulkanGpuQueue.h"
#include "B3DVulkanGpuCommandBuffer.h"
#include "B3DVulkanGpuTimelineFence.h"
#include "B3DIVulkanRenderWindowSurface.h"
#include "B3DVulkanHeapBackend.h"
#include "B3DVulkanSubmitThread.h"
#include "B3DVulkanSwapChain.h"
#include "Profiling/B3DRenderStats.h"

using namespace b3d;
using namespace b3d::render;

void VulkanGpuQueue::SubmitWorkBuffer::Clear()
{
	SignalSemaphores.Clear();
	SignalValues.Clear();
	WaitSemaphores.Clear();
	CommandBuffers.Clear();
}

VulkanGpuQueue::VulkanGpuQueue(VulkanGpuDevice& device, GpuQueueType type, u32 index, VkQueue vulkanQueue)
	: GpuQueue(device, type, index), mQueue(vulkanQueue)
{
	for(u32 i = 0; i < B3D_MAX_UNIQUE_QUEUES; i++)
		mSubmitDstWaitMask[i] = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;
}

void VulkanGpuQueue::SubmitCommandBuffer(const GpuSubmissionInformation& information, bool flushTransferCommandBuffer)
{
	if(flushTransferCommandBuffer)
		mGpuDevice.SubmitTransferCommandBuffers();

	if (!B3D_ENSURE(information.CommandBuffer))
		return;

	VulkanGpuCommandBuffer& vulkanCommandBuffer = static_cast<VulkanGpuCommandBuffer&>(*information.CommandBuffer);
	if (!B3D_ENSURE(vulkanCommandBuffer.GetQueueType() == mType))
		return;

	if (vulkanCommandBuffer.GetState() == GpuCommandBufferState::Executing)
	{
		B3D_LOG(Error, LogRenderBackend, "Cannot submit a command buffer that's still executing.");
		return;
	}

	if (!B3D_ENSURE(!vulkanCommandBuffer.IsInRenderPass()))
		vulkanCommandBuffer.EndRenderPass();

	if (vulkanCommandBuffer.IsRecording())
		vulkanCommandBuffer.End();

	vulkanCommandBuffer.SetIsSubmitted();
	GetVulkanSubmitThread().QueueSubmit(std::static_pointer_cast<VulkanGpuCommandBuffer>(information.CommandBuffer), *this, information.SyncMask, information.SignalFences);
}

void VulkanGpuQueue::PresentRenderWindow(const TShared<RenderWindow>& renderWindow, GpuQueueMask syncMask)
{
	if(renderWindow == nullptr)
		return;

	IVulkanRenderWindowSurface* surface = static_cast<IVulkanRenderWindowSurface*>(renderWindow->GetRenderWindowSurface().get());
	if(surface == nullptr)
		return;

	renderWindow->NotifySwapBuffersRequested();
	surface->SwapBuffers(*this, syncMask);

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

	SubmitWorkBuffer& workBuffer = AcquireSubmitWorkBuffer();
	RegisterSemaphoresAndGetHandles(waitSemaphores, workBuffer.WaitSemaphores);

	VkSwapchainKHR vkSwapChain = swapChain->GetHandle();

	VkPresentInfoKHR presentInfo;
	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	presentInfo.pNext = nullptr;
	presentInfo.swapchainCount = 1;
	presentInfo.pSwapchains = &vkSwapChain;
	presentInfo.pImageIndices = &swapChainImageIndex;
	presentInfo.pResults = nullptr;

	// Wait before presenting, if required
	const u32 waitSemaphoreCount = (u32)workBuffer.WaitSemaphores.Size();
	if(waitSemaphoreCount > 0)
	{
		presentInfo.pWaitSemaphores = workBuffer.WaitSemaphores.Data();
		presentInfo.waitSemaphoreCount = waitSemaphoreCount;
	}
	else
	{
		presentInfo.pWaitSemaphores = nullptr;
		presentInfo.waitSemaphoreCount = 0;
	}

	VkResult result = vkQueuePresentKHR(mQueue, &presentInfo);
	B3D_ASSERT(result == VK_SUCCESS || result == VK_SUBOPTIMAL_KHR || result == VK_ERROR_OUT_OF_DATE_KHR);

	mActiveSubmissions.push_back(QueueSubmissionInformation(swapChain, mNextSubmitIndex++, 1));
	mActiveCommandBuffers.push(QueueSubmissionEntryInformation(nullptr, waitSemaphoreCount));

	ReleaseAllSubmitWorkBuffers();
	return result;
}

void VulkanGpuQueue::WaitUntilIdle()
{
	GetVulkanSubmitThread().WaitUntilIdle(*this);
}

VkSubmitInfo VulkanGpuQueue::RegisterSubmissionAndGenerateSubmitInfo(const TShared<VulkanGpuCommandBuffer>& commandBuffer, const TArrayView<VulkanSemaphore*>& waitSemaphores, TArrayView<const GpuTimelineFenceAndValue> signalFences)
{
	TInlineArray<TShared<VulkanGpuCommandBuffer>, 1> commandBuffers = { commandBuffer };
	return RegisterSubmissionAndGenerateSubmitInfo(commandBuffers, waitSemaphores, signalFences);
}

VkSubmitInfo VulkanGpuQueue::RegisterSubmissionAndGenerateSubmitInfo(const TArrayView<TShared<VulkanGpuCommandBuffer>>& commandBuffers, const TArrayView<VulkanSemaphore*>& waitSemaphores, TArrayView<const GpuTimelineFenceAndValue> signalFences)
{
	SubmitWorkBuffer& workBuffer = AcquireSubmitWorkBuffer();

	RegisterSemaphoresAndGetHandles(waitSemaphores, workBuffer.WaitSemaphores);
	const u32 waitSemaphoreCount = (u32)workBuffer.WaitSemaphores.Size();

	u32 commandBufferCount = 0;
	for (const auto& entry : commandBuffers)
	{
		if (!B3D_ENSURE(entry))
			continue;

		entry->SetIsSubmitted();
		workBuffer.CommandBuffers.Add(entry->GetVulkanHandle());
		mActiveCommandBuffers.push(QueueSubmissionEntryInformation(entry, waitSemaphoreCount));
		commandBufferCount++;
	}

	u32 binarySignalCount = 0;
	if (!commandBuffers.IsEmpty())
	{
		binarySignalCount = commandBuffers.back()->AllocateSignalSemaphores(workBuffer.SignalSemaphores);
		for (u32 i = 0; i < binarySignalCount; ++i)
			workBuffer.SignalValues.Add(0);

		mLastSubmittedCommandBuffer = commandBuffers.back(); // Needs to be set because GetSubmitInfo depends on it
		mLastCBSemaphoreUsed = false;
	}

	const bool hasTimelineSignals = !signalFences.IsEmpty();
	if (hasTimelineSignals)
	{
		for (const GpuTimelineFenceAndValue& entry : signalFences)
		{
			B3D_ASSERT(entry.Fence != nullptr);

			VulkanGpuTimelineFence* vulkanFence = static_cast<VulkanGpuTimelineFence*>(entry.Fence.get());
			const VkSemaphore semaphore = vulkanFence->GetTimelineSemaphore();

			if (semaphore == VK_NULL_HANDLE)
				continue;

			workBuffer.SignalSemaphores.Add(semaphore);
			workBuffer.SignalValues.Add(entry.Value);
		}
	}

	B3D_ASSERT(workBuffer.SignalSemaphores.Size() == workBuffer.SignalValues.Size());

	const u32 totalSignalCount = (u32)workBuffer.SignalSemaphores.Size();

	VkSubmitInfo vkSubmitInfo;
	vkSubmitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	vkSubmitInfo.pNext = nullptr;
	vkSubmitInfo.commandBufferCount = commandBufferCount;
	vkSubmitInfo.pCommandBuffers = commandBufferCount > 0 ? workBuffer.CommandBuffers.Data() : nullptr;
	vkSubmitInfo.waitSemaphoreCount = waitSemaphoreCount;

	if(waitSemaphoreCount != 0)
	{
		vkSubmitInfo.pWaitSemaphores = workBuffer.WaitSemaphores.Data();
		vkSubmitInfo.pWaitDstStageMask = mSubmitDstWaitMask;
	}
	else
	{
		vkSubmitInfo.pWaitSemaphores = nullptr;
		vkSubmitInfo.pWaitDstStageMask = nullptr;
	}

	vkSubmitInfo.signalSemaphoreCount = totalSignalCount;
	vkSubmitInfo.pSignalSemaphores = totalSignalCount > 0 ? workBuffer.SignalSemaphores.Data() : nullptr;

	if (hasTimelineSignals)
	{
		workBuffer.TimelineSubmitInfo = {};
		workBuffer.TimelineSubmitInfo.sType = VK_STRUCTURE_TYPE_TIMELINE_SEMAPHORE_SUBMIT_INFO_KHR;
		workBuffer.TimelineSubmitInfo.pNext = nullptr;
		workBuffer.TimelineSubmitInfo.waitSemaphoreValueCount = 0;
		workBuffer.TimelineSubmitInfo.pWaitSemaphoreValues = nullptr;
		workBuffer.TimelineSubmitInfo.signalSemaphoreValueCount = totalSignalCount;
		workBuffer.TimelineSubmitInfo.pSignalSemaphoreValues = totalSignalCount > 0 ? workBuffer.SignalValues.Data() : nullptr;

		vkSubmitInfo.pNext = &workBuffer.TimelineSubmitInfo;
	}

	return vkSubmitInfo;
}

void VulkanGpuQueue::ExecuteSubmitOnSubmitThread(const GpuCommandBufferSubmitInformation& submitInformation, GpuQueueMask syncMask, TArrayView<const GpuTimelineFenceAndValue> signalFences)
{
	AssertIfNotVulkanSubmitThread();

	if (!B3D_ENSURE(submitInformation.PrimaryCommandBuffer))
		return;

	VulkanGpuDevice& device = static_cast<VulkanGpuDevice&>(mGpuDevice);

	// No need to explicitly sync with any entries on the same queue
	const GpuQueueMask queueMask(GetId());
	syncMask &= ~queueMask;

	B3D_ASSERT(B3DSize(submitInformation.SourceQueueTransitionCommandBuffer) == GQT_COUNT);
	for(u32 queueTypeIndex = 0; queueTypeIndex < GQT_COUNT; ++queueTypeIndex)
	{
		if (submitInformation.SourceQueueTransitionCommandBuffer[queueTypeIndex] == nullptr)
			continue;

		const GpuQueueType transitionQueueType = (GpuQueueType)queueTypeIndex;

		// Find an appropriate queue to execute on
		u32 transitionQueueIndex = 0;
		TShared<VulkanGpuQueue> transitionQueue = nullptr;

		const u32 queueCount = device.GetQueueCount(transitionQueueType);
		for(u32 queueIndex = 0; queueIndex < queueCount; queueIndex++)
		{
			// Try to find a queue not currently executing
			const TShared<VulkanGpuQueue>& curQueue = std::static_pointer_cast<VulkanGpuQueue>(device.GetQueue(transitionQueueType, queueIndex));
			if(!curQueue->IsExecuting())
			{
				transitionQueue = curQueue;
				transitionQueueIndex = queueIndex;
			}
		}

		// Can't find empty one, use the first one then
		if(transitionQueue == nullptr)
		{
			transitionQueue = std::static_pointer_cast<VulkanGpuQueue>(device.GetQueue(transitionQueueType, 0));
			transitionQueueIndex = 0;
		}

		syncMask |= GpuQueueId(transitionQueueType, transitionQueueIndex);

		GpuCommandBufferSubmitInformation transitionSubmitInformation;
		transitionSubmitInformation.PrimaryCommandBuffer = submitInformation.SourceQueueTransitionCommandBuffer[queueTypeIndex];

		transitionQueue->ExecuteSubmitOnSubmitThread(transitionSubmitInformation, GpuQueueMask::kNone, {});
	}

	B3D_ENSURE(mWaitSemaphoreBuffer.Empty());
	mWaitSemaphoreBuffer.Append(submitInformation.Semaphores.begin(), submitInformation.Semaphores.end());

	device.GetSyncSemaphores(syncMask, mWaitSemaphoreBuffer);

	B3D_ASSERT(mActiveSubmitWorkBufferCount == 0);

	TInlineArray<VkSubmitInfo, 3> submitInfos;

	if (submitInformation.DestinationQueueTransitionCommandBuffer != nullptr)
	{
		submitInfos.Add(RegisterSubmissionAndGenerateSubmitInfo(submitInformation.DestinationQueueTransitionCommandBuffer, mWaitSemaphoreBuffer));
		mWaitSemaphoreBuffer.Clear(); // No need to wait on these again with later submissions
	}

	submitInfos.Add(RegisterSubmissionAndGenerateSubmitInfo(submitInformation.PrimaryCommandBuffer, mWaitSemaphoreBuffer, signalFences));
	mWaitSemaphoreBuffer.Clear();

	mActiveSubmissions.push_back(QueueSubmissionInformation(mLastSubmittedCommandBuffer, mNextSubmitIndex++, (u32)submitInfos.Size()));

	VkResult result = vkQueueSubmit(mQueue, (u32)submitInfos.Size(), submitInfos.Data(), mLastSubmittedCommandBuffer->GetFence());
	B3D_ASSERT(result == VK_SUCCESS);

	ReleaseAllSubmitWorkBuffers();
}

void VulkanGpuQueue::RefreshCompletionStateOnSubmitThread(bool forceWait, bool queueEmpty, u32 lastSubmitIndex)
{
	AssertIfNotVulkanSubmitThread();

	u32 lastFinishedSubmission = 0;

	auto it = mActiveSubmissions.begin();
	while(it != mActiveSubmissions.end())
	{
		const TShared<VulkanGpuCommandBuffer> cmdBuffer = it->LastSubmittedCommandBuffer;
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

	WaitGroup waitGroup;

	{
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
						semaphore->NotifyDone(0, GpuAccessFlag::Read | GpuAccessFlag::Write);
				});

				waitGroup.Increment();
				if (isPresentCall)
				{
					messageBackQueue.PostCommand([swapChain = it->PresentOperationSwapChain, waitGroup = forceWait ? &waitGroup : nullptr]
					{
						swapChain->NotifyUnbound();

						if(waitGroup != nullptr)
							waitGroup->NotifyDone();
					}, "CommandBufferCompleteCallback");
				}
				else
				{
					messageBackQueue.PostCommand([commandBuffer = queueSubmissionInformation.CommandBuffer, waitGroup = forceWait ? &waitGroup : nullptr]()
					{
						commandBuffer->mState = GpuCommandBufferState::Done;
						commandBuffer->OnDidComplete();
						commandBuffer->Reset();

						if(waitGroup != nullptr)
							waitGroup->NotifyDone();
					}, "CommandBufferCompleteCallback");
				}

				if(mLastSubmittedCommandBuffer == queueSubmissionInformation.CommandBuffer)
					mLastSubmittedCommandBuffer = nullptr;
			}

			it = mActiveSubmissions.erase(it);
		}
	}

	// Ensure the message back callbacks also trigger in the force wait case
	if(forceWait)
		waitGroup.Wait();
}

u32 VulkanGpuQueue::RegisterSemaphoresAndGetHandles(const TArrayView<VulkanSemaphore*>& inSemaphores, TInlineArray<VkSemaphore, 8>& outSemaphores)
{
	AssertIfNotVulkanSubmitThread();

	u32 count = 0;
	for(const auto& semaphore : inSemaphores)
	{
		semaphore->NotifyBound();
		semaphore->NotifyUsed(GetId(), GpuAccessFlag::Read | GpuAccessFlag::Write);

		outSemaphores.Add(semaphore->GetHandle());
		count++;
		mActiveSemaphores.push(semaphore);
	}

	// Wait on previous CB, as we want execution to proceed in order
	if(mLastSubmittedCommandBuffer != nullptr && (mLastSubmittedCommandBuffer->IsSubmitted() || mLastSubmittedCommandBuffer->IsDone()) && !mLastCBSemaphoreUsed)
	{
		VulkanSemaphore* prevSemaphore = mLastSubmittedCommandBuffer->GetIntraQueueSemaphore();

		prevSemaphore->NotifyBound();
		prevSemaphore->NotifyUsed(GetId(), GpuAccessFlag::Read | GpuAccessFlag::Write);

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

VulkanGpuQueue::SubmitWorkBuffer& VulkanGpuQueue::AcquireSubmitWorkBuffer()
{
	AssertIfNotVulkanSubmitThread();

	if (mActiveSubmitWorkBufferCount >= mSubmitWorkBufferPool.size())
		mSubmitWorkBufferPool.push_back(B3DMakeUnique<SubmitWorkBuffer>());

	SubmitWorkBuffer& buffer = *mSubmitWorkBufferPool[mActiveSubmitWorkBufferCount++];
	buffer.Clear();
	return buffer;
}

void VulkanGpuQueue::ReleaseAllSubmitWorkBuffers()
{
	AssertIfNotVulkanSubmitThread();
	mActiveSubmitWorkBufferCount = 0;
}
