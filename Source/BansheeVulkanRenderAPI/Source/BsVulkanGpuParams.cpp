//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsVulkanGpuParams.h"
#include "BsVulkanUtility.h"
#include "BsVulkanRenderAPI.h"
#include "BsVulkanDevice.h"
#include "BsVulkanGpuParamBlockBuffer.h"
#include "BsVulkanGpuBuffer.h"
#include "BsVulkanTexture.h"
#include "BsVulkanHardwareBuffer.h"
#include "BsVulkanDescriptorSet.h"
#include "BsVulkanSamplerState.h"
#include "BsVulkanGpuBuffer.h"
#include "BsVulkanCommandBuffer.h"
#include "BsGpuParamDesc.h"

namespace BansheeEngine
{
	VulkanGpuParams::VulkanGpuParams(const GPU_PARAMS_DESC& desc, GpuDeviceFlags deviceMask)
		: GpuParamsCore(desc, deviceMask), mPerDeviceData(), mDeviceMask(deviceMask), mData(nullptr), mSetsDirty(nullptr)
	{
		// Generate all required bindings
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
		for (UINT32 i = 0; i < numSets; i++)
		{
			bindingsPerSet[i] = 0;
			for (UINT32 j = 0; j < numElementTypes; j++)
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
				for (UINT32 k = 0; k < elementsInSet; k++)
				{
					VkDescriptorSetLayoutBinding& binding = bindings[globalBindingIdx + k];
					binding.binding = bindingsPerSet[i] + k;
				}

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
			setUpBindings(paramDesc->samplers, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);

			// Set up buffer bindings
			for (auto& entry : paramDesc->buffers)
			{
				bool isLoadStore = entry.second.type != GPOT_BYTE_BUFFER &&
					entry.second.type != GPOT_STRUCTURED_BUFFER;

				UINT32 bindingIdx = bindingOffsets[entry.second.set] + entry.second.slot;

				VkDescriptorSetLayoutBinding& binding = bindings[bindingIdx];
				binding.descriptorCount = 1;
				binding.stageFlags |= stageFlagsLookup[i];
				binding.descriptorType = isLoadStore ? VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER : VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER;
			}
		}

		VulkanRenderAPI& rapi = static_cast<VulkanRenderAPI&>(RenderAPICore::instance());
		VulkanDevice* devices[BS_MAX_DEVICES];

		// Allocate layouts per-device
		UINT32 numDevices = 0;
		for (UINT32 i = 0; i < BS_MAX_DEVICES; i++)
		{
			if (VulkanUtility::isDeviceIdxSet(rapi, i, deviceMask))
				devices[i] = rapi._getDevice(i).get();
			else
				devices[i] = nullptr;

			numDevices++;
		}

		// Note: I'm assuming a single WriteInfo per binding, but if arrays sizes larger than 1 are eventually supported
		// I'll need to adjust the code.
		UINT32 setsDirtyBytes = sizeof(bool) * numSets;
		UINT32 perSetBytes = sizeof(PerSetData) * numSets;
		UINT32 writeSetInfosBytes = sizeof(VkWriteDescriptorSet) * numBindings;
		UINT32 writeInfosBytes = sizeof(WriteInfo) * numBindings;
		mData = (UINT8*)bs_alloc(setsDirtyBytes + (perSetBytes + writeSetInfosBytes + writeInfosBytes) * numDevices);
		UINT8* dataIter = mData;

		mSetsDirty = (bool*)dataIter;
		memset(mSetsDirty, 1, setsDirtyBytes);
		dataIter += setsDirtyBytes;

		for(UINT32 i = 0; i < BS_MAX_DEVICES; i++)
		{
			if(devices[i] == nullptr)
			{
				mPerDeviceData[i].numSets = 0;
				mPerDeviceData[i].perSetData = nullptr;

				continue;
			}

			mPerDeviceData[i].numSets = numSets;
			mPerDeviceData[i].perSetData = (PerSetData*)dataIter;
			dataIter += sizeof(perSetBytes);

			VulkanDescriptorManager& descManager = devices[i]->getDescriptorManager();

			UINT32 bindingOffset = 0;
			for (UINT32 j = 0; j < numSets; j++)
			{
				UINT32 numBindingsPerSet = bindingsPerSet[j];

				PerSetData& perSetData = mPerDeviceData[i].perSetData[j];
				perSetData.writeSetInfos = (VkWriteDescriptorSet*)dataIter;
				dataIter += sizeof(VkWriteDescriptorSet) * numBindingsPerSet;

				perSetData.writeInfos = (WriteInfo*)dataIter;
				dataIter += sizeof(WriteInfo) * numBindingsPerSet;

				VkDescriptorSetLayoutBinding* perSetBindings = &bindings[bindingOffset];
				perSetData.layout = descManager.getLayout(perSetBindings, numBindingsPerSet);
				perSetData.set = descManager.createSet(perSetData.layout);
				perSetData.numElements = numBindingsPerSet;

				for(UINT32 k = 0; k < numBindingsPerSet; k++)
				{
					// Note: Instead of using one structure per binding, it's possible to update multiple at once
					// by specifying larger descriptorCount, if they all share type and shader stages.
					VkWriteDescriptorSet& writeSetInfo = perSetData.writeSetInfos[k];
					writeSetInfo.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
					writeSetInfo.pNext = nullptr;
					writeSetInfo.dstSet = VK_NULL_HANDLE; // TODO
					writeSetInfo.dstBinding = perSetBindings[k].binding;
					writeSetInfo.dstArrayElement = 0;
					writeSetInfo.descriptorCount = perSetBindings[k].descriptorCount;
					writeSetInfo.descriptorType = perSetBindings[k].descriptorType;

					bool isImage = writeSetInfo.descriptorType == VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER ||
						writeSetInfo.descriptorType == VK_DESCRIPTOR_TYPE_STORAGE_IMAGE ||
						writeSetInfo.descriptorType == VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;

					if(isImage)
					{
						bool isLoadStore = writeSetInfo.descriptorType == VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;

						VkDescriptorImageInfo& imageInfo = perSetData.writeInfos[k].image;
						imageInfo.sampler = VK_NULL_HANDLE;
						imageInfo.imageView = VK_NULL_HANDLE;
						imageInfo.imageLayout = isLoadStore ? VK_IMAGE_LAYOUT_GENERAL : VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

						writeSetInfo.pImageInfo = &imageInfo;
						writeSetInfo.pBufferInfo = nullptr;
						writeSetInfo.pTexelBufferView = nullptr;
					}
					else
					{
						bool isLoadStore = writeSetInfo.descriptorType == VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER;

						if (!isLoadStore)
						{
							VkDescriptorBufferInfo& bufferInfo = perSetData.writeInfos[k].buffer;
							bufferInfo.buffer = VK_NULL_HANDLE;
							bufferInfo.offset = 0;
							bufferInfo.range = VK_WHOLE_SIZE;

							writeSetInfo.pBufferInfo = &bufferInfo;
						}
						else
							writeSetInfo.pBufferInfo = nullptr;

						writeSetInfo.pTexelBufferView = nullptr;
						writeSetInfo.pImageInfo = nullptr;
					}
				}

				bindingOffset += numBindingsPerSet;
			}
		}

		bs_stack_free(bindingOffsets);
		bs_stack_free(bindings);
		bs_stack_free(bindingsPerSet);
	}

	VulkanGpuParams::~VulkanGpuParams()
	{
		for (UINT32 i = 0; i < BS_MAX_DEVICES; i++)
		{
			for (UINT32 j = 0; j < mPerDeviceData[i].numSets; j++)
				mPerDeviceData[i].perSetData[j].set->destroy();
		}

		bs_free(mData); // Everything allocated under a single buffer to a single free is enough
	}

	void VulkanGpuParams::setParamBlockBuffer(UINT32 set, UINT32 slot, const SPtr<GpuParamBlockBufferCore>& paramBlockBuffer)
	{
		GpuParamsCore::setParamBlockBuffer(set, slot, paramBlockBuffer);

		VulkanGpuParamBlockBufferCore* vulkanParamBlockBuffer =
			static_cast<VulkanGpuParamBlockBufferCore*>(paramBlockBuffer.get());

		for (UINT32 i = 0; i < BS_MAX_DEVICES; i++)
		{
			if (mPerDeviceData[i].perSetData == nullptr)
				continue;

			VulkanBuffer* bufferRes = vulkanParamBlockBuffer->getResource(i);
			if (bufferRes != nullptr)
				mPerDeviceData[i].perSetData[set].writeInfos[slot].buffer.buffer = bufferRes->getHandle();
			else
				mPerDeviceData[i].perSetData[set].writeInfos[slot].buffer.buffer = VK_NULL_HANDLE;
		}

		mSetsDirty[set] = true;
	}

	void VulkanGpuParams::setTexture(UINT32 set, UINT32 slot, const SPtr<TextureCore>& texture)
	{
		GpuParamsCore::setTexture(set, slot, texture);

		VulkanTextureCore* vulkanTexture = static_cast<VulkanTextureCore*>(texture.get());
		for (UINT32 i = 0; i < BS_MAX_DEVICES; i++)
		{
			if (mPerDeviceData[i].perSetData == nullptr)
				continue;

			mPerDeviceData[i].perSetData[set].writeInfos[slot].image.imageView = vulkanTexture->getView(i);
		}

		mSetsDirty[set] = true;
	}

	void VulkanGpuParams::setLoadStoreTexture(UINT32 set, UINT32 slot, const SPtr<TextureCore>& texture, 
		const TextureSurface& surface)
	{
		GpuParamsCore::setLoadStoreTexture(set, slot, texture, surface);

		VulkanTextureCore* vulkanTexture = static_cast<VulkanTextureCore*>(texture.get());
		for (UINT32 i = 0; i < BS_MAX_DEVICES; i++)
		{
			if (mPerDeviceData[i].perSetData == nullptr)
				continue;

			mPerDeviceData[i].perSetData[set].writeInfos[slot].image.imageView = vulkanTexture->getView(i, surface);
		}

		mSetsDirty[set] = true;
	}

	void VulkanGpuParams::setBuffer(UINT32 set, UINT32 slot, const SPtr<GpuBufferCore>& buffer)
	{
		GpuParamsCore::setBuffer(set, slot, buffer);

		VulkanGpuBufferCore* vulkanBuffer = static_cast<VulkanGpuBufferCore*>(buffer.get());
		for (UINT32 i = 0; i < BS_MAX_DEVICES; i++)
		{
			if (mPerDeviceData[i].perSetData == nullptr)
				continue;

			VulkanBuffer* bufferRes = vulkanBuffer->getResource(i);
			if (bufferRes != nullptr)
				mPerDeviceData[i].perSetData[set].writeInfos[slot].bufferView = bufferRes->getView();
			else
				mPerDeviceData[i].perSetData[set].writeInfos[slot].bufferView = VK_NULL_HANDLE;
		}

		mSetsDirty[set] = true;
	}

	void VulkanGpuParams::setSamplerState(UINT32 set, UINT32 slot, const SPtr<SamplerStateCore>& sampler)
	{
		GpuParamsCore::setSamplerState(set, slot, sampler);

		VulkanSamplerStateCore* vulkanSampler = static_cast<VulkanSamplerStateCore*>(sampler.get());
		for(UINT32 i = 0; i < BS_MAX_DEVICES; i++)
		{
			if (mPerDeviceData[i].perSetData == nullptr)
				continue;

			VulkanSampler* samplerRes = vulkanSampler->getResource(i);
			if (samplerRes != nullptr)
				mPerDeviceData[i].perSetData[set].writeInfos[slot].image.sampler = samplerRes->getHandle();
			else
				mPerDeviceData[i].perSetData[set].writeInfos[slot].image.sampler = VK_NULL_HANDLE;
		}

		mSetsDirty[set] = true;
	}

	void VulkanGpuParams::setLoadStoreSurface(UINT32 set, UINT32 slot, const TextureSurface& surface)
	{
		GpuParamsCore::setLoadStoreSurface(set, slot, surface);

		SPtr<TextureCore> texture = getLoadStoreTexture(set, slot);
		if (texture == nullptr)
			return;

		VulkanTextureCore* vulkanTexture = static_cast<VulkanTextureCore*>(texture.get());
		for (UINT32 i = 0; i < BS_MAX_DEVICES; i++)
		{
			if (mPerDeviceData[i].perSetData == nullptr)
				continue;

			mPerDeviceData[i].perSetData[set].writeInfos[slot].image.imageView = vulkanTexture->getView(i, surface);
		}

		mSetsDirty[set] = true;
	}

	void VulkanGpuParams::prepareForSubmit(VulkanCmdBuffer* buffer, UnorderedMap<UINT32, TransitionInfo>& transitionInfo)
	{
		UINT32 deviceIdx = buffer->getDeviceIdx();
		UINT32 queueFamily = buffer->getQueueFamily();

		const PerDeviceData& perDeviceData = mPerDeviceData[deviceIdx];
		if (perDeviceData.perSetData == nullptr)
			return;

		for(UINT32 i = 0; i < perDeviceData.numSets; i++)
		{
			if (!mSetsDirty[i])
				continue;

			// Note: Currently I write to the entire set at once, but it might be beneficial to remember only the exact
			// entries that were updated, and only write to them individually.
			const PerSetData& perSetData = perDeviceData.perSetData[i];
			perSetData.set->write(perSetData.writeSetInfos, perSetData.numElements);

			mSetsDirty[i] = false;
		}

		auto registerBuffer = [&](VulkanBuffer* resource, VkAccessFlags accessFlags, VulkanUseFlags useFlags)
		{
			if (resource == nullptr)
				return;

			buffer->registerResource(resource, useFlags);

			if (resource->isExclusive())
			{
				UINT32 currentQueueFamily = resource->getQueueFamily();
				if (currentQueueFamily != queueFamily)
				{
					Vector<VkBufferMemoryBarrier>& barriers = transitionInfo[currentQueueFamily].bufferBarriers;

					barriers.push_back(VkBufferMemoryBarrier());
					VkBufferMemoryBarrier& barrier = barriers.back();
					barrier.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
					barrier.pNext = nullptr;
					barrier.srcAccessMask = accessFlags;
					barrier.dstAccessMask = accessFlags;
					barrier.srcQueueFamilyIndex = currentQueueFamily;
					barrier.dstQueueFamilyIndex = queueFamily;
					barrier.buffer = resource->getHandle();
					barrier.offset = 0;
					barrier.size = VK_WHOLE_SIZE;
				}
			}
		};

		auto registerImage = [&](VulkanImage* resource, VkAccessFlags accessFlags, VkImageLayout layout, 
			const VkImageSubresourceRange& range, VulkanUseFlags useFlags)
		{
			buffer->registerResource(resource, useFlags);

			UINT32 currentQueueFamily = resource->getQueueFamily();
			bool queueMismatch = resource->isExclusive() && currentQueueFamily != queueFamily;

			if (queueMismatch || resource->getLayout() != layout)
			{
				Vector<VkImageMemoryBarrier>& barriers = transitionInfo[currentQueueFamily].imageBarriers;

				barriers.push_back(VkImageMemoryBarrier());
				VkImageMemoryBarrier& barrier = barriers.back();
				barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
				barrier.pNext = nullptr;
				barrier.srcAccessMask = accessFlags;
				barrier.dstAccessMask = accessFlags;
				barrier.srcQueueFamilyIndex = currentQueueFamily;
				barrier.dstQueueFamilyIndex = queueFamily;
				barrier.oldLayout = resource->getLayout();
				barrier.newLayout = layout;
				barrier.image = resource->getHandle();
				barrier.subresourceRange = range;

				resource->setLayout(layout);
			}
		};

		for(UINT32 i = 0; i < mNumElements[(UINT32)ElementType::ParamBlock]; i++)
		{
			if (mParamBlockBuffers[i] == nullptr)
				continue;

			VulkanGpuParamBlockBufferCore* element = static_cast<VulkanGpuParamBlockBufferCore*>(mParamBlockBuffers[i].get());

			VulkanBuffer* resource = element->getResource(deviceIdx);
			registerBuffer(resource, VK_ACCESS_UNIFORM_READ_BIT, VulkanUseFlag::Read);
		}

		for (UINT32 i = 0; i < mNumElements[(UINT32)ElementType::Buffer]; i++)
		{
			if (mBuffers[i] == nullptr)
				continue;

			VulkanGpuBufferCore* element = static_cast<VulkanGpuBufferCore*>(mBuffers[i].get());

			VkAccessFlags accessFlags = VK_ACCESS_SHADER_READ_BIT;
			VulkanUseFlags useFlags = VulkanUseFlag::Read;

			if(element->getProperties().getRandomGpuWrite())
			{
				accessFlags |= VK_ACCESS_SHADER_WRITE_BIT;
				useFlags |= VulkanUseFlag::Write;
			}

			VulkanBuffer* resource = element->getResource(deviceIdx);
			registerBuffer(resource, accessFlags, useFlags);
		}

		for (UINT32 i = 0; i < mNumElements[(UINT32)ElementType::SamplerState]; i++)
		{
			if (mSamplerStates[i] == nullptr)
				continue;

			VulkanSamplerStateCore* element = static_cast<VulkanSamplerStateCore*>(mSamplerStates[i].get());

			VulkanSampler* resource = element->getResource(deviceIdx);
			if (resource == nullptr)
				continue;

			buffer->registerResource(resource, VulkanUseFlag::Read);
		}

		for (UINT32 i = 0; i < mNumElements[(UINT32)ElementType::LoadStoreTexture]; i++)
		{
			if (mLoadStoreTextures[i] == nullptr)
				continue;

			VulkanTextureCore* element = static_cast<VulkanTextureCore*>(mLoadStoreTextures[i].get());

			VulkanImage* resource = element->getResource(deviceIdx);
			if (resource == nullptr)
				continue;

			VkAccessFlags accessFlags = VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT;
			VulkanUseFlags useFlags = VulkanUseFlag::Read | VulkanUseFlag::Write;

			const TextureSurface& surface = mLoadStoreSurfaces[i];
			VkImageSubresourceRange subresource;
			subresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			subresource.baseArrayLayer = surface.arraySlice;
			subresource.layerCount = surface.numArraySlices;
			subresource.baseMipLevel = surface.mipLevel;
			subresource.levelCount = surface.numMipLevels;

			registerImage(resource, accessFlags, VK_IMAGE_LAYOUT_GENERAL, subresource, useFlags);
		}

		for (UINT32 i = 0; i < mNumElements[(UINT32)ElementType::Texture]; i++)
		{
			if (mTextures[i] == nullptr)
				continue;

			VulkanTextureCore* element = static_cast<VulkanTextureCore*>(mTextures[i].get());

			VulkanImage* resource = element->getResource(deviceIdx);
			if (resource == nullptr)
				continue;

			const TextureProperties& props = element->getProperties();

			bool isDepthStencil = (props.getUsage() & TU_DEPTHSTENCIL) != 0;

			VkImageSubresourceRange subresource;
			subresource.aspectMask = isDepthStencil ? VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT
													: VK_IMAGE_ASPECT_COLOR_BIT;

			subresource.baseArrayLayer = 0;
			subresource.layerCount = props.getNumFaces();
			subresource.baseMipLevel = 0;
			subresource.levelCount = props.getNumMipmaps();

			registerImage(resource, VK_ACCESS_SHADER_READ_BIT, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, subresource, 
				VulkanUseFlag::Read);
		}
	}

	void VulkanGpuParams::bind(VulkanCommandBuffer& buffer)
	{
		UINT32 deviceIdx = buffer.getDeviceIdx();

		PerDeviceData& perDeviceData = mPerDeviceData[deviceIdx];
		if (perDeviceData.perSetData == nullptr)
			return;

		VulkanCmdBuffer* internalCB = buffer.getInternal();
		VkCommandBuffer vkCB = internalCB->getHandle();

		VkPipelineBindPoint bindPoint;
		GpuQueueType queueType = buffer.getType();
		switch(queueType)
		{
		case GQT_GRAPHICS:
			bindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
			break;
		case GQT_COMPUTE:
			bindPoint = VK_PIPELINE_BIND_POINT_COMPUTE;
			break;
		case GQT_UPLOAD:
		default:
			LOGERR("Cannot bind GpuParams on the upload queue. Ignoring.");
			return;
		}

		VkDescriptorSet* sets = bs_stack_alloc<VkDescriptorSet>(perDeviceData.numSets);
		for (UINT32 i = 0; i < perDeviceData.numSets; i++)
		{
			VulkanDescriptorSet* set = perDeviceData.perSetData[i].set;

			internalCB->registerResource(set, VulkanUseFlag::Read);
			sets[i] = set->getHandle();
		}

		vkCmdBindDescriptorSets(vkCB, bindPoint, perDeviceData.pipelineLayout, 0, 
			perDeviceData.numSets, sets, 0, nullptr);

		bs_stack_free(sets);
	}
}