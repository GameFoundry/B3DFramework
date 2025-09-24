//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DVulkanDescriptorLayout.h"
#include "B3DVulkanGpuDevice.h"

using namespace b3d;
using namespace b3d::render;

VulkanDescriptorLayout::VulkanDescriptorLayout(VulkanGpuDevice& device, VkDescriptorSetLayoutBinding* bindings, u32 numBindings)
	: mDevice(device)
{
	mHash = CalculateHash(bindings, numBindings);

	VkDescriptorSetLayoutCreateInfo layoutCI;
	layoutCI.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	layoutCI.pNext = nullptr;
	layoutCI.flags = 0;
	layoutCI.bindingCount = numBindings;
	layoutCI.pBindings = bindings;

	VkResult result = vkCreateDescriptorSetLayout(device.GetLogical(), &layoutCI, gVulkanAllocator, &mLayout);
	B3D_ASSERT(result == VK_SUCCESS);
}

VulkanDescriptorLayout::~VulkanDescriptorLayout()
{
	vkDestroyDescriptorSetLayout(mDevice.GetLogical(), mLayout, gVulkanAllocator);
}

size_t VulkanDescriptorLayout::CalculateHash(VkDescriptorSetLayoutBinding* bindings, u32 numBindings)
{
	size_t hash = 0;
	for(u32 i = 0; i < numBindings; i++)
	{
		size_t bindingHash = 0;
		B3DCombineHash(bindingHash, bindings[i].binding);
		B3DCombineHash(bindingHash, bindings[i].descriptorCount);
		B3DCombineHash(bindingHash, bindings[i].descriptorType);
		B3DCombineHash(bindingHash, bindings[i].stageFlags);
		B3D_ASSERT(bindings[i].pImmutableSamplers == nullptr); // Not accounted for in hash, assumed always null

		B3DCombineHash(hash, bindingHash);
	}

	return hash;
}
