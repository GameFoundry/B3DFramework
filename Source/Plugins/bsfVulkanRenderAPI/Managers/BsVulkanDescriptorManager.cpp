//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Managers/BsVulkanDescriptorManager.h"
#include "BsVulkanDescriptorLayout.h"
#include "BsVulkanDescriptorSet.h"
#include "BsVulkanDescriptorPool.h"
#include "BsVulkanDevice.h"
#include "BsVulkanResource.h"

namespace bs { namespace ct
{
	VulkanLayoutKey::VulkanLayoutKey(VkDescriptorSetLayoutBinding* bindings, UINT32 numBindings)
		:numBindings(numBindings), bindings(bindings)
	{ }

	bool VulkanLayoutKey::operator==(const VulkanLayoutKey& rhs) const
	{
		// If both have a layout, use that to compare directly, otherwise do it per-binding
		if (layout != nullptr && rhs.layout != nullptr)
			return layout == rhs.layout;

		if (numBindings != rhs.numBindings)
			return false;

		for (UINT32 i = 0; i < numBindings; i++)
		{
			if (bindings[i].binding != rhs.bindings[i].binding)
				return false;

			if (bindings[i].descriptorType != rhs.bindings[i].descriptorType)
				return false;

			if (bindings[i].descriptorCount != rhs.bindings[i].descriptorCount)
				return false;

			if (bindings[i].stageFlags != rhs.bindings[i].stageFlags)
				return false;
		}

		return true;
	}

	VulkanPipelineLayoutKey::VulkanPipelineLayoutKey(VulkanDescriptorLayout** layouts, UINT32 numLayouts)
		:numLayouts(numLayouts), layouts(layouts)
	{
		
	}

	bool VulkanPipelineLayoutKey::operator==(const VulkanPipelineLayoutKey& rhs) const
	{
		if (numLayouts != rhs.numLayouts)
			return false;

		for (UINT32 i = 0; i < numLayouts; i++)
		{
			if (layouts[i] != rhs.layouts[i])
				return false;
		}

		return true;
	}

	size_t VulkanPipelineLayoutKey::calculateHash() const
	{
		size_t hash = 0;
		for (UINT32 i = 0; i < numLayouts; i++)
			bs_hash_combine(hash, layouts[i]->getHash());

		return hash;
	}

	VulkanDescriptorManager::VulkanDescriptorManager(VulkanDevice& device)
		:mDevice(device)
	{
		mPools.push_back(bs_new<VulkanDescriptorPool>(device));
	}

	VulkanDescriptorManager::~VulkanDescriptorManager()
	{
		for (auto& entry : mLayouts)
		{
			bs_delete(entry.layout);
			bs_free(entry.bindings);
		}

		for (auto& entry : mPipelineLayouts)
		{
			bs_free(entry.first.layouts);
			vkDestroyPipelineLayout(mDevice.getLogical(), entry.second, gVulkanAllocator);
		}

		for (auto& entry : mPools)
			bs_delete(entry);
	}

	VulkanDescriptorLayout* VulkanDescriptorManager::getLayout(VkDescriptorSetLayoutBinding* bindings, UINT32 numBindings)
	{
		VulkanLayoutKey key(bindings, numBindings);

		auto iterFind = mLayouts.find(key);
		if (iterFind != mLayouts.end())
			return iterFind->layout;

		// Create new
		key.bindings = bs_allocN<VkDescriptorSetLayoutBinding>(numBindings);
		memcpy(key.bindings, bindings, numBindings * sizeof(VkDescriptorSetLayoutBinding));

		key.layout = bs_new<VulkanDescriptorLayout>(mDevice, key.bindings, numBindings);
		mLayouts.insert(key);
		
		return key.layout;
	}

	VulkanDescriptorSet* VulkanDescriptorManager::createSet(VulkanDescriptorLayout* layout)
	{
		// Note: We always retrieve the last created pool, even though there could be free room in earlier pools. However
		// that requires additional tracking. Since the assumption is that the first pool will be large enough for all
		// descriptors, and the only reason to create a second pool is fragmentation, this approach should not result in
		// a major resource waste.
		VkDescriptorSetLayout setLayout = layout->getHandle();

		VkDescriptorSetAllocateInfo allocateInfo;
		allocateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		allocateInfo.pNext = nullptr;
		allocateInfo.descriptorPool = mPools.back()->getHandle();
		allocateInfo.descriptorSetCount = 1;
		allocateInfo.pSetLayouts = &setLayout;

		VkDescriptorSet set;
		VkResult result = vkAllocateDescriptorSets(mDevice.getLogical(), &allocateInfo, &set);
		if(result < 0) // Possible fragmentation, try in a new pool
		{
			mPools.push_back(bs_new<VulkanDescriptorPool>(mDevice));
			allocateInfo.descriptorPool = mPools.back()->getHandle();

			result = vkAllocateDescriptorSets(mDevice.getLogical(), &allocateInfo, &set);
			assert(result == VK_SUCCESS);
		}

		return mDevice.getResourceManager().create<VulkanDescriptorSet>(set, allocateInfo.descriptorPool);
	}

	VkPipelineLayout VulkanDescriptorManager::getPipelineLayout(VulkanDescriptorLayout** layouts, UINT32 numLayouts)
	{
		VulkanPipelineLayoutKey key(layouts, numLayouts);

		auto iterFind = mPipelineLayouts.find(key);
		if (iterFind != mPipelineLayouts.end())
			return iterFind->second;

		// Create new
		VkDescriptorSetLayout* setLayouts = (VkDescriptorSetLayout*)bs_stack_alloc(sizeof(VkDescriptorSetLayout) * numLayouts);
		for(UINT32 i = 0; i < numLayouts; i++)
			setLayouts[i] = layouts[i]->getHandle();

		VkPipelineLayoutCreateInfo layoutCI;
		layoutCI.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		layoutCI.pNext = nullptr;
		layoutCI.flags = 0;
		layoutCI.pushConstantRangeCount = 0;
		layoutCI.pPushConstantRanges = nullptr;
		layoutCI.setLayoutCount = numLayouts;
		layoutCI.pSetLayouts = setLayouts;

		VkPipelineLayout pipelineLayout;
		VkResult result = vkCreatePipelineLayout(mDevice.getLogical(), &layoutCI, gVulkanAllocator, &pipelineLayout);
		assert(result == VK_SUCCESS);

		bs_stack_free(setLayouts);

		key.layouts = (VulkanDescriptorLayout**)bs_alloc(sizeof(VulkanDescriptorLayout*) * numLayouts);
		memcpy(key.layouts, layouts, sizeof(VulkanDescriptorLayout*) * numLayouts);

		mPipelineLayouts.insert(std::make_pair(key, pipelineLayout));
		return pipelineLayout;
	}
}}