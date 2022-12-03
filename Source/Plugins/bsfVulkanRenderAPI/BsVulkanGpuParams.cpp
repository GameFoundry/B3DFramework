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

using namespace bs;
using namespace bs::ct;

VulkanGpuParams::VulkanGpuParams(const SPtr<GpuPipelineParamInfo>& paramInfo, GpuDeviceFlags deviceMask)
	: GpuParams(paramInfo, deviceMask), mPerDeviceData(), mDeviceMask(deviceMask)
{
}

VulkanGpuParams::~VulkanGpuParams()
{
	Lock lock(mMutex);

	u32 numSets = mParamInfo->GetNumSets();
	for(u32 i = 0; i < B3D_MAX_DEVICES; i++)
	{
		if(mPerDeviceData[i].PerSetData == nullptr)
			continue;

		for(u32 j = 0; j < numSets; j++)
		{
			for(auto& entry : mPerDeviceData[i].PerSetData[j].Sets)
				entry->Destroy();

			mPerDeviceData[i].PerSetData[j].Sets.~Vector<VulkanDescriptorSet*>();
		}
	}
}

void VulkanGpuParams::Initialize()
{
	VulkanGpuPipelineParamInfo& vkParamInfo = static_cast<VulkanGpuPipelineParamInfo&>(*mParamInfo);

	VulkanRenderAPI& rapi = static_cast<VulkanRenderAPI&>(RenderAPI::Instance());
	VulkanDevice* devices[B3D_MAX_DEVICES];

	VulkanUtility::GetDevices(rapi, mDeviceMask, devices);

	u32 numDevices = 0;
	for(u32 i = 0; i < B3D_MAX_DEVICES; i++)
	{
		if(devices[i] != nullptr)
			numDevices++;
	}

	u32 numParamBlocks = vkParamInfo.GetNumElements(GpuPipelineParamInfo::ParamType::ParamBlock);
	u32 numTextures = vkParamInfo.GetNumElements(GpuPipelineParamInfo::ParamType::Texture);
	u32 numStorageTextures = vkParamInfo.GetNumElements(GpuPipelineParamInfo::ParamType::LoadStoreTexture);
	u32 numBuffers = vkParamInfo.GetNumElements(GpuPipelineParamInfo::ParamType::Buffer);
	u32 numSamplers = vkParamInfo.GetNumElements(GpuPipelineParamInfo::ParamType::SamplerState);
	u32 numSets = vkParamInfo.GetNumSets();
	u32 numBindings = vkParamInfo.GetNumElements();

	if(numSets == 0)
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
	B3DZeroOut(mSetsDirty, numSets);

	VulkanSamplerState* defaultSampler = static_cast<VulkanSamplerState*>(SamplerState::GetDefault().get());
	VulkanTextureManager& vkTexManager = static_cast<VulkanTextureManager&>(TextureManager::Instance());
	VulkanHardwareBufferManager& vkBufManager = static_cast<VulkanHardwareBufferManager&>(
		HardwareBufferManager::Instance());

	for(u32 i = 0; i < B3D_MAX_DEVICES; i++)
	{
		if(devices[i] == nullptr)
		{
			mPerDeviceData[i].PerSetData = nullptr;

			continue;
		}

		mPerDeviceData[i].PerSetData = mAlloc.Alloc<PerSetData>(numSets);
		mPerDeviceData[i].SampledImages = mAlloc.Alloc<VkImage>(numTextures);
		mPerDeviceData[i].StorageImages = mAlloc.Alloc<VkImage>(numStorageTextures);
		mPerDeviceData[i].UniformBuffers = mAlloc.Alloc<VkBuffer>(numParamBlocks);
		mPerDeviceData[i].Buffers = mAlloc.Alloc<VkBuffer>(numBuffers);
		mPerDeviceData[i].Samplers = mAlloc.Alloc<VkSampler>(numSamplers);

		B3DZeroOut(mPerDeviceData[i].SampledImages, numTextures);
		B3DZeroOut(mPerDeviceData[i].StorageImages, numStorageTextures);
		B3DZeroOut(mPerDeviceData[i].UniformBuffers, numParamBlocks);
		B3DZeroOut(mPerDeviceData[i].Buffers, numBuffers);
		B3DZeroOut(mPerDeviceData[i].Samplers, numSamplers);

		VulkanDescriptorManager& descManager = devices[i]->GetDescriptorManager();
		VulkanSampler* vkDefaultSampler = defaultSampler->GetResource(i);

		for(u32 j = 0; j < numSets; j++)
		{
			u32 numBindingsPerSet = vkParamInfo.GetNumBindings(j);

			PerSetData& perSetData = mPerDeviceData[i].PerSetData[j];
			new(&perSetData.Sets) Vector<VulkanDescriptorSet*>();

			perSetData.WriteSetInfos = mAlloc.Alloc<VkWriteDescriptorSet>(numBindingsPerSet);
			perSetData.WriteInfos = mAlloc.Alloc<WriteInfo>(numBindingsPerSet);

			VulkanDescriptorLayout* layout = vkParamInfo.GetLayout(i, j);
			perSetData.NumElements = numBindingsPerSet;
			perSetData.LatestSet = descManager.CreateSet(layout);
			perSetData.Sets.push_back(perSetData.LatestSet);

			VkDescriptorSetLayoutBinding* perSetBindings = vkParamInfo.GetBindings(j);
			GpuParamObjectType* types = vkParamInfo.GetLayoutTypes(j);
			GpuBufferFormat* elementTypes = vkParamInfo.GetLayoutElementTypes(j);
			for(u32 k = 0; k < numBindingsPerSet; k++)
			{
				// Note: Instead of using one structure per binding, it's possible to update multiple at once
				// by specifying larger descriptorCount, if they all share type and shader stages.
				VkWriteDescriptorSet& writeSetInfo = perSetData.WriteSetInfos[k];
				writeSetInfo.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
				writeSetInfo.pNext = nullptr;
				writeSetInfo.dstSet = VK_NULL_HANDLE;
				writeSetInfo.dstBinding = perSetBindings[k].binding;
				writeSetInfo.dstArrayElement = 0;
				writeSetInfo.descriptorCount = perSetBindings[k].descriptorCount;
				writeSetInfo.descriptorType = perSetBindings[k].descriptorType;

				u32 slot = perSetBindings[k].binding;

				bool isSampler = writeSetInfo.descriptorType == VK_DESCRIPTOR_TYPE_SAMPLER;
				if(isSampler)
				{
					VkDescriptorImageInfo& imageInfo = perSetData.WriteInfos[k].Image;
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

					if(isImage)
					{
						VulkanImage* res = vkTexManager.GetDummyTexture(types[k])->GetResource(i);
						VkFormat format = VulkanTextureManager::GetDummyViewFormat(elementTypes[k]);

						VkDescriptorImageInfo& imageInfo = perSetData.WriteInfos[k].Image;

						bool isLoadStore = writeSetInfo.descriptorType == VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
						if(isLoadStore)
						{
							imageInfo.sampler = VK_NULL_HANDLE;
							imageInfo.imageView = res->GetView(format, false);
							imageInfo.imageLayout = VK_IMAGE_LAYOUT_GENERAL;

							u32 sequentialIdx = vkParamInfo.GetSequentialSlot(GpuPipelineParamInfo::ParamType::LoadStoreTexture, j, slot);
							mPerDeviceData[i].StorageImages[sequentialIdx] = res->GetHandle();
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

							u32 sequentialIdx = vkParamInfo.GetSequentialSlot(GpuPipelineParamInfo::ParamType::Texture, j, slot);
							mPerDeviceData[i].SampledImages[sequentialIdx] = res->GetHandle();
						}

						writeSetInfo.pImageInfo = &imageInfo;
						writeSetInfo.pBufferInfo = nullptr;
						writeSetInfo.pTexelBufferView = nullptr;
					}
					else
					{
						bool useView = writeSetInfo.descriptorType != VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER &&
							writeSetInfo.descriptorType != VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;

						if(!useView)
						{
							VkDescriptorBufferInfo& bufferInfo = perSetData.WriteInfos[k].Buffer;
							bufferInfo.offset = 0;
							bufferInfo.range = VK_WHOLE_SIZE;

							if(writeSetInfo.descriptorType == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER)
							{
								VulkanHardwareBuffer* buffer = vkBufManager.GetDummyUniformBuffer();
								bufferInfo.buffer = buffer->GetResource(i)->GetHandle();

								u32 sequentialIdx = vkParamInfo.GetSequentialSlot(GpuPipelineParamInfo::ParamType::ParamBlock, j, slot);
								mPerDeviceData[i].UniformBuffers[sequentialIdx] = bufferInfo.buffer;
							}
							else
							{
								VulkanHardwareBuffer* buffer = vkBufManager.GetDummyUniformBuffer();
								bufferInfo.buffer = buffer->GetResource(i)->GetHandle();

								u32 sequentialIdx = vkParamInfo.GetSequentialSlot(GpuPipelineParamInfo::ParamType::Buffer, j, slot);
								mPerDeviceData[i].Buffers[sequentialIdx] = bufferInfo.buffer;
							}

							writeSetInfo.pBufferInfo = &bufferInfo;
							writeSetInfo.pTexelBufferView = nullptr;
						}
						else
						{
							writeSetInfo.pBufferInfo = nullptr;

							bool isLoadStore = writeSetInfo.descriptorType == VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER;

							VulkanBuffer* buffer;
							if(isLoadStore)
								buffer = vkBufManager.GetDummyStorageBuffer()->GetResource(i);
							else
								buffer = vkBufManager.GetDummyReadBuffer()->GetResource(i);

							VkFormat format = VulkanUtility::GetBufferFormat(elementTypes[k]);
							VkBufferView bufferView = buffer->GetView(format);

							perSetData.WriteInfos[k].BufferView = bufferView;
							writeSetInfo.pBufferInfo = nullptr;
							writeSetInfo.pTexelBufferView = &perSetData.WriteInfos[k].BufferView;

							u32 sequentialIdx = vkParamInfo.GetSequentialSlot(GpuPipelineParamInfo::ParamType::Buffer, j, slot);
							mPerDeviceData[i].Buffers[sequentialIdx] = buffer->GetHandle();
						}

						writeSetInfo.pImageInfo = nullptr;
					}
				}
			}
		}
	}

	GpuParams::Initialize();
}

void VulkanGpuParams::SetParamBlockBuffer(u32 set, u32 slot, const SPtr<GpuParamBlockBuffer>& paramBlockBuffer)
{
	GpuParams::SetParamBlockBuffer(set, slot, paramBlockBuffer);

	VulkanGpuPipelineParamInfo& vkParamInfo = static_cast<VulkanGpuPipelineParamInfo&>(*mParamInfo);
	u32 bindingIdx = vkParamInfo.GetBindingIdx(set, slot);
	if(bindingIdx == (u32)-1)
	{
		B3D_LOG(Error, RenderBackend, "Provided set/slot combination is not used by the GPU program: {0},{1}.", set, slot);
		return;
	}

	u32 sequentialIdx = vkParamInfo.GetSequentialSlot(GpuPipelineParamInfo::ParamType::ParamBlock, set, slot);

	Lock lock(mMutex);

	auto* vulkanParamBlockBuffer = static_cast<VulkanGpuParamBlockBuffer*>(paramBlockBuffer.get());
	for(u32 i = 0; i < B3D_MAX_DEVICES; i++)
	{
		if(mPerDeviceData[i].PerSetData == nullptr)
			continue;

		VulkanBuffer* bufferRes;
		if(vulkanParamBlockBuffer != nullptr)
			bufferRes = vulkanParamBlockBuffer->GetResource(i);
		else
			bufferRes = nullptr;

		PerSetData& perSetData = mPerDeviceData[i].PerSetData[set];
		if(bufferRes != nullptr)
		{
			VkBuffer buffer = bufferRes->GetHandle();

			perSetData.WriteInfos[bindingIdx].Buffer.buffer = buffer;
			mPerDeviceData[i].UniformBuffers[sequentialIdx] = buffer;
		}
		else
		{
			auto& vkBufManager = static_cast<VulkanHardwareBufferManager&>(HardwareBufferManager::Instance());
			VkBuffer buffer = vkBufManager.GetDummyUniformBuffer()->GetResource(i)->GetHandle();

			perSetData.WriteInfos[bindingIdx].Buffer.buffer = buffer;
			mPerDeviceData[i].UniformBuffers[sequentialIdx] = buffer;
		}
	}

	mSetsDirty[set] = true;
}

void VulkanGpuParams::SetTexture(u32 set, u32 slot, const SPtr<Texture>& texture, const TextureSurface& surface)
{
	GpuParams::SetTexture(set, slot, texture, surface);

	VulkanGpuPipelineParamInfo& vkParamInfo = static_cast<VulkanGpuPipelineParamInfo&>(*mParamInfo);
	u32 bindingIdx = vkParamInfo.GetBindingIdx(set, slot);
	if(bindingIdx == (u32)-1)
	{
		B3D_LOG(Error, RenderBackend, "Provided set/slot combination is not used by the GPU program: {0},{1}.", set, slot);
		return;
	}

	u32 sequentialIdx = vkParamInfo.GetSequentialSlot(GpuPipelineParamInfo::ParamType::Texture, set, slot);

	Lock lock(mMutex);

	VulkanTexture* vulkanTexture = static_cast<VulkanTexture*>(texture.get());
	for(u32 i = 0; i < B3D_MAX_DEVICES; i++)
	{
		if(mPerDeviceData[i].PerSetData == nullptr)
			continue;

		VulkanImage* imageRes;
		if(vulkanTexture != nullptr)
			imageRes = vulkanTexture->GetResource(i);
		else
			imageRes = nullptr;

		PerSetData& perSetData = mPerDeviceData[i].PerSetData[set];
		if(imageRes != nullptr)
		{
			perSetData.WriteInfos[bindingIdx].Image.imageView = imageRes->GetView(surface, false);
			mPerDeviceData[i].SampledImages[sequentialIdx] = imageRes->GetHandle();
		}
		else
		{
			auto& vkTexManager = static_cast<VulkanTextureManager&>(TextureManager::Instance());

			GpuParamObjectType* types = vkParamInfo.GetLayoutTypes(set);
			GpuBufferFormat* elementTypes = vkParamInfo.GetLayoutElementTypes(set);

			imageRes = vkTexManager.GetDummyTexture(types[bindingIdx])->GetResource(i);
			VkFormat format = VulkanTextureManager::GetDummyViewFormat(elementTypes[bindingIdx]);

			perSetData.WriteInfos[bindingIdx].Image.imageView = imageRes->GetView(format, false);
			mPerDeviceData[i].SampledImages[sequentialIdx] = imageRes->GetHandle();
		}
	}

	mSetsDirty[set] = true;
}

void VulkanGpuParams::SetLoadStoreTexture(u32 set, u32 slot, const SPtr<Texture>& texture, const TextureSurface& surface)
{
	GpuParams::SetLoadStoreTexture(set, slot, texture, surface);

	VulkanGpuPipelineParamInfo& vkParamInfo = static_cast<VulkanGpuPipelineParamInfo&>(*mParamInfo);
	u32 bindingIdx = vkParamInfo.GetBindingIdx(set, slot);
	if(bindingIdx == (u32)-1)
	{
		B3D_LOG(Error, RenderBackend, "Provided set/slot combination is not used by the GPU program: {0},{1}.", set, slot);
		return;
	}

	u32 sequentialIdx = vkParamInfo.GetSequentialSlot(GpuPipelineParamInfo::ParamType::LoadStoreTexture, set, slot);

	Lock lock(mMutex);

	VulkanTexture* vulkanTexture = static_cast<VulkanTexture*>(texture.get());
	for(u32 i = 0; i < B3D_MAX_DEVICES; i++)
	{
		if(mPerDeviceData[i].PerSetData == nullptr)
			continue;

		VulkanImage* imageRes;
		if(vulkanTexture != nullptr)
			imageRes = vulkanTexture->GetResource(i);
		else
			imageRes = nullptr;

		PerSetData& perSetData = mPerDeviceData[i].PerSetData[set];
		if(imageRes != nullptr)
		{
			perSetData.WriteInfos[bindingIdx].Image.imageView = imageRes->GetView(surface, false);
			mPerDeviceData[i].StorageImages[sequentialIdx] = imageRes->GetHandle();
		}
		else
		{
			auto& vkTexManager = static_cast<VulkanTextureManager&>(TextureManager::Instance());

			GpuParamObjectType* types = vkParamInfo.GetLayoutTypes(set);
			GpuBufferFormat* elementTypes = vkParamInfo.GetLayoutElementTypes(set);

			imageRes = vkTexManager.GetDummyTexture(types[bindingIdx])->GetResource(i);
			VkFormat format = VulkanTextureManager::GetDummyViewFormat(elementTypes[bindingIdx]);

			perSetData.WriteInfos[bindingIdx].Image.imageView = imageRes->GetView(format, false);
			mPerDeviceData[i].StorageImages[sequentialIdx] = imageRes->GetHandle();
		}
	}

	mSetsDirty[set] = true;
}

void VulkanGpuParams::SetBuffer(u32 set, u32 slot, const SPtr<GpuBuffer>& buffer)
{
	GpuParams::SetBuffer(set, slot, buffer);

	VulkanGpuPipelineParamInfo& vkParamInfo = static_cast<VulkanGpuPipelineParamInfo&>(*mParamInfo);
	u32 bindingIdx = vkParamInfo.GetBindingIdx(set, slot);
	if(bindingIdx == (u32)-1)
	{
		B3D_LOG(Error, RenderBackend, "Provided set/slot combination is not used by the GPU program: {0},{1}.", set, slot);
		return;
	}

	u32 sequentialIdx = vkParamInfo.GetSequentialSlot(GpuPipelineParamInfo::ParamType::Buffer, set, slot);

	Lock lock(mMutex);

	VulkanGpuBuffer* vulkanBuffer = static_cast<VulkanGpuBuffer*>(buffer.get());
	for(u32 i = 0; i < B3D_MAX_DEVICES; i++)
	{
		if(mPerDeviceData[i].PerSetData == nullptr)
			continue;

		VulkanBuffer* bufferRes;

		if(vulkanBuffer != nullptr)
			bufferRes = vulkanBuffer->GetResource(i);
		else
			bufferRes = nullptr;

		PerSetData& perSetData = mPerDeviceData[i].PerSetData[set];
		VkWriteDescriptorSet& writeSetInfo = perSetData.WriteSetInfos[bindingIdx];

		bool useView = writeSetInfo.descriptorType != VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
		if(useView)
		{
			VkBufferView bufferView;
			if(bufferRes != nullptr)
			{
				bufferView = vulkanBuffer->GetView(i);
				mPerDeviceData[i].Buffers[sequentialIdx] = bufferRes->GetHandle();
			}
			else
			{
				auto& vkBufManager = static_cast<VulkanHardwareBufferManager&>(HardwareBufferManager::Instance());
				bool isLoadStore = writeSetInfo.descriptorType == VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER;

				VulkanBuffer* buffer;
				if(isLoadStore)
					buffer = vkBufManager.GetDummyStorageBuffer()->GetResource(i);
				else
					buffer = vkBufManager.GetDummyReadBuffer()->GetResource(i);

				GpuBufferFormat* elementTypes = vkParamInfo.GetLayoutElementTypes(set);
				VkFormat format = VulkanUtility::GetBufferFormat(elementTypes[bindingIdx]);
				bufferView = buffer->GetView(format);

				mPerDeviceData[i].Buffers[sequentialIdx] = buffer->GetHandle();
			}

			perSetData.WriteInfos[bindingIdx].BufferView = bufferView;
			writeSetInfo.pTexelBufferView = &perSetData.WriteInfos[bindingIdx].BufferView;
		}
		else // Structured storage buffer
		{
			if(bufferRes != nullptr)
			{
				VkBuffer vkBuffer = bufferRes->GetHandle();

				perSetData.WriteInfos[bindingIdx].Buffer.buffer = vkBuffer;
				mPerDeviceData[i].Buffers[sequentialIdx] = vkBuffer;
			}
			else
			{
				auto& vkBufManager = static_cast<VulkanHardwareBufferManager&>(HardwareBufferManager::Instance());

				VkBuffer buffer = vkBufManager.GetDummyStructuredBuffer()->GetResource(i)->GetHandle();

				perSetData.WriteInfos[bindingIdx].Buffer.buffer = buffer;
				mPerDeviceData[i].Buffers[sequentialIdx] = buffer;
			}

			writeSetInfo.pTexelBufferView = nullptr;
		}
	}

	mSetsDirty[set] = true;
}

void VulkanGpuParams::SetSamplerState(u32 set, u32 slot, const SPtr<SamplerState>& sampler)
{
	GpuParams::SetSamplerState(set, slot, sampler);

	VulkanGpuPipelineParamInfo& vkParamInfo = static_cast<VulkanGpuPipelineParamInfo&>(*mParamInfo);
	u32 bindingIdx = vkParamInfo.GetBindingIdx(set, slot);
	if(bindingIdx == (u32)-1)
	{
		B3D_LOG(Error, RenderBackend, "Provided set/slot combination is not used by the GPU program: {0},{1}.", set, slot);
		return;
	}

	u32 sequentialIdx = vkParamInfo.GetSequentialSlot(GpuPipelineParamInfo::ParamType::SamplerState, set, slot);

	Lock lock(mMutex);

	VulkanSamplerState* vulkanSampler = static_cast<VulkanSamplerState*>(sampler.get());
	for(u32 i = 0; i < B3D_MAX_DEVICES; i++)
	{
		if(mPerDeviceData[i].PerSetData == nullptr)
			continue;

		PerSetData& perSetData = mPerDeviceData[i].PerSetData[set];

		VulkanSampler* samplerRes;
		if(vulkanSampler != nullptr)
			samplerRes = vulkanSampler->GetResource(i);
		else
			samplerRes = nullptr;

		if(samplerRes != nullptr)
		{
			VkSampler vkSampler = samplerRes->GetHandle();

			perSetData.WriteInfos[bindingIdx].Image.sampler = vkSampler;
			mPerDeviceData[i].Samplers[sequentialIdx] = vkSampler;
		}
		else
		{
			VulkanSamplerState* defaultSampler =
				static_cast<VulkanSamplerState*>(SamplerState::GetDefault().get());

			VkSampler vkSampler = defaultSampler->GetResource(i)->GetHandle();
			;
			perSetData.WriteInfos[bindingIdx].Image.sampler = vkSampler;

			mPerDeviceData[i].Samplers[sequentialIdx] = 0;
		}
	}

	mSetsDirty[set] = true;
}

u32 VulkanGpuParams::GetNumSets() const
{
	return mParamInfo->GetNumSets();
}

void VulkanGpuParams::PrepareForBind(VulkanCmdBuffer& buffer, VkDescriptorSet* sets)
{
	u32 deviceIdx = buffer.GetDeviceIdx();

	PerDeviceData& perDeviceData = mPerDeviceData[deviceIdx];
	if(perDeviceData.PerSetData == nullptr)
		return;

	VulkanGpuPipelineParamInfo& vkParamInfo = static_cast<VulkanGpuPipelineParamInfo&>(*mParamInfo);

	u32 numParamBlocks = vkParamInfo.GetNumElements(GpuPipelineParamInfo::ParamType::ParamBlock);
	u32 numTextures = vkParamInfo.GetNumElements(GpuPipelineParamInfo::ParamType::Texture);
	u32 numStorageTextures = vkParamInfo.GetNumElements(GpuPipelineParamInfo::ParamType::LoadStoreTexture);
	u32 numBuffers = vkParamInfo.GetNumElements(GpuPipelineParamInfo::ParamType::Buffer);
	u32 numSamplers = vkParamInfo.GetNumElements(GpuPipelineParamInfo::ParamType::SamplerState);
	u32 numSets = vkParamInfo.GetNumSets();

	Lock lock(mMutex);

	// Registers resources with the command buffer, and check if internal resource handled changed (in which case set
	// needs updating - this can happen due to resource writes, as internally system might find it more performant
	// to discard used resources and create new ones).
	// Note: Makes the assumption that this object (and all of the resources it holds) are externally locked, and will
	// not be modified on another thread while being bound.
	for(u32 i = 0; i < numParamBlocks; i++)
	{
		VulkanBuffer* resource = nullptr;
		if(mParamBlockBuffers[i] != nullptr)
		{
			VulkanGpuParamBlockBuffer* element = static_cast<VulkanGpuParamBlockBuffer*>(mParamBlockBuffers[i].get());
			resource = element->GetResource(deviceIdx);
		}

		if(resource == nullptr)
		{
			auto& vkBufManager = static_cast<VulkanHardwareBufferManager&>(HardwareBufferManager::Instance());
			resource = vkBufManager.GetDummyUniformBuffer()->GetResource(deviceIdx);

			if(resource == nullptr)
				continue;
		}

		u32 set, slot;
		mParamInfo->GetBinding(GpuPipelineParamInfo::ParamType::ParamBlock, i, set, slot);

		u32 bindingIdx = vkParamInfo.GetBindingIdx(set, slot);
		VkDescriptorSetLayoutBinding* perSetBindings = vkParamInfo.GetBindings(set);
		VkPipelineStageFlags stages = VulkanUtility::ShaderToPipelineStage(perSetBindings[bindingIdx].stageFlags);

		// Register with command buffer
		buffer.RegisterBuffer(resource, BufferUseFlagBits::Parameter, VulkanAccessFlag::Read, stages);

		// Check if internal resource changed from what was previously bound in the descriptor set
		B3D_ASSERT(perDeviceData.UniformBuffers[i] != VK_NULL_HANDLE);

		VkBuffer vkBuffer = resource->GetHandle();
		if(perDeviceData.UniformBuffers[i] != vkBuffer)
		{
			perDeviceData.UniformBuffers[i] = vkBuffer;
			perDeviceData.PerSetData[set].WriteInfos[bindingIdx].Buffer.buffer = vkBuffer;

			mSetsDirty[set] = true;
		}
	}

	for(u32 i = 0; i < numBuffers; i++)
	{
		u32 set, slot;
		mParamInfo->GetBinding(GpuPipelineParamInfo::ParamType::Buffer, i, set, slot);
		u32 bindingIdx = vkParamInfo.GetBindingIdx(set, slot);

		GpuParamObjectType* types = vkParamInfo.GetLayoutTypes(set);
		GpuParamObjectType type = types[bindingIdx];

		VulkanAccessFlags useFlags = VulkanAccessFlag::Read;
		VulkanBuffer* resource = nullptr;
		if(mBuffers[i] != nullptr)
		{
			auto* element = static_cast<VulkanGpuBuffer*>(mBuffers[i].get());
			resource = element->GetResource(deviceIdx);

			if((element->GetProperties().GetUsage() & GBU_LOADSTORE) == GBU_LOADSTORE)
				useFlags |= VulkanAccessFlag::Write;
		}

		if(resource == nullptr)
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

			if(resource == nullptr)
				continue;
		}

		// Register with command buffer
		VkDescriptorSetLayoutBinding* perSetBindings = vkParamInfo.GetBindings(set);
		VkPipelineStageFlags stages = VulkanUtility::ShaderToPipelineStage(perSetBindings[bindingIdx].stageFlags);
		buffer.RegisterBuffer(resource, BufferUseFlagBits::Generic, useFlags, stages);

		// Check if internal resource changed from what was previously bound in the descriptor set
		B3D_ASSERT(perDeviceData.Buffers[i] != VK_NULL_HANDLE);

		VkBuffer vkBuffer = resource->GetHandle();
		if(perDeviceData.Buffers[i] != vkBuffer)
		{
			perDeviceData.Buffers[i] = vkBuffer;

			VkBufferView view = VK_NULL_HANDLE;
			if(type != GPOT_STRUCTURED_BUFFER && type != GPOT_RWSTRUCTURED_BUFFER)
			{
				if(mBuffers[i] != nullptr)
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

			PerSetData& perSetData = perDeviceData.PerSetData[set];
			if(view)
			{
				perSetData.WriteInfos[bindingIdx].BufferView = view;
				perSetData.WriteSetInfos[bindingIdx].pTexelBufferView = &perSetData.WriteInfos[bindingIdx].BufferView;
			}
			else // Structured storage buffer
			{
				perSetData.WriteInfos[bindingIdx].Buffer.buffer = vkBuffer;
				perSetData.WriteSetInfos[bindingIdx].pTexelBufferView = nullptr;
			}

			mSetsDirty[set] = true;
		}
	}

	for(u32 i = 0; i < numSamplers; i++)
	{
		if(mSamplerStates[i] == nullptr)
			continue;

		VulkanSamplerState* element = static_cast<VulkanSamplerState*>(mSamplerStates[i].get());
		VulkanSampler* resource = element->GetResource(deviceIdx);
		if(resource == nullptr)
			continue;

		// Register with command buffer
		buffer.RegisterResource(resource, VulkanAccessFlag::Read);

		// Check if internal resource changed from what was previously bound in the descriptor set
		B3D_ASSERT(perDeviceData.Samplers[i] != VK_NULL_HANDLE);

		VkSampler vkSampler = resource->GetHandle();
		if(perDeviceData.Samplers[i] != vkSampler)
		{
			perDeviceData.Samplers[i] = vkSampler;

			u32 set, slot;
			mParamInfo->GetBinding(GpuPipelineParamInfo::ParamType::SamplerState, i, set, slot);

			u32 bindingIdx = vkParamInfo.GetBindingIdx(set, slot);
			perDeviceData.PerSetData[set].WriteInfos[bindingIdx].Image.sampler = vkSampler;

			mSetsDirty[set] = true;
		}
	}

	for(u32 i = 0; i < numStorageTextures; i++)
	{
		u32 set, slot;
		mParamInfo->GetBinding(GpuPipelineParamInfo::ParamType::LoadStoreTexture, i, set, slot);
		u32 bindingIdx = vkParamInfo.GetBindingIdx(set, slot);

		VulkanImage* resource = nullptr;
		if(mLoadStoreTextureData[i].Texture != nullptr)
		{
			auto* element = static_cast<VulkanTexture*>(mLoadStoreTextureData[i].Texture.get());
			resource = element->GetResource(deviceIdx);
		}

		if(resource == nullptr)
		{
			auto& vkTexManager = static_cast<VulkanTextureManager&>(TextureManager::Instance());

			GpuParamObjectType* types = vkParamInfo.GetLayoutTypes(set);
			resource = vkTexManager.GetDummyTexture(types[bindingIdx])->GetResource(deviceIdx);

			if(resource == nullptr)
				continue;
		}

		const TextureSurface& surface = mLoadStoreTextureData[i].Surface;
		VkImageSubresourceRange range = resource->GetRange(surface);

		VkDescriptorSetLayoutBinding* perSetBindings = vkParamInfo.GetBindings(set);
		VkPipelineStageFlags stages = VulkanUtility::ShaderToPipelineStage(perSetBindings[bindingIdx].stageFlags);

		// Register with command buffer
		VulkanAccessFlags useFlags = VulkanAccessFlag::Read | VulkanAccessFlag::Write;
		buffer.RegisterImageShader(resource, range, VK_IMAGE_LAYOUT_GENERAL, useFlags, stages);

		// Check if internal resource changed from what was previously bound in the descriptor set
		B3D_ASSERT(perDeviceData.StorageImages[i] != VK_NULL_HANDLE);

		VkImage vkImage = resource->GetHandle();
		if(perDeviceData.StorageImages[i] != vkImage)
		{
			perDeviceData.StorageImages[i] = vkImage;

			VkImageView view;
			if(mLoadStoreTextureData[i].Texture)
				view = resource->GetView(surface, false);
			else
			{
				GpuBufferFormat* elementTypes = vkParamInfo.GetLayoutElementTypes(set);
				view = resource->GetView(VulkanTextureManager::GetDummyViewFormat(elementTypes[bindingIdx]));
			}

			perDeviceData.PerSetData[set].WriteInfos[bindingIdx].Image.imageView = view;
			mSetsDirty[set] = true;
		}
	}

	for(u32 i = 0; i < numTextures; i++)
	{
		u32 set, slot;
		mParamInfo->GetBinding(GpuPipelineParamInfo::ParamType::Texture, i, set, slot);
		u32 bindingIdx = vkParamInfo.GetBindingIdx(set, slot);

		VulkanImage* resource = nullptr;
		VkImageLayout layout;
		if(mSampledTextureData[i].Texture != nullptr)
		{
			VulkanTexture* element = static_cast<VulkanTexture*>(mSampledTextureData[i].Texture.get());
			resource = element->GetResource(deviceIdx);

			const TextureProperties& props = element->GetProperties();
			// Keep dynamic textures in general layout, so they can be easily mapped by CPU
			if(props.GetUsage() & TU_DYNAMIC)
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

			if(resource == nullptr)
				continue;
		}

		// Register with command buffer
		const TextureSurface& surface = mSampledTextureData[i].Surface;
		VkImageSubresourceRange range = resource->GetRange(surface);

		VkDescriptorSetLayoutBinding* perSetBindings = vkParamInfo.GetBindings(set);
		VkPipelineStageFlags stages = VulkanUtility::ShaderToPipelineStage(perSetBindings[bindingIdx].stageFlags);

		buffer.RegisterImageShader(resource, range, layout, VulkanAccessFlag::Read, stages);

		// Actual layout might be different than requested if the image is also used as a FB attachment
		layout = buffer.GetCurrentLayout(resource, range, true);

		// Check if internal resource changed from what was previously bound in the descriptor set
		B3D_ASSERT(perDeviceData.SampledImages[i] != VK_NULL_HANDLE);

		VkDescriptorImageInfo& imgInfo = perDeviceData.PerSetData[set].WriteInfos[bindingIdx].Image;

		VkImage vkImage = resource->GetHandle();
		if(perDeviceData.SampledImages[i] != vkImage)
		{
			perDeviceData.SampledImages[i] = vkImage;

			VkImageView view;
			if(mSampledTextureData[i].Texture)
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
	VulkanDevice& device = *rapi.GetDevice(deviceIdx);
	VulkanDescriptorManager& descManager = device.GetDescriptorManager();

	for(u32 i = 0; i < numSets; i++)
	{
		PerSetData& perSetData = perDeviceData.PerSetData[i];

		if(!mSetsDirty[i]) // Set not dirty, just use the last one we wrote (this is fine even across multiple command buffers)
			continue;

		// Set is dirty, we need to update
		//// Use latest unless already used, otherwise try to find an unused one
		if(perSetData.LatestSet->IsBound()) // Checking this is okay, because it's only modified below when we call registerResource, which is under the same lock as this
		{
			perSetData.LatestSet = nullptr;

			for(auto& entry : perSetData.Sets)
			{
				if(!entry->IsBound())
				{
					perSetData.LatestSet = entry;
					break;
				}
			}

			// Cannot find an empty set, allocate a new one
			if(perSetData.LatestSet == nullptr)
			{
				VulkanDescriptorLayout* layout = vkParamInfo.GetLayout(deviceIdx, i);
				perSetData.LatestSet = descManager.CreateSet(layout);
				perSetData.Sets.push_back(perSetData.LatestSet);
			}
		}

		// Note: Currently I write to the entire set at once, but it might be beneficial to remember only the exact
		// entries that were updated, and only write to them individually.
		perSetData.LatestSet->Write(perSetData.WriteSetInfos, perSetData.NumElements);

		mSetsDirty[i] = false;
	}

	for(u32 i = 0; i < numSets; i++)
	{
		VulkanDescriptorSet* set = perDeviceData.PerSetData[i].LatestSet;

		buffer.RegisterResource(set, VulkanAccessFlag::Read);
		sets[i] = set->GetHandle();
	}
}
