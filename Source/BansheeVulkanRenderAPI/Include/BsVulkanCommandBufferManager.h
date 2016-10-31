//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsVulkanPrerequisites.h"
#include "BsCommandBufferManager.h"

namespace BansheeEngine
{
	/** @addtogroup Vulkan
	 *  @{
	 */

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
		SPtr<CommandBuffer> createInternal(UINT32 id, GpuQueueType type, UINT32 deviceIdx = 0, UINT32 queueIdx = 0,
			bool secondary = false) override;

		/** 
		 * Returns a set of command buffer semaphores depending on the provided sync mask. 
		 *
		 * @param[in]	deviceIdx	Index of the device to get the semaphores for.
		 * @param[in]	syncMask	Mask that has a bit enabled for each command buffer to retrieve the semaphore for.
		 *							If the command buffer is not currently executing, semaphore won't be returned.
		 * @param[out]	semaphores	List containing all the required semaphores. Semaphores are tightly packed at the
		 *							beginning of the array.
		 * @param[out]	count		Number of semaphores provided in the @p semaphores array.
		 */
		void getSyncSemaphores(UINT32 deviceIdx, UINT32 syncMask, VkSemaphore(&semaphores)[BS_MAX_COMMAND_BUFFERS], 
			UINT32& count);

		/** 
		 * Checks if any of the active command buffers finished executing on the device and updates their states 
		 * accordingly. 
		 */
		void refreshStates(UINT32 deviceIdx);

	private:
		const VulkanRenderAPI& mRapi;
	};

	/** @} */
}