//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsVulkanPrerequisites.h"

namespace BansheeEngine
{
	/** @addtogroup Vulkan
	 *  @{
	 */

	/** Wrapper for the Vulkan descriptor layout object. */
	class VulkanDescriptorLayout
	{
	public:
		VulkanDescriptorLayout(VulkanDevice& device, VkDescriptorSetLayoutBinding* bindings, UINT32 numBindings);
		~VulkanDescriptorLayout();

		/** Returns a handle to the Vulkan set layout object. */
		VkDescriptorSetLayout getHandle() const { return mLayout; }

	protected:
		VulkanDevice& mDevice;
		VkDescriptorSetLayout mLayout;
	};

	/** @} */
}
