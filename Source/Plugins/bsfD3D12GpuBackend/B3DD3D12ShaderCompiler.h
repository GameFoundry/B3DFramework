//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DD3D12Prerequisites.h"
#include "GpuBackend/B3DGpuProgram.h"

namespace b3d
{
	namespace render
	{
		/** @addtogroup D3D12GpuBackend
		 *  @{
		 */

		/** Utility class for compiling HLSL shaders and performing reflection. */
		class D3D12ShaderCompiler
		{
		public:
			/**
			 * Compiles HLSL source code to D3D12 bytecode and populates reflection information.
			 *
			 * @param desc			Description of the shader to compile.
			 * @param bytecode		Output bytecode object that will be populated with compiled shader and reflection data.
			 * @return				True if compilation was successful.
			 */
			static bool CompileShader(const GpuProgramCreateInformation& desc, TShared<GpuProgramBytecode>& bytecode);

		private:
			/** Performs shader reflection to extract parameter descriptions and vertex inputs. */
			static void ReflectShader(ID3DBlob* shaderBlob, GpuProgramType type, GpuProgramBytecode& bytecode);

			/** Reflects constant buffers and their members. */
			static void ReflectConstantBuffers(ID3D12ShaderReflection* reflection, const D3D12_SHADER_DESC& shaderDesc,
				GpuProgramParameterDescription& paramDesc);

			/** Reflects bound resources (textures, samplers, UAVs, etc.). */
			static void ReflectBoundResources(ID3D12ShaderReflection* reflection, const D3D12_SHADER_DESC& shaderDesc,
				GpuProgramParameterDescription& paramDesc);

			/** Reflects vertex input attributes for vertex shaders. */
			static void ReflectVertexInput(ID3D12ShaderReflection* reflection, const D3D12_SHADER_DESC& shaderDesc,
				Vector<VertexElement>& vertexInput);

			/** Converts D3D12 shader type to GpuParamDataType. */
			static GpuParamDataType ConvertD3DTypeToGpuParamDataType(const D3D12_SHADER_TYPE_DESC& typeDesc);

			/** Parses HLSL semantic name (e.g., "POSITION0") to semantic type and index. */
			static bool ParseSemanticName(const char* semanticName, VertexElementSemantic& semantic, u16& index);

			/** Converts D3D12 signature parameter to vertex element type. */
			static VertexElementType ConvertD3DSignatureToVertexType(const D3D12_SIGNATURE_PARAMETER_DESC& desc);

			/** Converts GPU program type to HLSL shader model target. */
			static const char* GetShaderTarget(GpuProgramType type);
		};

		/** @} */
	} // namespace render
} // namespace b3d
