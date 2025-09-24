//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DVulkanPrerequisites.h"

namespace b3d
{
	namespace render
	{
		/** @addtogroup Vulkan
		 *  @{
		 */

		/** Pool that allocates and distributes Vulkan descriptor sets. */
		class VulkanDescriptorPool
		{
		public:
			VulkanDescriptorPool(VulkanGpuDevice& device);
			~VulkanDescriptorPool();

			/** Returns a handle to the internal Vulkan descriptor pool. */
			VkDescriptorPool GetVulkanHandle() const { return mPool; }

		private:
			static const u32 sMaxSets = 8192;
			static const u32 sMaxSampledImages = 4096;
			static const u32 sMaxImages = 2048;
			static const u32 sMaxSampledBuffers = 2048;
			static const u32 sMaxBuffers = 2048;
			static const u32 sMaxUniformBuffers = 2048;

			VulkanGpuDevice& mDevice;
			VkDescriptorPool mPool;
		};

		/** @} */
	} // namespace render
} // namespace b3d
