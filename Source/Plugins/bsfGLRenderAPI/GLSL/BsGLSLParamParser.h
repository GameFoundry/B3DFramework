//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsGLPrerequisites.h"
#include "RenderAPI/BsVertexDeclaration.h"
#include "Debug/BsDebug.h"
#include "Error/BsException.h"
#include "RenderAPI/BsGpuParamDesc.h"

namespace bs
{
	namespace ct
	{
		/** @addtogroup GL
		 *  @{
		 */

		/**	Holds a GLSL program input attribute used in vertex programs. */
		struct GLSLAttribute
		{
			/** Constructs a new attribute from a name and a semantic that represents in which way is the attribute used. */
			GLSLAttribute(const String& name, VertexElementSemantic semantic)
				: mName(name), mSemantic(semantic)
			{}

			/**
			 * Return true if attribute name matches the specified name and returns optional semantic index if it exists. Start
			 * of the two compared strings must match, and the remaining non-matching bit will be assumed to be the semantic
			 * index. Returns -1 if no match is made.
			 */
			i32 MatchesName(const String& name);

			/**	Returns the semantic of this attribute. */
			VertexElementSemantic GetSemantic() const { return mSemantic; }

		private:
			String mName;
			VertexElementSemantic mSemantic;
		};

		/** Helper class that is able to parse a GLSL GPU program and retrieve used uniforms and input attributes. */
		class GLSLParamParser
		{
		public:
			/**
			 * Parses a compiled OpenGL program and outputs a parameter description that contains information about used
			 * uniforms.
			 *
			 * @param[in]	glProgram		OpenGL handle to the GPU program.
			 * @param[in]	type			Type of the GPU program we're parsing.
			 * @param[out]	returnParamDesc	Output structure containing the parsed data.
			 */
			void BuildUniformDescriptions(GLuint glProgram, GpuProgramType type, GpuParamDesc& returnParamDesc);

			/**
			 * Parses a compiled OpenGL program and outputs vertex element list that describes input attributes to the program.
			 * Only valid for vertex programs.
			 *
			 * @param[in]	glProgram	OpenGL handle to the GPU program.
			 */
			Vector<VertexElement> BuildVertexDeclaration(GLuint glProgram);

			/**
			 * Calculates the size and alignment of a single element within a shader interface block using the std140 layout.
			 *
			 * @param[in]		type		Type of the element. Structs are not supported.
			 * @param[in]		arraySize	Number of array elements of the element (1 if it's not an array).
			 * @param[in, out]	offset		Current location in some parent buffer at which the element should be placed at. If the
			 *								location doesn't match the element's alignment, the value will be modified to a valid
			 *								alignment. In multiples of 4 bytes.
			 * @return						Size of the element, in multiples of 4 bytes.
			 */
			static u32 CalcInterfaceBlockElementSizeAndOffset(GpuParamDataType type, u32 arraySize, u32& offset);

		private:
			/** Types of HLSL parameters. */
			enum class ParamType
			{
				UniformBlock,
				Texture,
				Sampler,
				Image,
				StorageBlock,
				Count // Keep at end
			};

			/**
			 * Populates information for uniform with the specified index into the provided structure.
			 *
			 * @param[in]	desc			Output structure containing the parsed data.
			 * @param[in]	paramName		Name of the uniform.
			 * @param[in]	programHandle	Internal OpenGL handle to the GPU program.
			 * @param[in]	uniformIndex	Unique uniform index to retrieve data from. Obtained from OpenGL parsing methods.
			 */
			void DetermineParamInfo(GpuParamDataDesc& desc, const String& paramName, GLuint programHandle, GLuint uniformIndex);

			/**
			 * Attempts to find out a vertex element semantic based on input parameter name. GLSL has no concept of semantics,
			 * so we require all shaders to use specific names for attributes so that we know what they are used for.
			 *
			 * Valid names and semantics:
			 *  bs_position - VES_POSITION
			 *  bs_normal - VES_NORMAL
			 *  bs_tangent - VES_TANGENT
			 *  bs_bitangent - VES_BITANGENT
			 *  bs_texcoord - VES_TEXCOORD
			 *  bs_color - VES_COLOR
			 *  bs_blendweights - VES_BLEND_WEIGHTS
			 *  bs_blendindices - VES_BLEND_INDICES
			 *
			 * You may append a number to the end of the name to specify semantic index.
			 *
			 * @return	True if it succeeds, false if it fails.
			 */
			bool AttribNameToElementSemantic(const String& name, VertexElementSemantic& semantic, u16& index);

			/**	Converts an OpenGL type to vertex element type. */
			VertexElementType GlTypeToAttributeType(GLenum glType);

			/** Maps a parameter in a specific shader stage, of a specific type to a unique set index. */
			static u32 MapParameterToSet(GpuProgramType progType, ParamType paramType);
		};

		/** @} */
	} // namespace ct
} // namespace bs
