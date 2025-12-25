//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DVulkanPrerequisites.h"
#include "B3DVulkanResource.h"

namespace b3d
{
	namespace render
	{
		class VulkanGpuParameterSetPool;

		/** @addtogroup Vulkan
		 *  @{
		 */

		/** Wrapper for the Vulkan descriptor set object. */
		class VulkanDescriptorSet : public VulkanResource
		{
		public:
			VulkanDescriptorSet(VulkanResourceManager* owner, VkDescriptorSet set, VkDescriptorPool descriptorPool, VulkanGpuParameterSetPool* pool, const StringView& name = "");
			~VulkanDescriptorSet();

			/** Returns a handle to the Vulkan descriptor set object. */
			VkDescriptorSet GetVulkanHandle() const { return mSet; }

			/** Returns the Vulkan descriptor pool this set was allocated from. */
			VkDescriptorPool GetVkDescriptorPool() const { return mVkDescriptorPool; }

			/** Updates the descriptor set with the provided values. */
			void Write(TArrayView<VkWriteDescriptorSet> entries);

		protected:
			VkDescriptorSet mSet;
			VkDescriptorPool mVkDescriptorPool = VK_NULL_HANDLE;
			VulkanGpuParameterSetPool* mPool = nullptr;
		};

		/** @} */
	} // namespace render
} // namespace b3d
