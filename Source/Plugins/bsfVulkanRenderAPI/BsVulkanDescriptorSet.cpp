//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsVulkanDescriptorSet.h"
#include "BsVulkanDevice.h"

namespace bs { namespace ct
{
	VulkanDescriptorSet::VulkanDescriptorSet(VulkanResourceManager* owner, VkDescriptorSet set, VkDescriptorPool pool)
		:VulkanResource(owner, true), mSet(set), mPool(pool)
	{ }

	VulkanDescriptorSet::~VulkanDescriptorSet()
	{
		VkResult result = vkFreeDescriptorSets(mOwner->GetDevice().GetLogical(), mPool, 1, &mSet);
		assert(result == VK_SUCCESS);
	}

	void VulkanDescriptorSet::Write(VkWriteDescriptorSet* entries, UINT32 count)
	{
		for (UINT32 i = 0; i < count; i++)
			entries[i].dstSet = mSet;

		vkUpdateDescriptorSets(mOwner->GetDevice().GetLogical(), count, entries, 0, nullptr);
	}
}}
