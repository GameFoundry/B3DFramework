//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsVulkanPrerequisites.h"
#include "RenderAPI/BsRenderAPI.h"
#include "Managers/BsVulkanDescriptorManager.h"

namespace bs
{
	namespace ct
	{
		/** @addtogroup Vulkan
		 *  @{
		 */

		/** Contains format describing a Vulkan surface. */
		struct SurfaceFormat
		{
			VkFormat ColorFormat;
			VkFormat DepthFormat;
			VkColorSpaceKHR ColorSpace;
		};

		/** Represents a single GPU device usable by Vulkan. */
		class VulkanDevice
		{
		public:
			VulkanDevice(VkPhysicalDevice device, u32 deviceIdx);
			~VulkanDevice();

			/** Returns an object describing the physical properties of the device. */
			VkPhysicalDevice GetPhysical() const { return mPhysicalDevice; }

			/** Returns an object describing the logical properties of the device. */
			VkDevice GetLogical() const { return mLogicalDevice; }

			/** Returns true if the device is one of the primary GPU's. */
			bool IsPrimary() const { return mIsPrimary; }

			/** Returns the unique index of the device. */
			u32 GetIndex() const { return mDeviceIdx; }

			/** Blocks the calling thread until all operations on the device finish. */
			void WaitIdle();

			/**
			 * Checks if any of the active command buffers finished executing on the device and updates their states
			 * accordingly.
			 */
			void RefreshStates(bool forceWait = false);

			/** Returns a set of properties describing the physical device. */
			const VkPhysicalDeviceProperties& GetDeviceProperties() const { return mDeviceProperties; }

			/** Returns a set of features that the application can use to check if a specific feature is supported. */
			const VkPhysicalDeviceFeatures& GetDeviceFeatures() const { return mDeviceFeatures; }

			/** Returns a set of properties describing the memory of the physical device. */
			const VkPhysicalDeviceMemoryProperties& GetMemoryProperties() const { return mMemoryProperties; }

			/** Returns the number of queue supported on the device, per type. */
			u32 GetNumQueues(GpuQueueType type) const { return (u32)mQueueInfos[(int)type].Queues.size(); }

			/** Returns queue of the specified type at the specified index. Index must be in range [0, getNumQueues()). */
			VulkanQueue* GetQueue(GpuQueueType type, u32 idx) const { return mQueueInfos[(int)type].Queues[idx]; }

			/**
			 * Returns index of the queue family for the specified queue type. Returns -1 if no queues for the specified type
			 * exist. There will always be a queue family for the graphics type.
			 */
			u32 GetQueueFamily(GpuQueueType type) const { return mQueueInfos[(int)type].FamilyIndex; }

			/**
			 * Fills out a mask that has bits set for every queue index that maps to the same physical queue as the provided
			 * index. This is useful as different queue indices will sometimes map to the same physical queue.
			 */
			u32 GetQueueMask(GpuQueueType type, u32 queueIdx) const;

			/** Returns the best matching surface format according to the provided parameters. */
			SurfaceFormat GetSurfaceFormat(const VkSurfaceKHR& surface, bool gamma) const;

			/** Returns a pool that can be used for allocating command buffers for all queues on this device. */
			VulkanCmdBufferPool& GetCmdBufferPool() const { return *mCommandBufferPool; }

			/** Returns a pool that can be used for allocating queries on this device. */
			VulkanQueryPool& GetQueryPool() const { return *mQueryPool; }

			/** Returns a manager that can be used for allocating descriptor layouts and sets. */
			VulkanDescriptorManager& GetDescriptorManager() const { return *mDescriptorManager; }

			/** Returns a manager that can be used for allocating Vulkan objects wrapped as managed resources. */
			VulkanResourceManager& GetResourceManager() const { return *mResourceManager; }

			/**
			 * Allocates memory for the provided image, and binds it to the image. Returns null if it cannot find memory
			 * with the specified flags.
			 */
			VmaAllocation AllocateMemory(VkImage image, VmaMemoryUsage usage);

			/**
			 * Allocates memory for the provided buffer, and binds it to the buffer. Returns null if it cannot find memory
			 * with the specified flags.
			 */
			VmaAllocation AllocateMemory(VkBuffer buffer, VmaMemoryUsage usage);

			/** Frees a previously allocated block of memory. */
			void FreeMemory(VmaAllocation allocation);

			/**
			 * Returns a pointer to internal buffer memory. The allocation must be in host visible memory. The GPU
			 * must not be currently writing to the buffer. If the allocation is done in non-coherent memory, you need
			 * to call InvalidateMemory() on the touched memory range before accessing it. Must be followed by
			 * UnmapMemory() once done using the buffer.
			 */
			void* MapMemory(const VmaAllocation& allocation) const;

			/** Unmaps the memory mapped via MapMemory(). */
			void UnmapMemory(const VmaAllocation& allocation) const;

			/**
			 * Invalidates a memory range of the provided allocation. This ensures any data written by the GPU will be visible
			 * to the CPU. Only relevant for non-coherent memory.
			 */
			void InvalidateMemory(const VmaAllocation& allocation, VkDeviceSize offset = 0 , VkDeviceSize size = VK_WHOLE_SIZE) const;

			/**
			 * Flushes a memory range of the provided allocation. This ensures any data written by the CPU will be visible
			 * to the GPU. Only relevant for non-coherent memory.
			 */
			void FlushMemory(const VmaAllocation& allocation, VkDeviceSize offset = 0, VkDeviceSize size = VK_WHOLE_SIZE) const;

			/** Returns the device memory block and offset into the block for a specific memory allocation. */
			void GetAllocationInfo(VmaAllocation allocation, VkDeviceMemory& memory, VkDeviceSize& offset);

		private:
			friend class VulkanRenderAPI;

			/** Attempts to find a memory type that matches the requirements bits and the requested flags. */
			uint32_t FindMemoryType(uint32_t requirementBits, VkMemoryPropertyFlags wantedFlags);

			/** Marks the device as a primary device. */
			void SetIsPrimary() { mIsPrimary = true; }

			/** Changes the index of the device in the global device list. */
			void SetIndex(u32 index) { mDeviceIdx = index; }

			VkPhysicalDevice mPhysicalDevice;
			VkDevice mLogicalDevice = nullptr;
			bool mIsPrimary = false;
			u32 mDeviceIdx;

			VulkanCmdBufferPool* mCommandBufferPool;
			VulkanQueryPool* mQueryPool;
			VulkanDescriptorManager* mDescriptorManager;
			VulkanResourceManager* mResourceManager;
			VmaAllocator mAllocator;

			VkPhysicalDeviceProperties mDeviceProperties;
			VkPhysicalDeviceFeatures mDeviceFeatures;
			VkPhysicalDeviceMemoryProperties mMemoryProperties;

			/** Contains data about a set of queues of a specific type. */
			struct QueueInfo
			{
				u32 FamilyIndex;
				Vector<VulkanQueue*> Queues;
			};

			QueueInfo mQueueInfos[GQT_COUNT];
		};

		/** @} */
	} // namespace ct
} // namespace bs
