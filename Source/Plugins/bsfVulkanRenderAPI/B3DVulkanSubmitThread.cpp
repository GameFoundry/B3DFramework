//************************************ B3D Framework - Copyright 2022 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DVulkanSubmitThread.h"
#include "B3DApplication.h"
#include "B3DVulkanGpuCommandBuffer.h"
#include "B3DVulkanGpuDevice.h"
#include "B3DVulkanGpuQueue.h"
#include "B3DVulkanSwapChain.h"
#include "Threading/B3DBlockingCall.h"
#include "Threading/B3DScheduler.h"

using namespace b3d;
using namespace b3d::render;

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
	auto fnCommand = [commandBuffer, &queue, syncMask]() mutable
	{
		GpuCommandBufferSubmitInformation submitInformation = commandBuffer->PrepareForSubmitOnSubmitThread(queue.GetType(), queue.GetIndex());

		syncMask |= commandBuffer->GetQueueSyncMask();
		queue.ExecuteSubmitOnSubmitThread(submitInformation, syncMask);
	};

	commandBuffer->NotifyWillQueueForSubmit();
	RunSubmitThreadCommand(mCommandQueue, std::move(fnCommand), "Command buffer submit");

	if (blocking)
	{
		WaitUntilIdle();
	}
}

void VulkanSubmitThread::QueuePresent(VulkanGpuQueue& queue, VulkanSwapChain& swapChain, GpuQueueMask syncMask)
{
	u32 acquiredImageIndex;
	const bool acquireSuccess = swapChain.TryGetFirstAcquiredImageIndex(acquiredImageIndex);
	if(!acquireSuccess)
	{
		B3D_LOG(Error, RenderBackend, "Unable to present image. No image has been acquired on the swap chain.");
		return;
	}

	auto fnCommand = [this, acquiredImageIndex, &queue, &swapChain, syncMask]
	{
		VulkanGpuDevice& device = queue.GetDevice();

		const VkResult result = kEnableSubmitThread ? RunBlockingCallAsYieldable(vkDeviceWaitIdle, device.GetLogical()) : vkDeviceWaitIdle(device.GetLogical());
		B3D_ASSERT(result == VK_SUCCESS);

		device.DoForEachQueue([](VulkanGpuQueue& queue)
		{
			queue.RefreshCompletionStateOnSubmitThread(true, false);
		});

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

void VulkanSubmitThread::QueueRefreshCommandBufferCompletionStates(const VulkanGpuDevice* device)
{
	if(device == nullptr)
		return;

	auto fnCommand = [this, device]
	{
		device->DoForEachQueue([](VulkanGpuQueue& queue) { queue.RefreshCompletionStateOnSubmitThread(false, false); });
	};

	RunSubmitThreadCommand(mCommandQueue, std::move(fnCommand), "Queue command buffer refresh");
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
