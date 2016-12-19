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
		void submit(VulkanCmdBuffer* cmdBuffer, VulkanSemaphore** waitSemaphores, UINT32 semaphoresCount);

		/** 
		 * Presents the back buffer of the provided swap chain. 
		 *
		 * @param[in]	swapChain			Swap chain whose back buffer to present.
		 * @param[in]	waitSemaphores		Optional semaphores to wait on before presenting the queue.
		 * @param[in]	semaphoresCount		Number of semaphores in the @p semaphores array.
		 */
		void present(VulkanSwapChain* swapChain, VulkanSemaphore** waitSemaphores, UINT32 semaphoresCount);

		/** Blocks the calling thread until all operations on the queue finish. */
		void waitIdle() const;

		/** 
		 * Checks if any of the active command buffers finished executing on the queue and updates their states 
		 * accordingly. 
		 * 
		 * @param[in]	queueEmpty	Set to true if the caller guarantees the queue will be empty (e.g. on shutdown). This
		 *							allows the system to free all needed resources.
		 */
		void refreshStates(bool queueEmpty = false);

		/** Returns the last command buffer that was submitted on this queue. */
		VulkanCmdBuffer* getLastCommandBuffer() const { return mLastCommandBuffer; }

	protected:
		/** Information about a single submitted command buffer. */
		struct SubmitInfo
		{
			SubmitInfo(VulkanCmdBuffer* cmdBuffer, UINT32 submitIdx, UINT32 numSemaphores)
				:cmdBuffer(cmdBuffer), submitIdx(submitIdx), numSemaphores(numSemaphores)
			{ }

			VulkanCmdBuffer* cmdBuffer;
			UINT32 submitIdx;
			UINT32 numSemaphores;
		};

		VulkanDevice& mDevice;
		VkQueue mQueue;
		GpuQueueType mType;
		UINT32 mIndex;
		VkPipelineStageFlags mSubmitDstWaitMask[BS_MAX_UNIQUE_QUEUES];

		List<SubmitInfo> mActiveBuffers;
		Queue<VulkanSemaphore*> mActiveSemaphores;
		VulkanCmdBuffer* mLastCommandBuffer;
		UINT32 mNextSubmitIdx;

		VkSemaphore mSemaphoresTemp[BS_MAX_UNIQUE_QUEUES + 1]; // +1 for present semaphore
	};

	/** @} */
}
