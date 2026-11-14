//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "Testing/B3DTestSuite.h"

namespace b3d
{
	/**
	 * End-to-end self-tests for the per-memory-type TGpuTlsfAllocator pool wired into VulkanGpuDevice
	 * and the underlying VulkanHeapBackend. Tests bail out gracefully when no Vulkan device is available
	 * so the suite stays runnable on headless CI without a Vulkan-capable GPU.
	 */
	class VulkanAllocatorTestSuite : public TestSuite
	{
	public:
		VulkanAllocatorTestSuite();

	private:
		/** A 1 MiB host-visible heap can be allocated, persistently mapped, written to, and freed. */
		void TestHostVisibleHeapCreateAndDestroy();

		/** A 1 MiB device-local heap can be allocated and freed (no mapping, mirrors texture / VBO usage). */
		void TestDeviceLocalHeapCreateAndDestroy();

		/** Allocates a host-visible buffer, asserts the persistent map is writable, and frees it cleanly. */
		void TestAllocateAndFreeHostVisibleBuffer();

		/** Allocates a DEVICE_LOCAL-only image (depth-stencil shape), asserts no map pointer, and frees. */
		void TestAllocateAndFreeDeviceLocalImage();

		/** Allocates enough small buffers to overflow the initial heap and asserts a second heap is grown. */
		void TestBufferHeapGrowth();

		/** Asserts a depth-stencil-shaped image lands in a memory type that includes DEVICE_LOCAL. */
		void TestDepthStencilUsesDeviceLocal();

		/** Asserts buffers and optimally tiled images use different persistent allocators and heaps. */
		void TestBuffersAndOptimalImagesUseSeparatePersistentHeaps();

		/**
		 * Asserts a buffer in GpuBufferMemory::Mapped memory is host-visible, and device-local exactly when the device can map its GPU memory
		 * or memory is unified. With Resizable BAR it is then in the largest local heap.
		 */
		void TestCpuWrittenBufferPlacement();

		/**
		 * Asserts buffers in GpuBufferMemory::Device memory are device-local, and host-visible and mapped exactly when the device can map its
		 * GPU memory, which only Resizable BAR and some unified memory devices can. Such mapped buffers use other allocators than
		 * GpuBufferMemory::Mapped buffers of the same Vulkan memory type, since their heaps fall back to other memory. Buffers the GPU writes
		 * are device-local, and GpuBufferMemory::MappedCached buffers host-visible, mapped, and CPU-cached where the device has such memory.
		 */
		void TestGpuReadBufferPlacement();
	};
} // namespace b3d
