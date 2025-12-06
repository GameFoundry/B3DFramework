//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DVulkanGpuPipelineParameterLayout.h"
#include "B3DVulkanUtility.h"
#include "B3DVulkanGpuDevice.h"
#include "RenderAPI/B3DGpuProgramParameterDescription.h"

using namespace b3d;
using namespace b3d::render;

VulkanGpuPipelineParameterSetLayout::VulkanGpuPipelineParameterSetLayout(VulkanGpuDevice& gpuDevice, const GpuProgramParameterDescription& parameterDescription)
	: GpuPipelineParameterSetLayout(parameterDescription), mGpuDevice(gpuDevice)
{
	const u32 slotCount = (u32)mUniforms.size();

	mAllocator.Reserve<VkDescriptorSetLayoutBinding>(mBindingCount)
		.Reserve<GpuParameterObjectType>(mBindingCount)
		.Reserve<GpuBufferFormat>(mBindingCount)
		.Reserve<GpuBufferFormat>(mBindingCount)
		.Reserve<u32>(slotCount)
		.Reserve<u32>(slotCount)
		.Initialize();

	mSlotToUsedBindingSequentialIndex = mAllocator.Allocate<u32>(slotCount);
	mSlotToUsedResourceSequentialIndex = mAllocator.Allocate<u32>(slotCount);
	mBindings = mAllocator.Allocate<VkDescriptorSetLayoutBinding>(mBindingCount, true);
	mTypes = mAllocator.Allocate<GpuParameterObjectType>(mBindingCount, true);
	mElementTypes = mAllocator.Allocate<GpuBufferFormat>(mBindingCount, true);
	mArraySizes = mAllocator.Allocate<u32>(mBindingCount, true);

	u32 usedBindingSlotCount = 0;
	u32 usedResourceSlotCount = 0;
	for(u32 slotIndex = 0; slotIndex < slotCount; slotIndex++)
	{
		UniformInformation* uniformInformation = mUniforms[slotIndex];

		if(uniformInformation == nullptr)
		{
			mSlotToUsedBindingSequentialIndex[slotIndex] = ~0u;
			mSlotToUsedResourceSequentialIndex[slotIndex] = ~0u;

			continue;
		}

		const u32 arraySize = uniformInformation->ArraySize;

		VkDescriptorSetLayoutBinding& binding = mBindings[usedBindingSlotCount];
		binding.binding = slotIndex;

		mSlotToUsedBindingSequentialIndex[slotIndex] = usedBindingSlotCount;
		mSlotToUsedResourceSequentialIndex[slotIndex] = usedResourceSlotCount;

		usedBindingSlotCount++;
		usedResourceSlotCount += arraySize;
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
	auto fnSetUniformBindings = [this, fnGetShaderStageFlags](const PerTypeUniformArray& uniforms, VkDescriptorType descriptorType)
	{
		for(const auto& entry : uniforms)
		{
			const u32 usedBindingSequentialIndex = GetUsedBindingSequentialIndex(entry->Slot);
			B3D_ASSERT(usedBindingSequentialIndex != ~0u);

			VkDescriptorSetLayoutBinding& binding = mBindings[usedBindingSequentialIndex];
			binding.descriptorCount = 1;
			binding.stageFlags |= fnGetShaderStageFlags(entry->Usage);
			binding.descriptorType = descriptorType;
		}
	};

	auto fnSetBindings = [this, fnGetShaderStageFlags](const PerTypeUniformArray& uniforms, VkDescriptorType descriptorType)
	{
		for(const auto& entry : uniforms)
		{
			const u32 usedBindingSequentialIndex = GetUsedBindingSequentialIndex(entry->Slot);
			B3D_ASSERT(usedBindingSequentialIndex != ~0u);

			VkDescriptorSetLayoutBinding& binding = mBindings[usedBindingSequentialIndex];
			binding.descriptorCount = entry->ArraySize;
			binding.stageFlags |= fnGetShaderStageFlags(entry->Usage);
			binding.descriptorType = descriptorType;

			mTypes[usedBindingSequentialIndex] = entry->ObjectType;
			mElementTypes[usedBindingSequentialIndex] = entry->ElementType;
			mArraySizes[usedBindingSequentialIndex] = entry->ArraySize;
		}
	};

	fnSetUniformBindings(mUniformsPerType[(u32)GpuParameterType::UniformBuffer], VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC);
	fnSetBindings(mUniformsPerType[(u32)GpuParameterType::SampledTexture], VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE);
	fnSetBindings(mUniformsPerType[(u32)GpuParameterType::StorageTexture], VK_DESCRIPTOR_TYPE_STORAGE_IMAGE);

	// Set up sampler bindings
	for(auto& entry : mUniformsPerType[(u32)GpuParameterType::Sampler])
	{
		const u32 usedBindingSequentialIndex = GetUsedBindingSequentialIndex(entry->Slot);
		B3D_ASSERT(usedBindingSequentialIndex != ~0u);

		VkDescriptorSetLayoutBinding& binding = mBindings[usedBindingSequentialIndex];

		// If we already assigned an image to this binding slot, then it's a combined image/sampler
		if(binding.descriptorType == VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE)
			binding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		else
		{
			binding.descriptorCount = entry->ArraySize;
			binding.stageFlags |= fnGetShaderStageFlags(entry->Usage);
			binding.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER;

			mTypes[usedBindingSequentialIndex] = entry->ObjectType;
			mElementTypes[usedBindingSequentialIndex] = entry->ElementType;
			mArraySizes[usedBindingSequentialIndex] = entry->ArraySize;
		}
	}

	// Set up buffer bindings
	for(auto& entry : mUniformsPerType[(u32)GpuParameterType::StorageBuffer])
	{
		const u32 usedBindingSequentialIndex = GetUsedBindingSequentialIndex(entry->Slot);
		B3D_ASSERT(usedBindingSequentialIndex != ~0u);

		VkDescriptorSetLayoutBinding& binding = mBindings[usedBindingSequentialIndex];
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

		mTypes[usedBindingSequentialIndex] = entry->ObjectType;
		mElementTypes[usedBindingSequentialIndex] = entry->ElementType;
		mArraySizes[usedBindingSequentialIndex] = entry->ArraySize;
	}

	// Allocate layout
	VulkanDescriptorManager& descriptorManager = mGpuDevice.GetDescriptorManager();
	mLayout = descriptorManager.GetLayout(mBindings);
}

VulkanGpuPipelineParameterLayout::VulkanGpuPipelineParameterLayout(VulkanGpuDevice& gpuDevice, const GpuPipelineParameterLayoutCreateInformation& createInformation)
	: GpuPipelineParameterLayout(createInformation), mGpuDevice(gpuDevice)
{}

SPtr<GpuPipelineParameterSetLayout> VulkanGpuPipelineParameterLayout::CreateSet(const GpuProgramParameterDescription& parameterDescription) const
{
	return B3DMakeShared<VulkanGpuPipelineParameterSetLayout>(mGpuDevice, parameterDescription);
}


