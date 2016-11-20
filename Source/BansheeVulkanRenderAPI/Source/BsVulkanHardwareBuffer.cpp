//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsVulkanHardwareBuffer.h"
#include "BsVulkanRenderAPI.h"
#include "BsVulkanDevice.h"
#include "BsVulkanUtility.h"
#include "BsVulkanCommandBufferManager.h"
#include "BsVulkanCommandBuffer.h"

namespace bs
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

	UINT8* VulkanBuffer::map(VkDeviceSize offset, VkDeviceSize length) const
	{
		VulkanDevice& device = mOwner->getDevice();

		UINT8* data;
		VkResult result = vkMapMemory(device.getLogical(), mMemory, offset, length, 0, (void**)&data);
		assert(result == VK_SUCCESS);

		return data;
	}

	void VulkanBuffer::unmap()
	{
		VulkanDevice& device = mOwner->getDevice();

		vkUnmapMemory(device.getLogical(), mMemory);
	}

	void VulkanBuffer::copy(VulkanTransferBuffer* cb, VulkanBuffer* destination, VkDeviceSize srcOffset, 
		VkDeviceSize dstOffset, VkDeviceSize length)
	{
		VkBufferCopy region;
		region.size = length;
		region.srcOffset = srcOffset;
		region.dstOffset = dstOffset;

		vkCmdCopyBuffer(cb->getCB()->getHandle(), mBuffer, destination->getHandle(), 1, &region);
	}

	VulkanHardwareBuffer::VulkanHardwareBuffer(BufferType type, GpuBufferFormat format, GpuBufferUsage usage, 
		UINT32 size, GpuDeviceFlags deviceMask)
		: HardwareBuffer(size), mBuffers(), mStagingBuffer(nullptr), mMappedDeviceIdx(-1), mMappedGlobalQueueIdx(-1)
		, mMappedOffset(0), mMappedSize(0), mMappedLockOptions(GBL_WRITE_ONLY)
		, mDirectlyMappable((usage & GBU_DYNAMIC) != 0)
		, mSupportsGPUWrites(type == BT_STORAGE), mRequiresView(false), mReadable((usage & GBU_READABLE) != 0)
		, mIsMapped(false)
	{
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
			mRequiresView = true;
			break;
		case BT_STORAGE:
			usageFlags = VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_STORAGE_TEXEL_BUFFER_BIT;
			mRequiresView = true;
			break;
		}

		mBufferCI.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		mBufferCI.pNext = nullptr;
		mBufferCI.flags = 0;
		mBufferCI.size = size;
		mBufferCI.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		mBufferCI.usage = usageFlags;
		mBufferCI.queueFamilyIndexCount = 0;
		mBufferCI.pQueueFamilyIndices = nullptr;

		mViewCI.sType = VK_STRUCTURE_TYPE_BUFFER_VIEW_CREATE_INFO;
		mViewCI.pNext = nullptr;
		mViewCI.flags = 0;
		mViewCI.format = VulkanUtility::getBufferFormat(format);
		mViewCI.offset = 0;
		mViewCI.range = VK_WHOLE_SIZE;

		VulkanRenderAPI& rapi = static_cast<VulkanRenderAPI&>(RenderAPICore::instance());
		VulkanDevice* devices[BS_MAX_DEVICES];
		VulkanUtility::getDevices(rapi, deviceMask, devices);

		// Allocate buffers per-device
		for (UINT32 i = 0; i < BS_MAX_DEVICES; i++)
		{
			if (devices[i] == nullptr)
				continue;

			mBuffers[i] = createBuffer(*devices[i], false, mReadable);
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

		assert(mStagingBuffer == nullptr);
	}

	VulkanBuffer* VulkanHardwareBuffer::createBuffer(VulkanDevice& device, bool staging, bool readable)
	{
		VkBufferUsageFlags usage = mBufferCI.usage;
		if (staging)
		{
			mBufferCI.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;

			// Staging buffers are used as a destination for reads
			if (readable)
				mBufferCI.usage |= VK_BUFFER_USAGE_TRANSFER_DST_BIT;
		}
		else if(readable) // Non-staging readable
			mBufferCI.usage |= VK_BUFFER_USAGE_TRANSFER_SRC_BIT;

		VkMemoryPropertyFlags flags = (mDirectlyMappable || staging) ?
			(VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT) : // Note: Try using cached memory
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;

		VkDevice vkDevice = device.getLogical();

		VkBuffer buffer;
		VkResult result = vkCreateBuffer(vkDevice, &mBufferCI, gVulkanAllocator, &buffer);
		assert(result == VK_SUCCESS);

		VkMemoryRequirements memReqs;
		vkGetBufferMemoryRequirements(vkDevice, buffer, &memReqs);

		VkDeviceMemory memory = device.allocateMemory(memReqs, flags);
		result = vkBindBufferMemory(vkDevice, buffer, memory, 0);
		assert(result == VK_SUCCESS);

		VkBufferView view;
		if (mRequiresView && !staging)
		{
			mViewCI.buffer = buffer;

			result = vkCreateBufferView(vkDevice, &mViewCI, gVulkanAllocator, &view);
			assert(result == VK_SUCCESS);
		}
		else
			view = VK_NULL_HANDLE;

		mBufferCI.usage = usage; // Restore original usage
		return device.getResourceManager().create<VulkanBuffer>(buffer, view, memory);
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
			return nullptr;

		mIsMapped = true;
		mMappedDeviceIdx = deviceIdx;
		mMappedGlobalQueueIdx = queueIdx;
		mMappedOffset = offset;
		mMappedSize = length;
		mMappedLockOptions = options;

		VulkanRenderAPI& rapi = static_cast<VulkanRenderAPI&>(RenderAPICore::instance());
		VulkanDevice& device = *rapi._getDevice(deviceIdx);

		VulkanCommandBufferManager& cbManager = gVulkanCBManager();
		GpuQueueType queueType;
		UINT32 localQueueIdx = CommandSyncMask::getQueueIdxAndType(queueIdx, queueType);

		VkAccessFlags accessFlags;
		if (options == GBL_READ_ONLY)
			accessFlags = VK_ACCESS_HOST_READ_BIT;
		else if (options == GBL_READ_WRITE)
			accessFlags = VK_ACCESS_HOST_READ_BIT | VK_ACCESS_HOST_WRITE_BIT;
		else
			accessFlags = VK_ACCESS_HOST_WRITE_BIT;

		// If memory is host visible try mapping it directly
		if(mDirectlyMappable)
		{
			// If GPU has the ability to write to the buffer we must issue a pipeline barrier to prevent any memory hazards
			//  - Additionally it might be possible the GPU is /currently/ writing to the buffer, in which case we need to
			//    wait for those writes to finish before continuing
			if(mSupportsGPUWrites) // Note: It might be tempting to only do this step only if buffer is currently being 
								   // written to, but that doesn't guarantee memory visibility if it was written to recently
			{
				// First try to avoid the expensive wait operation and barrier
				if(options == GBL_WRITE_ONLY_NO_OVERWRITE) // Caller guarantees he won't touch the same data as the GPU, so just map
					return buffer->map(offset, length);

				if(options == GBL_WRITE_ONLY_DISCARD) // Caller doesn't care about buffer contents, so just discard the 
				{									  // existing buffer and create a new one
					buffer->destroy();

					buffer = createBuffer(device, false, mReadable);
					mBuffers[deviceIdx] = buffer;

					return buffer->map(offset, length);
				}

				// Otherwise we need to wait until (potential) GPU write completes, and issue a barrier so:
				//  - If reading: the device makes the written memory available for read (read-after-write hazard)
				//  - If writing: ensures our writes properly overlap with GPU writes (write-after-write hazard)
				VulkanTransferBuffer* transferCB = cbManager.getTransferBuffer(deviceIdx, queueType, localQueueIdx);

				// Ensure flush() will wait for all queues currently writing to the buffer (if any) to finish
				UINT32 writeUseMask = buffer->getUseInfo(VulkanUseFlag::Write);
				transferCB->appendMask(writeUseMask); 

				// Issue barrier to avoid memory hazards
				transferCB->memoryBarrier(buffer->getHandle(),
										  VK_ACCESS_SHADER_WRITE_BIT,
										  accessFlags,
										  // Last stages that could have written to the buffer:
										  VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT | VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, 
										  VK_PIPELINE_STAGE_HOST_BIT
				);

				// Submit the command buffer and wait until it finishes
				transferCB->flush(true);
				assert(!buffer->isUsed());
			}

			return buffer->map(offset, length);
		}
		else // Otherwise we use a staging buffer
		{
			bool needRead = options == GBL_READ_WRITE || options == GBL_READ_ONLY;

			// Allocate a staging buffer
			mStagingBuffer = createBuffer(device, true, needRead);

			if (needRead) // If reading, we need to copy the current contents of the buffer to the staging buffer
			{
				VulkanTransferBuffer* transferCB = cbManager.getTransferBuffer(deviceIdx, queueType, localQueueIdx);
				
				// Similar to above, if buffer supports GPU writes, we need to wait on any potential writes to complete
				if(mSupportsGPUWrites)
				{
					// Ensure flush() will wait for all queues currently writing to the buffer (if any) to finish
					UINT32 writeUseMask = buffer->getUseInfo(VulkanUseFlag::Write);
					transferCB->appendMask(writeUseMask);
				}

				// Queue copy command
				buffer->copy(transferCB, mStagingBuffer, offset, offset, length);

				// Ensure data written to the staging buffer is visible
				transferCB->memoryBarrier(buffer->getHandle(),
										  VK_ACCESS_TRANSFER_WRITE_BIT,
										  accessFlags,
										  VK_PIPELINE_STAGE_TRANSFER_BIT,
										  VK_PIPELINE_STAGE_HOST_BIT
				);

				// Submit the command buffer and wait until it finishes
				transferCB->flush(true);
				assert(!buffer->isUsed());
			}

			return mStagingBuffer->map(offset, length);
		}
	}

	void VulkanHardwareBuffer::unmap()
	{
		// Possibly map() failed with some error
		if (!mIsMapped)
			return;

		// Note: If we did any writes they need to be made visible to the GPU. However there is no need to execute 
		// a pipeline barrier because (as per spec) host writes are implicitly visible to the device.

		if(mDirectlyMappable)
			mBuffers[mMappedDeviceIdx]->unmap();
		else
		{
			bool isWrite = mMappedLockOptions != GBL_READ_ONLY;

			// We the caller wrote anything to the staging buffer, we need to upload it back to the main buffer
			if(isWrite)
			{
				VulkanRenderAPI& rapi = static_cast<VulkanRenderAPI&>(RenderAPICore::instance());
				VulkanDevice& device = *rapi._getDevice(mMappedDeviceIdx);

				VulkanCommandBufferManager& cbManager = gVulkanCBManager();
				GpuQueueType queueType;
				UINT32 localQueueIdx = CommandSyncMask::getQueueIdxAndType(mMappedGlobalQueueIdx, queueType);

				VulkanBuffer* buffer = mBuffers[mMappedDeviceIdx];
				VulkanTransferBuffer* transferCB = cbManager.getTransferBuffer(mMappedDeviceIdx, queueType, localQueueIdx);

				// If the buffer is used in any way on the GPU, we need to wait for that use to finish before
				// we issue our copy
				UINT32 useMask = buffer->getUseInfo(VulkanUseFlag::Read | VulkanUseFlag::Write);
				if(useMask != 0) // Buffer is currently used on the GPU
				{
					// Try to avoid the wait
					if (mMappedLockOptions == GBL_WRITE_ONLY_NO_OVERWRITE) // Caller guarantees he won't touch the same data as the GPU, so just copy
					{
						// Fall through to copy()
					}
					else if (mMappedLockOptions == GBL_WRITE_ONLY_DISCARD) // Caller doesn't care about buffer contents, so just discard the 
					{													   // existing buffer and create a new one
						buffer->destroy();

						buffer = createBuffer(device, false, mReadable);
						mBuffers[mMappedDeviceIdx] = buffer;
					} 
					else // Otherwise we have no choice but to issue a dependency between the queues
						transferCB->appendMask(useMask);
				}
				
				// Queue copy command
				mStagingBuffer->copy(transferCB, buffer, mMappedOffset, mMappedOffset, mMappedSize);

				// Notify the command buffer that these resources are being used on it
				transferCB->getCB()->registerResource(mStagingBuffer, VK_ACCESS_TRANSFER_READ_BIT, VulkanUseFlag::Read);
				transferCB->getCB()->registerResource(buffer, VK_ACCESS_TRANSFER_WRITE_BIT, VulkanUseFlag::Write);

				// We don't actually flush the transfer buffer here since it's an expensive operation, but it's instead
				// done automatically before next "normal" command buffer submission.
			}

			mStagingBuffer->unmap();

			mStagingBuffer->destroy();
			mStagingBuffer = nullptr;
		}

		mIsMapped = false;
	}

	void VulkanHardwareBuffer::copyData(HardwareBuffer& srcBuffer, UINT32 srcOffset,
		UINT32 dstOffset, UINT32 length, bool discardWholeBuffer, UINT32 queueIdx)
	{
		if ((dstOffset + length) > mSize)
		{
			LOGERR("Provided offset(" + toString(dstOffset) + ") + length(" + toString(length) + ") "
				   "is larger than the destination buffer " + toString(mSize) + ". Copy operation aborted.");

			return;
		}

		if ((srcOffset + length) > srcBuffer.getSize())
		{
			LOGERR("Provided offset(" + toString(srcOffset) + ") + length(" + toString(length) + ") "
				   "is larger than the source buffer " + toString(srcBuffer.getSize()) + ". Copy operation aborted.");

			return;
		}

		VulkanHardwareBuffer& vkSource = static_cast<VulkanHardwareBuffer&>(srcBuffer);

		VulkanRenderAPI& rapi = static_cast<VulkanRenderAPI&>(RenderAPICore::instance());
		VulkanCommandBufferManager& cbManager = gVulkanCBManager();

		GpuQueueType queueType;
		UINT32 localQueueIdx = CommandSyncMask::getQueueIdxAndType(queueIdx, queueType);

		// Perform copy on every device that has both buffers
		for (UINT32 i = 0; i < BS_MAX_DEVICES; i++)
		{
			VulkanBuffer* src = vkSource.mBuffers[i];
			VulkanBuffer* dst = mBuffers[i];

			if (src == nullptr || dst == nullptr)
				continue;

			VulkanDevice& device = *rapi._getDevice(i);
			VulkanTransferBuffer* transferCB = cbManager.getTransferBuffer(i, queueType, localQueueIdx);

			// If either source or destination buffer is currently being written to do need to sync the copy operation so
			// it executes after both are done

			// If destination is being used on the GPU we need to wait until it finishes before writing to it
			UINT32 dstUseMask = dst->getUseInfo(VulkanUseFlag::Read | VulkanUseFlag::Write);

			// If discard is enabled and destination is used, instead of waiting just discard the existing buffer and make a new one
			if(dstUseMask != 0 && discardWholeBuffer)
			{
				dst->destroy();

				dst = createBuffer(device, false, mReadable);
				mBuffers[mMappedDeviceIdx] = dst;

				dstUseMask = 0;
			}

			// If source buffer is being written to on the GPU we need to wait until it finishes, before executing copy
			UINT32 srcUseMask = src->getUseInfo(VulkanUseFlag::Write);

			// Wait if anything is using the buffers
			if(dstUseMask != 0 || srcUseMask != 0)
				transferCB->appendMask(dstUseMask | srcUseMask);

			src->copy(transferCB, dst, srcOffset, dstOffset, length);

			// Notify the command buffer that these resources are being used on it
			transferCB->getCB()->registerResource(src, VK_ACCESS_TRANSFER_READ_BIT, VulkanUseFlag::Read);
			transferCB->getCB()->registerResource(dst, VK_ACCESS_TRANSFER_WRITE_BIT, VulkanUseFlag::Write);

			// We don't actually flush the transfer buffer here since it's an expensive operation, but it's instead
			// done automatically before next "normal" command buffer submission.
		}
	}

	void VulkanHardwareBuffer::readData(UINT32 offset, UINT32 length, void* dest, UINT32 deviceIdx, UINT32 queueIdx)
	{
		void* lockedData = lock(offset, length, GBL_READ_ONLY, deviceIdx, queueIdx);
		memcpy(dest, lockedData, length);
		unlock();
	}

	void VulkanHardwareBuffer::writeData(UINT32 offset, UINT32 length, const void* source, BufferWriteType writeFlags, 
		UINT32 queueIdx)
	{
		GpuLockOptions lockOptions = GBL_WRITE_ONLY;

		if (writeFlags == BTW_NO_OVERWRITE)
			lockOptions = GBL_WRITE_ONLY_NO_OVERWRITE;
		else if (writeFlags == BWT_DISCARD)
			lockOptions = GBL_WRITE_ONLY_DISCARD;

		// Write to every device
		for (UINT32 i = 0; i < BS_MAX_DEVICES; i++)
		{
			if (mBuffers[i] == nullptr)
				continue;

			void* lockedData = lock(offset, length, lockOptions, i, queueIdx);
			memcpy(lockedData, source, length);
			unlock();
		}
	}
}