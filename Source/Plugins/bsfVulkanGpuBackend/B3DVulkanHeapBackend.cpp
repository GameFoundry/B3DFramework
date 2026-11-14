//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DVulkanHeapBackend.h"
#include "B3DVulkanGpuDevice.h"

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

IGpuHeap* VulkanHeapBackend::CreateHeap(u64 sizeInBytes, const VulkanHeapCreateInformation& createInformation)
{
	const VkPhysicalDeviceMemoryProperties& memProps = mDevice->GetMemoryProperties();

	u32 memoryTypeIndex = ChooseMemoryType(memProps, createInformation.MemoryTypeBits, createInformation.PropertyFlags);
	B3D_ASSERT(memoryTypeIndex != VK_MAX_MEMORY_TYPES && "No memory type satisfies the requested MemoryTypeBits + PropertyFlags.");

	// Host-visible types in GPU memory hold data the CPU writes. Their heap can be small (the 256 MB window of integrated GPUs) or shared
	// with every other resource, so once the heap is close to full, or the allocation fails, such a heap is in the fallback type, where
	// the same resources can live. The heap keeps a tenth of its size free for the resources that must be in it.
	const u32 fallbackMemoryTypeIndex = createInformation.FallbackMemoryTypeIndex;
	if(fallbackMemoryTypeIndex != VK_MAX_MEMORY_TYPES && fallbackMemoryTypeIndex != memoryTypeIndex)
	{
		const u32 memoryHeapIndex = memProps.memoryTypes[memoryTypeIndex].heapIndex;
		const VkDeviceSize memoryHeapSize = memProps.memoryHeaps[memoryHeapIndex].size;

		Lock lock(mHeapPoolMutex);
		if(mMemoryHeapUsage[memoryHeapIndex] + sizeInBytes > memoryHeapSize - memoryHeapSize / 10)
		{
			constexpr u64 kMb = 1024 * 1024;
			B3D_LOG(Warning, LogRenderBackend, "Vulkan: GPU memory heap {0} is close to full ({1} of {2} MiB used), so a {3} MiB heap of CPU-written resources is in "
				"memory type {4} instead.", memoryHeapIndex, mMemoryHeapUsage[memoryHeapIndex] / kMb, memoryHeapSize / kMb, sizeInBytes / kMb, fallbackMemoryTypeIndex);
			memoryTypeIndex = fallbackMemoryTypeIndex;
		}
	}

	VkMemoryAllocateInfo allocateInfo = {};
	allocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocateInfo.allocationSize = sizeInBytes;
	allocateInfo.memoryTypeIndex = memoryTypeIndex;

	// Any buffer may be an acceleration structure build input, which the build reads through its device address
	VkMemoryAllocateFlagsInfo allocateFlagsInfo = {};
	allocateFlagsInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_FLAGS_INFO;
	allocateFlagsInfo.flags = VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT;
	if(mDevice->IsBufferDeviceAddressEnabled())
		allocateInfo.pNext = &allocateFlagsInfo;

	VkDeviceMemory memory = VK_NULL_HANDLE;
	VkResult result = vkAllocateMemory(mLogicalDevice, &allocateInfo, gVulkanAllocator, &memory);
	if(result != VK_SUCCESS && fallbackMemoryTypeIndex != VK_MAX_MEMORY_TYPES && memoryTypeIndex != fallbackMemoryTypeIndex)
	{
		B3D_LOG(Warning, LogRenderBackend, "Vulkan: Failed to allocate {0} bytes of GPU memory for CPU-written resources (result={1}), so they are in "
			"memory type {2} instead.", sizeInBytes, (i32)result, fallbackMemoryTypeIndex);
		memoryTypeIndex = fallbackMemoryTypeIndex;
		allocateInfo.memoryTypeIndex = memoryTypeIndex;
		result = vkAllocateMemory(mLogicalDevice, &allocateInfo, gVulkanAllocator, &memory);
	}

	B3D_ASSERT(result == VK_SUCCESS);

	// A heap that fell back to a type the CPU can't map stays unmapped, so its buffers are written through staging buffers
	void* mapped = nullptr;
	const VkMemoryPropertyFlags typeFlags = memProps.memoryTypes[memoryTypeIndex].propertyFlags;
	if(createInformation.MapPersistently && (typeFlags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT) != 0)
	{
		result = vkMapMemory(mLogicalDevice, memory, 0, VK_WHOLE_SIZE, 0, &mapped);
		B3D_ASSERT(result == VK_SUCCESS);
	}

	Lock lock(mHeapPoolMutex);
	mMemoryHeapUsage[memProps.memoryTypes[memoryTypeIndex].heapIndex] += sizeInBytes;

	VulkanGpuHeap* heap = mHeapPool.Allocate();
	heap->Memory = memory;
	heap->Size = sizeInBytes;
	heap->Mapped = mapped;
	heap->MemoryTypeIndex = memoryTypeIndex;

	return heap;
}

void VulkanHeapBackend::DestroyHeap(IGpuHeap* handle)
{
	if(handle == nullptr)
		return;

	VulkanGpuHeap* heap = static_cast<VulkanGpuHeap*>(handle);

	if(heap->Memory != VK_NULL_HANDLE)
	{
		if(heap->Mapped != nullptr)
			vkUnmapMemory(mLogicalDevice, heap->Memory);

		vkFreeMemory(mLogicalDevice, heap->Memory, gVulkanAllocator);
	}

	Lock lock(mHeapPoolMutex);
	mMemoryHeapUsage[mDevice->GetMemoryProperties().memoryTypes[heap->MemoryTypeIndex].heapIndex] -= heap->Size;
	mHeapPool.Release(heap);
}

// Conformance check
B3D_STATIC_ASSERT_HEAP_BACKEND_IS_VALID(VulkanHeapBackend);
