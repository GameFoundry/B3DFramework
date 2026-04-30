//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DVulkanHeapBackendTestSuite.h"
#include "B3DVulkanGpuBackend.h"
#include "B3DVulkanGpuDevice.h"
#include "B3DVulkanHeapBackend.h"

using namespace b3d;
using namespace b3d::render;

namespace
{
	/**
	 * Returns the first Vulkan device, or nullptr when no Vulkan device is up. Tests bail out
	 * gracefully on nullptr to keep the suite usable on machines where the Vulkan plugin
	 * couldn't bring a device up (e.g. headless CI without a Vulkan-capable GPU).
	 */
	VulkanGpuDevice* GetActiveVulkanDevice()
	{
		VulkanGpuBackend& backend = GetVulkanGpuBackend();
		if (backend.GetDeviceCount() == 0)
			return nullptr;

		return backend.GetVulkanDevice(0).get();
	}
}

VulkanHeapBackendTestSuite::VulkanHeapBackendTestSuite()
	: TestSuite("VulkanHeapBackendTestSuite")
{
	B3D_ADD_TEST(VulkanHeapBackendTestSuite::TestHostVisibleHeapCreateAndDestroy)
	B3D_ADD_TEST(VulkanHeapBackendTestSuite::TestDeviceLocalHeapCreateAndDestroy)
}

void VulkanHeapBackendTestSuite::TestHostVisibleHeapCreateAndDestroy()
{
	VulkanGpuDevice* device = GetActiveVulkanDevice();
	if (device == nullptr)
		return;

	VulkanHeapBackend& backend = device->GetHeapBackend();

	constexpr u64 kHeapSize = 1ull << 20;

	VulkanHeapCreateInformation desc;
	desc.PropertyFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
	desc.MapPersistently = true;

	VulkanHeapHandle heap = backend.CreateHeap(kHeapSize, desc);
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

void VulkanHeapBackendTestSuite::TestDeviceLocalHeapCreateAndDestroy()
{
	VulkanGpuDevice* device = GetActiveVulkanDevice();
	if (device == nullptr)
		return;

	VulkanHeapBackend& backend = device->GetHeapBackend();

	constexpr u64 kHeapSize = 1ull << 20;

	VulkanHeapCreateInformation desc;
	desc.PropertyFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
	desc.MapPersistently = false;

	// DEVICE_LOCAL memory is generally not host-visible — this case complements the host-visible
	// test by walking the non-mapping CreateHeap branch on a memory type that real GPU-resident
	// allocators (textures, vertex/index buffers) actually use.
	VulkanHeapHandle heap = backend.CreateHeap(kHeapSize, desc);
	B3D_TEST_ASSERT(heap.Memory != VK_NULL_HANDLE)
	B3D_TEST_ASSERT(heap.Size == kHeapSize)
	B3D_TEST_ASSERT(heap.Mapped == nullptr)

	backend.DestroyHeap(heap);
}
