//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DVulkanGpuBuffer.h"

#include "B3DApplication.h"
#include "B3DVulkanGpuDevice.h"
#include "B3DVulkanUtility.h"
#include "B3DVulkanGpuCommandBuffer.h"
#include "B3DVulkanGpuBackend.h"

using namespace b3d;
using namespace b3d::render;

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

#if B3D_BUILD_TYPE_DEVELOPMENT

void VulkanBuffer::InitializeSuballocationTracking(u32 suballocationCount, u32 suballocationSize)
{
	Lock lock(mMutex);

	mSuballocationSize = suballocationSize;

	// Only track if there are multiple suballocations (single suballocation falls back to whole-buffer tracking)
	if(suballocationCount > 1)
	{
		mSuballocationStates.Clear();
		for(u32 subIndex = 0; subIndex < suballocationCount; ++subIndex)
			mSuballocationStates.Add(SuballocationTrackingState{});
	}
}

void VulkanBuffer::NotifySuballocationBound(u32 suballocationIndex)
{
	Lock lock(mMutex);
	if(mSuballocationStates.Empty())
		return; // Single suballocation, use existing whole-buffer tracking

	B3D_ASSERT(suballocationIndex < mSuballocationStates.Size());
	mSuballocationStates[suballocationIndex].BoundCount++;
}

void VulkanBuffer::NotifySuballocationUsed(u32 suballocationIndex)
{
	Lock lock(mMutex);
	if(mSuballocationStates.Empty())
		return;

	B3D_ASSERT(suballocationIndex < mSuballocationStates.Size());
	mSuballocationStates[suballocationIndex].UseCount++;
}

void VulkanBuffer::NotifySuballocationDone(u32 suballocationIndex)
{
	Lock lock(mMutex);
	if(mSuballocationStates.Empty())
		return;

	B3D_ASSERT(suballocationIndex < mSuballocationStates.Size());
	B3D_ASSERT(mSuballocationStates[suballocationIndex].UseCount > 0);
	B3D_ASSERT(mSuballocationStates[suballocationIndex].BoundCount > 0);
	mSuballocationStates[suballocationIndex].UseCount--;
	mSuballocationStates[suballocationIndex].BoundCount--;
}

void VulkanBuffer::NotifySuballocationUnbound(u32 suballocationIndex)
{
	Lock lock(mMutex);
	if(mSuballocationStates.Empty())
		return;

	B3D_ASSERT(suballocationIndex < mSuballocationStates.Size());
	B3D_ASSERT(mSuballocationStates[suballocationIndex].BoundCount > 0);
	mSuballocationStates[suballocationIndex].BoundCount--;
}

bool VulkanBuffer::IsSuballocationBound(u32 suballocationIndex) const
{
	Lock lock(mMutex);
	if(mSuballocationStates.Empty())
		return mBountCount > 0; // Fall back to whole-buffer tracking

	B3D_ASSERT(suballocationIndex < mSuballocationStates.Size());
	return mSuballocationStates[suballocationIndex].BoundCount > 0;
}

bool VulkanBuffer::IsSuballocationInUse(u32 suballocationIndex) const
{
	Lock lock(mMutex);
	if(mSuballocationStates.Empty())
		return mUsedCount > 0; // Fall back to whole-buffer tracking

	B3D_ASSERT(suballocationIndex < mSuballocationStates.Size());
	return mSuballocationStates[suballocationIndex].UseCount > 0;
}

u32 VulkanBuffer::GetSuballocationIndexForOffset(u32 offset) const
{
	if(mSuballocationSize == 0)
		return 0;

	return offset / mSuballocationSize;
}

bool VulkanBuffer::IsRangeBound(u32 offset, u32 size) const
{
	Lock lock(mMutex);
	if(mSuballocationStates.Empty())
		return mBountCount > 0; // Fall back to whole-buffer tracking

	const u32 firstSuballoc = GetSuballocationIndexForOffset(offset);
	const u32 lastSuballoc = GetSuballocationIndexForOffset(offset + size - 1);

	for(u32 subIndex = firstSuballoc; subIndex <= lastSuballoc && subIndex < mSuballocationStates.Size(); ++subIndex)
	{
		if(mSuballocationStates[subIndex].BoundCount > 0)
			return true;
	}
	return false;
}

bool VulkanBuffer::IsRangeInUse(u32 offset, u32 size) const
{
	Lock lock(mMutex);
	if(mSuballocationStates.Empty())
		return mUsedCount > 0; // Fall back to whole-buffer tracking

	const u32 firstSuballoc = GetSuballocationIndexForOffset(offset);
	const u32 lastSuballoc = GetSuballocationIndexForOffset(offset + size - 1);

	for(u32 subIndex = firstSuballoc; subIndex <= lastSuballoc && subIndex < mSuballocationStates.Size(); ++subIndex)
	{
		if(mSuballocationStates[subIndex].UseCount > 0)
			return true;
	}
	return false;
}
#endif // B3D_BUILD_TYPE_DEVELOPMENT

VulkanGpuBuffer::VulkanGpuBuffer(VulkanGpuDevice& device, const GpuBufferCreateInformation& createInformation)
	: GpuBuffer(device, createInformation, b3d::GpuBuffer::CalculateSuballocatedBufferSize(createInformation, device)), mDirectlyMappable((createInformation.Flags.IsSetAny(GpuBufferFlag::StoreOnCPUWithGPUAccess)) != 0 || createInformation.Type == GpuBufferType::StagingRead || createInformation.Type == GpuBufferType::StagingWrite), mSupportsGPUWrites(createInformation.Flags.IsSet(GpuBufferFlag::AllowUnorderedAccessOnTheGPU)), mIsMapped(false)
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

	if(mInformation.Flags.IsSet(GpuBufferFlag::AllowUnorderedAccessOnTheGPU))
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

	// TODO - Should all buffer really be readable by default?
	mBuffer = CreateBuffer(GetVulkanDevice(), mTotalSize, mInformation.Type == GpuBufferType::StagingRead || mInformation.Type == GpuBufferType::StagingWrite, mInformation.Type != GpuBufferType::StagingWrite);

#if B3D_BUILD_TYPE_DEVELOPMENT
	// Initialize suballocation tracking for debug builds
	if(mBuffer != nullptr)
		mBuffer->InitializeSuballocationTracking(mInformation.SuballocationCount, mSuballocationSize);
#endif
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
	GpuQueueMask useMask = buffer->GetUseInfo(GpuAccessFlag::Read | GpuAccessFlag::Write);

	// Note: Even if GPU isn't currently using the buffer, but the buffer supports GPU writes, we consider it as
	// being used because the write could have completed yet still not visible, so we need to issue a pipeline
	// barrier below.
	const bool isUsedOnGPU = !useMask.IsEmpty() || mSupportsGPUWrites; // TODO - mSupportsGPUWrites check doesn't seem right here. 

	// TODO - Not issuing barrier here

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
			// Warn unless user claims to know what he is doing by using the no overwrite flag
			if(options != GBL_WRITE_ONLY_NO_OVERWRITE)
			{
#if B3D_BUILD_TYPE_DEVELOPMENT
				if(buffer->IsRangeBound(offset, length))
				{
					B3D_LOG(Warning, RenderBackend, "Writing to a buffer that is currently bound on a command buffer. Previous usages of the buffer will be affected. Buffer: {0}", mName);
				}
#endif
			}

			return buffer->Map(offset, length, isReadRequired);
		}
	}

	// TODO - Once we remove discard here, validate the the buffer range is not being used

	// Used on the GPU but caller doesn't care about buffer contents, so just discard the existing buffer and create a new one
	if(canDiscardBuffer)
	{
		buffer->Destroy();

		buffer = CreateBuffer(GetVulkanDevice(), mTotalSize, false, true);
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

GpuQueueMask VulkanGpuBuffer::GetUseMask(GpuAccessFlags accessFlags)
{
	return mBuffer->GetUseInfo(accessFlags);
}

VkBufferView VulkanGpuBuffer::GetOrCreateView(GpuBufferFormat format) const
{
	if(mInformation.Type != GpuBufferType::SimpleStorage || mBuffer == nullptr)
		return nullptr;

	if(format == BF_UNKNOWN)
		format = mInformation.SimpleStorage.Format;

	return mBuffer->GetOrCreateView(VulkanUtility::GetBufferFormat(format));
}

void VulkanGpuBuffer::RecreateInternalBuffer()
{
	VulkanBuffer* newBuffer = CreateBuffer(GetVulkanDevice(), mTotalSize, false, true);
	mBuffer->Destroy();
	mBuffer = newBuffer;

#if B3D_BUILD_TYPE_DEVELOPMENT
	// Initialize suballocation tracking for the new buffer
	if(mBuffer != nullptr)
		mBuffer->InitializeSuballocationTracking(mInformation.SuballocationCount, mSuballocationSize);
#endif
}
