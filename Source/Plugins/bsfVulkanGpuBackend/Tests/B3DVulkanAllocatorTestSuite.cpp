//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DVulkanAllocatorTestSuite.h"
#include "B3DVulkanGpuBackend.h"
#include "B3DVulkanGpuDevice.h"
#include "B3DVulkanHeapBackend.h"
#include "GpuBackend/Allocators/B3DGpuTlsfAllocator.h"
#include "GpuBackend/B3DGpuBuffer.h"
#include "GpuBackend/B3DGpuDeviceCapabilities.h"
#include "Image/B3DTexture.h"
#include "Renderer/B3DRenderer.h"

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
	TGpuTlsfAllocator<VulkanHeapBackend>::Configuration BuildAllocatorConfig(const VkPhysicalDeviceMemoryProperties& memProps, u32 memoryTypeIndex)
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
	B3D_ADD_TEST(VulkanAllocatorTestSuite::TestBuffersAndOptimalImagesUseSeparatePersistentHeaps)
	B3D_ADD_TEST(VulkanAllocatorTestSuite::TestCpuWrittenBufferPlacement)
	B3D_ADD_TEST(VulkanAllocatorTestSuite::TestGpuReadBufferPlacement)
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

	IGpuHeap* heapHandle = backend.CreateHeap(kHeapSize, createInformation);
	const VulkanGpuHeap& heap = ToVulkanGpuHeap(heapHandle);
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

	backend.DestroyHeap(heapHandle);
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
	IGpuHeap* heapHandle = backend.CreateHeap(kHeapSize, createInformation);
	const VulkanGpuHeap& heap = ToVulkanGpuHeap(heapHandle);
	B3D_TEST_ASSERT(heap.Memory != VK_NULL_HANDLE)
	B3D_TEST_ASSERT(heap.Size == kHeapSize)
	B3D_TEST_ASSERT(heap.Mapped == nullptr)

	backend.DestroyHeap(heapHandle);
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

	auto config = BuildAllocatorConfig(memProps, memoryTypeIndex);
	TGpuTlsfAllocator<VulkanHeapBackend> allocator(&device->GetHeapBackend(), &render::GetRenderer()->GetFrameCompletionTracker(), config);

	GpuAllocation allocation;
	const bool ok = allocator.TryAllocate(requirements.size, (u32)requirements.alignment, allocation);
	B3D_TEST_ASSERT(ok)
	B3D_TEST_ASSERT(allocation.IsOwned())
	B3D_TEST_ASSERT(ToVulkanGpuHeap(allocation.Heap).Mapped != nullptr)

	// Sentinel write to confirm the persistent map is actually writable for this allocation's slice.
	{
		u8* mapped = static_cast<u8*>(ToVulkanGpuHeap(allocation.Heap).Mapped) + allocation.Offset;
		mapped[0] = 0xA5;
		mapped[kSize - 1] = 0x5A;
		B3D_TEST_ASSERT(mapped[0] == 0xA5)
		B3D_TEST_ASSERT(mapped[kSize - 1] == 0x5A)
	}

	vkDestroyBuffer(device->GetLogical(), buffer, nullptr);

	allocator.FreeAndReclaim(allocation);
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

	auto config = BuildAllocatorConfig(memProps, memoryTypeIndex);
	TGpuTlsfAllocator<VulkanHeapBackend> allocator(&device->GetHeapBackend(), &render::GetRenderer()->GetFrameCompletionTracker(), config);

	GpuAllocation allocation;
	const bool ok = allocator.TryAllocate(requirements.size, (u32)requirements.alignment, allocation);
	B3D_TEST_ASSERT(ok)
	B3D_TEST_ASSERT(allocation.IsOwned())

	const VkMemoryPropertyFlags flags = memProps.memoryTypes[ToVulkanGpuHeap(allocation.Heap).MemoryTypeIndex].propertyFlags;
	B3D_TEST_ASSERT((flags & VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT) != 0)

	vkDestroyImage(device->GetLogical(), image, nullptr);

	allocator.FreeAndReclaim(allocation);
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

	auto config = BuildAllocatorConfig(memProps, memoryTypeIndex);
	// Force a small initial heap so we can validate growth without allocating tens of MiB.
	config.InitialHeapSize = 4ull * 1024 * 1024;
	config.MaxHeapSize = 16ull * 1024 * 1024;

	TGpuTlsfAllocator<VulkanHeapBackend> allocator(&device->GetHeapBackend(), &render::GetRenderer()->GetFrameCompletionTracker(), config);

	// Allocate enough 1 MiB chunks to exceed the 4 MiB initial heap and force a second heap.
	constexpr u32 kAllocCount = 6;
	GpuAllocation allocations[kAllocCount];
	for (u32 entryIndex = 0; entryIndex < kAllocCount; entryIndex++)
	{
		const bool ok = allocator.TryAllocate(probeReq.size, (u32)probeReq.alignment, allocations[entryIndex]);
		B3D_TEST_ASSERT(ok)
	}

	B3D_TEST_ASSERT(allocator.GetHeapCount() > 1)

	for (u32 entryIndex = 0; entryIndex < kAllocCount; entryIndex++)
		allocator.FreeAndReclaim(allocations[entryIndex]);

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

void VulkanAllocatorTestSuite::TestBuffersAndOptimalImagesUseSeparatePersistentHeaps()
{
	VulkanGpuDevice* device = GetActiveVulkanDevice();
	if (device == nullptr)
		return;

	TextureCreateInformation textureInformation;
	textureInformation.Name = "Separate heaps test";
	textureInformation.Width = 64;
	textureInformation.Height = 64;
	textureInformation.Format = PF_RGBA8;
	textureInformation.Usage = TextureUsageFlag::Default;

	const GpuMemoryRequirements bufferRequirements = device->GetMemoryRequirements(GpuBufferCreateInformation::CreateStructuredStorage(sizeof(u32), 64));
	const GpuMemoryRequirements textureRequirements = device->GetMemoryRequirements(textureInformation);
	B3D_TEST_ASSERT(bufferRequirements.MemoryType != textureRequirements.MemoryType)

	IGpuAllocator& bufferAllocator = device->GetPersistentAllocator(bufferRequirements.MemoryType);
	IGpuAllocator& textureAllocator = device->GetPersistentAllocator(textureRequirements.MemoryType);
	B3D_TEST_ASSERT(&bufferAllocator != &textureAllocator)

	GpuAllocation bufferAllocation;
	GpuAllocation textureAllocation;
	B3D_TEST_ASSERT(bufferAllocator.TryAllocate(bufferRequirements.Size, (u32)bufferRequirements.Alignment, bufferAllocation))
	B3D_TEST_ASSERT(textureAllocator.TryAllocate(textureRequirements.Size, (u32)textureRequirements.Alignment, textureAllocation))
	B3D_TEST_ASSERT(bufferAllocation.Heap != textureAllocation.Heap)

	bufferAllocator.FreeAndReclaim(bufferAllocation);
	textureAllocator.FreeAndReclaim(textureAllocation);
}

void VulkanAllocatorTestSuite::TestCpuWrittenBufferPlacement()
{
	VulkanGpuDevice* device = GetActiveVulkanDevice();
	if (device == nullptr)
		return;

	const GpuDeviceCapabilities& capabilities = device->GetCapabilities();
	const VkPhysicalDeviceMemoryProperties& memoryProperties = device->GetMemoryProperties();

	const GpuMemoryRequirements requirements = device->GetMemoryRequirements(GpuBufferCreateInformation::CreateUniform(256));
	const VkMemoryType& memoryType = memoryProperties.memoryTypes[VulkanMemoryType::Decode(requirements.MemoryType).Index];
	B3D_TEST_ASSERT((memoryType.propertyFlags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT) != 0)

	const bool isDeviceLocal = (memoryType.propertyFlags & VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT) != 0;
	const bool isUnified = capabilities.HasCapability(RSC_UNIFIED_MEMORY);
	B3D_TEST_ASSERT(isDeviceLocal == (device->HasFullSpeedMappableGpuMemory() || isUnified))

	// Mappable GPU memory of a discrete GPU is Resizable BAR, which covers all of it. Integrated GPUs can keep their device-local,
	// host-visible memory in a heap smaller than the largest one.
	B3D_TEST_ASSERT(isUnified || device->HasFullSpeedMappableGpuMemory() == capabilities.HasCapability(RSC_RESIZABLE_BAR))
	if (capabilities.HasCapability(RSC_RESIZABLE_BAR))
	{
		u64 largestLocalHeapSize = 0;
		for (u32 heapIndex = 0; heapIndex < memoryProperties.memoryHeapCount; heapIndex++)
		{
			if ((memoryProperties.memoryHeaps[heapIndex].flags & VK_MEMORY_HEAP_DEVICE_LOCAL_BIT) != 0)
				largestLocalHeapSize = std::max(memoryProperties.memoryHeaps[heapIndex].size, largestLocalHeapSize);
		}

		B3D_TEST_ASSERT(memoryProperties.memoryHeaps[memoryType.heapIndex].size == largestLocalHeapSize)
	}
}

void VulkanAllocatorTestSuite::TestGpuReadBufferPlacement()
{
	VulkanGpuDevice* device = GetActiveVulkanDevice();
	if (device == nullptr)
		return;

	const GpuDeviceCapabilities& capabilities = device->GetCapabilities();
	const bool hasFullSpeedMappableMemory = device->HasFullSpeedMappableGpuMemory();
	B3D_TEST_ASSERT(!hasFullSpeedMappableMemory || capabilities.HasCapability(RSC_UNIFIED_MEMORY) || capabilities.HasCapability(RSC_RESIZABLE_BAR))

	const VkPhysicalDeviceMemoryProperties& memoryProperties = device->GetMemoryProperties();
	const auto fnGetMemoryType = [device](GpuBufferMemory memory, GpuBufferFlags flags)
	{
		return VulkanMemoryType::Decode(device->GetMemoryRequirements(GpuBufferCreateInformation::CreateStructuredStorage(sizeof(u32), 64, flags, memory)).MemoryType);
	};

	const auto fnGetMemoryTypeFlags = [&memoryProperties](const VulkanMemoryType& memoryType)
	{
		return memoryProperties.memoryTypes[memoryType.Index].propertyFlags;
	};

	// Buffers in device memory are in mapped GPU memory where the device can map it
	const VulkanMemoryType deviceMemoryType = fnGetMemoryType(GpuBufferMemory::Device, GpuBufferFlag::None);
	const VkMemoryPropertyFlags deviceFlags = fnGetMemoryTypeFlags(deviceMemoryType);
	B3D_TEST_ASSERT((deviceFlags & VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT) != 0)
	B3D_TEST_ASSERT(!hasFullSpeedMappableMemory || (deviceFlags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT) != 0)

	const TShared<render::GpuBuffer> deviceBuffer = device->CreateGpuBuffer(GpuBufferCreateInformation::CreateStructuredStorage(sizeof(u32), 64));
	B3D_TEST_ASSERT(deviceBuffer != nullptr && (deviceBuffer->GetMappedMemory() != nullptr) == ((deviceFlags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT) != 0))

	// Mapped buffers in device memory fall back to unmapped GPU memory, and buffers in mapped memory to system memory, where the device
	// has such memory
	if(hasFullSpeedMappableMemory)
	{
		const VulkanMemoryType mappedMemoryType = fnGetMemoryType(GpuBufferMemory::Mapped, GpuBufferFlag::None);
		B3D_TEST_ASSERT(deviceMemoryType.Index == mappedMemoryType.Index)

		if(deviceMemoryType.FallbackIndex != VK_MAX_MEMORY_TYPES)
		{
			const VkMemoryPropertyFlags fallbackFlags = memoryProperties.memoryTypes[deviceMemoryType.FallbackIndex].propertyFlags;
			B3D_TEST_ASSERT((fallbackFlags & VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT) != 0 && (fallbackFlags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT) == 0)
		}

		if(mappedMemoryType.FallbackIndex != VK_MAX_MEMORY_TYPES)
		{
			const VkMemoryPropertyFlags fallbackFlags = memoryProperties.memoryTypes[mappedMemoryType.FallbackIndex].propertyFlags;
			B3D_TEST_ASSERT((fallbackFlags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT) != 0 && (fallbackFlags & VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT) == 0)
		}
	}

	// Buffers the GPU writes are in GPU memory
	B3D_TEST_ASSERT((fnGetMemoryTypeFlags(fnGetMemoryType(GpuBufferMemory::Device, GpuBufferFlag::GpuWrite)) & VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT) != 0)

	// Buffers in cached memory are host-visible and mapped, and CPU-cached where the device has such memory
	const VkMemoryPropertyFlags readFlags = fnGetMemoryTypeFlags(fnGetMemoryType(GpuBufferMemory::MappedCached, GpuBufferFlag::None));
	B3D_TEST_ASSERT((readFlags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT) != 0)

	const TShared<render::GpuBuffer> cachedBuffer = device->CreateGpuBuffer(GpuBufferCreateInformation::CreateStructuredStorage(sizeof(u32), 64,
		GpuBufferFlag::None, GpuBufferMemory::MappedCached));
	B3D_TEST_ASSERT(cachedBuffer != nullptr && cachedBuffer->GetMappedMemory() != nullptr)

	bool hasCachedMemory = false;
	for (u32 typeIndex = 0; typeIndex < memoryProperties.memoryTypeCount; typeIndex++)
		hasCachedMemory |= (memoryProperties.memoryTypes[typeIndex].propertyFlags & VK_MEMORY_PROPERTY_HOST_CACHED_BIT) != 0;

	B3D_TEST_ASSERT(!hasCachedMemory || (readFlags & VK_MEMORY_PROPERTY_HOST_CACHED_BIT) != 0)
}
