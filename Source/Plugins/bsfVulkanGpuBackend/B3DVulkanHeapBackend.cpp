//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DVulkanHeapBackend.h"
#include "B3DVulkanGpuDevice.h"
#include "B3DVulkanSubmitThread.h"

using namespace b3d;
using namespace b3d::render;

namespace
{
	/**
	 * Picks a memory-type index satisfying both @p memoryTypeBits (the @c VkMemoryRequirements mask
	 * from a representative resource) and @p propertyFlags (the caller's required property bits).
	 * Returns @c VK_MAX_MEMORY_TYPES on failure so the caller can assert / fall back.
	 */
	u32 ChooseMemoryType(const VkPhysicalDeviceMemoryProperties& memoryProperties, u32 memoryTypeBits, VkMemoryPropertyFlags propertyFlags)
	{
		for(u32 typeIndex = 0; typeIndex < memoryProperties.memoryTypeCount; ++typeIndex)
		{
			const bool typeAllowed = (memoryTypeBits & (1u << typeIndex)) != 0;
			if(!typeAllowed)
				continue;

			const VkMemoryPropertyFlags typeFlags = memoryProperties.memoryTypes[typeIndex].propertyFlags;
			if((typeFlags & propertyFlags) == propertyFlags)
				return typeIndex;
		}

		return VK_MAX_MEMORY_TYPES;
	}
}

VulkanHeapBackend::VulkanHeapBackend(VulkanGpuDevice& device)
	: mDevice(&device) , mLogicalDevice(device.GetLogical())
{
}

VulkanHeapBackend::~VulkanHeapBackend() = default;

VulkanHeapHandle VulkanHeapBackend::CreateHeap(u64 sizeInBytes, const VulkanHeapCreateInformation& createInformation)
{
	const VkPhysicalDeviceMemoryProperties& memProps = mDevice->GetMemoryProperties();

	const u32 memoryTypeIndex = ChooseMemoryType(memProps, createInformation.MemoryTypeBits, createInformation.PropertyFlags);
	B3D_ASSERT(memoryTypeIndex != VK_MAX_MEMORY_TYPES && "No memory type satisfies the requested MemoryTypeBits + PropertyFlags.");

	VkMemoryAllocateInfo allocateInfo = {};
	allocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocateInfo.allocationSize = sizeInBytes;
	allocateInfo.memoryTypeIndex = memoryTypeIndex;

	VulkanHeapHandle handle = {};
	handle.Size = sizeInBytes;
	handle.MemoryTypeIndex = memoryTypeIndex;

	VkResult result = vkAllocateMemory(mLogicalDevice, &allocateInfo, gVulkanAllocator, &handle.Memory);
	B3D_ASSERT(result == VK_SUCCESS);

	if(createInformation.MapPersistently)
	{
		const VkMemoryPropertyFlags typeFlags = memProps.memoryTypes[memoryTypeIndex].propertyFlags;
		B3D_ASSERT((typeFlags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT) != 0 && "MapPersistently set on a non-HOST_VISIBLE memory type.");

		result = vkMapMemory(mLogicalDevice, handle.Memory, 0, VK_WHOLE_SIZE, 0, &handle.Mapped);
		B3D_ASSERT(result == VK_SUCCESS);
	}

	return handle;
}

void VulkanHeapBackend::DestroyHeap(VulkanHeapHandle handle)
{
	if(handle.Memory == VK_NULL_HANDLE)
		return;

	if(handle.Mapped != nullptr)
		vkUnmapMemory(mLogicalDevice, handle.Memory);

	vkFreeMemory(mLogicalDevice, handle.Memory, gVulkanAllocator);
}

// Conformance check
B3D_STATIC_ASSERT_HEAP_BACKEND_IS_VALID(VulkanHeapBackend);
