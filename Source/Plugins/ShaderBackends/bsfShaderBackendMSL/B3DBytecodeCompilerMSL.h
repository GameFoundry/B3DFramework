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

		class GLSLToSPIRV;

		/**
		 * Bytecode compiler for the native Metal backend's @c msl language. Cross-compiles engine VKSL source to
		 * argument-buffer Metal Shading Language in two steps: GLSL/VKSL -> SPIR-V (via the owned GLSLToSPIRV) followed by
		 * SPIR-V -> MSL (via SPIRV-Cross), packing the result into the WriteMetalBytecode blob layout that MetalGpuProgram
		 * consumes. Device-independent, so it also runs in the headless shader-cook tool.
		 */
		class BytecodeCompilerMSL final : public IGpuBytecodeCompiler
		{
		public:
			BytecodeCompilerMSL(const char* compilerId, u32 compilerVersion);
			~BytecodeCompilerMSL();

			/** Performs the VKSL -> SPIR-V -> argument-buffer MSL conversion and packs the result. */
			TShared<GpuProgramBytecode> CompileBytecode(const GpuProgramCreateInformation& createInformation) override;

		private:
			TUnique<GLSLToSPIRV> mConverter;
		};

		/**
		 * Constructs the native Metal bytecode compiler (engine VKSL source -> argument-buffer MSL, via SPIR-V). The
		 * factory's @c msl suffix matches the engine shading-language id (the first field of B3D_SHADER_CROSS_COMPILE_TARGETS).
		 */
		TShared<IGpuBytecodeCompiler> CreateBytecodeCompilermsl();

		/** @} */
	} // namespace render
} // namespace b3d

#endif // B3D_PLATFORM_MACOS
