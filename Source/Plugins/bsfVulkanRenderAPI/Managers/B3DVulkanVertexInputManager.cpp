//************************************ B3D Framework - Copyright 2026 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Managers/B3DVulkanVertexInputManager.h"
#include "B3DVulkanUtility.h"
#include "Profiling/B3DRenderStats.h"
#include "RenderAPI/B3DVertexDescription.h"

using namespace b3d;
using namespace b3d::render;

VulkanVertexInput::VulkanVertexInput(u32 id, const VkPipelineVertexInputStateCreateInfo& createInfo, u32 bindingCount)
	: mId(id), mBindingCount(bindingCount), mCreateInfo(createInfo)
{}

const int VulkanVertexInputManager::kElementCountToPrune;

size_t VulkanVertexInputManager::HashFunc::operator()(const VertexDeclarationKey& key) const
{
	size_t hash = 0;
	B3DCombineHash(hash, key.BufferDeclarationId);
	B3DCombineHash(hash, key.ShaderDeclarationId);

	return hash;
}

bool VulkanVertexInputManager::EqualFunc::operator()(const VertexDeclarationKey& a, const VertexDeclarationKey& b) const
{
	if(a.BufferDeclarationId != b.BufferDeclarationId)
		return false;

	if(a.ShaderDeclarationId != b.ShaderDeclarationId)
		return false;

	return true;
}

VulkanVertexInputManager::VulkanVertexInputManager()
{
	Lock lock(mMutex);

	mNextId = 1;
	mWarningShown = false;
	mLastUsedCounter = 0;
}

VulkanVertexInputManager::~VulkanVertexInputManager()
{
	Lock lock(mMutex);

	while(mVertexInputMap.begin() != mVertexInputMap.end())
	{
		auto firstElem = mVertexInputMap.begin();
		mVertexInputMap.erase(firstElem);
	}
}

SPtr<VulkanVertexInput> VulkanVertexInputManager::GetVertexInfo(const SPtr<VertexDescription>& vertexBufferDescription, const SPtr<VertexDescription>& shaderInputDescription)
{
	Lock lock(mMutex);

	VertexDeclarationKey pair;
	pair.BufferDeclarationId = vertexBufferDescription->GetId();
	pair.ShaderDeclarationId = shaderInputDescription->GetId();

	auto iterFind = mVertexInputMap.find(pair);
	if(iterFind == mVertexInputMap.end())
	{
		if(mVertexInputMap.size() >= kDeclarationBufferSize)
			RemoveLeastUsed(); // Prune so the buffer doesn't just infinitely grow

		AddNew(vertexBufferDescription, shaderInputDescription);

		iterFind = mVertexInputMap.find(pair);
	}

	iterFind->second.LastUsedIdx = ++mLastUsedCounter;
	return iterFind->second.VertexInput;
}

void VulkanVertexInputManager::AddNew(const SPtr<VertexDescription>& vertexBufferDescription, const SPtr<VertexDescription>& shaderInputDescription)
{
	const TInlineArray<VertexElement, 8>& vertexBufferElements = vertexBufferDescription->GetElements();
	const TInlineArray<VertexElement, 8>& shaderInputElements = shaderInputDescription->GetElements();

	const u32 attributeCount = (u32)shaderInputElements.size();

	bool areAnyShaderInputsMissing = false;
	u32 bindingCount = 0;
	for(const auto& shaderInputElement : shaderInputElements)
	{
		const VertexElement* matchingElement = nullptr;
		for(const auto& vertexBufferElement : vertexBufferElements)
		{
			if(shaderInputElement.GetSemantic() == vertexBufferElement.GetSemantic() && shaderInputElement.GetSemanticIndex() == vertexBufferElement.GetSemanticIndex())
			{
				matchingElement = &vertexBufferElement;
				break;
			}
		}

		if(matchingElement == nullptr)
		{
			areAnyShaderInputsMissing = true;
			continue;
		}

		bindingCount = Math::Max(bindingCount, (u32)matchingElement->GetStreamIndex() + 1);
	}

	// Add extra binding to store the empty vertex buffer for missing attributes
	if(areAnyShaderInputsMissing)
		bindingCount++;

	VertexInputEntry newEntry;
	GroupAllocator& alloc = newEntry.Allocator;

	alloc.Reserve<VkVertexInputAttributeDescription>(attributeCount)
		.Reserve<VkVertexInputBindingDescription>(bindingCount)
		.Initialize();

	newEntry.Attributes = alloc.Allocate<VkVertexInputAttributeDescription>(attributeCount);
	newEntry.Bindings = alloc.Allocate<VkVertexInputBindingDescription>(bindingCount);

	bool* isFirstAttributeInVertexBuffer = B3DStackAllocate<bool>(bindingCount);
	for(u32 i = 0; i < bindingCount; i++)
	{
		// This is the empty buffer binding we use if any shader inputs are missing
		const bool isNullBinding = areAnyShaderInputsMissing && ((i + 1) == bindingCount);

		VkVertexInputBindingDescription& binding = newEntry.Bindings[i];
		binding.binding = i;
		binding.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
		binding.stride = isNullBinding ? 0 : vertexBufferDescription->GetVertexStride(i);

		isFirstAttributeInVertexBuffer[i] = true;
	}

	u32 attributeIndex = 0;
	for(auto& shaderInputElement : shaderInputElements)
	{
		VkVertexInputAttributeDescription& attribute = newEntry.Attributes[attributeIndex];

		const VertexElement* matchingElement = nullptr;
		for(auto& vertexBufferElement : vertexBufferElements)
		{
			if(shaderInputElement.GetSemantic() == vertexBufferElement.GetSemantic() && shaderInputElement.GetSemanticIndex() == vertexBufferElement.GetSemanticIndex())
			{
				matchingElement = &vertexBufferElement;
				break;
			}
		}

		attribute.location = shaderInputElement.GetOffset();

		bool isSteppingPerVertex;
		if(matchingElement != nullptr)
		{
			attribute.binding = matchingElement->GetStreamIndex();
			attribute.format = VulkanUtility::GetVertexType(matchingElement->GetType());
			attribute.offset = matchingElement->GetOffset();

			isSteppingPerVertex = matchingElement->GetInstanceStepRate() == 0;
		}
		else
		{
			attribute.binding = bindingCount - 1;
			attribute.format = VulkanUtility::GetVertexType(shaderInputElement.GetType());
			attribute.offset = 0;

			isSteppingPerVertex = true;
		}

		VkVertexInputBindingDescription& binding = newEntry.Bindings[attribute.binding];
		if(isFirstAttributeInVertexBuffer[attribute.binding])
		{
			binding.inputRate = isSteppingPerVertex ? VK_VERTEX_INPUT_RATE_VERTEX : VK_VERTEX_INPUT_RATE_INSTANCE;
			isFirstAttributeInVertexBuffer[attribute.binding] = false;
		}
		else
		{
			if((binding.inputRate == VK_VERTEX_INPUT_RATE_VERTEX && !isSteppingPerVertex) ||
			   (binding.inputRate == VK_VERTEX_INPUT_RATE_INSTANCE && isSteppingPerVertex))
			{
				B3D_LOG(Error, LogRenderBackend, "Found multiple vertex attributes belonging to the same binding but with "
											 "different input rates. All attributes in a binding must have the same input rate. Ignoring "
											 "invalid input rates.");
			}
		}

		attributeIndex++;
	}

	B3DStackFree(isFirstAttributeInVertexBuffer);

	VkPipelineVertexInputStateCreateInfo vertexInputCI;
	vertexInputCI.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	vertexInputCI.pNext = nullptr;
	vertexInputCI.flags = 0;
	vertexInputCI.pVertexBindingDescriptions = newEntry.Bindings.Data();
	vertexInputCI.vertexBindingDescriptionCount = bindingCount;
	vertexInputCI.pVertexAttributeDescriptions = newEntry.Attributes.Data();
	vertexInputCI.vertexAttributeDescriptionCount = attributeCount;

	// Create key and add to the layout map
	VertexDeclarationKey pair;
	pair.BufferDeclarationId = vertexBufferDescription->GetId();
	pair.ShaderDeclarationId = shaderInputDescription->GetId();

	newEntry.VertexInput = B3DMakeShared<VulkanVertexInput>(mNextId++, vertexInputCI, bindingCount);
	newEntry.LastUsedIdx = ++mLastUsedCounter;

	mVertexInputMap[pair] = std::move(newEntry);
}

void VulkanVertexInputManager::RemoveLeastUsed()
{
	Lock lock(mMutex);

	if(!mWarningShown)
	{
		B3D_LOG(Warning, LogRenderBackend, "Vertex input buffer is full, pruning last {0} elements. This is "
									   "probably okay unless you are creating a massive amount of input layouts as they will get re-created every "
									   "frame. In that case you should increase the layout buffer size. This warning won't be shown again.",
			   kElementCountToPrune);

		mWarningShown = true;
	}

	Map<u32, VertexDeclarationKey> leastFrequentlyUsedMap;

	for(auto iter = mVertexInputMap.begin(); iter != mVertexInputMap.end(); ++iter)
		leastFrequentlyUsedMap[iter->second.LastUsedIdx] = iter->first;

	u32 elemsRemoved = 0;
	for(auto iter = leastFrequentlyUsedMap.begin(); iter != leastFrequentlyUsedMap.end(); ++iter)
	{
		auto inputLayoutIter = mVertexInputMap.find(iter->second);
		mVertexInputMap.erase(inputLayoutIter);

		elemsRemoved++;
		if(elemsRemoved >= kElementCountToPrune)
			break;
	}
}
