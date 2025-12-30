//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DVulkanGpuParameterSet.h"
#include "B3DVulkanGpuParameterSetPool.h"
#include "B3DVulkanUtility.h"
#include "B3DVulkanGpuDevice.h"
#include "B3DVulkanTexture.h"
#include "B3DVulkanGpuBuffer.h"
#include "B3DVulkanDescriptorSet.h"
#include "B3DVulkanDescriptorLayout.h"
#include "B3DVulkanGpuPipelineParameterLayout.h"
#include "B3DVulkanGpuCommandBuffer.h"
#include "B3DVulkanSamplerState.h"
#include "Managers/B3DVulkanTextureManager.h"
#include "RenderAPI/B3DGpuProgramParameterDescription.h"

using namespace b3d;
using namespace b3d::render;

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
		B3D_LOG(Error, LogRenderBackend, "Unable to bind texture. Image view is not of expected type. Expected {0} but got {1}.", (u32)expectedType, (u32)actualType);
	}

	return isViewValid;
}

VulkanGpuParameterSet::VulkanGpuParameterSet(VulkanGpuDevice& gpuDevice, const SPtr<GpuPipelineParameterSetLayout>& parameterSetLayout,
	u32 set, VulkanGpuParameterSetPool& pool)
	: GpuParameterSet(parameterSetLayout, set), mGpuDevice(gpuDevice)
{
	mOwnerPool = &pool;
}

VulkanGpuParameterSet::~VulkanGpuParameterSet()
{
	Lock lock(mMutex);

	if (mSetInformation.DescriptorSet == nullptr)
		return;

	mSetInformation.DescriptorSet->Destroy();
}

void VulkanGpuParameterSet::Initialize()
{
	VulkanGpuPipelineParameterSetLayout& vulkanGpuPipelineParameterSetLayout = static_cast<VulkanGpuPipelineParameterSetLayout&>(*mParameterSetLayout);

	const u32 uniformBufferCount = vulkanGpuPipelineParameterSetLayout.GetResourceCount(GpuParameterType::UniformBuffer);
	const u32 sampledTextureCount = vulkanGpuPipelineParameterSetLayout.GetResourceCount(GpuParameterType::SampledTexture);
	const u32 storageTextureCount = vulkanGpuPipelineParameterSetLayout.GetResourceCount(GpuParameterType::StorageTexture);
	const u32 bufferCount = vulkanGpuPipelineParameterSetLayout.GetResourceCount(GpuParameterType::StorageBuffer);
	const u32 samplerCount = vulkanGpuPipelineParameterSetLayout.GetResourceCount(GpuParameterType::Sampler);
	const u32 resourceCount = vulkanGpuPipelineParameterSetLayout.GetResourceCount();
	const u32 bindingCount = vulkanGpuPipelineParameterSetLayout.GetBindingCount();

	// Note: I'm assuming a single WriteInfo per binding, but if arrays sizes larger than 1 are eventually supported
	// I'll need to adjust the code.
	mAllocator.Reserve<VkWriteDescriptorSet>(bindingCount)
		.Reserve<VkDescriptorImageInfo>(resourceCount)
		.Reserve<VkDescriptorBufferInfo>(resourceCount)
		.Reserve<VkBufferView>(resourceCount)
		.Reserve<VkImage>(sampledTextureCount)
		.Reserve<VkImage>(storageTextureCount)
		.Reserve<VkBuffer>(uniformBufferCount)
		.Reserve<VkBuffer>(bufferCount)
		.Reserve<VkSampler>(samplerCount)
		.Initialize();

	Lock lock(mMutex); // Set write operations need to be thread safe

	SPtr<VulkanSamplerState> defaultSampler = std::static_pointer_cast<VulkanSamplerState>(mGpuDevice.FindOrCreateSamplerState(SamplerStateCreateInformation()));
	VulkanTextureManager& vkTexManager = static_cast<VulkanTextureManager&>(TextureManager::Instance());
	const VulkanBuiltinResources& builtinResources = (mGpuDevice.GetBuiltinResources());

	mSampledImages = mAllocator.Allocate<VkImage>(sampledTextureCount, true);
	mStorageImages = mAllocator.Allocate<VkImage>(storageTextureCount, true);
	mUniformBuffers = mAllocator.Allocate<VkBuffer>(uniformBufferCount, true);
	mBuffers = mAllocator.Allocate<VkBuffer>(bufferCount, true);
	mSamplers = mAllocator.Allocate<VkSampler>(samplerCount, true);

	VulkanDescriptorManager& descManager = mGpuDevice.GetDescriptorManager();
	VulkanSampler* vkDefaultSampler = defaultSampler->GetVulkanResource();

	mSetInformation.WriteSetInfos = mAllocator.Allocate<VkWriteDescriptorSet>(bindingCount);
	mSetInformation.ImageWriteInfos = mAllocator.Allocate<VkDescriptorImageInfo>(resourceCount);
	mSetInformation.BufferWriteInfos = mAllocator.Allocate<VkDescriptorBufferInfo>(resourceCount);
	mSetInformation.BufferViews = mAllocator.Allocate<VkBufferView>(resourceCount);

	VulkanDescriptorLayout* layout = vulkanGpuPipelineParameterSetLayout.GetLayout();
	mSetInformation.ElementCount = bindingCount;

	VulkanGpuParameterSetPool* vulkanPool = static_cast<VulkanGpuParameterSetPool*>(mOwnerPool);
	mSetInformation.DescriptorSet = vulkanPool->AllocateDescriptorSet(layout->GetVulkanHandle());
	if (mSetInformation.DescriptorSet == nullptr)
	{
		B3D_LOG(Error, LogRenderBackend, "Failed to allocate descriptor set from pool.");
		return;
	}

	const TArrayView<const VkDescriptorSetLayoutBinding> perSetBindings = vulkanGpuPipelineParameterSetLayout.GetBindings();
	const TArrayView<const GpuParameterObjectType> types = vulkanGpuPipelineParameterSetLayout.GetTypes();
	const TArrayView<const GpuBufferFormat> elementTypes = vulkanGpuPipelineParameterSetLayout.GetElementTypes();
	for(u32 layoutBindingIndex = 0; layoutBindingIndex < bindingCount; layoutBindingIndex++)
	{
		// Note: Instead of using one structure per binding, it's possible to update multiple at once
		// by specifying larger descriptorCount, if they all share type and shader stages.
		VkWriteDescriptorSet& writeSetInfo = mSetInformation.WriteSetInfos[layoutBindingIndex];
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
			const u32 usedResourceSequentialIndex = vulkanGpuPipelineParameterSetLayout.GetUsedResourceSequentialIndex(slot, 0);

			VkDescriptorImageInfo* imageInfos = &mSetInformation.ImageWriteInfos[usedResourceSequentialIndex];
			for(u32 arrayIndex = 0; arrayIndex < writeSetInfo.descriptorCount; arrayIndex++)
			{
				imageInfos[arrayIndex].sampler = vkDefaultSampler->GetVulkanHandle();
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
				const VulkanImage *const imageResource = vkTexManager.GetDummyTexture(types[layoutBindingIndex])->GetVulkanResource();
				const VkFormat format = VulkanTextureManager::GetDummyViewFormat(elementTypes[layoutBindingIndex]);

				const bool isLoadStore = writeSetInfo.descriptorType == VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
				const u32 usedResourceSequentialIndex = vulkanGpuPipelineParameterSetLayout.GetUsedResourceSequentialIndex(slot, 0);
				const u32 sequentialResourceIndex = vulkanGpuPipelineParameterSetLayout.GetSequentialResourceIndex(slot, 0);

				VkDescriptorImageInfo* imageInfos = &mSetInformation.ImageWriteInfos[usedResourceSequentialIndex];
				for(u32 arrayIndex = 0; arrayIndex < writeSetInfo.descriptorCount; arrayIndex++)
				{
					if(isLoadStore)
					{
						imageInfos[arrayIndex].sampler = VK_NULL_HANDLE;
						imageInfos[arrayIndex].imageView = imageResource->GetView(format, false).Handle;
						imageInfos[arrayIndex].imageLayout = VK_IMAGE_LAYOUT_GENERAL;

						mStorageImages[sequentialResourceIndex + arrayIndex] = imageResource->GetVulkanHandle();
					}
					else
					{
						const bool isCombinedImageSampler = writeSetInfo.descriptorType == VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
						if(isCombinedImageSampler)
							imageInfos[arrayIndex].sampler = vkDefaultSampler->GetVulkanHandle();
						else
							imageInfos[arrayIndex].sampler = VK_NULL_HANDLE;

						imageInfos[arrayIndex].imageView = imageResource->GetView(format, false).Handle;
						imageInfos[arrayIndex].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

						mSampledImages[sequentialResourceIndex + arrayIndex] = imageResource->GetVulkanHandle();
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
					const bool isUniformBuffer =
						writeSetInfo.descriptorType == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER ||
						writeSetInfo.descriptorType == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;

					const u32 usedResourceSequentialIndex = vulkanGpuPipelineParameterSetLayout.GetUsedResourceSequentialIndex(slot, 0);
					const u32 sequentialResourceIndex = vulkanGpuPipelineParameterSetLayout.GetSequentialResourceIndex(slot, 0);

					VkDescriptorBufferInfo* bufferInfos = &mSetInformation.BufferWriteInfos[usedResourceSequentialIndex];
					for(u32 arrayIndex = 0; arrayIndex < writeSetInfo.descriptorCount; arrayIndex++)
					{
						bufferInfos[arrayIndex].offset = 0;
						bufferInfos[arrayIndex].range = VK_WHOLE_SIZE;

						if(isUniformBuffer)
						{
							VulkanGpuBuffer* const buffer = builtinResources.DummyUniformBuffer.get();
							bufferInfos[arrayIndex].buffer = buffer->GetVulkanResource()->GetVulkanHandle();

							mUniformBuffers[sequentialResourceIndex + arrayIndex] = bufferInfos[arrayIndex].buffer;
						}
						else
						{
							VulkanGpuBuffer* const buffer = builtinResources.DummyUniformBuffer.get();
							bufferInfos[arrayIndex].buffer = buffer->GetVulkanResource()->GetVulkanHandle();

							mBuffers[sequentialResourceIndex + arrayIndex] = bufferInfos[arrayIndex].buffer;
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
						buffer = builtinResources.DummyStorageBuffer->GetVulkanResource();
					else
						buffer = builtinResources.DummyReadBuffer->GetVulkanResource();

					const VkFormat format = VulkanUtility::GetBufferFormat(elementTypes[layoutBindingIndex]);
					const VkBufferView bufferView = buffer->GetOrCreateView(format);

					const u32 usedResourceSequentialIndex = vulkanGpuPipelineParameterSetLayout.GetUsedResourceSequentialIndex(slot, 0);
					const u32 sequentialResourceIndex = vulkanGpuPipelineParameterSetLayout.GetSequentialResourceIndex(slot, 0);

					VkBufferView* const bufferViews = &mSetInformation.BufferViews[usedResourceSequentialIndex];
					for(u32 arrayIndex = 0; arrayIndex < writeSetInfo.descriptorCount; arrayIndex++)
					{
						bufferViews[arrayIndex] = bufferView;

						mBuffers[sequentialResourceIndex + arrayIndex] = buffer->GetVulkanHandle();
					}

					writeSetInfo.pBufferInfo = nullptr;
					writeSetInfo.pTexelBufferView = bufferViews;
				}

				writeSetInfo.pImageInfo = nullptr;
			}
		}
	}

	GpuParameterSet::Initialize();
}

bool VulkanGpuParameterSet::SetUniformBuffer(u32 slot,const SPtr<GpuBuffer>& uniformBuffer, u32 arrayIndex, u32 offset)
{
	if (!GpuParameterSet::SetUniformBuffer(slot, uniformBuffer, arrayIndex, offset))
		return false;

	VulkanGpuPipelineParameterSetLayout& pipelineParameterInformationSet = static_cast<VulkanGpuPipelineParameterSetLayout&>(*mParameterSetLayout);
	const u32 usedResourceSequentialIndex = pipelineParameterInformationSet.GetUsedResourceSequentialIndex(slot, arrayIndex);
	if(usedResourceSequentialIndex == ~0u)
	{
		B3D_LOG(Error, LogRenderBackend, "Provided set/slot combination is not used by the GPU program: {0},{1}.", mSet, slot);
		return false;
	}

	const u32 sequentialResourceIndex = pipelineParameterInformationSet.GetSequentialResourceIndex(slot, arrayIndex);

	Lock lock(mMutex);

	auto* vulkanUniformBuffer = static_cast<VulkanGpuBuffer*>(uniformBuffer.get());

	VulkanBuffer* vulkanBuffer;
	VkDeviceSize bufferSize;
	if(vulkanUniformBuffer != nullptr)
	{
		vulkanBuffer = vulkanUniformBuffer->GetVulkanResource();
		bufferSize = vulkanUniformBuffer->GetSuballocationSize();
	}
	else
	{
		vulkanBuffer = nullptr;
		bufferSize = VK_WHOLE_SIZE;
	}

	VkBuffer vkBuffer = VK_NULL_HANDLE;
	if(vulkanBuffer != nullptr)
	{
		vkBuffer = vulkanBuffer->GetVulkanHandle();
	}
	else
	{
		const VulkanBuiltinResources& builtinResources = (mGpuDevice.GetBuiltinResources());
		vkBuffer = builtinResources.DummyUniformBuffer->GetVulkanResource()->GetVulkanHandle();
	}

	if(vkBuffer != mUniformBuffers[sequentialResourceIndex])
	{
		mSetInformation.BufferWriteInfos[usedResourceSequentialIndex].buffer = vkBuffer;
		mSetInformation.BufferWriteInfos[usedResourceSequentialIndex].range = bufferSize;
		mUniformBuffers[sequentialResourceIndex] = vkBuffer;

		mSetDirty = true;
	}

	return true;
}

bool VulkanGpuParameterSet::SetSampledTexture(u32 slot, const SPtr<Texture>& texture, const TextureSurface& surface, u32 arrayIndex)
{
	if (!GpuParameterSet::SetSampledTexture(slot, texture, surface, arrayIndex))
		return false;

	VulkanGpuPipelineParameterSetLayout& pipelineParameterInformationSet = static_cast<VulkanGpuPipelineParameterSetLayout&>(*mParameterSetLayout);

	const u32 usedBindingSequentialIndex = pipelineParameterInformationSet.GetUsedBindingSequentialIndex(slot);
	const u32 usedResourceSequentialIndex = pipelineParameterInformationSet.GetUsedResourceSequentialIndex(slot, arrayIndex);

	if(usedResourceSequentialIndex == ~0u || usedBindingSequentialIndex == ~0u)
	{
		B3D_LOG(Error, LogRenderBackend, "Provided set/slot combination is not used by the GPU program: {0},{1}.", mSet, slot);
		return false;
	}

	const u32 sequentialResourceIndex = pipelineParameterInformationSet.GetSequentialResourceIndex(slot, arrayIndex);

	Lock lock(mMutex);

	VulkanTexture* vulkanTexture = static_cast<VulkanTexture*>(texture.get());

	VulkanImage* vulkanImage;
	if(vulkanTexture != nullptr)
		vulkanImage = vulkanTexture->GetVulkanResource();
	else
		vulkanImage = nullptr;

	const TArrayView<const GpuParameterObjectType> types = pipelineParameterInformationSet.GetTypes();
	const GpuParameterObjectType objectType = types[usedBindingSequentialIndex];

	VulkanImageView imageView;
	VkImage vkImage = VK_NULL_HANDLE;
	if(vulkanImage != nullptr)
	{
		imageView = vulkanImage->GetView(surface, false);
		vkImage = vulkanImage->GetVulkanHandle();

		if(!EnsureImageViewValidForShader(imageView, objectType))
			vulkanImage = nullptr;
	}

	if(vulkanImage == nullptr)
	{
		auto& vkTexManager = static_cast<VulkanTextureManager&>(TextureManager::Instance());

		const TArrayView<const GpuBufferFormat> elementTypes = pipelineParameterInformationSet.GetElementTypes();

		vulkanImage = vkTexManager.GetDummyTexture(types[usedBindingSequentialIndex])->GetVulkanResource();
		VkFormat format = VulkanTextureManager::GetDummyViewFormat(elementTypes[usedBindingSequentialIndex]);

		imageView = vulkanImage->GetView(format, false);
		vkImage = vulkanImage->GetVulkanHandle();
	}

	if(vkImage != mSampledImages[sequentialResourceIndex] || imageView.Handle != mSetInformation.ImageWriteInfos[usedResourceSequentialIndex].imageView)
	{
		mSetInformation.ImageWriteInfos[usedResourceSequentialIndex].imageView = imageView.Handle;
		mSampledImages[sequentialResourceIndex] = vkImage;

		mSetDirty = true;
	}

	return true;
}

bool VulkanGpuParameterSet::SetStorageTexture(u32 slot, const SPtr<Texture>& texture, const TextureSurface& surface, u32 arrayIndex)
{
	if (!GpuParameterSet::SetStorageTexture(slot, texture, surface, arrayIndex))
		return false;

	VulkanGpuPipelineParameterSetLayout& pipelineParameterInformationSet = static_cast<VulkanGpuPipelineParameterSetLayout&>(*mParameterSetLayout);
	const u32 usedBindingSequentialIndex = pipelineParameterInformationSet.GetUsedBindingSequentialIndex(slot);
	const u32 usedResourceSequentialIndex = pipelineParameterInformationSet.GetUsedResourceSequentialIndex(slot, arrayIndex);

	if(usedBindingSequentialIndex == ~0u || usedResourceSequentialIndex == ~0u)
	{
		B3D_LOG(Error, LogRenderBackend, "Provided set/slot combination is not used by the GPU program: {0},{1}.", mSet, slot);
		return false;
	}

	const u32 sequentialResourceIndex = pipelineParameterInformationSet.GetSequentialResourceIndex(slot, arrayIndex);

	Lock lock(mMutex);

	VulkanTexture* vulkanTexture = static_cast<VulkanTexture*>(texture.get());

	VulkanImage* vulkanImage;
	if(vulkanTexture != nullptr)
		vulkanImage = vulkanTexture->GetVulkanResource();
	else
		vulkanImage = nullptr;

	const TArrayView<const GpuParameterObjectType> types = pipelineParameterInformationSet.GetTypes();
	const GpuParameterObjectType objectType = types[usedBindingSequentialIndex];

	VulkanImageView imageView;
	VkImage vkImage = VK_NULL_HANDLE;
	if(vulkanImage != nullptr)
	{
		imageView = vulkanImage->GetView(surface, false);
		vkImage = vulkanImage->GetVulkanHandle();

		if(!EnsureImageViewValidForShader(imageView, objectType))
			vulkanImage = nullptr;
	}

	if(vulkanImage == nullptr)
	{
		auto& vkTexManager = static_cast<VulkanTextureManager&>(TextureManager::Instance());

		const TArrayView<const GpuBufferFormat> elementTypes = pipelineParameterInformationSet.GetElementTypes();

		vulkanImage = vkTexManager.GetDummyTexture(types[usedBindingSequentialIndex])->GetVulkanResource();
		VkFormat format = VulkanTextureManager::GetDummyViewFormat(elementTypes[usedBindingSequentialIndex]);

		imageView = vulkanImage->GetView(format, false);
		vkImage = vulkanImage->GetVulkanHandle();
	}

	if(vkImage != mStorageImages[sequentialResourceIndex] || imageView.Handle != mSetInformation.ImageWriteInfos[usedResourceSequentialIndex].imageView)
	{
		mSetInformation.ImageWriteInfos[usedResourceSequentialIndex].imageView = imageView.Handle;
		mStorageImages[sequentialResourceIndex] = vkImage;

		mSetDirty = true;
	}

	return true;
}

bool VulkanGpuParameterSet::SetStorageBuffer(u32 slot, const SPtr<GpuBuffer>& buffer, u32 arrayIndex, GpuBufferViewInformation view)
{
	if (!GpuParameterSet::SetStorageBuffer(slot, buffer, arrayIndex, view))
		return false;

	VulkanGpuPipelineParameterSetLayout& pipelineParameterInformationSet = static_cast<VulkanGpuPipelineParameterSetLayout&>(*mParameterSetLayout);
	const u32 usedBindingSequentialIndex = pipelineParameterInformationSet.GetUsedBindingSequentialIndex(slot);
	const u32 usedResourceSequentialIndex = pipelineParameterInformationSet.GetUsedResourceSequentialIndex(slot, arrayIndex);

	if(usedBindingSequentialIndex == ~0u || usedResourceSequentialIndex == ~0u)
	{
		B3D_LOG(Error, LogRenderBackend, "Provided set/slot combination is not used by the GPU program: {0},{1}.", mSet, slot);
		return false;
	}

	const u32 sequentialResourceIndex = pipelineParameterInformationSet.GetSequentialResourceIndex(slot, arrayIndex);

	Lock lock(mMutex);

	VulkanGpuBuffer* vulkanBuffer = static_cast<VulkanGpuBuffer*>(buffer.get());

	VulkanBuffer* bufferResource = nullptr;
	u32 bufferSize = 0;
	u32 bufferOffset = 0;
	if(vulkanBuffer != nullptr)
	{
		bufferResource = vulkanBuffer->GetVulkanResource();
		bufferOffset = view.Offset;
		bufferSize = view.Range == 0 ? vulkanBuffer->GetSuballocationSize() : view.Range;
	}

	VkWriteDescriptorSet& writeSetInfo = mSetInformation.WriteSetInfos[usedBindingSequentialIndex];

	const bool useView = writeSetInfo.descriptorType != VK_DESCRIPTOR_TYPE_STORAGE_BUFFER && writeSetInfo.descriptorType != VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC;
	VkBufferView vkBufferView = VK_NULL_HANDLE;
	if(bufferResource == nullptr)
	{
		const VulkanBuiltinResources& builtinResources = (mGpuDevice.GetBuiltinResources());
		if(useView)
		{
			const bool isLoadStore = writeSetInfo.descriptorType == VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER;
			if(isLoadStore)
				bufferResource = builtinResources.DummyStorageBuffer->GetVulkanResource();
			else
				bufferResource = builtinResources.DummyReadBuffer->GetVulkanResource();

			const TArrayView<const GpuBufferFormat> elementTypes = pipelineParameterInformationSet.GetElementTypes();
			const VkFormat format = VulkanUtility::GetBufferFormat(elementTypes[usedBindingSequentialIndex]);
			vkBufferView = bufferResource->GetOrCreateView(format);
		}
		else
		{
			bufferResource = builtinResources.DummyStructuredBuffer->GetVulkanResource();
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

	const VkBuffer vkBuffer = bufferResource->GetVulkanHandle();

	const u32 oldBufferRange = mStorageBufferData->View.Range == 0 ? (mStorageBufferData->Buffer != nullptr ? mStorageBufferData->Buffer->GetSuballocationSize() : 0) : mStorageBufferData->View.Range;

	const bool isBufferViewChanged = (useView && mSetInformation.BufferViews[usedResourceSequentialIndex] != vkBufferView) || mStorageBufferData->View.Offset != bufferOffset || oldBufferRange != bufferSize;
	if(mBuffers[sequentialResourceIndex] != vkBuffer || isBufferViewChanged)
	{
		if(useView)
		{
			mSetInformation.BufferViews[usedResourceSequentialIndex] = vkBufferView;
			writeSetInfo.pTexelBufferView = &mSetInformation.BufferViews[usedResourceSequentialIndex];
		}
		else // Structured storage buffer
		{
			mSetInformation.BufferWriteInfos[usedResourceSequentialIndex].buffer = vkBuffer;
			mSetInformation.BufferWriteInfos[usedResourceSequentialIndex].offset = bufferOffset;
			mSetInformation.BufferWriteInfos[usedResourceSequentialIndex].range = bufferSize == 0 ? VK_WHOLE_SIZE : bufferSize;
			mBuffers[sequentialResourceIndex] = vkBuffer;

			writeSetInfo.pTexelBufferView = nullptr;
		}

		mBuffers[sequentialResourceIndex] = vkBuffer;
		mSetDirty = true;
	}

	return true;
}

bool VulkanGpuParameterSet::SetSamplerState(u32 slot, const SPtr<SamplerState>& sampler, u32 arrayIndex)
{
	if (!GpuParameterSet::SetSamplerState(slot, sampler, arrayIndex))
		return false;

	VulkanGpuPipelineParameterSetLayout& pipelineParameterInformationSet = static_cast<VulkanGpuPipelineParameterSetLayout&>(*mParameterSetLayout);
	const u32 usedResourceSequentialIndex = pipelineParameterInformationSet.GetUsedResourceSequentialIndex(slot, arrayIndex);
	if(usedResourceSequentialIndex == ~0u)
	{
		B3D_LOG(Error, LogRenderBackend, "Provided set/slot combination is not used by the GPU program: {0},{1}.", mSet, slot);
		return false;
	}

	const u32 sequentialResourceIndex = pipelineParameterInformationSet.GetSequentialResourceIndex(slot, arrayIndex);

	Lock lock(mMutex);

	VulkanSamplerState* vulkanSampler = static_cast<VulkanSamplerState*>(sampler.get());

	SPtr<VulkanSamplerState> defaultSampler;
	VulkanSampler* samplerRes;
	if(vulkanSampler != nullptr)
		samplerRes = vulkanSampler->GetVulkanResource();
	else
	{
		defaultSampler = std::static_pointer_cast<VulkanSamplerState>(mGpuDevice.FindOrCreateSamplerState(SamplerStateCreateInformation()));
		samplerRes = defaultSampler->GetVulkanResource();
	}

	VkSampler vkSampler = samplerRes->GetVulkanHandle();
	if(mSamplers[sequentialResourceIndex] != vkSampler)
	{
		mSetInformation.ImageWriteInfos[usedResourceSequentialIndex].sampler = vkSampler;
		mSamplers[sequentialResourceIndex] = vkSampler;

		mSetDirty = true;
	}

	return true;
}

void VulkanGpuParameterSet::PrepareForBind(VulkanGpuCommandBuffer& commandBuffer, VulkanResourceTracker& resourceTracker, VulkanBarrierHelper& barrierHelper, VkDescriptorSet& outSet, TInlineArray<u32, 4>& outDynamicOffsets)
{
	VulkanGpuPipelineParameterSetLayout& pipelineParameterInformationSet = static_cast<VulkanGpuPipelineParameterSetLayout&>(*mParameterSetLayout);

	const u32 uniformBufferBindingCount = pipelineParameterInformationSet.GetBindingCount(GpuParameterType::UniformBuffer);
	const u32 sampledTextureBindingCount = pipelineParameterInformationSet.GetBindingCount(GpuParameterType::SampledTexture);
	const u32 storageTextureBindingCount = pipelineParameterInformationSet.GetBindingCount(GpuParameterType::StorageTexture);
	const u32 storageBufferBindingCount = pipelineParameterInformationSet.GetBindingCount(GpuParameterType::StorageBuffer);
	const u32 samplerBindingCount = pipelineParameterInformationSet.GetBindingCount(GpuParameterType::Sampler);

	FrameAllocatorScope frameScope;
	FrameVector<u32> dynamicOffsetMapping(mSetInformation.ElementCount, ~0u);

	Lock lock(mMutex);

	// Registers resources with the command buffer, and check if internal resource handled changed (in which case set
	// needs updating - this can happen due to resource writes, as internally system might find it more performant
	// to discard used resources and create new ones).
	// Note: Makes the assumption that this object (and all of the resources it holds) are externally locked, and will
	// not be modified on another thread while being bound.
	for(u32 sequentialBindingIndex = 0; sequentialBindingIndex < uniformBufferBindingCount; sequentialBindingIndex++)
	{
		const u32 arraySize = pipelineParameterInformationSet.GetArraySize(GpuParameterType::UniformBuffer, sequentialBindingIndex);
		const u32 slot = pipelineParameterInformationSet.GetSlot(GpuParameterType::UniformBuffer, sequentialBindingIndex);

		for(u32 arrayIndex = 0; arrayIndex < arraySize; arrayIndex++)
		{
			const u32 sequentialResourceIndex = pipelineParameterInformationSet.GetSequentialResourceIndex(slot, arrayIndex);
			const u32 usedBindingSequentialIndex = pipelineParameterInformationSet.GetUsedBindingSequentialIndex(slot);
			const u32 usedResourceSequentialIndex = pipelineParameterInformationSet.GetUsedResourceSequentialIndex(slot, arrayIndex);

			VulkanBuffer* resource = nullptr;
			VkDeviceSize bufferSize = VK_WHOLE_SIZE;
			u32 dynamicOffset = 0;

			if(mUniformBufferData[sequentialResourceIndex].Buffer != nullptr)
			{
				VulkanGpuBuffer *const element = static_cast<VulkanGpuBuffer*>(mUniformBufferData[sequentialResourceIndex].Buffer.get());
				resource = element->GetVulkanResource();
				bufferSize = element->GetSuballocationSize();
				dynamicOffset = mUniformBufferData[sequentialResourceIndex].Offset;
			}

			if(resource == nullptr)
			{
				const VulkanBuiltinResources& builtinResources = mGpuDevice.GetBuiltinResources();
				resource = builtinResources.DummyUniformBuffer->GetVulkanResource();

				if(resource == nullptr)
					continue;
			}

			TArrayView<const VkDescriptorSetLayoutBinding> perSetBindings = pipelineParameterInformationSet.GetBindings();
			GpuResourceUseFlags useFlags = VulkanUtility::ShaderToResourceUseFlags(perSetBindings[usedBindingSequentialIndex].stageFlags) | GpuResourceUseFlag::UniformBuffer;

			// Register with command buffer
			resourceTracker.TrackBufferUsage(resource, useFlags, GpuAccessFlag::Read, barrierHelper, dynamicOffset);

			// Check if internal resource changed from what was previously bound in the descriptor set
			B3D_ASSERT(mUniformBuffers[sequentialResourceIndex] != VK_NULL_HANDLE);

			VkBuffer vkBuffer = resource->GetVulkanHandle();
			if(mUniformBuffers[sequentialResourceIndex] != vkBuffer)
			{
				mUniformBuffers[sequentialResourceIndex] = vkBuffer;
				mSetInformation.BufferWriteInfos[usedResourceSequentialIndex].buffer = vkBuffer;
				mSetInformation.BufferWriteInfos[usedResourceSequentialIndex].range = bufferSize;

				mSetDirty = true;
			}

			dynamicOffsetMapping[usedBindingSequentialIndex] = dynamicOffset;
		}
	}

	for(u32 sequentialBindingIndex = 0; sequentialBindingIndex < storageBufferBindingCount; sequentialBindingIndex++)
	{
		const u32 arraySize = pipelineParameterInformationSet.GetArraySize(GpuParameterType::StorageBuffer, sequentialBindingIndex);
		const u32 slot = pipelineParameterInformationSet.GetSlot(GpuParameterType::StorageBuffer, sequentialBindingIndex);

		for(u32 arrayIndex = 0; arrayIndex < arraySize; ++arrayIndex)
		{
			const u32 sequentialResourceIndex = pipelineParameterInformationSet.GetSequentialResourceIndex(slot, arrayIndex);
			const u32 usedBindingSequentialIndex = pipelineParameterInformationSet.GetUsedBindingSequentialIndex(slot);
			const u32 usedResourceSequentialIndex = pipelineParameterInformationSet.GetUsedResourceSequentialIndex(slot, arrayIndex);

			TArrayView<const GpuParameterObjectType> types = pipelineParameterInformationSet.GetTypes();
			GpuParameterObjectType type = types[usedBindingSequentialIndex];

			GpuAccessFlags accessFlags = GpuAccessFlag::Read;
			VulkanBuffer* resource = nullptr;
			VkDeviceSize bufferSize = VK_WHOLE_SIZE;

			const bool supportsDynamicOffset = type == GPOT_STRUCTURED_BUFFER || type == GPOT_RWSTRUCTURED_BUFFER;
			u32 dynamicOffset = supportsDynamicOffset ? 0 : ~0u;

			if(mStorageBufferData[sequentialResourceIndex].Buffer != nullptr)
			{
				if(supportsDynamicOffset)
					dynamicOffset = mStorageBufferData[sequentialResourceIndex].View.Offset;

				auto* element = static_cast<VulkanGpuBuffer*>(mStorageBufferData[sequentialResourceIndex].Buffer.get());
				resource = element->GetVulkanResource();

				bufferSize = element->GetSuballocationSize();
			}

			if(resource == nullptr)
			{
				const VulkanBuiltinResources& builtinResources = mGpuDevice.GetBuiltinResources();

				switch(type)
				{
				case GPOT_BYTE_BUFFER:
					resource = builtinResources.DummyReadBuffer->GetVulkanResource();
					break;
				case GPOT_RWBYTE_BUFFER:
					resource = builtinResources.DummyStorageBuffer->GetVulkanResource();
					break;
				case GPOT_STRUCTURED_BUFFER:
				case GPOT_RWSTRUCTURED_BUFFER:
					resource = builtinResources.DummyStructuredBuffer->GetVulkanResource();
					break;
				default:
					break;
				}

				if(resource == nullptr)
					continue;
			}

			if(type == GPOT_RWBYTE_BUFFER || type == GPOT_RWSTRUCTURED_BUFFER)
				accessFlags |= GpuAccessFlag::Write;

			// Register with command buffer
			TArrayView<const VkDescriptorSetLayoutBinding> perSetBindings = pipelineParameterInformationSet.GetBindings();
			GpuResourceUseFlags useFlags = VulkanUtility::ShaderToResourceUseFlags(perSetBindings[usedBindingSequentialIndex].stageFlags) | GpuResourceUseFlag::ShaderAccess;

			resourceTracker.TrackBufferUsage(resource, useFlags, accessFlags, barrierHelper, dynamicOffset);

			// Check if internal resource changed from what was previously bound in the descriptor set
			B3D_ASSERT(mBuffers[sequentialResourceIndex] != VK_NULL_HANDLE);

			VkBuffer vkBuffer = resource->GetVulkanHandle();
			if(mBuffers[sequentialResourceIndex] != vkBuffer)
			{
				mBuffers[sequentialResourceIndex] = vkBuffer;

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
						TArrayView<const GpuBufferFormat> elementTypes = pipelineParameterInformationSet.GetElementTypes();
						view = resource->GetOrCreateView(VulkanUtility::GetBufferFormat(elementTypes[usedBindingSequentialIndex]));
					}
				}

				if(view)
				{
					const u32 usedResourceSequentialFirstArrayEntryIndex = pipelineParameterInformationSet.GetUsedResourceSequentialIndex(slot, arrayIndex);

					mSetInformation.BufferViews[usedResourceSequentialIndex] = view;
					mSetInformation.WriteSetInfos[usedBindingSequentialIndex].pTexelBufferView = &mSetInformation.BufferViews[usedResourceSequentialFirstArrayEntryIndex];
				}
				else // Structured storage buffer
				{
					mSetInformation.BufferWriteInfos[usedResourceSequentialIndex].buffer = vkBuffer;
					mSetInformation.BufferWriteInfos[usedResourceSequentialIndex].range = bufferSize;
					mSetInformation.WriteSetInfos[usedBindingSequentialIndex].pTexelBufferView = nullptr;
				}

				mSetDirty = true;
			}

			dynamicOffsetMapping[usedBindingSequentialIndex] = dynamicOffset;
		}
	}

	for(u32 sequentialBindingIndex = 0; sequentialBindingIndex < samplerBindingCount; sequentialBindingIndex++)
	{
		const u32 arraySize = pipelineParameterInformationSet.GetArraySize(GpuParameterType::Sampler, sequentialBindingIndex);
		const u32 slot = pipelineParameterInformationSet.GetSlot(GpuParameterType::Sampler, sequentialBindingIndex);

		for(u32 arrayIndex = 0; arrayIndex < arraySize; arrayIndex++)
		{
			const u32 sequentialResourceIndex = pipelineParameterInformationSet.GetSequentialResourceIndex(slot, arrayIndex);

			if(mSamplerStates[sequentialResourceIndex] == nullptr)
				continue;

			VulkanSamplerState* element = static_cast<VulkanSamplerState*>(mSamplerStates[sequentialResourceIndex].get());
			VulkanSampler* resource = element->GetVulkanResource();
			if(resource == nullptr)
				continue;

			// Register with command buffer
			resourceTracker.TrackResourceUsage(resource, GpuAccessFlag::Read);

			// Check if internal resource changed from what was previously bound in the descriptor set
			B3D_ASSERT(mSamplers[sequentialResourceIndex] != VK_NULL_HANDLE);

			VkSampler vkSampler = resource->GetVulkanHandle();
			if(mSamplers[sequentialResourceIndex] != vkSampler)
			{
				mSamplers[sequentialResourceIndex] = vkSampler;

				const u32 usedResourceSequentialIndex = pipelineParameterInformationSet.GetUsedResourceSequentialIndex(slot, arrayIndex);
				mSetInformation.ImageWriteInfos[usedResourceSequentialIndex].sampler = vkSampler;

				mSetDirty = true;
			}
		}
	}

	for(u32 sequentialBindingIndex = 0; sequentialBindingIndex < storageTextureBindingCount; sequentialBindingIndex++)
	{
		const u32 arraySize = pipelineParameterInformationSet.GetArraySize(GpuParameterType::StorageTexture, sequentialBindingIndex);
		const u32 slot = pipelineParameterInformationSet.GetSlot(GpuParameterType::StorageTexture, sequentialBindingIndex);

		const u32 usedBindingSequentialIndex = pipelineParameterInformationSet.GetUsedBindingSequentialIndex(slot);
		for(u32 arrayIndex = 0; arrayIndex < arraySize; arrayIndex++)
		{
			const u32 sequentialResourceIndex = pipelineParameterInformationSet.GetSequentialResourceIndex(slot, arrayIndex);

			VulkanImage* vulkanImage = nullptr;
			if(mStorageTextureData[sequentialResourceIndex].Texture != nullptr)
			{
				auto* element = static_cast<VulkanTexture*>(mStorageTextureData[sequentialResourceIndex].Texture.get());
				vulkanImage = element->GetVulkanResource();
			}

			const TextureSurface& surface = mStorageTextureData[sequentialResourceIndex].Surface;
			const TArrayView<const GpuParameterObjectType> types = pipelineParameterInformationSet.GetTypes();
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

				vulkanImage = vkTexManager.GetDummyTexture(objectType)->GetVulkanResource();

				if(vulkanImage == nullptr)
					continue;

				const TArrayView<const GpuBufferFormat> elementTypes = pipelineParameterInformationSet.GetElementTypes();
				const VkFormat format = VulkanTextureManager::GetDummyViewFormat(elementTypes[usedBindingSequentialIndex]);

				imageView = vulkanImage->GetView(format, false);
			}

			// Register with command buffer
			const TArrayView<const VkDescriptorSetLayoutBinding> perSetBindings = pipelineParameterInformationSet.GetBindings();

			const GpuResourceUseFlags useFlags = VulkanUtility::ShaderToResourceUseFlags(perSetBindings[usedBindingSequentialIndex].stageFlags) | GpuResourceUseFlag::ShaderAccess;
			const VkImageSubresourceRange range = vulkanImage->GetRange(surface);

			resourceTracker.TrackImageUsage(vulkanImage, range, VK_IMAGE_LAYOUT_GENERAL, VK_IMAGE_LAYOUT_GENERAL, useFlags, GpuAccessFlag::Read | GpuAccessFlag::Write, barrierHelper);

			// Check if internal resource changed from what was previously bound in the descriptor set
			B3D_ASSERT(mStorageImages[sequentialResourceIndex] != VK_NULL_HANDLE);

			VkImage vkImage = vulkanImage->GetVulkanHandle();
			if(mStorageImages[sequentialResourceIndex] != vkImage)
			{
				mStorageImages[sequentialResourceIndex] = vkImage;

				const u32 usedResourceSequentialIndex = pipelineParameterInformationSet.GetUsedResourceSequentialIndex(slot, arrayIndex);
				mSetInformation.ImageWriteInfos[usedResourceSequentialIndex].imageView = imageView.Handle;

				mSetDirty = true;
			}
		}
	}

	for(u32 sequentialBindingIndex = 0; sequentialBindingIndex < sampledTextureBindingCount; sequentialBindingIndex++)
	{
		const u32 arraySize = pipelineParameterInformationSet.GetArraySize(GpuParameterType::SampledTexture, sequentialBindingIndex);
		const u32 slot = pipelineParameterInformationSet.GetSlot(GpuParameterType::SampledTexture, sequentialBindingIndex);

		const u32 usedBindingSequentialIndex = pipelineParameterInformationSet.GetUsedBindingSequentialIndex(slot);

		for(u32 arrayIndex = 0; arrayIndex < arraySize; arrayIndex++)
		{
			const u32 sequentialResourceIndex = pipelineParameterInformationSet.GetSequentialResourceIndex(slot, arrayIndex);
			const u32 usedResourceSequentialIndex = pipelineParameterInformationSet.GetUsedResourceSequentialIndex(slot, arrayIndex);

			VulkanImage* vulkanImage = nullptr;
			VkImageLayout layout;
			if(mSampledTextureData[sequentialResourceIndex].Texture != nullptr)
			{
				VulkanTexture* element = static_cast<VulkanTexture*>(mSampledTextureData[sequentialResourceIndex].Texture.get());
				vulkanImage = element->GetVulkanResource();

				// Keep dynamic textures in general layout, so they can be easily mapped by CPU
				const TextureProperties& props = element->GetProperties();
				if(props.Usage.IsSet(TextureUsageFlag::StoreOnCPUWithGPUAccess))
					layout = VK_IMAGE_LAYOUT_GENERAL;
				else
					layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			}

			const TextureSurface& surface = mSampledTextureData[sequentialResourceIndex].Surface;
			const TArrayView<const GpuParameterObjectType> types = pipelineParameterInformationSet.GetTypes();
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

				vulkanImage = vkTexManager.GetDummyTexture(objectType)->GetVulkanResource();
				layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

				if(vulkanImage == nullptr)
					continue;

				const TArrayView<const GpuBufferFormat> elementTypes = pipelineParameterInformationSet.GetElementTypes();
				const VkFormat format = VulkanTextureManager::GetDummyViewFormat(elementTypes[usedBindingSequentialIndex]);

				imageView = vulkanImage->GetView(format, false);
			}

			// Register with command buffer
			VkImageSubresourceRange range = vulkanImage->GetRange(surface);

			const TArrayView<const VkDescriptorSetLayoutBinding> perSetBindings = pipelineParameterInformationSet.GetBindings();

			const GpuResourceUseFlags useFlags = VulkanUtility::ShaderToResourceUseFlags(perSetBindings[usedBindingSequentialIndex].stageFlags) | GpuResourceUseFlag::ShaderAccess;
			resourceTracker.TrackImageUsage(vulkanImage, range, layout, layout, useFlags, GpuAccessFlag::Read, barrierHelper);

			// Actual layout might be different than requested if the image is also used as a FB attachment
			layout = commandBuffer.GetCurrentLayout(vulkanImage, range, true); // TODO - Might not be necessary provided the resource tracker is aware the image is being used in the framebuffer

			// Check if internal resource changed from what was previously bound in the descriptor set
			B3D_ASSERT(mSampledImages[sequentialResourceIndex] != VK_NULL_HANDLE);

			VkDescriptorImageInfo& imageInfo = mSetInformation.ImageWriteInfos[usedResourceSequentialIndex];

			VkImage vkImage = vulkanImage->GetVulkanHandle();
			if(mSampledImages[sequentialResourceIndex] != vkImage)
			{
				mSampledImages[sequentialResourceIndex] = vkImage;

				imageInfo.imageView = imageView.Handle;
				mSetDirty = true;
			}

			if(imageInfo.imageLayout != layout)
			{
				imageInfo.imageLayout = layout;
				mSetDirty = true;
			}
		}
	}

	// Output dynamic offsets
	for(u32 dynamicOffset : dynamicOffsetMapping)
	{
		if(dynamicOffset != ~0u)
			outDynamicOffsets.Add(dynamicOffset);
	}

	// Acquire sets as needed, and update their contents if dirty
	if(mSetDirty)
	{
		// Set is dirty, we need to update
		// If current set is bound, allocate new from pool
		if(mSetInformation.DescriptorSet->IsBound())
		{
			VulkanGpuParameterSetPool* vulkanPool = static_cast<VulkanGpuParameterSetPool*>(mOwnerPool);
			VulkanDescriptorLayout* layout = pipelineParameterInformationSet.GetLayout();

			VulkanDescriptorSet* oldSet = mSetInformation.DescriptorSet;
			VulkanDescriptorSet* newSet = vulkanPool->AllocateDescriptorSet(layout->GetVulkanHandle());

			if (newSet != nullptr)
			{
				mSetInformation.DescriptorSet = newSet;

				if (oldSet != nullptr)
					oldSet->Destroy();
			}
			else
			{
				B3D_LOG(Warning, LogRenderBackend, "Pool exhausted during descriptor set reallocation. Reusing bound set.");
			}
		}

		// Note: Currently I write to the entire set at once, but it might be beneficial to remember only the exact
		// entries that were updated, and only write to them individually.
		mSetInformation.DescriptorSet->Write(mSetInformation.WriteSetInfos);

		mSetDirty = false;
	}

	VulkanDescriptorSet* const set = mSetInformation.DescriptorSet;

	resourceTracker.TrackResourceUsage(set, GpuAccessFlag::Read);
	outSet = set->GetVulkanHandle();
}
