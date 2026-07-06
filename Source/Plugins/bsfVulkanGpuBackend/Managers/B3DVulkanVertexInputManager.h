//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DVulkanPrerequisites.h"
#include "GpuBackend/B3DGpuVertexInputManager.h"
#include "Allocators/B3DGroupAlloc.h"
#include "Utility/B3DModule.h"

namespace b3d
{
	namespace render
	{
		/** @addtogroup Vulkan
		 *  @{
		 */

		/**
		 * Contains data describing vertex inputs for a graphics pipeline. Owns the attribute and binding descriptions
		 * the create info points to, so the create info stays valid for the lifetime of this object.
		 */
		class VulkanVertexInput
		{
		public:
			/**
			 * @param	id			Identifier which uniquely represents this vertex input configuration.
			 * @param	layout		Vertex buffer layout resolved against the vertex shader inputs.
			 */
			VulkanVertexInput(u32 id, const GpuVertexInputLayout& layout);

			/** Returns an object contining the necessary information to initialize the vertex input on a pipeline. */
			const VkPipelineVertexInputStateCreateInfo* GetCreateInfo() const { return &mCreateInfo; }

			/** Returns an identifier which uniquely represents this vertex input configuration. */
			u32 GetId() const { return mId; }

			/** Returns the number of vertex buffers that are expected to be bound as vertex input. */
			u32 GetVertexBufferBindingCount() const { return (u32)mBindings.Size(); }

		private:
			u32 mId;

			GroupAllocator mAllocator;
			TArrayView<VkVertexInputAttributeDescription> mAttributes;
			TArrayView<VkVertexInputBindingDescription> mBindings;
			VkPipelineVertexInputStateCreateInfo mCreateInfo;
		};

		class VulkanVertexInputManager;
		extern template class TGpuVertexInputManager<VulkanVertexInputManager, TShared<VulkanVertexInput>>;

		/**
		 * Maps vertex buffer structure and vertex shader inputs in order to create vertex input description usable by Vulkan.
		 */
		class VulkanVertexInputManager : public Module<VulkanVertexInputManager>, public TGpuVertexInputManager<VulkanVertexInputManager, TShared<VulkanVertexInput>>
		{
		public:
			~VulkanVertexInputManager();

		private:
			friend class TGpuVertexInputManager<VulkanVertexInputManager, TShared<VulkanVertexInput>>;

			/** Creates a vertex input object for the provided resolved layout. */
			TShared<VulkanVertexInput> CreateVertexInput(const GpuVertexInputLayout& layout);

			/** Releases a vertex input object evicted from the cache. */
			void DestroyVertexInput(TShared<VulkanVertexInput>& vertexInput);

			u32 mNextId = 1;
		};

		/** @} */
	} // namespace render
} // namespace b3d
