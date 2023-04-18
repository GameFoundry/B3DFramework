//************************************ bs::framework - Copyright 2022 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsVulkanSubmitThread.h"
#include "BsVulkanGpuCommandBuffer.h"
#include "BsVulkanGpuDevice.h"
#include "BsVulkanGpuQueue.h"
#include "BsVulkanSwapChain.h"
#include "Threading/BsTaskScheduler.h"

using namespace bs::ct;

static constexpr bool kEnableSubmitThread = true;

static void RunSubmitThreadCommand(bs::WorkerThreadWithCommandQueue& commandQueue, std::function<void()>&& function, const char* commandName, bool waitUntilComplete = false)
{
	if(kEnableSubmitThread)
		commandQueue.QueueCommand(std::move(function), commandName, waitUntilComplete);
	else
		function();
}

VulkanSubmitThread::VulkanSubmitThread()
	: mCommandQueue("VulkanSubmitThread")
{
	auto fnInitialize = [this]()
	{
		const u32 deviceCount = GetVulkanGpuBackend().GetDeviceCount();
		for (u32 deviceIndex = 0; deviceIndex < deviceCount; deviceIndex++)
		{
			const SPtr<VulkanGpuDevice>& device = GetVulkanGpuBackend().GetVulkanDevice(deviceIndex);

			for (u32 gpuQueueUsageIndex = 0; gpuQueueUsageIndex < GQT_COUNT; gpuQueueUsageIndex++)
			{
				const GpuQueueUsage queueUsage = (GpuQueueUsage)gpuQueueUsageIndex;
				if (device->GetQueueCount(queueUsage) == 0)
					continue;

				GpuCommandBufferPoolCreateInformation poolCreateInformation;
				poolCreateInformation.Thread = B3D_CURRENT_THREAD_ID;
				poolCreateInformation.Usage = queueUsage;

				mCommandBufferPools[deviceIndex][gpuQueueUsageIndex] = std::static_pointer_cast<VulkanGpuCommandBufferPool>(device->CreateGpuCommandBufferPool(poolCreateInformation));
			}
		}
	};

	RunSubmitThreadCommand(mCommandQueue, std::move(fnInitialize), "Initialize submit thread");
}

VulkanSubmitThread::~VulkanSubmitThread()
{
	auto fnDestroy = [this]()
	{
		for (auto& poolsOnDevice : mCommandBufferPools)
		{
			for (auto& pool : poolsOnDevice)
			{
				pool = nullptr;
			}
		}
	};

	RunSubmitThreadCommand(mCommandQueue, std::move(fnDestroy), "Cleanup submit thread", true);
}

void VulkanSubmitThread::QueueSubmit(VulkanInternalCommandBuffer& commandBuffer, VulkanGpuQueue& queue, u32 syncMask, bool blocking)
{
	auto fnCommand = [&commandBuffer, &queue, syncMask]()
	{
		if(!commandBuffer.IsReadyForSubmit())
			return;

		commandBuffer.Submit(&queue, syncMask);
	};

	commandBuffer.NotifyWillQueueForSubmit();
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

		TaskScheduler::Instance().AddWorker();
		device.WaitUntilIdle();

		device.DoForEachQueue([](VulkanGpuQueue& queue)
		{
			queue.RefreshCompletionStateOnSubmitThread(true, false);
		});

		swapChain.Present(acquiredImageIndex, queue, syncMask);

		TaskScheduler::Instance().RemoveWorker();
	};

	RunSubmitThreadCommand(mCommandQueue, std::move(fnCommand), "Swap chain present");
	swapChain.NotifyWasPresentQueued(acquiredImageIndex);
}

void VulkanSubmitThread::QueueImageAcquire(VulkanSwapChain& swapChain)
{
	auto fnCommand = [this, &swapChain]
	{
		TaskScheduler::Instance().AddWorker();
		swapChain.AcquireImage();
		TaskScheduler::Instance().RemoveWorker();

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
	auto fnCommand = [performCleanupForShutdown]()
	{
		const u32 deviceCount = GetVulkanGpuBackend().GetDeviceCount();
		for(u32 deviceIndex = 0; deviceIndex < deviceCount; deviceIndex++)
		{
			const SPtr<VulkanGpuDevice>& device = GetVulkanGpuBackend().GetVulkanDevice(deviceIndex);

			TaskScheduler::Instance().AddWorker();
			device->WaitUntilIdle();
			TaskScheduler::Instance().RemoveWorker();

			device->DoForEachQueue([performCleanupForShutdown](VulkanGpuQueue& queue)
			{
				queue.RefreshCompletionStateOnSubmitThread(true, performCleanupForShutdown);
			});
		}
	};

	if(kEnableSubmitThread)
		TaskScheduler::Instance().AddWorker();

	RunSubmitThreadCommand(mCommandQueue, std::move(fnCommand), "Device wait idle", true);

	if(kEnableSubmitThread)
		TaskScheduler::Instance().RemoveWorker();
}

void VulkanSubmitThread::RefreshCommandBufferCompletionStates() const
{
	const u32 deviceCount = GetVulkanGpuBackend().GetDeviceCount();
	for(u32 deviceIndex = 0; deviceIndex < deviceCount; deviceIndex++)
	{
		const SPtr<VulkanGpuDevice>& device = GetVulkanGpuBackend().GetVulkanDevice(deviceIndex);

		device->DoForEachQueue([](VulkanGpuQueue& queue) { queue.RefreshCompletionStateOnRenderThread(); });
	}

	Lock lock(mImageAcquireMutex);
	for(VulkanSwapChain* swapChain : mSwapChainsWithAcquiredImages)
	{
		B3D_ASSERT(swapChain != nullptr);
		swapChain->NotifyUnbound();
	}

	mSwapChainsWithAcquiredImages.clear();
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

		B3D_ASSERT((B3D_CURRENT_THREAD_ID == VulkanSubmitThread::Instance().GetThreadId()) && "This method can only be accessed from the submit thread.");
	}
} // namespace bs::ct
