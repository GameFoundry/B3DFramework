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
#include "BsVulkanDescriptorLayout.h"
#include "BsVulkanSamplerState.h"
#include "BsVulkanGpuPipelineParamInfo.h"
#include "BsVulkanCommandBuffer.h"
#include "BsGpuParamDesc.h"

namespace bs
{
	VulkanGpuParams::VulkanGpuParams(const SPtr<GpuPipelineParamInfoCore>& paramInfo, GpuDeviceFlags deviceMask)
		: GpuParamsCore(paramInfo, deviceMask), mPerDeviceData(), mDeviceMask(deviceMask), mData(nullptr), mSetsDirty(nullptr)
	{
		
	}

	VulkanGpuParams::~VulkanGpuParams()
	{
		{
			Lock lock(mMutex);

			UINT32 numSets = mParamInfo->getNumSets();
			for (UINT32 i = 0; i < BS_MAX_DEVICES; i++)
			{
				for (UINT32 j = 0; j < numSets; j++)
				{
					for (auto& entry : mPerDeviceData[i].perSetData[j].sets)
						entry->destroy();

					mPerDeviceData[i].perSetData[j].sets.~Vector<VulkanDescriptorSet*>();
				}
			}
		}

		bs_free(mData); // Everything allocated under a single buffer to a single free is enough
	}

	void VulkanGpuParams::initialize()
	{
		VulkanGpuPipelineParamInfo& vkParamInfo = static_cast<VulkanGpuPipelineParamInfo&>(*mParamInfo);

		VulkanRenderAPI& rapi = static_cast<VulkanRenderAPI&>(RenderAPICore::instance());
		VulkanDevice* devices[BS_MAX_DEVICES];

		VulkanUtility::getDevices(rapi, mDeviceMask, devices);

		UINT32 numDevices = 0;
		for (UINT32 i = 0; i < BS_MAX_DEVICES; i++)
		{
			if (devices[i] != nullptr)
				numDevices++;
		}

		UINT32 numSets = vkParamInfo.getNumSets();
		UINT32 numBindings = vkParamInfo.getNumElements();

		// Note: I'm assuming a single WriteInfo per binding, but if arrays sizes larger than 1 are eventually supported
		// I'll need to adjust the code.
		UINT32 setsDirtyBytes = sizeof(bool) * numSets;
		UINT32 perSetBytes = sizeof(PerSetData) * numSets;
		UINT32 writeSetInfosBytes = sizeof(VkWriteDescriptorSet) * numBindings;
		UINT32 writeInfosBytes = sizeof(WriteInfo) * numBindings;
		mData = (UINT8*)bs_alloc(setsDirtyBytes + (perSetBytes + writeSetInfosBytes + writeInfosBytes) * numDevices);
		UINT8* dataIter = mData;

		Lock lock(mMutex); // Set write operations need to be thread safe

		mSetsDirty = (bool*)dataIter;
		memset(mSetsDirty, 1, setsDirtyBytes);
		dataIter += setsDirtyBytes;

		for (UINT32 i = 0; i < BS_MAX_DEVICES; i++)
		{
			if (devices[i] == nullptr)
			{
				mPerDeviceData[i].perSetData = nullptr;

				continue;
			}

			mPerDeviceData[i].perSetData = (PerSetData*)dataIter;
			dataIter += perSetBytes;

			VulkanDescriptorManager& descManager = devices[i]->getDescriptorManager();
			for (UINT32 j = 0; j < numSets; j++)
			{
				UINT32 numBindingsPerSet = vkParamInfo.getNumBindings(j);

				PerSetData& perSetData = mPerDeviceData[i].perSetData[j];
				new (&perSetData.sets) Vector<VulkanDescriptorSet*>();

				perSetData.writeSetInfos = (VkWriteDescriptorSet*)dataIter;
				dataIter += sizeof(VkWriteDescriptorSet) * numBindingsPerSet;

				perSetData.writeInfos = (WriteInfo*)dataIter;
				dataIter += sizeof(WriteInfo) * numBindingsPerSet;

				VulkanDescriptorLayout* layout = vkParamInfo.getLayout(i, j);
				perSetData.numElements = numBindingsPerSet;
				perSetData.latestSet = descManager.createSet(layout);
				perSetData.sets.push_back(perSetData.latestSet);

				VkDescriptorSetLayoutBinding* perSetBindings = vkParamInfo.getBindings(j);
				for (UINT32 k = 0; k < numBindingsPerSet; k++)
				{
					// Note: Instead of using one structure per binding, it's possible to update multiple at once
					// by specifying larger descriptorCount, if they all share type and shader stages.
					VkWriteDescriptorSet& writeSetInfo = perSetData.writeSetInfos[k];
					writeSetInfo.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
					writeSetInfo.pNext = nullptr;
					writeSetInfo.dstSet = VK_NULL_HANDLE;
					writeSetInfo.dstBinding = perSetBindings[k].binding;
					writeSetInfo.dstArrayElement = 0;
					writeSetInfo.descriptorCount = perSetBindings[k].descriptorCount;
					writeSetInfo.descriptorType = perSetBindings[k].descriptorType;

					bool isImage = writeSetInfo.descriptorType == VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER ||
						writeSetInfo.descriptorType == VK_DESCRIPTOR_TYPE_STORAGE_IMAGE ||
						writeSetInfo.descriptorType == VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;

					if (isImage)
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
			}
		}

		GpuParamsCore::initialize();
	}

	void VulkanGpuParams::setParamBlockBuffer(UINT32 set, UINT32 slot, const SPtr<GpuParamBlockBufferCore>& paramBlockBuffer)
	{
		GpuParamsCore::setParamBlockBuffer(set, slot, paramBlockBuffer);

		Lock(mMutex);

		VulkanGpuParamBlockBufferCore* vulkanParamBlockBuffer =
			static_cast<VulkanGpuParamBlockBufferCore*>(paramBlockBuffer.get());

		for (UINT32 i = 0; i < BS_MAX_DEVICES; i++)
		{
			if (mPerDeviceData[i].perSetData == nullptr)
				continue;

			VulkanBuffer* bufferRes;
			if (vulkanParamBlockBuffer != nullptr)
				bufferRes = vulkanParamBlockBuffer->getResource(i);
			else
				bufferRes = nullptr;

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

		Lock(mMutex);

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

		Lock(mMutex);

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

		Lock(mMutex);

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

		Lock(mMutex);

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

		Lock(mMutex);

		VulkanTextureCore* vulkanTexture = static_cast<VulkanTextureCore*>(texture.get());
		for (UINT32 i = 0; i < BS_MAX_DEVICES; i++)
		{
			if (mPerDeviceData[i].perSetData == nullptr)
				continue;

			mPerDeviceData[i].perSetData[set].writeInfos[slot].image.imageView = vulkanTexture->getView(i, surface);
		}

		mSetsDirty[set] = true;
	}

	UINT32 VulkanGpuParams::getNumSets() const
	{
		return mParamInfo->getNumSets();
	}

	void VulkanGpuParams::prepareForBind(VulkanCmdBuffer& buffer, VkDescriptorSet* sets)
	{
		UINT32 deviceIdx = buffer.getDeviceIdx();

		PerDeviceData& perDeviceData = mPerDeviceData[deviceIdx];
		if (perDeviceData.perSetData == nullptr)
			return;

		UINT32 numParamBlocks = mParamInfo->getNumElements(GpuPipelineParamInfo::ParamType::ParamBlock);
		UINT32 numTextures = mParamInfo->getNumElements(GpuPipelineParamInfo::ParamType::Texture);
		UINT32 numStorageTextures = mParamInfo->getNumElements(GpuPipelineParamInfo::ParamType::LoadStoreTexture);
		UINT32 numBuffers = mParamInfo->getNumElements(GpuPipelineParamInfo::ParamType::Buffer);
		UINT32 numSamplers = mParamInfo->getNumElements(GpuPipelineParamInfo::ParamType::SamplerState);
		UINT32 numSets = mParamInfo->getNumSets();

		// Registers resources with the command buffer
		// Note: Makes the assumption that this object (and all of the resources it holds) are externally locked, and will
		// not be modified on another thread while being bound.
		for (UINT32 i = 0; i < numParamBlocks; i++)
		{
			if (mParamBlockBuffers[i] == nullptr)
				continue;

			VulkanGpuParamBlockBufferCore* element = static_cast<VulkanGpuParamBlockBufferCore*>(mParamBlockBuffers[i].get());

			VulkanBuffer* resource = element->getResource(deviceIdx);
			buffer.registerResource(resource, VK_ACCESS_UNIFORM_READ_BIT, VulkanUseFlag::Read);
		}

		for (UINT32 i = 0; i < numBuffers; i++)
		{
			if (mBuffers[i] == nullptr)
				continue;

			VulkanGpuBufferCore* element = static_cast<VulkanGpuBufferCore*>(mBuffers[i].get());

			VkAccessFlags accessFlags = VK_ACCESS_SHADER_READ_BIT;
			VulkanUseFlags useFlags = VulkanUseFlag::Read;

			if (element->getProperties().getRandomGpuWrite())
			{
				accessFlags |= VK_ACCESS_SHADER_WRITE_BIT;
				useFlags |= VulkanUseFlag::Write;
			}

			VulkanBuffer* resource = element->getResource(deviceIdx);
			buffer.registerResource(resource, accessFlags, useFlags);
		}

		for (UINT32 i = 0; i < numSamplers; i++)
		{
			if (mSamplerStates[i] == nullptr)
				continue;

			VulkanSamplerStateCore* element = static_cast<VulkanSamplerStateCore*>(mSamplerStates[i].get());

			VulkanSampler* resource = element->getResource(deviceIdx);
			if (resource == nullptr)
				continue;

			buffer.registerResource(resource, VulkanUseFlag::Read);
		}

		for (UINT32 i = 0; i < numStorageTextures; i++)
		{
			if (mLoadStoreTextures[i] == nullptr)
				continue;

			VulkanTextureCore* element = static_cast<VulkanTextureCore*>(mLoadStoreTextures[i].get());

			VulkanImage* resource = element->getResource(deviceIdx);
			if (resource == nullptr)
				continue;

			const TextureProperties& props = element->getProperties();
			VkAccessFlags accessFlags = VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT;
			VulkanUseFlags useFlags = VulkanUseFlag::Read | VulkanUseFlag::Write;

			buffer.registerResource(resource, accessFlags, resource->getLayout(), VK_IMAGE_LAYOUT_GENERAL, useFlags);
		}

		for (UINT32 i = 0; i < numTextures; i++)
		{
			if (mTextures[i] == nullptr)
				continue;

			VulkanTextureCore* element = static_cast<VulkanTextureCore*>(mTextures[i].get());

			VulkanImage* resource = element->getResource(deviceIdx);
			if (resource == nullptr)
				continue;

			const TextureProperties& props = element->getProperties();

			VkImageLayout layout;

			// Keep dynamic textures in general layout, so they can be easily mapped by CPU
			if (props.getUsage() & TU_DYNAMIC)
				layout = VK_IMAGE_LAYOUT_GENERAL;
			else
				layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

			buffer.registerResource(resource, VK_ACCESS_SHADER_READ_BIT, resource->getLayout(), layout, VulkanUseFlag::Read);
		}

		// Acquire sets as needed, and updated their contents if dirty
		VulkanRenderAPI& rapi = static_cast<VulkanRenderAPI&>(RenderAPICore::instance());
		VulkanDevice& device = *rapi._getDevice(deviceIdx);
		VulkanDescriptorManager& descManager = device.getDescriptorManager();
		VulkanGpuPipelineParamInfo& vkParamInfo = static_cast<VulkanGpuPipelineParamInfo&>(*mParamInfo);

		Lock(mMutex);
		for (UINT32 i = 0; i < numSets; i++)
		{
			PerSetData& perSetData = perDeviceData.perSetData[i];

			if (!mSetsDirty[i]) // Set not dirty, just use the last one we wrote (this is fine even across multiple command buffers)
				continue;

			// Set is dirty, we need to update
			//// Use latest unless already used, otherwise try to find an unused one
			if(perSetData.latestSet->isBound()) // Checking this is okay, because it's only modified below when we call registerResource, which is under the same lock as this
			{
				perSetData.latestSet = nullptr;

				for(auto& entry : perSetData.sets)
				{
					if(!entry->isBound())
					{
						perSetData.latestSet = entry;
						break;
					}
				}

				// Cannot find an empty set, allocate a new one
				VulkanDescriptorLayout* layout = vkParamInfo.getLayout(deviceIdx, i);
				perSetData.latestSet = descManager.createSet(layout);
				perSetData.sets.push_back(perSetData.latestSet);
			}

			// Note: Currently I write to the entire set at once, but it might be beneficial to remember only the exact
			// entries that were updated, and only write to them individually.
			perSetData.latestSet->write(perSetData.writeSetInfos, perSetData.numElements);

			mSetsDirty[i] = false;
		}

		for (UINT32 i = 0; i < numSets; i++)
		{
			VulkanDescriptorSet* set = perDeviceData.perSetData[i].latestSet;

			buffer.registerResource(set, VulkanUseFlag::Read);
			sets[i] = set->getHandle();
		}
	}
}