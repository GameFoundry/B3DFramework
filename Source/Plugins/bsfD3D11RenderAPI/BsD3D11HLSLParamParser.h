//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsD3D11Prerequisites.h"
#include "RenderAPI/BsVertexDeclaration.h"

namespace bs
{
	namespace ct
	{
		/** @addtogroup D3D11
		 *  @{
		 */

		/**	Handles parsing of GPU program microcode and extracting constant and input parameter data. */
		class D3D11HLSLParamParser
		{
		public:
			/**
			 * Parses the provided microcode and outputs constant parameter descriptions, and optionally for vertex GPU programs
			 * a set of input parameters.
			 *
			 * @param[in]	microcode	Compiled GPU program microcode to parse.
			 * @param[in]	type		Type of the GPU program.
			 * @param[out]	desc		Output object that will contain parameter descriptions.
			 * @param[out]	inputParams	Output object that will contain a set of program input parameters. Can be null if not
			 *							required. Only relevant for vertex programs.
			 */
			void Parse(ID3DBlob* microcode, GpuProgramType type, GpuParamDesc& desc, Vector<VertexElement>* inputParams);

		private:
			/** Types of HLSL parameters. */
			enum class ParamType
			{
				ConstantBuffer,
				Texture,
				Sampler,
				UAV,
				Count // Keep at end
			};

			/**
			 * Parses the provided constant buffer retrieving information about all of its members and storing them in the
			 * provided GPU params description object.
			 */
			void ParseBuffer(ID3D11ShaderReflectionConstantBuffer* bufferReflection, GpuParamDesc& desc);

			/**	Parses the resource description structure and stores it in the provided GPU params description object. */
			void ParseResource(D3D11_SHADER_INPUT_BIND_DESC& resourceDesc, GpuProgramType type, GpuParamDesc& desc);

			/**
			 * Parses a variable with the specified type and variable description. Adds the variable in the provided GPU params
			 * description object and assigns it to the provided param block.
			 */
			void ParseVariable(D3D11_SHADER_TYPE_DESC& varTypeDesc, D3D11_SHADER_VARIABLE_DESC& varDesc, GpuParamDesc& desc, GpuParamBlockDesc& paramBlock);

			/** Maps a parameter in a specific shader stage, of a specific type to a unique set index. */
			static u32 MapParameterToSet(GpuProgramType progType, ParamType paramType);
		};

		/** @} */
	} // namespace ct
} // namespace bs
