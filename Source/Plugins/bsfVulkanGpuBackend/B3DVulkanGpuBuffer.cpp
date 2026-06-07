//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DVulkanGpuBuffer.h"

#include "B3DApplication.h"
#include "B3DVulkanGpuDevice.h"
#include "B3DVulkanUtility.h"
#include "B3DVulkanGpuCommandBuffer.h"
#include "B3DVulkanGpuBackend.h"

using namespace b3d;
using namespace b3d::render;

VulkanBuffer::VulkanBuffer(VulkanResourceManager* owner, const VulkanBufferCreateInformation& createInformation, VkBuffer buffer, VulkanAllocationResult allocation, VulkanGpuBuffer* parent)
	: VulkanResource(owner, false, createInformation.DebugName), mType(createInformation.Type), mFlags(createInformation.Flags), mBuffer(buffer), mAllocation(allocation), mParent(parent), mMappedMemory(allocation.MappedMemory)
{
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

	if (mAllocation.IsValid())
		device.FreeMemory(mAllocation);
}

IGpuResource* VulkanBuffer::MoveAllocation(render::GpuCommandBuffer& commandBuffer, const GpuResourceLocation& newLocationBase)
{
	B3D_ASSERT(mParent != nullptr && "VulkanBuffer::MoveAllocation invoked on an untracked wrapper (no parent VulkanGpuBuffer).");
	B3D_ASSERT(mParent->GetVulkanResource() == this && "Parent's mBuffer no longer points at this wrapper — proxy invariant broken.");

	const auto& newLocation = static_cast<const TGpuResourceLocation<VulkanHeapBackend>&>(newLocationBase);

	VulkanAllocationResult preReserved;
	preReserved.Location = newLocation;
	preReserved.MappedMemory = newLocation.Heap.Mapped != nullptr ? static_cast<u8*>(newLocation.Heap.Mapped) + newLocation.Offset : nullptr;

	VulkanBuffer* newBuffer = mParent->RelocateInternalBuffer(preReserved, commandBuffer);

	// Destroy self
	Destroy();

	return newBuffer;
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

	mMappedOffset = offset;
	mMappedSize = size;

	return device.MapMemory(mAllocation, offset);
}

void VulkanBuffer::Unmap(bool isFlushRequired)
{
	VulkanGpuDevice& device = mOwner->GetDevice();

	device.UnmapMemory(mAllocation);

	if(isFlushRequired)
		device.FlushMemory(mAllocation, mMappedOffset, mMappedSize);
}

void VulkanBuffer::Flush(VkDeviceSize offset, VkDeviceSize size)
{
	VulkanGpuDevice& device = mOwner->GetDevice();
	device.FlushMemory(mAllocation, offset, size);
}

void VulkanBuffer::Invalidate(VkDeviceSize offset, VkDeviceSize size)
{
	VulkanGpuDevice& device = mOwner->GetDevice();
	device.InvalidateMemory(mAllocation, offset, size);
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
	: GpuBuffer(device, createInformation, b3d::GpuBuffer::CalculateSuballocatedBufferSize(createInformation, device)), mDirectlyMappable((createInformation.Flags.IsSetAny(GpuBufferFlag::StoreOnCPUWithGPUAccess)) != 0 || createInformation.Type == GpuBufferType::StagingRead || createInformation.Type == GpuBufferType::StagingWrite), mSupportsGPUWrites(createInformation.Flags.IsSet(GpuBufferFlag::AllowUnorderedAccessOnTheGPU))
	{ }

VulkanGpuBuffer::~VulkanGpuBuffer()
{
	if(mBuffer != nullptr)
		mBuffer->Destroy();
}

void VulkanGpuBuffer::Initialize()
{
	RecreateInternalBuffer();
}

VulkanBuffer* VulkanGpuBuffer::CreateBuffer(VulkanGpuDevice& device, u32 size, bool staging, bool readable, const VulkanAllocationResult* preAllocatedMemory)
{
	// Not allowed to have size 0 buffer
	if(size == 0)
		size = 64;

	const GpuBufferType newBufferType = staging ? readable ? GpuBufferType::StagingRead : GpuBufferType::StagingWrite : mInformation.Type;
	const GpuBufferFlags newBufferFlags = staging ? (GpuBufferFlags)0 : mInformation.Flags;

	// Transient buffers are allocated from the linear (bump) allocator, which doesn't participate in defragmentation
	const bool transient = newBufferFlags.IsSet(GpuBufferFlag::Transient);

	// Persistent buffers pass `this` as the proxy parent so defrag can recreate them in place.
	VulkanGpuBuffer* const proxyParent = (staging || transient) ? nullptr : this;

	const String debugName = staging ? StringUtility::Format("Staging buffer ({0})", mName) : mName;

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

	VulkanBufferCreateInformation info;
	info.VkCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	info.VkCreateInfo.pNext = nullptr;
	info.VkCreateInfo.flags = 0;
	info.VkCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	info.VkCreateInfo.usage = usageFlags;
	info.VkCreateInfo.queueFamilyIndexCount = 0;
	info.VkCreateInfo.pQueueFamilyIndices = nullptr;
	info.Type = newBufferType;
	info.Flags = newBufferFlags;
	info.DebugName = debugName;
	info.VkCreateInfo.size = size;
	if(staging)
	{
		info.VkCreateInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;

		// Staging buffers are used as a destination for reads
		if(readable)
			info.VkCreateInfo.usage |= VK_BUFFER_USAGE_TRANSFER_DST_BIT;
	}
	else if(readable) // Non-staging readable
		info.VkCreateInfo.usage |= VK_BUFFER_USAGE_TRANSFER_SRC_BIT;

	// Map the engine's high-level buffer-usage hint to (required, preferred) memory-property flags. The
	// PickMemoryTypeIndex picker prefers the lowest-index type that has all required bits and the most
	// matching preferred bits — this mirrors VMA's prior behaviour for these usage classes.
	VkMemoryPropertyFlags requiredFlags = 0;
	VkMemoryPropertyFlags preferredFlags = 0;
	if(staging)
	{
		if(readable)
		{
			// CPU readback (e.g. transfer-dst staging): host-visible + cached for fast CPU reads.
			requiredFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;
			preferredFlags = VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_CACHED_BIT;
		}
		else
		{
			// CPU upload staging: host-visible + coherent for write-combined upload.
			requiredFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;
			preferredFlags = VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
		}
	}
	else if(mInformation.Type == GpuBufferType::StagingRead)
	{
		requiredFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;
		preferredFlags = VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_CACHED_BIT;
	}
	else if(mInformation.Type == GpuBufferType::StagingWrite)
	{
		requiredFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;
		preferredFlags = VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
	}
	else if(mInformation.Flags.IsSet(GpuBufferFlag::StoreOnCPUWithGPUAccess))
	{
		// Persistent CPU-write-then-GPU-read buffer (e.g. dynamic uniform buffers). Prefer ReBAR-style
		// HOST_VISIBLE + DEVICE_LOCAL when available, otherwise plain HOST_VISIBLE.
		requiredFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;
		preferredFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
	}
	else
	{
		// Pure GPU buffer (vertex / index / structured / uniform with TRANSFER_DST init): DEVICE_LOCAL only.
		requiredFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
		preferredFlags = 0;
	}

	VulkanBuffer* const vulkanBuffer = preAllocatedMemory != nullptr 
		? device.CreateBuffer(info, *preAllocatedMemory, proxyParent)
		: device.CreateBuffer(info, requiredFlags, preferredFlags, proxyParent);

	if(vulkanBuffer != nullptr)
		vulkanBuffer->SetName(debugName);

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

GpuQueueMask VulkanGpuBuffer::GetUseMask(GpuAccessFlags accessFlags)
{
	return mBuffer->GetUseInfo(accessFlags);
}

void VulkanGpuBuffer::Flush(u32 offset, u32 size)
{
	mBuffer->Flush(offset, size);
}

void VulkanGpuBuffer::Invalidate(u32 offset, u32 size)
{
	mBuffer->Invalidate(offset, size);
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

	if (mBuffer != nullptr)
		mBuffer->Destroy();

	mBuffer = newBuffer;
	mMappedMemory = newBuffer->GetMappedMemory();

#if B3D_BUILD_TYPE_DEVELOPMENT
	// Initialize suballocation tracking for the new buffer
	if(mBuffer != nullptr)
		mBuffer->InitializeSuballocationTracking(mInformation.SuballocationCount, mSuballocationSize);
#endif
}

VulkanBuffer* VulkanGpuBuffer::RelocateInternalBuffer(const VulkanAllocationResult& preReserved, render::GpuCommandBuffer& commandBuffer)
{
	VulkanBuffer* const oldBuffer = mBuffer;

	const bool isStaging = mInformation.Type == GpuBufferType::StagingRead || mInformation.Type == GpuBufferType::StagingWrite;

	// TODO - Should all buffer really be readable by default?
	const bool isReadable = mInformation.Type != GpuBufferType::StagingWrite;

	VulkanBuffer* newBuffer = CreateBuffer(GetVulkanDevice(), mTotalSize, isStaging, isReadable, &preReserved);

	if(oldBuffer != nullptr)
	{
		auto& vulkanCb = static_cast<VulkanGpuCommandBuffer&>(commandBuffer);
		vulkanCb.CopyBufferToBuffer(oldBuffer, newBuffer, 0, 0, mTotalSize);
	}

	mBuffer = newBuffer;
	mMappedMemory = newBuffer->GetMappedMemory();

#if B3D_BUILD_TYPE_DEVELOPMENT
	if(mBuffer != nullptr)
		mBuffer->InitializeSuballocationTracking(mInformation.SuballocationCount, mSuballocationSize);
#endif

	return newBuffer;
}
