//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DPrerequisites.h"

#if B3D_PLATFORM_MACOS

#include "Material/B3DShaderCompiler.h"

namespace b3d
{
	/**
	 * Translates the engine's private Vulkan-style shading language (VKSL) into Metal Shading Language (MSL).
	 *
	 * Generated resource names encode the engine binding coordinates required by native Metal reflection.
	 */
	class MetalSourceCompiler final
	{
	public:
		/**
		 * Compiles a VKSL GPU program into MSL that uses the native Metal argument-buffer ABI.
		 *
		 * @param	vkslSource				VKSL source containing a single GPU program with a "main" entry point.
		 * @param	programType				Stage represented by @p vkslSource. Vertex, fragment, and compute stages are supported.
		 * @param	pushConstantBufferSize	Push-constant block size in bytes, or zero when unused.
		 * @param	outMslSource			Receives the generated MSL source on success; cleared on failure.
		 * @return							Compilation result. A non-empty error message indicates failure.
		 */
		static ShaderCompilerResult Compile(const String& vkslSource, GpuProgramType programType, u32 pushConstantBufferSize, String& outMslSource);
	};
} // namespace b3d

#endif // B3D_PLATFORM_MACOS
