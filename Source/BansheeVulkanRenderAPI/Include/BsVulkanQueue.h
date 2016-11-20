//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsVulkanPrerequisites.h"

namespace bs
{
	/** @addtogroup Vulkan
	 *  @{
	 */

	/** Wrapper for the Vulkan device queue. */
	class VulkanQueue
	{
	public:
		VulkanQueue(VulkanDevice& device, VkQueue queue, GpuQueueType type, UINT32 index);

		/** Returns the internal handle to the Vulkan queue object. */
		VkQueue getHandle() const { return mQueue; }
		
		/** Returns the device that owns the queue. */
		VulkanDevice& getDevice() const { return mDevice; }

		/** Returns the type of the queue. */
		GpuQueueType getType() const { return mType; }

		/** Returns the unique index of the queue, for its type. */
		UINT32 getIndex() const { return mIndex; }

		/** 
		 * Checks if anything is currently executing on this queue. 
		 *
		 * @note	This status is only updated after a VulkanCommandBufferManager::refreshStates() call.
		 */
		bool isExecuting() const;

		/** Submits the provided command buffer on the queue. */
		void submit(VulkanCmdBuffer* cmdBuffer, VkSemaphore* waitSemaphores, UINT32 semaphoresCount);

		/** Blocks the calling thread until all operations on the queue finish. */
		void waitIdle() const;

	protected:
		VulkanDevice& mDevice;
		VkQueue mQueue;
		GpuQueueType mType;
		UINT32 mIndex;

		VulkanCmdBuffer* mLastCommandBuffer;
	};

	/** @} */
}
