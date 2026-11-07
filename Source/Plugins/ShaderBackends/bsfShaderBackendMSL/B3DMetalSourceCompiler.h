//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DPrerequisites.h"

#if B3D_PLATFORM_MACOS

#include "Material/B3DShaderCompiler.h"
#include "Material/B3DShaderReflection.h"

namespace b3d
{
	/**
	 * Translates HLSL generated for the Vulkan target into Metal Shading Language (MSL), by way of SPIR-V.
	 *
	 * Generated resource names encode the engine binding coordinates required by native Metal reflection.
	 */
	class MetalSourceCompiler final
	{
	public:
		/**
		 * Compiles an HLSL GPU program into MSL that uses the native Metal argument-buffer ABI.
		 *
		 * @param	hlslSource				HLSL source containing a single GPU program, using the entry-point name reserved for its stage.
		 * @param	programType				Stage represented by @p hlslSource. Vertex, fragment, and compute stages are supported.
		 * @param	shaderReflection		Source metadata for @p hlslSource.
		 * @param	outMslSource			Receives the generated MSL source on success; cleared on failure.
		 * @return							Compilation result. A non-empty error message indicates failure.
		 */
		static ShaderCompilerResult Compile(const String& hlslSource, GpuProgramType programType, const TShared<ShaderReflection>& shaderReflection, String& outMslSource);
	};
} // namespace b3d

#endif // B3D_PLATFORM_MACOS
