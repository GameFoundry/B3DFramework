//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DPrerequisites.h"

namespace b3d
{
	class IGpuBytecodeCompiler;

	namespace render
	{
		/** @addtogroup GpuBackend-Internal
		 *  @{
		 */

		/** Identifier of the compiler used for compiling HLSL GPU programs (stamped into the produced bytecode). */
		inline constexpr const char* kHlslCompilerId = "HLSL_FXC";

		/**
		 * Version of the compiler used for compiling HLSL GPU programs. Tick this whenever the compiler updates in order
		 * to force bytecode to rebuild.
		 */
		inline constexpr u32 kHlslCompilerVersion = 2;

		/**
		 * Constructs the device-independent hlsl bytecode compiler (engine HLSL source -> DXBC via the D3D shader
		 * compiler, with D3D reflection). The "hlsl" suffix matches the engine shading-language id (the first field of
		 * B3D_SHADER_CROSS_COMPILE_TARGETS).
		 */
		TShared<IGpuBytecodeCompiler> CreateBytecodeCompilerhlsl();

		/** @} */
	} // namespace render
} // namespace b3d
