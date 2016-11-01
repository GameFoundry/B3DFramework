//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsVulkanDescriptorManager.h"
#include "BsVulkanDescriptorLayout.h"
#include "BsVulkanDescriptorPool.h"
#include "BsVulkanDevice.h"
#include "BsVulkanResource.h"

namespace BansheeEngine
{
	VulkanLayoutKey::VulkanLayoutKey(VkDescriptorSetLayoutBinding* bindings, UINT32 numBindings)
		:bindings(bindings), numBindings(numBindings)
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
}