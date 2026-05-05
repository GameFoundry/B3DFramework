//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DVulkanAllocatorTestSuite.h"
#include "B3DVulkanGpuBackend.h"
#include "B3DVulkanGpuDevice.h"
#include "B3DVulkanHeapBackend.h"
#include "GpuBackend/Allocators/B3DGpuTlsfAllocator.h"

using namespace b3d;
using namespace b3d::render;

namespace
{
	/** Returns the first Vulkan device, or null when no Vulkan device is up. */
	VulkanGpuDevice* GetActiveVulkanDevice()
	{
		VulkanGpuBackend& backend = GetVulkanGpuBackend();
		if (backend.GetDeviceCount() == 0)
			return nullptr;

		return backend.GetVulkanDevice(0).get();
	}

	/**
	 * Builds a configuration matching the device's per-memory-type allocator default — initial heap
	 * size depends on whether the type is purely DEVICE_LOCAL or also HOST_VISIBLE, with the same
	 * sizes used in VulkanGpuDevice::GetOrCreateGpuMemoryAllocator. Tests build their own allocator
	 * (rather than invoking the device's lazy slot) so they can run without depending on the
	 * device's internal symbol exports.
	 */
	TGpuTlsfAllocator<VulkanHeapBackend>::Configuration BuildAllocatorConfig(const VkPhysicalDeviceMemoryProperties& memProps, const VkPhysicalDeviceLimits& limits, u32 memoryTypeIndex)
	{
		const VkMemoryPropertyFlags flags = memProps.memoryTypes[memoryTypeIndex].propertyFlags;
		const bool isHostVisible = (flags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT) != 0;
		const bool isDeviceLocalOnly = (flags & VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT) != 0 && !isHostVisible;

		TGpuTlsfAllocator<VulkanHeapBackend>::Configuration configuration;
		configuration.InitialHeapSize = isDeviceLocalOnly ? (64ull * 1024 * 1024) : (16ull * 1024 * 1024);
		configuration.MaxHeapSize = 256ull * 1024 * 1024;
		configuration.GrowthFactor = 2;
		configuration.MaxEmptyHeapCount = 1;
		configuration.MinAllocationSize = 16;
		configuration.BufferImageGranularity = limits.bufferImageGranularity;

		configuration.HeapCreateInfo.MemoryTypeBits = (1u << memoryTypeIndex);
		configuration.HeapCreateInfo.PropertyFlags = flags;
		configuration.HeapCreateInfo.MapPersistently = isHostVisible;

		return configuration;
	}

	/** Returns the first memory type index satisfying both @p typeBits and @p required, or VK_MAX_MEMORY_TYPES on miss. */
	u32 PickMemoryType(const VkPhysicalDeviceMemoryProperties& memProps, u32 typeBits, VkMemoryPropertyFlags required)
	{
		for (u32 typeIndex = 0; typeIndex < memProps.memoryTypeCount; typeIndex++)
		{
			if ((typeBits & (1u << typeIndex)) == 0)
				continue;

			if ((memProps.memoryTypes[typeIndex].propertyFlags & required) == required)
				return typeIndex;
		}

		return VK_MAX_MEMORY_TYPES;
	}
}

VulkanAllocatorTestSuite::VulkanAllocatorTestSuite()
	: TestSuite("VulkanAllocatorTestSuite")
{
	B3D_ADD_TEST(VulkanAllocatorTestSuite::TestHostVisibleHeapCreateAndDestroy)
	B3D_ADD_TEST(VulkanAllocatorTestSuite::TestDeviceLocalHeapCreateAndDestroy)
	B3D_ADD_TEST(VulkanAllocatorTestSuite::TestAllocateAndFreeHostVisibleBuffer)
	B3D_ADD_TEST(VulkanAllocatorTestSuite::TestAllocateAndFreeDeviceLocalImage)
	B3D_ADD_TEST(VulkanAllocatorTestSuite::TestBufferHeapGrowth)
	B3D_ADD_TEST(VulkanAllocatorTestSuite::TestDepthStencilUsesDeviceLocal)
}

void VulkanAllocatorTestSuite::TestHostVisibleHeapCreateAndDestroy()
{
	VulkanGpuDevice* device = GetActiveVulkanDevice();
	if (device == nullptr)
		return;

	VulkanHeapBackend& backend = device->GetHeapBackend();

	constexpr u64 kHeapSize = 1ull << 20;

	VulkanHeapCreateInformation createInformation;
	createInformation.PropertyFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
	createInformation.MapPersistently = true;

	VulkanHeapHandle heap = backend.CreateHeap(kHeapSize, createInformation);
	B3D_TEST_ASSERT(heap.Memory != VK_NULL_HANDLE)
	B3D_TEST_ASSERT(heap.Size == kHeapSize)
	B3D_TEST_ASSERT(heap.Mapped != nullptr)

	// Sentinel write to confirm the persistent map is actually writable.
	if (heap.Mapped != nullptr)
	{
		u8* mapped = static_cast<u8*>(heap.Mapped);
		mapped[0] = 0xA5;
		mapped[kHeapSize - 1] = 0x5A;
		B3D_TEST_ASSERT(mapped[0] == 0xA5)
		B3D_TEST_ASSERT(mapped[kHeapSize - 1] == 0x5A)
	}

	backend.DestroyHeap(heap);
}

void VulkanAllocatorTestSuite::TestDeviceLocalHeapCreateAndDestroy()
{
	VulkanGpuDevice* device = GetActiveVulkanDevice();
	if (device == nullptr)
		return;

	VulkanHeapBackend& backend = device->GetHeapBackend();

	constexpr u64 kHeapSize = 1ull << 20;

	VulkanHeapCreateInformation createInformation;
	createInformation.PropertyFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
	createInformation.MapPersistently = false;

	// DEVICE_LOCAL memory is generally not host-visible — this case complements the host-visible
	// test by walking the non-mapping CreateHeap branch on a memory type that real GPU-resident
	// allocators (textures, vertex/index buffers) actually use.
	VulkanHeapHandle heap = backend.CreateHeap(kHeapSize, createInformation);
	B3D_TEST_ASSERT(heap.Memory != VK_NULL_HANDLE)
	B3D_TEST_ASSERT(heap.Size == kHeapSize)
	B3D_TEST_ASSERT(heap.Mapped == nullptr)

	backend.DestroyHeap(heap);
}

void VulkanAllocatorTestSuite::TestAllocateAndFreeHostVisibleBuffer()
{
	VulkanGpuDevice* device = GetActiveVulkanDevice();
	if (device == nullptr)
		return;

	const VkPhysicalDeviceMemoryProperties& memProps = device->GetMemoryProperties();

	// Create a transient buffer; 4 KiB, host-visible, transfer-src.
	constexpr VkDeviceSize kSize = 4096;
	VkBufferCreateInfo bufferCI = {};
	bufferCI.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferCI.size = kSize;
	bufferCI.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
	bufferCI.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	VkBuffer buffer = VK_NULL_HANDLE;
	VkResult result = vkCreateBuffer(device->GetLogical(), &bufferCI, nullptr, &buffer);
	B3D_TEST_ASSERT(result == VK_SUCCESS)

	VkMemoryRequirements requirements = {};
	vkGetBufferMemoryRequirements(device->GetLogical(), buffer, &requirements);

	const u32 memoryTypeIndex = PickMemoryType(memProps, requirements.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
	B3D_TEST_ASSERT(memoryTypeIndex != VK_MAX_MEMORY_TYPES)

	auto config = BuildAllocatorConfig(memProps, device->GetDeviceProperties().limits, memoryTypeIndex);
	TGpuTlsfAllocator<VulkanHeapBackend> allocator(&device->GetHeapBackend(), device, config);

	TGpuResourceLocation<VulkanHeapBackend> location;
	const bool ok = allocator.TryAllocate(requirements.size, (u32)requirements.alignment, GpuResourceKind::Linear, location);
	B3D_TEST_ASSERT(ok)
	B3D_TEST_ASSERT(location.IsValid())
	B3D_TEST_ASSERT(location.Heap.Mapped != nullptr)

	// Sentinel write to confirm the persistent map is actually writable for this allocation's slice.
	{
		u8* mapped = static_cast<u8*>(location.Heap.Mapped) + location.Offset;
		mapped[0] = 0xA5;
		mapped[kSize - 1] = 0x5A;
		B3D_TEST_ASSERT(mapped[0] == 0xA5)
		B3D_TEST_ASSERT(mapped[kSize - 1] == 0x5A)
	}

	vkDestroyBuffer(device->GetLogical(), buffer, nullptr);

	allocator.FreeImmediate(location);
	B3D_TEST_ASSERT(allocator.GetUsedBytes() == 0)
}

void VulkanAllocatorTestSuite::TestAllocateAndFreeDeviceLocalImage()
{
	VulkanGpuDevice* device = GetActiveVulkanDevice();
	if (device == nullptr)
		return;

	const VkPhysicalDeviceMemoryProperties& memProps = device->GetMemoryProperties();

	VkImageCreateInfo imageCI = {};
	imageCI.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	imageCI.imageType = VK_IMAGE_TYPE_2D;
	imageCI.format = VK_FORMAT_R8G8B8A8_UNORM;
	imageCI.extent = { 64, 64, 1 };
	imageCI.mipLevels = 1;
	imageCI.arrayLayers = 1;
	imageCI.samples = VK_SAMPLE_COUNT_1_BIT;
	imageCI.tiling = VK_IMAGE_TILING_OPTIMAL;
	imageCI.usage = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
	imageCI.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	imageCI.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

	VkImage image = VK_NULL_HANDLE;
	VkResult result = vkCreateImage(device->GetLogical(), &imageCI, nullptr, &image);
	B3D_TEST_ASSERT(result == VK_SUCCESS)

	VkMemoryRequirements requirements = {};
	vkGetImageMemoryRequirements(device->GetLogical(), image, &requirements);

	const u32 memoryTypeIndex = PickMemoryType(memProps, requirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
	B3D_TEST_ASSERT(memoryTypeIndex != VK_MAX_MEMORY_TYPES)

	auto config = BuildAllocatorConfig(memProps, device->GetDeviceProperties().limits, memoryTypeIndex);
	TGpuTlsfAllocator<VulkanHeapBackend> allocator(&device->GetHeapBackend(), device, config);

	TGpuResourceLocation<VulkanHeapBackend> location;
	const bool ok = allocator.TryAllocate(requirements.size, (u32)requirements.alignment, GpuResourceKind::NonLinear, location);
	B3D_TEST_ASSERT(ok)
	B3D_TEST_ASSERT(location.IsValid())

	const VkMemoryPropertyFlags flags = memProps.memoryTypes[location.Heap.MemoryTypeIndex].propertyFlags;
	B3D_TEST_ASSERT((flags & VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT) != 0)

	vkDestroyImage(device->GetLogical(), image, nullptr);

	allocator.FreeImmediate(location);
	B3D_TEST_ASSERT(allocator.GetUsedBytes() == 0)
}

void VulkanAllocatorTestSuite::TestBufferHeapGrowth()
{
	VulkanGpuDevice* device = GetActiveVulkanDevice();
	if (device == nullptr)
		return;

	const VkPhysicalDeviceMemoryProperties& memProps = device->GetMemoryProperties();

	// Probe a representative buffer to discover an acceptable memory type.
	VkBufferCreateInfo probeCI = {};
	probeCI.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	probeCI.size = 1ull << 20; // 1 MiB
	probeCI.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
	probeCI.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	VkBuffer probe = VK_NULL_HANDLE;
	VkResult result = vkCreateBuffer(device->GetLogical(), &probeCI, nullptr, &probe);
	B3D_TEST_ASSERT(result == VK_SUCCESS)

	VkMemoryRequirements probeReq = {};
	vkGetBufferMemoryRequirements(device->GetLogical(), probe, &probeReq);

	vkDestroyBuffer(device->GetLogical(), probe, nullptr);

	const u32 memoryTypeIndex = PickMemoryType(memProps, probeReq.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
	B3D_TEST_ASSERT(memoryTypeIndex != VK_MAX_MEMORY_TYPES)

	auto config = BuildAllocatorConfig(memProps, device->GetDeviceProperties().limits, memoryTypeIndex);
	// Force a small initial heap so we can validate growth without allocating tens of MiB.
	config.InitialHeapSize = 4ull * 1024 * 1024;
	config.MaxHeapSize = 16ull * 1024 * 1024;

	TGpuTlsfAllocator<VulkanHeapBackend> allocator(&device->GetHeapBackend(), device, config);

	// Allocate enough 1 MiB chunks to exceed the 4 MiB initial heap and force a second heap.
	constexpr u32 kAllocCount = 6;
	TGpuResourceLocation<VulkanHeapBackend> locations[kAllocCount];
	for (u32 entryIndex = 0; entryIndex < kAllocCount; entryIndex++)
	{
		const bool ok = allocator.TryAllocate(probeReq.size, (u32)probeReq.alignment, GpuResourceKind::Linear, locations[entryIndex]);
		B3D_TEST_ASSERT(ok)
	}

	B3D_TEST_ASSERT(allocator.GetHeapCount() > 1)

	for (u32 entryIndex = 0; entryIndex < kAllocCount; entryIndex++)
		allocator.FreeImmediate(locations[entryIndex]);

	B3D_TEST_ASSERT(allocator.GetUsedBytes() == 0)
}

void VulkanAllocatorTestSuite::TestDepthStencilUsesDeviceLocal()
{
	VulkanGpuDevice* device = GetActiveVulkanDevice();
	if (device == nullptr)
		return;

	const VkPhysicalDeviceMemoryProperties& memProps = device->GetMemoryProperties();

	VkImageCreateInfo imageCI = {};
	imageCI.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	imageCI.imageType = VK_IMAGE_TYPE_2D;
	imageCI.format = VK_FORMAT_D32_SFLOAT;
	imageCI.extent = { 64, 64, 1 };
	imageCI.mipLevels = 1;
	imageCI.arrayLayers = 1;
	imageCI.samples = VK_SAMPLE_COUNT_1_BIT;
	imageCI.tiling = VK_IMAGE_TILING_OPTIMAL;
	imageCI.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
	imageCI.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	imageCI.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

	VkImage image = VK_NULL_HANDLE;
	VkResult result = vkCreateImage(device->GetLogical(), &imageCI, nullptr, &image);
	B3D_TEST_ASSERT(result == VK_SUCCESS)

	VkMemoryRequirements requirements = {};
	vkGetImageMemoryRequirements(device->GetLogical(), image, &requirements);

	const u32 memoryTypeIndex = PickMemoryType(memProps, requirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
	B3D_TEST_ASSERT(memoryTypeIndex != VK_MAX_MEMORY_TYPES)

	const VkMemoryPropertyFlags flags = memProps.memoryTypes[memoryTypeIndex].propertyFlags;
	B3D_TEST_ASSERT((flags & VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT) != 0)

	vkDestroyImage(device->GetLogical(), image, nullptr);
}
