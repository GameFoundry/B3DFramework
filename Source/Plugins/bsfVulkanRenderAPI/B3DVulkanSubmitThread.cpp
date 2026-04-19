//************************************ B3D Framework - Copyright 2026 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DVulkanSubmitThread.h"
#include "B3DApplication.h"
#include "B3DVulkanGpuCommandBuffer.h"
#include "B3DVulkanGpuDevice.h"
#include "B3DVulkanGpuQueue.h"
#include "B3DVulkanSwapChain.h"
#include "Threading/B3DBlockingCall.h"
#include "Threading/B3DScheduler.h"
#include "Utility/B3DConfigVariable.h"

using namespace b3d;
using namespace b3d::render;

/** Toggle submit thread usage. If disabled all commands are executed on the calling thread - for debugging only. */
static constexpr bool kEnableSubmitThread = true;

static void RunSubmitThreadCommand(SingleConsumerQueue& commandQueue, std::function<void()>&& function, const char* commandName, bool waitUntilComplete = false)
{
	if (kEnableSubmitThread)
		commandQueue.PostCommand(std::move(function), commandName, waitUntilComplete);
	else
		function();
}

VulkanSubmitThread::VulkanSubmitThread(VulkanGpuDevice& gpuDevice)
	: mGpuDevice(gpuDevice)
{
	if (kEnableSubmitThread)
	{
		mCommandQueue.ScheduleRunUntilShutdown(GetApplication().GetTaskScheduler(), false);
	}

	auto fnInitialize = [this]()
	{
		for (u32 queueTypeIndex = 0; queueTypeIndex < GQT_COUNT; queueTypeIndex++)
		{
			const GpuQueueType queueType = (GpuQueueType)queueTypeIndex;
			if (mGpuDevice.GetQueueCount(queueType) == 0)
				continue;

			GpuCommandBufferPoolCreateInformation poolCreateInformation;
			poolCreateInformation.Thread = B3D_CURRENT_THREAD_ID;
			poolCreateInformation.Type = queueType;

			mCommandBufferPools[queueTypeIndex] = std::static_pointer_cast<VulkanGpuCommandBufferPool>(mGpuDevice.CreateGpuCommandBufferPool(poolCreateInformation));
		}
	};

	// Must wait until it starts so we have a fiber assigned for thread id checks
	RunSubmitThreadCommand(mCommandQueue, std::move(fnInitialize), "Initialize submit thread", true);
}

VulkanSubmitThread::~VulkanSubmitThread()
{
	auto fnDestroy = [this]()
	{
		for (auto& pool : mCommandBufferPools)
		{
			pool = nullptr;
		}
	};

	RunSubmitThreadCommand(mCommandQueue, std::move(fnDestroy), "Cleanup submit thread");
	mCommandQueue.PostRequestShutdownCommand(true);
}

void VulkanSubmitThread::QueueSubmit(const SPtr<VulkanGpuCommandBuffer>& commandBuffer, VulkanGpuQueue& queue, GpuQueueMask syncMask, bool blocking)
{
	auto fnCommand = [this, commandBuffer, &queue, syncMask]() mutable
	{
		GpuCommandBufferSubmitInformation submitInformation = commandBuffer->PrepareForSubmitOnSubmitThread(queue.GetType(), queue.GetIndex());

		syncMask |= commandBuffer->GetQueueSyncMask();
		queue.ExecuteSubmitOnSubmitThread(submitInformation, syncMask);

		// Track this as the last submitted command buffer for the current frame
		mCurrentFrameLastCommandBuffer = commandBuffer;
	};

	commandBuffer->NotifyWillQueueForSubmit();
	RunSubmitThreadCommand(mCommandQueue, std::move(fnCommand), "Command buffer submit");

	if (blocking)
		WaitUntilIdle();
}

void VulkanSubmitThread::QueuePresent(VulkanGpuQueue& queue, VulkanSwapChain& swapChain, GpuQueueMask syncMask)
{
	u32 acquiredImageIndex;
	const bool acquireSuccess = swapChain.TryGetFirstAcquiredImageIndex(acquiredImageIndex);
	if(!acquireSuccess)
	{
		B3D_LOG(Error, LogRenderBackend, "Unable to present image. No image has been acquired on the swap chain.");
		return;
	}

	auto fnCommand = [acquiredImageIndex, &queue, &swapChain, syncMask]
	{
		swapChain.Present(acquiredImageIndex, queue, syncMask);
	};

	RunSubmitThreadCommand(mCommandQueue, std::move(fnCommand), "Swap chain present");
	swapChain.NotifyWasPresentQueued(acquiredImageIndex);
}

void VulkanSubmitThread::QueueImageAcquire(VulkanSwapChain& swapChain)
{
	auto fnCommand = [this, &swapChain]
	{
		kEnableSubmitThread ? RunBlockingCallAsYieldable([&swapChain] { swapChain.AcquireImage(); }) : swapChain.AcquireImage();

		swapChain.GetMessageQueue().PostCommand([&swapChain] { swapChain.NotifyUnbound(); });
	};

	B3D_ASSERT(!swapChain.IsRetired());

	swapChain.NotifyWasImageAcquireQueued();
	RunSubmitThreadCommand(mCommandQueue, std::move(fnCommand), "Acquire swap chain image");
}

void VulkanSubmitThread::QueueEndFrameAndWaitForPreviousFrame()
{
	const u32 frameIndex = mCurrentFrameIndex;
	const u32 nextFrameIndex = (frameIndex + 1) % kFrameCount;

	mCurrentFrameIndex = nextFrameIndex;

	// Mark this frame's end processing as pending (will be signalled when submit thread finishes)
	mFrameMarkers[frameIndex].CompletionEvent.Reset();

	auto fnCommand = [this, frameIndex, nextFrameIndex]
	{
		mFrameMarkers[frameIndex].LastCommandBuffer = mCurrentFrameLastCommandBuffer;
		mCurrentFrameLastCommandBuffer = nullptr;

		// Wait for that frame's last command buffer to complete
		const SPtr<VulkanGpuCommandBuffer>& commandBufferToWaitFor = mFrameMarkers[nextFrameIndex].LastCommandBuffer;
		if (commandBufferToWaitFor != nullptr)
		{
			const bool completed = commandBufferToWaitFor->UpdateExecutionStatus(true);
			B3D_ASSERT(completed && "Frame completion wait timed out!");
		}

		// Refresh completion states for finished command buffers
		mGpuDevice.DoForEachQueue([](VulkanGpuQueue& queue)
		{
			queue.RefreshCompletionStateOnSubmitThread(false, false);
		});

		// TODO: This could be signalled earlier. In case the command buffer finishes earlier the submit thread could set the signal
		// before this point. This would avoid the render thread blocking if the command buffer is already finished.
		mFrameMarkers[nextFrameIndex].CompletionEvent.Signal();
	};

	RunSubmitThreadCommand(mCommandQueue, std::move(fnCommand), "End frame");

	// We're about to start rendering frame at 'nextFrameIndex', so we must make sure it has completed on the GPU, and we have sent the Reset() calls to their
	// message queues, as we're about to re-use those command buffers.
	mCurrentFrameIndex = nextFrameIndex;
	mFrameMarkers[nextFrameIndex].CompletionEvent.Wait();
}

void VulkanSubmitThread::WaitUntilIdle(bool performCleanupForShutdown)
{
	auto fnCommand = [this, performCleanupForShutdown]()
	{
		const VkResult result = kEnableSubmitThread ? RunBlockingCallAsYieldable(vkDeviceWaitIdle, mGpuDevice.GetLogical()) : vkDeviceWaitIdle(mGpuDevice.GetLogical());
		B3D_ASSERT(result == VK_SUCCESS);

		mGpuDevice.DoForEachQueue([performCleanupForShutdown](VulkanGpuQueue& queue)
		{
			queue.RefreshCompletionStateOnSubmitThread(true, performCleanupForShutdown);
		});

		if (performCleanupForShutdown)
		{
			for (auto& pool : mCommandBufferPools)
			{
				pool = nullptr;
			}
		}
	};

	RunSubmitThreadCommand(mCommandQueue, std::move(fnCommand), "Device wait idle", true);
}

void VulkanSubmitThread::WaitUntilIdle(VulkanGpuQueue& queue)
{
	auto fnCommand = [&queue]()
	{
		const VkResult result = kEnableSubmitThread ? RunBlockingCallAsYieldable(vkQueueWaitIdle, queue.GetVulkanHandle()) : vkQueueWaitIdle(queue.GetVulkanHandle());
		B3D_ASSERT(result == VK_SUCCESS);

		queue.RefreshCompletionStateOnSubmitThread(true);
	};

	RunSubmitThreadCommand(mCommandQueue, std::move(fnCommand), "Queue wait idle", true);
}

u32 VulkanSubmitThread::GetThreadId() const
{
	return mCommandQueue.GetThreadId();
}

namespace b3d::render {
	VulkanSubmitThread& GetVulkanSubmitThread()
	{
		return VulkanSubmitThread::Instance();
	}

	void AssertIfNotVulkanSubmitThread()
	{
		if(!kEnableSubmitThread)
			return;

		const u32 currentThreadId = Thread::GetCurrentThreadId();
		const u32 submitThreadId = VulkanSubmitThread::Instance().GetThreadId();

		B3D_ASSERT((currentThreadId == submitThreadId) && "This method can only be accessed from the submit thread.");
	}
} // namespace b3d::render
