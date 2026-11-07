//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DDxcBytecodeCompiler.h"

#include "spirv_cross/spirv_cross_parsed_ir.hpp"

namespace b3d
{
	namespace render
	{
		/** @addtogroup GpuBackend-Internal
		 *  @{
		 */

		/**
		 * Compiles engine HLSL source code into SPIR-V using the DirectX Shader Compiler. Also performs SPIRV-Cross
		 * reflection on the produced module to populate parameter and vertex-input descriptions. The module keeps every
		 * resource the source declares, even the ones its stage never reads.
		 */
		class HLSLToSPIRV final : public DxcBytecodeCompiler
		{
		public:
			/**
			 * @param	compilerId		Identifier stored in produced bytecode, used to detect stale cached output.
			 * @param	compilerVersion	Version stored in produced bytecode, used to detect stale cached output.
			 */
			HLSLToSPIRV(const char* compilerId, u32 compilerVersion);

			/**
			 * Parses a compiled module for SPIRV-Cross so reflection reports every resource the module declares, because
			 * both SPIR-V and DXC strip (or don't report) unused resources.
			 *
			 * @param	words		Contents of the SPIR-V module.
			 * @param	wordCount	Number of 32-bit words in @p words.
			 * @return				Module to construct a SPIRV-Cross compiler from.
			 */
			static spirv_cross::ParsedIR ParseModule(const u32* words, size_t wordCount);

		protected:
			void AddCompileArguments(Vector<WString>& outArguments) const override;
			bool ReflectBytecode(const GpuProgramCreateInformation& createInformation, IDxcResult& result, IDxcBlob& program, GpuProgramBytecode& outBytecode) override;
		};

		/** @} */
	} // namespace render
} // namespace b3d
