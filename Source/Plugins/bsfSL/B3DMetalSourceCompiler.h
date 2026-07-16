//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DSLPrerequisites.h"
#include "Material/B3DShaderCompiler.h"

#if B3D_PLATFORM_MACOS

namespace b3d
{
	/** Internal implementation of the HLSLCrossCompiler MSL target. */
	class MetalSourceCompiler final
	{
	public:
		/** Compiles the private VKSL intermediate into native argument-buffer MSL source. */
		static ShaderCompilerResult Compile(const String& source, GpuProgramType type, String& outSource);
	};
} // namespace b3d

#endif // B3D_PLATFORM_MACOS
