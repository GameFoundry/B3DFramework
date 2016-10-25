//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsVulkanPrerequisites.h"
#include "BsCommandBuffer.h"
#include "BsVulkanRenderAPI.h"

namespace BansheeEngine
{
	/** @addtogroup Vulkan
	 *  @{
	 */

	/** CommandBuffer implementation for Vulkan. */
	class VulkanCommandBuffer : public CommandBuffer
	{
	public:
		~VulkanCommandBuffer();

		/** Returns the internal Vulkan command buffer wrapped by this object. */
		VkCommandBuffer getInternal() const { return mBuffer; }

	private:
		friend class VulkanCommandBufferManager;

		VulkanCommandBuffer(const SPtr<VulkanDevice>& device, CommandBufferType type, UINT32 syncMask, bool secondary);

		VkCommandBuffer mBuffer;
		VkCommandPool mPool;
		VkDevice mDevice;
	};

	/** @} */
}