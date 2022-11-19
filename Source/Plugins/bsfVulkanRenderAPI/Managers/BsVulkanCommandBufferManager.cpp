//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Managers/BsVulkanCommandBufferManager.h"
#include "BsVulkanCommandBuffer.h"
#include "BsVulkanRenderAPI.h"
#include "BsVulkanDevice.h"
#include "BsVulkanQueue.h"
#include "BsVulkanTexture.h"

using namespace bs;
using namespace bs::ct;

VulkanTransferBuffer::VulkanTransferBuffer(VulkanDevice* device, GpuQueueType type, u32 queueIdx)
	: mDevice(device), mType(type), mQueueIdx(queueIdx)
{
	u32 numQueues = device->GetNumQueues(mType);
	if(numQueues == 0)
	{
		mType = GQT_GRAPHICS;
		numQueues = device->GetNumQueues(GQT_GRAPHICS);
	}

	u32 physicalQueueIdx = queueIdx % numQueues;
	mQueue = device->GetQueue(mType, physicalQueueIdx);
	mQueueMask = device->GetQueueMask(mType, queueIdx);
}

VulkanTransferBuffer::~VulkanTransferBuffer()
{
	if(mCB != nullptr)
		mCB->End();
}

void VulkanTransferBuffer::Allocate()
{
	if(mCB != nullptr)
		return;

	u32 queueFamily = mDevice->GetQueueFamily(mType);
	mCB = mDevice->GetCmdBufferPool().GetBuffer(queueFamily, false);
}

void VulkanTransferBuffer::Flush(bool wait)
{
	if(mCB == nullptr)
		return;

	u32 syncMask = mSyncMask & ~mQueueMask; // Don't sync with itself

	mCB->End();
	mCB->Submit(mQueue, mQueueIdx, syncMask);

	if(wait)
	{
		mQueue->WaitIdle();
		mDevice->RefreshStates(true);

		B3D_ASSERT(!mCB->IsSubmitted());
	}

	mCB = nullptr;
}

VulkanCommandBufferManager::VulkanCommandBufferManager(const VulkanRenderAPI& rapi)
	: mRapi(rapi), mDeviceData(nullptr), mNumDevices(rapi.GetNumDevices())
{
	mDeviceData = B3DNewMultiple<PerDeviceData>(mNumDevices);
	for(u32 i = 0; i < mNumDevices; i++)
	{
		SPtr<VulkanDevice> device = rapi.GetDeviceInternal(i);

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
	B3DDeleteMultiple(mDeviceData, mNumDevices);
}

SPtr<CommandBuffer> VulkanCommandBufferManager::CreateInternal(GpuQueueType type, u32 deviceIdx, u32 queueIdx, bool secondary)
{
	u32 numDevices = mRapi.GetNumDevicesInternal();
	if(deviceIdx >= numDevices)
	{
		B3D_LOG(Error, RenderBackend, "Cannot create command buffer, invalid device index: {0}. Valid range: [0, {1}).", deviceIdx, numDevices);

		return nullptr;
	}

	SPtr<VulkanDevice> device = mRapi.GetDeviceInternal(deviceIdx);

	CommandBuffer* buffer =
		new(B3DAllocate<VulkanCommandBuffer>()) VulkanCommandBuffer(*device, type, deviceIdx, queueIdx, secondary);

	return B3DMakeSharedFromExisting(buffer);
}

void VulkanCommandBufferManager::GetSyncSemaphores(u32 deviceIdx, u32 syncMask, VulkanSemaphore** semaphores, u32& count)
{
	bool semaphoreRequestFailed = false;
	SPtr<VulkanDevice> device = mRapi.GetDeviceInternal(deviceIdx);

	u32 semaphoreIdx = 0;
	for(u32 i = 0; i < GQT_COUNT; i++)
	{
		GpuQueueType queueType = (GpuQueueType)i;

		u32 numQueues = device->GetNumQueues(queueType);
		for(u32 j = 0; j < numQueues; j++)
		{
			VulkanQueue* queue = device->GetQueue(queueType, j);
			VulkanCmdBuffer* lastCB = queue->GetLastCommandBuffer();

			// Check if a buffer is currently executing on the queue
			if(lastCB == nullptr || !lastCB->IsSubmitted())
				continue;

			// Check if we care about this specific queue
			u32 queueMask = device->GetQueueMask(queueType, j);
			if((syncMask & queueMask) == 0)
				continue;

			VulkanSemaphore* semaphore = lastCB->RequestInterQueueSemaphore();
			if(semaphore == nullptr)
			{
				semaphoreRequestFailed = true;
				continue;
			}

			semaphores[semaphoreIdx++] = semaphore;
		}
	}

	count = semaphoreIdx;

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
	B3D_ASSERT(deviceIdx < mNumDevices);

	PerDeviceData& deviceData = mDeviceData[deviceIdx];

	VulkanTransferBuffer* transferBuffer = &deviceData.TransferBuffers[type][queueIdx];
	transferBuffer->Allocate();
	return transferBuffer;
}

void VulkanCommandBufferManager::FlushTransferBuffers(u32 deviceIdx)
{
	B3D_ASSERT(deviceIdx < mNumDevices);

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
