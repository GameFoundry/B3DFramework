#include "BsVulkanDescriptorLayout.h"
#include "BsVulkanDevice.h"

namespace bs
{
	VulkanDescriptorLayout::VulkanDescriptorLayout(VulkanDevice& device, VkDescriptorSetLayoutBinding* bindings, 
		UINT32 numBindings)
		:mDevice(device)
	{
		mHash = calculateHash(bindings, numBindings);

		VkDescriptorSetLayoutCreateInfo layoutCI;
		layoutCI.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		layoutCI.pNext = nullptr;
		layoutCI.flags = 0;
		layoutCI.bindingCount = numBindings;
		layoutCI.pBindings = bindings;

		VkResult result = vkCreateDescriptorSetLayout(device.getLogical(), &layoutCI, gVulkanAllocator, &mLayout);
		assert(result == VK_SUCCESS);
	}

	VulkanDescriptorLayout::~VulkanDescriptorLayout()
	{
		vkDestroyDescriptorSetLayout(mDevice.getLogical(), mLayout, gVulkanAllocator);
	}

	size_t VulkanDescriptorLayout::calculateHash(VkDescriptorSetLayoutBinding* bindings, UINT32 numBindings)
	{
		size_t hash = 0;
		for (UINT32 i = 0; i < numBindings; i++)
		{
			size_t bindingHash = 0;
			hash_combine(bindingHash, bindings[i].binding);
			hash_combine(bindingHash, bindings[i].descriptorCount);
			hash_combine(bindingHash, bindings[i].descriptorType);
			hash_combine(bindingHash, bindings[i].stageFlags);
			assert(bindings[i].pImmutableSamplers == nullptr); // Not accounted for in hash, assumed always null

			hash_combine(hash, bindingHash);
		}

		return hash;
	}
}