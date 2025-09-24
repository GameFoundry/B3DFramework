//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DVulkanGpuPipelineParameterLayout.h"
#include "B3DVulkanUtility.h"
#include "B3DVulkanGpuDevice.h"
#include "RenderAPI/B3DGpuProgramParameterDescription.h"

using namespace b3d;
using namespace b3d::render;

VulkanGpuPipelineParameterLayout::VulkanGpuPipelineParameterLayout(VulkanGpuDevice& gpuDevice, const GpuPipelineParameterLayoutCreateInformation& createInformation)
	: GpuPipelineParameterLayout(createInformation), mGpuDevice(gpuDevice), mLayouts(), mLayoutInfos()
{}

void VulkanGpuPipelineParameterLayout::Initialize()
{
	const u32 setCount = (u32)mSets.size();

	u32 totalSlotCount = 0;
	for(u32 setIndex = 0; setIndex < setCount; setIndex++)
		totalSlotCount += (u32)mSets[setIndex].Uniforms.size();

	mAlloc.Reserve<VkDescriptorSetLayoutBinding>(mBindingCount)
		.Reserve<GpuParameterObjectType>(mBindingCount)
		.Reserve<GpuBufferFormat>(mBindingCount)
		.Reserve<GpuBufferFormat>(mBindingCount)
		.Reserve<LayoutInfo>(setCount)
		.Reserve<VulkanDescriptorLayout*>(setCount)
		.Reserve<SetExtraInfo>(setCount)
		.Reserve<u32>(totalSlotCount)
		.Reserve<u32>(totalSlotCount)
		.Init();

	mLayoutInfos = mAlloc.Alloc<LayoutInfo>(setCount);
	VkDescriptorSetLayoutBinding* bindings = mAlloc.Alloc<VkDescriptorSetLayoutBinding>(mBindingCount);
	GpuParameterObjectType* types = mAlloc.Alloc<GpuParameterObjectType>(mBindingCount);
	GpuBufferFormat* elementTypes = mAlloc.Alloc<GpuBufferFormat>(mBindingCount);
	u32* elementArraySizes = mAlloc.Alloc<u32>(mBindingCount);

	mLayouts = mAlloc.Alloc<VulkanDescriptorLayout*>(setCount);
	mSetExtraInfos = mAlloc.Alloc<SetExtraInfo>(setCount);

	if(bindings != nullptr)
		B3DZeroOut(bindings, mBindingCount);

	if(types != nullptr)
		B3DZeroOut(types, mBindingCount);

	if(elementTypes != nullptr)
		B3DZeroOut(elementTypes, mBindingCount);

	if(elementArraySizes != nullptr)
		B3DZeroOut(elementArraySizes, mBindingCount);

	u32 usedBindingSlotCount = 0;
	u32 usedResourceSlotCount = 0;
	for(u32 setIndex = 0; setIndex < setCount; setIndex++)
	{
		mSetExtraInfos[setIndex].SlotToUsedBindingSequentialIndex = mAlloc.Alloc<u32>((u32)mSets[setIndex].Uniforms.Size());
		mSetExtraInfos[setIndex].SlotToUsedResourceSequentialIndex = mAlloc.Alloc<u32>((u32)mSets[setIndex].Uniforms.Size());

		mLayoutInfos[setIndex].BindingCount = 0;
		mLayoutInfos[setIndex].ResourceCount = 0;
		mLayoutInfos[setIndex].Bindings = nullptr;
		mLayoutInfos[setIndex].Types = nullptr;
		mLayoutInfos[setIndex].ElementTypes = nullptr;
		mLayoutInfos[setIndex].ArraySizes = nullptr;

		for(u32 slotIndex = 0; slotIndex < (u32)mSets[setIndex].Uniforms.Size(); slotIndex++)
		{
			UniformInformation* uniformInformation = mSets[setIndex].Uniforms[slotIndex];

			if(uniformInformation == nullptr)
			{
				mSetExtraInfos[setIndex].SlotToUsedBindingSequentialIndex[slotIndex] = ~0u;
				mSetExtraInfos[setIndex].SlotToUsedResourceSequentialIndex[slotIndex] = ~0u;

				continue;
			}

			const u32 arraySize = uniformInformation->ArraySize;

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
	for(u32 setIndex = 0; setIndex < setCount; setIndex++)
	{
		mLayoutInfos[setIndex].Bindings = &bindings[offset];
		mLayoutInfos[setIndex].Types = &types[offset];
		mLayoutInfos[setIndex].ElementTypes = &elementTypes[offset];
		mLayoutInfos[setIndex].ArraySizes = &elementArraySizes[offset];

		offset += mLayoutInfos[setIndex].BindingCount;
	}

	auto fnGetShaderStageFlags = [](const GpuProgramStageBits& bits)
	{
		VkShaderStageFlags flags = 0;
		if(bits.IsSet(GpuProgramStageBit::Vertex))
			flags |= VK_SHADER_STAGE_VERTEX_BIT;

		if(bits.IsSet(GpuProgramStageBit::Fragment))
			flags |= VK_SHADER_STAGE_FRAGMENT_BIT;

		if(bits.IsSet(GpuProgramStageBit::Hull))
			flags |= VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT;

		if(bits.IsSet(GpuProgramStageBit::Domain))
			flags |= VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT;

		if(bits.IsSet(GpuProgramStageBit::Geometry))
			flags |= VK_SHADER_STAGE_GEOMETRY_BIT;

		if(bits.IsSet(GpuProgramStageBit::Compute))
			flags |= VK_SHADER_STAGE_COMPUTE_BIT;

		return flags;
	};

	using PerTypeUniformArray = std::decay_t<decltype(mUniformsPerType[0])>;
	auto setUpBlockBindings = [this, &bindings, fnGetShaderStageFlags](const PerTypeUniformArray& uniforms, VkDescriptorType descriptorType)
	{
		for(const auto& entry : uniforms)
		{
			const u32 usedBindingSequentialIndex = GetUsedBindingSequentialIndex(entry->Set, entry->Slot);
			B3D_ASSERT(usedBindingSequentialIndex != ~0u);

			VkDescriptorSetLayoutBinding& binding = bindings[usedBindingSequentialIndex];
			binding.descriptorCount = 1;
			binding.stageFlags |= fnGetShaderStageFlags(entry->Usage);
			binding.descriptorType = descriptorType;
		}
	};

	auto setUpBindings = [this, &bindings, &types, &elementTypes, &elementArraySizes, fnGetShaderStageFlags](const PerTypeUniformArray& uniforms, VkDescriptorType descriptorType)
	{
		for(const auto& entry : uniforms)
		{
			const u32 usedBindingSequentialIndex = GetUsedBindingSequentialIndex(entry->Set, entry->Slot);
			B3D_ASSERT(usedBindingSequentialIndex != ~0u);

			VkDescriptorSetLayoutBinding& binding = bindings[usedBindingSequentialIndex];
			binding.descriptorCount = entry->ArraySize;
			binding.stageFlags |= fnGetShaderStageFlags(entry->Usage);
			binding.descriptorType = descriptorType;

			types[usedBindingSequentialIndex] = entry->ObjectType;
			elementTypes[usedBindingSequentialIndex] = entry->ElementType;
			elementArraySizes[usedBindingSequentialIndex] = entry->ArraySize;
		}
	};

	setUpBlockBindings(mUniformsPerType[(u32)GpuParameterType::UniformBuffer], VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC);
	setUpBindings(mUniformsPerType[(u32)GpuParameterType::SampledTexture], VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE);
	setUpBindings(mUniformsPerType[(u32)GpuParameterType::StorageTexture], VK_DESCRIPTOR_TYPE_STORAGE_IMAGE);

	// Set up sampler bindings
	for(auto& entry : mUniformsPerType[(u32)GpuParameterType::Sampler])
	{
		const u32 usedBindingSequentialIndex = GetUsedBindingSequentialIndex(entry->Set, entry->Slot);
		B3D_ASSERT(usedBindingSequentialIndex != ~0u);

		VkDescriptorSetLayoutBinding& binding = bindings[usedBindingSequentialIndex];

		// If we already assigned an image to this binding slot, then it's a combined image/sampler
		if(binding.descriptorType == VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE)
			binding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		else
		{
			binding.descriptorCount = entry->ArraySize;
			binding.stageFlags |= fnGetShaderStageFlags(entry->Usage);
			binding.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER;

			types[usedBindingSequentialIndex] = entry->ObjectType;
			elementTypes[usedBindingSequentialIndex] = entry->ElementType;
			elementArraySizes[usedBindingSequentialIndex] = entry->ArraySize;
		}
	}

	// Set up buffer bindings
	for(auto& entry : mUniformsPerType[(u32)GpuParameterType::StorageBuffer])
	{
		const u32 usedBindingSequentialIndex = GetUsedBindingSequentialIndex(entry->Set, entry->Slot);
		B3D_ASSERT(usedBindingSequentialIndex != ~0u);

		VkDescriptorSetLayoutBinding& binding = bindings[usedBindingSequentialIndex];
		binding.descriptorCount = entry->ArraySize;
		binding.stageFlags |= fnGetShaderStageFlags(entry->Usage);

		switch(entry->ObjectType)
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

		types[usedBindingSequentialIndex] = entry->ObjectType;
		elementTypes[usedBindingSequentialIndex] = entry->ElementType;
		elementArraySizes[usedBindingSequentialIndex] = entry->ArraySize;
	}

	// Allocate layouts
	VulkanDescriptorManager& descriptorManager = mGpuDevice.GetDescriptorManager();
	for(u32 setIndex = 0; setIndex < setCount; setIndex++)
		mLayouts[setIndex] = descriptorManager.GetLayout(mLayoutInfos[setIndex].Bindings, mLayoutInfos[setIndex].BindingCount);
}
