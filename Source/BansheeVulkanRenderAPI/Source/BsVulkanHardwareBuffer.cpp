//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsVulkanHardwareBuffer.h"
#include "BsVulkanRenderAPI.h"
#include "BsVulkanDevice.h"
#include "BsVulkanUtility.h"
#include "BsException.h"

namespace BansheeEngine
{
	VulkanBuffer::VulkanBuffer(VulkanResourceManager* owner, VkBuffer buffer, VkBufferView view, VkDeviceMemory memory)
		:VulkanResource(owner, false), mBuffer(buffer), mView(view), mMemory(memory)
	{

	}

	VulkanBuffer::~VulkanBuffer()
	{
		VulkanDevice& device = mOwner->getDevice();

		if (mView != VK_NULL_HANDLE)
			vkDestroyBufferView(device.getLogical(), mView, gVulkanAllocator);

		vkDestroyBuffer(device.getLogical(), mBuffer, gVulkanAllocator);
		device.freeMemory(mMemory);
	}

	VulkanHardwareBuffer::VulkanHardwareBuffer(BufferType type, GpuBufferFormat format, GpuBufferUsage usage, 
		UINT32 size, GpuDeviceFlags deviceMask)
		: HardwareBuffer(usage), mBuffers()
	{
		bool staging = type == BT_STAGING;
		bool needsView = false;

		VkMemoryPropertyFlags flags = staging ?
			(VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT) : // Note: Try using cached uncoherent memory
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;

		VkBufferUsageFlags usageFlags = 0;
		switch(type)
		{
		case BT_VERTEX:
			usageFlags = VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
			break;
		case BT_INDEX:
			usageFlags = VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
			break;
		case BT_UNIFORM:
			usageFlags = VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
			break;
		case BT_GENERIC:
			usageFlags = VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_UNIFORM_TEXEL_BUFFER_BIT;
			needsView = true;
			break;
		case BT_STORAGE:
			usageFlags = VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_STORAGE_TEXEL_BUFFER_BIT;
			needsView = true;
			break;
		case BT_STAGING:
			usageFlags = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
			break;
		}

		VulkanRenderAPI& rapi = static_cast<VulkanRenderAPI&>(RenderAPICore::instance());
		VulkanDevice* devices[BS_MAX_DEVICES];
		VulkanUtility::getDevices(rapi, deviceMask, devices);

		// Allocate buffers per-device
		for (UINT32 i = 0; i < BS_MAX_DEVICES; i++)
		{
			if (devices[i] == nullptr)
				break;

			VkBufferCreateInfo bufferCI;
			bufferCI.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
			bufferCI.pNext = nullptr;
			bufferCI.flags = 0; 
			bufferCI.size = size;
			bufferCI.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
			bufferCI.usage = usageFlags;
			bufferCI.queueFamilyIndexCount = 0;
			bufferCI.pQueueFamilyIndices = nullptr;

			VkDevice device = devices[i]->getLogical();

			VkBuffer buffer;
			VkResult result = vkCreateBuffer(device, &bufferCI, gVulkanAllocator, &buffer);
			assert(result == VK_SUCCESS);

			VkMemoryRequirements memReqs;
			vkGetBufferMemoryRequirements(device, buffer, &memReqs);

			VkDeviceMemory memory = devices[i]->allocateMemory(memReqs, flags);
			result = vkBindBufferMemory(device, buffer, memory, 0);
			assert(result == VK_SUCCESS);

			VkBufferView view;
			if (needsView)
			{
				VkBufferViewCreateInfo bufferViewCI;
				bufferViewCI.sType = VK_STRUCTURE_TYPE_BUFFER_VIEW_CREATE_INFO;
				bufferViewCI.pNext = nullptr;
				bufferViewCI.flags = 0;
				bufferViewCI.buffer = buffer;
				bufferViewCI.format = VulkanUtility::getBufferFormat(format);
				bufferViewCI.offset = 0;
				bufferViewCI.range = VK_WHOLE_SIZE;

				result = vkCreateBufferView(device, &bufferViewCI, gVulkanAllocator, &view);
				assert(result == VK_SUCCESS);
			}
			else
				view = VK_NULL_HANDLE;

			mBuffers[i] = devices[i]->getResourceManager().create<VulkanBuffer>(buffer, view, memory);
		}

		mSizeInBytes = size;
	}

	VulkanHardwareBuffer::~VulkanHardwareBuffer()
	{
		for (UINT32 i = 0; i < BS_MAX_DEVICES; i++)
		{
			if (mBuffers[i] == nullptr)
				return;

			mBuffers[i]->destroy();
		}
	}

	void* VulkanHardwareBuffer::map(UINT32 offset, UINT32 length, GpuLockOptions options, UINT32 syncMask)
	{
		if ((offset + length) > mSizeInBytes)
			BS_EXCEPT(RenderingAPIException, "Provided offset(" + toString(offset) + ") + length(" + toString(length) + ") "
				"is larger than the buffer " + toString(mSizeInBytes) + ".");		

		switch (options)
		{
		case GBL_WRITE_ONLY_DISCARD:

			break;
		case GBL_WRITE_ONLY_NO_OVERWRITE:

			break;
		case GBL_WRITE_ONLY:

			break;
		case GBL_READ_WRITE:

			break;
		case GBL_READ_ONLY:

			break;
		}

		return nullptr;
	}

	void VulkanHardwareBuffer::unmap()
	{

	}

	void VulkanHardwareBuffer::copyData(HardwareBuffer& srcBuffer, UINT32 srcOffset,
		UINT32 dstOffset, UINT32 length, bool discardWholeBuffer, UINT32 syncMask)
	{

	}

	void VulkanHardwareBuffer::readData(UINT32 offset, UINT32 length, void* pDest, UINT32 syncMask)
	{
		
	}

	void VulkanHardwareBuffer::writeData(UINT32 offset, UINT32 length, const void* pSource, BufferWriteType writeFlags, UINT32 syncMask)
	{
		
	}
}