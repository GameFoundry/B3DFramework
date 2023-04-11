//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Managers/BsVulkanCommandBufferManager.h"
#include "BsVulkanCommandBuffer.h"
#include "BsVulkanRenderAPI.h"
#include "BsVulkanGpuDevice.h"
#include "BsVulkanQueue.h"
#include "BsVulkanSubmitThread.h"
#include "BsVulkanTexture.h"

using namespace bs;
using namespace bs::ct;

VulkanTransferBuffer::VulkanTransferBuffer(VulkanGpuDevice* device, GpuQueueType type, u32 queueIdx)
	: mDevice(device), mType(type), mQueueIndex(queueIdx)
{
	u32 queueCount = device->GetQueueCountForType(mType);
	if(queueCount == 0)
	{
		mType = GQT_GRAPHICS;
		queueCount = device->GetQueueCountForType(GQT_GRAPHICS);
	}

	const u32 physicalQueueIndex = queueIdx % queueCount;
	mQueue = device->GetQueue(mType, physicalQueueIndex);
	mQueueMask = device->GetQueueMask(mType, queueIdx);
}

VulkanTransferBuffer::~VulkanTransferBuffer()
{
	if(mCommandBuffer != nullptr)
		mCommandBuffer->End();
}

void VulkanTransferBuffer::Allocate()
{
	if(mCommandBuffer != nullptr)
		return;

	const u32 queueFamily = mDevice->GetQueueFamily(mType);
	mCommandBuffer = mDevice->GetCommandBufferPool().GetBuffer(queueFamily);
}

void VulkanTransferBuffer::Flush(bool wait)
{
	// If not recording we assume it was already submitted externally
	if(mCommandBuffer == nullptr || !mCommandBuffer->IsRecording())
		return;

	const u32 syncMask = mSyncMask & ~mQueueMask; // Don't sync with itself

	mCommandBuffer->End();

	GetVulkanSubmitThread().QueueSubmit(*mCommandBuffer, *mQueue, mQueueIndex, syncMask);
	mCommandBuffer = nullptr;

	if(wait)
	{
		GetVulkanSubmitThread().WaitUntilIdle();
		GetVulkanSubmitThread().RefreshCommandBufferCompletionStates();
	}
}

VulkanCommandBufferManager::VulkanCommandBufferManager()
	: mDeviceData(nullptr), mDeviceCount(GetVulkanGpuBackend().GetDeviceCount())
{
	mDeviceData = B3DNewMultiple<PerDeviceData>(mDeviceCount);
	for(u32 i = 0; i < mDeviceCount; i++)
	{
		const SPtr<VulkanGpuDevice>& device = GetVulkanGpuBackend().GetVulkanDevice(i);

		for(u32 j = 0; j < GQT_COUNT; j++)
		{
			GpuQueueType queueType = (GpuQueueType)j;

			for(u32 k = 0; k < BS_MAX_QUEUES_PER_TYPE; k++)
				mDeviceData[i].TransferBuffers[j][k] = VulkanTransferBuffer(device.get(), queueType, k);
		}
	}
}

VulkanCommandBufferManager::~VulkanCommandBufferManager()
{
	B3DDeleteMultiple(mDeviceData, mDeviceCount);
}

SPtr<CommandBuffer> VulkanCommandBufferManager::CreateInternal(GpuQueueType queueType)
{
	const SPtr<VulkanGpuDevice>& device = GetVulkanGpuBackend().GetVulkanDevice(0);

	CommandBuffer* buffer = new(B3DAllocate<VulkanCommandBuffer>()) VulkanCommandBuffer(*device, queueType);

	return B3DMakeSharedFromExisting(buffer);
}

void VulkanCommandBufferManager::GetSyncSemaphores(u32 deviceIdx, u32 syncMask, VulkanSemaphore** semaphores, u32& count)
{
	AssertIfNotVulkanSubmitThread();

	bool semaphoreRequestFailed = false;
	const SPtr<VulkanGpuDevice>& device = GetVulkanGpuBackend().GetVulkanDevice(deviceIdx);

	u32 semaphoreIndex = 0;
	for(u32 queueTypeIndex = 0; queueTypeIndex < GQT_COUNT; queueTypeIndex++)
	{
		const GpuQueueType queueType = (GpuQueueType)queueTypeIndex;

		const u32 queueCount = device->GetQueueCountForType(queueType);
		for(u32 queueIndex = 0; queueIndex < queueCount; queueIndex++)
		{
			VulkanQueue* queue = device->GetQueue(queueType, queueIndex);
			VulkanInternalCommandBuffer* lastCommandBuffer = queue->GetLastCommandBuffer();

			// Check if a buffer is currently executing on the queue
			if(lastCommandBuffer == nullptr || (!lastCommandBuffer->IsSubmitted() && !lastCommandBuffer->IsDone()))
				continue;

			// Check if we care about this specific queue
			u32 queueMask = device->GetQueueMask(queueType, queueIndex);
			if((syncMask & queueMask) == 0)
				continue;

			VulkanSemaphore* semaphore = lastCommandBuffer->RequestInterQueueSemaphore();
			if(semaphore == nullptr)
			{
				semaphoreRequestFailed = true;
				continue;
			}

			semaphores[semaphoreIndex++] = semaphore;
		}
	}

	count = semaphoreIndex;

	if(semaphoreRequestFailed)
	{
		B3D_LOG(Error, RenderBackend, "Failed to allocate semaphores for a command buffer sync. This means some of the "
									 "dependency requests will not be fulfilled. This happened because a command buffer has too many "
									 "dependant command buffers. The maximum allowed number is {0} but can be increased by incrementing the "
									 "value of BS_MAX_VULKAN_CB_DEPENDENCIES.",
			   BS_MAX_VULKAN_CB_DEPENDENCIES);
	}
}

VulkanTransferBuffer* VulkanCommandBufferManager::GetTransferBuffer(u32 deviceIdx, GpuQueueType type, u32 queueIdx)
{
	B3D_ASSERT(deviceIdx < mDeviceCount);

	PerDeviceData& deviceData = mDeviceData[deviceIdx];

	VulkanTransferBuffer* transferBuffer = &deviceData.TransferBuffers[type][queueIdx];
	transferBuffer->Allocate();
	return transferBuffer;
}

void VulkanCommandBufferManager::FlushTransferBuffers(u32 deviceIdx)
{
	B3D_ASSERT(deviceIdx < mDeviceCount);

	PerDeviceData& deviceData = mDeviceData[deviceIdx];
	for(u32 i = 0; i < GQT_COUNT; i++)
	{
		for(u32 j = 0; j < BS_MAX_QUEUES_PER_TYPE; j++)
			deviceData.TransferBuffers[i][j].Flush(false);
	}
}

namespace bs { namespace ct {
VulkanCommandBufferManager& GetVulkanCommandBufferManager()
{
	return static_cast<VulkanCommandBufferManager&>(CommandBufferManager::Instance());
}
}} // namespace bs::ct
