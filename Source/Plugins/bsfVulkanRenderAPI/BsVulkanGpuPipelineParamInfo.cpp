//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsVulkanGpuPipelineParamInfo.h"
#include "BsVulkanUtility.h"
#include "BsVulkanRenderAPI.h"
#include "BsVulkanDevice.h"
#include "RenderAPI/BsGpuParamDesc.h"

using namespace bs;
using namespace bs::ct;

VulkanGpuPipelineParamInfo::VulkanGpuPipelineParamInfo(const GPU_PIPELINE_PARAMS_DESC& desc, GpuDeviceFlags deviceMask)
	: GpuPipelineParamInfo(desc, deviceMask), mDeviceMask(deviceMask), mLayouts(), mLayoutInfos()
{}

void VulkanGpuPipelineParamInfo::Initialize()
{
	VulkanRenderAPI& rapi = static_cast<VulkanRenderAPI&>(RenderAPI::Instance());

	VulkanDevice* devices[BS_MAX_DEVICES];
	VulkanUtility::GetDevices(rapi, mDeviceMask, devices);

	u32 numDevices = 0;
	for(u32 i = 0; i < BS_MAX_DEVICES; i++)
	{
		if(devices[i] != nullptr)
			numDevices++;
	}

	u32 totalNumSlots = 0;
	for(u32 i = 0; i < mNumSets; i++)
		totalNumSlots += mSetInfos[i].NumSlots;

	mAlloc.Reserve<VkDescriptorSetLayoutBinding>(mNumElements)
		.Reserve<GpuParamObjectType>(mNumElements)
		.Reserve<GpuBufferFormat>(mNumElements)
		.Reserve<LayoutInfo>(mNumSets)
		.Reserve<VulkanDescriptorLayout*>(mNumSets * numDevices)
		.Reserve<SetExtraInfo>(mNumSets)
		.Reserve<u32>(totalNumSlots)
		.Init();

	mLayoutInfos = mAlloc.Alloc<LayoutInfo>(mNumSets);
	VkDescriptorSetLayoutBinding* bindings = mAlloc.Alloc<VkDescriptorSetLayoutBinding>(mNumElements);
	GpuParamObjectType* types = mAlloc.Alloc<GpuParamObjectType>(mNumElements);
	GpuBufferFormat* elementTypes = mAlloc.Alloc<GpuBufferFormat>(mNumElements);

	for(u32 i = 0; i < BS_MAX_DEVICES; i++)
	{
		if(devices[i] == nullptr)
		{
			mLayouts[i] = nullptr;
			continue;
		}

		mLayouts[i] = mAlloc.Alloc<VulkanDescriptorLayout*>(mNumSets);
	}

	mSetExtraInfos = mAlloc.Alloc<SetExtraInfo>(mNumSets);

	if(bindings != nullptr)
		bs_zero_out(bindings, mNumElements);

	if(types != nullptr)
		bs_zero_out(types, mNumElements);

	if(elementTypes != nullptr)
		bs_zero_out(elementTypes, mNumElements);

	u32 globalBindingIdx = 0;
	for(u32 i = 0; i < mNumSets; i++)
	{
		mSetExtraInfos[i].SlotIndices = mAlloc.Alloc<u32>(mSetInfos[i].NumSlots);

		mLayoutInfos[i].NumBindings = 0;
		mLayoutInfos[i].Bindings = nullptr;
		mLayoutInfos[i].Types = nullptr;
		mLayoutInfos[i].ElementTypes = nullptr;

		for(u32 j = 0; j < mSetInfos[i].NumSlots; j++)
		{
			if(mSetInfos[i].SlotIndices[j] == (u32)-1)
			{
				mSetExtraInfos[i].SlotIndices[j] = (u32)-1;
				continue;
			}

			VkDescriptorSetLayoutBinding& binding = bindings[globalBindingIdx];
			binding.binding = j;

			mSetExtraInfos[i].SlotIndices[j] = globalBindingIdx;
			mLayoutInfos[i].NumBindings++;
			globalBindingIdx++;
		}
	}

	u32 offset = 0;
	for(u32 i = 0; i < mNumSets; i++)
	{
		mLayoutInfos[i].Bindings = &bindings[offset];
		mLayoutInfos[i].Types = &types[offset];
		mLayoutInfos[i].ElementTypes = &elementTypes[offset];
		offset += mLayoutInfos[i].NumBindings;
	}

	VkShaderStageFlags stageFlagsLookup[6];
	stageFlagsLookup[GPT_VERTEX_PROGRAM] = VK_SHADER_STAGE_VERTEX_BIT;
	stageFlagsLookup[GPT_HULL_PROGRAM] = VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT;
	stageFlagsLookup[GPT_DOMAIN_PROGRAM] = VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT;
	stageFlagsLookup[GPT_GEOMETRY_PROGRAM] = VK_SHADER_STAGE_GEOMETRY_BIT;
	stageFlagsLookup[GPT_FRAGMENT_PROGRAM] = VK_SHADER_STAGE_FRAGMENT_BIT;
	stageFlagsLookup[GPT_COMPUTE_PROGRAM] = VK_SHADER_STAGE_COMPUTE_BIT;

	u32 numParamDescs = sizeof(mParamDescs) / sizeof(mParamDescs[0]);
	for(u32 i = 0; i < numParamDescs; i++)
	{
		const SPtr<GpuParamDesc>& paramDesc = mParamDescs[i];
		if(paramDesc == nullptr)
			continue;

		auto setUpBlockBindings = [&](auto& params, VkDescriptorType descType)
		{
			for(auto& entry : params)
			{
				u32 bindingIdx = GetBindingIdx(entry.second.Set, entry.second.Slot);
				assert(bindingIdx != (u32)-1);

				VkDescriptorSetLayoutBinding& binding = bindings[bindingIdx];
				binding.descriptorCount = 1;
				binding.stageFlags |= stageFlagsLookup[i];
				binding.descriptorType = descType;
			}
		};

		auto setUpBindings = [&](auto& params, VkDescriptorType descType)
		{
			for(auto& entry : params)
			{
				u32 bindingIdx = GetBindingIdx(entry.second.Set, entry.second.Slot);
				assert(bindingIdx != (u32)-1);

				VkDescriptorSetLayoutBinding& binding = bindings[bindingIdx];
				binding.descriptorCount = 1;
				binding.stageFlags |= stageFlagsLookup[i];
				binding.descriptorType = descType;

				types[bindingIdx] = entry.second.Type;
				elementTypes[bindingIdx] = entry.second.ElementType;
			}
		};

		setUpBlockBindings(paramDesc->ParamBlocks, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);
		setUpBindings(paramDesc->Textures, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE);
		setUpBindings(paramDesc->LoadStoreTextures, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE);

		// Set up sampler bindings
		for(auto& entry : paramDesc->Samplers)
		{
			u32 bindingIdx = GetBindingIdx(entry.second.Set, entry.second.Slot);
			assert(bindingIdx != (u32)-1);

			VkDescriptorSetLayoutBinding& binding = bindings[bindingIdx];

			// If we already assigned an image to this binding slot, then it's a combined image/sampler
			if(binding.descriptorType == VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE)
				binding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			else
			{
				binding.descriptorCount = 1;
				binding.stageFlags |= stageFlagsLookup[i];
				binding.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER;

				types[bindingIdx] = entry.second.Type;
				elementTypes[bindingIdx] = entry.second.ElementType;
			}
		}

		// Set up buffer bindings
		for(auto& entry : paramDesc->Buffers)
		{
			u32 bindingIdx = GetBindingIdx(entry.second.Set, entry.second.Slot);
			assert(bindingIdx != (u32)-1);

			VkDescriptorSetLayoutBinding& binding = bindings[bindingIdx];
			binding.descriptorCount = 1;
			binding.stageFlags |= stageFlagsLookup[i];

			switch(entry.second.Type)
			{
			default:
			case GPOT_BYTE_BUFFER:
				binding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER;
				break;
			case GPOT_RWBYTE_BUFFER:
				binding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER;
				break;
			case GPOT_STRUCTURED_BUFFER:
			case GPOT_RWSTRUCTURED_BUFFER:
				binding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
				break;
			}

			types[bindingIdx] = entry.second.Type;
			elementTypes[bindingIdx] = entry.second.ElementType;
		}
	}

	// Allocate layouts per-device
	for(u32 i = 0; i < BS_MAX_DEVICES; i++)
	{
		if(mLayouts[i] == nullptr)
			continue;

		VulkanDescriptorManager& descManager = devices[i]->GetDescriptorManager();
		for(u32 j = 0; j < mNumSets; j++)
			mLayouts[i][j] = descManager.GetLayout(mLayoutInfos[j].Bindings, mLayoutInfos[j].NumBindings);
	}
}

VulkanDescriptorLayout* VulkanGpuPipelineParamInfo::GetLayout(u32 deviceIdx, u32 layoutIdx) const
{
	if(deviceIdx >= BS_MAX_DEVICES || mLayouts[deviceIdx] == nullptr)
		return nullptr;

	return mLayouts[deviceIdx][layoutIdx];
}
