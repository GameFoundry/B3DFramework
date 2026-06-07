//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DVulkanPrerequisites.h"
#include "GpuBackend/Allocators/B3DGpuAllocator.h"
#include "Utility/B3DPool.h"
#include "Threading/B3DThreading.h"

namespace b3d
{
	namespace render
	{
		class VulkanGpuDevice;
	}

	/** @addtogroup Vulkan
	 *  @{
	 */

	/** References a Vulkan memory heap, as returned by VulkanHeapBackend. */
	struct VulkanGpuHeap : IGpuHeap
	{
		VkDeviceMemory Memory = VK_NULL_HANDLE; /**< Backing device memory. Resources are bound at allocator-supplied offsets. */
		VkDeviceSize Size = 0; /**< Total heap size in bytes. */
		void* Mapped = nullptr; /**< Persistent CPU map; nullptr unless @c VulkanHeapCreateInformation::MapPersistently was set. */
		u32 MemoryTypeIndex = 0; /**< Index into VkPhysicalDeviceMemoryProperties::memoryTypes; recorded for introspection. */
	};

	/** Downcasts an opaque engine heap handle to the concrete Vulkan heap it must refer to. */
	inline VulkanGpuHeap& ToVulkanGpuHeap(IGpuHeap* heap)
	{
		B3D_ASSERT(heap != nullptr);
		return *static_cast<VulkanGpuHeap*>(heap);
	}

	/** Initializer struct for VulkanHeapBackend::CreateHeap. */
	struct VulkanHeapCreateInformation
	{
		u32 MemoryTypeBits = ~0u; /**< Bitmask of memory-type indices that can satisfy this heap. */
		VkMemoryPropertyFlags PropertyFlags = 0; /**< Required Vulkan memory-property flags. */
		bool MapPersistently = false; /**< If true, the heap is mapped until it is destroyed. */
	};

	/** Vulkan implementation of the GpuHeapBackend trait. */
	class B3D_EXPORT VulkanHeapBackend
	{
	public:
		using HeapHandle = IGpuHeap*;
		using HeapCreateInformation = VulkanHeapCreateInformation;

		explicit VulkanHeapBackend(render::VulkanGpuDevice& device);
		~VulkanHeapBackend();

		VulkanHeapBackend(const VulkanHeapBackend&) = delete;
		VulkanHeapBackend& operator=(const VulkanHeapBackend&) = delete;

		/** @name GpuHeapBackend trait surface.
		 *  @{
		 */

		/**
		 * Allocates a backing heap of @p sizeInBytes bytes according to @p createInformation. Returns a
		 * stable VulkanGpuHeap pointer (as IGpuHeap*) minted from the backend's pool.
		 */
		HeapHandle CreateHeap(u64 sizeInBytes, const HeapCreateInformation& createInformation);

		/** Frees the device memory and returns the heap object to the pool. */
		void DestroyHeap(HeapHandle handle);

		/** @} */

	private:
		render::VulkanGpuDevice* mDevice = nullptr;
		VkDevice mLogicalDevice = VK_NULL_HANDLE;

		/** Pool of heap objects with stable addresses. Guarded by mHeapPoolMutex — CreateHeap/DestroyHeap are called concurrently from per-memory-type allocators. */
		TPool<VulkanGpuHeap> mHeapPool;
		Mutex mHeapPoolMutex;
	};

	/** @} */
} // namespace b3d
