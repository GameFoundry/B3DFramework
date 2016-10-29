//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsVulkanPrerequisites.h"

namespace BansheeEngine
{
	/** @addtogroup Vulkan
	 *  @{
	 */

	/** Wrapper for the Vulkan device queue. */
	class VulkanQueue
	{
	public:
		VulkanQueue(VkQueue queue);

		/** Returns the internal handle to the Vulkan queue object. */
		VkQueue getHandle() const { return mQueue; }
		
		/** 
		 * Notifies the queue that a command buffer was submitted. 
		 *
		 * @param[in]	cmdBuffer		Command buffer that was submitted.
		 * @param[in]	fenceCounter	Fence counter of the command buffer at time of submission. This counter gets
		 *								incremented whenever a command buffer is done executing on the device. This allow
		 *								us to know when the queue is done with a command buffer.
		 */
		void notifySubmit(const VulkanCommandBuffer& cmdBuffer, UINT32 fenceCounter);

	protected:
		VkQueue mQueue;
		VkSemaphore mSemaphoresTemp[BS_MAX_COMMAND_BUFFERS];

		UINT32 mFenceCounter;
		UINT32 mLastCommandBufferId;
	};

	/** @} */
}
