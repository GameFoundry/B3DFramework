//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsVulkanPrerequisites.h"
#include "BsVulkanResource.h"

namespace BansheeEngine
{
	/** @addtogroup Vulkan
	 *  @{
	 */

	/** Wrapper for the Vulkan descriptor set object. */
	class VulkanDescriptorSet : public VulkanResource
	{
	public:
		VulkanDescriptorSet(VulkanResourceManager* owner, VkDescriptorSet set, VkDescriptorPool pool);
		~VulkanDescriptorSet();

		/** Returns a handle to the Vulkan descriptor set object. */
		VkDescriptorSet getHandle() const { return mSet; }

		/** Updates the descriptor set with the provided values. */
		void write(VkWriteDescriptorSet* entries, UINT32 count);

	protected:
		VkDescriptorSet mSet;
		VkDescriptorPool mPool;
	};

	/** @} */
}
