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

		UINT32 numSets = mParamInfo->GetNumSets();
		for (UINT32 i = 0; i < BS_MAX_DEVICES; i++)
		{
			if (mPerDeviceData[i].perSetData == nullptr)
				continue;

			for (UINT32 j = 0; j < numSets; j++)
			{
				for (auto& entry : mPerDeviceData[i].perSetData[j].sets)
					entry->Destroy();

				mPerDeviceData[i].perSetData[j].sets.~Vector<VulkanDescriptorSet*>();
			}
		}
	}

	void VulkanGpuParams::Initialize()
	{
		VulkanGpuPipelineParamInfo& vkParamInfo = static_cast<VulkanGpuPipelineParamInfo&>(*mParamInfo);

		VulkanRenderAPI& rapi = static_cast<VulkanRenderAPI&>(RenderAPI::Instance());
		VulkanDevice* devices[BS_MAX_DEVICES];

		VulkanUtility::GetDevices(rapi, mDeviceMask, devices);

		UINT32 numDevices = 0;
		for (UINT32 i = 0; i < BS_MAX_DEVICES; i++)
		{
			if (devices[i] != nullptr)
				numDevices++;
		}

		UINT32 numParamBlocks = vkParamInfo.GetNumElements(GpuPipelineParamInfo::ParamType::ParamBlock);
		UINT32 numTextures = vkParamInfo.GetNumElements(GpuPipelineParamInfo::ParamType::Texture);
		UINT32 numStorageTextures = vkParamInfo.GetNumElements(GpuPipelineParamInfo::ParamType::LoadStoreTexture);
		UINT32 numBuffers = vkParamInfo.GetNumElements(GpuPipelineParamInfo::ParamType::Buffer);
		UINT32 numSamplers = vkParamInfo.GetNumElements(GpuPipelineParamInfo::ParamType::SamplerState);
		UINT32 numSets = vkParamInfo.GetNumSets();
		UINT32 numBindings = vkParamInfo.GetNumElements();

		if (numSets == 0)
			return;

		// Note: I'm assuming a single WriteInfo per binding, but if arrays sizes larger than 1 are eventually supported
		// I'll need to adjust the code.
		mAlloc.Reserve<bool>(numSets)
			.Reserve<PerSetData>(numSets * numDevices)
			.Reserve<VkWriteDescriptorSet>(numBindings * numDevices)
			.Reserve<WriteInfo>(numBindings * numDevices)
			.Reserve<VkImage>(numTextures * numDevices)
			.Reserve<VkImage>(numStorageTextures * numDevices)
			.Reserve<VkBuffer>(numParamBlocks * numDevices)
			.Reserve<VkBuffer>(numBuffers * numDevices)
			.Reserve<VkSampler>(numSamplers * numDevices)
			.Init();

		Lock lock(mMutex); // Set write operations need to be thread safe

		mSetsDirty = mAlloc.Alloc<bool>(numSets);
		bs_zero_out(mSetsDirty, numSets);

		VulkanSamplerState* defaultSampler = static_cast<VulkanSamplerState*>(SamplerState::GetDefault().get());
		VulkanTextureManager& vkTexManager = static_cast<VulkanTextureManager&>(TextureManager::Instance());
		VulkanHardwareBufferManager& vkBufManager = static_cast<VulkanHardwareBufferManager&>(
			HardwareBufferManager::Instance());

		for (UINT32 i = 0; i < BS_MAX_DEVICES; i++)
		{
			if (devices[i] == nullptr)
			{
				mPerDeviceData[i].perSetData = nullptr;

				continue;
			}

			mPerDeviceData[i].perSetData = mAlloc.Alloc<PerSetData>(numSets);
			mPerDeviceData[i].sampledImages = mAlloc.Alloc<VkImage>(numTextures);
			mPerDeviceData[i].storageImages = mAlloc.Alloc<VkImage>(numStorageTextures);
			mPerDeviceData[i].uniformBuffers = mAlloc.Alloc<VkBuffer>(numParamBlocks);
			mPerDeviceData[i].buffers = mAlloc.Alloc<VkBuffer>(numBuffers);
			mPerDeviceData[i].samplers = mAlloc.Alloc<VkSampler>(numSamplers);

			bs_zero_out(mPerDeviceData[i].sampledImages, numTextures);
			bs_zero_out(mPerDeviceData[i].storageImages, numStorageTextures);
			bs_zero_out(mPerDeviceData[i].uniformBuffers, numParamBlocks);
			bs_zero_out(mPerDeviceData[i].buffers, numBuffers);
			bs_zero_out(mPerDeviceData[i].samplers, numSamplers);

			VulkanDescriptorManager& descManager = devices[i]->GetDescriptorManager();
			VulkanSampler* vkDefaultSampler = defaultSampler->GetResource(i);

			for (UINT32 j = 0; j < numSets; j++)
			{
				UINT32 numBindingsPerSet = vkParamInfo.GetNumBindings(j);

				PerSetData& perSetData = mPerDeviceData[i].perSetData[j];
				new (&perSetData.sets) Vector<VulkanDescriptorSet*>();

				perSetData.writeSetInfos = mAlloc.Alloc<VkWriteDescriptorSet>(numBindingsPerSet);
				perSetData.writeInfos = mAlloc.Alloc<WriteInfo>(numBindingsPerSet);

				VulkanDescriptorLayout* layout = vkParamInfo.GetLayout(i, j);
				perSetData.numElements = numBindingsPerSet;
				perSetData.latestSet = descManager.CreateSet(layout);
				perSetData.sets.push_back(perSetData.latestSet);

				VkDescriptorSetLayoutBinding* perSetBindings = vkParamInfo.GetBindings(j);
				GpuParamObjectType* types = vkParamInfo.GetLayoutTypes(j);
				GpuBufferFormat* elementTypes = vkParamInfo.GetLayoutElementTypes(j);
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

					UINT32 slot = perSetBindings[k].binding;

					bool isSampler = writeSetInfo.descriptorType == VK_DESCRIPTOR_TYPE_SAMPLER;
					if(isSampler)
					{
						VkDescriptorImageInfo& imageInfo = perSetData.writeInfos[k].image;
						imageInfo.sampler = vkDefaultSampler->GetHandle();
						imageInfo.imageView = VK_NULL_HANDLE;
						imageInfo.imageLayout = VK_IMAGE_LAYOUT_UNDEFINED;

						writeSetInfo.pImageInfo = &imageInfo;
						writeSetInfo.pBufferInfo = nullptr;
						writeSetInfo.pTexelBufferView = nullptr;
					}
					else
					{
						bool isImage = writeSetInfo.descriptorType == VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER ||
							writeSetInfo.descriptorType == VK_DESCRIPTOR_TYPE_STORAGE_IMAGE ||
							writeSetInfo.descriptorType == VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;

						if (isImage)
						{
							VulkanImage* res = vkTexManager.GetDummyTexture(types[k])->GetResource(i);
							VkFormat format = VulkanTextureManager::GetDummyViewFormat(elementTypes[k]);

							VkDescriptorImageInfo& imageInfo = perSetData.writeInfos[k].image;

							bool isLoadStore = writeSetInfo.descriptorType == VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
							if (isLoadStore)
							{
								imageInfo.sampler = VK_NULL_HANDLE;
								imageInfo.imageView = res->GetView(format, false);
								imageInfo.imageLayout = VK_IMAGE_LAYOUT_GENERAL;

								UINT32 sequentialIdx = vkParamInfo.GetSequentialSlot(GpuPipelineParamInfo::ParamType::LoadStoreTexture, j, slot);
								mPerDeviceData[i].storageImages[sequentialIdx] = res->GetHandle();
							}
							else
							{
								bool isCombinedImageSampler = writeSetInfo.descriptorType == VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;

								if(isCombinedImageSampler)
									imageInfo.sampler = vkDefaultSampler->GetHandle();
								else
									imageInfo.sampler = VK_NULL_HANDLE;

								imageInfo.imageView = res->GetView(format, false);
								imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

								UINT32 sequentialIdx = vkParamInfo.GetSequentialSlot(GpuPipelineParamInfo::ParamType::Texture, j, slot);
								mPerDeviceData[i].sampledImages[sequentialIdx] = res->GetHandle();
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

								if (writeSetInfo.descriptorType == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER)
								{
									VulkanHardwareBuffer* buffer = vkBufManager.GetDummyUniformBuffer();
									bufferInfo.buffer = buffer->GetResource(i)->GetHandle();

									UINT32 sequentialIdx = vkParamInfo.GetSequentialSlot(GpuPipelineParamInfo::ParamType::ParamBlock, j, slot);
									mPerDeviceData[i].uniformBuffers[sequentialIdx] = bufferInfo.buffer;
								}
								else
								{
									VulkanHardwareBuffer* buffer = vkBufManager.GetDummyUniformBuffer();
									bufferInfo.buffer = buffer->GetResource(i)->GetHandle();

									UINT32 sequentialIdx = vkParamInfo.GetSequentialSlot(GpuPipelineParamInfo::ParamType::Buffer, j, slot);
									mPerDeviceData[i].buffers[sequentialIdx] = bufferInfo.buffer;
								}

								writeSetInfo.pBufferInfo = &bufferInfo;
								writeSetInfo.pTexelBufferView = nullptr;
							}
							else
							{
								writeSetInfo.pBufferInfo = nullptr;

								bool isLoadStore = writeSetInfo.descriptorType == VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER;

								VulkanBuffer* buffer;
								if (isLoadStore)
									buffer = vkBufManager.GetDummyStorageBuffer()->GetResource(i);
								else
									buffer = vkBufManager.GetDummyReadBuffer()->GetResource(i);

								VkFormat format = VulkanUtility::GetBufferFormat(elementTypes[k]);
								VkBufferView bufferView = buffer->GetView(format);

								perSetData.writeInfos[k].bufferView = bufferView;
								writeSetInfo.pBufferInfo = nullptr;
								writeSetInfo.pTexelBufferView = &perSetData.writeInfos[k].bufferView;

								UINT32 sequentialIdx = vkParamInfo.GetSequentialSlot(GpuPipelineParamInfo::ParamType::Buffer, j, slot);
								mPerDeviceData[i].buffers[sequentialIdx] = buffer->GetHandle();
							}

							writeSetInfo.pImageInfo = nullptr;
						}
					}
				}
			}
		}

		GpuParams::Initialize();
	}

	void VulkanGpuParams::SetParamBlockBuffer(UINT32 set, UINT32 slot, const SPtr<GpuParamBlockBuffer>& paramBlockBuffer)
	{
		GpuParams::SetParamBlockBuffer(set, slot, paramBlockBuffer);

		VulkanGpuPipelineParamInfo& vkParamInfo = static_cast<VulkanGpuPipelineParamInfo&>(*mParamInfo);
		UINT32 bindingIdx = vkParamInfo.GetBindingIdx(set, slot);
		if(bindingIdx == (UINT32)-1)
		{
			BS_LOG(Error, RenderBackend, "Provided set/slot combination is not used by the GPU program: {0},{1}.",
				set, slot);
			return;
		}

		UINT32 sequentialIdx = vkParamInfo.GetSequentialSlot(GpuPipelineParamInfo::ParamType::ParamBlock, set, slot);

		Lock lock(mMutex);

		auto* vulkanParamBlockBuffer = static_cast<VulkanGpuParamBlockBuffer*>(paramBlockBuffer.get());
		for (UINT32 i = 0; i < BS_MAX_DEVICES; i++)
		{
			if (mPerDeviceData[i].perSetData == nullptr)
				continue;

			VulkanBuffer* bufferRes;
			if (vulkanParamBlockBuffer != nullptr)
				bufferRes = vulkanParamBlockBuffer->GetResource(i);
			else
				bufferRes = nullptr;

			PerSetData& perSetData = mPerDeviceData[i].perSetData[set];
			if (bufferRes != nullptr)
			{
				VkBuffer buffer = bufferRes->GetHandle();

				perSetData.writeInfos[bindingIdx].buffer.buffer = buffer;
				mPerDeviceData[i].uniformBuffers[sequentialIdx] = buffer;
			}
			else
			{
				auto& vkBufManager = static_cast<VulkanHardwareBufferManager&>(HardwareBufferManager::Instance());
				VkBuffer buffer = vkBufManager.GetDummyUniformBuffer()->GetResource(i)->GetHandle();

				perSetData.writeInfos[bindingIdx].buffer.buffer = buffer;
				mPerDeviceData[i].uniformBuffers[sequentialIdx] = buffer;
			}
		}

		mSetsDirty[set] = true;
	}

	void VulkanGpuParams::SetTexture(UINT32 set, UINT32 slot, const SPtr<Texture>& texture, const TextureSurface& surface)
	{
		GpuParams::SetTexture(set, slot, texture, surface);

		VulkanGpuPipelineParamInfo& vkParamInfo = static_cast<VulkanGpuPipelineParamInfo&>(*mParamInfo);
		UINT32 bindingIdx = vkParamInfo.GetBindingIdx(set, slot);
		if (bindingIdx == (UINT32)-1)
		{
			BS_LOG(Error, RenderBackend, "Provided set/slot combination is not used by the GPU program: {0},{1}.",
				set, slot);
			return;
		}

		UINT32 sequentialIdx = vkParamInfo.GetSequentialSlot(GpuPipelineParamInfo::ParamType::Texture, set, slot);

		Lock lock(mMutex);

		VulkanTexture* vulkanTexture = static_cast<VulkanTexture*>(texture.get());
		for (UINT32 i = 0; i < BS_MAX_DEVICES; i++)
		{
			if (mPerDeviceData[i].perSetData == nullptr)
				continue;

			VulkanImage* imageRes;
			if (vulkanTexture != nullptr)
				imageRes = vulkanTexture->GetResource(i);
			else
				imageRes = nullptr;

			PerSetData& perSetData = mPerDeviceData[i].perSetData[set];
			if (imageRes != nullptr)
			{
				auto& texProps = texture->GetProperties();

				TextureSurface actualSurface = surface;
				if (surface.numMipLevels == 0)
					actualSurface.numMipLevels = texProps.GetNumMipmaps() + 1;
				
				if(surface.numFaces == 0)
					actualSurface.numFaces = texProps.GetNumFaces();

				perSetData.writeInfos[bindingIdx].image.imageView = imageRes->GetView(actualSurface, false);
				mPerDeviceData[i].sampledImages[sequentialIdx] = imageRes->GetHandle();
			}
			else
			{
				auto& vkTexManager = static_cast<VulkanTextureManager&>(TextureManager::Instance());

				GpuParamObjectType* types = vkParamInfo.GetLayoutTypes(set);
				GpuBufferFormat* elementTypes = vkParamInfo.GetLayoutElementTypes(set);

				imageRes = vkTexManager.GetDummyTexture(types[bindingIdx])->GetResource(i);
				VkFormat format = VulkanTextureManager::GetDummyViewFormat(elementTypes[bindingIdx]);

				perSetData.writeInfos[bindingIdx].image.imageView = imageRes->GetView(format, false);
				mPerDeviceData[i].sampledImages[sequentialIdx] = imageRes->GetHandle();
			}
		}

		mSetsDirty[set] = true;
	}

	void VulkanGpuParams::SetLoadStoreTexture(UINT32 set, UINT32 slot, const SPtr<Texture>& texture,
		const TextureSurface& surface)
	{
		GpuParams::SetLoadStoreTexture(set, slot, texture, surface);

		VulkanGpuPipelineParamInfo& vkParamInfo = static_cast<VulkanGpuPipelineParamInfo&>(*mParamInfo);
		UINT32 bindingIdx = vkParamInfo.GetBindingIdx(set, slot);
		if (bindingIdx == (UINT32)-1)
		{
			BS_LOG(Error, RenderBackend, "Provided set/slot combination is not used by the GPU program: {0},{1}.",
				set, slot);
			return;
		}

		UINT32 sequentialIdx = vkParamInfo.GetSequentialSlot(GpuPipelineParamInfo::ParamType::LoadStoreTexture, set, slot);

		Lock lock(mMutex);

		VulkanTexture* vulkanTexture = static_cast<VulkanTexture*>(texture.get());
		for (UINT32 i = 0; i < BS_MAX_DEVICES; i++)
		{
			if (mPerDeviceData[i].perSetData == nullptr)
				continue;

			VulkanImage* imageRes;
			if (vulkanTexture != nullptr)
				imageRes = vulkanTexture->GetResource(i);
			else
				imageRes = nullptr;

			PerSetData& perSetData = mPerDeviceData[i].perSetData[set];
			if (imageRes != nullptr)
			{
				perSetData.writeInfos[bindingIdx].image.imageView = imageRes->GetView(surface, false);
				mPerDeviceData[i].storageImages[sequentialIdx] = imageRes->GetHandle();
			}
			else
			{
				auto& vkTexManager = static_cast<VulkanTextureManager&>(TextureManager::Instance());

				GpuParamObjectType* types = vkParamInfo.GetLayoutTypes(set);
				GpuBufferFormat* elementTypes = vkParamInfo.GetLayoutElementTypes(set);

				imageRes = vkTexManager.GetDummyTexture(types[bindingIdx])->GetResource(i);
				VkFormat format = VulkanTextureManager::GetDummyViewFormat(elementTypes[bindingIdx]);

				perSetData.writeInfos[bindingIdx].image.imageView = imageRes->GetView(format, false);
				mPerDeviceData[i].storageImages[sequentialIdx] = imageRes->GetHandle();
			}
		}

		mSetsDirty[set] = true;
	}

	void VulkanGpuParams::SetBuffer(UINT32 set, UINT32 slot, const SPtr<GpuBuffer>& buffer)
	{
		GpuParams::SetBuffer(set, slot, buffer);

		VulkanGpuPipelineParamInfo& vkParamInfo = static_cast<VulkanGpuPipelineParamInfo&>(*mParamInfo);
		UINT32 bindingIdx = vkParamInfo.GetBindingIdx(set, slot);
		if (bindingIdx == (UINT32)-1)
		{
			BS_LOG(Error, RenderBackend, "Provided set/slot combination is not used by the GPU program: {0},{1}.",
				set, slot);
			return;
		}

		UINT32 sequentialIdx = vkParamInfo.GetSequentialSlot(GpuPipelineParamInfo::ParamType::Buffer, set, slot);

		Lock lock(mMutex);

		VulkanGpuBuffer* vulkanBuffer = static_cast<VulkanGpuBuffer*>(buffer.get());
		for (UINT32 i = 0; i < BS_MAX_DEVICES; i++)
		{
			if (mPerDeviceData[i].perSetData == nullptr)
				continue;

			VulkanBuffer* bufferRes;
			
			if (vulkanBuffer != nullptr)
				bufferRes = vulkanBuffer->GetResource(i);
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
					bufferView = vulkanBuffer->GetView(i);
					mPerDeviceData[i].buffers[sequentialIdx] = bufferRes->GetHandle();
				}
				else
				{
					auto& vkBufManager = static_cast<VulkanHardwareBufferManager&>(HardwareBufferManager::Instance());
					bool isLoadStore = writeSetInfo.descriptorType == VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER;

					VulkanBuffer* buffer;
					if (isLoadStore)
						buffer = vkBufManager.GetDummyStorageBuffer()->GetResource(i);
					else
						buffer = vkBufManager.GetDummyReadBuffer()->GetResource(i);

					GpuBufferFormat* elementTypes = vkParamInfo.GetLayoutElementTypes(set);
					VkFormat format = VulkanUtility::GetBufferFormat(elementTypes[bindingIdx]);
					bufferView = buffer->GetView(format);

					mPerDeviceData[i].buffers[sequentialIdx] = buffer->GetHandle();
				}

				perSetData.writeInfos[bindingIdx].bufferView = bufferView;
				writeSetInfo.pTexelBufferView = &perSetData.writeInfos[bindingIdx].bufferView;
			}
			else // Structured storage buffer
			{
				if (bufferRes != nullptr)
				{
					VkBuffer vkBuffer = bufferRes->GetHandle();

					perSetData.writeInfos[bindingIdx].buffer.buffer = vkBuffer;
					mPerDeviceData[i].buffers[sequentialIdx] = vkBuffer;
				}
				else
				{
					auto& vkBufManager = static_cast<VulkanHardwareBufferManager&>(HardwareBufferManager::Instance());

					VkBuffer buffer = vkBufManager.GetDummyStructuredBuffer()->GetResource(i)->GetHandle();

					perSetData.writeInfos[bindingIdx].buffer.buffer = buffer;
					mPerDeviceData[i].buffers[sequentialIdx] = buffer;
				}

				writeSetInfo.pTexelBufferView = nullptr;
			}
		}

		mSetsDirty[set] = true;
	}

	void VulkanGpuParams::SetSamplerState(UINT32 set, UINT32 slot, const SPtr<SamplerState>& sampler)
	{
		GpuParams::SetSamplerState(set, slot, sampler);

		VulkanGpuPipelineParamInfo& vkParamInfo = static_cast<VulkanGpuPipelineParamInfo&>(*mParamInfo);
		UINT32 bindingIdx = vkParamInfo.GetBindingIdx(set, slot);
		if (bindingIdx == (UINT32)-1)
		{
			BS_LOG(Error, RenderBackend, "Provided set/slot combination is not used by the GPU program: {0},{1}.",
				set, slot);
			return;
		}

		UINT32 sequentialIdx = vkParamInfo.GetSequentialSlot(GpuPipelineParamInfo::ParamType::SamplerState, set, slot);

		Lock lock(mMutex);

		VulkanSamplerState* vulkanSampler = static_cast<VulkanSamplerState*>(sampler.get());
		for(UINT32 i = 0; i < BS_MAX_DEVICES; i++)
		{
			if (mPerDeviceData[i].perSetData == nullptr)
				continue;

			PerSetData& perSetData = mPerDeviceData[i].perSetData[set];

			VulkanSampler* samplerRes;
			if (vulkanSampler != nullptr)
				samplerRes = vulkanSampler->GetResource(i);
			else
				samplerRes = nullptr;

			if (samplerRes != nullptr)
			{
				VkSampler vkSampler = samplerRes->GetHandle();

				perSetData.writeInfos[bindingIdx].image.sampler = vkSampler;
				mPerDeviceData[i].samplers[sequentialIdx] = vkSampler;
			}
			else
			{
				VulkanSamplerState* defaultSampler =
					static_cast<VulkanSamplerState*>(SamplerState::GetDefault().get());

				VkSampler vkSampler = defaultSampler->GetResource(i)->GetHandle();;
				perSetData.writeInfos[bindingIdx].image.sampler = vkSampler;

				mPerDeviceData[i].samplers[sequentialIdx] = 0;
			}
		}

		mSetsDirty[set] = true;
	}

	UINT32 VulkanGpuParams::GetNumSets() const
	{
		return mParamInfo->GetNumSets();
	}

	void VulkanGpuParams::PrepareForBind(VulkanCmdBuffer& buffer, VkDescriptorSet* sets)
	{
		UINT32 deviceIdx = buffer.GetDeviceIdx();

		PerDeviceData& perDeviceData = mPerDeviceData[deviceIdx];
		if (perDeviceData.perSetData == nullptr)
			return;

		VulkanGpuPipelineParamInfo& vkParamInfo = static_cast<VulkanGpuPipelineParamInfo&>(*mParamInfo);

		UINT32 numParamBlocks = vkParamInfo.GetNumElements(GpuPipelineParamInfo::ParamType::ParamBlock);
		UINT32 numTextures = vkParamInfo.GetNumElements(GpuPipelineParamInfo::ParamType::Texture);
		UINT32 numStorageTextures = vkParamInfo.GetNumElements(GpuPipelineParamInfo::ParamType::LoadStoreTexture);
		UINT32 numBuffers = vkParamInfo.GetNumElements(GpuPipelineParamInfo::ParamType::Buffer);
		UINT32 numSamplers = vkParamInfo.GetNumElements(GpuPipelineParamInfo::ParamType::SamplerState);
		UINT32 numSets = vkParamInfo.GetNumSets();

		Lock lock(mMutex);

		// Registers resources with the command buffer, and check if internal resource handled changed (in which case set
		// needs updating - this can happen due to resource writes, as internally system might find it more performant
		// to discard used resources and create new ones).
		// Note: Makes the assumption that this object (and all of the resources it holds) are externally locked, and will
		// not be modified on another thread while being bound.
		for (UINT32 i = 0; i < numParamBlocks; i++)
		{
			VulkanBuffer* resource = nullptr;
			if (mParamBlockBuffers[i] != nullptr)
			{
				VulkanGpuParamBlockBuffer* element = static_cast<VulkanGpuParamBlockBuffer*>(mParamBlockBuffers[i].get());
				resource = element->GetResource(deviceIdx);
			}

			if (resource == nullptr)
			{
				auto& vkBufManager = static_cast<VulkanHardwareBufferManager&>(HardwareBufferManager::Instance());
				resource = vkBufManager.GetDummyUniformBuffer()->GetResource(deviceIdx);

				if (resource == nullptr)
					continue;
			}

			UINT32 set, slot;
			mParamInfo->GetBinding(GpuPipelineParamInfo::ParamType::ParamBlock, i, set, slot);

			UINT32 bindingIdx = vkParamInfo.GetBindingIdx(set, slot);
			VkDescriptorSetLayoutBinding* perSetBindings = vkParamInfo.GetBindings(set);
			VkPipelineStageFlags stages = VulkanUtility::ShaderToPipelineStage(perSetBindings[bindingIdx].stageFlags);

			// Register with command buffer
			buffer.RegisterBuffer(resource, BufferUseFlagBits::Parameter, VulkanAccessFlag::Read, stages);

			// Check if internal resource changed from what was previously bound in the descriptor set
			assert(perDeviceData.uniformBuffers[i] != VK_NULL_HANDLE);

			VkBuffer vkBuffer = resource->GetHandle();
			if(perDeviceData.uniformBuffers[i] != vkBuffer)
			{
				perDeviceData.uniformBuffers[i] = vkBuffer;
				perDeviceData.perSetData[set].writeInfos[bindingIdx].buffer.buffer = vkBuffer;

				mSetsDirty[set] = true;
			}
		}

		for (UINT32 i = 0; i < numBuffers; i++)
		{
			UINT32 set, slot;
			mParamInfo->GetBinding(GpuPipelineParamInfo::ParamType::Buffer, i, set, slot);
			UINT32 bindingIdx = vkParamInfo.GetBindingIdx(set, slot);

			GpuParamObjectType* types = vkParamInfo.GetLayoutTypes(set);
			GpuParamObjectType type = types[bindingIdx];

			VulkanAccessFlags useFlags = VulkanAccessFlag::Read;
			VulkanBuffer* resource = nullptr;
			if (mBuffers[i] != nullptr)
			{
				auto* element = static_cast<VulkanGpuBuffer*>(mBuffers[i].get());
				resource = element->GetResource(deviceIdx);

				if ((element->GetProperties().GetUsage() & GBU_LOADSTORE) == GBU_LOADSTORE)
					useFlags |= VulkanAccessFlag::Write;
			}

			if (resource == nullptr)
			{
				auto& vkBufManager = static_cast<VulkanHardwareBufferManager&>(HardwareBufferManager::Instance());

				switch(type)
				{
				case GPOT_BYTE_BUFFER:
					resource = vkBufManager.GetDummyReadBuffer()->GetResource(deviceIdx);
					break;
				case GPOT_RWBYTE_BUFFER:
					resource = vkBufManager.GetDummyStorageBuffer()->GetResource(deviceIdx);
					useFlags |= VulkanAccessFlag::Write;
					break;
				case GPOT_STRUCTURED_BUFFER:
				case GPOT_RWSTRUCTURED_BUFFER:
					resource = vkBufManager.GetDummyStructuredBuffer()->GetResource(deviceIdx);
					useFlags |= VulkanAccessFlag::Write;
					break;
				default:
					break;
				}

				if (resource == nullptr)
					continue;
			}

			// Register with command buffer
			VkDescriptorSetLayoutBinding* perSetBindings = vkParamInfo.GetBindings(set);
			VkPipelineStageFlags stages = VulkanUtility::ShaderToPipelineStage(perSetBindings[bindingIdx].stageFlags);
			buffer.RegisterBuffer(resource, BufferUseFlagBits::Generic, useFlags, stages);

			// Check if internal resource changed from what was previously bound in the descriptor set
			assert(perDeviceData.buffers[i] != VK_NULL_HANDLE);

			VkBuffer vkBuffer = resource->GetHandle();
			if (perDeviceData.buffers[i] != vkBuffer)
			{
				perDeviceData.buffers[i] = vkBuffer;

				VkBufferView view = VK_NULL_HANDLE;
				if (type != GPOT_STRUCTURED_BUFFER && type != GPOT_RWSTRUCTURED_BUFFER)
				{
					if (mBuffers[i] != nullptr)
					{
						auto* element = static_cast<VulkanGpuBuffer*>(mBuffers[i].get());
						view = element->GetView(deviceIdx);
					}
					else
					{
						GpuBufferFormat* elementTypes = vkParamInfo.GetLayoutElementTypes(set);
						view = resource->GetView(VulkanUtility::GetBufferFormat(elementTypes[bindingIdx]));
					}
				}

				PerSetData& perSetData = perDeviceData.perSetData[set];
				if (view)
				{
					perSetData.writeInfos[bindingIdx].bufferView = view;
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
			VulkanSampler* resource = element->GetResource(deviceIdx);
			if (resource == nullptr)
				continue;

			// Register with command buffer
			buffer.RegisterResource(resource, VulkanAccessFlag::Read);

			// Check if internal resource changed from what was previously bound in the descriptor set
			assert(perDeviceData.samplers[i] != VK_NULL_HANDLE);

			VkSampler vkSampler = resource->GetHandle();
			if (perDeviceData.samplers[i] != vkSampler)
			{
				perDeviceData.samplers[i] = vkSampler;

				UINT32 set, slot;
				mParamInfo->GetBinding(GpuPipelineParamInfo::ParamType::SamplerState, i, set, slot);

				UINT32 bindingIdx = vkParamInfo.GetBindingIdx(set, slot);
				perDeviceData.perSetData[set].writeInfos[bindingIdx].image.sampler = vkSampler;

				mSetsDirty[set] = true;
			}
		}

		for (UINT32 i = 0; i < numStorageTextures; i++)
		{
			UINT32 set, slot;
			mParamInfo->GetBinding(GpuPipelineParamInfo::ParamType::LoadStoreTexture, i, set, slot);
			UINT32 bindingIdx = vkParamInfo.GetBindingIdx(set, slot);

			VulkanImage* resource = nullptr;
			if (mLoadStoreTextureData[i].texture != nullptr)
			{
				auto* element = static_cast<VulkanTexture*>(mLoadStoreTextureData[i].texture.get());
				resource = element->GetResource(deviceIdx);
			}

			if(resource == nullptr)
			{
				auto& vkTexManager = static_cast<VulkanTextureManager&>(TextureManager::Instance());

				GpuParamObjectType* types = vkParamInfo.GetLayoutTypes(set);
				resource = vkTexManager.GetDummyTexture(types[bindingIdx])->GetResource(deviceIdx);

				if (resource == nullptr)
					continue;
			}

			const TextureSurface& surface = mLoadStoreTextureData[i].surface;
			VkImageSubresourceRange range = resource->GetRange(surface);

			VkDescriptorSetLayoutBinding* perSetBindings = vkParamInfo.GetBindings(set);
			VkPipelineStageFlags stages = VulkanUtility::ShaderToPipelineStage(perSetBindings[bindingIdx].stageFlags);

			// Register with command buffer
			VulkanAccessFlags useFlags = VulkanAccessFlag::Read | VulkanAccessFlag::Write;
			buffer.RegisterImageShader(resource, range, VK_IMAGE_LAYOUT_GENERAL, useFlags, stages);

			// Check if internal resource changed from what was previously bound in the descriptor set
			assert(perDeviceData.storageImages[i] != VK_NULL_HANDLE);

			VkImage vkImage = resource->GetHandle();
			if (perDeviceData.storageImages[i] != vkImage)
			{
				perDeviceData.storageImages[i] = vkImage;

				VkImageView view;
				if (mLoadStoreTextureData[i].texture)
					view = resource->GetView(surface, false);
				else
				{
					GpuBufferFormat* elementTypes = vkParamInfo.GetLayoutElementTypes(set);
					view = resource->GetView(VulkanTextureManager::GetDummyViewFormat(elementTypes[bindingIdx]));
				}

				perDeviceData.perSetData[set].writeInfos[bindingIdx].image.imageView = view;
				mSetsDirty[set] = true;
			}
		}

		for (UINT32 i = 0; i < numTextures; i++)
		{
			UINT32 set, slot;
			mParamInfo->GetBinding(GpuPipelineParamInfo::ParamType::Texture, i, set, slot);
			UINT32 bindingIdx = vkParamInfo.GetBindingIdx(set, slot);

			VulkanImage* resource = nullptr;
			VkImageLayout layout;
			if (mSampledTextureData[i].texture != nullptr)
			{
				VulkanTexture* element = static_cast<VulkanTexture*>(mSampledTextureData[i].texture.get());
				resource = element->GetResource(deviceIdx);

				const TextureProperties& props = element->GetProperties();
				// Keep dynamic textures in general layout, so they can be easily mapped by CPU
				if (props.GetUsage() & TU_DYNAMIC)
					layout = VK_IMAGE_LAYOUT_GENERAL;
				else
					layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			}

			if(resource == nullptr)
			{
				auto& vkTexManager = static_cast<VulkanTextureManager&>(TextureManager::Instance());

				GpuParamObjectType* types = vkParamInfo.GetLayoutTypes(set);
				resource = vkTexManager.GetDummyTexture(types[bindingIdx])->GetResource(deviceIdx);
				layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

				if (resource == nullptr)
					continue;
			}

			// Register with command buffer
			const TextureSurface& surface = mSampledTextureData[i].surface;
			VkImageSubresourceRange range = resource->GetRange(surface);

			VkDescriptorSetLayoutBinding* perSetBindings = vkParamInfo.GetBindings(set);
			VkPipelineStageFlags stages = VulkanUtility::ShaderToPipelineStage(perSetBindings[bindingIdx].stageFlags);

			buffer.RegisterImageShader(resource, range, layout, VulkanAccessFlag::Read, stages);

			// Actual layout might be different than requested if the image is also used as a FB attachment
			layout = buffer.GetCurrentLayout(resource, range, true);

			// Check if internal resource changed from what was previously bound in the descriptor set
			assert(perDeviceData.sampledImages[i] != VK_NULL_HANDLE);

			VkDescriptorImageInfo& imgInfo = perDeviceData.perSetData[set].writeInfos[bindingIdx].image;

			VkImage vkImage = resource->GetHandle();
			if (perDeviceData.sampledImages[i] != vkImage)
			{
				perDeviceData.sampledImages[i] = vkImage;

				VkImageView view;
				if (mSampledTextureData[i].texture)
					view = resource->GetView(surface, false);
				else
				{
					GpuBufferFormat* elementTypes = vkParamInfo.GetLayoutElementTypes(set);
					view = resource->GetView(VulkanTextureManager::GetDummyViewFormat(elementTypes[bindingIdx]));
				}

				imgInfo.imageView = view;
				mSetsDirty[set] = true;
			}

			if(imgInfo.imageLayout != layout)
			{
				imgInfo.imageLayout = layout;
				mSetsDirty[set] = true;
			}
		}

		// Acquire sets as needed, and updated their contents if dirty
		VulkanRenderAPI& rapi = static_cast<VulkanRenderAPI&>(RenderAPI::Instance());
		VulkanDevice& device = *rapi.GetDeviceInternal(deviceIdx);
		VulkanDescriptorManager& descManager = device.GetDescriptorManager();

		for (UINT32 i = 0; i < numSets; i++)
		{
			PerSetData& perSetData = perDeviceData.perSetData[i];

			if (!mSetsDirty[i]) // Set not dirty, just use the last one we wrote (this is fine even across multiple command buffers)
				continue;

			// Set is dirty, we need to update
			//// Use latest unless already used, otherwise try to find an unused one
			if(perSetData.latestSet->IsBound()) // Checking this is okay, because it's only modified below when we call registerResource, which is under the same lock as this
			{
				perSetData.latestSet = nullptr;

				for(auto& entry : perSetData.sets)
				{
					if(!entry->IsBound())
					{
						perSetData.latestSet = entry;
						break;
					}
				}

				// Cannot find an empty set, allocate a new one
				if (perSetData.latestSet == nullptr)
				{
					VulkanDescriptorLayout* layout = vkParamInfo.GetLayout(deviceIdx, i);
					perSetData.latestSet = descManager.CreateSet(layout);
					perSetData.sets.push_back(perSetData.latestSet);
				}
			}

			// Note: Currently I write to the entire set at once, but it might be beneficial to remember only the exact
			// entries that were updated, and only write to them individually.
			perSetData.latestSet->Write(perSetData.writeSetInfos, perSetData.numElements);

			mSetsDirty[i] = false;
		}

		for (UINT32 i = 0; i < numSets; i++)
		{
			VulkanDescriptorSet* set = perDeviceData.perSetData[i].latestSet;

			buffer.RegisterResource(set, VulkanAccessFlag::Read);
			sets[i] = set->GetHandle();
		}
	}
}}
