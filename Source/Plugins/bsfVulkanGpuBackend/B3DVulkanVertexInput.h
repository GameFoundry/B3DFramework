//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DVulkanPrerequisites.h"
#include "B3DVulkanResource.h"
#include "GpuBackend/B3DGpuVertexInputManager.h"
#include "Allocators/B3DGroupAlloc.h"

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
		class VulkanVertexInput : public TVulkanResource<GpuVertexInput>
		{
		public:
			/**
			 * @param	owner		Manager that takes care of tracking and releasing of this object.
			 * @param	id			Identifier which uniquely represents this vertex input configuration.
			 * @param	layout		Vertex buffer layout resolved against the vertex shader inputs.
			 */
			VulkanVertexInput(VulkanResourceManager* owner, u32 id, const GpuVertexInputLayout& layout);

			/** Returns an object contining the necessary information to initialize the vertex input on a pipeline. */
			const VkPipelineVertexInputStateCreateInfo* GetCreateInfo() const { return &mCreateInfo; }

			/** Returns the number of vertex buffers that are expected to be bound as vertex input. */
			u32 GetVertexBufferBindingCount() const { return (u32)mBindings.Size(); }

		private:
			GroupAllocator mAllocator;
			TArrayView<VkVertexInputAttributeDescription> mAttributes;
			TArrayView<VkVertexInputBindingDescription> mBindings;
			VkPipelineVertexInputStateCreateInfo mCreateInfo;
		};

		/** @} */
	} // namespace render
} // namespace b3d
