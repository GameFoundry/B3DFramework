#include "BsVulkanDescriptorLayout.h"
#include "BsVulkanDevice.h"

namespace BansheeEngine
{
	VulkanDescriptorLayout::VulkanDescriptorLayout(VulkanDevice& device, VkDescriptorSetLayoutBinding* bindings, 
		UINT32 numBindings)
		:mDevice(device)
	{
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
}