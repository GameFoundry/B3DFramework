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

			TShared<GpuProgramBytecode> CompileBytecode(const GpuProgramCreateInformation& createInformation) override;
			bool IsUpToDate(const GpuProgramBytecode& bytecode) const override;

		private:
			const char* mCompilerId;
			u32 mCompilerVersion;
		};

		/** @} */
	} // namespace render
} // namespace b3d
