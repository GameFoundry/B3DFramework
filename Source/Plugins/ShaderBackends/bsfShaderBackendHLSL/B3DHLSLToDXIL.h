//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DDxcBytecodeCompiler.h"

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
		class HLSLToDXIL final : public DxcBytecodeCompiler
		{
		public:
			/**
			 * @param	compilerId		Identifier stamped into the produced bytecode.
			 * @param	compilerVersion	Version stamped into the produced bytecode, used to detect stale bytecode.
			 */
			HLSLToDXIL(const char* compilerId, u32 compilerVersion);

		protected:
			bool ReflectBytecode(const GpuProgramCreateInformation& createInformation, IDxcResult& result, IDxcBlob& program, GpuProgramBytecode& outBytecode) override;
		};

		/** @} */
	} // namespace render
} // namespace b3d
