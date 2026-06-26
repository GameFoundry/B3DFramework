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

		/**
		 * Constructs the device-independent vksl bytecode compiler (engine VKSL / GLSL source -> SPIR-V via glslang,
		 * with SPIRV-Cross reflection). The "vksl" suffix matches the engine shading-language id (the first field of
		 * B3D_SHADER_CROSS_COMPILE_TARGETS).
		 */
		TShared<IGpuBytecodeCompiler> CreateBytecodeCompilervksl();

		/** @} */
	} // namespace render
} // namespace b3d
