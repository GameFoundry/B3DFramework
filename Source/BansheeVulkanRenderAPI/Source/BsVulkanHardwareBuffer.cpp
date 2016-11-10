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
		: HardwareBuffer(size), mBuffers(), mStaging(type == BT_STAGING)
	{
		bool needsView = false;

		VkMemoryPropertyFlags flags = mStaging ?
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
				continue;

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
	}

	VulkanHardwareBuffer::~VulkanHardwareBuffer()
	{
		for (UINT32 i = 0; i < BS_MAX_DEVICES; i++)
		{
			if (mBuffers[i] == nullptr)
				continue;

			mBuffers[i]->destroy();
		}
	}

	void* VulkanHardwareBuffer::map(UINT32 offset, UINT32 length, GpuLockOptions options, UINT32 deviceIdx, UINT32 queueIdx)
	{
		if ((offset + length) > mSize)
		{
			LOGERR("Provided offset(" + toString(offset) + ") + length(" + toString(length) + ") "
				   "is larger than the buffer " + toString(mSize) + ".");

			return nullptr;
		}

		VulkanBuffer* buffer = mBuffers[deviceIdx];

		if (buffer == nullptr)
			return;

		bool directMap = mStaging && !buffer->isUsed();

		// If memory is host visible and buffer isn't used on the GPU, map directly (no need for pipeline barriers
		// with access modifiers since we're sure the buffer isn't used on the GPU)
		if (directMap)
			return buffer->map();

		// TODO - Allocate staging buffer

		bool needRead = options == GBL_READ_WRITE || options == GBL_READ_ONLY;
		if(needRead)
		{
			// TODO - Get command buffer on wanted queue (getTransferBuffer(deviceIdx, queueIdx))
			//      - Generate sync mask depending on where the resource is used on (VulkanResource::getUseInfo())
			//      - Register this buffer and staging buffer with the transfer buffer, updating the transfer buffer's sync mask
			//      - Flush the transfer buffer, wait for it to complete, and refresh CB states
			//      - Proceed below
		}

		// TODO - Return staging buffer->map()
		//      - Set mRequiresUpload field to true
		//      - Remember lock mode
		//      - Remember staging buffer
		//      - Remember lock queue and device

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
		// TODO - If direct map (mRequiresUpload == false), simply unmap
		// TODO - If mRequiresUpload is true
		//      - Get command buffer on locked queue and device
		//      - If lock was discard
		//        - Create a brand new internal buffer
		//        - Call destroy on the old one
		//        - Issue copy on the CB without a sync mask (register both resources on CB)
		//      - If lock was no overwrite
		//        - Issue copy on the CB without a sync mask (register both resources on CB)
		//      - Otherwise issue copy with a sync mask depending on current use flags
		//      - Destroy staging buffer
	}

	void VulkanHardwareBuffer::copyData(HardwareBuffer& srcBuffer, UINT32 srcOffset,
		UINT32 dstOffset, UINT32 length, bool discardWholeBuffer, UINT32 queueIdx)
	{
		// TODO - Queue copy command on the requested queue
		//      - Issue semaphores if src buffer is currently used
		//      - Otherwise just create a new buffer and write to it
		//      - If current buffer is currently being written to by the GPU, issue a wait and log a performance warning
	}

	void VulkanHardwareBuffer::readData(UINT32 offset, UINT32 length, void* pDest, UINT32 queueIdx)
	{
		// TODO - Just use lock/unlock
	}

	void VulkanHardwareBuffer::writeData(UINT32 offset, UINT32 length, const void* pSource, BufferWriteType writeFlags, 
		UINT32 queueIdx)
	{
		// TODO - Just use lock/unlock
	}
}