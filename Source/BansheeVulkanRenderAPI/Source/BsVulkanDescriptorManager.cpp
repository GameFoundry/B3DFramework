//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsVulkanDescriptorManager.h"
#include "BsVulkanDescriptorLayout.h"

namespace BansheeEngine
{
	VulkanLayoutKey::VulkanLayoutKey(VkDescriptorSetLayoutBinding* bindings, UINT32 numBindings)
		:bindings(bindings), numBindings(numBindings)
	{ }

	bool VulkanLayoutKey::operator==(const VulkanLayoutKey& rhs) const
	{
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

	size_t VulkanLayoutKey::calculateHash() const
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

	VulkanDescriptorManager::VulkanDescriptorManager(VulkanDevice& device)
		:mDevice(device)
	{

	}

	VulkanDescriptorManager::~VulkanDescriptorManager()
	{
		for (auto& entry : mSets)
		{
			bs_delete(entry.first.layout);
			bs_free(entry.first.bindings);
		}

		// TODO - Ensure all sets get released
	}

	VulkanDescriptorLayout* VulkanDescriptorManager::getLayout(VkDescriptorSetLayoutBinding* bindings, UINT32 numBindings)
	{
		VulkanLayoutKey key(bindings, numBindings);

		auto iterFind = mSets.find(key);
		if (iterFind != mSets.end())
			return iterFind->first.layout;

		// Create new
		key.bindings = bs_allocN<VkDescriptorSetLayoutBinding>(numBindings);
		memcpy(key.bindings, bindings, numBindings * sizeof(VkDescriptorSetLayoutBinding));

		key.layout = bs_new<VulkanDescriptorLayout>(mDevice, key.bindings, numBindings);
		mSets.insert(std::make_pair(key, 0));
		
		return key.layout;
	}
}