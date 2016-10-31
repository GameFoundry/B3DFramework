//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsVulkanPrerequisites.h"
#include "BsRenderAPI.h"
#include "BsVulkanDescriptorManager.h"

namespace BansheeEngine
{
	/** @addtogroup Vulkan
	 *  @{
	 */

	/** Represents a single GPU device usable by Vulkan. */
	class VulkanDevice
	{
	public:
		VulkanDevice(VkPhysicalDevice device);
		~VulkanDevice();

		/** Returns an object describing the physical properties of the device. */
		VkPhysicalDevice getPhysical() const { return mPhysicalDevice; }

		/** Returns an object describing the logical properties of the device. */
		VkDevice getLogical() const { return mLogicalDevice; }

		/** Returns a set of properties describing the physical device. */
		const VkPhysicalDeviceProperties& getDeviceProperties() const { return mDeviceProperties; }

		/** Returns a set of features that the application can use to check if a specific feature is supported. */
		const VkPhysicalDeviceFeatures& getDeviceFeatures() const { return mDeviceFeatures; }

		/** Returns a set of properties describing the memory of the physical device. */
		const VkPhysicalDeviceMemoryProperties& getMemoryProperties() const { return mMemoryProperties; }

		/** Returns the number of queue supported on the device, per type. */
		UINT32 getNumQueues(GpuQueueType type) const { return (UINT32)mQueueInfos[(int)type].queues.size(); }

		/** Returns queue of the specified type at the specified index. Index must be in range [0, getNumQueues()). */
		VulkanQueue* getQueue(GpuQueueType type, UINT32 idx) const { return mQueueInfos[(int)type].queues[idx]; }

		/** 
		 * Returns index of the queue family for the specified queue type. Returns -1 if no queues for the specified type 
		 * exist. There will always be a queue family for the graphics type.
		 */
		UINT32 getQueueFamily(GpuQueueType type) const { return mQueueInfos[(int)type].familyIdx; }

		/** Returns a pool that can be used for allocating command buffers for all queues on this device. */
		VulkanCmdBufferPool& getCmdBufferPool() const { return *mCommandBufferPool; }

		/** Returns a manager that can be used for allocating descriptor layouts and sets. */
		VulkanDescriptorManager& getDescriptorManager() const { return *mDescriptorManager; }

		/** Returns a manager that can be used for allocating Vulkan objects wrapped as managed resources. */
		VulkanResourceManager& getResourceManager() const { return *mResourceManager; }

		/** 
		 * Allocates memory for the provided image, and binds it to the image. Returns null if it cannot find memory
		 * with the specified flags.
		 */
		VkDeviceMemory allocateMemory(VkImage image, VkMemoryPropertyFlags flags);

		/** 
		 * Allocates memory for the provided buffer, and binds it to the buffer. Returns null if it cannot find memory
		 * with the specified flags.
		 */
		VkDeviceMemory allocateMemory(VkBuffer buffer, VkMemoryPropertyFlags flags);

		/** 
		 * Allocates a block of memory according to the provided memory requirements. Returns null if it cannot find memory
		 * with the specified flags. 
		 */
		VkDeviceMemory allocateMemory(const VkMemoryRequirements& reqs, VkMemoryPropertyFlags flags);

		/** Frees a previously allocated block of memory. */
		void freeMemory(VkDeviceMemory memory);

	private:
		/** Attempts to find a memory type that matches the requirements bits and the requested flags. */
		uint32_t findMemoryType(uint32_t requirementBits, VkMemoryPropertyFlags wantedFlags);

		VkPhysicalDevice mPhysicalDevice;
		VkDevice mLogicalDevice;

		VulkanCmdBufferPool* mCommandBufferPool;
		VulkanDescriptorManager* mDescriptorManager;
		VulkanResourceManager* mResourceManager;

		VkPhysicalDeviceProperties mDeviceProperties;
		VkPhysicalDeviceFeatures mDeviceFeatures;
		VkPhysicalDeviceMemoryProperties mMemoryProperties;

		/** Contains data about a set of queues of a specific type. */
		struct QueueInfo
		{
			UINT32 familyIdx;
			Vector<VulkanQueue*> queues;
		};

		QueueInfo mQueueInfos[GQT_COUNT];

	};

	/** @} */
}
