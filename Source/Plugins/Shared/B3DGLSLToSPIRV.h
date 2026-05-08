//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DPrerequisites.h"
#include "Utility/B3DModule.h"

namespace b3d
{
	struct GpuProgramCreateInformation;
	struct GpuProgramBytecode;

	namespace render
	{
		/** @addtogroup GpuBackend-Internal
		 *  @{
		 */

		/**
		 * Compiles engine VKSL / GLSL source code into SPIR-V using glslang. Also performs SPIRV-Cross
		 * reflection on the produced module to populate parameter- and vertex-input descriptions.
		 */
		class GLSLToSPIRV : public Module<GLSLToSPIRV>
		{
		public:
			GLSLToSPIRV();
			~GLSLToSPIRV();

			/**
			 * Performs the GLSL / VKSL -> SPIR-V conversion and populates reflection info on the result.
			 *
			 * @param	createInformation	Shader source + metadata.
			 * @param	compilerId			Backend-specific compiler identifier, written into the returned bytecode.
			 * @param	compilerVersion		Backend-specific compiler version.
			 */
			TShared<GpuProgramBytecode> Convert(const GpuProgramCreateInformation& createInformation, const char* compilerId, u32 compilerVersion);
		};

		/** @} */
	} // namespace render
} // namespace b3d
