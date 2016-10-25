//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsVulkanPrerequisites.h"
#include "BsModule.h"

namespace BansheeEngine
{
	/** @addtogroup Vulkan
	 *  @{
	 */

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
			VertexInputEntry() {}

			VkVertexInputAttributeDescription* attributes;
			VkVertexInputBindingDescription* bindings;
			VkPipelineVertexInputStateCreateInfo vertexInputCI;
			UINT32 lastUsedIdx;
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
		const VkPipelineVertexInputStateCreateInfo& getVertexInfo(const SPtr<VertexDeclarationCore>& vbDecl, 
			const SPtr<VertexDeclarationCore>& shaderDecl);

	private:
		/**	Creates a vertex input using the specified parameters and stores it in the input layout map. */
		void addNew(const SPtr<VertexDeclarationCore>& vbDecl, const SPtr<VertexDeclarationCore>& shaderDecl);

		/**	Removes the least used vertex input. */
		void removeLeastUsed();

	private:
		static const int DECLARATION_BUFFER_SIZE = 1024;
		static const int NUM_ELEMENTS_TO_PRUNE = 64;

		UnorderedMap<VertexDeclarationKey, VertexInputEntry*, HashFunc, EqualFunc> mVertexInputMap;

		bool mWarningShown;
		UINT32 mLastUsedCounter;
    };

	/** @} */
}
