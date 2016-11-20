//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsVulkanPrerequisites.h"

namespace bs
{
	/** @addtogroup Vulkan
	 *  @{
	 */

	/** Pool that allocates and distributes Vulkan descriptor sets. */
	class VulkanDescriptorPool
	{
	public:
		VulkanDescriptorPool(VulkanDevice& device);
		~VulkanDescriptorPool();

		/** Returns a handle to the internal Vulkan descriptor pool. */
		VkDescriptorPool getHandle() const { return mPool; }

	private:
		static const UINT32 sMaxSets = 8192;
		static const UINT32 sMaxSampledImages = 4096;
		static const UINT32 sMaxImages = 2048;
		static const UINT32 sMaxSampledBuffers = 2048;
		static const UINT32 sMaxBuffers = 2048;
		static const UINT32 sMaxUniformBuffers = 2048;

		VulkanDevice& mDevice;
		VkDescriptorPool mPool;
	};

	/** @} */
}