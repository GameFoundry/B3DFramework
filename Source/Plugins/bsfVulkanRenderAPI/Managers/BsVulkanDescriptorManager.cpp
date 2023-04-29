//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Managers/BsVulkanDescriptorManager.h"
#include "BsVulkanDescriptorLayout.h"
#include "BsVulkanDescriptorSet.h"
#include "BsVulkanDescriptorPool.h"
#include "BsVulkanGpuDevice.h"
#include "BsVulkanResource.h"

using namespace bs;
using namespace bs::ct;

VulkanLayoutKey::VulkanLayoutKey(VkDescriptorSetLayoutBinding* bindings, u32 numBindings)
	: NumBindings(numBindings), Bindings(bindings)
{}

bool VulkanLayoutKey::operator==(const VulkanLayoutKey& rhs) const
{
	// If both have a layout, use that to compare directly, otherwise do it per-binding
	if(Layout != nullptr && rhs.Layout != nullptr)
		return Layout == rhs.Layout;

	if(NumBindings != rhs.NumBindings)
		return false;

	for(u32 i = 0; i < NumBindings; i++)
	{
		if(Bindings[i].binding != rhs.Bindings[i].binding)
			return false;

		if(Bindings[i].descriptorType != rhs.Bindings[i].descriptorType)
			return false;

		if(Bindings[i].descriptorCount != rhs.Bindings[i].descriptorCount)
			return false;

		if(Bindings[i].stageFlags != rhs.Bindings[i].stageFlags)
			return false;
	}

	return true;
}

VulkanPipelineLayoutKey::VulkanPipelineLayoutKey(VulkanDescriptorLayout** layouts, u32 numLayouts)
	: NumLayouts(numLayouts), Layouts(layouts)
{
}

bool VulkanPipelineLayoutKey::operator==(const VulkanPipelineLayoutKey& rhs) const
{
	if(NumLayouts != rhs.NumLayouts)
		return false;

	for(u32 i = 0; i < NumLayouts; i++)
	{
		if(Layouts[i] != rhs.Layouts[i])
			return false;
	}

	return true;
}

size_t VulkanPipelineLayoutKey::CalculateHash() const
{
	size_t hash = 0;
	for(u32 i = 0; i < NumLayouts; i++)
		B3DCombineHash(hash, Layouts[i]->GetHash());

	return hash;
}

VulkanDescriptorManager::VulkanDescriptorManager(VulkanGpuDevice& device)
	: mDevice(device)
{
	mPools.push_back(B3DNew<VulkanDescriptorPool>(device));
}

VulkanDescriptorManager::~VulkanDescriptorManager()
{
	for(auto& entry : mLayouts)
	{
		B3DDelete(entry.Layout);
		B3DFree(entry.Bindings);
	}

	for(auto& entry : mPipelineLayouts)
	{
		B3DFree(entry.first.Layouts);
		vkDestroyPipelineLayout(mDevice.GetLogical(), entry.second, gVulkanAllocator);
	}

	for(auto& entry : mPools)
		B3DDelete(entry);
}

VulkanDescriptorLayout* VulkanDescriptorManager::GetLayout(VkDescriptorSetLayoutBinding* bindings, u32 numBindings)
{
	VulkanLayoutKey key(bindings, numBindings);

	auto iterFind = mLayouts.find(key);
	if(iterFind != mLayouts.end())
		return iterFind->Layout;

	// Create new
	key.Bindings = B3DAllocateMultiple<VkDescriptorSetLayoutBinding>(numBindings);
	memcpy(key.Bindings, bindings, numBindings * sizeof(VkDescriptorSetLayoutBinding));

	key.Layout = B3DNew<VulkanDescriptorLayout>(mDevice, key.Bindings, numBindings);
	mLayouts.insert(key);

	return key.Layout;
}

VulkanDescriptorSet* VulkanDescriptorManager::CreateSet(VulkanDescriptorLayout* layout)
{
	// Note: We always retrieve the last created pool, even though there could be free room in earlier pools. However
	// that requires additional tracking. Since the assumption is that the first pool will be large enough for all
	// descriptors, and the only reason to create a second pool is fragmentation, this approach should not result in
	// a major resource waste.
	VkDescriptorSetLayout setLayout = layout->GetVulkanHandle();

	VkDescriptorSetAllocateInfo allocateInfo;
	allocateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	allocateInfo.pNext = nullptr;
	allocateInfo.descriptorPool = mPools.back()->GetVulkanHandle();
	allocateInfo.descriptorSetCount = 1;
	allocateInfo.pSetLayouts = &setLayout;

	VkDescriptorSet set;
	VkResult result = vkAllocateDescriptorSets(mDevice.GetLogical(), &allocateInfo, &set);
	if(result < 0) // Possible fragmentation, try in a new pool
	{
		mPools.push_back(B3DNew<VulkanDescriptorPool>(mDevice));
		allocateInfo.descriptorPool = mPools.back()->GetVulkanHandle();

		result = vkAllocateDescriptorSets(mDevice.GetLogical(), &allocateInfo, &set);
		B3D_ASSERT(result == VK_SUCCESS);
	}

	return mDevice.GetResourceManager().Create<VulkanDescriptorSet>(set, allocateInfo.descriptorPool);
}

VkPipelineLayout VulkanDescriptorManager::GetPipelineLayout(VulkanDescriptorLayout** layouts, u32 numLayouts)
{
	VulkanPipelineLayoutKey key(layouts, numLayouts);

	auto iterFind = mPipelineLayouts.find(key);
	if(iterFind != mPipelineLayouts.end())
		return iterFind->second;

	// Create new
	VkDescriptorSetLayout* setLayouts = (VkDescriptorSetLayout*)B3DStackAllocate(sizeof(VkDescriptorSetLayout) * numLayouts);
	for(u32 i = 0; i < numLayouts; i++)
		setLayouts[i] = layouts[i]->GetVulkanHandle();

	VkPipelineLayoutCreateInfo layoutCI;
	layoutCI.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	layoutCI.pNext = nullptr;
	layoutCI.flags = 0;
	layoutCI.pushConstantRangeCount = 0;
	layoutCI.pPushConstantRanges = nullptr;
	layoutCI.setLayoutCount = numLayouts;
	layoutCI.pSetLayouts = setLayouts;

	VkPipelineLayout pipelineLayout;
	VkResult result = vkCreatePipelineLayout(mDevice.GetLogical(), &layoutCI, gVulkanAllocator, &pipelineLayout);
	B3D_ASSERT(result == VK_SUCCESS);

	B3DStackFree(setLayouts);

	key.Layouts = (VulkanDescriptorLayout**)B3DAllocate(sizeof(VulkanDescriptorLayout*) * numLayouts);
	memcpy(key.Layouts, layouts, sizeof(VulkanDescriptorLayout*) * numLayouts);

	mPipelineLayouts.insert(std::make_pair(key, pipelineLayout));
	return pipelineLayout;
}
