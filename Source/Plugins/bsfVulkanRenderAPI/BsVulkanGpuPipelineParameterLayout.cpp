//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsVulkanGpuPipelineParameterLayout.h"
#include "BsVulkanUtility.h"
#include "BsVulkanRenderAPI.h"
#include "BsVulkanGpuDevice.h"
#include "RenderAPI/BsGpuProgramParameterDescription.h"

using namespace bs;
using namespace bs::ct;

VulkanGpuPipelineParameterLayout::VulkanGpuPipelineParameterLayout(VulkanGpuDevice& gpuDevice, const GpuPipelineParameterLayoutCreateInformation& createInformation)
	: GpuPipelineParameterLayout(createInformation), mGpuDevice(gpuDevice), mLayouts(), mLayoutInfos()
{}

void VulkanGpuPipelineParameterLayout::Initialize()
{
	VulkanRenderAPI& rapi = static_cast<VulkanRenderAPI&>(RenderAPI::Instance());

	u32 totalSlotCount = 0;
	for(u32 i = 0; i < mSetCount; i++)
		totalSlotCount += mSetInfos[i].SlotCount;

	mAlloc.Reserve<VkDescriptorSetLayoutBinding>(mBindingSlotCount)
		.Reserve<GpuParameterObjectType>(mBindingSlotCount)
		.Reserve<GpuBufferFormat>(mBindingSlotCount)
		.Reserve<GpuBufferFormat>(mBindingSlotCount)
		.Reserve<LayoutInfo>(mSetCount)
		.Reserve<VulkanDescriptorLayout*>(mSetCount)
		.Reserve<SetExtraInfo>(mSetCount)
		.Reserve<u32>(totalSlotCount)
		.Reserve<u32>(totalSlotCount)
		.Init();

	mLayoutInfos = mAlloc.Alloc<LayoutInfo>(mSetCount);
	VkDescriptorSetLayoutBinding* bindings = mAlloc.Alloc<VkDescriptorSetLayoutBinding>(mBindingSlotCount);
	GpuParameterObjectType* types = mAlloc.Alloc<GpuParameterObjectType>(mBindingSlotCount);
	GpuBufferFormat* elementTypes = mAlloc.Alloc<GpuBufferFormat>(mBindingSlotCount);
	u32* elementArraySizes = mAlloc.Alloc<u32>(mBindingSlotCount);

	mLayouts = mAlloc.Alloc<VulkanDescriptorLayout*>(mSetCount);
	mSetExtraInfos = mAlloc.Alloc<SetExtraInfo>(mSetCount);

	if(bindings != nullptr)
		B3DZeroOut(bindings, mBindingSlotCount);

	if(types != nullptr)
		B3DZeroOut(types, mBindingSlotCount);

	if(elementTypes != nullptr)
		B3DZeroOut(elementTypes, mBindingSlotCount);

	if(elementArraySizes != nullptr)
		B3DZeroOut(elementArraySizes, mBindingSlotCount);

	u32 usedBindingSlotCount = 0;
	u32 usedResourceSlotCount = 0;
	for(u32 setIndex = 0; setIndex < mSetCount; setIndex++)
	{
		mSetExtraInfos[setIndex].SlotToUsedBindingSequentialIndex = mAlloc.Alloc<u32>(mSetInfos[setIndex].SlotCount);
		mSetExtraInfos[setIndex].SlotToUsedResourceSequentialIndex = mAlloc.Alloc<u32>(mSetInfos[setIndex].SlotCount);

		mLayoutInfos[setIndex].BindingCount = 0;
		mLayoutInfos[setIndex].ResourceCount = 0;
		mLayoutInfos[setIndex].Bindings = nullptr;
		mLayoutInfos[setIndex].Types = nullptr;
		mLayoutInfos[setIndex].ElementTypes = nullptr;
		mLayoutInfos[setIndex].ArraySizes = nullptr;

		for(u32 slotIndex = 0; slotIndex < mSetInfos[setIndex].SlotCount; slotIndex++)
		{
			if(mSetInfos[setIndex].SlotToSequentialResourceIndex[slotIndex] == ~0u)
			{
				mSetExtraInfos[setIndex].SlotToUsedBindingSequentialIndex[slotIndex] = ~0u;
				mSetExtraInfos[setIndex].SlotToUsedResourceSequentialIndex[slotIndex] = ~0u;

				continue;
			}

			const u32 arraySize = mSetInfos[setIndex].SlotArraySizes[slotIndex];

			VkDescriptorSetLayoutBinding& binding = bindings[usedBindingSlotCount];
			binding.binding = slotIndex;

			mSetExtraInfos[setIndex].SlotToUsedBindingSequentialIndex[slotIndex] = usedBindingSlotCount;
			mSetExtraInfos[setIndex].SlotToUsedResourceSequentialIndex[slotIndex] = usedResourceSlotCount;

			mLayoutInfos[setIndex].BindingCount++;
			mLayoutInfos[setIndex].ResourceCount += arraySize;

			usedBindingSlotCount++;
			usedResourceSlotCount += arraySize;
		}
	}

	u32 offset = 0;
	for(u32 setIndex = 0; setIndex < mSetCount; setIndex++)
	{
		mLayoutInfos[setIndex].Bindings = &bindings[offset];
		mLayoutInfos[setIndex].Types = &types[offset];
		mLayoutInfos[setIndex].ElementTypes = &elementTypes[offset];
		mLayoutInfos[setIndex].ArraySizes = &elementArraySizes[offset];

		offset += mLayoutInfos[setIndex].BindingCount;
	}

	VkShaderStageFlags stageFlagsLookup[6];
	stageFlagsLookup[GPT_VERTEX_PROGRAM] = VK_SHADER_STAGE_VERTEX_BIT;
	stageFlagsLookup[GPT_HULL_PROGRAM] = VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT;
	stageFlagsLookup[GPT_DOMAIN_PROGRAM] = VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT;
	stageFlagsLookup[GPT_GEOMETRY_PROGRAM] = VK_SHADER_STAGE_GEOMETRY_BIT;
	stageFlagsLookup[GPT_FRAGMENT_PROGRAM] = VK_SHADER_STAGE_FRAGMENT_BIT;
	stageFlagsLookup[GPT_COMPUTE_PROGRAM] = VK_SHADER_STAGE_COMPUTE_BIT;

	u32 numParamDescs = sizeof(mPerProgramParameterDescriptions) / sizeof(mPerProgramParameterDescriptions[0]);
	for(u32 i = 0; i < numParamDescs; i++)
	{
		const SPtr<GpuProgramParameterDescription>& paramDesc = mPerProgramParameterDescriptions[i];
		if(paramDesc == nullptr)
			continue;

		auto setUpBlockBindings = [&](auto& params, VkDescriptorType descType)
		{
			for(auto& entry : params)
			{
				const u32 usedBindingSequentialIndex = GetUsedBindingSequentialIndex(entry.second.Set, entry.second.Slot);
				B3D_ASSERT(usedBindingSequentialIndex != ~0u);

				VkDescriptorSetLayoutBinding& binding = bindings[usedBindingSequentialIndex];
				binding.descriptorCount = 1;
				binding.stageFlags |= stageFlagsLookup[i];
				binding.descriptorType = descType;
			}
		};

		auto setUpBindings = [&](auto& params, VkDescriptorType descType)
		{
			for(auto& entry : params)
			{
				const u32 usedBindingSequentialIndex = GetUsedBindingSequentialIndex(entry.second.Set, entry.second.Slot);
				B3D_ASSERT(usedBindingSequentialIndex != ~0u);

				VkDescriptorSetLayoutBinding& binding = bindings[usedBindingSequentialIndex];
				binding.descriptorCount = entry.second.ArraySize;
				binding.stageFlags |= stageFlagsLookup[i];
				binding.descriptorType = descType;

				types[usedBindingSequentialIndex] = entry.second.Type;
				elementTypes[usedBindingSequentialIndex] = entry.second.ElementType;
				elementArraySizes[usedBindingSequentialIndex] = entry.second.ArraySize;
			}
		};

		setUpBlockBindings(paramDesc->DataParameterBlocks, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC);
		setUpBindings(paramDesc->Textures, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE);
		setUpBindings(paramDesc->StorageTextures, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE);

		// Set up sampler bindings
		for(auto& entry : paramDesc->Samplers)
		{
			const u32 usedBindingSequentialIndex = GetUsedBindingSequentialIndex(entry.second.Set, entry.second.Slot);
			B3D_ASSERT(usedBindingSequentialIndex != ~0u);

			VkDescriptorSetLayoutBinding& binding = bindings[usedBindingSequentialIndex];

			// If we already assigned an image to this binding slot, then it's a combined image/sampler
			if(binding.descriptorType == VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE)
				binding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			else
			{
				binding.descriptorCount = entry.second.ArraySize;
				binding.stageFlags |= stageFlagsLookup[i];
				binding.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER;

				types[usedBindingSequentialIndex] = entry.second.Type;
				elementTypes[usedBindingSequentialIndex] = entry.second.ElementType;
				elementArraySizes[usedBindingSequentialIndex] = entry.second.ArraySize;
			}
		}

		// Set up buffer bindings
		for(auto& entry : paramDesc->Buffers)
		{
			const u32 usedBindingSequentialIndex = GetUsedBindingSequentialIndex(entry.second.Set, entry.second.Slot);
			B3D_ASSERT(usedBindingSequentialIndex != ~0u);

			VkDescriptorSetLayoutBinding& binding = bindings[usedBindingSequentialIndex];
			binding.descriptorCount = entry.second.ArraySize;
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
				binding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC;
				break;
			}

			types[usedBindingSequentialIndex] = entry.second.Type;
			elementTypes[usedBindingSequentialIndex] = entry.second.ElementType;
			elementArraySizes[usedBindingSequentialIndex] = entry.second.ArraySize;
		}
	}

	// Allocate layouts
	VulkanDescriptorManager& descriptorManager = mGpuDevice.GetDescriptorManager();
	for(u32 setIndex = 0; setIndex < mSetCount; setIndex++)
		mLayouts[setIndex] = descriptorManager.GetLayout(mLayoutInfos[setIndex].Bindings, mLayoutInfos[setIndex].BindingCount);
}
