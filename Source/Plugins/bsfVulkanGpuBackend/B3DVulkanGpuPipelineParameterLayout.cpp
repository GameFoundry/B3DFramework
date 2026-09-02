//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DVulkanGpuPipelineParameterLayout.h"
#include "B3DVulkanUtility.h"
#include "B3DVulkanGpuDevice.h"
#include "GpuBackend/B3DGpuPushConstants.h"
#include "GpuBackend/B3DGpuProgramParameterDescription.h"

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

	using PerTypeUniformArray = std::decay_t<decltype(mUniformsPerType[0])>;
	auto fnSetUniformBindings = [this](const PerTypeUniformArray& uniforms, VkDescriptorType descriptorType)
	{
		for(const auto& entry : uniforms)
		{
			const u32 usedBindingSequentialIndex = GetUsedBindingSequentialIndex(entry->Slot);
			B3D_ASSERT(usedBindingSequentialIndex != ~0u);

			VkDescriptorSetLayoutBinding& binding = mBindings[usedBindingSequentialIndex];
			binding.descriptorCount = 1;
			binding.stageFlags |= VulkanUtility::GetShaderStages(entry->Usage);
			binding.descriptorType = descriptorType;
		}
	};

	auto fnSetBindings = [this](const PerTypeUniformArray& uniforms, VkDescriptorType descriptorType)
	{
		for(const auto& entry : uniforms)
		{
			const u32 usedBindingSequentialIndex = GetUsedBindingSequentialIndex(entry->Slot);
			B3D_ASSERT(usedBindingSequentialIndex != ~0u);

			VkDescriptorSetLayoutBinding& binding = mBindings[usedBindingSequentialIndex];
			binding.descriptorCount = entry->ArraySize;
			binding.stageFlags |= VulkanUtility::GetShaderStages(entry->Usage);
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
			binding.stageFlags |= VulkanUtility::GetShaderStages(entry->Usage);
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
		binding.stageFlags |= VulkanUtility::GetShaderStages(entry->Usage);

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
	: GpuPipelineParameterLayout(gpuDevice, createInformation)
{
	const u32 pushConstantBufferSize = GetPushConstantBufferSize();
	if(pushConstantBufferSize == 0)
		return;

	if(!B3D_ENSURE_LOG((pushConstantBufferSize & 3u) == 0 && pushConstantBufferSize <= kMaxPushConstantSizeInBytes,
		"Vulkan pipeline declares an invalid push-constant buffer size of {0} bytes; expected a four-byte-aligned size no greater than {1} bytes.", pushConstantBufferSize, kMaxPushConstantSizeInBytes))
	{
		return;
	}

	if(!B3D_ENSURE_LOG(pushConstantBufferSize <= gpuDevice.GetCapabilities().MaximumPushConstantSize,
		"Vulkan pipeline requires {0} push-constant bytes, but the device supports only {1} bytes.", pushConstantBufferSize, gpuDevice.GetCapabilities().MaximumPushConstantSize))
	{
		return;
	}

	GpuProgramStageBits pushConstantStages = GpuProgramStageBit::None;
	if(createInformation.Vertex != nullptr && createInformation.Vertex->PushConstantBufferSize != 0)
		pushConstantStages |= GpuProgramStageBit::Vertex;

	if(createInformation.Fragment != nullptr && createInformation.Fragment->PushConstantBufferSize != 0)
		pushConstantStages |= GpuProgramStageBit::Fragment;

	if(createInformation.Hull != nullptr && createInformation.Hull->PushConstantBufferSize != 0)
		pushConstantStages |= GpuProgramStageBit::Hull;

	if(createInformation.Domain != nullptr && createInformation.Domain->PushConstantBufferSize != 0)
		pushConstantStages |= GpuProgramStageBit::Domain;

	if(createInformation.Geometry != nullptr && createInformation.Geometry->PushConstantBufferSize != 0)
		pushConstantStages |= GpuProgramStageBit::Geometry;

	if(createInformation.Compute != nullptr && createInformation.Compute->PushConstantBufferSize != 0)
		pushConstantStages |= GpuProgramStageBit::Compute;

	VkPushConstantRange pushConstantRange;
	pushConstantRange.stageFlags = VulkanUtility::GetShaderStages(pushConstantStages);
	pushConstantRange.offset = 0;
	pushConstantRange.size = pushConstantBufferSize;

	if(!B3D_ENSURE_LOG(pushConstantRange.stageFlags != 0, "Vulkan pipeline declares push constants without a shader stage."))
		return;

	mPushConstantRange = pushConstantRange;
}


