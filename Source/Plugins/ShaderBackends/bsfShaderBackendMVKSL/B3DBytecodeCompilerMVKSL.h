//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DPrerequisites.h"

#if B3D_PLATFORM_MACOS

#include "Material/B3DShaderCompiler.h"

namespace b3d
{
	namespace render
	{
		/** @addtogroup GpuBackend-Internal
		 *  @{
		 */

		/** Identifier of the compiler used for compiling MoltenVK GPU programs. */
		inline constexpr const char* kMoltenVkCompilerId = "MoltenVK";

		/**
		 * Version of the compiler used for compiling MoltenVK GPU programs. Tick this whenever the compiler updates in
		 * order to force bytecode to rebuild.
		 */
		inline constexpr u32 kMoltenVkCompilerVersion = 1;

		class GLSLToSPIRV;

		/**
		 * Compiles engine MVKSL source into MoltenVK MSL bytecode: GLSL / VKSL -> SPIR-V via GLSLToSPIRV, then
		 * SPIR-V -> MSL via SPIRV-Cross.
		 */
		class BytecodeCompilerMVKSL final : public IGpuBytecodeCompiler
		{
		public:
			BytecodeCompilerMVKSL(const char* compilerId, u32 compilerVersion);
			~BytecodeCompilerMVKSL();

			TShared<GpuProgramBytecode> CompileBytecode(const GpuProgramCreateInformation& createInformation) override;
			bool IsUpToDate(const GpuProgramBytecode& bytecode) const override;

		private:
			TUnique<GLSLToSPIRV> mConverter;
		};

		/**
		 * Constructs the MoltenVK (Vulkan-on-macOS) mvksl bytecode compiler (engine MVKSL / VKSL source -> MoltenVK MSL,
		 * via SPIR-V). The "mvksl" suffix matches the engine shading-language id (the first field of B3D_SHADER_CROSS_COMPILE_TARGETS).
		 */
		TShared<IGpuBytecodeCompiler> CreateBytecodeCompilermvksl();

		/** @} */
	} // namespace render
} // namespace b3d

#endif // B3D_PLATFORM_MACOS
