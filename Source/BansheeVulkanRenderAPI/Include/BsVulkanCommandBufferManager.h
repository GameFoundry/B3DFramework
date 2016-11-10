//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsVulkanPrerequisites.h"
#include "BsCommandBufferManager.h"
#include "BsVulkanCommandBuffer.h"

namespace BansheeEngine
{
	/** @addtogroup Vulkan
	 *  @{
	 */

	/** Wrapper around a command buffer used specifically for transfer operations. */
	class VulkanTransferBufferInfo
	{
	public:
		VulkanTransferBufferInfo(UINT32 queueIdx);

		/** 
		 * OR's the provided sync mask with the internal sync mask. The sync mask determines on which queues should
		 * the buffer wait on before executing. See CommandSyncMask.
		 */
		void appendMask(UINT32 syncMask) { mSyncMask |= syncMask; }

		/** Resets the sync mask. */
		void clearMask() { mSyncMask = 0; }

		/** Returns the internal command buffer. */
		VulkanCmdBuffer* getCB() const { return mCB; }
	private:
		friend class VulkanCommandBufferManager;

		VulkanCmdBuffer* mCB;
		UINT32 mSyncMask;
		UINT32 mQueueIdx;
	};

	/** 
	 * Handles creation of Vulkan command buffers. See CommandBuffer. 
	 *
	 * @note Core thread only.
	 */
	class VulkanCommandBufferManager : public CommandBufferManager
	{
	public:
		VulkanCommandBufferManager(const VulkanRenderAPI& rapi);
		~VulkanCommandBufferManager();

		/** @copydoc CommandBufferManager::createInternal() */
		SPtr<CommandBuffer> createInternal(GpuQueueType type, UINT32 deviceIdx = 0, UINT32 queueIdx = 0,
			bool secondary = false) override;

		/** Notifies the manager that this buffer was just submitted to the queue for execution. */
		void setActiveBuffer(GpuQueueType type, UINT32 deviceIdx, UINT32 queueIdx, VulkanCmdBuffer* buffer);

		/** 
		 * Returns a set of command buffer semaphores depending on the provided sync mask. 
		 *
		 * @param[in]	deviceIdx	Index of the device to get the semaphores for.
		 * @param[in]	syncMask	Mask that has a bit enabled for each command buffer to retrieve the semaphore for.
		 *							If the command buffer is not currently executing, semaphore won't be returned.
		 * @param[out]	semaphores	List containing all the required semaphores. Semaphores are tightly packed at the
		 *							beginning of the array. Must be able to hold at least BS_MAX_COMMAND_BUFFERS entries.
		 * @param[out]	count		Number of semaphores provided in the @p semaphores array.
		 */
		void getSyncSemaphores(UINT32 deviceIdx, UINT32 syncMask, VkSemaphore* semaphores, UINT32& count);

		/** 
		 * Checks if any of the active command buffers finished executing on the device and updates their states 
		 * accordingly. 
		 */
		void refreshStates(UINT32 deviceIdx);

		/** 
		 * Returns an command buffer that can be used for executing transfer operations on the specified queue. 
		 * Transfer buffers are automatically flushed (submitted) whenever a new (normal) command buffer is about to
		 * execute.
		 */
		VulkanTransferBufferInfo* getTransferBuffer(UINT32 deviceIdx, GpuQueueType type, UINT32 queueIdx);

		/** Submits all transfer command buffers, ensuring all queued transfer operations get executed. */
		void flushTransferBuffers(UINT32 deviceIdx);

	private:
		/** Contains command buffers specific to one device. */
		struct PerDeviceData
		{
			VulkanCmdBuffer* activeBuffers[BS_MAX_UNIQUE_QUEUES];
			VulkanTransferBufferInfo transferBuffers[BS_MAX_UNIQUE_QUEUES];
		};

		const VulkanRenderAPI& mRapi;

		PerDeviceData* mDeviceData;
		UINT32 mNumDevices;
	};

	/** @} */
}
