//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DVulkanDescriptorSet.h"
#include "B3DVulkanGpuDevice.h"

using namespace b3d;
using namespace b3d::render;

VulkanDescriptorSet::VulkanDescriptorSet(VulkanResourceManager* owner, VkDescriptorSet set, VkDescriptorPool pool, const StringView& name)
	: VulkanResource(owner, true, name), mSet(set), mPool(pool)
{}

VulkanDescriptorSet::~VulkanDescriptorSet()
{
	VkResult result = vkFreeDescriptorSets(mOwner->GetDevice().GetLogical(), mPool, 1, &mSet);
	B3D_ASSERT(result == VK_SUCCESS);
}

void VulkanDescriptorSet::Write(TArrayView<VkWriteDescriptorSet> entries)
{
	for(u32 i = 0; i < (u32)entries.size(); i++)
		entries[i].dstSet = mSet;

	vkUpdateDescriptorSets(mOwner->GetDevice().GetLogical(), (u32)entries.size(), entries.data(), 0, nullptr);
}
