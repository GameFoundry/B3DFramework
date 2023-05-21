//************************************ bs::framework - Copyright 2022 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsVulkanSubmitThread.h"
#include "BsCoreApplication.h"
#include "BsVulkanGpuCommandBuffer.h"
#include "BsVulkanGpuDevice.h"
#include "BsVulkanGpuQueue.h"
#include "BsVulkanSwapChain.h"
#include "Threading/BsScheduler.h"
#include "Threading/BsTaskScheduler.h"

using namespace bs;
using namespace bs::ct;

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
		auto fnRun = [this]() { mCommandQueue.RunUntilShutdown(); };
		GetCoreApplication().GetTaskScheduler().Post(SchedulerTask(std::move(fnRun)));
	}

	auto fnInitialize = [this]()
	{
		for (u32 gpuQueueUsageIndex = 0; gpuQueueUsageIndex < GQT_COUNT; gpuQueueUsageIndex++)
		{
			const GpuQueueUsage queueUsage = (GpuQueueUsage)gpuQueueUsageIndex;
			if (mGpuDevice.GetQueueCount(queueUsage) == 0)
				continue;

			GpuCommandBufferPoolCreateInformation poolCreateInformation;
			poolCreateInformation.Thread = B3D_CURRENT_THREAD_ID;
			poolCreateInformation.Usage = queueUsage;

			mCommandBufferPools[gpuQueueUsageIndex] = std::static_pointer_cast<VulkanGpuCommandBufferPool>(mGpuDevice.CreateGpuCommandBufferPool(poolCreateInformation));
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

void VulkanSubmitThread::QueueSubmit(const SPtr<VulkanGpuCommandBuffer>& commandBuffer, VulkanGpuQueue& queue, u32 syncMask, bool blocking)
{
	auto fnCommand = [commandBuffer, &queue, syncMask]() mutable
	{
		GpuCommandBufferSubmitInformation submitInformation = commandBuffer->PrepareForSubmitOnSubmitThread(queue.GetUsage(), queue.GetIndex());

		syncMask |= commandBuffer->GetSyncMask();
		queue.ExecuteSubmitOnSubmitThread(submitInformation, syncMask);
	};

	commandBuffer->NotifyWillQueueForSubmit();
	RunSubmitThreadCommand(mCommandQueue, std::move(fnCommand), "Command buffer submit");

	if(blocking)
	{
		WaitUntilIdle();
		RefreshCommandBufferCompletionStates();
	}
}

void VulkanSubmitThread::QueuePresent(VulkanGpuQueue& queue, VulkanSwapChain& swapChain, u32 syncMask)
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

		// TODO - BlockingCall()
		VkResult result = vkDeviceWaitIdle(device.GetLogical());
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
		// TODO - BlockingCall()
		swapChain.AcquireImage();

		Lock acquireLock(mImageAcquireMutex);
		mSwapChainsWithAcquiredImages.push_back(&swapChain);
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
		// TODO - BlockingCall()
		const VkResult result = vkDeviceWaitIdle(mGpuDevice.GetLogical());
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

	if(kEnableSubmitThread)
		TaskScheduler::Instance().AddWorker();

	RunSubmitThreadCommand(mCommandQueue, std::move(fnCommand), "Device wait idle", true);

	if(kEnableSubmitThread)
		TaskScheduler::Instance().RemoveWorker();
}

void VulkanSubmitThread::WaitUntilIdle(VulkanGpuQueue& queue)
{
	auto fnCommand = [&queue]()
	{
		// TODO - BlockingCall()
		const VkResult result = vkQueueWaitIdle(queue.GetVulkanHandle());
		B3D_ASSERT(result == VK_SUCCESS);

		queue.RefreshCompletionStateOnSubmitThread(true);
	};

	if(kEnableSubmitThread)
		TaskScheduler::Instance().AddWorker();

	RunSubmitThreadCommand(mCommandQueue, std::move(fnCommand), "Queue wait idle", true);

	if(kEnableSubmitThread)
		TaskScheduler::Instance().RemoveWorker();
}

void VulkanSubmitThread::RefreshCommandBufferCompletionStates() const
{
	mGpuDevice.DoForEachQueue([](VulkanGpuQueue& queue) { queue.RefreshCompletionStateOnRenderThread(); });

	Lock lock(mImageAcquireMutex);
	for(VulkanSwapChain* swapChain : mSwapChainsWithAcquiredImages)
	{
		B3D_ASSERT(swapChain != nullptr);
		swapChain->NotifyUnbound();
	}

	mSwapChainsWithAcquiredImages.clear();
}

u32 VulkanSubmitThread::GetThreadId() const
{
	return mCommandQueue.GetThreadId();
}

namespace bs::ct {
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
} // namespace bs::ct
