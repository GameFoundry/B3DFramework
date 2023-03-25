//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsVulkanGpuParameters.h"
#include "BsVulkanUtility.h"
#include "BsVulkanRenderAPI.h"
#include "BsVulkanGpuDevice.h"
#include "BsVulkanTexture.h"
#include "BsVulkanGpuBuffer.h"
#include "BsVulkanDescriptorSet.h"
#include "BsVulkanDescriptorLayout.h"
#include "BsVulkanSamplerState.h"
#include "BsVulkanGpuPipelineParamInfo.h"
#include "BsVulkanCommandBuffer.h"
#include "Managers/BsVulkanTextureManager.h"
#include "RenderAPI/BsGpuParamDesc.h"

using namespace bs;
using namespace bs::ct;

static bool EnsureImageViewValidForShader(const VulkanImageView& view, const GpuParameterObjectType expectedType)
{
	bool isViewValid = false;
	GpuParameterObjectType actualType = GPOT_UNKNOWN;
	switch(view.Type)
	{
	case VK_IMAGE_VIEW_TYPE_1D:
		isViewValid = GpuObjectParameterTypeInformation::Is1DTexture(expectedType);
		actualType = GPOT_TEXTURE1D;
		break;
	case VK_IMAGE_VIEW_TYPE_2D:
		isViewValid = GpuObjectParameterTypeInformation::Is2DTexture(expectedType);
		actualType = GPOT_TEXTURE2D;
		break;
	case VK_IMAGE_VIEW_TYPE_3D:
		isViewValid = GpuObjectParameterTypeInformation::Is3DTexture(expectedType);
		actualType = GPOT_TEXTURE3D;
		break;
	case VK_IMAGE_VIEW_TYPE_CUBE:
		isViewValid = GpuObjectParameterTypeInformation::IsCubeTexture(expectedType);
		actualType = GPOT_TEXTURECUBE;
		break;
	case VK_IMAGE_VIEW_TYPE_1D_ARRAY:
		isViewValid = GpuObjectParameterTypeInformation::Is1DTextureArray(expectedType);
		actualType = GPOT_TEXTURE1DARRAY;
		break;
	case VK_IMAGE_VIEW_TYPE_2D_ARRAY:
		isViewValid = GpuObjectParameterTypeInformation::Is2DTextureArray(expectedType);
		actualType = GPOT_TEXTURE2DARRAY;
		break;
	case VK_IMAGE_VIEW_TYPE_CUBE_ARRAY:
		isViewValid = GpuObjectParameterTypeInformation::IsCubeTextureArray(expectedType);
		actualType = GPOT_TEXTURECUBEARRAY;
		break;
	default: break;
	}

	if(!isViewValid)
	{
		B3D_LOG(Error, RenderBackend, "Unable to bind texture. Image view is not of expected type. Expected {0} but got {1}.", (u32)expectedType, (u32)actualType);
	}

	return isViewValid;
}

VulkanGpuParameters::VulkanGpuParameters(VulkanGpuDevice& gpuDevice, const SPtr<GpuPipelineParamInfo>& parameterLayout)
	: GpuParameters(parameterLayout), mGpuDevice(gpuDevice), mPerDeviceData()
{
}

VulkanGpuParameters::~VulkanGpuParameters()
{
	Lock lock(mMutex);

	u32 numSets = mParameterLayout->GetSetCount();
	if(mPerDeviceData.PerSetData == nullptr)
		return;

	for(u32 j = 0; j < numSets; j++)
	{
		for(auto& entry : mPerDeviceData.PerSetData[j].Sets)
			entry->Destroy();

		mPerDeviceData.PerSetData[j].Sets.~Vector<VulkanDescriptorSet*>();
	}
}

void VulkanGpuParameters::Initialize()
{
	VulkanGpuPipelineParamInfo& vkParamInfo = static_cast<VulkanGpuPipelineParamInfo&>(*mParameterLayout);

	const u32 parameterBlockCount = vkParamInfo.GetResourceCount(GpuPipelineParamInfo::GpuParameterType::UniformBuffer);
	const u32 sampledTextureCount = vkParamInfo.GetResourceCount(GpuPipelineParamInfo::GpuParameterType::SampledTexture);
	const u32 storageTextureCount = vkParamInfo.GetResourceCount(GpuPipelineParamInfo::GpuParameterType::StorageTexture);
	const u32 bufferCount = vkParamInfo.GetResourceCount(GpuPipelineParamInfo::GpuParameterType::StorageBuffer);
	const u32 samplerCount = vkParamInfo.GetResourceCount(GpuPipelineParamInfo::GpuParameterType::Sampler);
	const u32 setCount = vkParamInfo.GetSetCount();
	const u32 resourceCount = vkParamInfo.GetResourceCount();
	const u32 bindingCount = vkParamInfo.GetBindingSlotCount();

	if(setCount == 0)
		return;

	// Note: I'm assuming a single WriteInfo per binding, but if arrays sizes larger than 1 are eventually supported
	// I'll need to adjust the code.
	mAlloc.Reserve<bool>(setCount)
		.Reserve<PerSetData>(setCount)
		.Reserve<VkWriteDescriptorSet>(bindingCount)
		.Reserve<VkDescriptorImageInfo>(resourceCount)
		.Reserve<VkDescriptorBufferInfo>(resourceCount)
		.Reserve<VkBufferView>(resourceCount)
		.Reserve<VkImage>(sampledTextureCount)
		.Reserve<VkImage>(storageTextureCount)
		.Reserve<VkBuffer>(parameterBlockCount)
		.Reserve<VkBuffer>(bufferCount)
		.Reserve<VkSampler>(samplerCount)
		.Init();

	Lock lock(mMutex); // Set write operations need to be thread safe

	mSetsDirty = mAlloc.Alloc<bool>(setCount);
	B3DZeroOut(mSetsDirty, setCount);

	VulkanSamplerState* defaultSampler = static_cast<VulkanSamplerState*>(SamplerState::GetDefault().get());
	VulkanTextureManager& vkTexManager = static_cast<VulkanTextureManager&>(TextureManager::Instance());
	const VulkanBuiltinResources& builtinResources = (mGpuDevice.GetBuiltinResources());

	mPerDeviceData.PerSetData = mAlloc.Alloc<PerSetData>(setCount);
	mPerDeviceData.SampledImages = mAlloc.Alloc<VkImage>(sampledTextureCount);
	mPerDeviceData.StorageImages = mAlloc.Alloc<VkImage>(storageTextureCount);
	mPerDeviceData.UniformBuffers = mAlloc.Alloc<VkBuffer>(parameterBlockCount);
	mPerDeviceData.Buffers = mAlloc.Alloc<VkBuffer>(bufferCount);
	mPerDeviceData.Samplers = mAlloc.Alloc<VkSampler>(samplerCount);

	B3DZeroOut(mPerDeviceData.SampledImages, sampledTextureCount);
	B3DZeroOut(mPerDeviceData.StorageImages, storageTextureCount);
	B3DZeroOut(mPerDeviceData.UniformBuffers, parameterBlockCount);
	B3DZeroOut(mPerDeviceData.Buffers, bufferCount);
	B3DZeroOut(mPerDeviceData.Samplers, samplerCount);

	VulkanDescriptorManager& descManager = mGpuDevice.GetDescriptorManager();
	VulkanSampler* vkDefaultSampler = defaultSampler->GetResource(0);

	for(u32 setIndex = 0; setIndex < setCount; setIndex++)
	{
		const u32 layoutBindingCount = vkParamInfo.GetLayoutBindingCount(setIndex);
		const u32 layoutResourceCount = vkParamInfo.GetLayoutResourceCount(setIndex);

		PerSetData& perSetData = mPerDeviceData.PerSetData[setIndex];
		new(&perSetData.Sets) Vector<VulkanDescriptorSet*>();

		perSetData.WriteSetInfos = mAlloc.Alloc<VkWriteDescriptorSet>(layoutBindingCount);
		perSetData.ImageWriteInfos = mAlloc.Alloc<VkDescriptorImageInfo>(layoutResourceCount);
		perSetData.BufferWriteInfos = mAlloc.Alloc<VkDescriptorBufferInfo>(layoutResourceCount);
		perSetData.BufferViews = mAlloc.Alloc<VkBufferView>(layoutResourceCount);

		VulkanDescriptorLayout* layout = vkParamInfo.GetLayout(0, setIndex);
		perSetData.ElementCount = layoutBindingCount;
		perSetData.LastFreeSetIndex = 0;
		perSetData.LastUsedSet = descManager.CreateSet(layout);
		perSetData.Sets.push_back(perSetData.LastUsedSet);

		VkDescriptorSetLayoutBinding* perSetBindings = vkParamInfo.GetLayoutBindings(setIndex);
		GpuParameterObjectType* types = vkParamInfo.GetLayoutTypes(setIndex);
		GpuBufferFormat* elementTypes = vkParamInfo.GetLayoutElementTypes(setIndex);
		for(u32 layoutBindingIndex = 0; layoutBindingIndex < layoutBindingCount; layoutBindingIndex++)
		{
			// Note: Instead of using one structure per binding, it's possible to update multiple at once
			// by specifying larger descriptorCount, if they all share type and shader stages.
			VkWriteDescriptorSet& writeSetInfo = perSetData.WriteSetInfos[layoutBindingIndex];
			writeSetInfo.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			writeSetInfo.pNext = nullptr;
			writeSetInfo.dstSet = VK_NULL_HANDLE;
			writeSetInfo.dstBinding = perSetBindings[layoutBindingIndex].binding;
			writeSetInfo.dstArrayElement = 0;
			writeSetInfo.descriptorCount = perSetBindings[layoutBindingIndex].descriptorCount;
			writeSetInfo.descriptorType = perSetBindings[layoutBindingIndex].descriptorType;

			const u32 slot = perSetBindings[layoutBindingIndex].binding;

			const bool isSampler = writeSetInfo.descriptorType == VK_DESCRIPTOR_TYPE_SAMPLER;
			if(isSampler)
			{
				const u32 usedResourceSequentialIndex = vkParamInfo.GetUsedResourceSequentialIndex(setIndex, slot, 0);

				VkDescriptorImageInfo* imageInfos = &perSetData.ImageWriteInfos[usedResourceSequentialIndex];
				for(u32 arrayIndex = 0; arrayIndex < writeSetInfo.descriptorCount; arrayIndex++)
				{
					imageInfos[arrayIndex].sampler = vkDefaultSampler->GetHandle();
					imageInfos[arrayIndex].imageView = VK_NULL_HANDLE;
					imageInfos[arrayIndex].imageLayout = VK_IMAGE_LAYOUT_UNDEFINED;
				}

				writeSetInfo.pImageInfo = imageInfos;
				writeSetInfo.pBufferInfo = nullptr;
				writeSetInfo.pTexelBufferView = nullptr;
			}
			else
			{
				const bool isImage = writeSetInfo.descriptorType == VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER ||
					writeSetInfo.descriptorType == VK_DESCRIPTOR_TYPE_STORAGE_IMAGE ||
					writeSetInfo.descriptorType == VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;

				if(isImage)
				{
					const VulkanImage *const imageResource = vkTexManager.GetDummyTexture(types[layoutBindingIndex])->GetResource(0);
					const VkFormat format = VulkanTextureManager::GetDummyViewFormat(elementTypes[layoutBindingIndex]);

					const bool isLoadStore = writeSetInfo.descriptorType == VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
					const u32 usedResourceSequentialIndex = vkParamInfo.GetUsedResourceSequentialIndex(setIndex, slot, 0);
					const u32 sequentialResourceIndex = isLoadStore
						? vkParamInfo.GetSequentialResourceIndex(GpuPipelineParamInfo::GpuParameterType::StorageTexture, setIndex, slot, 0)
						: vkParamInfo.GetSequentialResourceIndex(GpuPipelineParamInfo::GpuParameterType::SampledTexture, setIndex, slot, 0);

					VkDescriptorImageInfo* imageInfos = &perSetData.ImageWriteInfos[usedResourceSequentialIndex];
					for(u32 arrayIndex = 0; arrayIndex < writeSetInfo.descriptorCount; arrayIndex++)
					{
						if(isLoadStore)
						{
							imageInfos[arrayIndex].sampler = VK_NULL_HANDLE;
							imageInfos[arrayIndex].imageView = imageResource->GetView(format, false).Handle;
							imageInfos[arrayIndex].imageLayout = VK_IMAGE_LAYOUT_GENERAL;

							mPerDeviceData.StorageImages[sequentialResourceIndex + arrayIndex] = imageResource->GetHandle();
						}
						else
						{
							const bool isCombinedImageSampler = writeSetInfo.descriptorType == VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
							if(isCombinedImageSampler)
								imageInfos[arrayIndex].sampler = vkDefaultSampler->GetHandle();
							else
								imageInfos[arrayIndex].sampler = VK_NULL_HANDLE;

							imageInfos[arrayIndex].imageView = imageResource->GetView(format, false).Handle;
							imageInfos[arrayIndex].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

							mPerDeviceData.SampledImages[sequentialResourceIndex + arrayIndex] = imageResource->GetHandle();
						}
					}

					writeSetInfo.pImageInfo = imageInfos;
					writeSetInfo.pBufferInfo = nullptr;
					writeSetInfo.pTexelBufferView = nullptr;
				}
				else
				{
					const bool useView = writeSetInfo.descriptorType != VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER &&
						writeSetInfo.descriptorType != VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC &&
						writeSetInfo.descriptorType != VK_DESCRIPTOR_TYPE_STORAGE_BUFFER &&
						writeSetInfo.descriptorType != VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC;

					if(!useView)
					{
						const bool isParameterBlock =
							writeSetInfo.descriptorType == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER ||
							writeSetInfo.descriptorType == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;

						const u32 usedResourceSequentialIndex = vkParamInfo.GetUsedResourceSequentialIndex(setIndex, slot, 0);
						const u32 sequentialResourceIndex = isParameterBlock
							? vkParamInfo.GetSequentialResourceIndex(GpuPipelineParamInfo::GpuParameterType::UniformBuffer, setIndex, slot, 0)
							: vkParamInfo.GetSequentialResourceIndex(GpuPipelineParamInfo::GpuParameterType::StorageBuffer, setIndex, slot, 0);

						VkDescriptorBufferInfo* bufferInfos = &perSetData.BufferWriteInfos[usedResourceSequentialIndex];
						for(u32 arrayIndex = 0; arrayIndex < writeSetInfo.descriptorCount; arrayIndex++)
						{
							bufferInfos[arrayIndex].offset = 0;
							bufferInfos[arrayIndex].range = VK_WHOLE_SIZE;

							if(isParameterBlock)
							{
								VulkanGpuBuffer* const buffer = builtinResources.DummyUniformBuffer.get();
								bufferInfos[arrayIndex].buffer = buffer->GetResource(0)->GetHandle();

								mPerDeviceData.UniformBuffers[sequentialResourceIndex + arrayIndex] = bufferInfos[arrayIndex].buffer;
							}
							else
							{
								VulkanGpuBuffer* const buffer = builtinResources.DummyUniformBuffer.get();
								bufferInfos[arrayIndex].buffer = buffer->GetResource(0)->GetHandle();

								mPerDeviceData.Buffers[sequentialResourceIndex + arrayIndex] = bufferInfos[arrayIndex].buffer;
							}
						}

						writeSetInfo.pBufferInfo = bufferInfos;
						writeSetInfo.pTexelBufferView = nullptr;
					}
					else
					{
						writeSetInfo.pBufferInfo = nullptr;

						const bool isLoadStore = writeSetInfo.descriptorType == VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER;

						VulkanBuffer* buffer;
						if(isLoadStore)
							buffer = builtinResources.DummyStorageBuffer->GetResource(0);
						else
							buffer = builtinResources.DummyReadBuffer->GetResource(0);

						const VkFormat format = VulkanUtility::GetBufferFormat(elementTypes[layoutBindingIndex]);
						const VkBufferView bufferView = buffer->GetOrCreateView(format);

						const u32 usedResourceSequentialIndex = vkParamInfo.GetUsedResourceSequentialIndex(setIndex, slot, 0);
						const u32 sequentialResourceIndex = vkParamInfo.GetSequentialResourceIndex(GpuPipelineParamInfo::GpuParameterType::StorageBuffer, setIndex, slot, 0);

						VkBufferView* const bufferViews = &perSetData.BufferViews[usedResourceSequentialIndex];
						for(u32 arrayIndex = 0; arrayIndex < writeSetInfo.descriptorCount; arrayIndex++)
						{
							bufferViews[arrayIndex] = bufferView;

							mPerDeviceData.Buffers[sequentialResourceIndex + arrayIndex] = buffer->GetHandle();
						}

						writeSetInfo.pBufferInfo = nullptr;
						writeSetInfo.pTexelBufferView = bufferViews;
					}

					writeSetInfo.pImageInfo = nullptr;
				}
			}
		}
	}

	GpuParameters::Initialize();
}

bool VulkanGpuParameters::SetUniformBuffer(u32 set, u32 slot,const SPtr<GpuBuffer>& paramBlockBuffer, u32 arrayIndex, u32 offset)
{
	if (!GpuParameters::SetUniformBuffer(set, slot, paramBlockBuffer, arrayIndex, offset))
		return false;

	VulkanGpuPipelineParamInfo& pipelineParameterInformation = static_cast<VulkanGpuPipelineParamInfo&>(*mParameterLayout);
	const u32 usedResourceSequentialIndex = pipelineParameterInformation.GetUsedResourceSequentialIndex(set, slot, arrayIndex);
	if(usedResourceSequentialIndex == ~0u)
	{
		B3D_LOG(Error, RenderBackend, "Provided set/slot combination is not used by the GPU program: {0},{1}.", set, slot);
		return false;
	}

	const u32 sequentialResourceIndex = pipelineParameterInformation.GetSequentialResourceIndex(GpuPipelineParamInfo::GpuParameterType::UniformBuffer, set, slot, arrayIndex);

	Lock lock(mMutex);

	auto* vulkanParamBlockBuffer = static_cast<VulkanGpuBuffer*>(paramBlockBuffer.get());
	if(mPerDeviceData.PerSetData == nullptr)
		return false;

	VulkanBuffer* vulkanBuffer;
	VkDeviceSize bufferSize;
	if(vulkanParamBlockBuffer != nullptr)
	{
		vulkanBuffer = vulkanParamBlockBuffer->GetResource(0);
		bufferSize = vulkanParamBlockBuffer->GetSize();
	}
	else
	{
		vulkanBuffer = nullptr;
		bufferSize = VK_WHOLE_SIZE;
	}

	PerSetData& perSetData = mPerDeviceData.PerSetData[set];

	VkBuffer vkBuffer = VK_NULL_HANDLE;
	if(vulkanBuffer != nullptr)
	{
		vkBuffer = vulkanBuffer->GetHandle();
	}
	else
	{
		const VulkanBuiltinResources& builtinResources = (mGpuDevice.GetBuiltinResources());
		vkBuffer = builtinResources.DummyUniformBuffer->GetResource(0)->GetHandle();
	}

	if(vkBuffer != mPerDeviceData.UniformBuffers[sequentialResourceIndex])
	{
		perSetData.BufferWriteInfos[usedResourceSequentialIndex].buffer = vkBuffer;
		perSetData.BufferWriteInfos[usedResourceSequentialIndex].range = bufferSize;
		mPerDeviceData.UniformBuffers[sequentialResourceIndex] = vkBuffer;

		mSetsDirty[set] = true;
	}

	return true;
}

bool VulkanGpuParameters::SetSampledTexture(u32 set, u32 slot, const SPtr<Texture>& texture, const TextureSurface& surface, u32 arrayIndex)
{
	if (!GpuParameters::SetSampledTexture(set, slot, texture, surface, arrayIndex))
		return false;

	VulkanGpuPipelineParamInfo& pipelineParameterInformation = static_cast<VulkanGpuPipelineParamInfo&>(*mParameterLayout);

	const u32 usedBindingSequentialIndex = pipelineParameterInformation.GetUsedBindingSequentialIndex(set, slot);
	const u32 usedResourceSequentialIndex = pipelineParameterInformation.GetUsedResourceSequentialIndex(set, slot, arrayIndex);

	if(usedResourceSequentialIndex == ~0u || usedBindingSequentialIndex == ~0u)
	{
		B3D_LOG(Error, RenderBackend, "Provided set/slot combination is not used by the GPU program: {0},{1}.", set, slot);
		return false;
	}

	const u32 sequentialResourceIndex = pipelineParameterInformation.GetSequentialResourceIndex(GpuPipelineParamInfo::GpuParameterType::SampledTexture, set, slot, arrayIndex);

	Lock lock(mMutex);

	VulkanTexture* vulkanTexture = static_cast<VulkanTexture*>(texture.get());
	if(mPerDeviceData.PerSetData == nullptr)
		return false;

	VulkanImage* vulkanImage;
	if(vulkanTexture != nullptr)
		vulkanImage = vulkanTexture->GetResource(0);
	else
		vulkanImage = nullptr;

	PerSetData& perSetData = mPerDeviceData.PerSetData[set];
	const GpuParameterObjectType* const types = pipelineParameterInformation.GetLayoutTypes(set);
	const GpuParameterObjectType objectType = types[usedBindingSequentialIndex];

	VulkanImageView imageView;
	VkImage vkImage = VK_NULL_HANDLE;
	if(vulkanImage != nullptr)
	{
		imageView = vulkanImage->GetView(surface, false);
		vkImage = vulkanImage->GetHandle();

		if(!EnsureImageViewValidForShader(imageView, objectType))
			vulkanImage = nullptr;
	}

	if(vulkanImage == nullptr)
	{
		auto& vkTexManager = static_cast<VulkanTextureManager&>(TextureManager::Instance());

		GpuBufferFormat* elementTypes = pipelineParameterInformation.GetLayoutElementTypes(set);

		vulkanImage = vkTexManager.GetDummyTexture(types[usedBindingSequentialIndex])->GetResource(0);
		VkFormat format = VulkanTextureManager::GetDummyViewFormat(elementTypes[usedBindingSequentialIndex]);

		imageView = vulkanImage->GetView(format, false);
		vkImage = vulkanImage->GetHandle();
	}

	if(vkImage != mPerDeviceData.SampledImages[sequentialResourceIndex] || imageView.Handle != perSetData.ImageWriteInfos[usedResourceSequentialIndex].imageView)
	{
		perSetData.ImageWriteInfos[usedResourceSequentialIndex].imageView = imageView.Handle;
		mPerDeviceData.SampledImages[sequentialResourceIndex] = vkImage;

		mSetsDirty[set] = true;
	}

	return true;
}

bool VulkanGpuParameters::SetStorageTexture(u32 set, u32 slot, const SPtr<Texture>& texture, const TextureSurface& surface, u32 arrayIndex)
{
	if (!GpuParameters::SetStorageTexture(set, slot, texture, surface, arrayIndex))
		return false;

	VulkanGpuPipelineParamInfo& pipelineParameterInformation = static_cast<VulkanGpuPipelineParamInfo&>(*mParameterLayout);
	const u32 usedBindingSequentialIndex = pipelineParameterInformation.GetUsedBindingSequentialIndex(set, slot);
	const u32 usedResourceSequentialIndex = pipelineParameterInformation.GetUsedResourceSequentialIndex(set, slot, arrayIndex);

	if(usedBindingSequentialIndex == ~0u || usedResourceSequentialIndex == ~0u)
	{
		B3D_LOG(Error, RenderBackend, "Provided set/slot combination is not used by the GPU program: {0},{1}.", set, slot);
		return false;
	}

	const u32 sequentialResourceIndex = pipelineParameterInformation.GetSequentialResourceIndex(GpuPipelineParamInfo::GpuParameterType::StorageTexture, set, slot, arrayIndex);

	Lock lock(mMutex);

	VulkanTexture* vulkanTexture = static_cast<VulkanTexture*>(texture.get());
	if(mPerDeviceData.PerSetData == nullptr)
		return false;

	VulkanImage* vulkanImage;
	if(vulkanTexture != nullptr)
		vulkanImage = vulkanTexture->GetResource(0);
	else
		vulkanImage = nullptr;

	PerSetData& perSetData = mPerDeviceData.PerSetData[set];
	const GpuParameterObjectType* const types = pipelineParameterInformation.GetLayoutTypes(set);
	const GpuParameterObjectType objectType = types[usedBindingSequentialIndex];

	VulkanImageView imageView;
	VkImage vkImage = VK_NULL_HANDLE;
	if(vulkanImage != nullptr)
	{
		imageView = vulkanImage->GetView(surface, false);
		vkImage = vulkanImage->GetHandle();

		if(!EnsureImageViewValidForShader(imageView, objectType))
			vulkanImage = nullptr;
	}

	if(vulkanImage == nullptr)
	{
		auto& vkTexManager = static_cast<VulkanTextureManager&>(TextureManager::Instance());

		GpuBufferFormat* elementTypes = pipelineParameterInformation.GetLayoutElementTypes(set);

		vulkanImage = vkTexManager.GetDummyTexture(types[usedBindingSequentialIndex])->GetResource(0);
		VkFormat format = VulkanTextureManager::GetDummyViewFormat(elementTypes[usedBindingSequentialIndex]);

		imageView = vulkanImage->GetView(format, false);
		vkImage = vulkanImage->GetHandle();
	}

	if(vkImage != mPerDeviceData.StorageImages[sequentialResourceIndex] || imageView.Handle != perSetData.ImageWriteInfos[usedResourceSequentialIndex].imageView)
	{
		perSetData.ImageWriteInfos[usedResourceSequentialIndex].imageView = imageView.Handle;
		mPerDeviceData.StorageImages[sequentialResourceIndex] = vkImage;

		mSetsDirty[set] = true;
	}

	return true;
}

bool VulkanGpuParameters::SetStorageBuffer(u32 set, u32 slot, const SPtr<GpuBuffer>& buffer, u32 arrayIndex, GpuStorageBufferViewInformation view)
{
	if (!GpuParameters::SetStorageBuffer(set, slot, buffer, arrayIndex, view))
		return false;

	VulkanGpuPipelineParamInfo& vkParamInfo = static_cast<VulkanGpuPipelineParamInfo&>(*mParameterLayout);
	const u32 usedBindingSequentialIndex = vkParamInfo.GetUsedBindingSequentialIndex(set, slot);
	const u32 usedResourceSequentialIndex = vkParamInfo.GetUsedResourceSequentialIndex(set, slot, arrayIndex);

	if(usedBindingSequentialIndex == ~0u || usedResourceSequentialIndex == ~0u)
	{
		B3D_LOG(Error, RenderBackend, "Provided set/slot combination is not used by the GPU program: {0},{1}.", set, slot);
		return false;
	}

	const u32 sequentialResourceIndex = vkParamInfo.GetSequentialResourceIndex(GpuPipelineParamInfo::GpuParameterType::StorageBuffer, set, slot, arrayIndex);

	Lock lock(mMutex);

	VulkanGpuBuffer* vulkanBuffer = static_cast<VulkanGpuBuffer*>(buffer.get());
	if(mPerDeviceData.PerSetData == nullptr)
		return false;

	VulkanBuffer* bufferResource;
	if(vulkanBuffer != nullptr)
		bufferResource = vulkanBuffer->GetResource(0);
	else
		bufferResource = nullptr;

	PerSetData& perSetData = mPerDeviceData.PerSetData[set];
	VkWriteDescriptorSet& writeSetInfo = perSetData.WriteSetInfos[usedBindingSequentialIndex];

	const bool useView = writeSetInfo.descriptorType != VK_DESCRIPTOR_TYPE_STORAGE_BUFFER && writeSetInfo.descriptorType != VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC;
	VkBufferView vkBufferView = VK_NULL_HANDLE;
	if(bufferResource == nullptr)
	{
		const VulkanBuiltinResources& builtinResources = (mGpuDevice.GetBuiltinResources());
		if(useView)
		{
			const bool isLoadStore = writeSetInfo.descriptorType == VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER;
			if(isLoadStore)
				bufferResource = builtinResources.DummyStorageBuffer->GetResource(0);
			else
				bufferResource = builtinResources.DummyReadBuffer->GetResource(0);

			const GpuBufferFormat* const elementTypes = vkParamInfo.GetLayoutElementTypes(set);
			const VkFormat format = VulkanUtility::GetBufferFormat(elementTypes[usedBindingSequentialIndex]);
			vkBufferView = bufferResource->GetOrCreateView(format);
		}
		else
		{
			bufferResource = builtinResources.DummyStructuredBuffer->GetResource(0);
		}
	}
	else
	{
		if(useView)
		{
			const GpuBufferFormat format = mStorageBufferData[sequentialResourceIndex].View.Format;
			vkBufferView = vulkanBuffer->GetOrCreateView(format);
		}
	}

	const VkBuffer vkBuffer = bufferResource->GetHandle();

	const bool isBufferViewChanged = useView && perSetData.BufferViews[usedResourceSequentialIndex] != vkBufferView;
	if(mPerDeviceData.Buffers[sequentialResourceIndex] != vkBuffer || isBufferViewChanged)
	{
		if(useView)
		{
			perSetData.BufferViews[usedResourceSequentialIndex] = vkBufferView;
			writeSetInfo.pTexelBufferView = &perSetData.BufferViews[usedResourceSequentialIndex];
		}
		else // Structured storage buffer
		{
			perSetData.BufferWriteInfos[usedResourceSequentialIndex].buffer = vkBuffer;
			mPerDeviceData.Buffers[sequentialResourceIndex] = vkBuffer;

			writeSetInfo.pTexelBufferView = nullptr;
		}

		mPerDeviceData.Buffers[sequentialResourceIndex] = vkBuffer;
		mSetsDirty[set] = true;
	}

	return true;
}

bool VulkanGpuParameters::SetSamplerState(u32 set, u32 slot, const SPtr<SamplerState>& sampler, u32 arrayIndex)
{
	if (!GpuParameters::SetSamplerState(set, slot, sampler, arrayIndex))
		return false;

	VulkanGpuPipelineParamInfo& vkParamInfo = static_cast<VulkanGpuPipelineParamInfo&>(*mParameterLayout);
	const u32 usedResourceSequentialIndex = vkParamInfo.GetUsedResourceSequentialIndex(set, slot, arrayIndex);
	if(usedResourceSequentialIndex == ~0u)
	{
		B3D_LOG(Error, RenderBackend, "Provided set/slot combination is not used by the GPU program: {0},{1}.", set, slot);
		return false;
	}

	const u32 sequentialResourceIndex = vkParamInfo.GetSequentialResourceIndex(GpuPipelineParamInfo::GpuParameterType::Sampler, set, slot, arrayIndex);

	Lock lock(mMutex);

	VulkanSamplerState* vulkanSampler = static_cast<VulkanSamplerState*>(sampler.get());
	if(mPerDeviceData.PerSetData == nullptr)
		return false;

	PerSetData& perSetData = mPerDeviceData.PerSetData[set];

	VulkanSampler* samplerRes;
	if(vulkanSampler != nullptr)
		samplerRes = vulkanSampler->GetResource(0);
	else
	{
		const VulkanSamplerState* const defaultSampler =
			static_cast<const VulkanSamplerState* const>(SamplerState::GetDefault().get());

		samplerRes = defaultSampler->GetResource(0);
	}

	VkSampler vkSampler = samplerRes->GetHandle();
	if(mPerDeviceData.Samplers[sequentialResourceIndex] != vkSampler)
	{
		perSetData.ImageWriteInfos[usedResourceSequentialIndex].sampler = vkSampler;
		mPerDeviceData.Samplers[sequentialResourceIndex] = vkSampler;

		mSetsDirty[set] = true;
	}

	mSetsDirty[set] = true;
	return true;
}

u32 VulkanGpuParameters::GetSetCount() const
{
	return mParameterLayout->GetSetCount();
}

void VulkanGpuParameters::PrepareForBind(VulkanInternalCommandBuffer& buffer, VkDescriptorSet* outSets, Vector<u32>& outDynamicOffsets)
{
	u32 deviceIdx = buffer.GetDeviceIndex();

	PerDeviceData& perDeviceData = mPerDeviceData;
	if(perDeviceData.PerSetData == nullptr)
		return;

	VulkanGpuPipelineParamInfo& vkParamInfo = static_cast<VulkanGpuPipelineParamInfo&>(*mParameterLayout);

	u32 parameterBlockBindingCount = vkParamInfo.GetBindingSlotCount(GpuPipelineParamInfo::GpuParameterType::UniformBuffer);
	u32 sampledTextureBindingCount = vkParamInfo.GetBindingSlotCount(GpuPipelineParamInfo::GpuParameterType::SampledTexture);
	u32 storageTextureBindingCount = vkParamInfo.GetBindingSlotCount(GpuPipelineParamInfo::GpuParameterType::StorageTexture);
	u32 bufferBindingCount = vkParamInfo.GetBindingSlotCount(GpuPipelineParamInfo::GpuParameterType::StorageBuffer);
	u32 samplerBindingCount = vkParamInfo.GetBindingSlotCount(GpuPipelineParamInfo::GpuParameterType::Sampler);
	u32 setCount = vkParamInfo.GetSetCount();

	FrameScope frameScope;
	FrameVector<u32> dynamicOffsetMapping(perDeviceData.PerSetData->ElementCount, ~0u);

	Lock lock(mMutex);

	// Registers resources with the command buffer, and check if internal resource handled changed (in which case set
	// needs updating - this can happen due to resource writes, as internally system might find it more performant
	// to discard used resources and create new ones).
	// Note: Makes the assumption that this object (and all of the resources it holds) are externally locked, and will
	// not be modified on another thread while being bound.
	for(u32 sequentialBindingIndex = 0; sequentialBindingIndex < parameterBlockBindingCount; sequentialBindingIndex++)
	{
		const u32 arraySize = vkParamInfo.GetArraySize(GpuPipelineParamInfoBase::GpuParameterType::UniformBuffer, sequentialBindingIndex);

		u32 set, slot;
		mParameterLayout->GetBinding(GpuPipelineParamInfoBase::GpuParameterType::UniformBuffer, sequentialBindingIndex, set, slot);

		for(u32 arrayIndex = 0; arrayIndex < arraySize; arrayIndex++)
		{
			const u32 sequentialResourceIndex = vkParamInfo.GetSequentialResourceIndex(GpuPipelineParamInfoBase::GpuParameterType::UniformBuffer, set, slot, arrayIndex);
			const u32 usedBindingSequentialIndex = vkParamInfo.GetUsedBindingSequentialIndex(set, slot);
			const u32 usedResourceSequentialIndex = vkParamInfo.GetUsedResourceSequentialIndex(set, slot, arrayIndex);

			VulkanBuffer* resource = nullptr;
			VkDeviceSize bufferSize = VK_WHOLE_SIZE;
			u32 dynamicOffset = 0;

			if(mUniformBufferData[sequentialResourceIndex].Buffer != nullptr)
			{
				VulkanGpuBuffer *const element = static_cast<VulkanGpuBuffer*>(mUniformBufferData[sequentialResourceIndex].Buffer.get());
				resource = element->GetResource(deviceIdx);
				bufferSize = element->GetSize();
				dynamicOffset = mUniformBufferData[sequentialResourceIndex].Offset;
			}

			if(resource == nullptr)
			{
				const VulkanBuiltinResources& builtinResources = mGpuDevice.GetBuiltinResources();
				resource = builtinResources.DummyUniformBuffer->GetResource(deviceIdx);

				if(resource == nullptr)
					continue;
			}

			VkDescriptorSetLayoutBinding* perSetBindings = vkParamInfo.GetLayoutBindings(set);
			VkPipelineStageFlags stages = VulkanUtility::ShaderToPipelineStage(perSetBindings[usedBindingSequentialIndex].stageFlags);

			// Register with command buffer
			buffer.RegisterBuffer(resource, BufferUseFlagBits::Parameter, VulkanAccessFlag::Read, stages);

			// Check if internal resource changed from what was previously bound in the descriptor set
			B3D_ASSERT(perDeviceData.UniformBuffers[sequentialResourceIndex] != VK_NULL_HANDLE);

			VkBuffer vkBuffer = resource->GetHandle();
			if(perDeviceData.UniformBuffers[sequentialResourceIndex] != vkBuffer)
			{
				perDeviceData.UniformBuffers[sequentialResourceIndex] = vkBuffer;
				perDeviceData.PerSetData[set].BufferWriteInfos[usedResourceSequentialIndex].buffer = vkBuffer;
				perDeviceData.PerSetData[set].BufferWriteInfos[usedResourceSequentialIndex].range = bufferSize;

				mSetsDirty[set] = true;
			}

			dynamicOffsetMapping[usedBindingSequentialIndex] = dynamicOffset;
		}
	}

	for(u32 sequentialBindingIndex = 0; sequentialBindingIndex < bufferBindingCount; sequentialBindingIndex++)
	{
		const u32 arraySize = vkParamInfo.GetArraySize(GpuPipelineParamInfoBase::GpuParameterType::StorageBuffer, sequentialBindingIndex);

		u32 set, slot;
		mParameterLayout->GetBinding(GpuPipelineParamInfo::GpuParameterType::StorageBuffer, sequentialBindingIndex, set, slot);

		for(u32 arrayIndex = 0; arrayIndex < arraySize; ++arrayIndex)
		{
			const u32 sequentialResourceIndex = vkParamInfo.GetSequentialResourceIndex(GpuPipelineParamInfo::GpuParameterType::StorageBuffer, set, slot, arrayIndex);
			const u32 usedBindingSequentialIndex = vkParamInfo.GetUsedBindingSequentialIndex(set, slot);
			const u32 usedResourceSequentialIndex = vkParamInfo.GetUsedResourceSequentialIndex(set, slot, arrayIndex);

			GpuParameterObjectType* types = vkParamInfo.GetLayoutTypes(set);
			GpuParameterObjectType type = types[usedBindingSequentialIndex];

			VulkanAccessFlags useFlags = VulkanAccessFlag::Read;
			VulkanBuffer* resource = nullptr;

			const bool supportsDynamicOffset = type == GPOT_STRUCTURED_BUFFER || type == GPOT_RWSTRUCTURED_BUFFER;
			u32 dynamicOffset = supportsDynamicOffset ? 0 : ~0u;

			if(mStorageBufferData[sequentialResourceIndex].Buffer != nullptr)
			{
				if(supportsDynamicOffset)
					dynamicOffset = mStorageBufferData[sequentialResourceIndex].View.Offset;

				auto* element = static_cast<VulkanGpuBuffer*>(mStorageBufferData[sequentialResourceIndex].Buffer.get());
				resource = element->GetResource(deviceIdx);

				if(element->GetInformation().Flags.IsSet(GpuBufferFlag::AllowWritesOnTheGPU))
					useFlags |= VulkanAccessFlag::Write;
			}

			if(resource == nullptr)
			{
				const VulkanBuiltinResources& builtinResources = mGpuDevice.GetBuiltinResources();

				switch(type)
				{
				case GPOT_BYTE_BUFFER:
					resource = builtinResources.DummyReadBuffer->GetResource(deviceIdx);
					break;
				case GPOT_RWBYTE_BUFFER:
					resource = builtinResources.DummyStorageBuffer->GetResource(deviceIdx);
					useFlags |= VulkanAccessFlag::Write;
					break;
				case GPOT_STRUCTURED_BUFFER:
				case GPOT_RWSTRUCTURED_BUFFER:
					resource = builtinResources.DummyStructuredBuffer->GetResource(deviceIdx);
					useFlags |= VulkanAccessFlag::Write;
					break;
				default:
					break;
				}

				if(resource == nullptr)
					continue;
			}

			// Register with command buffer
			VkDescriptorSetLayoutBinding* perSetBindings = vkParamInfo.GetLayoutBindings(set);
			VkPipelineStageFlags stages = VulkanUtility::ShaderToPipelineStage(perSetBindings[usedBindingSequentialIndex].stageFlags);
			buffer.RegisterBuffer(resource, BufferUseFlagBits::Generic, useFlags, stages);

			// Check if internal resource changed from what was previously bound in the descriptor set
			B3D_ASSERT(perDeviceData.Buffers[sequentialResourceIndex] != VK_NULL_HANDLE);

			VkBuffer vkBuffer = resource->GetHandle();
			if(perDeviceData.Buffers[sequentialResourceIndex] != vkBuffer)
			{
				perDeviceData.Buffers[sequentialResourceIndex] = vkBuffer;

				VkBufferView view = VK_NULL_HANDLE;
				if(type != GPOT_STRUCTURED_BUFFER && type != GPOT_RWSTRUCTURED_BUFFER)
				{
					if(mStorageBufferData[sequentialResourceIndex].Buffer != nullptr)
					{
						auto* element = static_cast<VulkanGpuBuffer*>(mStorageBufferData[sequentialResourceIndex].Buffer.get());
						view = element->GetOrCreateView(mStorageBufferData[sequentialResourceIndex].View.Format);
					}
					else
					{
						GpuBufferFormat* elementTypes = vkParamInfo.GetLayoutElementTypes(set);
						view = resource->GetOrCreateView(VulkanUtility::GetBufferFormat(elementTypes[usedBindingSequentialIndex]));
					}
				}

				PerSetData& perSetData = perDeviceData.PerSetData[set];
				if(view)
				{
					const u32 usedResourceSequentialFirstArrayEntryIndex = vkParamInfo.GetUsedResourceSequentialIndex(set, slot, arrayIndex);

					perSetData.BufferViews[usedResourceSequentialIndex] = view;
					perSetData.WriteSetInfos[usedBindingSequentialIndex].pTexelBufferView = &perSetData.BufferViews[usedResourceSequentialFirstArrayEntryIndex];
				}
				else // Structured storage buffer
				{
					perSetData.BufferWriteInfos[usedResourceSequentialIndex].buffer = vkBuffer;
					perSetData.WriteSetInfos[usedBindingSequentialIndex].pTexelBufferView = nullptr;
				}

				mSetsDirty[set] = true;
			}

			dynamicOffsetMapping[usedBindingSequentialIndex] = dynamicOffset;
		}
	}

	for(u32 sequentialBindingIndex = 0; sequentialBindingIndex < samplerBindingCount; sequentialBindingIndex++)
	{
		const u32 arraySize = vkParamInfo.GetArraySize(GpuPipelineParamInfoBase::GpuParameterType::Sampler, sequentialBindingIndex);

		u32 set, slot;
		mParameterLayout->GetBinding(GpuPipelineParamInfoBase::GpuParameterType::Sampler, sequentialBindingIndex, set, slot);

		for(u32 arrayIndex = 0; arrayIndex < arraySize; arrayIndex++)
		{
			const u32 sequentialResourceIndex = vkParamInfo.GetSequentialResourceIndex(GpuPipelineParamInfoBase::GpuParameterType::Sampler, set, slot, arrayIndex);

			if(mSamplerStates[sequentialResourceIndex] == nullptr)
				continue;

			VulkanSamplerState* element = static_cast<VulkanSamplerState*>(mSamplerStates[sequentialResourceIndex].get());
			VulkanSampler* resource = element->GetResource(deviceIdx);
			if(resource == nullptr)
				continue;

			// Register with command buffer
			buffer.RegisterResource(resource, VulkanAccessFlag::Read);

			// Check if internal resource changed from what was previously bound in the descriptor set
			B3D_ASSERT(perDeviceData.Samplers[sequentialResourceIndex] != VK_NULL_HANDLE);

			VkSampler vkSampler = resource->GetHandle();
			if(perDeviceData.Samplers[sequentialResourceIndex] != vkSampler)
			{
				perDeviceData.Samplers[sequentialResourceIndex] = vkSampler;

				const u32 usedResourceSequentialIndex = vkParamInfo.GetUsedResourceSequentialIndex(set, slot, arrayIndex);
				perDeviceData.PerSetData[set].ImageWriteInfos[usedResourceSequentialIndex].sampler = vkSampler;

				mSetsDirty[set] = true;
			}
		}
	}

	for(u32 sequentialBindingIndex = 0; sequentialBindingIndex < storageTextureBindingCount; sequentialBindingIndex++)
	{
		const u32 arraySize = vkParamInfo.GetArraySize(GpuPipelineParamInfoBase::GpuParameterType::StorageTexture, sequentialBindingIndex);

		u32 set, slot;
		mParameterLayout->GetBinding(GpuPipelineParamInfoBase::GpuParameterType::StorageTexture, sequentialBindingIndex, set, slot);

		const u32 usedBindingSequentialIndex = vkParamInfo.GetUsedBindingSequentialIndex(set, slot);
		for(u32 arrayIndex = 0; arrayIndex < arraySize; arrayIndex++)
		{
			const u32 sequentialResourceIndex = vkParamInfo.GetSequentialResourceIndex(GpuPipelineParamInfoBase::GpuParameterType::StorageTexture, set, slot, arrayIndex);

			VulkanImage* vulkanImage = nullptr;
			if(mStorageTextureData[sequentialResourceIndex].Texture != nullptr)
			{
				auto* element = static_cast<VulkanTexture*>(mStorageTextureData[sequentialResourceIndex].Texture.get());
				vulkanImage = element->GetResource(deviceIdx);
			}

			const TextureSurface& surface = mStorageTextureData[sequentialResourceIndex].Surface;
			const GpuParameterObjectType* const types = vkParamInfo.GetLayoutTypes(set);
			const GpuParameterObjectType objectType = types[usedBindingSequentialIndex];

			VulkanImageView imageView;
			if(vulkanImage != nullptr)
			{
				imageView = vulkanImage->GetView(surface, false);

				if(!EnsureImageViewValidForShader(imageView, objectType))
					vulkanImage = nullptr;
			}

			if(vulkanImage == nullptr)
			{
				auto& vkTexManager = static_cast<VulkanTextureManager&>(TextureManager::Instance());

				vulkanImage = vkTexManager.GetDummyTexture(objectType)->GetResource(deviceIdx);

				if(vulkanImage == nullptr)
					continue;

				const GpuBufferFormat* const elementTypes = vkParamInfo.GetLayoutElementTypes(set);
				const VkFormat format = VulkanTextureManager::GetDummyViewFormat(elementTypes[usedBindingSequentialIndex]);

				imageView = vulkanImage->GetView(format, false);
			}

			// Register with command buffer
			VkDescriptorSetLayoutBinding* perSetBindings = vkParamInfo.GetLayoutBindings(set);
			VkPipelineStageFlags stages = VulkanUtility::ShaderToPipelineStage(perSetBindings[usedBindingSequentialIndex].stageFlags);

			const VulkanAccessFlags useFlags = VulkanAccessFlag::Read | VulkanAccessFlag::Write;
			const VkImageSubresourceRange range = vulkanImage->GetRange(surface);

			buffer.RegisterImageShader(vulkanImage, range, VK_IMAGE_LAYOUT_GENERAL, useFlags, stages);

			// Check if internal resource changed from what was previously bound in the descriptor set
			B3D_ASSERT(perDeviceData.StorageImages[sequentialResourceIndex] != VK_NULL_HANDLE);

			VkImage vkImage = vulkanImage->GetHandle();
			if(perDeviceData.StorageImages[sequentialResourceIndex] != vkImage)
			{
				perDeviceData.StorageImages[sequentialResourceIndex] = vkImage;

				const u32 usedResourceSequentialIndex = vkParamInfo.GetUsedResourceSequentialIndex(set, slot, arrayIndex);
				perDeviceData.PerSetData[set].ImageWriteInfos[usedResourceSequentialIndex].imageView = imageView.Handle;

				mSetsDirty[set] = true;
			}
		}
	}

	for(u32 sequentialBindingIndex = 0; sequentialBindingIndex < sampledTextureBindingCount; sequentialBindingIndex++)
	{
		const u32 arraySize = vkParamInfo.GetArraySize(GpuPipelineParamInfoBase::GpuParameterType::SampledTexture, sequentialBindingIndex);

		u32 set, slot;
		mParameterLayout->GetBinding(GpuPipelineParamInfoBase::GpuParameterType::SampledTexture, sequentialBindingIndex, set, slot);

		const u32 usedBindingSequentialIndex = vkParamInfo.GetUsedBindingSequentialIndex(set, slot);

		for(u32 arrayIndex = 0; arrayIndex < arraySize; arrayIndex++)
		{
			const u32 sequentialResourceIndex = vkParamInfo.GetSequentialResourceIndex(GpuPipelineParamInfoBase::GpuParameterType::SampledTexture, set, slot, arrayIndex);
			const u32 usedResourceSequentialIndex = vkParamInfo.GetUsedResourceSequentialIndex(set, slot, arrayIndex);

			VulkanImage* vulkanImage = nullptr;
			VkImageLayout layout;
			if(mSampledTextureData[sequentialResourceIndex].Texture != nullptr)
			{
				VulkanTexture* element = static_cast<VulkanTexture*>(mSampledTextureData[sequentialResourceIndex].Texture.get());
				vulkanImage = element->GetResource(deviceIdx);

				// Keep dynamic textures in general layout, so they can be easily mapped by CPU
				const TextureProperties& props = element->GetProperties();
				if(props.Usage & TU_DYNAMIC)
					layout = VK_IMAGE_LAYOUT_GENERAL;
				else
					layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			}

			const TextureSurface& surface = mSampledTextureData[sequentialResourceIndex].Surface;
			const GpuParameterObjectType* const types = vkParamInfo.GetLayoutTypes(set);
			const GpuParameterObjectType objectType = types[usedBindingSequentialIndex];

			VulkanImageView imageView;
			if(vulkanImage != nullptr)
			{
				imageView = vulkanImage->GetView(surface, false);

				if(!EnsureImageViewValidForShader(imageView, objectType))
					vulkanImage = nullptr;
			}

			if(vulkanImage == nullptr)
			{
				auto& vkTexManager = static_cast<VulkanTextureManager&>(TextureManager::Instance());

				vulkanImage = vkTexManager.GetDummyTexture(objectType)->GetResource(deviceIdx);
				layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

				if(vulkanImage == nullptr)
					continue;

				const GpuBufferFormat* const elementTypes = vkParamInfo.GetLayoutElementTypes(set);
				const VkFormat format = VulkanTextureManager::GetDummyViewFormat(elementTypes[usedBindingSequentialIndex]);

				imageView = vulkanImage->GetView(format, false);
			}

			// Register with command buffer
			VkImageSubresourceRange range = vulkanImage->GetRange(surface);

			VkDescriptorSetLayoutBinding* perSetBindings = vkParamInfo.GetLayoutBindings(set);
			VkPipelineStageFlags stages = VulkanUtility::ShaderToPipelineStage(perSetBindings[usedBindingSequentialIndex].stageFlags);

			buffer.RegisterImageShader(vulkanImage, range, layout, VulkanAccessFlag::Read, stages);

			// Actual layout might be different than requested if the image is also used as a FB attachment
			layout = buffer.GetCurrentLayout(vulkanImage, range, true);

			// Check if internal resource changed from what was previously bound in the descriptor set
			B3D_ASSERT(perDeviceData.SampledImages[sequentialResourceIndex] != VK_NULL_HANDLE);

			VkDescriptorImageInfo& imageInfo = perDeviceData.PerSetData[set].ImageWriteInfos[usedResourceSequentialIndex];

			VkImage vkImage = vulkanImage->GetHandle();
			if(perDeviceData.SampledImages[sequentialResourceIndex] != vkImage)
			{
				perDeviceData.SampledImages[sequentialResourceIndex] = vkImage;

				imageInfo.imageView = imageView.Handle;
				mSetsDirty[set] = true;
			}

			if(imageInfo.imageLayout != layout)
			{
				imageInfo.imageLayout = layout;
				mSetsDirty[set] = true;
			}
		}
	}

	// Output dynamic offsets
	for(u32 dynamicOffset : dynamicOffsetMapping)
	{
		if(dynamicOffset != ~0u)
			outDynamicOffsets.push_back(dynamicOffset);
	}

	// Acquire sets as needed, and updated their contents if dirty
	const SPtr<VulkanGpuDevice>& device = GetVulkanGpuBackend().GetVulkanDevice(deviceIdx);
	VulkanDescriptorManager& descManager = device->GetDescriptorManager();

	for(u32 setIndex = 0; setIndex < setCount; setIndex++)
	{
		PerSetData& perSetData = perDeviceData.PerSetData[setIndex];

		if(!mSetsDirty[setIndex]) // Set not dirty, just use the last one we wrote (this is fine even across multiple command buffers)
			continue;

		// Set is dirty, we need to update
		//// Use latest unless already used, otherwise try to find an unused one
		if(perSetData.LastUsedSet->IsBound()) // Checking this is okay, because it's only modified below when we call registerResource, which is under the same lock as this
		{
			perSetData.LastUsedSet = nullptr;

			for(size_t setCacheIndex = 0; setCacheIndex < perSetData.Sets.size(); setCacheIndex++)
			{
				const u32 setIndex = (perSetData.LastFreeSetIndex + setCacheIndex) % (u32)perSetData.Sets.size();
				if(!perSetData.Sets[setIndex]->IsBound())
				{
					perSetData.LastUsedSet = perSetData.Sets[setIndex];
					perSetData.LastFreeSetIndex = setIndex;

					break;
				}
			}

			// Cannot find an empty set, allocate a new one
			if(perSetData.LastUsedSet == nullptr)
			{
				VulkanDescriptorLayout* layout = vkParamInfo.GetLayout(deviceIdx, setIndex);
				perSetData.LastUsedSet = descManager.CreateSet(layout);
				perSetData.Sets.push_back(perSetData.LastUsedSet);
			}
		}

		// Note: Currently I write to the entire set at once, but it might be beneficial to remember only the exact
		// entries that were updated, and only write to them individually.
		perSetData.LastUsedSet->Write(perSetData.WriteSetInfos, perSetData.ElementCount);

		mSetsDirty[setIndex] = false;
	}

	for(u32 i = 0; i < setCount; i++)
	{
		VulkanDescriptorSet* set = perDeviceData.PerSetData[i].LastUsedSet;

		buffer.RegisterResource(set, VulkanAccessFlag::Read);
		outSets[i] = set->GetHandle();
	}
}
