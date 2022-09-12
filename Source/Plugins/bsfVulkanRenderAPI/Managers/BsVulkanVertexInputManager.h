//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsVulkanPrerequisites.h"
#include "Allocators/BsGroupAlloc.h"
#include "Utility/BsModule.h"

namespace bs { namespace ct
{
	/** @addtogroup Vulkan
	 *  @{
	 */

	/** Contains data describing vertex inputs for a graphics pipeline. */
	class VulkanVertexInput
	{
	public:
		VulkanVertexInput(UINT32 id, const VkPipelineVertexInputStateCreateInfo& createInfo);

		/** Returns an object contining the necessary information to initialize the vertex input on a pipeline. */
		const VkPipelineVertexInputStateCreateInfo* getCreateInfo() const { return &mCreateInfo; }

		/** Returns an identifier which uniquely represents this vertex input configuration. */
		UINT32 GetId() const { return mId; }

	private:
		UINT32 mId;
		VkPipelineVertexInputStateCreateInfo mCreateInfo;
	};

	/**
	 * Maps vertex buffer structure and vertex shader inputs in order to create vertex input description usable by Vulkan.
	 */
	class VulkanVertexInputManager : public Module<VulkanVertexInputManager>
    {
	private:
		/**	Key uniquely identifying buffer and shader vertex declarations. */
		struct VertexDeclarationKey
		{
			UINT32 bufferDeclId;
			UINT32 shaderDeclId;
		};

		/**	Creates a hash from vertex declaration key. */
		class HashFunc
		{
		public:
			::std::size_t operator()(const VertexDeclarationKey& key) const;
		};

		/**	Compares two vertex declaration keys. */
		class EqualFunc
		{
		public:
			bool operator()(const VertexDeclarationKey& a, const VertexDeclarationKey& b) const;
		};

		/**	Contains data about a single instance of vertex input object. */
		struct VertexInputEntry
		{
			VkVertexInputAttributeDescription* attributes;
			VkVertexInputBindingDescription* bindings;
			SPtr<VulkanVertexInput> vertexInput;
			UINT32 lastUsedIdx;

			GroupAlloc allocator;
		};

	public:
		VulkanVertexInputManager();
		~VulkanVertexInputManager();

		/**
		 * Returns an object that describes how vertex buffer elements map to vertex shader inputs.
		 *
		 * @param[in]	vbDecl		Describes the structure of a single vertex in a vertex buffer.
		 * @param[in]	shaderDecl	Describes the vertex element inputs expected by a vertex shader.
		 * @return					Vertex input state description, usable by Vulkan.
		 */
		SPtr<VulkanVertexInput> getVertexInfo(const SPtr<VertexDeclaration>& vbDecl,
											  const SPtr<VertexDeclaration>& shaderDecl);

	private:
		/**	Creates a vertex input using the specified parameters and stores it in the input layout map. */
		void AddNew(const SPtr<VertexDeclaration>& vbDecl, const SPtr<VertexDeclaration>& shaderDecl);

		/**	Removes the least used vertex input. */
		void RemoveLeastUsed();

	private:
		static const int DECLARATION_BUFFER_SIZE = 1024;
		static const int NUM_ELEMENTS_TO_PRUNE = 64;

		UnorderedMap<VertexDeclarationKey, VertexInputEntry, HashFunc, EqualFunc> mVertexInputMap;

		UINT32 mNextId;
		bool mWarningShown;
		UINT32 mLastUsedCounter;

		Mutex mMutex;
    };

	/** @} */
}}
