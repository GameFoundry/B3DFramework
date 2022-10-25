//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsVulkanPrerequisites.h"

namespace bs
{
	namespace ct
	{
		/** @addtogroup Vulkan
		 *  @{
		 */

		/** Wrapper for the Vulkan descriptor layout object. */
		class VulkanDescriptorLayout
		{
		public:
			VulkanDescriptorLayout(VulkanDevice& device, VkDescriptorSetLayoutBinding* bindings, u32 numBindings);
			~VulkanDescriptorLayout();

			/** Returns a handle to the Vulkan set layout object. */
			VkDescriptorSetLayout GetHandle() const { return mLayout; }

			/** Returns a hash value for the descriptor layout. */
			size_t GetHash() const { return mHash; }

			/** Calculates a has value for the provided descriptor set layout bindings. */
			static size_t CalculateHash(VkDescriptorSetLayoutBinding* bindings, u32 numBindings);

		protected:
			VulkanDevice& mDevice;
			VkDescriptorSetLayout mLayout;
			size_t mHash;
		};

		/** @} */
	} // namespace ct
} // namespace bs
