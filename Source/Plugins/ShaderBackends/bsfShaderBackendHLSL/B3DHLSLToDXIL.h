//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DPrerequisites.h"
#include "Material/B3DShaderCompiler.h"
#include <Windows.h>
#include <dxcapi.h>
#include <wrl/client.h>

namespace b3d
{
	namespace render
	{
		/** @addtogroup GpuBackend-Internal
		 *  @{
		 */

		/**
		 * Compiles engine HLSL source code into DXIL using the DirectX Shader Compiler. Also reflects the produced
		 * bytecode through the D3D reflection interface to populate parameter and vertex-input descriptions, with
		 * parameter slots encoded per B3DHLSLShaderABI.h.
		 */
		class HLSLToDXIL final : public IGpuBytecodeCompiler
		{
		public:
			/**
			 * @param	compilerId		Identifier stamped into the produced bytecode.
			 * @param	compilerVersion	Version stamped into the produced bytecode, used to detect stale bytecode.
			 */
			HLSLToDXIL(const char* compilerId, u32 compilerVersion);

			TShared<GpuProgramBytecode> CompileBytecode(const GpuProgramCreateInformation& createInformation) override;
			bool IsUpToDate(const GpuProgramBytecode& bytecode) const override;

		private:
			const char* mCompilerId;
			u32 mCompilerVersion;
			Microsoft::WRL::ComPtr<IDxcUtils> mUtilities;
			Microsoft::WRL::ComPtr<IDxcCompiler3> mCompiler;
		};

		/** @} */
	} // namespace render
} // namespace b3d
