//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsVulkanGpuParams.h"
#include "BsGpuParamDesc.h"

namespace BansheeEngine
{
	VulkanGpuParams::VulkanGpuParams(const GPU_PARAMS_DESC& desc, GpuDeviceFlags deviceMask)
		:GpuParamsCore(desc, deviceMask)
	{
		UINT32 numBindings = 0;
		UINT32 numSets = 0;
		
		UINT32 numElementTypes = (UINT32)ElementType::Count;
		for (UINT32 i = 0; i < numElementTypes; i++)
		{
			numBindings += mNumElements[i];
			numSets = std::max(numSets, mNumSets[i]);
		}

		UINT32 bindingsPerSetBytes = sizeof(UINT32) * numSets;
		UINT32* bindingsPerSet = (UINT32*)bs_stack_alloc(bindingsPerSetBytes);

		UINT32 bindingsSize = sizeof(VkDescriptorSetLayoutBinding) * numBindings;
		VkDescriptorSetLayoutBinding* bindings = (VkDescriptorSetLayoutBinding*)bs_stack_alloc(bindingsSize);
		memset(bindings, 0, bindingsSize);

		UINT32 globalBindingIdx = 0;
		for(UINT32 i = 0; i < numSets; i++)
		{
			bindingsPerSet[i] = 0;
			for(UINT32 j = 0; j < numElementTypes; j++)
			{
				if (i >= mNumSets[j])
					continue;

				UINT32 start = mOffsets[j][i];

				UINT32 end;
				if (i < (mNumSets[j] - 1))
					end = mOffsets[j][i + 1];
				else
					end = mNumElements[j];

				UINT32 elementsInSet = end - start;
				for(UINT32 k = 0; k < elementsInSet; k++)
				{
					VkDescriptorSetLayoutBinding& binding = bindings[globalBindingIdx + k];
					binding.binding = bindingsPerSet[i] + k;
				};

				globalBindingIdx += elementsInSet;
				bindingsPerSet[i] += elementsInSet;
			}
		}

		UINT32* bindingOffsets = (UINT32*)bs_stack_alloc(sizeof(UINT32) * numSets);
		if (numSets > 0)
		{
			bindingOffsets[0] = 0;

			for (UINT32 i = 1; i < numSets; i++)
				bindingOffsets[i] = bindingsPerSet[i - 1];
		}

		VkShaderStageFlags stageFlagsLookup[6];
		stageFlagsLookup[GPT_VERTEX_PROGRAM] = VK_SHADER_STAGE_VERTEX_BIT;
		stageFlagsLookup[GPT_HULL_PROGRAM] = VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT;
		stageFlagsLookup[GPT_DOMAIN_PROGRAM] = VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT;
		stageFlagsLookup[GPT_GEOMETRY_PROGRAM] = VK_SHADER_STAGE_GEOMETRY_BIT;
		stageFlagsLookup[GPT_FRAGMENT_PROGRAM] = VK_SHADER_STAGE_FRAGMENT_BIT;
		stageFlagsLookup[GPT_COMPUTE_PROGRAM] = VK_SHADER_STAGE_COMPUTE_BIT;

		UINT32 numParamDescs = sizeof(mParamDescs) / sizeof(mParamDescs[0]);
		for (UINT32 i = 0; i < numParamDescs; i++)
		{
			const SPtr<GpuParamDesc>& paramDesc = mParamDescs[i];
			if (paramDesc == nullptr)
				continue;

			auto setUpBindings = [&](auto& params, VkDescriptorType descType)
			{
				for (auto& entry : params)
				{
					UINT32 bindingIdx = bindingOffsets[entry.second.set] + entry.second.slot;

					VkDescriptorSetLayoutBinding& binding = bindings[bindingIdx];
					binding.descriptorCount = 1;
					binding.stageFlags |= stageFlagsLookup[i];
					binding.descriptorType = descType;
				}
			};

			// Note: Assuming all textures and samplers use the same set/slot combination, and that they're combined
			setUpBindings(paramDesc->paramBlocks, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);
			setUpBindings(paramDesc->textures, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
			setUpBindings(paramDesc->loadStoreTextures, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE);
			setUpBindings(paramDesc->buffers, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
			setUpBindings(paramDesc->samplers, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
		}


		for (UINT32 i = 0; i < numSets; i++)
		{
		}

		// TODO - Actually allocate and create layouts and sets
		// TODO - Create sets per-device
		// TODO - Prepare write descs and see what's the best way to update them

		bs_stack_free(bindingOffsets);
		bs_stack_free(bindings);
		bs_stack_free(bindingsPerSet);

		


		// Note: Set layout creation should be moved up to Shader, since one layout can be shared between multiple
		// GPU params. Right now we take the easy route and just create a new layout every time (meaning there's a lot of
		// duplicates).
	}

	VulkanGpuParams::~VulkanGpuParams()
	{
		// TODO - Need to wait to ensure it isn't used on the GPU anymore

		// TODO - CLean up mSets
		// - Queue for destroy, remember fence counters for all available queues, only destroy after all queues execute?
		// - Or ensure the object knows which queue it was used on?
	}

	void VulkanGpuParams::setParamBlockBuffer(UINT32 set, UINT32 slot, const ParamsBufferType& paramBlockBuffer)
	{
		GpuParamsCore::setParamBlockBuffer(set, slot, paramBlockBuffer);
	}

	void VulkanGpuParams::setTexture(UINT32 set, UINT32 slot, const TextureType& texture)
	{
		GpuParamsCore::setTexture(set, slot, texture);
	}

	void VulkanGpuParams::setLoadStoreTexture(UINT32 set, UINT32 slot, const TextureType& texture, const TextureSurface& surface)
	{
		GpuParamsCore::setLoadStoreTexture(set, slot, texture, surface);
	}

	void VulkanGpuParams::setBuffer(UINT32 set, UINT32 slot, const BufferType& buffer)
	{
		GpuParamsCore::setBuffer(set, slot, buffer);
	}

	void VulkanGpuParams::setSamplerState(UINT32 set, UINT32 slot, const SamplerType& sampler)
	{
		GpuParamsCore::setSamplerState(set, slot, sampler);
	}

	void VulkanGpuParams::setLoadStoreSurface(UINT32 set, UINT32 slot, const TextureSurface& surface)
	{
		GpuParamsCore::setLoadStoreSurface(set, slot, surface);
	}
}