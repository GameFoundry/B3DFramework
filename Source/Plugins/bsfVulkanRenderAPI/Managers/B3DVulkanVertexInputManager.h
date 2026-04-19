//************************************ B3D Framework - Copyright 2026 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DVulkanPrerequisites.h"
#include "Allocators/B3DGroupAlloc.h"
#include "Utility/B3DModule.h"

namespace b3d
{
	namespace render
	{
		/** @addtogroup Vulkan
		 *  @{
		 */

		/** Contains data describing vertex inputs for a graphics pipeline. */
		class VulkanVertexInput
		{
		public:
			VulkanVertexInput(u32 id, const VkPipelineVertexInputStateCreateInfo& createInfo, u32 bindingCount);

			/** Returns an object contining the necessary information to initialize the vertex input on a pipeline. */
			const VkPipelineVertexInputStateCreateInfo* GetCreateInfo() const { return &mCreateInfo; }

			/** Returns an identifier which uniquely represents this vertex input configuration. */
			u32 GetId() const { return mId; }

			/** Returns the number of vertex buffers that are expected to be bound as vertex input. */
			u32 GetVertexBufferBindingCount() const { return mBindingCount; }

		private:
			u32 mId;
			u32 mBindingCount = 0;
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
				u32 BufferDeclarationId;
				u32 ShaderDeclarationId;
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
				TArrayView<VkVertexInputAttributeDescription> Attributes;
				TArrayView<VkVertexInputBindingDescription> Bindings;
				SPtr<VulkanVertexInput> VertexInput;
				u32 LastUsedIdx;

				GroupAllocator Allocator;
			};

		public:
			VulkanVertexInputManager();
			~VulkanVertexInputManager();

			/**
			 * Returns an object that describes how vertex buffer elements map to vertex shader inputs.
			 *
			 * @param[in]	vertexBufferDescription		Describes the structure of a single vertex in a vertex buffer.
			 * @param[in]	shaderInputDescription		Describes the vertex element inputs expected by a vertex shader.
			 * @return									Vertex input state description, usable by Vulkan.
			 */
			SPtr<VulkanVertexInput> GetVertexInfo(const SPtr<VertexDescription>& vertexBufferDescription, const SPtr<VertexDescription>& shaderInputDescription);

		private:
			/**	Creates a vertex input using the specified parameters and stores it in the input layout map. */
			void AddNew(const SPtr<VertexDescription>& vertexBufferDescription, const SPtr<VertexDescription>& shaderInputDescription);

			/**	Removes the least used vertex input. */
			void RemoveLeastUsed();

		private:
			static const int kDeclarationBufferSize = 1024;
			static const int kElementCountToPrune = 64;

			UnorderedMap<VertexDeclarationKey, VertexInputEntry, HashFunc, EqualFunc> mVertexInputMap;

			u32 mNextId;
			bool mWarningShown;
			u32 mLastUsedCounter;

			Mutex mMutex;
		};

		/** @} */
	} // namespace render
} // namespace b3d
