//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Managers/BsVulkanVertexInputManager.h"
#include "BsVulkanUtility.h"
#include "RenderAPI/BsVertexDeclaration.h"
#include "Profiling/BsRenderStats.h"

using namespace bs;
using namespace bs::ct;

VulkanVertexInput::VulkanVertexInput(u32 id, const VkPipelineVertexInputStateCreateInfo& createInfo)
	: mId(id), mCreateInfo(createInfo)
{}

const int VulkanVertexInputManager::NUM_ELEMENTS_TO_PRUNE;

size_t VulkanVertexInputManager::HashFunc::operator()(const VertexDeclarationKey& key) const
{
	size_t hash = 0;
	bs_hash_combine(hash, key.BufferDeclId);
	bs_hash_combine(hash, key.ShaderDeclId);

	return hash;
}

bool VulkanVertexInputManager::EqualFunc::operator()(const VertexDeclarationKey& a, const VertexDeclarationKey& b) const
{
	if(a.BufferDeclId != b.BufferDeclId)
		return false;

	if(a.ShaderDeclId != b.ShaderDeclId)
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

SPtr<VulkanVertexInput> VulkanVertexInputManager::GetVertexInfo(
	const SPtr<VertexDeclaration>& vbDecl, const SPtr<VertexDeclaration>& shaderDecl)
{
	Lock lock(mMutex);

	VertexDeclarationKey pair;
	pair.BufferDeclId = vbDecl->GetId();
	pair.ShaderDeclId = shaderDecl->GetId();

	auto iterFind = mVertexInputMap.find(pair);
	if(iterFind == mVertexInputMap.end())
	{
		if(mVertexInputMap.size() >= DECLARATION_BUFFER_SIZE)
			RemoveLeastUsed(); // Prune so the buffer doesn't just infinitely grow

		AddNew(vbDecl, shaderDecl);

		iterFind = mVertexInputMap.find(pair);
	}

	iterFind->second.LastUsedIdx = ++mLastUsedCounter;
	return iterFind->second.VertexInput;
}

void VulkanVertexInputManager::AddNew(const SPtr<VertexDeclaration>& vbDecl, const SPtr<VertexDeclaration>& shaderInputDecl)
{
	const Vector<VertexElement>& vbElements = vbDecl->GetProperties().GetElements();
	const Vector<VertexElement>& inputElements = shaderInputDecl->GetProperties().GetElements();

	u32 numAttributes = 0;
	u32 numBindings = 0;

	for(auto& vbElem : vbElements)
	{
		bool foundSemantic = false;
		for(auto& inputElem : inputElements)
		{
			if(inputElem.GetSemantic() == vbElem.GetSemantic() && inputElem.GetSemanticIdx() == vbElem.GetSemanticIdx())
			{
				foundSemantic = true;
				break;
			}
		}

		if(!foundSemantic) // Shader needs to have a matching input attribute, otherwise we skip it
			continue;

		numAttributes++;
		numBindings = std::max(numBindings, (u32)vbElem.GetStreamIdx() + 1);
	}

	VertexInputEntry newEntry;
	GroupAlloc& alloc = newEntry.Allocator;

	alloc.Reserve<VkVertexInputAttributeDescription>(numAttributes)
		.Reserve<VkVertexInputBindingDescription>(numBindings)
		.Init();

	newEntry.Attributes = alloc.Alloc<VkVertexInputAttributeDescription>(numAttributes);
	newEntry.Bindings = alloc.Alloc<VkVertexInputBindingDescription>(numBindings);

	for(u32 i = 0; i < numBindings; i++)
	{
		VkVertexInputBindingDescription& binding = newEntry.Bindings[i];
		binding.binding = i;
		binding.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
		binding.stride = vbDecl->GetProperties().GetVertexSize(i);
	}

	u32 attribIdx = 0;
	bool isFirstInBinding = true;
	for(auto& vbElem : vbElements)
	{
		VkVertexInputAttributeDescription& attribute = newEntry.Attributes[attribIdx];

		bool foundSemantic = false;
		for(auto& inputElem : inputElements)
		{
			if(inputElem.GetSemantic() == vbElem.GetSemantic() && inputElem.GetSemanticIdx() == vbElem.GetSemanticIdx())
			{
				foundSemantic = true;
				attribute.location = inputElem.GetOffset();
				break;
			}
		}

		if(!foundSemantic) // Shader needs to have a matching input attribute, otherwise we skip it
			continue;

		attribute.binding = vbElem.GetStreamIdx();
		attribute.format = VulkanUtility::GetVertexType(vbElem.GetType());
		attribute.offset = vbElem.GetOffset();

		VkVertexInputBindingDescription& binding = newEntry.Bindings[attribute.binding];

		bool isPerVertex = vbElem.GetInstanceStepRate() == 0;
		if(isFirstInBinding)
		{
			binding.inputRate = isPerVertex ? VK_VERTEX_INPUT_RATE_VERTEX : VK_VERTEX_INPUT_RATE_INSTANCE;
			isFirstInBinding = false;
		}
		else
		{
			if((binding.inputRate == VK_VERTEX_INPUT_RATE_VERTEX && !isPerVertex) ||
			   (binding.inputRate == VK_VERTEX_INPUT_RATE_INSTANCE && isPerVertex))
			{
				BS_LOG(Error, RenderBackend, "Found multiple vertex attributes belonging to the same binding but with "
											 "different input rates. All attributes in a binding must have the same input rate. Ignoring "
											 "invalid input rates.");
			}
		}

		attribIdx++;
	}

	numAttributes = attribIdx; // It's possible some attributes were invalid, in which case we keep the memory allocated but ignore them otherwise

	VkPipelineVertexInputStateCreateInfo vertexInputCI;
	vertexInputCI.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	vertexInputCI.pNext = nullptr;
	vertexInputCI.flags = 0;
	vertexInputCI.pVertexBindingDescriptions = newEntry.Bindings;
	vertexInputCI.vertexBindingDescriptionCount = numBindings;
	vertexInputCI.pVertexAttributeDescriptions = newEntry.Attributes;
	vertexInputCI.vertexAttributeDescriptionCount = numAttributes;

	// Create key and add to the layout map
	VertexDeclarationKey pair;
	pair.BufferDeclId = vbDecl->GetId();
	pair.ShaderDeclId = shaderInputDecl->GetId();

	newEntry.VertexInput = B3DMakeShared<VulkanVertexInput>(mNextId++, vertexInputCI);
	newEntry.LastUsedIdx = ++mLastUsedCounter;

	mVertexInputMap[pair] = std::move(newEntry);
}

void VulkanVertexInputManager::RemoveLeastUsed()
{
	Lock lock(mMutex);

	if(!mWarningShown)
	{
		BS_LOG(Warning, RenderBackend, "Vertex input buffer is full, pruning last {0} elements. This is "
									   "probably okay unless you are creating a massive amount of input layouts as they will get re-created every "
									   "frame. In that case you should increase the layout buffer size. This warning won't be shown again.",
			   NUM_ELEMENTS_TO_PRUNE);

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
		if(elemsRemoved >= NUM_ELEMENTS_TO_PRUNE)
			break;
	}
}
