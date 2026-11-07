//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DPrerequisites.h"
#include "Material/B3DShaderCompiler.h"
#if B3D_PLATFORM_WIN32
#include <Windows.h>
#include <wrl/client.h>
#endif
#include <dxcapi.h>

namespace b3d
{
	namespace render
	{
		/** @addtogroup GpuBackend-Internal
		 *  @{
		 */

#if B3D_PLATFORM_WIN32
		template <class T>
		using TDxcPointer = Microsoft::WRL::ComPtr<T>;
#else
		template <class T>
		using TDxcPointer = CComPtr<T>;
#endif

		/**
		 * Compiles engine HLSL source code using the DirectX Shader Compiler. Implementations choose the output format
		 * through additional compiler arguments, and reflect the program the compiler produces.
		 *
		 * @note	This file is compiled into every shader backend that uses it, and those backends can link into the same
		 *			binary. Its code must therefore not vary per backend, as the linker keeps only one of the copies.
		 */
		class DxcBytecodeCompiler : public IGpuBytecodeCompiler
		{
		public:
			TShared<GpuProgramBytecode> CompileBytecode(const GpuProgramCreateInformation& createInformation) final;
			bool IsUpToDate(const GpuProgramBytecode& bytecode) const final;

		protected:
			/**
			 * @param	compilerId		Identifier stored in produced bytecode, used to detect stale cached output.
			 * @param	compilerVersion	Version stored in produced bytecode, used to detect stale cached output.
			 */
			DxcBytecodeCompiler(const char* compilerId, u32 compilerVersion);

			/**
			 * Appends the arguments specific to this output format, if any, to the ones all compilations share.
			 *
			 * @param	outArguments	List of compiler arguments to append to.
			 */
			virtual void AddCompileArguments(Vector<WString>& outArguments) const { }

			/**
			 * Fills in the parameter descriptions, vertex inputs and other reflected data of a successfully compiled program.
			 *
			 * @param	createInformation	GPU program source and metadata the program was compiled from.
			 * @param	result				Result of the compilation.
			 * @param	program				Compiled program.
			 * @param	outBytecode			Bytecode to fill in. On failure its messages should describe the error.
			 * @return						True if successful, false if the program should not be used.
			 */
			virtual bool ReflectBytecode(const GpuProgramCreateInformation& createInformation, IDxcResult& result, IDxcBlob& program, GpuProgramBytecode& outBytecode) = 0;

			TDxcPointer<IDxcUtils> mUtilities;

		private:
			const char* mCompilerId;
			u32 mCompilerVersion;
			TDxcPointer<IDxcCompiler3> mCompiler;
		};

		/** @} */
	} // namespace render
} // namespace b3d
