//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "GLSL/BsGLSLParamParser.h"
#include "RenderAPI/BsGpuParams.h"

namespace bs
{
	namespace ct
	{
		i32 GLSLAttribute::MatchesName(const String& name)
		{
			if(name.length() >= mName.length())
			{
				if(name.substr(0, mName.length()) == mName)
				{
					String indexStr = name.substr(mName.length(), name.length());
					return parseu32(indexStr, 0);
				}
			}

			return -1;
		}

		Vector<VertexElement> GLSLParamParser::BuildVertexDeclaration(GLuint glProgram)
		{
			GLint numAttributes = 0;
			glGetProgramiv(glProgram, GL_ACTIVE_ATTRIBUTES, &numAttributes);
			BS_CHECK_GL_ERROR();

			GLint maxNameSize = 0;
			glGetProgramiv(glProgram, GL_ACTIVE_ATTRIBUTE_MAX_LENGTH, &maxNameSize);
			BS_CHECK_GL_ERROR();

			GLchar* attributeName = (GLchar*)bs_alloc(sizeof(GLchar) * maxNameSize);

			Vector<VertexElement> elementList;
			for(GLint i = 0; i < numAttributes; i++)
			{
				GLint attribSize = 0;
				GLenum attribType = 0;
				glGetActiveAttrib(glProgram, i, maxNameSize, nullptr, &attribSize, &attribType, attributeName);
				BS_CHECK_GL_ERROR();

				VertexElementSemantic semantic = VES_POSITION;
				u16 index = 0;
				if(AttribNameToElementSemantic(attributeName, semantic, index))
				{
					VertexElementType type = GlTypeToAttributeType(attribType);
					u32 slot = glGetAttribLocation(glProgram, attributeName);
					BS_CHECK_GL_ERROR();

					elementList.push_back(VertexElement(0, slot, type, semantic, index));
				}
				else
				{
					// Ignore built-in attributes
					if(memcmp(attributeName, "gl_", 3) != 0)
					{
						BS_LOG(Warning, RenderBackend, "Cannot determine vertex input attribute type for attribute: {0}", String(attributeName));
					}
				}
			}

			bs_free(attributeName);

			return elementList;
		}

		u32 GLSLParamParser::CalcInterfaceBlockElementSizeAndOffset(GpuParamDataType type, u32 arraySize, u32& offset)
		{
			const GpuParamDataTypeInfo& typeInfo = bs::GpuParams::PARAM_SIZES.Lookup[type];
			u32 size = (typeInfo.BaseTypeSize * typeInfo.NumColumns * typeInfo.NumRows) / 4;
			u32 alignment = typeInfo.Alignment / 4;

			// Fix alignment if needed
			u32 alignOffset = offset % alignment;
			if(alignOffset != 0)
			{
				u32 padding = (alignment - alignOffset);
				offset += padding;
			}

			if(arraySize > 1)
			{
				// Array elements are always padded and aligned to vec4
				alignOffset = size % 4;
				if(alignOffset != 0)
				{
					u32 padding = (4 - alignOffset);
					size += padding;
				}

				alignOffset = offset % 4;
				if(alignOffset != 0)
				{
					u32 padding = (4 - alignOffset);
					offset += padding;
				}

				return size;
			}
			else
				return size;
		}

		VertexElementType GLSLParamParser::GlTypeToAttributeType(GLenum glType)
		{
			switch(glType)
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

		bool GLSLParamParser::AttribNameToElementSemantic(const String& name, VertexElementSemantic& semantic, u16& index)
		{
			static GLSLAttribute attributes[] = {
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

			static const u32 numAttribs = sizeof(attributes) / sizeof(attributes[0]);

			for(u32 i = 0; i < numAttribs; i++)
			{
				i32 attribIndex = attributes[i].MatchesName(name);
				if(attribIndex != -1)
				{
					index = attribIndex;
					semantic = attributes[i].GetSemantic();
					return true;
				}
			}

			return false;
		}

		void GLSLParamParser::BuildUniformDescriptions(GLuint glProgram, GpuProgramType type, GpuParamDesc& returnParamDesc)
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

			GLchar* uniformName = (GLchar*)bs_alloc(sizeof(GLchar) * maxBufferSize);

			GpuParamBlockDesc newGlobalBlockDesc;
			newGlobalBlockDesc.Slot = 0;
			newGlobalBlockDesc.Set = MapParameterToSet(type, ParamType::UniformBlock);
			newGlobalBlockDesc.Name = "BS_INTERNAL_Globals";
			newGlobalBlockDesc.BlockSize = 0;
			newGlobalBlockDesc.IsShareable = false;

			returnParamDesc.ParamBlocks[newGlobalBlockDesc.Name] = newGlobalBlockDesc;
			GpuParamBlockDesc& globalBlockDesc = returnParamDesc.ParamBlocks[newGlobalBlockDesc.Name];

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

			Map<u32, String> blockSlotToName;
			Set<String> blockNames;
			for(GLuint index = 0; index < (GLuint)uniformBlockCount; index++)
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
				newBlockDesc.Slot = index + 1;
				newBlockDesc.Set = MapParameterToSet(type, ParamType::UniformBlock);
				newBlockDesc.Name = uniformName;
				newBlockDesc.BlockSize = 0;
				newBlockDesc.IsShareable = true;

				returnParamDesc.ParamBlocks[newBlockDesc.Name] = newBlockDesc;
				blockSlotToName.insert(std::make_pair(index + 1, newBlockDesc.Name));
				blockNames.insert(newBlockDesc.Name);
			}

#if BS_OPENGL_4_3 || BS_OPENGLES_3_1
			// Scan through the shared storage blocks
			GLint storageBlockCount = 0;
			glGetProgramInterfaceiv(glProgram, GL_SHADER_STORAGE_BLOCK, GL_ACTIVE_RESOURCES, &storageBlockCount);
			BS_CHECK_GL_ERROR();

			for(GLuint index = 0; index < (GLuint)storageBlockCount; index++)
			{
				GLsizei unusedSize = 0;
				glGetProgramResourceName(glProgram, GL_SHADER_STORAGE_BLOCK, index, maxBufferSize, &unusedSize, uniformName);
				BS_CHECK_GL_ERROR();

				GpuParamObjectDesc bufferParam;
				bufferParam.Name = uniformName;
				bufferParam.Slot = index;
				bufferParam.Type = GPOT_RWSTRUCTURED_BUFFER;
				bufferParam.Set = MapParameterToSet(type, ParamType::StorageBlock);

				returnParamDesc.Buffers.insert(std::make_pair(uniformName, bufferParam));
			}
#endif

			Map<String, u32> foundFirstArrayIndex;
			Map<String, GpuParamDataDesc> foundStructs;

			// Get the number of active uniforms
			GLint uniformCount = 0;
			glGetProgramiv(glProgram, GL_ACTIVE_UNIFORMS, &uniformCount);
			BS_CHECK_GL_ERROR();

			// Loop over each of the active uniforms, and add them to the reference container
			// only do this for user defined uniforms, ignore built in gl state uniforms
			for(GLuint index = 0; index < (GLuint)uniformCount; index++)
			{
				GLsizei arraySize = 0;
				glGetActiveUniformName(glProgram, index, maxBufferSize, &arraySize, uniformName);
				BS_CHECK_GL_ERROR();

				String paramName = String(uniformName);

				// Naming rules and packing rules used here are described in
				// OpenGL Core Specification 2.11.4

				// Check if parameter is a part of a struct
				Vector<String> nameElements = StringUtil::Tokenise(paramName, ".");

				bool inStruct = false;
				String structName;
				if(nameElements.size() > 1)
				{
					auto uniformBlockFind = blockNames.find(nameElements[0]);

					// Check if the name is not a struct, and instead a Uniform block namespace
					if(uniformBlockFind != blockNames.end())
					{
						// Possibly it's a struct inside a named uniform block
						if(nameElements.size() > 2)
						{
							inStruct = true;
							structName = nameElements[1];
							paramName = nameElements.back();
						}
					}
					else
					{
						inStruct = true;
						structName = nameElements[0];
						paramName = nameElements.back();
					}
				}

				String cleanParamName = paramName; // Param name without array indexes

				// Check if the parameter is in an array
				u32 arrayIdx = 0;
				bool isInArray = false;
				if(inStruct)
				{
					// If the uniform name has a "[" in it then its an array element uniform.
					String::size_type arrayStart = structName.find("[");
					String::size_type arrayEnd = structName.find("]");
					if(arrayStart != String::npos)
					{
						String strArrIdx = structName.substr(arrayStart + 1, arrayEnd - (arrayStart + 1));
						arrayIdx = parseu32(strArrIdx, 0);
						isInArray = true;

						structName = structName.substr(0, arrayStart);
					}
				}

				{
					// If the uniform name has a "[" in it then its an array element uniform.
					String::size_type arrayStart = cleanParamName.find("[");
					String::size_type arrayEnd = cleanParamName.find("]");
					if(arrayStart != String::npos)
					{
						String strArrIdx = cleanParamName.substr(arrayStart + 1, arrayEnd - (arrayStart + 1));

						// If in struct, we don't care about individual element array indices
						if(!inStruct)
						{
							arrayIdx = parseu32(strArrIdx, 0);
							isInArray = true;
						}

						cleanParamName = cleanParamName.substr(0, arrayStart);
					}
				}

				// GLSL will optimize out unused array indexes, so there's no guarantee that 0 is the first,
				// so we store the first one here
				int firstArrayIndex = 0;
				if(isInArray)
				{
					String nameToSearch = cleanParamName;
					if(inStruct)
						nameToSearch = structName;

					auto arrayIndexFind = foundFirstArrayIndex.find(nameToSearch);
					if(arrayIndexFind == foundFirstArrayIndex.end())
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
				switch(uniformType)
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

				if(isSampler)
				{
					GpuParamObjectDesc samplerParam;
					samplerParam.Name = paramName;
					samplerParam.Type = samplerType;
					samplerParam.Slot = glGetUniformLocation(glProgram, uniformName);
					samplerParam.Set = MapParameterToSet(type, ParamType::Sampler);

					GpuParamObjectDesc textureParam;
					textureParam.Name = paramName;
					textureParam.Type = textureType;
					textureParam.Slot = samplerParam.Slot;
					textureParam.Set = MapParameterToSet(type, ParamType::Texture);

					returnParamDesc.Samplers.insert(std::make_pair(paramName, samplerParam));
					returnParamDesc.Textures.insert(std::make_pair(paramName, textureParam));

					BS_CHECK_GL_ERROR();
				}
				else if(isImage)
				{
					GpuParamObjectDesc textureParam;
					textureParam.Name = paramName;
					textureParam.Type = textureType;
					textureParam.Slot = glGetUniformLocation(glProgram, uniformName);
					textureParam.Set = MapParameterToSet(type, ParamType::Image);

					returnParamDesc.LoadStoreTextures.insert(std::make_pair(paramName, textureParam));

					BS_CHECK_GL_ERROR();
				}
				else if(isBuffer)
				{
					GpuParamObjectDesc bufferParam;
					bufferParam.Name = paramName;
					bufferParam.Type = GPOT_BYTE_BUFFER;
					bufferParam.Slot = glGetUniformLocation(glProgram, uniformName);
					bufferParam.Set = MapParameterToSet(type, ParamType::Texture);

					returnParamDesc.Buffers.insert(std::make_pair(paramName, bufferParam));

					BS_CHECK_GL_ERROR();
				}
				else if(isRWBuffer)
				{
					GpuParamObjectDesc bufferParam;
					bufferParam.Name = paramName;
					bufferParam.Type = GPOT_RWBYTE_BUFFER;
					bufferParam.Slot = glGetUniformLocation(glProgram, uniformName);
					bufferParam.Set = MapParameterToSet(type, ParamType::Image);

					returnParamDesc.Buffers.insert(std::make_pair(paramName, bufferParam));

					BS_CHECK_GL_ERROR();
				}
				else
				{
					// If array index is larger than 0 and uniform is not a part of a struct,
					// it means we already processed it (struct arrays are processed differently)
					if(!inStruct && arrayIdx != 0)
						continue;

					GLint blockIndex;
					glGetActiveUniformsiv(glProgram, 1, &index, GL_UNIFORM_BLOCK_INDEX, &blockIndex);
					BS_CHECK_GL_ERROR();

					GpuParamDataDesc gpuParam;

					if(isInArray)
						gpuParam.Name = cleanParamName;
					else
						gpuParam.Name = paramName;

					DetermineParamInfo(gpuParam, paramName, glProgram, index);

					if(blockIndex != -1)
					{
						GLint blockOffset;
						glGetActiveUniformsiv(glProgram, 1, &index, GL_UNIFORM_OFFSET, &blockOffset);

						blockOffset = blockOffset / 4;

						gpuParam.GpuMemOffset = blockOffset;

						String& blockName = blockSlotToName[blockIndex + 1];
						GpuParamBlockDesc& curBlockDesc = returnParamDesc.ParamBlocks[blockName];

						gpuParam.ParamBlockSlot = curBlockDesc.Slot;
						gpuParam.ParamBlockSet = MapParameterToSet(type, ParamType::UniformBlock);
						gpuParam.CpuMemOffset = blockOffset;
						curBlockDesc.BlockSize = std::max(curBlockDesc.BlockSize, gpuParam.CpuMemOffset + gpuParam.ArrayElementStride * gpuParam.ArraySize);

						BS_CHECK_GL_ERROR();
					}
					else
					{
						gpuParam.GpuMemOffset = glGetUniformLocation(glProgram, uniformName);
						gpuParam.ParamBlockSlot = 0;
						gpuParam.ParamBlockSet = MapParameterToSet(type, ParamType::UniformBlock);
						gpuParam.CpuMemOffset = globalBlockDesc.BlockSize;

						globalBlockDesc.BlockSize = std::max(globalBlockDesc.BlockSize, gpuParam.CpuMemOffset + gpuParam.ArrayElementStride * gpuParam.ArraySize);

						BS_CHECK_GL_ERROR();
					}

					// If parameter is not a part of a struct we're done. Also done if parameter is part of a struct, but
					// not part of a uniform block (in which case we treat struct members as separate parameters)
					if(!inStruct || blockIndex == -1)
					{
						returnParamDesc.Params.insert(std::make_pair(gpuParam.Name, gpuParam));
						continue;
					}

					// If the parameter is part of a struct, then we need to update the struct definition
					auto findExistingStruct = foundStructs.find(structName);

					// Create new definition if one doesn't exist
					if(findExistingStruct == foundStructs.end())
					{
						foundStructs[structName] = GpuParamDataDesc();
						GpuParamDataDesc& structDesc = foundStructs[structName];
						structDesc.Type = GPDT_STRUCT;
						structDesc.Name = structName;
						structDesc.ArraySize = 1;
						structDesc.ElementSize = 0;
						structDesc.ArrayElementStride = 0;
						structDesc.GpuMemOffset = gpuParam.GpuMemOffset;
						structDesc.CpuMemOffset = gpuParam.CpuMemOffset;
						structDesc.ParamBlockSlot = gpuParam.ParamBlockSlot;
						structDesc.ParamBlockSet = gpuParam.ParamBlockSet;
					}

					// Update struct with size of the new parameter
					GpuParamDataDesc& structDesc = foundStructs[structName];

					if(arrayIdx == (u32)firstArrayIndex) // Determine element size only using the first array element
					{
						structDesc.ElementSize = std::max(structDesc.ElementSize, gpuParam.CpuMemOffset + gpuParam.ArrayElementStride * gpuParam.ArraySize);

						structDesc.GpuMemOffset = std::min(structDesc.GpuMemOffset, gpuParam.GpuMemOffset);
						structDesc.CpuMemOffset = std::min(structDesc.CpuMemOffset, gpuParam.CpuMemOffset);
					}

					structDesc.ArraySize = std::max(structDesc.ArraySize, arrayIdx + 1);
				}
			}

			for(auto& entry : foundStructs)
			{
				entry.second.ElementSize = entry.second.ElementSize - entry.second.CpuMemOffset;
				entry.second.ArrayElementStride = Math::DivideAndRoundUp(entry.second.ElementSize, 4U) * 4;

				returnParamDesc.Params.insert(std::make_pair(entry.first, entry.second));
			}

			// Param blocks always need to be a multiple of 4, so make it so
			for(auto iter = returnParamDesc.ParamBlocks.begin(); iter != returnParamDesc.ParamBlocks.end(); ++iter)
			{
				GpuParamBlockDesc& blockDesc = iter->second;

				if(blockDesc.BlockSize % 4 != 0)
					blockDesc.BlockSize += (4 - (blockDesc.BlockSize % 4));
			}

#if BS_DEBUG_MODE
			// Check if manually calculated and OpenGL buffer sizes match
			for(auto iter = returnParamDesc.ParamBlocks.begin(); iter != returnParamDesc.ParamBlocks.end(); ++iter)
			{
				if(iter->second.Slot == 0)
					continue;

				GLint blockSize = 0;
				glGetActiveUniformBlockiv(glProgram, iter->second.Slot - 1, GL_UNIFORM_BLOCK_DATA_SIZE, &blockSize);
				BS_CHECK_GL_ERROR();

				assert(blockSize % 4 == 0);
				blockSize = blockSize / 4;

				if((i32)iter->second.BlockSize != blockSize)
					BS_EXCEPT(InternalErrorException, "OpenGL specified and manual uniform block buffer sizes don't match!");
			}
#endif

			bs_free(uniformName);
		}

		void GLSLParamParser::DetermineParamInfo(GpuParamDataDesc& desc, const String& paramName, GLuint programHandle, GLuint uniformIndex)
		{
			GLint arraySize;
			glGetActiveUniformsiv(programHandle, 1, &uniformIndex, GL_UNIFORM_SIZE, &arraySize);
			BS_CHECK_GL_ERROR();
			desc.ArraySize = arraySize;

			GLint uniformType;
			glGetActiveUniformsiv(programHandle, 1, &uniformIndex, GL_UNIFORM_TYPE, &uniformType);
			BS_CHECK_GL_ERROR();

			switch(uniformType)
			{
			case GL_BOOL:
				desc.Type = GPDT_BOOL;
				desc.ElementSize = 1;
				break;
			case GL_FLOAT:
				desc.Type = GPDT_FLOAT1;
				desc.ElementSize = 1;
				break;
			case GL_FLOAT_VEC2:
				desc.Type = GPDT_FLOAT2;
				desc.ElementSize = 2;
				break;
			case GL_FLOAT_VEC3:
				desc.Type = GPDT_FLOAT3;
				desc.ElementSize = 3;
				break;
			case GL_FLOAT_VEC4:
				desc.Type = GPDT_FLOAT4;
				desc.ElementSize = 4;
				break;
			case GL_INT:
			case GL_UNSIGNED_INT:
				desc.Type = GPDT_INT1;
				desc.ElementSize = 1;
				break;
			case GL_INT_VEC2:
			case GL_UNSIGNED_INT_VEC2:
				desc.Type = GPDT_INT2;
				desc.ElementSize = 2;
				break;
			case GL_INT_VEC3:
			case GL_UNSIGNED_INT_VEC3:
				desc.Type = GPDT_INT3;
				desc.ElementSize = 3;
				break;
			case GL_INT_VEC4:
			case GL_UNSIGNED_INT_VEC4:
				desc.Type = GPDT_INT4;
				desc.ElementSize = 4;
				break;
			case GL_FLOAT_MAT2:
				desc.Type = GPDT_MATRIX_2X2;
				desc.ElementSize = 4;
				break;
			case GL_FLOAT_MAT3:
				desc.Type = GPDT_MATRIX_3X3;
				desc.ElementSize = 9;
				break;
			case GL_FLOAT_MAT4:
				desc.Type = GPDT_MATRIX_4X4;
				desc.ElementSize = 16;
				break;
			case GL_FLOAT_MAT2x3:
				desc.Type = GPDT_MATRIX_2X3;
				desc.ElementSize = 6;
				break;
			case GL_FLOAT_MAT3x2:
				desc.Type = GPDT_MATRIX_3X2;
				desc.ElementSize = 6;
				break;
			case GL_FLOAT_MAT2x4:
				desc.Type = GPDT_MATRIX_2X4;
				desc.ElementSize = 8;
				break;
			case GL_FLOAT_MAT4x2:
				desc.Type = GPDT_MATRIX_4X2;
				desc.ElementSize = 8;
				break;
			case GL_FLOAT_MAT3x4:
				desc.Type = GPDT_MATRIX_3X4;
				desc.ElementSize = 12;
				break;
			case GL_FLOAT_MAT4x3:
				desc.Type = GPDT_MATRIX_4X3;
				desc.ElementSize = 12;
				break;
			default:
				BS_EXCEPT(InternalErrorException, "Invalid shader parameter type: " + toString(uniformType) + " for parameter " + paramName);
			}

			if(arraySize > 1)
			{
				GLint arrayStride;
				glGetActiveUniformsiv(programHandle, 1, &uniformIndex, GL_UNIFORM_ARRAY_STRIDE, &arrayStride);
				BS_CHECK_GL_ERROR();

				if(arrayStride > 0)
				{
					assert(arrayStride % 4 == 0);

					desc.ArrayElementStride = arrayStride / 4;
				}
				else
					desc.ArrayElementStride = desc.ElementSize;
			}
			else
				desc.ArrayElementStride = desc.ElementSize;
		}

		u32 GLSLParamParser::MapParameterToSet(GpuProgramType progType, ParamType paramType)
		{
			u32 progTypeIdx = (u32)progType;
			u32 paramTypeIdx = (u32)paramType;

			return progTypeIdx * (u32)ParamType::Count + paramTypeIdx;
		}
	} // namespace ct
} // namespace bs
