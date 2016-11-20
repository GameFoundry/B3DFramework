//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsVulkanDescriptorSet.h"
#include "BsVulkanDevice.h"

namespace bs
{
	VulkanDescriptorSet::VulkanDescriptorSet(VulkanResourceManager* owner, VkDescriptorSet set, VkDescriptorPool pool)
		:VulkanResource(owner, true), mSet(set), mPool(pool)
	{ }

	VulkanDescriptorSet::~VulkanDescriptorSet()
	{
		VkResult result = vkFreeDescriptorSets(mOwner->getDevice().getLogical(), mPool, 1, &mSet);
		assert(result == VK_SUCCESS);
	}

	void VulkanDescriptorSet::write(VkWriteDescriptorSet* entries, UINT32 count)
	{
		for (UINT32 i = 0; i < count; i++)
			entries[i].dstSet = mSet;

		vkUpdateDescriptorSets(mOwner->getDevice().getLogical(), count, entries, 0, nullptr);
	}
}