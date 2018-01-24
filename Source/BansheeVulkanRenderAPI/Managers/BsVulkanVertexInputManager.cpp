//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "Managers/BsVulkanVertexInputManager.h"
#include "BsVulkanUtility.h"
#include "RenderAPI/BsVertexDeclaration.h"
#include "Profiling/BsRenderStats.h"

namespace bs { namespace ct
{
	VulkanVertexInput::VulkanVertexInput(UINT32 id, const VkPipelineVertexInputStateCreateInfo& createInfo)
		:mId(id), mCreateInfo(createInfo)
	{ }

	size_t VulkanVertexInputManager::HashFunc::operator()(const VertexDeclarationKey& key) const
	{
		size_t hash = 0;
		hash_combine(hash, key.bufferDeclId);
		hash_combine(hash, key.shaderDeclId);

		return hash;
	}

	bool VulkanVertexInputManager::EqualFunc::operator()(const VertexDeclarationKey& a, const VertexDeclarationKey& b) const
	{
		if (a.bufferDeclId != b.bufferDeclId)
			return false;

		if (a.shaderDeclId != b.shaderDeclId)
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

		while (mVertexInputMap.begin() != mVertexInputMap.end())
		{
			auto firstElem = mVertexInputMap.begin();
			mVertexInputMap.erase(firstElem);
		}
	}

	SPtr<VulkanVertexInput> VulkanVertexInputManager::getVertexInfo(
		const SPtr<VertexDeclaration>& vbDecl, const SPtr<VertexDeclaration>& shaderDecl)
	{
		Lock lock(mMutex);

		VertexDeclarationKey pair;
		pair.bufferDeclId = vbDecl->getId();
		pair.shaderDeclId = shaderDecl->getId();

		auto iterFind = mVertexInputMap.find(pair);
		if (iterFind == mVertexInputMap.end())
		{
			if (mVertexInputMap.size() >= DECLARATION_BUFFER_SIZE)
				removeLeastUsed(); // Prune so the buffer doesn't just infinitely grow

			addNew(vbDecl, shaderDecl);

			iterFind = mVertexInputMap.find(pair);
		}

		iterFind->second.lastUsedIdx = ++mLastUsedCounter;
		return iterFind->second.vertexInput;
	}

	void VulkanVertexInputManager::addNew(const SPtr<VertexDeclaration>& vbDecl, 
		const SPtr<VertexDeclaration>& shaderInputDecl)
	{
		const List<VertexElement>& vbElements = vbDecl->getProperties().getElements();
		const List<VertexElement>& inputElements = shaderInputDecl->getProperties().getElements();

		UINT32 numAttributes = 0;
		UINT32 numBindings = 0;

		for (auto& vbElem : vbElements)
		{
			bool foundSemantic = false;
			for (auto& inputElem : inputElements)
			{
				if (inputElem.getSemantic() == vbElem.getSemantic() && inputElem.getSemanticIdx() == vbElem.getSemanticIdx())
				{
					foundSemantic = true;
					break;
				}
			}

			if (!foundSemantic) // Shader needs to have a matching input attribute, otherwise we skip it
				continue;

			numAttributes++;
			numBindings = std::max(numBindings, (UINT32)vbElem.getStreamIdx() + 1);
		}

		VertexInputEntry newEntry;
		GroupAlloc& alloc = newEntry.allocator;

		alloc.reserve<VkVertexInputAttributeDescription>(numAttributes)
			 .reserve<VkVertexInputBindingDescription>(numBindings)
			 .init();

		newEntry.attributes = alloc.alloc<VkVertexInputAttributeDescription>(numAttributes);
		newEntry.bindings = alloc.alloc<VkVertexInputBindingDescription>(numBindings);

		for (UINT32 i = 0; i < numBindings; i++)
		{
			VkVertexInputBindingDescription& binding = newEntry.bindings[i];
			binding.binding = i;
			binding.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
			binding.stride = vbDecl->getProperties().getVertexSize(i);
		}

		UINT32 attribIdx = 0;
		bool isFirstInBinding = true;
		for (auto& vbElem : vbElements)
		{
			VkVertexInputAttributeDescription& attribute = newEntry.attributes[attribIdx];

			bool foundSemantic = false;
			for (auto& inputElem : inputElements)
			{
				if (inputElem.getSemantic() == vbElem.getSemantic() && inputElem.getSemanticIdx() == vbElem.getSemanticIdx())
				{
					foundSemantic = true;
					attribute.location = inputElem.getOffset();
					break;
				}
			}

			if (!foundSemantic) // Shader needs to have a matching input attribute, otherwise we skip it
				continue;

			attribute.binding = vbElem.getStreamIdx();
			attribute.format = VulkanUtility::getVertexType(vbElem.getType());
			attribute.offset = vbElem.getOffset();

			VkVertexInputBindingDescription& binding = newEntry.bindings[attribute.binding];

			bool isPerVertex = vbElem.getInstanceStepRate() == 0;
			if (isFirstInBinding)
			{
				binding.inputRate = isPerVertex ? VK_VERTEX_INPUT_RATE_VERTEX : VK_VERTEX_INPUT_RATE_INSTANCE;
				isFirstInBinding = false;
			}
			else
			{
				if ((binding.inputRate == VK_VERTEX_INPUT_RATE_VERTEX && !isPerVertex) ||
					(binding.inputRate == VK_VERTEX_INPUT_RATE_INSTANCE && isPerVertex))
				{
					LOGERR("Found multiple vertex attributes belonging to the same binding but with different input rates. "
						"All attributes in a binding must have the same input rate. Ignoring invalid input rates.")
				}
			}

			attribIdx++;
		}

		numAttributes = attribIdx; // It's possible some attributes were invalid, in which case we keep the memory allocated but ignore them otherwise

		VkPipelineVertexInputStateCreateInfo vertexInputCI;
		vertexInputCI.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
		vertexInputCI.pNext = nullptr;
		vertexInputCI.flags = 0;
		vertexInputCI.pVertexBindingDescriptions = newEntry.bindings;
		vertexInputCI.vertexBindingDescriptionCount = numBindings;
		vertexInputCI.pVertexAttributeDescriptions = newEntry.attributes;
		vertexInputCI.vertexAttributeDescriptionCount = numAttributes;

		// Create key and add to the layout map
		VertexDeclarationKey pair;
		pair.bufferDeclId = vbDecl->getId();
		pair.shaderDeclId = shaderInputDecl->getId();

		newEntry.vertexInput = bs_shared_ptr_new<VulkanVertexInput>(mNextId++, vertexInputCI);
		newEntry.lastUsedIdx = ++mLastUsedCounter;

		mVertexInputMap[pair] = std::move(newEntry);
	}

	void VulkanVertexInputManager::removeLeastUsed()
	{
		Lock lock(mMutex);

		if (!mWarningShown)
		{
			LOGWRN("Vertex input buffer is full, pruning last " + toString(NUM_ELEMENTS_TO_PRUNE) + " elements. This is "
				"probably okay unless you are creating a massive amount of input layouts as they will get re-created every "
				"frame. In that case you should increase the layout buffer size. This warning won't be shown again.");

			mWarningShown = true;
		}

		Map<UINT32, VertexDeclarationKey> leastFrequentlyUsedMap;

		for (auto iter = mVertexInputMap.begin(); iter != mVertexInputMap.end(); ++iter)
			leastFrequentlyUsedMap[iter->second.lastUsedIdx] = iter->first;

		UINT32 elemsRemoved = 0;
		for (auto iter = leastFrequentlyUsedMap.begin(); iter != leastFrequentlyUsedMap.end(); ++iter)
		{
			auto inputLayoutIter = mVertexInputMap.find(iter->second);
			mVertexInputMap.erase(inputLayoutIter);

			elemsRemoved++;
			if (elemsRemoved >= NUM_ELEMENTS_TO_PRUNE)
				break;
		}
	}
}}