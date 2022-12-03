//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsVulkanPrerequisites.h"
#include "BsVulkanResource.h"
#include "RenderAPI/BsGpuProgram.h"

namespace bs
{
	namespace ct
	{
		/** @addtogroup Vulkan
		 *  @{
		 */

		/** Wrapper around a Vulkan shader module (GPU program) that manages its usage and lifetime. */
		class VulkanShaderModule : public VulkanResource
		{
		public:
			VulkanShaderModule(VulkanResourceManager* owner, VkShaderModule module);
			~VulkanShaderModule();

			/** Returns the internal handle to the Vulkan object. */
			VkShaderModule GetHandle() const { return mModule; }

			/** Assigns an name to the shader module, primarily used for easier debugging. */
			void SetName(const StringView& name);

		private:
			VkShaderModule mModule;
		};

		/**	Abstraction of a Vulkan shader object. */
		class VulkanGpuProgram : public GpuProgram
		{
		public:
			virtual ~VulkanGpuProgram();

			void SetName(const StringView& name) override;

			/**
			 * Returns the shader module for the specified device. If program device mask doesn't include the provided device,
			 * null is returned.
			 */
			VulkanShaderModule* GetShaderModule(u32 deviceIdx) const { return mModules[deviceIdx]; }

			/** Returns the name of the program entry point function. */
			const String& GetEntryPoint() const { return mEntryPoint; }

		protected:
			friend class VulkanGLSLProgramFactory;

			VulkanGpuProgram(const GpuProgramCreateInformation& desc, GpuDeviceFlags deviceMask);

			void Initialize() override;

		private:
			GpuDeviceFlags mDeviceMask;
			VulkanShaderModule* mModules[B3D_MAX_DEVICES];
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
	} // namespace ct
} // namespace bs
