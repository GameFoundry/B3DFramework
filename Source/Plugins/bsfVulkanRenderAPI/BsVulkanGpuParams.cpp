//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
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
#include "Managers/BsVulkanTextureManager.h"
#include "Managers/BsVulkanHardwareBufferManager.h"
#include "RenderAPI/BsGpuParamDesc.h"

namespace bs { namespace ct
{
	VulkanGpuParams::VulkanGpuParams(const SPtr<GpuPipelineParamInfo>& paramInfo, GpuDeviceFlags deviceMask)
		: GpuParams(paramInfo, deviceMask), mPerDeviceData(), mDeviceMask(deviceMask)
	{
		
	}

	VulkanGpuParams::~VulkanGpuParams()
	{
		Lock lock(mMutex);

		UINT32 numSets = mParamInfo->getNumSets();
		for (UINT32 i = 0; i < BS_MAX_DEVICES; i++)
		{
			if (mPerDeviceData[i].perSetData == nullptr)
				continue;

			for (UINT32 j = 0; j < numSets; j++)
			{
				for (auto& entry : mPerDeviceData[i].perSetData[j].sets)
					entry->destroy();

				mPerDeviceData[i].perSetData[j].sets.~Vector<VulkanDescriptorSet*>();
			}
		}
	}

	void VulkanGpuParams::initialize()
	{
		VulkanGpuPipelineParamInfo& vkParamInfo = static_cast<VulkanGpuPipelineParamInfo&>(*mParamInfo);

		VulkanRenderAPI& rapi = static_cast<VulkanRenderAPI&>(RenderAPI::instance());
		VulkanDevice* devices[BS_MAX_DEVICES];

		VulkanUtility::getDevices(rapi, mDeviceMask, devices);

		UINT32 numDevices = 0;
		for (UINT32 i = 0; i < BS_MAX_DEVICES; i++)
		{
			if (devices[i] != nullptr)
				numDevices++;
		}

		UINT32 numParamBlocks = vkParamInfo.getNumElements(GpuPipelineParamInfo::ParamType::ParamBlock);
		UINT32 numTextures = vkParamInfo.getNumElements(GpuPipelineParamInfo::ParamType::Texture);
		UINT32 numStorageTextures = vkParamInfo.getNumElements(GpuPipelineParamInfo::ParamType::LoadStoreTexture);
		UINT32 numBuffers = vkParamInfo.getNumElements(GpuPipelineParamInfo::ParamType::Buffer);
		UINT32 numSamplers = vkParamInfo.getNumElements(GpuPipelineParamInfo::ParamType::SamplerState);
		UINT32 numSets = vkParamInfo.getNumSets();
		UINT32 numBindings = vkParamInfo.getNumElements();

		if (numSets == 0)
			return;

		// Note: I'm assuming a single WriteInfo per binding, but if arrays sizes larger than 1 are eventually supported
		// I'll need to adjust the code.
		mAlloc.reserve<bool>(numSets)
			.reserve<PerSetData>(numSets * numDevices)
			.reserve<VkWriteDescriptorSet>(numBindings * numDevices)
			.reserve<WriteInfo>(numBindings * numDevices)
			.reserve<VkImage>(numTextures * numDevices)
			.reserve<VkImage>(numStorageTextures * numDevices)
			.reserve<VkBuffer>(numParamBlocks * numDevices)
			.reserve<VkBuffer>(numBuffers * numDevices)
			.reserve<VkSampler>(numSamplers * numDevices)
			.init();

		Lock lock(mMutex); // Set write operations need to be thread safe

		mSetsDirty = mAlloc.alloc<bool>(numSets);
		bs_zero_out(mSetsDirty, numSets);

		VulkanSamplerState* defaultSampler = static_cast<VulkanSamplerState*>(SamplerState::getDefault().get());
		VulkanTextureManager& vkTexManager = static_cast<VulkanTextureManager&>(TextureManager::instance());
		VulkanHardwareBufferManager& vkBufManager = static_cast<VulkanHardwareBufferManager&>(
			HardwareBufferManager::instance());

		for (UINT32 i = 0; i < BS_MAX_DEVICES; i++)
		{
			if (devices[i] == nullptr)
			{
				mPerDeviceData[i].perSetData = nullptr;

				continue;
			}

			mPerDeviceData[i].perSetData = mAlloc.alloc<PerSetData>(numSets);
			mPerDeviceData[i].sampledImages = mAlloc.alloc<VkImage>(numTextures);
			mPerDeviceData[i].storageImages = mAlloc.alloc<VkImage>(numStorageTextures);
			mPerDeviceData[i].uniformBuffers = mAlloc.alloc<VkBuffer>(numParamBlocks);
			mPerDeviceData[i].buffers = mAlloc.alloc<VkBuffer>(numBuffers);
			mPerDeviceData[i].samplers = mAlloc.alloc<VkSampler>(numSamplers);

			bs_zero_out(mPerDeviceData[i].sampledImages, numTextures);
			bs_zero_out(mPerDeviceData[i].storageImages, numStorageTextures);
			bs_zero_out(mPerDeviceData[i].uniformBuffers, numParamBlocks);
			bs_zero_out(mPerDeviceData[i].buffers, numBuffers);
			bs_zero_out(mPerDeviceData[i].samplers, numSamplers);

			VulkanDescriptorManager& descManager = devices[i]->getDescriptorManager();
			VulkanSampler* vkDefaultSampler = defaultSampler->getResource(i);

			for (UINT32 j = 0; j < numSets; j++)
			{
				UINT32 numBindingsPerSet = vkParamInfo.getNumBindings(j);

				PerSetData& perSetData = mPerDeviceData[i].perSetData[j];
				new (&perSetData.sets) Vector<VulkanDescriptorSet*>();

				perSetData.writeSetInfos = mAlloc.alloc<VkWriteDescriptorSet>(numBindingsPerSet);
				perSetData.writeInfos = mAlloc.alloc<WriteInfo>(numBindingsPerSet);

				VulkanDescriptorLayout* layout = vkParamInfo.getLayout(i, j);
				perSetData.numElements = numBindingsPerSet;
				perSetData.latestSet = descManager.createSet(layout);
				perSetData.sets.push_back(perSetData.latestSet);

				VkDescriptorSetLayoutBinding* perSetBindings = vkParamInfo.getBindings(j);
				GpuParamObjectType* types = vkParamInfo.getLayoutTypes(j);
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
						imageInfo.sampler = vkDefaultSampler->getHandle();
						
						if(isLoadStore)
						{
							imageInfo.imageView = vkTexManager.getDummyImageView(types[k], i);
							imageInfo.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
						}
						else
						{
							imageInfo.imageView = vkTexManager.getDummyImageView(types[k], i);
							imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
						}

						writeSetInfo.pImageInfo = &imageInfo;
						writeSetInfo.pBufferInfo = nullptr;
						writeSetInfo.pTexelBufferView = nullptr;
					}
					else
					{
						bool useView = writeSetInfo.descriptorType != VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER && 
							writeSetInfo.descriptorType != VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;

						if (!useView)
						{
							VkDescriptorBufferInfo& bufferInfo = perSetData.writeInfos[k].buffer;
							bufferInfo.offset = 0;
							bufferInfo.range = VK_WHOLE_SIZE;

							if(writeSetInfo.descriptorType == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER)
								bufferInfo.buffer = vkBufManager.getDummyUniformBuffer(i);
							else
								bufferInfo.buffer = vkBufManager.getDummyStructuredBuffer(i);

							writeSetInfo.pBufferInfo = &bufferInfo;
							writeSetInfo.pTexelBufferView = nullptr;
						}
						else
						{
							writeSetInfo.pBufferInfo = nullptr;

							bool isLoadStore = writeSetInfo.descriptorType == VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER;
							VkBufferView bufferView;
							if(isLoadStore)
								bufferView = vkBufManager.getDummyStorageBufferView(i);
							else
								bufferView = vkBufManager.getDummyReadBufferView(i);

							perSetData.writeInfos[k].bufferView = bufferView;
							writeSetInfo.pBufferInfo = nullptr;
							writeSetInfo.pTexelBufferView = &perSetData.writeInfos[k].bufferView;
						}

						writeSetInfo.pImageInfo = nullptr;
					}
				}
			}
		}

		GpuParams::initialize();
	}

	void VulkanGpuParams::setParamBlockBuffer(UINT32 set, UINT32 slot, const SPtr<GpuParamBlockBuffer>& paramBlockBuffer)
	{
		GpuParams::setParamBlockBuffer(set, slot, paramBlockBuffer);

		VulkanGpuPipelineParamInfo& vkParamInfo = static_cast<VulkanGpuPipelineParamInfo&>(*mParamInfo);
		UINT32 bindingIdx = vkParamInfo.getBindingIdx(set, slot);
		if(bindingIdx == (UINT32)-1)
		{
			LOGERR("Provided set/slot combination is not used by the GPU program: " + toString(set) + "," + 
				toString(slot) + ".");
			return;
		}

		UINT32 sequentialIdx = vkParamInfo.getSequentialSlot(GpuPipelineParamInfo::ParamType::ParamBlock, set, slot);

		Lock lock(mMutex);

		VulkanGpuParamBlockBuffer* vulkanParamBlockBuffer =
			static_cast<VulkanGpuParamBlockBuffer*>(paramBlockBuffer.get());
		for (UINT32 i = 0; i < BS_MAX_DEVICES; i++)
		{
			if (mPerDeviceData[i].perSetData == nullptr)
				continue;

			VulkanBuffer* bufferRes;
			if (vulkanParamBlockBuffer != nullptr)
				bufferRes = vulkanParamBlockBuffer->getResource(i);
			else
				bufferRes = nullptr;

			PerSetData& perSetData = mPerDeviceData[i].perSetData[set];
			if (bufferRes != nullptr)
			{
				VkBuffer buffer = bufferRes->getHandle();

				perSetData.writeInfos[bindingIdx].buffer.buffer = buffer;
				mPerDeviceData[i].uniformBuffers[sequentialIdx] = buffer;
			}
			else
			{
				VulkanHardwareBufferManager& vkBufManager = static_cast<VulkanHardwareBufferManager&>(
					HardwareBufferManager::instance());

				perSetData.writeInfos[bindingIdx].buffer.buffer = vkBufManager.getDummyUniformBuffer(i);
				mPerDeviceData[i].uniformBuffers[sequentialIdx] = VK_NULL_HANDLE;
			}
		}

		mSetsDirty[set] = true;
	}

	void VulkanGpuParams::setTexture(UINT32 set, UINT32 slot, const SPtr<Texture>& texture, const TextureSurface& surface)
	{
		GpuParams::setTexture(set, slot, texture, surface);

		VulkanGpuPipelineParamInfo& vkParamInfo = static_cast<VulkanGpuPipelineParamInfo&>(*mParamInfo);
		UINT32 bindingIdx = vkParamInfo.getBindingIdx(set, slot);
		if (bindingIdx == (UINT32)-1)
		{
			LOGERR("Provided set/slot combination is not used by the GPU program: " + toString(set) + "," +
				   toString(slot) + ".");
			return;
		}

		UINT32 sequentialIdx = vkParamInfo.getSequentialSlot(GpuPipelineParamInfo::ParamType::Texture, set, slot);

		Lock lock(mMutex);

		VulkanTexture* vulkanTexture = static_cast<VulkanTexture*>(texture.get());
		for (UINT32 i = 0; i < BS_MAX_DEVICES; i++)
		{
			if (mPerDeviceData[i].perSetData == nullptr)
				continue;

			VulkanImage* imageRes;
			if (vulkanTexture != nullptr)
				imageRes = vulkanTexture->getResource(i);
			else
				imageRes = nullptr;

			PerSetData& perSetData = mPerDeviceData[i].perSetData[set];
			if (imageRes != nullptr)
			{
				auto& texProps = texture->getProperties();

				TextureSurface actualSurface = surface;
				if (surface.numMipLevels == 0)
					actualSurface.numMipLevels = texProps.getNumMipmaps() + 1;
				
				if(surface.numFaces == 0)
					actualSurface.numFaces = texProps.getNumFaces();

				perSetData.writeInfos[bindingIdx].image.imageView = imageRes->getView(actualSurface, false);
				mPerDeviceData[i].sampledImages[sequentialIdx] = imageRes->getHandle();
			}
			else
			{
				VulkanTextureManager& vkTexManager = static_cast<VulkanTextureManager&>(
					TextureManager::instance());

				GpuParamObjectType* types = vkParamInfo.getLayoutTypes(set);
				GpuParamObjectType type = types[bindingIdx];

				perSetData.writeInfos[bindingIdx].image.imageView = vkTexManager.getDummyImageView(type, i);
				mPerDeviceData[i].sampledImages[sequentialIdx] = VK_NULL_HANDLE;
			}
		}

		mSetsDirty[set] = true;
	}

	void VulkanGpuParams::setLoadStoreTexture(UINT32 set, UINT32 slot, const SPtr<Texture>& texture, 
		const TextureSurface& surface)
	{
		GpuParams::setLoadStoreTexture(set, slot, texture, surface);

		VulkanGpuPipelineParamInfo& vkParamInfo = static_cast<VulkanGpuPipelineParamInfo&>(*mParamInfo);
		UINT32 bindingIdx = vkParamInfo.getBindingIdx(set, slot);
		if (bindingIdx == (UINT32)-1)
		{
			LOGERR("Provided set/slot combination is not used by the GPU program: " + toString(set) + "," +
				   toString(slot) + ".");
			return;
		}

		UINT32 sequentialIdx = vkParamInfo.getSequentialSlot(GpuPipelineParamInfo::ParamType::LoadStoreTexture, set, slot);

		Lock lock(mMutex);

		VulkanTexture* vulkanTexture = static_cast<VulkanTexture*>(texture.get());
		for (UINT32 i = 0; i < BS_MAX_DEVICES; i++)
		{
			if (mPerDeviceData[i].perSetData == nullptr)
				continue;

			VulkanImage* imageRes;
			if (vulkanTexture != nullptr)
				imageRes = vulkanTexture->getResource(i);
			else
				imageRes = nullptr;

			PerSetData& perSetData = mPerDeviceData[i].perSetData[set];
			if (imageRes != nullptr)
			{
				perSetData.writeInfos[bindingIdx].image.imageView = imageRes->getView(surface, false);
				mPerDeviceData[i].storageImages[sequentialIdx] = imageRes->getHandle();
			}
			else
			{
				VulkanTextureManager& vkTexManager = static_cast<VulkanTextureManager&>(
					TextureManager::instance());

				GpuParamObjectType* types = vkParamInfo.getLayoutTypes(set);
				GpuParamObjectType type = types[bindingIdx];

				perSetData.writeInfos[bindingIdx].image.imageView = vkTexManager.getDummyImageView(type, i);
				mPerDeviceData[i].storageImages[sequentialIdx] = VK_NULL_HANDLE;
			}
		}

		mSetsDirty[set] = true;
	}

	void VulkanGpuParams::setBuffer(UINT32 set, UINT32 slot, const SPtr<GpuBuffer>& buffer)
	{
		GpuParams::setBuffer(set, slot, buffer);

		VulkanGpuPipelineParamInfo& vkParamInfo = static_cast<VulkanGpuPipelineParamInfo&>(*mParamInfo);
		UINT32 bindingIdx = vkParamInfo.getBindingIdx(set, slot);
		if (bindingIdx == (UINT32)-1)
		{
			LOGERR("Provided set/slot combination is not used by the GPU program: " + toString(set) + "," +
				   toString(slot) + ".");
			return;
		}

		UINT32 sequentialIdx = vkParamInfo.getSequentialSlot(GpuPipelineParamInfo::ParamType::Buffer, set, slot);

		Lock lock(mMutex);

		VulkanGpuBuffer* vulkanBuffer = static_cast<VulkanGpuBuffer*>(buffer.get());
		for (UINT32 i = 0; i < BS_MAX_DEVICES; i++)
		{
			if (mPerDeviceData[i].perSetData == nullptr)
				continue;

			VulkanBuffer* bufferRes;
			
			if (vulkanBuffer != nullptr)
				bufferRes = vulkanBuffer->getResource(i);
			else
				bufferRes = nullptr;

			PerSetData& perSetData = mPerDeviceData[i].perSetData[set];
			VkWriteDescriptorSet& writeSetInfo = perSetData.writeSetInfos[bindingIdx];

			bool useView = writeSetInfo.descriptorType != VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
			if (useView)
			{
				VkBufferView bufferView;
				if (bufferRes != nullptr)
				{
					bufferView = vulkanBuffer->getView(i);
					mPerDeviceData[i].buffers[sequentialIdx] = bufferRes->getHandle();
				}
				else
				{
					VulkanHardwareBufferManager& vkBufManager = static_cast<VulkanHardwareBufferManager&>(
						HardwareBufferManager::instance());

					bool isLoadStore = writeSetInfo.descriptorType == VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER;
					if (isLoadStore)
						bufferView = vkBufManager.getDummyStorageBufferView(i);
					else
						bufferView = vkBufManager.getDummyReadBufferView(i);

					mPerDeviceData[i].buffers[sequentialIdx] = nullptr;
				}

				perSetData.writeInfos[bindingIdx].bufferView = bufferView;
				writeSetInfo.pTexelBufferView = &perSetData.writeInfos[bindingIdx].bufferView;
			}
			else // Structured storage buffer
			{
				if (bufferRes != nullptr)
				{
					VkBuffer vkBuffer = bufferRes->getHandle();

					perSetData.writeInfos[bindingIdx].buffer.buffer = vkBuffer;
					mPerDeviceData[i].buffers[sequentialIdx] = vkBuffer;
				}
				else
				{
					VulkanHardwareBufferManager& vkBufManager = static_cast<VulkanHardwareBufferManager&>(
						HardwareBufferManager::instance());

					perSetData.writeInfos[bindingIdx].buffer.buffer = vkBufManager.getDummyStructuredBuffer(i);
					mPerDeviceData[i].buffers[sequentialIdx] = nullptr;
				}

				writeSetInfo.pTexelBufferView = nullptr;
			}
		}

		mSetsDirty[set] = true;
	}

	void VulkanGpuParams::setSamplerState(UINT32 set, UINT32 slot, const SPtr<SamplerState>& sampler)
	{
		GpuParams::setSamplerState(set, slot, sampler);

		VulkanGpuPipelineParamInfo& vkParamInfo = static_cast<VulkanGpuPipelineParamInfo&>(*mParamInfo);
		UINT32 bindingIdx = vkParamInfo.getBindingIdx(set, slot);
		if (bindingIdx == (UINT32)-1)
		{
			LOGERR("Provided set/slot combination is not used by the GPU program: " + toString(set) + "," +
				   toString(slot) + ".");
			return;
		}

		UINT32 sequentialIdx = vkParamInfo.getSequentialSlot(GpuPipelineParamInfo::ParamType::SamplerState, set, slot);

		Lock lock(mMutex);

		VulkanSamplerState* vulkanSampler = static_cast<VulkanSamplerState*>(sampler.get());
		for(UINT32 i = 0; i < BS_MAX_DEVICES; i++)
		{
			if (mPerDeviceData[i].perSetData == nullptr)
				continue;

			PerSetData& perSetData = mPerDeviceData[i].perSetData[set];

			VulkanSampler* samplerRes;
			if (vulkanSampler != nullptr)
				samplerRes = vulkanSampler->getResource(i);
			else
				samplerRes = nullptr;

			if (samplerRes != nullptr)
			{
				VkSampler vkSampler = samplerRes->getHandle();

				perSetData.writeInfos[bindingIdx].image.sampler = vkSampler;
				mPerDeviceData[i].samplers[sequentialIdx] = vkSampler;
			}
			else
			{
				VulkanSamplerState* defaultSampler = 
					static_cast<VulkanSamplerState*>(SamplerState::getDefault().get());

				VkSampler vkSampler = defaultSampler->getResource(i)->getHandle();;
				perSetData.writeInfos[bindingIdx].image.sampler = vkSampler;

				mPerDeviceData[i].samplers[sequentialIdx] = 0;
			}
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

		VulkanGpuPipelineParamInfo& vkParamInfo = static_cast<VulkanGpuPipelineParamInfo&>(*mParamInfo);

		UINT32 numParamBlocks = vkParamInfo.getNumElements(GpuPipelineParamInfo::ParamType::ParamBlock);
		UINT32 numTextures = vkParamInfo.getNumElements(GpuPipelineParamInfo::ParamType::Texture);
		UINT32 numStorageTextures = vkParamInfo.getNumElements(GpuPipelineParamInfo::ParamType::LoadStoreTexture);
		UINT32 numBuffers = vkParamInfo.getNumElements(GpuPipelineParamInfo::ParamType::Buffer);
		UINT32 numSamplers = vkParamInfo.getNumElements(GpuPipelineParamInfo::ParamType::SamplerState);
		UINT32 numSets = vkParamInfo.getNumSets();

		Lock lock(mMutex);

		// Registers resources with the command buffer, and check if internal resource handled changed (in which case set
		// needs updating - this can happen due to resource writes, as internally system might find it more performant
		// to discard used resources and create new ones).
		// Note: Makes the assumption that this object (and all of the resources it holds) are externally locked, and will
		// not be modified on another thread while being bound.
		for (UINT32 i = 0; i < numParamBlocks; i++)
		{
			if (mParamBlockBuffers[i] == nullptr)
				continue;

			VulkanGpuParamBlockBuffer* element = static_cast<VulkanGpuParamBlockBuffer*>(mParamBlockBuffers[i].get());
			VulkanBuffer* resource = element->getResource(deviceIdx);
			if (resource == nullptr)
				continue;

			UINT32 set, slot;
			mParamInfo->getBinding(GpuPipelineParamInfo::ParamType::ParamBlock, i, set, slot);

			UINT32 bindingIdx = vkParamInfo.getBindingIdx(set, slot);
			VkDescriptorSetLayoutBinding* perSetBindings = vkParamInfo.getBindings(set);
			VkPipelineStageFlags stages = perSetBindings[bindingIdx].stageFlags;

			// Register with command buffer
			buffer.registerBuffer(resource, BufferUseFlagBits::Parameter, VulkanAccessFlag::Read, stages);

			// Check if internal resource changed from what was previously bound in the descriptor set
			assert(perDeviceData.uniformBuffers[i] != VK_NULL_HANDLE);

			VkBuffer vkBuffer = resource->getHandle();
			if(perDeviceData.uniformBuffers[i] != vkBuffer)
			{
				perDeviceData.uniformBuffers[i] = vkBuffer;
				perDeviceData.perSetData[set].writeInfos[bindingIdx].buffer.buffer = vkBuffer;

				mSetsDirty[set] = true;
			}
		}

		for (UINT32 i = 0; i < numBuffers; i++)
		{
			if (mBuffers[i] == nullptr)
				continue;

			VulkanGpuBuffer* element = static_cast<VulkanGpuBuffer*>(mBuffers[i].get());
			VulkanBuffer* resource = element->getResource(deviceIdx);
			if (resource == nullptr)
				continue;

			// Register with command buffer
			VulkanAccessFlags useFlags = VulkanAccessFlag::Read;

			if ((element->getProperties().getUsage() & GBU_LOADSTORE) == GBU_LOADSTORE)
				useFlags |= VulkanAccessFlag::Write;

			UINT32 set, slot;
			mParamInfo->getBinding(GpuPipelineParamInfo::ParamType::Buffer, i, set, slot);

			UINT32 bindingIdx = vkParamInfo.getBindingIdx(set, slot);
			VkDescriptorSetLayoutBinding* perSetBindings = vkParamInfo.getBindings(set);
			VkPipelineStageFlags stages = perSetBindings[bindingIdx].stageFlags;

			buffer.registerBuffer(resource, BufferUseFlagBits::Generic, useFlags, stages);

			// Check if internal resource changed from what was previously bound in the descriptor set
			assert(perDeviceData.buffers[i] != VK_NULL_HANDLE);

			VkBuffer vkBuffer = resource->getHandle();
			if (perDeviceData.buffers[i] != vkBuffer)
			{
				perDeviceData.buffers[i] = vkBuffer;

				PerSetData& perSetData = perDeviceData.perSetData[set];
				VkWriteDescriptorSet& writeSetInfo = perSetData.writeSetInfos[bindingIdx];

				bool useView = writeSetInfo.descriptorType != VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
				if (useView)
				{
					perSetData.writeInfos[bindingIdx].bufferView = element->getView(deviceIdx);
					perSetData.writeSetInfos[bindingIdx].pTexelBufferView = &perSetData.writeInfos[bindingIdx].bufferView;
				}
				else // Structured storage buffer
				{
					perSetData.writeInfos[bindingIdx].buffer.buffer = vkBuffer;
					perSetData.writeSetInfos[bindingIdx].pTexelBufferView = nullptr;
				}

				mSetsDirty[set] = true;
			}
		}

		for (UINT32 i = 0; i < numSamplers; i++)
		{
			if (mSamplerStates[i] == nullptr)
				continue;

			VulkanSamplerState* element = static_cast<VulkanSamplerState*>(mSamplerStates[i].get());
			VulkanSampler* resource = element->getResource(deviceIdx);
			if (resource == nullptr)
				continue;

			// Register with command buffer
			buffer.registerResource(resource, VulkanAccessFlag::Read);

			// Check if internal resource changed from what was previously bound in the descriptor set
			assert(perDeviceData.samplers[i] != VK_NULL_HANDLE);

			VkSampler vkSampler = resource->getHandle();
			if (perDeviceData.samplers[i] != vkSampler)
			{
				perDeviceData.samplers[i] = vkSampler;

				UINT32 set, slot;
				mParamInfo->getBinding(GpuPipelineParamInfo::ParamType::SamplerState, i, set, slot);

				UINT32 bindingIdx = vkParamInfo.getBindingIdx(set, slot);
				perDeviceData.perSetData[set].writeInfos[bindingIdx].image.sampler = vkSampler;

				mSetsDirty[set] = true;
			}
		}

		for (UINT32 i = 0; i < numStorageTextures; i++)
		{
			if (mLoadStoreTextureData[i].texture == nullptr)
				continue;

			VulkanTexture* element = static_cast<VulkanTexture*>(mLoadStoreTextureData[i].texture.get());
			VulkanImage* resource = element->getResource(deviceIdx);
			if (resource == nullptr)
				continue;

			const TextureSurface& surface = mLoadStoreTextureData[i].surface;
			VkImageSubresourceRange range = resource->getRange(surface);

			UINT32 set, slot;
			mParamInfo->getBinding(GpuPipelineParamInfo::ParamType::LoadStoreTexture, i, set, slot);

			UINT32 bindingIdx = vkParamInfo.getBindingIdx(set, slot);
			VkDescriptorSetLayoutBinding* perSetBindings = vkParamInfo.getBindings(set);
			VkPipelineStageFlags stages = perSetBindings[bindingIdx].stageFlags;

			// Register with command buffer
			VulkanAccessFlags useFlags = VulkanAccessFlag::Read | VulkanAccessFlag::Write;
			buffer.registerImageShader(resource, range, VK_IMAGE_LAYOUT_GENERAL, useFlags, stages);

			// Check if internal resource changed from what was previously bound in the descriptor set
			assert(perDeviceData.storageImages[i] != VK_NULL_HANDLE);

			VkImage vkImage = resource->getHandle();
			if (perDeviceData.storageImages[i] != vkImage)
			{
				perDeviceData.storageImages[i] = vkImage;

				perDeviceData.perSetData[set].writeInfos[bindingIdx].image.imageView = resource->getView(surface, false);;

				mSetsDirty[set] = true;
			}
		}

		for (UINT32 i = 0; i < numTextures; i++)
		{
			if (mSampledTextureData[i].texture == nullptr)
				continue;

			VulkanTexture* element = static_cast<VulkanTexture*>(mSampledTextureData[i].texture.get());
			VulkanImage* resource = element->getResource(deviceIdx);
			if (resource == nullptr)
				continue;

			// Register with command buffer
			const TextureProperties& props = element->getProperties();

			const TextureSurface& surface = mSampledTextureData[i].surface;
			VkImageSubresourceRange range = resource->getRange(surface);

			// Keep dynamic textures in general layout, so they can be easily mapped by CPU
			VkImageLayout layout;
			if (props.getUsage() & TU_DYNAMIC)
				layout = VK_IMAGE_LAYOUT_GENERAL;
			else
				layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

			UINT32 set, slot;
			mParamInfo->getBinding(GpuPipelineParamInfo::ParamType::Texture, i, set, slot);

			UINT32 bindingIdx = vkParamInfo.getBindingIdx(set, slot);
			VkDescriptorSetLayoutBinding* perSetBindings = vkParamInfo.getBindings(set);
			VkPipelineStageFlags stages = perSetBindings[bindingIdx].stageFlags;

			buffer.registerImageShader(resource, range, layout, VulkanAccessFlag::Read, stages);

			// Actual layout might be different than requested if the image is also used as a FB attachment
			layout = buffer.getCurrentLayout(resource, range, true);

			// Check if internal resource changed from what was previously bound in the descriptor set
			assert(perDeviceData.sampledImages[i] != VK_NULL_HANDLE);

			VkDescriptorImageInfo& imgInfo = perDeviceData.perSetData[set].writeInfos[bindingIdx].image;

			VkImage vkImage = resource->getHandle();
			if (perDeviceData.sampledImages[i] != vkImage)
			{
				perDeviceData.sampledImages[i] = vkImage;
				imgInfo.imageView = resource->getView(surface, false);

				mSetsDirty[set] = true;
			}

			if(imgInfo.imageLayout != layout)
			{
				imgInfo.imageLayout = layout;
				mSetsDirty[set] = true;
			}
		}

		// Acquire sets as needed, and updated their contents if dirty
		VulkanRenderAPI& rapi = static_cast<VulkanRenderAPI&>(RenderAPI::instance());
		VulkanDevice& device = *rapi._getDevice(deviceIdx);
		VulkanDescriptorManager& descManager = device.getDescriptorManager();

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
				if (perSetData.latestSet == nullptr)
				{
					VulkanDescriptorLayout* layout = vkParamInfo.getLayout(deviceIdx, i);
					perSetData.latestSet = descManager.createSet(layout);
					perSetData.sets.push_back(perSetData.latestSet);
				}
			}

			// Note: Currently I write to the entire set at once, but it might be beneficial to remember only the exact
			// entries that were updated, and only write to them individually.
			perSetData.latestSet->write(perSetData.writeSetInfos, perSetData.numElements);

			mSetsDirty[i] = false;
		}

		for (UINT32 i = 0; i < numSets; i++)
		{
			VulkanDescriptorSet* set = perDeviceData.perSetData[i].latestSet;

			buffer.registerResource(set, VulkanAccessFlag::Read);
			sets[i] = set->getHandle();
		}
	}
}}