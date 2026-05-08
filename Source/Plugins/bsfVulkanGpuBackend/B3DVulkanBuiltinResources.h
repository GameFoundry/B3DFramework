//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DVulkanPrerequisites.h"

namespace b3d
{
	namespace render
	{
		/** @addtogroup Vulkan
		 *  @{
		 */

		/** Contains various resources that are often required by the Vulkan backend. */
		class VulkanBuiltinResources
		{
		public:
			VulkanBuiltinResources(VulkanGpuDevice& gpuDevice);

			/** Initializes all the builtin resources. */
			void Initialize();

			/** Destroys all the builtin resources. */
			void Cleanup();

			/** Buffer that can be used for buffer read operations when no other buffer is bound. */
			TShared<VulkanGpuBuffer> DummyReadBuffer;

			/** Buffer that can be used for buffer storage operations when no other buffer is bound. */
			TShared<VulkanGpuBuffer> DummyStorageBuffer;

			/** Buffer that can be used for uniform storage when no other buffer is bound. */
			TShared<VulkanGpuBuffer> DummyUniformBuffer;

			/** Buffer that can be used for structured storage when no other buffer is bound. */
			TShared<VulkanGpuBuffer> DummyStructuredBuffer;

			/** Buffer that can be used for vertex buffers when no other buffer is bound. */
			TShared<VulkanGpuBuffer> DummyVertexBuffer;

		private:
			VulkanGpuDevice& mGpuDevice;
		};

		/** @} */
	} // namespace render
} // namespace b3d
