//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsVulkanPrerequisites.h"

namespace bs
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

		/** Returns a hash value for the descriptor layout. */
		size_t getHash() const { return mHash; }

		/** Calculates a has value for the provided descriptor set layout bindings. */
		static size_t calculateHash(VkDescriptorSetLayoutBinding* bindings, UINT32 numBindings);

	protected:
		VulkanDevice& mDevice;
		VkDescriptorSetLayout mLayout;
		size_t mHash;
	};

	/** @} */
}
