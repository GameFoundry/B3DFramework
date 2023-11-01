//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsVulkanGpuBuffer.h"
#include "BsVulkanGpuDevice.h"
#include "BsVulkanUtility.h"
#include "BsVulkanGpuCommandBuffer.h"
#include "BsVulkanTexture.h"
#include "BsVulkanGpuBackend.h"

using namespace bs;
using namespace bs::ct;

VulkanBuffer::VulkanBuffer(VulkanResourceManager* owner, GpuBufferType type, GpuBufferFlags flags, VkBuffer buffer, VmaAllocation allocation, u32 rowPitch, u32 slicePitch, const StringView& name)
	: VulkanResource(owner, false, name), mType(type), mFlags(flags), mBuffer(buffer), mAllocation(allocation), mRowPitch(rowPitch)
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

VkAccessFlags VulkanBuffer::GetAccessFlags() const
{
	VkAccessFlags accessFlags = 0;
	switch(mType)
	{
	case GpuBufferType::SimpleStorage:
	case GpuBufferType::StructuredStorage:
		accessFlags |= VK_ACCESS_SHADER_READ_BIT;
		break;
	case GpuBufferType::Index:
		accessFlags |= VK_ACCESS_INDEX_READ_BIT;
		break;
	case GpuBufferType::Vertex:
		accessFlags |= VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT;
		break;
	case GpuBufferType::Uniform:
		accessFlags |= VK_ACCESS_UNIFORM_READ_BIT;
		break;
	case GpuBufferType::StagingRead:
		accessFlags |= VK_ACCESS_HOST_READ_BIT;
		break;
	case GpuBufferType::StagingWrite:
		accessFlags |= VK_ACCESS_HOST_WRITE_BIT;
		break;
	}

	if(mFlags.IsSet(GpuBufferFlag::AllowWritesOnTheGPU))
		accessFlags |= VK_ACCESS_SHADER_WRITE_BIT;

	return accessFlags;
}

VulkanGpuBuffer::VulkanGpuBuffer(VulkanGpuDevice& device, const GpuBufferCreateInformation& createInformation)
	: GpuBuffer(createInformation, bs::GpuBuffer::CalculateSuballocatedBufferSize(createInformation, device)), mDevice(device), mDirectlyMappable((createInformation.Flags.IsSetAny(GpuBufferFlag::StoreOnCPUWithGPUAccess)) != 0 || createInformation.Type == GpuBufferType::StagingRead || createInformation.Type == GpuBufferType::StagingWrite), mSupportsGPUWrites(createInformation.Flags.IsSet(GpuBufferFlag::AllowWritesOnTheGPU)), mIsMapped(false)
	{ }

VulkanGpuBuffer::~VulkanGpuBuffer()
{
	if(mBuffer != nullptr)
		mBuffer->Destroy();
}

void VulkanGpuBuffer::Initialize()
{
	VkBufferUsageFlags usageFlags = 0;
	switch(mInformation.Type)
	{
	case GpuBufferType::Vertex:
		usageFlags = VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
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
	case GpuBufferType::StagingRead:
		usageFlags = VK_BUFFER_USAGE_TRANSFER_DST_BIT;
		break;
	case GpuBufferType::StagingWrite:
		usageFlags = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
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

	mBuffer = CreateBuffer(mDevice, mTotalSize, false, true);
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
	if(staging)
	{
		if(readable)
			memoryUsage = VMA_MEMORY_USAGE_GPU_TO_CPU;
		else
			memoryUsage = VMA_MEMORY_USAGE_CPU_ONLY;
	}
	else if(mInformation.Type == GpuBufferType::StagingRead)
		memoryUsage = VMA_MEMORY_USAGE_GPU_TO_CPU;
	else if(mInformation.Type == GpuBufferType::StagingWrite)
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

	const GpuBufferType newBufferType = staging ? readable ? GpuBufferType::StagingRead : GpuBufferType::StagingWrite : mInformation.Type;
	const GpuBufferFlags newBufferFlags = staging ? (GpuBufferFlags)0 : mInformation.Flags;

	VulkanBuffer *const vulkanBuffer = device.GetResourceManager().Create<VulkanBuffer>(newBufferType, newBufferFlags, buffer, allocation);

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

void* VulkanGpuBuffer::Map(u32 offset, u32 length, GpuLockOptions options)
{
	if((offset + length) > mTotalSize)
	{
		B3D_LOG(Error, RenderBackend, "Cannot map GpuBuffer memory for buffer '{0}'. Provided offset:{1} + length:{2} is larger than the buffer size:{3}.", mName, offset, length, mTotalSize);

		return nullptr;
	}

	if(length == 0)
		return nullptr;

	VulkanBuffer* buffer = mBuffer;

	if(buffer == nullptr)
		return nullptr;

	if(!mDirectlyMappable) // TODO - Need to check if this is memory on an integrated GPU, in which case it might be directly mappable always
	{
		B3D_LOG(Error, RenderBackend, "Cannot map GpuBuffer memory for buffer '{0}'. The buffer has not been created with CPU-visible flags.", mName);
		return nullptr;
	}

	mIsMapped = true;

	const bool canDiscardBuffer =
		(options == GBL_WRITE_ONLY_DISCARD) ||
		(options == GBL_WRITE_ONLY_DISCARD_RANGE && offset == 0 && length == mTotalSize);

	// Check is the GPU currently reading or writing from the buffer
	u32 useMask = buffer->GetUseInfo(VulkanAccessFlag::Read | VulkanAccessFlag::Write);

	// Note: Even if GPU isn't currently using the buffer, but the buffer supports GPU writes, we consider it as
	// being used because the write could have completed yet still not visible, so we need to issue a pipeline
	// barrier below.
	const bool isUsedOnGPU = useMask != 0 || mSupportsGPUWrites;

	const bool isReadRequired = options == GBL_READ_ONLY || options == GBL_READ_WRITE;
	const bool isWrite = options != GBL_READ_ONLY;

	// If not used on the GPU, of the caller explicitly states he doesn't care, map the buffer
	if(!isUsedOnGPU || options == GBL_WRITE_ONLY_NO_OVERWRITE)
	{
		// Warn if we have already bound the buffer to a command buffer previously, as that could have unintended consequences since previous commands could be affected
		if(!buffer->IsBound() || !isWrite)
		{
			return buffer->Map(offset, length, isReadRequired);
		}
		else if(canDiscardBuffer)
		{
			// Fall through
		}
		else
		{
			B3D_LOG(Warning, RenderBackend, "Writing to a buffer that is currently bound on a command buffer. Previous usages of the buffer will be affected. Buffer: {0}", mName);

			return buffer->Map(offset, length, isReadRequired);
		}
	}

	// Used on the GPU but caller doesn't care about buffer contents, so just discard the existing buffer and create a new one
	if(canDiscardBuffer)
	{
		buffer->Destroy();

		buffer = CreateBuffer(mDevice, mTotalSize, false, true);
		mBuffer = buffer;

		return buffer->Map(offset, length);
	}

	B3D_LOG(Error, RenderBackend, "Cannot map GpuBuffer memory for buffer '{0}'. The buffer is currently being used by the GPU.", mName);
	return nullptr;
}

void VulkanGpuBuffer::Unmap()
{
	// Possibly Map() failed with some error
	if(!mIsMapped)
		return;

	if(!B3D_ENSURE(mBuffer != nullptr))
		return;

	// Note: If we did any writes they need to be made visible to the GPU. However there is no need to execute
	// a pipeline barrier because (as per spec) host writes are implicitly visible to the device.
	mBuffer->Unmap(true);
	mIsMapped = false;
}

void VulkanGpuBuffer::CopyData(GpuBuffer& srcBuffer, u32 srcOffset, u32 dstOffset, u32 length, bool discardWholeBuffer, const SPtr<GpuCommandBuffer>& commandBuffer)
{
	if((dstOffset + length) > mTotalSize)
	{
		B3D_LOG(Error, RenderBackend, "Provided offset({0}) + length({1}) is larger than the destination buffer {2}. "
									 "Copy operation aborted.",
			   dstOffset, length, mTotalSize);

		return;
	}

	if((srcOffset + length) > srcBuffer.GetTotalSize())
	{
		B3D_LOG(Error, RenderBackend, "Provided offset({0}) + length({1}) is larger than the source buffer {2}. "
									 "Copy operation aborted.",
			   srcOffset, length, srcBuffer.GetTotalSize());

		return;
	}

	auto& vkSource = static_cast<VulkanGpuBuffer&>(srcBuffer);
	auto& vulkanCommandBuffer = static_cast<VulkanGpuCommandBuffer&>(*commandBuffer);

	VulkanBuffer* src = vkSource.mBuffer;
	VulkanBuffer* dst = mBuffer;

	if(src == nullptr || dst == nullptr)
		return;

	if(vulkanCommandBuffer.IsInRenderPass())
		vulkanCommandBuffer.EndRenderPass(true);

	vulkanCommandBuffer.CopyBufferToBuffer(src, dst, srcOffset, dstOffset, length);

	// Notify the command buffer that these resources are being used on it
	// TODO - Move this within VulkanCmdBuffer::CopyBufferToBuffer?
	vulkanCommandBuffer.RegisterBuffer(src, BufferUseFlagBits::Transfer, VulkanAccessFlag::Read);
	vulkanCommandBuffer.RegisterBuffer(dst, BufferUseFlagBits::Transfer, VulkanAccessFlag::Write);
}

void VulkanGpuBuffer::ReadData(u32 offset, u32 length, void* destination, const SPtr<GpuQueue>& gpuQueue)
{
	if((offset + length) > mTotalSize)
	{
		B3D_LOG(Error, RenderBackend, "Provided offset({0}) + length({1}) is larger than the buffer {2}.", offset, length, mTotalSize);
		return;
	}

	if(length == 0)
		return;

	if(mBuffer == nullptr)
		return;

	GpuQueue& transferGpuQueue = gpuQueue != nullptr ? *gpuQueue : *mDevice.GetQueue(GQT_GRAPHICS, 0);
	SPtr<VulkanGpuCommandBuffer> vulkanCommandBuffer;

	// Check is the GPU currently writing to the buffer
	const u32 writeUseMask = mBuffer->GetUseInfo(VulkanAccessFlag::Write);

	if(mDirectlyMappable) // TODO - Need to check if this is memory on an integrated GPU, in which case it might be directly mappable always
	{
		// Note: Even if GPU isn't currently using the buffer, but the buffer supports GPU writes, we consider it as
		// being used because the write could have completed yet still not visible, so we need to issue a pipeline
		// barrier below.
		const bool isUsedOnGPU = writeUseMask != 0 || mSupportsGPUWrites;

		// If used on the GPU, we need to wait until all write operations complete before mapping it
		if(isUsedOnGPU)
		{
			if (vulkanCommandBuffer == nullptr)
				vulkanCommandBuffer = std::static_pointer_cast<VulkanGpuCommandBuffer>(transferGpuQueue.GetOrCreateTransferCommandBuffer());

			// Make any writes visible before mapping
			if(mSupportsGPUWrites)
			{
				// Issue a barrier so :
				//  - If reading: the device makes the written memory available for read (read-after-write hazard)
				//  - If writing: ensures our writes properly overlap with GPU writes (write-after-write hazard)
				vulkanCommandBuffer->MemoryBarrier(mBuffer->GetVulkanHandle(), VK_ACCESS_SHADER_WRITE_BIT, VK_ACCESS_HOST_READ_BIT,
										  // Last stages that could have written to the buffer:
										  VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT | VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_PIPELINE_STAGE_HOST_BIT);
			}

			// Submit the command buffer and wait until it finishes
			vulkanCommandBuffer->AppendSyncMask(writeUseMask);
			transferGpuQueue.SubmitTransferCommandBuffer(true);
		}

		void* lockedData = Lock(offset, length, GBL_READ_ONLY);
		memcpy(destination, lockedData, length);
		Unlock();

		return;
	}

	// Not directly mappable, will need a staging buffer to copy into
	VulkanBuffer* const stagingBuffer = CreateBuffer(mDevice, length, true, true); // TODO - Allocate this from some memory pool

	// If buffer supports GPU writes or is currently being written to, we need to wait on any potential writes to complete
	u32 syncMask = 0;
	if(mSupportsGPUWrites || writeUseMask != 0)
	{
		// Ensure flush will wait for all queues currently writing to the buffer (if any) to finish
		syncMask = writeUseMask;
	}

	if (vulkanCommandBuffer == nullptr)
		vulkanCommandBuffer = std::static_pointer_cast<VulkanGpuCommandBuffer>(transferGpuQueue.GetOrCreateTransferCommandBuffer());

	// Queue copy command
	vulkanCommandBuffer->CopyBufferToBuffer(mBuffer, stagingBuffer, offset, 0, length);

	// Submit the command buffer and wait until it finishes
	vulkanCommandBuffer->AppendSyncMask(syncMask);
	transferGpuQueue.SubmitTransferCommandBuffer(true);

	void* lockedStagingData = stagingBuffer->Map(0, length, true);
	memcpy(destination, lockedStagingData, length);

	stagingBuffer->Unmap();
	stagingBuffer->Destroy();
}

void VulkanGpuBuffer::WriteData(u32 offset, u32 length, const void* source, BufferWriteType writeFlags, const SPtr<GpuCommandBuffer>& commandBuffer)
{
	if((offset + length) > mTotalSize)
	{
		B3D_LOG(Error, RenderBackend, "Provided offset({0}) + length({1}) is larger than the buffer {2}.", offset, length, mTotalSize);
		return;
	}

	if(length == 0)
		return;

	if(mBuffer == nullptr)
		return;

	GpuLockOptions options = GBL_WRITE_ONLY_DISCARD_RANGE;
	if(writeFlags == BWT_NO_OVERWRITE)
		options = GBL_WRITE_ONLY_NO_OVERWRITE;
	else if(writeFlags == BWT_DISCARD)
		options = GBL_WRITE_ONLY_DISCARD;

	const bool canDiscardBuffer =
		(options == GBL_WRITE_ONLY_DISCARD) ||
		(options == GBL_WRITE_ONLY_DISCARD_RANGE && offset == 0 && length == mTotalSize);

	// Check is the GPU currently reading or writing from the buffer
	const u32 useMask = mBuffer->GetUseInfo(VulkanAccessFlag::Read | VulkanAccessFlag::Write);

	if(mDirectlyMappable) // TODO - Need to check if this is memory on an integrated GPU, in which case it might be directly mappable always
	{
		// Note: Even if GPU isn't currently using the buffer, but the buffer supports GPU writes, we consider it as
		// being used because the write could have completed yet still not visible, so we need to issue a pipeline
		// barrier below.
		const bool isUsedOnGPU = useMask != 0 || mSupportsGPUWrites;

		// Even if the buffer is directly mappable we might wish to avoid mapping it directly in these situations:
		const bool shouldMapDirectly =
			(!isUsedOnGPU || options == GBL_WRITE_ONLY_NO_OVERWRITE) && // GPU is currently using the buffer and we cannot map it safely (unless user specifically requested the no-overwrite flag)
			(!mBuffer->IsBound() || (commandBuffer == nullptr && canDiscardBuffer)); // Buffer is bound to a command buffer already. If user provided a command buffer queue a write operation there instead of mapping directly. If not, discard the original buffer and lock a new copy of the buffer.

		if(shouldMapDirectly)
		{
			void* lockedData = Lock(offset, length, options);
			memcpy(lockedData, source, length);
			Unlock();

			return;
		}
	}

	// Can't use direct mapping, so use a staging buffer or memory
	// See if we can use the cheaper staging memory, rather than a staging buffer
	const bool useStagingMemory = offset % 4 == 0 && length % 4 == 0 && length <= 65536;

	// Create a staging buffer if needed
	VulkanBuffer* const stagingBuffer = !useStagingMemory ? CreateBuffer(mDevice, length, true, false) : nullptr;

	SPtr<VulkanGpuCommandBuffer> vulkanCommandBuffer = std::static_pointer_cast<VulkanGpuCommandBuffer>(commandBuffer != nullptr
		? commandBuffer
		: mDevice.GetQueue(GQT_GRAPHICS, 0)->GetOrCreateTransferCommandBuffer()
	);

	// Copy the source into the staging buffer
	if(stagingBuffer != nullptr)
	{
		void* lockedStagingData = stagingBuffer->Map(0, length);
		memcpy(lockedStagingData, source, length);

		stagingBuffer->Unmap();
	}

	// If the buffer is used in any way on the GPU, we need to wait for that use to finish before we issue our copy
	u32 syncMask = 0;
	if(useMask != 0 && options != GBL_WRITE_ONLY_NO_OVERWRITE) // Buffer is currently used on the GPU
	{
		syncMask = useMask;
	}

	// Check if the buffer will still be bound somewhere after the CBs using it finish
	const u32 useCount = mBuffer->GetUseCount();
	const u32 boundCount = mBuffer->GetBoundCount();

	const bool isBoundWithoutUse = boundCount > useCount;

	// If a buffer is bound a command buffer, and we're using a transfer buffer to update, warn as only the latest update will persist on the transfer buffer
	if(isBoundWithoutUse && commandBuffer == nullptr)
	{
		if(!canDiscardBuffer)
		{
			B3D_LOG(Warning, RenderBackend, "Writing to a buffer '{0}' that is currently bound on a command buffer, without providing an explicit command buffer. Such writes will be queued on the transfer buffer which is submitted before any user command buffers. This means multiple writes will overwrite it each other if not careful.", mName);
		}
		else
		{
			VulkanBuffer* newBuffer = CreateBuffer(mDevice, mTotalSize, false, true);
			mBuffer->Destroy();
			mBuffer = newBuffer;
		}
	}

	if(vulkanCommandBuffer->IsInRenderPass())
		vulkanCommandBuffer->EndRenderPass();

	// Queue copy/update command for the actual write
	if(stagingBuffer != nullptr)
	{
		vulkanCommandBuffer->CopyBufferToBuffer(stagingBuffer, mBuffer, 0, offset, length);

		// TODO - Move this within VulkanCmdBuffer::CopyBufferToBuffer?
		vulkanCommandBuffer->RegisterBuffer(stagingBuffer, BufferUseFlagBits::Transfer, VulkanAccessFlag::Read);
	}
	else // Staging memory
	{
		vulkanCommandBuffer->UpdateBuffer(mBuffer, (u8*)source, offset, length);
	}

	// TODO - Move this within VulkanCmdBuffer::CopyBufferToBuffer?
	vulkanCommandBuffer->RegisterBuffer(mBuffer, BufferUseFlagBits::Transfer, VulkanAccessFlag::Write);

	if(stagingBuffer != nullptr)
		stagingBuffer->Destroy();

	vulkanCommandBuffer->AppendSyncMask(syncMask);

	// We don't actually flush the transfer buffer here since it's an expensive operation, but it's instead
	// done automatically before next "normal" command buffer submission.
}

VkBufferView VulkanGpuBuffer::GetOrCreateView(GpuBufferFormat format) const
{
	if(mInformation.Type != GpuBufferType::SimpleStorage || mBuffer == nullptr)
		return nullptr;

	if(format == BF_UNKNOWN)
		format = mInformation.SimpleStorage.Format;

	return mBuffer->GetOrCreateView(VulkanUtility::GetBufferFormat(mInformation.SimpleStorage.Format));
}

