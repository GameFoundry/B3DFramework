//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "GLSL/BsGLSLParamParser.h"

namespace bs { namespace ct
{
	INT32 GLSLAttribute::matchesName(const String& name)
	{
		if (name.length() >= mName.length())
		{
			if (name.substr(0, mName.length()) == mName)
			{
				String indexStr = name.substr(mName.length(), name.length());
				return parseUINT32(indexStr, 0);
			}
		}

		return -1;
	}

	List<VertexElement> GLSLParamParser::buildVertexDeclaration(GLuint glProgram)
	{
		GLint numAttributes = 0;
		glGetProgramiv(glProgram, GL_ACTIVE_ATTRIBUTES, &numAttributes);
		BS_CHECK_GL_ERROR();

		GLint maxNameSize = 0;
		glGetProgramiv(glProgram, GL_ACTIVE_ATTRIBUTE_MAX_LENGTH, &maxNameSize);
		BS_CHECK_GL_ERROR();

		GLchar* attributeName = (GLchar*)bs_alloc(sizeof(GLchar)* maxNameSize);

		List<VertexElement> elementList;
		for (GLint i = 0; i < numAttributes; i++)
		{
			GLint attribSize = 0;
			GLenum attribType = 0;
			glGetActiveAttrib(glProgram, i, maxNameSize, nullptr, &attribSize, &attribType, attributeName);
			BS_CHECK_GL_ERROR();

			VertexElementSemantic semantic = VES_POSITION;
			UINT16 index = 0;
			if (attribNameToElementSemantic(attributeName, semantic, index))
			{
				VertexElementType type = glTypeToAttributeType(attribType);
				UINT32 slot = glGetAttribLocation(glProgram, attributeName);
				BS_CHECK_GL_ERROR();

				elementList.push_back(VertexElement(0, slot, type, semantic, index));
			}
			else
			{
				// Ignore built-in attributes
				if(memcmp(attributeName, "gl_", 3) != 0)
					LOGWRN("Cannot determine vertex input attribute type for attribute: " + String(attributeName));
			}
		}

		bs_free(attributeName);

		return elementList;
	}

	VertexElementType GLSLParamParser::glTypeToAttributeType(GLenum glType)
	{
		switch (glType)
		{
		case GL_FLOAT:
			return VET_FLOAT1;
		case GL_FLOAT_VEC2:
			return VET_FLOAT2;
		case GL_FLOAT_VEC3:
			return VET_FLOAT3;
		case GL_FLOAT_VEC4:
			return VET_FLOAT4;
		case GL_INT:
			return VET_INT1;
		case GL_INT_VEC2:
			return VET_INT2;
		case GL_INT_VEC3:
			return VET_INT3;
		case GL_INT_VEC4:
			return VET_INT4;
		case GL_UNSIGNED_INT:
			return VET_UINT1;
		case GL_UNSIGNED_INT_VEC2:
			return VET_UINT2;
		case GL_UNSIGNED_INT_VEC3:
			return VET_UINT3;
		case GL_UNSIGNED_INT_VEC4:
			return VET_UINT4;
		default:
			BS_EXCEPT(NotImplementedException, "Unsupported vertex attribute type.");
		}

		return VET_FLOAT4;
	}

	bool GLSLParamParser::attribNameToElementSemantic(const String& name, VertexElementSemantic& semantic, UINT16& index)
	{
		static GLSLAttribute attributes[] =
		{
			GLSLAttribute("bs_position", VES_POSITION),
			GLSLAttribute("bs_normal", VES_NORMAL),
			GLSLAttribute("bs_tangent", VES_TANGENT),
			GLSLAttribute("bs_bitangent", VES_BITANGENT),
			GLSLAttribute("bs_texcoord", VES_TEXCOORD),
			GLSLAttribute("bs_color", VES_COLOR),
			GLSLAttribute("bs_blendweights", VES_BLEND_WEIGHTS),
			GLSLAttribute("bs_blendindices", VES_BLEND_INDICES),
			GLSLAttribute("POSITION", VES_POSITION),
			GLSLAttribute("NORMAL", VES_NORMAL),
			GLSLAttribute("TANGENT", VES_TANGENT),
			GLSLAttribute("BITANGENT", VES_BITANGENT),
			GLSLAttribute("TEXCOORD", VES_TEXCOORD),
			GLSLAttribute("COLOR", VES_COLOR),
			GLSLAttribute("BLENDWEIGHT", VES_BLEND_WEIGHTS),
			GLSLAttribute("BLENDINDICES", VES_BLEND_INDICES)
		};

		static const UINT32 numAttribs = sizeof(attributes) / sizeof(attributes[0]);

		for (UINT32 i = 0; i < numAttribs; i++)
		{
			INT32 attribIndex = attributes[i].matchesName(name);
			if (attribIndex != -1)
			{
				index = attribIndex;
				semantic = attributes[i].getSemantic();
				return true;
			}
		}

		return false;
	}

	void GLSLParamParser::buildUniformDescriptions(GLuint glProgram, GpuProgramType type, GpuParamDesc& returnParamDesc)
	{
		// Scan through the active uniform blocks
		GLint maxBufferSize = 0;
		glGetProgramiv(glProgram, GL_ACTIVE_UNIFORM_MAX_LENGTH, &maxBufferSize);
		BS_CHECK_GL_ERROR();

		GLint maxBlockNameBufferSize = 0;
		glGetProgramiv(glProgram, GL_ACTIVE_UNIFORM_BLOCK_MAX_NAME_LENGTH, &maxBlockNameBufferSize);
		BS_CHECK_GL_ERROR();

		GLint maxStorageBlockNameBufferSize = 0;

#if BS_OPENGL_4_3 || BS_OPENGLES_3_1
		glGetProgramInterfaceiv(glProgram, GL_SHADER_STORAGE_BLOCK, GL_MAX_NAME_LENGTH, &maxStorageBlockNameBufferSize);
		BS_CHECK_GL_ERROR();
#endif

		maxBufferSize = std::max(maxBufferSize, maxBlockNameBufferSize);
		maxBufferSize = std::max(maxBufferSize, maxStorageBlockNameBufferSize);

		GLchar* uniformName = (GLchar*)bs_alloc(sizeof(GLchar)* maxBufferSize);

		GpuParamBlockDesc newGlobalBlockDesc;
		newGlobalBlockDesc.slot = 0;
		newGlobalBlockDesc.set = mapParameterToSet(type, ParamType::UniformBlock);
		newGlobalBlockDesc.name = "BS_INTERNAL_Globals";
		newGlobalBlockDesc.blockSize = 0;
		newGlobalBlockDesc.isShareable = false;

		returnParamDesc.paramBlocks[newGlobalBlockDesc.name] = newGlobalBlockDesc;
		GpuParamBlockDesc& globalBlockDesc = returnParamDesc.paramBlocks[newGlobalBlockDesc.name];

		// Enumerate uniform blocks
		GLint uniformBlockCount = 0;

#if BS_OPENGL_4_3 || BS_OPENGLES_3_1
		// Use program interface extension if available
		glGetProgramInterfaceiv(glProgram, GL_UNIFORM_BLOCK, GL_ACTIVE_RESOURCES, &uniformBlockCount);
		BS_CHECK_GL_ERROR();
#else
		// Fall back to old API if not available
		glGetProgramiv(glProgram, GL_ACTIVE_UNIFORM_BLOCKS, &uniformBlockCount);
		BS_CHECK_GL_ERROR();
#endif

		Map<UINT32, String> blockSlotToName;
		Set<String> blockNames;
		for (GLuint index = 0; index < (GLuint)uniformBlockCount; index++)
		{
			GLsizei unusedSize = 0;

#if BS_OPENGL_4_3 || BS_OPENGLES_3_1
			glGetProgramResourceName(glProgram, GL_UNIFORM_BLOCK, index, maxBufferSize, &unusedSize, uniformName);
			BS_CHECK_GL_ERROR();
#else
			glGetActiveUniformBlockName(glProgram, index, maxBlockNameBufferSize, &unusedSize, uniformName);
			BS_CHECK_GL_ERROR();
#endif

			GpuParamBlockDesc newBlockDesc;
			newBlockDesc.slot = index + 1;
			newBlockDesc.set = mapParameterToSet(type, ParamType::UniformBlock);
			newBlockDesc.name = uniformName;
			newBlockDesc.blockSize = 0;
			newBlockDesc.isShareable = true;

			returnParamDesc.paramBlocks[newBlockDesc.name] = newBlockDesc;
			blockSlotToName.insert(std::make_pair(index + 1, newBlockDesc.name));
			blockNames.insert(newBlockDesc.name);
		}

#if BS_OPENGL_4_3 || BS_OPENGLES_3_1
		// Scan through the shared storage blocks
		GLint storageBlockCount = 0;
		glGetProgramInterfaceiv(glProgram, GL_SHADER_STORAGE_BLOCK, GL_ACTIVE_RESOURCES, &storageBlockCount);
		BS_CHECK_GL_ERROR();

		for (GLuint index = 0; index < (GLuint)storageBlockCount; index++)
		{
			GLsizei unusedSize = 0;
			glGetProgramResourceName(glProgram, GL_SHADER_STORAGE_BLOCK, index, maxBufferSize, &unusedSize, uniformName);
			BS_CHECK_GL_ERROR();

			GpuParamObjectDesc bufferParam;
			bufferParam.name = uniformName;
			bufferParam.slot = index;
			bufferParam.type = GPOT_RWSTRUCTURED_BUFFER;
			bufferParam.set = mapParameterToSet(type, ParamType::StorageBlock);

			returnParamDesc.buffers.insert(std::make_pair(uniformName, bufferParam));
		}
#endif

		Map<String, UINT32> foundFirstArrayIndex;
		Map<String, GpuParamDataDesc> foundStructs;

		// Get the number of active uniforms
		GLint uniformCount = 0;
		glGetProgramiv(glProgram, GL_ACTIVE_UNIFORMS, &uniformCount);
		BS_CHECK_GL_ERROR();

		// Loop over each of the active uniforms, and add them to the reference container
		// only do this for user defined uniforms, ignore built in gl state uniforms
		for (GLuint index = 0; index < (GLuint)uniformCount; index++)
		{
			GLsizei arraySize = 0;
			glGetActiveUniformName(glProgram, index, maxBufferSize, &arraySize, uniformName);
			BS_CHECK_GL_ERROR();

			String paramName = String(uniformName);

			// Naming rules and packing rules used here are described in
			// OpenGL Core Specification 2.11.4

			// Check if parameter is a part of a struct
			Vector<String> nameElements = StringUtil::tokenise(paramName, ".");

			bool inStruct = false;
			String structName;
			if (nameElements.size() > 1)
			{
				auto uniformBlockFind = blockNames.find(nameElements[0]);

				// Check if the name is not a struct, and instead a Uniform block namespace
				if (uniformBlockFind != blockNames.end())
				{
					// Possibly it's a struct inside a named uniform block
					if (nameElements.size() > 2)
					{
						inStruct = true;
						structName = nameElements[1];
					}
				}
				else
				{
					inStruct = true;
					structName = nameElements[0];
				}
			}

			String cleanParamName = paramName; // Param name without array indexes

			// Check if the parameter is in an array
			UINT32 arrayIdx = 0;
			bool isInArray = false;
			if (inStruct)
			{
				// If the uniform name has a "[" in it then its an array element uniform.
				String::size_type arrayStart = structName.find("[");
				String::size_type arrayEnd = structName.find("]");
				if (arrayStart != String::npos)
				{
					String strArrIdx = structName.substr(arrayStart + 1, arrayEnd - (arrayStart + 1));
					arrayIdx = parseUINT32(strArrIdx, 0);
					isInArray = true;

					structName = structName.substr(0, arrayStart);
				}
			}
			else
			{
				// If the uniform name has a "[" in it then its an array element uniform.
				String::size_type arrayStart = cleanParamName.find("[");
				String::size_type arrayEnd = cleanParamName.find("]");
				if (arrayStart != String::npos)
				{
					String strArrIdx = cleanParamName.substr(arrayStart + 1, arrayEnd - (arrayStart + 1));
					arrayIdx = parseUINT32(strArrIdx, 0);
					isInArray = true;

					cleanParamName = cleanParamName.substr(0, arrayStart);
				}
			}

			if (inStruct)
			{
				// OpenGL makes struct management really difficult, which is why I have given up on implementing this so far
				// Some of the issues I encountered:
				//  - Elements will be optimized out if they are not used. This makes it hard to determine proper structure size.
				//     - If struct is within a Uniform buffer block, then it is possible because the element won't be optimized out of the buffer
				//     - If the struct is within a global buffer, it is impossible to determine actual size, since the element will be optimized out of the buffer too
				//     - Same issue happens with arrays, as OpenGL will optimize out array elements. With global buffers this makes it impossible to determine
				//       actual array size (for example suppose OpenGL optimized out few last elements)
				//        - Normal arrays work fine as OpenGL has utilities for reporting their actual size, but those do not work with structs

				BS_EXCEPT(NotImplementedException, "Structs are not supported.")
			}

			// GLSL will optimize out unused array indexes, so there's no guarantee that 0 is the first,
			// so we store the first one here
			int firstArrayIndex = 0;
			if (isInArray)
			{
				String& nameToSearch = cleanParamName;
				if (inStruct)
					nameToSearch = structName;

				auto arrayIndexFind = foundFirstArrayIndex.find(nameToSearch);
				if (arrayIndexFind == foundFirstArrayIndex.end())
				{
					foundFirstArrayIndex[nameToSearch] = arrayIdx;
				}

				firstArrayIndex = foundFirstArrayIndex[nameToSearch];
			}


			GLint uniformType;
			glGetActiveUniformsiv(glProgram, 1, &index, GL_UNIFORM_TYPE, &uniformType);
			BS_CHECK_GL_ERROR();

			GpuParamObjectType samplerType = GPOT_UNKNOWN;
			GpuParamObjectType textureType = GPOT_UNKNOWN;

			bool isSampler = false;
			bool isImage = false;
			bool isBuffer = false;
			bool isRWBuffer = false;
			switch (uniformType)
			{
			case GL_SAMPLER_1D:
			case GL_SAMPLER_1D_SHADOW:
			case GL_UNSIGNED_INT_SAMPLER_1D:
			case GL_INT_SAMPLER_1D:
				samplerType = GPOT_SAMPLER1D;
				textureType = GPOT_TEXTURE1D;
				isSampler = true;
				break;
			case GL_SAMPLER_1D_ARRAY:
			case GL_SAMPLER_1D_ARRAY_SHADOW:
			case GL_UNSIGNED_INT_SAMPLER_1D_ARRAY:
			case GL_INT_SAMPLER_1D_ARRAY:
				samplerType = GPOT_SAMPLER1D;
				textureType = GPOT_TEXTURE1DARRAY;
				isSampler = true;
				break;
			case GL_SAMPLER_2D:
			case GL_SAMPLER_2D_SHADOW:
			case GL_UNSIGNED_INT_SAMPLER_2D:
			case GL_INT_SAMPLER_2D:
				samplerType = GPOT_SAMPLER2D;
				textureType = GPOT_TEXTURE2D;
				isSampler = true;
				break;
			case GL_SAMPLER_2D_ARRAY:
			case GL_SAMPLER_2D_ARRAY_SHADOW:
			case GL_UNSIGNED_INT_SAMPLER_2D_ARRAY:
			case GL_INT_SAMPLER_2D_ARRAY:
				samplerType = GPOT_SAMPLER2D;
				textureType = GPOT_TEXTURE2DARRAY;
				isSampler = true;
				break;
			case GL_SAMPLER_2D_MULTISAMPLE:
			case GL_UNSIGNED_INT_SAMPLER_2D_MULTISAMPLE:
			case GL_INT_SAMPLER_2D_MULTISAMPLE:
				samplerType = GPOT_SAMPLER2DMS;
				textureType = GPOT_TEXTURE2DMS;
				isSampler = true;
				break;
			case GL_SAMPLER_2D_MULTISAMPLE_ARRAY:
			case GL_UNSIGNED_INT_SAMPLER_2D_MULTISAMPLE_ARRAY:
			case GL_INT_SAMPLER_2D_MULTISAMPLE_ARRAY:
				samplerType = GPOT_SAMPLER2DMS;
				textureType = GPOT_TEXTURE2DMSARRAY;
				isSampler = true;
				break;
			case GL_SAMPLER_3D:
			case GL_UNSIGNED_INT_SAMPLER_3D:
			case GL_INT_SAMPLER_3D:
				samplerType = GPOT_SAMPLER3D;
				textureType = GPOT_TEXTURE3D;
				isSampler = true;
				break;
			case GL_SAMPLER_CUBE:
			case GL_SAMPLER_CUBE_SHADOW:
			case GL_UNSIGNED_INT_SAMPLER_CUBE:
			case GL_INT_SAMPLER_CUBE:
				samplerType = GPOT_SAMPLERCUBE;
				textureType = GPOT_TEXTURECUBE;
				isSampler = true;
				break;
			case GL_SAMPLER_CUBE_MAP_ARRAY:
			case GL_SAMPLER_CUBE_MAP_ARRAY_SHADOW:
			case GL_UNSIGNED_INT_SAMPLER_CUBE_MAP_ARRAY:
			case GL_INT_SAMPLER_CUBE_MAP_ARRAY:
				samplerType = GPOT_SAMPLERCUBE;
				textureType = GPOT_TEXTURECUBEARRAY;
				isSampler = true;
				break;
			case GL_SAMPLER_BUFFER:
			case GL_UNSIGNED_INT_SAMPLER_BUFFER:
			case GL_INT_SAMPLER_BUFFER:
				isBuffer = true;
				break;
#if BS_OPENGL_4_2 || BS_OPENGLES_3_1
			case GL_IMAGE_1D:
			case GL_UNSIGNED_INT_IMAGE_1D:
			case GL_INT_IMAGE_1D:
				textureType = GPOT_RWTEXTURE1D;
				isImage = true;
				break;
			case GL_IMAGE_1D_ARRAY:
			case GL_UNSIGNED_INT_IMAGE_1D_ARRAY:
			case GL_INT_IMAGE_1D_ARRAY:
				textureType = GPOT_RWTEXTURE1DARRAY;
				isImage = true;
				break;
			case GL_IMAGE_2D:
			case GL_UNSIGNED_INT_IMAGE_2D:
			case GL_INT_IMAGE_2D:
				textureType = GPOT_RWTEXTURE2D;
				isImage = true;
				break;
			case GL_IMAGE_2D_ARRAY:
			case GL_UNSIGNED_INT_IMAGE_2D_ARRAY:
			case GL_INT_IMAGE_2D_ARRAY:
				textureType = GPOT_RWTEXTURE2DARRAY;
				isImage = true;
				break;
			case GL_IMAGE_2D_MULTISAMPLE:
			case GL_UNSIGNED_INT_IMAGE_2D_MULTISAMPLE:
			case GL_INT_IMAGE_2D_MULTISAMPLE:
				textureType = GPOT_RWTEXTURE2DMS;
				isImage = true;
				break;
			case GL_IMAGE_2D_MULTISAMPLE_ARRAY:
			case GL_UNSIGNED_INT_IMAGE_2D_MULTISAMPLE_ARRAY:
			case GL_INT_IMAGE_2D_MULTISAMPLE_ARRAY:
				textureType = GPOT_RWTEXTURE2DMSARRAY;
				isImage = true;
				break;
			case GL_IMAGE_3D:
			case GL_UNSIGNED_INT_IMAGE_3D:
			case GL_INT_IMAGE_3D:
				textureType = GPOT_RWTEXTURE3D;
				isImage = true;
				break;
			case GL_IMAGE_BUFFER:
			case GL_UNSIGNED_INT_IMAGE_BUFFER:
			case GL_INT_IMAGE_BUFFER:
				isRWBuffer = true;
				break;
#endif
			}

			if (isSampler)
			{
				GpuParamObjectDesc samplerParam;
				samplerParam.name = paramName;
				samplerParam.type = samplerType;
				samplerParam.slot = glGetUniformLocation(glProgram, uniformName);
				samplerParam.set = mapParameterToSet(type, ParamType::Sampler);

				GpuParamObjectDesc textureParam;
				textureParam.name = paramName;
				textureParam.type = textureType;
				textureParam.slot = samplerParam.slot;
				textureParam.set = mapParameterToSet(type, ParamType::Texture);

				returnParamDesc.samplers.insert(std::make_pair(paramName, samplerParam));
				returnParamDesc.textures.insert(std::make_pair(paramName, textureParam));

				BS_CHECK_GL_ERROR();
			}
			else if (isImage)
			{
				GpuParamObjectDesc textureParam;
				textureParam.name = paramName;
				textureParam.type = textureType;
				textureParam.slot = glGetUniformLocation(glProgram, uniformName);
				textureParam.set = mapParameterToSet(type, ParamType::Image);

				returnParamDesc.loadStoreTextures.insert(std::make_pair(paramName, textureParam));

				BS_CHECK_GL_ERROR();
			}
			else if (isBuffer)
			{
				GpuParamObjectDesc bufferParam;
				bufferParam.name = paramName;
				bufferParam.type = GPOT_BYTE_BUFFER;
				bufferParam.slot = glGetUniformLocation(glProgram, uniformName);
				bufferParam.set = mapParameterToSet(type, ParamType::Texture);

				returnParamDesc.buffers.insert(std::make_pair(paramName, bufferParam));

				BS_CHECK_GL_ERROR();
			}
			else if(isRWBuffer)
			{
				GpuParamObjectDesc bufferParam;
				bufferParam.name = paramName;
				bufferParam.type = GPOT_RWBYTE_BUFFER;
				bufferParam.slot = glGetUniformLocation(glProgram, uniformName);
				bufferParam.set = mapParameterToSet(type, ParamType::Image);

				returnParamDesc.buffers.insert(std::make_pair(paramName, bufferParam));

				BS_CHECK_GL_ERROR();
			}
			else
			{
				// If array index is larger than 0 and uniform is not a part of a struct,
				// it means we already processed it (struct arrays are processed differently)
				if (!inStruct && arrayIdx != 0)
					continue;

				GLint blockIndex;
				glGetActiveUniformsiv(glProgram, 1, &index, GL_UNIFORM_BLOCK_INDEX, &blockIndex);
				BS_CHECK_GL_ERROR();

				GpuParamDataDesc gpuParam;

				if (isInArray)
					gpuParam.name = cleanParamName;
				else
					gpuParam.name = paramName;

				determineParamInfo(gpuParam, paramName, glProgram, index);

				if (blockIndex != -1)
				{
					GLint blockOffset;
					glGetActiveUniformsiv(glProgram, 1, &index, GL_UNIFORM_OFFSET, &blockOffset);

					blockOffset = blockOffset / 4;

					gpuParam.gpuMemOffset = blockOffset;

					String& blockName = blockSlotToName[blockIndex + 1];
					GpuParamBlockDesc& curBlockDesc = returnParamDesc.paramBlocks[blockName];

					gpuParam.paramBlockSlot = curBlockDesc.slot;
					gpuParam.paramBlockSet = mapParameterToSet(type, ParamType::UniformBlock);
					gpuParam.cpuMemOffset = blockOffset;
					curBlockDesc.blockSize = std::max(curBlockDesc.blockSize, gpuParam.cpuMemOffset + gpuParam.arrayElementStride * gpuParam.arraySize);

					BS_CHECK_GL_ERROR();
				}
				else
				{
					gpuParam.gpuMemOffset = glGetUniformLocation(glProgram, uniformName);
					gpuParam.paramBlockSlot = 0;
					gpuParam.paramBlockSet = mapParameterToSet(type, ParamType::UniformBlock);
					gpuParam.cpuMemOffset = globalBlockDesc.blockSize;

					globalBlockDesc.blockSize = std::max(globalBlockDesc.blockSize, gpuParam.cpuMemOffset + gpuParam.arrayElementStride * gpuParam.arraySize);

					BS_CHECK_GL_ERROR();
				}

				// If parameter is not a part of a struct we're done
				if (!inStruct)
				{
					returnParamDesc.params.insert(std::make_pair(gpuParam.name, gpuParam));
					continue;
				}

				// If the parameter is part of a struct, then we need to update the struct definition
				auto findExistingStruct = foundStructs.find(structName);

				// Create new definition if one doesn't exist
				if (findExistingStruct == foundStructs.end())
				{
					foundStructs[structName] = GpuParamDataDesc();
					GpuParamDataDesc& structDesc = foundStructs[structName];
					structDesc.type = GPDT_STRUCT;
					structDesc.name = structName;
					structDesc.arraySize = 1;
					structDesc.elementSize = 0;
					structDesc.arrayElementStride = 0;
					structDesc.gpuMemOffset = gpuParam.gpuMemOffset;
					structDesc.cpuMemOffset = gpuParam.cpuMemOffset;
					structDesc.paramBlockSlot = gpuParam.paramBlockSlot;
					structDesc.paramBlockSet = gpuParam.paramBlockSet;
				}

				// Update struct with size of the new parameter
				GpuParamDataDesc& structDesc = foundStructs[structName];

				assert(gpuParam.cpuMemOffset >= structDesc.cpuMemOffset);
				if (arrayIdx == (UINT32)firstArrayIndex) // Determine element size only using the first array element
				{
					structDesc.elementSize = std::max(structDesc.elementSize, (gpuParam.cpuMemOffset - structDesc.cpuMemOffset) + gpuParam.arrayElementStride * gpuParam.arraySize);
					structDesc.arrayElementStride = structDesc.elementSize;
				}

				// New array element reached, determine arrayElementStride
				if (arrayIdx != (UINT32)firstArrayIndex)
				{
					UINT32 numElements = arrayIdx - firstArrayIndex;
					structDesc.arrayElementStride = (gpuParam.cpuMemOffset - structDesc.cpuMemOffset) / numElements;
				}

				structDesc.arraySize = std::max(structDesc.arraySize, arrayIdx + 1);
			}
		}

		for (auto iter = foundStructs.begin(); iter != foundStructs.end(); ++iter)
			returnParamDesc.params.insert(std::make_pair(iter->first, iter->second));

		// Param blocks always need to be a multiple of 4, so make it so
		for (auto iter = returnParamDesc.paramBlocks.begin(); iter != returnParamDesc.paramBlocks.end(); ++iter)
		{
			GpuParamBlockDesc& blockDesc = iter->second;

			if (blockDesc.blockSize % 4 != 0)
				blockDesc.blockSize += (4 - (blockDesc.blockSize % 4));
		}

#if BS_DEBUG_MODE
		// Check if manually calculated and OpenGL buffer sizes match
		UINT32 blockIdx = 0;
		for (auto iter = returnParamDesc.paramBlocks.begin(); iter != returnParamDesc.paramBlocks.end(); ++iter)
		{
			if (iter->second.slot == 0)
				continue;

			GLint blockSize = 0;
			glGetActiveUniformBlockiv(glProgram, blockIdx, GL_UNIFORM_BLOCK_DATA_SIZE, &blockSize);
			BS_CHECK_GL_ERROR();

			assert(blockSize % 4 == 0);
			blockSize = blockSize / 4;

			if ((INT32)iter->second.blockSize != blockSize)
				BS_EXCEPT(InternalErrorException, "OpenGL specified and manual uniform block buffer sizes don't match!");

			blockIdx++;
		}
#endif

		bs_free(uniformName);
	}

	void GLSLParamParser::determineParamInfo(GpuParamDataDesc& desc, const String& paramName, GLuint programHandle, GLuint uniformIndex)
	{
		GLint arraySize;
		glGetActiveUniformsiv(programHandle, 1, &uniformIndex, GL_UNIFORM_SIZE, &arraySize);
		BS_CHECK_GL_ERROR();
		desc.arraySize = arraySize;

		GLint uniformType;
		glGetActiveUniformsiv(programHandle, 1, &uniformIndex, GL_UNIFORM_TYPE, &uniformType);
		BS_CHECK_GL_ERROR();

		switch (uniformType)
		{
		case GL_BOOL:
			desc.type = GPDT_BOOL;
			desc.elementSize = 1;
			break;
		case GL_FLOAT:
			desc.type = GPDT_FLOAT1;
			desc.elementSize = 1;
			break;
		case GL_FLOAT_VEC2:
			desc.type = GPDT_FLOAT2;
			desc.elementSize = 2;
			break;
		case GL_FLOAT_VEC3:
			desc.type = GPDT_FLOAT3;
			desc.elementSize = 3;
			break;
		case GL_FLOAT_VEC4:
			desc.type = GPDT_FLOAT4;
			desc.elementSize = 4;
			break;
		case GL_INT:
		case GL_UNSIGNED_INT:
			desc.type = GPDT_INT1;
			desc.elementSize = 1;
			break;
		case GL_INT_VEC2:
		case GL_UNSIGNED_INT_VEC2:
			desc.type = GPDT_INT2;
			desc.elementSize = 2;
			break;
		case GL_INT_VEC3:
		case GL_UNSIGNED_INT_VEC3:
			desc.type = GPDT_INT3;
			desc.elementSize = 3;
			break;
		case GL_INT_VEC4:
		case GL_UNSIGNED_INT_VEC4:
			desc.type = GPDT_INT4;
			desc.elementSize = 4;
			break;
		case GL_FLOAT_MAT2:
			desc.type = GPDT_MATRIX_2X2;
			desc.elementSize = 4;
			break;
		case GL_FLOAT_MAT3:
			desc.type = GPDT_MATRIX_3X3;
			desc.elementSize = 9;
			break;
		case GL_FLOAT_MAT4:
			desc.type = GPDT_MATRIX_4X4;
			desc.elementSize = 16;
			break;
		case GL_FLOAT_MAT2x3:
			desc.type = GPDT_MATRIX_2X3;
			desc.elementSize = 6;
			break;
		case GL_FLOAT_MAT3x2:
			desc.type = GPDT_MATRIX_3X2;
			desc.elementSize = 6;
			break;
		case GL_FLOAT_MAT2x4:
			desc.type = GPDT_MATRIX_2X4;
			desc.elementSize = 8;
			break;
		case GL_FLOAT_MAT4x2:
			desc.type = GPDT_MATRIX_4X2;
			desc.elementSize = 8;
			break;
		case GL_FLOAT_MAT3x4:
			desc.type = GPDT_MATRIX_3X4;
			desc.elementSize = 12;
			break;
		case GL_FLOAT_MAT4x3:
			desc.type = GPDT_MATRIX_4X3;
			desc.elementSize = 12;
			break;
		default:
			BS_EXCEPT(InternalErrorException, "Invalid shader parameter type: " + toString(uniformType) + " for parameter " + paramName);
		}

		if (arraySize > 1)
		{
			GLint arrayStride;
			glGetActiveUniformsiv(programHandle, 1, &uniformIndex, GL_UNIFORM_ARRAY_STRIDE, &arrayStride);
			BS_CHECK_GL_ERROR();

			if (arrayStride > 0)
			{
				assert(arrayStride % 4 == 0);

				desc.arrayElementStride = arrayStride / 4;
			}
			else
				desc.arrayElementStride = desc.elementSize;
		}
		else
			desc.arrayElementStride = desc.elementSize;
	}

	UINT32 GLSLParamParser::mapParameterToSet(GpuProgramType progType, ParamType paramType)
	{
		UINT32 progTypeIdx = (UINT32)progType;
		UINT32 paramTypeIdx = (UINT32)paramType;

		return progTypeIdx * (UINT32)ParamType::Count + paramTypeIdx;
	}
}}