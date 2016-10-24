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

	private:
		friend class VulkanCommandBufferManager;

		VulkanCommandBuffer(CommandBufferType type, UINT32 deviceIdx, UINT32 syncMask, bool secondary);
	};

	/** @} */
}