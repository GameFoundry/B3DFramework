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
		 * Compiles engine HLSL source code into DXBC using the D3D shader compiler (fxc). Also reflects the produced
		 * bytecode through the D3D reflection interface to populate parameter- and vertex-input descriptions, with
		 * parameter slots encoded per B3DHLSLShaderABI.h.
		 */
		class HLSLToDXBC final : public IGpuBytecodeCompiler
		{
		public:
			HLSLToDXBC(const char* compilerId, u32 compilerVersion);

			TShared<GpuProgramBytecode> CompileBytecode(const GpuProgramCreateInformation& createInformation) override;
			bool IsUpToDate(const GpuProgramBytecode& bytecode) const override;

		private:
			const char* mCompilerId;
			u32 mCompilerVersion;
		};

		/** @} */
	} // namespace render
} // namespace b3d
