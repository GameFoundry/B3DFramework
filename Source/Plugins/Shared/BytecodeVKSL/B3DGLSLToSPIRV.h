//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DPrerequisites.h"
#include "Material/B3DShaderCompiler.h"

namespace b3d
{
	namespace render
	{
		/** @addtogroup GpuBackend-Internal
		 *  @{
		 */

		/**
		 * Compiles engine VKSL / GLSL source code into SPIR-V using glslang. Also performs SPIRV-Cross
		 * reflection on the produced module to populate parameter- and vertex-input descriptions.
		 */
		class GLSLToSPIRV final : public IGpuBytecodeCompiler
		{
		public:
			GLSLToSPIRV(const char* compilerId, u32 compilerVersion);
			~GLSLToSPIRV();

			/** Performs the GLSL / VKSL -> SPIR-V conversion and populates reflection info on the result. */
			TShared<GpuProgramBytecode> CompileBytecode(const GpuProgramCreateInformation& createInformation) override;

		private:
			const char* mCompilerId;
			u32 mCompilerVersion;
		};

		/** Identifier of the compiler used for compiling Vulkan GPU programs (stamped into the produced bytecode). */
		static constexpr const char* kVulkanCompilerId = "Vulkan";

		/**
		 * Version of the compiler used for compiling Vulkan GPU programs. Tick this whenever the compiler updates in order
		 * to force bytecode to rebuild.
		 */
		static constexpr u32 kVulkanCompilerVersion = 3;

		/** Identifier of the compiler used for compiling MoltenVK GPU programs. */
		static constexpr const char* kMoltenVkCompilerId = "MoltenVK";

		/**
		 * Version of the compiler used for compiling MoltenVK GPU programs. Tick this whenever the compiler updates in order
		 * to force bytecode to rebuild.
		 */
		static constexpr u32 kMoltenVkCompilerVersion = 1;

		/** @} */
	} // namespace render
} // namespace b3d
