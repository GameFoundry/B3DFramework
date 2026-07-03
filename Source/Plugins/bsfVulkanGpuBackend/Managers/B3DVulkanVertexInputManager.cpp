//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Managers/B3DVulkanVertexInputManager.h"
#include "B3DVulkanUtility.h"
#include "Profiling/B3DRenderStats.h"
#include "GpuBackend/B3DVertexDescription.h"

// Generic manager method definitions, followed by the explicit instantiation for the Vulkan vertex input types.
// Included here so the single instantiation lives in this translation unit. The header carries a matching
// `extern template` to suppress implicit instantiation elsewhere.
#include "GpuBackend/B3DGpuVertexInputManager.inl"

template class b3d::render::TGpuVertexInputManager<b3d::render::VulkanVertexInputManager, b3d::TShared<b3d::render::VulkanVertexInput>>;

using namespace b3d;
using namespace b3d::render;

VulkanVertexInput::VulkanVertexInput(u32 id, const TShared<VertexDescription>& vertexBufferDescription, const GpuVertexInputLayout& layout)
	: mId(id)
{
	const u32 attributeCount = (u32)layout.Attributes.Size();
	const u32 bindingCount = layout.StreamCount;

	mAllocator.Reserve<VkVertexInputAttributeDescription>(attributeCount)
		.Reserve<VkVertexInputBindingDescription>(bindingCount)
		.Initialize();

	mAttributes = mAllocator.Allocate<VkVertexInputAttributeDescription>(attributeCount);
	mBindings = mAllocator.Allocate<VkVertexInputBindingDescription>(bindingCount);

	bool* isFirstAttributeInVertexBuffer = B3DStackAllocate<bool>(bindingCount);
	for(u32 bindingIndex = 0; bindingIndex < bindingCount; bindingIndex++)
	{
		// The null stream is the empty buffer binding we use if any shader inputs are missing
		const bool isNullBinding = bindingIndex == layout.NullStreamIndex;

		VkVertexInputBindingDescription& binding = mBindings[bindingIndex];
		binding.binding = bindingIndex;
		binding.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
		binding.stride = isNullBinding ? 0 : vertexBufferDescription->GetVertexStride(bindingIndex);

		isFirstAttributeInVertexBuffer[bindingIndex] = true;
	}

	u32 attributeIndex = 0;
	for(const GpuVertexInputAttribute& resolvedAttribute : layout.Attributes)
	{
		VkVertexInputAttributeDescription& attribute = mAttributes[attributeIndex];
		attribute.location = resolvedAttribute.ShaderInput->GetOffset();
		attribute.binding = resolvedAttribute.StreamIndex;

		if(resolvedAttribute.BufferElement != nullptr)
		{
			attribute.format = VulkanUtility::GetVertexType(resolvedAttribute.BufferElement->GetType());
			attribute.offset = resolvedAttribute.BufferElement->GetOffset();
		}
		else
		{
			attribute.format = VulkanUtility::GetVertexType(resolvedAttribute.ShaderInput->GetType());
			attribute.offset = 0;
		}

		VkVertexInputBindingDescription& binding = mBindings[attribute.binding];
		if(isFirstAttributeInVertexBuffer[attribute.binding])
		{
			binding.inputRate = resolvedAttribute.SteppedPerInstance ? VK_VERTEX_INPUT_RATE_INSTANCE : VK_VERTEX_INPUT_RATE_VERTEX;
			isFirstAttributeInVertexBuffer[attribute.binding] = false;
		}
		else
		{
			if((binding.inputRate == VK_VERTEX_INPUT_RATE_VERTEX && resolvedAttribute.SteppedPerInstance) ||
			   (binding.inputRate == VK_VERTEX_INPUT_RATE_INSTANCE && !resolvedAttribute.SteppedPerInstance))
			{
				B3D_LOG(Error, LogRenderBackend, "Found multiple vertex attributes belonging to the same binding but with "
											 "different input rates. All attributes in a binding must have the same input rate. Ignoring "
											 "invalid input rates.");
			}
		}

		attributeIndex++;
	}

	B3DStackFree(isFirstAttributeInVertexBuffer);

	mCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	mCreateInfo.pNext = nullptr;
	mCreateInfo.flags = 0;
	mCreateInfo.pVertexBindingDescriptions = mBindings.Data();
	mCreateInfo.vertexBindingDescriptionCount = bindingCount;
	mCreateInfo.pVertexAttributeDescriptions = mAttributes.Data();
	mCreateInfo.vertexAttributeDescriptionCount = attributeCount;
}

VulkanVertexInputManager::~VulkanVertexInputManager()
{
	ReleaseAll();
}

TShared<VulkanVertexInput> VulkanVertexInputManager::CreateVertexInput(const TShared<VertexDescription>& vertexBufferDescription, const TShared<VertexDescription>& shaderInputDescription, const GpuVertexInputLayout& layout)
{
	return B3DMakeShared<VulkanVertexInput>(mNextId++, vertexBufferDescription, layout);
}

void VulkanVertexInputManager::DestroyVertexInput(TShared<VulkanVertexInput>& vertexInput)
{
	vertexInput = nullptr;
}
