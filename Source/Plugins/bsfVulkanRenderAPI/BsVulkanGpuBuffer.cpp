//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsVulkanGpuBuffer.h"
#include "BsVulkanRenderAPI.h"
#include "BsVulkanGpuDevice.h"
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
	VulkanGpuDevice& device = mOwner->GetDevice();

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
	VulkanGpuDevice& device = mOwner->GetDevice();

	if(isInvalidateRequired)
		device.InvalidateMemory(mAllocation, offset, size);

	void* data = device.MapMemory(mAllocation);

	mMappedOffset = offset;
	mMappedSize = size;

	return (u8*)data + offset;
}

void VulkanBuffer::Unmap(bool isFlushRequired)
{
	VulkanGpuDevice& device = mOwner->GetDevice();

	device.UnmapMemory(mAllocation);

	if(isFlushRequired)
		device.FlushMemory(mAllocation, mMappedOffset, mMappedSize);
}

void VulkanBuffer::Update(VulkanInternalCommandBuffer* cb, u8* data, VkDeviceSize offset, VkDeviceSize length)
{
	vkCmdUpdateBuffer(cb->GetHandle(), mBuffer, offset, length, (uint32_t*)data);
}

VkBufferView VulkanBuffer::GetOrCreateView(VkFormat format)
{
	Lock lock(mViewsMutex);

	const auto found = std::find_if(mViews.begin(), mViews.end(), [format](const ViewInformation& x) { return x.Format == format; });

	if(found != mViews.end())
		return found->View;

	VkBufferViewCreateInfo bufferViewCreateInfo;
	bufferViewCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_VIEW_CREATE_INFO;
	bufferViewCreateInfo.pNext = nullptr;
	bufferViewCreateInfo.flags = 0;
	bufferViewCreateInfo.offset = 0;
	bufferViewCreateInfo.range = VK_WHOLE_SIZE;
	bufferViewCreateInfo.format = format;
	bufferViewCreateInfo.buffer = mBuffer;

	VkBufferView view;
	VkResult result = vkCreateBufferView(GetDevice().GetLogical(), &bufferViewCreateInfo, gVulkanAllocator, &view);
	B3D_ASSERT(result == VK_SUCCESS);

	mViews.Add(ViewInformation(format, view));
	return view;
}

VulkanGpuBuffer::VulkanGpuBuffer(VulkanGpuDevice& device, const GpuBufferCreateInformation& createInformation)
	: GpuBuffer(createInformation), mDevice(device), mDirectlyMappable((createInformation.Flags.IsSetAny(GpuBufferFlag::StoreOnCPUWithGPUAccess | GpuBufferFlag::StoreOnCPU)) != 0), mSupportsGPUWrites(createInformation.Flags.IsSet(GpuBufferFlag::AllowWritesOnTheGPU)), mIsMapped(false)
{ }

VulkanGpuBuffer::~VulkanGpuBuffer()
{
	if(mBuffer != nullptr)
		mBuffer->Destroy();

	B3D_ASSERT(mStagingBuffer == nullptr);
}

void VulkanGpuBuffer::Initialize()
{
	VkBufferUsageFlags usageFlags = 0;
	switch(mInformation.Type)
	{
	case GpuBufferType::Vertex:
		usageFlags = VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
	if(mInformation.Flags.IsSet(GpuBufferFlag::AllowWritesOnTheGPU))
		usageFlags |= VK_BUFFER_USAGE_STORAGE_TEXEL_BUFFER_BIT;
		break;
	case GpuBufferType::Index:
		usageFlags = VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
		break;
	case GpuBufferType::Uniform:
		usageFlags = VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
		break;
	case GpuBufferType::SimpleStorage:
		usageFlags = VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_UNIFORM_TEXEL_BUFFER_BIT;
		break;
	case GpuBufferType::StructuredStorage:
		usageFlags = VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
		break;
	}

	if(mInformation.Flags.IsSet(GpuBufferFlag::AllowWritesOnTheGPU))
	{
		if(mInformation.Type == GpuBufferType::SimpleStorage)
			usageFlags |= VK_BUFFER_USAGE_STORAGE_TEXEL_BUFFER_BIT;
		else
			usageFlags |= VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
	}

	mBufferCI.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	mBufferCI.pNext = nullptr;
	mBufferCI.flags = 0;
	mBufferCI.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	mBufferCI.usage = usageFlags;
	mBufferCI.queueFamilyIndexCount = 0;
	mBufferCI.pQueueFamilyIndices = nullptr;

	mBuffer = CreateBuffer(mDevice, mSize, false, true);
}

VulkanBuffer* VulkanGpuBuffer::CreateBuffer(VulkanGpuDevice& device, u32 size, bool staging, bool readable)
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
	if(staging || mInformation.Flags.IsSet(GpuBufferFlag::StoreOnCPU))
		memoryUsage = VMA_MEMORY_USAGE_CPU_ONLY;
	else if(mInformation.Flags.IsSet(GpuBufferFlag::StoreOnCPUWithGPUAccess))
		memoryUsage = VMA_MEMORY_USAGE_CPU_TO_GPU;
	else // StoreOnGPU
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

void VulkanGpuBuffer::SetName(const StringView& name)
{
	GpuBuffer::SetName(name);

	if(vkSetDebugUtilsObjectNameEXT == nullptr)
		return;

	if(mBuffer != nullptr)
		mBuffer->SetName(name);
}

void* VulkanGpuBuffer::Map(u32 offset, u32 length, GpuLockOptions options, u32 deviceIdx, u32 queueIdx)
{
	B3D_ASSERT(deviceIdx == 0);

	if((offset + length) > mSize)
	{
		B3D_LOG(Error, RenderBackend, "Provided offset({0}) + length({1}) is larger than the buffer {2}.", offset, length, mSize);

		return nullptr;
	}

	if(length == 0)
		return nullptr;

	VulkanBuffer* buffer = mBuffer;

	if(buffer == nullptr)
		return nullptr;

	mIsMapped = true;
	mMappedGlobalQueueIdx = queueIdx;
	mMappedOffset = offset;
	mMappedSize = length;
	mMappedLockOptions = options;

	VulkanGpuDevice& device = *GetVulkanGpuBackend().GetVulkanDevice(deviceIdx);

	VulkanCommandBufferManager& cbManager = GetVulkanCommandBufferManager();
	GpuQueueType queueType;
	u32 localQueueIdx = CommandSyncMask::GetQueueIdxAndType(queueIdx, queueType);

	const bool canDiscardBuffer =
		(options == GBL_WRITE_ONLY_DISCARD) ||
		(options == GBL_WRITE_ONLY_DISCARD_RANGE && offset == 0 && length == mSize);

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
				if(!canDiscardBuffer)
				{
					u8* src = buffer->Map(0, mSize, mSupportsGPUWrites);
					u8* dst = newBuffer->Map(0, mSize);

					memcpy(dst, src, length);

					buffer->Unmap();
					newBuffer->Unmap(true);
				}

				buffer->Destroy();
				buffer = newBuffer;

				mBuffer = buffer;
			}

			return buffer->Map(offset, length, isReadRequired);
		}

		// Caller doesn't care about buffer contents, so just discard the existing buffer and create a new one
		if(canDiscardBuffer)
		{
			buffer->Destroy();

			buffer = CreateBuffer(device, mSize, false, true);
			mBuffer = buffer;

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
				u8* src = buffer->Map(0, mSize, mSupportsGPUWrites);
				u8* dst = newBuffer->Map(0, mSize);

				memcpy(dst, src, length);

				buffer->Unmap();
				newBuffer->Unmap(true);

				buffer->Destroy();
				buffer = newBuffer;
				mBuffer = buffer;
			}

			return buffer->Map(offset, length, mSupportsGPUWrites);
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

void VulkanGpuBuffer::Unmap()
{
	// Possibly map() failed with some error
	if(!mIsMapped)
		return;

	// Note: If we did any writes they need to be made visible to the GPU. However there is no need to execute
	// a pipeline barrier because (as per spec) host writes are implicitly visible to the device.

	if(mStagingMemory == nullptr && mStagingBuffer == nullptr) // We directly mapped the buffer
	{
		mBuffer->Unmap(true);
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
			VulkanGpuDevice& device = *GetVulkanGpuBackend().GetVulkanDevice(0);

			VulkanCommandBufferManager& cbManager = GetVulkanCommandBufferManager();
			GpuQueueType queueType;
			u32 localQueueIdx = CommandSyncMask::GetQueueIdxAndType(mMappedGlobalQueueIdx, queueType);

			VulkanBuffer* buffer = mBuffer;
			VulkanTransferBuffer* transferCB = cbManager.GetTransferBuffer(0, queueType, localQueueIdx);

			const bool canDiscardBuffer =
				(mMappedLockOptions == GBL_WRITE_ONLY_DISCARD) ||
				(mMappedLockOptions == GBL_WRITE_ONLY_DISCARD_RANGE && mMappedOffset == 0 && mMappedSize == mSize);

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
				else if(canDiscardBuffer)
				{
					buffer->Destroy();

					buffer = CreateBuffer(device, mSize, false, true);
					mBuffer = buffer;
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
					mBuffer = buffer;
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

void VulkanGpuBuffer::CopyData(GpuBuffer& srcBuffer, u32 srcOffset, u32 dstOffset, u32 length, bool discardWholeBuffer, const SPtr<CommandBuffer>& commandBuffer)
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

	VulkanGpuBuffer& vkSource = static_cast<VulkanGpuBuffer&>(srcBuffer);

	VulkanRenderAPI& rapi = static_cast<VulkanRenderAPI&>(RenderAPI::Instance());
	VulkanInternalCommandBuffer* vkCB;
	if(commandBuffer != nullptr)
		vkCB = static_cast<VulkanCommandBuffer*>(commandBuffer.get())->GetInternal();
	else
		vkCB = rapi.GetMainVulkanCommandBuffer()->GetInternal();

	u32 deviceIdx = vkCB->GetDeviceIndex();
	B3D_ASSERT(deviceIdx == 0);

	VulkanBuffer* src = vkSource.mBuffer;
	VulkanBuffer* dst = mBuffer;

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

void VulkanGpuBuffer::ReadData(u32 offset, u32 length, void* dest, u32 deviceIdx, u32 queueIdx)
{
	void* lockedData = Lock(offset, length, GBL_READ_ONLY, deviceIdx, queueIdx);
	memcpy(dest, lockedData, length);
	Unlock();
}

void VulkanGpuBuffer::WriteData(u32 offset, u32 length, const void* source, BufferWriteType writeFlags, u32 queueIdx)
{
	GpuLockOptions lockOptions = GBL_WRITE_ONLY_DISCARD_RANGE;

	if(writeFlags == BTW_NO_OVERWRITE)
		lockOptions = GBL_WRITE_ONLY_NO_OVERWRITE;
	else if(writeFlags == BWT_DISCARD)
		lockOptions = GBL_WRITE_ONLY_DISCARD;

	if(mBuffer != nullptr)
	{
		void* lockedData = Lock(offset, length, lockOptions, 0, queueIdx);
		memcpy(lockedData, source, length);
		Unlock();
	}
}

VkBufferView VulkanGpuBuffer::GetOrCreateView(GpuBufferFormat format) const
{
	if(mInformation.Type != GpuBufferType::SimpleStorage || mBuffer == nullptr)
		return nullptr;

	if(format == BF_UNKNOWN)
		format = mInformation.SimpleStorage.Format;

	return mBuffer->GetOrCreateView(VulkanUtility::GetBufferFormat(mInformation.SimpleStorage.Format));
}
