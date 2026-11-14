//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DVulkanGpuBuffer.h"

#include "B3DApplication.h"
#include "B3DVulkanGpuDevice.h"
#include "B3DVulkanUtility.h"
#include "B3DVulkanGpuCommandBuffer.h"
#include "B3DVulkanGpuBackend.h"
#include "GpuBackend/B3DGpuBackendUtility.h"

using namespace b3d;
using namespace b3d::render;

namespace
{
	/** Any structured buffer in GPU memory the GPU can write can be the scratch memory of an acceleration structure build. */
	bool IsAccelerationStructureScratchCapable(GpuBufferType type, GpuBufferMemory memory, GpuBufferFlags flags)
	{
		return type == GpuBufferType::StructuredStorage && memory == GpuBufferMemory::Device && flags.IsSet(GpuBufferFlag::GpuWrite);
	}
}

VulkanBuffer::VulkanBuffer(VulkanResourceManager* owner, const VulkanBufferCreateInformation& createInformation, VkBuffer buffer, VulkanAllocationResult allocation)
	: TVulkanResource<IGpuBufferResource>(owner, createInformation.VkCreateInfo.sharingMode == VK_SHARING_MODE_CONCURRENT, createInformation.DebugName), mFlags(createInformation.Flags), mBuffer(buffer)
	, mAllocation(allocation), mMappedMemory(allocation.MappedMemory)
{
	// Read once, so builds on any thread read the stored value
	if((createInformation.VkCreateInfo.usage & VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT) != 0)
	{
		VkBufferDeviceAddressInfo addressInfo;
		addressInfo.sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO;
		addressInfo.pNext = nullptr;
		addressInfo.buffer = mBuffer;

		mDeviceAddress = vkGetBufferDeviceAddress(mOwner->GetDevice().GetLogical(), &addressInfo);
	}
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

VulkanGpuBuffer::VulkanGpuBuffer(VulkanGpuDevice& device, const GpuBufferCreateInformation& createInformation, const GpuAllocation& allocation)
	: GpuBuffer(device, createInformation, b3d::GpuBuffer::CalculateSuballocatedBufferSize(createInformation, device), allocation), mSupportsGPUWrites(createInformation.Flags.IsSet(GpuBufferFlag::GpuWrite))
	{ }

VulkanGpuBuffer::~VulkanGpuBuffer()
{
	if(mBuffer != nullptr)
		mBuffer->Destroy();
}

void VulkanGpuBuffer::Initialize()
{
	RecreateInternalBuffer();
	GpuBuffer::Initialize();
}

VkBufferUsageFlags VulkanGpuBuffer::GetVkBufferUsageFlags(const VulkanGpuDevice& device, const GpuBufferCreateInformation& createInformation)
{
	// Buffer usage bits from the buffer type. Every type is a transfer destination (buffers are uploaded
	// to) except StagingWrite, which is a pure copy source. Shaders read vertex and index buffers through raw views.
	VkBufferUsageFlags usageFlags = 0;
	switch(createInformation.Type)
	{
	case GpuBufferType::Vertex:
		usageFlags = VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
		break;
	case GpuBufferType::Index:
		usageFlags = VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
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
	default:
		break;
	}

	if(createInformation.Flags.IsSet(GpuBufferFlag::GpuWrite))
	{
		if(createInformation.Type == GpuBufferType::SimpleStorage)
			usageFlags |= VK_BUFFER_USAGE_STORAGE_TEXEL_BUFFER_BIT;
		else
			usageFlags |= VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
	}

	// Acceleration structure builds read their geometry, instances and scratch memory through device addresses
	if(device.IsBufferDeviceAddressEnabled())
	{
		if(createInformation.Flags.IsSet(GpuBufferFlag::AccelerationStructureInput))
			usageFlags |= VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT | VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR;

		if(IsAccelerationStructureScratchCapable(createInformation.Type, createInformation.Memory, createInformation.Flags))
			usageFlags |= VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT;
	}

	// Persistent (non-staging) buffers are readable by default
	usageFlags |= VK_BUFFER_USAGE_TRANSFER_SRC_BIT;

	return usageFlags;
}

u32 VulkanGpuBuffer::GetMinimumAlignment(const VulkanGpuDevice& device, GpuBufferType type, GpuBufferMemory memory, GpuBufferFlags flags)
{
	u32 alignment = 1u;

	// The alignment is zero on devices without acceleration structures
	if(IsAccelerationStructureScratchCapable(type, memory, flags))
		alignment = std::max(device.GetCapabilities().RayTracing.ScratchOffsetAlignment, alignment);

	// Instance data of top level acceleration structure builds must be 16-byte aligned
	static constexpr u32 kInstanceAlignment = 16;
	if(type == GpuBufferType::StructuredStorage && flags.IsSet(GpuBufferFlag::AccelerationStructureInput))
		alignment = std::max(kInstanceAlignment, alignment);

	return alignment;
}

void VulkanGpuBuffer::GetMemoryFlags(const VulkanGpuDevice& device, GpuBufferType type, GpuBufferMemory memory, GpuBufferFlags flags,
	VulkanMemoryFlags& outMemoryFlags, TOptional<VulkanMemoryFlags>& outFallbackMemoryFlags)
{
	// Memory no buffer is ever in: protected, lazily allocated, and AMD's device coherent types, which need a feature the device doesn't enable
	constexpr VkMemoryPropertyFlags kUnusableFlags = VK_MEMORY_PROPERTY_PROTECTED_BIT | VK_MEMORY_PROPERTY_LAZILY_ALLOCATED_BIT |
		VK_MEMORY_PROPERTY_DEVICE_COHERENT_BIT_AMD | VK_MEMORY_PROPERTY_DEVICE_UNCACHED_BIT_AMD;

	outFallbackMemoryFlags.reset();

	// Map the engine's high-level buffer-usage hint to memory-property flags.
	if(type == GpuBufferType::StagingRead)
	{
		// CPU readback (transfer-dst staging): host-visible + cached for fast CPU reads.
		outMemoryFlags = VulkanMemoryFlags(VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_CACHED_BIT);
	}
	else if(type == GpuBufferType::StagingWrite)
	{
		// CPU upload staging: host-visible + coherent for write-combined upload.
		outMemoryFlags = VulkanMemoryFlags(VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
	}
	else
	{
		const bool isUnified = device.GetCapabilities().HasCapability(RSC_UNIFIED_MEMORY);
		const VkMemoryPropertyFlags unifiedLocalFlags = isUnified ? VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT : 0;

		// Mapped buffers stay mapped once the GPU memory they prefer is close to full, in system memory. Among the types that qualify,
		// the lowest index is write-combined rather than cached, as Vulkan lists a type before the types with more flags.
		const VulkanMemoryFlags systemMemoryFlags(VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT | kUnusableFlags);

		switch(GpuBackendUtility::GetBufferPlacement(memory, flags, device.HasFullSpeedMappableGpuMemory()))
		{
		case GpuBufferPlacement::MappedGpuOrUnmapped:
			// VulkanGpuDevice::InitializeCapabilities() checks the type these flags pick is mappable at full GPU speed. Near the budget the
			// buffers are better unmapped than read by the GPU from system memory.
			outMemoryFlags = VulkanMemoryFlags(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
			outFallbackMemoryFlags = VulkanMemoryFlags(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, 0, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | kUnusableFlags);
			break;
		case GpuBufferPlacement::MappedGpuOrSystem:
			// As above, but near the budget the buffers stay mapped, in system memory
			outMemoryFlags = VulkanMemoryFlags(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
			outFallbackMemoryFlags = systemMemoryFlags;
			break;
		case GpuBufferPlacement::MappedSystem:
			// Integrated GPUs read the part of memory reserved for them faster, though the HOST_VISIBLE part of it can be small
			outMemoryFlags = VulkanMemoryFlags(VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | unifiedLocalFlags);
			outFallbackMemoryFlags = systemMemoryFlags;
			break;
		case GpuBufferPlacement::MappedCached:
			// No DEVICE_LOCAL preference: it could pick an uncached type of the GPU's part of memory over a cached one (AMD). Should a
			// cached type in GPU memory be picked anyway, the fallback stays cached.
			outMemoryFlags = VulkanMemoryFlags(VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, VK_MEMORY_PROPERTY_HOST_CACHED_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
			outFallbackMemoryFlags = VulkanMemoryFlags(VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, VK_MEMORY_PROPERTY_HOST_CACHED_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
				VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT | kUnusableFlags);
			break;
		default:
			outMemoryFlags = VulkanMemoryFlags(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, 0);
			break;
		}
	}
}

VulkanBuffer* VulkanGpuBuffer::CreateBuffer(VulkanGpuDevice& device, u32 size, bool staging, bool readable, const GpuAllocation& requestedAllocation)
{
	// Not allowed to have size 0 buffer
	if(size == 0)
		size = 64;

	const GpuBufferType newBufferType = staging ? readable ? GpuBufferType::StagingRead : GpuBufferType::StagingWrite : mInformation.Type;
	const GpuBufferFlags newBufferFlags = staging ? (GpuBufferFlags)0 : mInformation.Flags;

	const String debugName = staging ? StringUtility::Format("Staging buffer ({0})", mName) : mName;

	// Resolve usage flags from the buffer's engine usage hint. The memory type (and so the memory-property
	// flags) was already resolved when the allocation was picked at proxy creation.
	VkBufferUsageFlags usageFlags = GetVkBufferUsageFlags(device, mInformation);

	// Buffers are shared so that accesses on any queue need no ownership transfer, which lets them rest
	const TInlineArray<u32, GQT_COUNT>& queueFamilies = device.GetQueueFamilies();
	const bool usesConcurrentSharing = queueFamilies.Size() > 1;

	VulkanBufferCreateInformation info;
	info.VkCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	info.VkCreateInfo.pNext = nullptr;
	info.VkCreateInfo.flags = 0;
	info.VkCreateInfo.sharingMode = usesConcurrentSharing ? VK_SHARING_MODE_CONCURRENT : VK_SHARING_MODE_EXCLUSIVE;
	info.VkCreateInfo.usage = usageFlags;
	info.VkCreateInfo.queueFamilyIndexCount = usesConcurrentSharing ? (u32)queueFamilies.Size() : 0;
	info.VkCreateInfo.pQueueFamilyIndices = usesConcurrentSharing ? queueFamilies.data() : nullptr;
	info.Type = newBufferType;
	info.Flags = newBufferFlags;
	info.DebugName = debugName;
	info.MinimumAlignment = GetMinimumAlignment(device, newBufferType, mInformation.Memory, newBufferFlags);
	info.VkCreateInfo.size = size;

	if(staging)
	{
		// Staging sub-buffer override usage as a pure copy source, plus a copy destination when readable.
		info.VkCreateInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;

		if(readable)
			info.VkCreateInfo.usage |= VK_BUFFER_USAGE_TRANSFER_DST_BIT;
	}

	GpuAllocation allocation;
	VulkanBuffer* const vulkanBuffer = device.CreateBuffer(info, requestedAllocation, allocation);
	SetCurrentAllocation(allocation);

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
	VulkanBuffer* newBuffer = CreateBuffer(GetVulkanDevice(), mTotalSize, false, true, mRequestedAllocation);

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

void VulkanGpuBuffer::SwapInternalBuffer(GpuBuffer& other)
{
	VulkanGpuBuffer& vulkanOther = static_cast<VulkanGpuBuffer&>(other);
	std::swap(mBuffer, vulkanOther.mBuffer);
	std::swap(mMappedMemory, vulkanOther.mMappedMemory);
}

