//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsVulkanHardwareBuffer.h"
#include "BsVulkanRenderAPI.h"
#include "BsVulkanDevice.h"
#include "BsVulkanUtility.h"
#include "Managers/BsVulkanCommandBufferManager.h"
#include "BsVulkanCommandBuffer.h"
#include "BsVulkanTexture.h"

using namespace bs;
using namespace bs::ct;

VulkanBuffer::VulkanBuffer(VulkanResourceManager* owner, VkBuffer buffer, VmaAllocation allocation, u32 rowPitch, u32 slicePitch)
	: VulkanResource(owner, false), mBuffer(buffer), mAllocation(allocation), mRowPitch(rowPitch)
{
	if(rowPitch != 0)
		mSliceHeight = slicePitch / rowPitch;
	else
		mSliceHeight = 0;
}

VulkanBuffer::~VulkanBuffer()
{
	VulkanDevice& device = mOwner->GetDevice();

	{
		Lock lock(mMutex);
		for(auto& entry : mViews)
			vkDestroyBufferView(device.GetLogical(), entry.View, gVulkanAllocator);
	}

	vkDestroyBuffer(device.GetLogical(), mBuffer, gVulkanAllocator);
	device.FreeMemory(mAllocation);
}

void VulkanBuffer::SetName(const StringView& name)
{
	if(vkSetDebugUtilsObjectNameEXT == nullptr)
		return;

	VkDebugUtilsObjectNameInfoEXT objectNameInfo;
	objectNameInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT;
	objectNameInfo.pNext = nullptr;
	objectNameInfo.objectType = VK_OBJECT_TYPE_BUFFER;
	objectNameInfo.objectHandle = (uint64_t)mBuffer;
	objectNameInfo.pObjectName = name.data();

	vkSetDebugUtilsObjectNameEXT(mOwner->GetDevice().GetLogical(), &objectNameInfo);
}

u8* VulkanBuffer::Map(VkDeviceSize offset, VkDeviceSize size, bool isInvalidateRequired) const
{
	VulkanDevice& device = mOwner->GetDevice();

	if(isInvalidateRequired)
		device.InvalidateMemory(mAllocation, offset, size);

	void* data = device.MapMemory(mAllocation);

	mMappedOffset = offset;
	mMappedSize = size;

	return (u8*)data + offset;
}

void VulkanBuffer::Unmap(bool isFlushRequired)
{
	VulkanDevice& device = mOwner->GetDevice();

	device.UnmapMemory(mAllocation);

	if(isFlushRequired)
		device.FlushMemory(mAllocation, mMappedOffset, mMappedSize);
}

void VulkanBuffer::Update(VulkanInternalCommandBuffer* cb, u8* data, VkDeviceSize offset, VkDeviceSize length)
{
	vkCmdUpdateBuffer(cb->GetHandle(), mBuffer, offset, length, (uint32_t*)data);
}

void VulkanBuffer::NotifyDone(u32 globalQueueIdx, VulkanAccessFlags useFlags)
{
	{
		Lock lock(mMutex);

		// Note: With often used buffers this block might never execute, in which case views won't  get freed.
		// If that ever becomes an issue (unlikely) then we'll need to track usage per-view.
		bool isLastHandle = mNumBoundHandles == 1;
		if(isLastHandle)
			DestroyUnusedViews();
	}

	VulkanResource::NotifyDone(globalQueueIdx, useFlags);
}

void VulkanBuffer::NotifyUnbound()
{
	{
		Lock lock(mMutex);

		// Note: With often used buffers this block might never execute, in which case views won't  get freed.
		// If that ever becomes an issue (unlikely) then we'll need to track usage per-view.
		bool isLastHandle = mNumBoundHandles == 1;
		if(isLastHandle)
			DestroyUnusedViews();
	}

	VulkanResource::NotifyUnbound();
}

VkBufferView VulkanBuffer::GetView(VkFormat format)
{
	Lock lock(mViewsMutex);

	const auto iterFind = std::find_if(mViews.begin(), mViews.end(), [format](const ViewInfo& x)
									   { return x.Format == format; });

	if(iterFind != mViews.end())
	{
		iterFind->UseCount++;
		return iterFind->View;
	}

	VkBufferViewCreateInfo viewCI;
	viewCI.sType = VK_STRUCTURE_TYPE_BUFFER_VIEW_CREATE_INFO;
	viewCI.pNext = nullptr;
	viewCI.flags = 0;
	viewCI.offset = 0;
	viewCI.range = VK_WHOLE_SIZE;
	viewCI.format = format;
	viewCI.buffer = mBuffer;

	VkBufferView view;
	VkResult result = vkCreateBufferView(GetDevice().GetLogical(), &viewCI, gVulkanAllocator, &view);
	B3D_ASSERT(result == VK_SUCCESS);

	mViews.push_back(ViewInfo(format, view));
	return view;
}

void VulkanBuffer::FreeView(VkBufferView view)
{
	Lock lock(mViewsMutex);

	const auto iterFind = std::find_if(mViews.begin(), mViews.end(), [view](const ViewInfo& x)
									   { return x.View == view; });

	if(iterFind != mViews.end())
	{
		B3D_ASSERT(iterFind->UseCount > 0);
		iterFind->UseCount--;
	}
	else
	{
		B3D_ASSERT(false);
	}
}

void VulkanBuffer::DestroyUnusedViews()
{
	Lock lock(mViewsMutex);

	for(auto iter = mViews.begin(); iter != mViews.end();)
	{
		if(iter->UseCount == 0)
		{
			vkDestroyBufferView(GetDevice().GetLogical(), iter->View, gVulkanAllocator);
			iter = mViews.erase(iter);
		}
		else
			++iter;
	}
}

VulkanHardwareBuffer::VulkanHardwareBuffer(BufferType type, GpuBufferFormat format, GpuBufferUsage usage, u32 size, GpuDeviceFlags deviceMask)
	: HardwareBuffer(size, usage, deviceMask), mBuffers(), mStagingBuffer(nullptr), mStagingMemory(nullptr), mMappedDeviceIdx(-1), mMappedGlobalQueueIdx(-1), mMappedOffset(0), mMappedSize(0), mMappedLockOptions(GBL_WRITE_ONLY), mDirectlyMappable((usage & GBU_DYNAMIC) != 0), mSupportsGPUWrites(type == BT_STRUCTURED || ((usage & GBU_LOADSTORE) == GBU_LOADSTORE)), mIsMapped(false)
{
	VkBufferUsageFlags usageFlags = 0;
	switch(type)
	{
	case BT_VERTEX:
		usageFlags = VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;

		if((usage & GBU_LOADSTORE) == GBU_LOADSTORE)
			usageFlags |= VK_BUFFER_USAGE_UNIFORM_TEXEL_BUFFER_BIT | VK_BUFFER_USAGE_STORAGE_TEXEL_BUFFER_BIT;
		break;
	case BT_INDEX:
		usageFlags = VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT;

		if((usage & GBU_LOADSTORE) == GBU_LOADSTORE)
			usageFlags |= VK_BUFFER_USAGE_UNIFORM_TEXEL_BUFFER_BIT | VK_BUFFER_USAGE_STORAGE_TEXEL_BUFFER_BIT;
		break;
	case BT_UNIFORM:
		usageFlags = VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
		break;
	case BT_GENERIC:
		usageFlags = VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_UNIFORM_TEXEL_BUFFER_BIT;

		if((usage & GBU_LOADSTORE) == GBU_LOADSTORE)
			usageFlags |= VK_BUFFER_USAGE_STORAGE_TEXEL_BUFFER_BIT;

		break;
	case BT_STRUCTURED:
		usageFlags = VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
		break;
	}

	mBufferCI.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	mBufferCI.pNext = nullptr;
	mBufferCI.flags = 0;
	mBufferCI.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	mBufferCI.usage = usageFlags;
	mBufferCI.queueFamilyIndexCount = 0;
	mBufferCI.pQueueFamilyIndices = nullptr;

	VulkanRenderAPI& rapi = static_cast<VulkanRenderAPI&>(RenderAPI::Instance());
	VulkanDevice* devices[B3D_MAX_DEVICES];
	VulkanUtility::GetDevices(rapi, deviceMask, devices);

	// Allocate buffers per-device
	for(u32 i = 0; i < B3D_MAX_DEVICES; i++)
	{
		if(devices[i] == nullptr)
			continue;

		mBuffers[i] = CreateBuffer(*devices[i], size, false, true);
	}
}

VulkanHardwareBuffer::~VulkanHardwareBuffer()
{
	for(u32 i = 0; i < B3D_MAX_DEVICES; i++)
	{
		if(mBuffers[i] == nullptr)
			continue;

		mBuffers[i]->Destroy();
	}

	B3D_ASSERT(mStagingBuffer == nullptr);
}

VulkanBuffer* VulkanHardwareBuffer::CreateBuffer(VulkanDevice& device, u32 size, bool staging, bool readable)
{
	// Not allowed to have size 0 buffer
	if(size == 0)
		size = 64;

	VkBufferUsageFlags usage = mBufferCI.usage;
	if(staging)
	{
		mBufferCI.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;

		// Staging buffers are used as a destination for reads
		if(readable)
			mBufferCI.usage |= VK_BUFFER_USAGE_TRANSFER_DST_BIT;
	}
	else if(readable) // Non-staging readable
		mBufferCI.usage |= VK_BUFFER_USAGE_TRANSFER_SRC_BIT;

	mBufferCI.size = size;

	VmaMemoryUsage memoryUsage;
	if(staging)
		memoryUsage = VMA_MEMORY_USAGE_CPU_ONLY;
	else if(mDirectlyMappable)
		memoryUsage = VMA_MEMORY_USAGE_CPU_TO_GPU;
	else
		memoryUsage = VMA_MEMORY_USAGE_GPU_ONLY;

	VkDevice vkDevice = device.GetLogical();

	VkBuffer buffer;
	VkResult result = vkCreateBuffer(vkDevice, &mBufferCI, gVulkanAllocator, &buffer);
	B3D_ASSERT(result == VK_SUCCESS);

	VmaAllocation allocation = device.AllocateMemory(buffer, memoryUsage);

	mBufferCI.usage = usage; // Restore original usage
	VulkanBuffer *const vulkanBuffer = device.GetResourceManager().Create<VulkanBuffer>(buffer, allocation);

	if(vulkanBuffer != nullptr)
	{
		if(staging)
			vulkanBuffer->SetName(StringUtil::Format("Staging buffer ({0})", mName));
		else
			vulkanBuffer->SetName(mName);
	}

	return vulkanBuffer;
}

void VulkanHardwareBuffer::SetName(const StringView& name)
{
	HardwareBuffer::SetName(name);

	if(vkSetDebugUtilsObjectNameEXT == nullptr)
		return;

	for(UINT32 deviceIndex = 0; deviceIndex < B3D_MAX_DEVICES; deviceIndex++)
	{
		if(mBuffers[deviceIndex] == nullptr)
			continue;

		mBuffers[deviceIndex]->SetName(name);
	}
}

void* VulkanHardwareBuffer::Map(u32 offset, u32 length, GpuLockOptions options, u32 deviceIdx, u32 queueIdx)
{
	if((offset + length) > mSize)
	{
		B3D_LOG(Error, RenderBackend, "Provided offset({0}) + length({1}) is larger than the buffer {2}.", offset, length, mSize);

		return nullptr;
	}

	if(length == 0)
		return nullptr;

	VulkanBuffer* buffer = mBuffers[deviceIdx];

	if(buffer == nullptr)
		return nullptr;

	mIsMapped = true;
	mMappedDeviceIdx = deviceIdx;
	mMappedGlobalQueueIdx = queueIdx;
	mMappedOffset = offset;
	mMappedSize = length;
	mMappedLockOptions = options;

	VulkanRenderAPI& rapi = static_cast<VulkanRenderAPI&>(RenderAPI::Instance());
	VulkanDevice& device = *rapi.GetDevice(deviceIdx);

	VulkanCommandBufferManager& cbManager = GetVulkanCommandBufferManager();
	GpuQueueType queueType;
	u32 localQueueIdx = CommandSyncMask::GetQueueIdxAndType(queueIdx, queueType);

	VkAccessFlags accessFlags;
	if(options == GBL_READ_ONLY)
		accessFlags = VK_ACCESS_HOST_READ_BIT;
	else if(options == GBL_READ_WRITE)
		accessFlags = VK_ACCESS_HOST_READ_BIT | VK_ACCESS_HOST_WRITE_BIT;
	else
		accessFlags = VK_ACCESS_HOST_WRITE_BIT;

	// If memory is host visible try mapping it directly
	if(mDirectlyMappable)
	{
		// Caller guarantees he won't touch the same data as the GPU, so just map even if the GPU is using the buffer
		if(options == GBL_WRITE_ONLY_NO_OVERWRITE)
			return buffer->Map(offset, length);

		// Check is the GPU currently reading or writing from the buffer
		u32 useMask = buffer->GetUseInfo(VulkanAccessFlag::Read | VulkanAccessFlag::Write);

		// Note: Even if GPU isn't currently using the buffer, but the buffer supports GPU writes, we consider it as
		// being used because the write could have completed yet still not visible, so we need to issue a pipeline
		// barrier below.
		const bool isUsedOnGPU = useMask != 0 || mSupportsGPUWrites;
		const bool isReadRequired = options == GBL_READ_ONLY || options == GBL_READ_WRITE;

		// We're safe to map directly since GPU isn't using the buffer
		if(!isUsedOnGPU)
		{
			// If some CB has an operation queued that will be using the current contents of the buffer, create a new
			// buffer so we don't modify the previous use of the buffer
			if(buffer->IsBound())
			{
				VulkanBuffer* newBuffer = CreateBuffer(device, mSize, false, true);

				// Copy contents of the current buffer to the new one, unless caller explicitly specifies he doesn't
				// care about the current contents
				if(options != GBL_WRITE_ONLY_DISCARD)
				{
					u8* src = buffer->Map(0, mSize, true);
					u8* dst = newBuffer->Map(0, mSize);

					memcpy(dst, src, length);

					buffer->Unmap();
					newBuffer->Unmap(true);
				}

				buffer->Destroy();
				buffer = newBuffer;
				mBuffers[deviceIdx] = buffer;
			}

			return buffer->Map(offset, length, isReadRequired);
		}

		// Caller doesn't care about buffer contents, so just discard the existing buffer and create a new one
		if(options == GBL_WRITE_ONLY_DISCARD)
		{
			buffer->Destroy();

			buffer = CreateBuffer(device, mSize, false, true);
			mBuffers[deviceIdx] = buffer;

			return buffer->Map(offset, length);
		}

		// We need to read the buffer contents
		if(isReadRequired)
		{
			// We need to wait until (potential) read/write operations complete
			VulkanTransferBuffer* transferCB = cbManager.GetTransferBuffer(deviceIdx, queueType, localQueueIdx);

			// Ensure flush() will wait for all queues currently using to the buffer (if any) to finish
			// If only reading, wait for all writes to complete, otherwise wait on both writes and reads
			if(options == GBL_READ_ONLY)
				useMask = buffer->GetUseInfo(VulkanAccessFlag::Write);
			else
				useMask = buffer->GetUseInfo(VulkanAccessFlag::Read | VulkanAccessFlag::Write);

			transferCB->AppendMask(useMask);

			// Make any writes visible before mapping
			if(mSupportsGPUWrites)
			{
				// Issue a barrier so :
				//  - If reading: the device makes the written memory available for read (read-after-write hazard)
				//  - If writing: ensures our writes properly overlap with GPU writes (write-after-write hazard)
				transferCB->GetInternalCommandBuffer()->MemoryBarrier(buffer->GetHandle(), VK_ACCESS_SHADER_WRITE_BIT, accessFlags,
										  // Last stages that could have written to the buffer:
										  VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT | VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_PIPELINE_STAGE_HOST_BIT);
			}

			// Submit the command buffer and wait until it finishes
			transferCB->Flush(true);

			// If writing and some CB has an operation queued that will be using the current contents of the buffer,
			// create a new  buffer so we don't modify the previous use of the buffer
			if(options == GBL_READ_WRITE && buffer->IsBound())
			{
				VulkanBuffer* newBuffer = CreateBuffer(device, mSize, false, true);

				// Copy contents of the current buffer to the new one
				u8* src = buffer->Map(0, mSize, true);
				u8* dst = newBuffer->Map(0, mSize);

				memcpy(dst, src, length);

				buffer->Unmap();
				newBuffer->Unmap(true);

				buffer->Destroy();
				buffer = newBuffer;
				mBuffers[deviceIdx] = buffer;
			}

			return buffer->Map(offset, length, true);
		}

		// Otherwise, we're doing write only, in which case it's best to use the staging buffer to avoid waiting
		// and blocking, so fall through
	}

	// Can't use direct mapping, so use a staging buffer or memory

	// We might need to copy the current contents of the buffer to the staging buffer. Even if the user doesn't plan on
	// reading, it is still required as we will eventually copy all of the contents back to the original buffer,
	// and we can't write potentially uninitialized data. The only exception is when the caller specifies the buffer
	// contents should be discarded in which he guarantees he will overwrite the entire locked area with his own
	// contents.
	bool needRead = options != GBL_WRITE_ONLY_DISCARD_RANGE && options != GBL_WRITE_ONLY_DISCARD;

	// See if we can use the cheaper staging memory, rather than a staging buffer
	if(!needRead && offset % 4 == 0 && length % 4 == 0 && length <= 65536)
	{
		mStagingMemory = (u8*)B3DAllocate(length);
		return mStagingMemory;
	}

	// Create a staging buffer
	mStagingBuffer = CreateBuffer(device, length, true, needRead);

	if(needRead)
	{
		VulkanTransferBuffer* transferCB = cbManager.GetTransferBuffer(deviceIdx, queueType, localQueueIdx);

		// Similar to above, if buffer supports GPU writes or is currently being written to, we need to wait on any
		// potential writes to complete
		u32 writeUseMask = buffer->GetUseInfo(VulkanAccessFlag::Write);
		if(mSupportsGPUWrites || writeUseMask != 0)
		{
			// Ensure flush() will wait for all queues currently writing to the buffer (if any) to finish
			transferCB->AppendMask(writeUseMask);
		}

		// Queue copy command
		transferCB->GetInternalCommandBuffer()->CopyBufferToBuffer(buffer, mStagingBuffer, offset, 0, length);

		// Ensure data written to the staging buffer is visible
		transferCB->GetInternalCommandBuffer()->MemoryBarrier(mStagingBuffer->GetHandle(), VK_ACCESS_TRANSFER_WRITE_BIT, accessFlags, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_HOST_BIT);

		// Submit the command buffer and wait until it finishes
		transferCB->Flush(true);
		B3D_ASSERT(!buffer->IsUsed());
	}

	return mStagingBuffer->Map(0, length);
}

void VulkanHardwareBuffer::Unmap()
{
	// Possibly map() failed with some error
	if(!mIsMapped)
		return;

	// Note: If we did any writes they need to be made visible to the GPU. However there is no need to execute
	// a pipeline barrier because (as per spec) host writes are implicitly visible to the device.

	if(mStagingMemory == nullptr && mStagingBuffer == nullptr) // We directly mapped the buffer
	{
		mBuffers[mMappedDeviceIdx]->Unmap(true);
	}
	else
	{
		if(mStagingBuffer != nullptr)
			mStagingBuffer->Unmap();

		bool isWrite = mMappedLockOptions != GBL_READ_ONLY;

		// We the caller wrote anything to the staging buffer, we need to upload it back to the main buffer
		if(isWrite)
		{
			VulkanRenderAPI& rapi = static_cast<VulkanRenderAPI&>(RenderAPI::Instance());
			VulkanDevice& device = *rapi.GetDevice(mMappedDeviceIdx);

			VulkanCommandBufferManager& cbManager = GetVulkanCommandBufferManager();
			GpuQueueType queueType;
			u32 localQueueIdx = CommandSyncMask::GetQueueIdxAndType(mMappedGlobalQueueIdx, queueType);

			VulkanBuffer* buffer = mBuffers[mMappedDeviceIdx];
			VulkanTransferBuffer* transferCB = cbManager.GetTransferBuffer(mMappedDeviceIdx, queueType, localQueueIdx);

			// If the buffer is used in any way on the GPU, we need to wait for that use to finish before
			// we issue our copy
			u32 useMask = buffer->GetUseInfo(VulkanAccessFlag::Read | VulkanAccessFlag::Write);
			bool isNormalWrite = false;
			if(useMask != 0) // Buffer is currently used on the GPU
			{
				// Try to avoid the wait by checking for special write conditions

				// Caller guarantees he won't touch the same data as the GPU, so just copy
				if(mMappedLockOptions == GBL_WRITE_ONLY_NO_OVERWRITE)
				{
					// Fall through to copy()
				}
				// Caller doesn't care about buffer contents, so just discard the existing buffer and create a new one
				else if(mMappedLockOptions == GBL_WRITE_ONLY_DISCARD)
				{
					buffer->Destroy();

					buffer = CreateBuffer(device, mSize, false, true);
					mBuffers[mMappedDeviceIdx] = buffer;
				}
				else // Otherwise we have no choice but to issue a dependency between the queues
				{
					transferCB->AppendMask(useMask);
					isNormalWrite = true;
				}
			}
			else
				isNormalWrite = true;

			// Check if the buffer will still be bound somewhere after the CBs using it finish
			if(isNormalWrite)
			{
				u32 useCount = buffer->GetUseCount();
				u32 boundCount = buffer->GetBoundCount();

				bool isBoundWithoutUse = boundCount > useCount;

				// If buffer is queued for some operation on a CB, then we need to make a copy of the buffer to
				// avoid modifying its use in the previous operation
				if(isBoundWithoutUse)
				{
					VulkanBuffer* newBuffer = CreateBuffer(device, mSize, false, true);

					// Avoid copying original contents if the staging buffer completely covers it
					if(mMappedOffset > 0 || mMappedSize != mSize)
					{
						transferCB->GetInternalCommandBuffer()->CopyBufferToBuffer(buffer, newBuffer, 0, 0, mSize);

						// TODO - Move this within VulkanCmdBuffer::CopyBufferToBuffer?
						transferCB->GetInternalCommandBuffer()->RegisterBuffer(buffer, BufferUseFlagBits::Transfer, VulkanAccessFlag::Read);
					}

					buffer->Destroy();
					buffer = newBuffer;
					mBuffers[mMappedDeviceIdx] = buffer;
				}
			}

			// Queue copy/update command
			if(mStagingBuffer != nullptr)
			{
				transferCB->GetInternalCommandBuffer()->CopyBufferToBuffer(mStagingBuffer, buffer, 0, mMappedOffset, mMappedSize);

				// TODO - Move this within VulkanCmdBuffer::CopyBufferToBuffer?
				transferCB->GetInternalCommandBuffer()->RegisterBuffer(mStagingBuffer, BufferUseFlagBits::Transfer, VulkanAccessFlag::Read);
			}
			else // Staging memory
			{
				buffer->Update(transferCB->GetInternalCommandBuffer(), mStagingMemory, mMappedOffset, mMappedSize);
			}

			// TODO - Move this within VulkanCmdBuffer::CopyBufferToBuffer?
			transferCB->GetInternalCommandBuffer()->RegisterBuffer(buffer, BufferUseFlagBits::Transfer, VulkanAccessFlag::Write);

			// We don't actually flush the transfer buffer here since it's an expensive operation, but it's instead
			// done automatically before next "normal" command buffer submission.
		}

		if(mStagingBuffer != nullptr)
		{
			mStagingBuffer->Destroy();
			mStagingBuffer = nullptr;
		}

		if(mStagingMemory != nullptr)
		{
			B3DFree(mStagingMemory);
			mStagingMemory = nullptr;
		}
	}

	mIsMapped = false;
}

void VulkanHardwareBuffer::CopyData(HardwareBuffer& srcBuffer, u32 srcOffset, u32 dstOffset, u32 length, bool discardWholeBuffer, const SPtr<CommandBuffer>& commandBuffer)
{
	if((dstOffset + length) > mSize)
	{
		B3D_LOG(Error, RenderBackend, "Provided offset({0}) + length({1}) is larger than the destination buffer {2}. "
									 "Copy operation aborted.",
			   dstOffset, length, mSize);

		return;
	}

	if((srcOffset + length) > srcBuffer.GetSize())
	{
		B3D_LOG(Error, RenderBackend, "Provided offset({0}) + length({1}) is larger than the source buffer {2}. "
									 "Copy operation aborted.",
			   srcOffset, length, srcBuffer.GetSize());

		return;
	}

	VulkanHardwareBuffer& vkSource = static_cast<VulkanHardwareBuffer&>(srcBuffer);

	VulkanRenderAPI& rapi = static_cast<VulkanRenderAPI&>(RenderAPI::Instance());
	VulkanInternalCommandBuffer* vkCB;
	if(commandBuffer != nullptr)
		vkCB = static_cast<VulkanCommandBuffer*>(commandBuffer.get())->GetInternal();
	else
		vkCB = rapi.GetMainVulkanCommandBuffer()->GetInternal();

	u32 deviceIdx = vkCB->GetDeviceIdx();

	VulkanBuffer* src = vkSource.mBuffers[deviceIdx];
	VulkanBuffer* dst = mBuffers[deviceIdx];

	if(src == nullptr || dst == nullptr)
		return;

	if(vkCB->IsInRenderPass())
		vkCB->EndRenderPass(true);

	vkCB->CopyBufferToBuffer(src, dst, srcOffset, dstOffset, length);

	// Notify the command buffer that these resources are being used on it
	// TODO - Move this within VulkanCmdBuffer::CopyBufferToBuffer?
	vkCB->RegisterBuffer(src, BufferUseFlagBits::Transfer, VulkanAccessFlag::Read);
	vkCB->RegisterBuffer(dst, BufferUseFlagBits::Transfer, VulkanAccessFlag::Write);
}

void VulkanHardwareBuffer::ReadData(u32 offset, u32 length, void* dest, u32 deviceIdx, u32 queueIdx)
{
	void* lockedData = Lock(offset, length, GBL_READ_ONLY, deviceIdx, queueIdx);
	memcpy(dest, lockedData, length);
	Unlock();
}

void VulkanHardwareBuffer::WriteData(u32 offset, u32 length, const void* source, BufferWriteType writeFlags, u32 queueIdx)
{
	GpuLockOptions lockOptions = GBL_WRITE_ONLY_DISCARD_RANGE;

	if(writeFlags == BTW_NO_OVERWRITE)
		lockOptions = GBL_WRITE_ONLY_NO_OVERWRITE;
	else if(writeFlags == BWT_DISCARD)
		lockOptions = GBL_WRITE_ONLY_DISCARD;

	// Write to every device
	for(u32 i = 0; i < B3D_MAX_DEVICES; i++)
	{
		if(mBuffers[i] == nullptr)
			continue;

		void* lockedData = Lock(offset, length, lockOptions, i, queueIdx);
		memcpy(lockedData, source, length);
		Unlock();
	}
}
