//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsVulkanDescriptorLayout.h"
#include "BsVulkanDevice.h"

namespace bs
{
	namespace ct
	{
		VulkanDescriptorLayout::VulkanDescriptorLayout(VulkanDevice& device, VkDescriptorSetLayoutBinding* bindings, u32 numBindings)
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
			assert(result == VK_SUCCESS);
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
				bs_hash_combine(bindingHash, bindings[i].binding);
				bs_hash_combine(bindingHash, bindings[i].descriptorCount);
				bs_hash_combine(bindingHash, bindings[i].descriptorType);
				bs_hash_combine(bindingHash, bindings[i].stageFlags);
				assert(bindings[i].pImmutableSamplers == nullptr); // Not accounted for in hash, assumed always null

				bs_hash_combine(hash, bindingHash);
			}

			return hash;
		}
	} // namespace ct
} // namespace bs
