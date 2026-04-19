//************************************ B3D Framework - Copyright 2026 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DVulkanPrerequisites.h"
#include "B3DVulkanResource.h"
#include "RenderAPI/B3DGpuProgram.h"

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

		/** Identifier of the compiler used for compiling Vulkan GPU programs. */
		static constexpr const char* VULKAN_COMPILER_ID = "Vulkan";

		/**
		 * Version of the compiler used for compiling Vulkan GPU programs. Tick this whenever the compiler updates in order
		 * to force bytecode to rebuild.
		 */
		static constexpr u32 VULKAN_COMPILER_VERSION = 1;

		/** Identifier of the compiler used for compiling MoltenVK GPU programs. */
		static constexpr const char* MOLTENVK_COMPILER_ID = "MoltenVK";

		/**
		 * Version of the compiler used for compiling MoltenVK GPU programs. Tick this whenever the compiler updates in order
		 * to force bytecode to rebuild.
		 */
		static constexpr u32 MOLTENVK_COMPILER_VERSION = 1;

		/** @} */
	} // namespace render
} // namespace b3d
