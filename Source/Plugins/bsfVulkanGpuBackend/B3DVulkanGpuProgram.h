//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DVulkanPrerequisites.h"
#include "B3DVulkanResource.h"
#include "GpuBackend/B3DGpuProgram.h"

namespace b3d
{
	namespace render
	{
		/** @addtogroup Vulkan
		 *  @{
		 */

		/** Wrapper around a Vulkan shader module (GPU program) that manages its usage and lifetime. */
		class VulkanShaderModule : public VulkanResource
		{
		public:
			VulkanShaderModule(VulkanResourceManager* owner, VkShaderModule module, const StringView& name = "");
			~VulkanShaderModule();

			/** Returns the internal handle to the Vulkan object. */
			VkShaderModule GetVulkanHandle() const { return mModule; }

			/** Assigns an name to the shader module, primarily used for easier debugging. */
			void SetName(const StringView& name);

		private:
			VkShaderModule mModule;
		};

		/**	Abstraction of a Vulkan shader object. */
		class VulkanGpuProgram : public GpuProgram
		{
		public:
			VulkanGpuProgram(VulkanGpuDevice& gpuDevice, const GpuProgramCreateInformation& createInformation);
			virtual ~VulkanGpuProgram();

			/** Returns the internal shader module. */
			VulkanShaderModule* GetVulkanResource() const { return mModule; }

			/** Returns the name of the program entry point function. */
			const String& GetEntryPoint() const { return mEntryPoint; }

		protected:
			void Initialize() override;

		private:
			VulkanGpuDevice& mGpuDevice;
			VulkanShaderModule* mModule = nullptr;
		};

		/** @} */
	} // namespace render
} // namespace b3d
