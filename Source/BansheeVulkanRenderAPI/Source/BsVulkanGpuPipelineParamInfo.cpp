//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsVulkanGpuPipelineParamInfo.h"
#include "BsVulkanUtility.h"
#include "BsVulkanRenderAPI.h"
#include "BsVulkanDevice.h"
#include "BsGpuParamDesc.h"

namespace bs
{
	VulkanGpuPipelineParamInfo::VulkanGpuPipelineParamInfo(const GPU_PIPELINE_PARAMS_DESC& desc, GpuDeviceFlags deviceMask)
		:GpuPipelineParamInfoCore(desc, deviceMask), mDeviceMask(deviceMask), mLayouts(), mLayoutInfos(), mData(nullptr)
	{ }

	VulkanGpuPipelineParamInfo::~VulkanGpuPipelineParamInfo()
	{
		bs_free(mData);
	}

	void VulkanGpuPipelineParamInfo::initialize()
	{
		VulkanRenderAPI& rapi = static_cast<VulkanRenderAPI&>(RenderAPICore::instance());

		VulkanDevice* devices[BS_MAX_DEVICES];
		VulkanUtility::getDevices(rapi, mDeviceMask, devices);

		UINT32 numDevices = 0;
		for (UINT32 i = 0; i < BS_MAX_DEVICES; i++)
		{
			if (devices[i] != nullptr)
				numDevices++;
		}

		UINT32 bindingsSize = sizeof(VkDescriptorSetLayoutBinding) * mNumElements;
		UINT32 layoutInfoSize = sizeof(LayoutInfo) * mNumSets;
		UINT32 layoutsSize = sizeof(VulkanDescriptorLayout*) * mNumSets;

		mData = (UINT8*)bs_alloc(bindingsSize + layoutInfoSize + layoutsSize * numDevices);
		UINT8* dataPtr = mData;

		mLayoutInfos = (LayoutInfo*)dataPtr;
		dataPtr += layoutInfoSize;

		VkDescriptorSetLayoutBinding* bindings = (VkDescriptorSetLayoutBinding*)dataPtr;
		dataPtr += bindingsSize;

		for (UINT32 i = 0; i < BS_MAX_DEVICES; i++)
		{
			if (devices[i] == nullptr)
			{
				mLayouts[i] = nullptr;
				continue;
			}

			mLayouts[i] = (VulkanDescriptorLayout**)dataPtr;
			dataPtr += layoutsSize;
		}

		memset(bindings, 0, bindingsSize);

		UINT32 globalBindingIdx = 0;
		for (UINT32 i = 0; i < mNumSets; i++)
		{
			mLayoutInfos[i].numBindings = 0;
			mLayoutInfos[i].bindings = nullptr;

			for (UINT32 j = 0; j < mSetInfos[i].numSlots; j++)
			{
				VkDescriptorSetLayoutBinding& binding = bindings[globalBindingIdx + j];
				binding.binding = mLayoutInfos[i].numBindings + j;
			}

			globalBindingIdx += mSetInfos[i].numSlots;
			mLayoutInfos[i].numBindings += mSetInfos[i].numSlots;
		}

		UINT32 offset = 0;
		for (UINT32 i = 0; i < mNumSets; i++)
		{
			mLayoutInfos[i].bindings = &bindings[offset];
			offset += mLayoutInfos[i].numBindings;
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
					VkDescriptorSetLayoutBinding& binding = mLayoutInfos[entry.second.set].bindings[entry.second.slot];
					binding.descriptorCount = 1;
					binding.stageFlags |= stageFlagsLookup[i];
					binding.descriptorType = descType;
				}
			};

			// Note: Assuming all textures and samplers use the same set/slot combination, and that they're combined
			setUpBindings(paramDesc->paramBlocks, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);
			setUpBindings(paramDesc->textures, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
			setUpBindings(paramDesc->loadStoreTextures, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE);
			setUpBindings(paramDesc->samplers, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);

			// Set up buffer bindings
			for (auto& entry : paramDesc->buffers)
			{
				bool isLoadStore = entry.second.type != GPOT_BYTE_BUFFER &&
					entry.second.type != GPOT_STRUCTURED_BUFFER;

				VkDescriptorSetLayoutBinding& binding = mLayoutInfos[entry.second.set].bindings[entry.second.slot];
				binding.descriptorCount = 1;
				binding.stageFlags |= stageFlagsLookup[i];
				binding.descriptorType = isLoadStore ? VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER : VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER;
			}
		}

		// Allocate layouts per-device
		for (UINT32 i = 0; i < BS_MAX_DEVICES; i++)
		{
			if (mLayouts[i] == nullptr)
				continue;

			VulkanDescriptorManager& descManager = devices[i]->getDescriptorManager();
			for (UINT32 j = 0; j < mNumSets; j++)
				mLayouts[i][j] = descManager.getLayout(mLayoutInfos[j].bindings, mLayoutInfos[j].numBindings);
		}
	}

	VulkanDescriptorLayout* VulkanGpuPipelineParamInfo::getLayout(UINT32 deviceIdx, UINT32 layoutIdx) const
	{
		if (deviceIdx >= BS_MAX_DEVICES || mLayouts[deviceIdx] == nullptr)
			return nullptr;

		return mLayouts[deviceIdx][layoutIdx];
	}
}