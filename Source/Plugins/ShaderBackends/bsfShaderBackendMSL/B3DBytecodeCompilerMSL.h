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

		/** Identifier of the compiler used for compiling Metal GPU programs. */
		inline constexpr const char* kMetalCompilerId = "Metal";

		/**
		 * Version of the compiler used for compiling Metal GPU programs. Tick this whenever the compiler updates in order
		 * to force bytecode to rebuild.
		 */
		inline constexpr u32 kMetalCompilerVersion = 1;

		/**
		 * Compiles Metal Shading Language into a native @c metallib and translates Metal's native function reflection into
		 * the engine's backend-independent program metadata.
		 */
		class BytecodeCompilerMSL final : public IGpuBytecodeCompiler
		{
		public:
			BytecodeCompilerMSL() = default;
			TShared<GpuProgramBytecode> CompileBytecode(const GpuProgramCreateInformation& createInformation) override;
			bool IsProgramTypeSupported(GpuProgramType type) const override
			{
				return type == GPT_VERTEX_PROGRAM || type == GPT_FRAGMENT_PROGRAM || type == GPT_COMPUTE_PROGRAM;
			}
			bool IsUpToDate(const GpuProgramBytecode& bytecode) const override;
		};

		/**
		 * Constructs the native Metal bytecode compiler (MSL -> metallib + native reflection). The
		 * factory's @c msl suffix matches the engine shading-language id (the first field of B3D_SHADER_CROSS_COMPILE_TARGETS).
		 */
		TShared<IGpuBytecodeCompiler> CreateBytecodeCompilermsl();

		/** @} */
	} // namespace render
} // namespace b3d

#endif // B3D_PLATFORM_MACOS
