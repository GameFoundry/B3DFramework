//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsVulkanDescriptorPool.h"
#include "BsVulkanGpuDevice.h"

using namespace b3d;
using namespace b3d::render;

VulkanDescriptorPool::VulkanDescriptorPool(VulkanGpuDevice& device)
	: mDevice(device)
{
	VkDescriptorPoolSize poolSizes[8];
	poolSizes[0].type = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
	poolSizes[0].descriptorCount = sMaxSampledImages;

	poolSizes[1].type = VK_DESCRIPTOR_TYPE_SAMPLER;
	poolSizes[1].descriptorCount = sMaxSampledImages;

	poolSizes[2].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	poolSizes[2].descriptorCount = sMaxSampledImages;

	poolSizes[3].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	poolSizes[3].descriptorCount = sMaxUniformBuffers;

	poolSizes[4].type = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
	poolSizes[4].descriptorCount = sMaxImages;

	poolSizes[5].type = VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER;
	poolSizes[5].descriptorCount = sMaxSampledBuffers;

	poolSizes[6].type = VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER;
	poolSizes[6].descriptorCount = sMaxBuffers;

	poolSizes[7].type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
	poolSizes[7].descriptorCount = sMaxBuffers;

	VkDescriptorPoolCreateInfo poolCI;
	poolCI.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	poolCI.pNext = nullptr;
	poolCI.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
	poolCI.maxSets = sMaxSets;
	poolCI.poolSizeCount = sizeof(poolSizes) / sizeof(poolSizes[0]);
	poolCI.pPoolSizes = poolSizes;

	VkResult result = vkCreateDescriptorPool(mDevice.GetLogical(), &poolCI, gVulkanAllocator, &mPool);
	B3D_ASSERT(result == VK_SUCCESS);
}

VulkanDescriptorPool::~VulkanDescriptorPool()
{
	vkDestroyDescriptorPool(mDevice.GetLogical(), mPool, gVulkanAllocator);
}
