//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Managers/B3DVulkanDescriptorManager.h"
#include "B3DVulkanDescriptorLayout.h"
#include "B3DVulkanDescriptorSet.h"
#include "B3DVulkanDescriptorPool.h"
#include "B3DVulkanGpuDevice.h"
#include "B3DVulkanResource.h"

using namespace b3d;
using namespace b3d::render;

VulkanLayoutKey::VulkanLayoutKey(TArrayView<VkDescriptorSetLayoutBinding> bindings)
	: Bindings(bindings)
{}

bool VulkanLayoutKey::operator==(const VulkanLayoutKey& rhs) const
{
	// If both have a layout, use that to compare directly, otherwise do it per-binding
	if(Layout != nullptr && rhs.Layout != nullptr)
		return Layout == rhs.Layout;

	if(Bindings.size() != rhs.Bindings.size())
		return false;

	for(u32 i = 0; i < Bindings.size(); i++)
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
		B3DFree((void*)entry.Bindings.Data());
	}

	for(auto& entry : mPipelineLayouts)
	{
		B3DFree(entry.first.Layouts);
		vkDestroyPipelineLayout(mDevice.GetLogical(), entry.second, gVulkanAllocator);
	}

	for(auto& entry : mPools)
		B3DDelete(entry);
}

VulkanDescriptorLayout* VulkanDescriptorManager::GetLayout(TArrayView<VkDescriptorSetLayoutBinding> bindings)
{
	VulkanLayoutKey key(bindings);

	auto iterFind = mLayouts.find(key);
	if(iterFind != mLayouts.end())
		return iterFind->Layout;

	// Create new
	const u32 bindingCount = (u32)bindings.Size();
	VkDescriptorSetLayoutBinding* const data = B3DAllocateMultiple<VkDescriptorSetLayoutBinding>(bindingCount);
	memcpy(data, bindings.data(), bindingCount * sizeof(VkDescriptorSetLayoutBinding));

	key.Bindings = TArrayView(data, bindingCount);

	key.Layout = B3DNew<VulkanDescriptorLayout>(mDevice, key.Bindings);
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

VkPipelineLayout VulkanDescriptorManager::GetPipelineLayout(VulkanDescriptorLayout** layouts, u32 bindingCount)
{
	VulkanPipelineLayoutKey key(layouts, bindingCount);

	auto iterFind = mPipelineLayouts.find(key);
	if(iterFind != mPipelineLayouts.end())
		return iterFind->second;

	// Create new
	VkDescriptorSetLayout* setLayouts = (VkDescriptorSetLayout*)B3DStackAllocate(sizeof(VkDescriptorSetLayout) * bindingCount);
	for(u32 i = 0; i < bindingCount; i++)
		setLayouts[i] = layouts[i]->GetVulkanHandle();

	VkPipelineLayoutCreateInfo layoutCI;
	layoutCI.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	layoutCI.pNext = nullptr;
	layoutCI.flags = 0;
	layoutCI.pushConstantRangeCount = 0;
	layoutCI.pPushConstantRanges = nullptr;
	layoutCI.setLayoutCount = bindingCount;
	layoutCI.pSetLayouts = setLayouts;

	VkPipelineLayout pipelineLayout;
	VkResult result = vkCreatePipelineLayout(mDevice.GetLogical(), &layoutCI, gVulkanAllocator, &pipelineLayout);
	B3D_ASSERT(result == VK_SUCCESS);

	B3DStackFree(setLayouts);

	key.Layouts = (VulkanDescriptorLayout**)B3DAllocate(sizeof(VulkanDescriptorLayout*) * bindingCount);
	memcpy(key.Layouts, layouts, sizeof(VulkanDescriptorLayout*) * bindingCount);

	mPipelineLayouts.insert(std::make_pair(key, pipelineLayout));
	return pipelineLayout;
}
