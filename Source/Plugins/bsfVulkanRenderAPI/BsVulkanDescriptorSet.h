//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsVulkanPrerequisites.h"
#include "BsVulkanResource.h"

namespace bs
{
	namespace ct
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
			VkDescriptorSet GetHandle() const { return mSet; }

			/** Updates the descriptor set with the provided values. */
			void Write(VkWriteDescriptorSet* entries, u32 count);

		protected:
			VkDescriptorSet mSet;
			VkDescriptorPool mPool;
		};

		/** @} */
	} // namespace ct
} // namespace bs
