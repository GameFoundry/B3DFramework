//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsGLPrerequisites.h"
#include "BsVertexDeclaration.h"
#include "BsDebug.h"
#include "BsException.h"
#include "BsGpuParamDesc.h"

namespace BansheeEngine
{
	/** @addtogroup GL
	 *  @{
	 */

	/**	Holds a GLSL program input attribute used in vertex programs. */
	struct GLSLAttribute
	{
		/** Constructs a new attribute from a name and a semantic that represents in which way is the attribute used. */
		GLSLAttribute(const String& name, VertexElementSemantic semantic)
		:mName(name), mSemantic(semantic)
		{ }

		/**
		 * Return true if attribute name matches the specified name and returns optional semantic index if it exists. Start
		 * of the two compared strings must match, and the remaining non-matching bit will be assumed to be the semantic
		 * index. Returns -1 if no match is made.
		 */
		INT32 matchesName(const String& name);

		/**	Returns the semantic of this attribute. */
		VertexElementSemantic getSemantic() const { return mSemantic; }

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
		void buildUniformDescriptions(GLuint glProgram, GpuProgramType type, GpuParamDesc& returnParamDesc);

		/**
		 * Parses a compiled OpenGL program and outputs vertex element list that describes input attributes to the program.
		 * Only valid for vertex programs.
		 *
		 * @param[in]	glProgram	OpenGL handle to the GPU program.
		 */
		List<VertexElement> buildVertexDeclaration(GLuint glProgram);

	private:
		/**
		 * Populates information for uniform with the specified index into the provided structure.
		 *
		 * @param[in]	desc			Output structure containing the parsed data.
		 * @param[in]	paramName		Name of the uniform.
		 * @param[in]	programHandle	Internal OpenGL handle to the GPU program.
		 * @param[in]	uniformIndex	Unique uniform index to retrieve data from. Obtained from OpenGL parsing methods.
		 */
		void determineParamInfo(GpuParamDataDesc& desc, const String& paramName, GLuint programHandle, GLuint uniformIndex);

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
		bool attribNameToElementSemantic(const String& name, VertexElementSemantic& semantic, UINT16& index);

		/**	Converts an OpenGL type to vertex element type. */
		VertexElementType glTypeToAttributeType(GLenum glType);
	};

	/** @} */
}