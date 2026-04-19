//************************************ B3D Framework - Copyright 2026 Marko Pintera **************************************//
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

		/** Wrapper for the Vulkan descriptor layout object. */
		class VulkanDescriptorLayout
		{
		public:
			VulkanDescriptorLayout(VulkanGpuDevice& device, TArrayView<VkDescriptorSetLayoutBinding> bindings);
			~VulkanDescriptorLayout();

			/** Returns a handle to the Vulkan set layout object. */
			VkDescriptorSetLayout GetVulkanHandle() const { return mLayout; }

			/** Returns a hash value for the descriptor layout. */
			size_t GetHash() const { return mHash; }

			/** Calculates a has value for the provided descriptor set layout bindings. */
			static size_t CalculateHash(TArrayView<VkDescriptorSetLayoutBinding> bindings);

		protected:
			VulkanGpuDevice& mDevice;
			VkDescriptorSetLayout mLayout;
			size_t mHash;
		};

		/** @} */
	} // namespace render
} // namespace b3d
