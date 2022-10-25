//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsVulkanGLSLToSPIRV.h"
#include "BsVulkanGpuProgram.h"
#include "BsVulkanUtility.h"
#include "RenderAPI/BsGpuParams.h"
#include "RenderAPI/BsGpuParamDesc.h"
#include "Math/BsMath.h"

#define AMD_EXTENSIONS
#define NV_EXTENSIONS
#include "glslang/Public/ShaderLang.h"
#include "glslang/Include/Types.h"
#include "glslang/Include/revision.h"
#include "SPIRV/GlslangToSpv.h"
#include "SPIRV/Logger.h"

namespace bs
{
	namespace ct
	{
		const TBuiltInResource DefaultTBuiltInResource = {
			/* .MaxLights = */ 32,
			/* .MaxClipPlanes = */ 6,
			/* .MaxTextureUnits = */ 32,
			/* .MaxTextureCoords = */ 32,
			/* .MaxVertexAttribs = */ 64,
			/* .MaxVertexUniformComponents = */ 4096,
			/* .MaxVaryingFloats = */ 64,
			/* .MaxVertexTextureImageUnits = */ 32,
			/* .MaxCombinedTextureImageUnits = */ 80,
			/* .MaxTextureImageUnits = */ 32,
			/* .MaxFragmentUniformComponents = */ 4096,
			/* .MaxDrawBuffers = */ 32,
			/* .MaxVertexUniformVectors = */ 128,
			/* .MaxVaryingVectors = */ 8,
			/* .MaxFragmentUniformVectors = */ 16,
			/* .MaxVertexOutputVectors = */ 16,
			/* .MaxFragmentInputVectors = */ 15,
			/* .MinProgramTexelOffset = */ -8,
			/* .MaxProgramTexelOffset = */ 7,
			/* .MaxClipDistances = */ 8,
			/* .MaxComputeWorkGroupCountX = */ 65535,
			/* .MaxComputeWorkGroupCountY = */ 65535,
			/* .MaxComputeWorkGroupCountZ = */ 65535,
			/* .MaxComputeWorkGroupSizeX = */ 1024,
			/* .MaxComputeWorkGroupSizeY = */ 1024,
			/* .MaxComputeWorkGroupSizeZ = */ 64,
			/* .MaxComputeUniformComponents = */ 1024,
			/* .MaxComputeTextureImageUnits = */ 16,
			/* .MaxComputeImageUniforms = */ 8,
			/* .MaxComputeAtomicCounters = */ 8,
			/* .MaxComputeAtomicCounterBuffers = */ 1,
			/* .MaxVaryingComponents = */ 60,
			/* .MaxVertexOutputComponents = */ 64,
			/* .MaxGeometryInputComponents = */ 64,
			/* .MaxGeometryOutputComponents = */ 128,
			/* .MaxFragmentInputComponents = */ 128,
			/* .MaxImageUnits = */ 8,
			/* .MaxCombinedImageUnitsAndFragmentOutputs = */ 8,
			/* .MaxCombinedShaderOutputResources = */ 8,
			/* .MaxImageSamples = */ 0,
			/* .MaxVertexImageUniforms = */ 0,
			/* .MaxTessControlImageUniforms = */ 0,
			/* .MaxTessEvaluationImageUniforms = */ 0,
			/* .MaxGeometryImageUniforms = */ 0,
			/* .MaxFragmentImageUniforms = */ 8,
			/* .MaxCombinedImageUniforms = */ 8,
			/* .MaxGeometryTextureImageUnits = */ 16,
			/* .MaxGeometryOutputVertices = */ 256,
			/* .MaxGeometryTotalOutputComponents = */ 1024,
			/* .MaxGeometryUniformComponents = */ 1024,
			/* .MaxGeometryVaryingComponents = */ 64,
			/* .MaxTessControlInputComponents = */ 128,
			/* .MaxTessControlOutputComponents = */ 128,
			/* .MaxTessControlTextureImageUnits = */ 16,
			/* .MaxTessControlUniformComponents = */ 1024,
			/* .MaxTessControlTotalOutputComponents = */ 4096,
			/* .MaxTessEvaluationInputComponents = */ 128,
			/* .MaxTessEvaluationOutputComponents = */ 128,
			/* .MaxTessEvaluationTextureImageUnits = */ 16,
			/* .MaxTessEvaluationUniformComponents = */ 1024,
			/* .MaxTessPatchComponents = */ 120,
			/* .MaxPatchVertices = */ 32,
			/* .MaxTessGenLevel = */ 64,
			/* .MaxViewports = */ 16,
			/* .MaxVertexAtomicCounters = */ 0,
			/* .MaxTessControlAtomicCounters = */ 0,
			/* .MaxTessEvaluationAtomicCounters = */ 0,
			/* .MaxGeometryAtomicCounters = */ 0,
			/* .MaxFragmentAtomicCounters = */ 8,
			/* .MaxCombinedAtomicCounters = */ 8,
			/* .MaxAtomicCounterBindings = */ 1,
			/* .MaxVertexAtomicCounterBuffers = */ 0,
			/* .MaxTessControlAtomicCounterBuffers = */ 0,
			/* .MaxTessEvaluationAtomicCounterBuffers = */ 0,
			/* .MaxGeometryAtomicCounterBuffers = */ 0,
			/* .MaxFragmentAtomicCounterBuffers = */ 1,
			/* .MaxCombinedAtomicCounterBuffers = */ 1,
			/* .MaxAtomicCounterBufferSize = */ 16384,
			/* .MaxTransformFeedbackBuffers = */ 4,
			/* .MaxTransformFeedbackInterleavedComponents = */ 64,
			/* .MaxCullDistances = */ 8,
			/* .MaxCombinedClipAndCullDistances = */ 8,
			/* .MaxSamples = */ 4,
#if GLSLANG_PATCH_LEVEL >= 2892
			/* .maxMeshOutputVerticesNV = */ 256,
			/* .maxMeshOutputPrimitivesNV = */ 512,
			/* .maxMeshWorkGroupSizeX_NV = */ 32,
			/* .maxMeshWorkGroupSizeY_NV = */ 1,
			/* .maxMeshWorkGroupSizeZ_NV = */ 1,
			/* .maxTaskWorkGroupSizeX_NV = */ 32,
			/* .maxTaskWorkGroupSizeY_NV = */ 1,
			/* .maxTaskWorkGroupSizeZ_NV = */ 1,
			/* .maxMeshViewCountNV = */ 4,
#endif
			/* .limits = */ {
				/* .nonInductiveForLoops = */ 1,
				/* .whileLoops = */ 1,
				/* .doWhileLoops = */ 1,
				/* .generalUniformIndexing = */ 1,
				/* .generalAttributeMatrixVectorIndexing = */ 1,
				/* .generalVaryingIndexing = */ 1,
				/* .generalSamplerIndexing = */ 1,
				/* .generalVariableIndexing = */ 1,
				/* .generalConstantMatrixVectorIndexing = */ 1,
			}
		};

		VertexElementType mapGLSLangToVertexElemType(const glslang::TType& type)
		{
			if(type.isVector())
			{
				u32 vectorSize = type.getVectorSize();

				switch(type.getBasicType())
				{
				case glslang::EbtFloat:
					switch(vectorSize)
					{
					case 2: return VET_FLOAT2;
					case 3: return VET_FLOAT3;
					case 4: return VET_FLOAT4;
					default: return VET_UNKNOWN;
					}
				case glslang::EbtInt:
					switch(vectorSize)
					{
					case 2: return VET_INT2;
					case 3: return VET_INT3;
					case 4: return VET_INT4;
					default: return VET_UNKNOWN;
					}
				case glslang::EbtUint:
					switch(vectorSize)
					{
					case 2: return VET_UINT2;
					case 3: return VET_UINT3;
					case 4: return VET_UINT4;
					default: return VET_UNKNOWN;
					}
				default:
					return VET_UNKNOWN;
				}
			}

			if(type.getVectorSize() == 1)
			{
				switch(type.getBasicType())
				{
				case glslang::EbtFloat: return VET_FLOAT1;
				case glslang::EbtInt: return VET_INT1;
				case glslang::EbtUint: return VET_UINT1;
				default: return VET_UNKNOWN;
				}
			}

			return VET_UNKNOWN;
		}

		GpuParamDataType mapGLSLangToGpuParamDataType(const glslang::TType& type)
		{
			if(type.getBasicType() == glslang::EbtStruct)
				return GPDT_STRUCT;

			if(type.isVector())
			{
				u32 vectorSize = type.getVectorSize();

				switch(type.getBasicType())
				{
				case glslang::EbtFloat:
					switch(vectorSize)
					{
					case 2: return GPDT_FLOAT2;
					case 3: return GPDT_FLOAT3;
					case 4: return GPDT_FLOAT4;
					default: return GPDT_UNKNOWN;
					}
				case glslang::EbtInt:
					switch(vectorSize)
					{
					case 2: return GPDT_INT2;
					case 3: return GPDT_INT3;
					case 4: return GPDT_INT4;
					default: return GPDT_UNKNOWN;
					}
				case glslang::EbtUint:
					switch(vectorSize)
					{
					case 2: return GPDT_INT2;
					case 3: return GPDT_INT3;
					case 4: return GPDT_INT4;
					default: return GPDT_UNKNOWN;
					}
				default:
					return GPDT_UNKNOWN;
				}
			}

			if(type.isMatrix())
			{
				switch(type.getBasicType())
				{
				case glslang::EbtFloat:
					switch(type.getMatrixCols())
					{
					case 2:
						switch(type.getMatrixRows())
						{
						case 2: return GPDT_MATRIX_2X2;
						case 3: return GPDT_MATRIX_3X2;
						case 4: return GPDT_MATRIX_4X2;
						default: return GPDT_UNKNOWN;
						}
					case 3:
						switch(type.getMatrixRows())
						{
						case 2: return GPDT_MATRIX_2X3;
						case 3: return GPDT_MATRIX_3X3;
						case 4: return GPDT_MATRIX_4X3;
						default: return GPDT_UNKNOWN;
						}
					case 4:
						switch(type.getMatrixRows())
						{
						case 2: return GPDT_MATRIX_2X4;
						case 3: return GPDT_MATRIX_3X4;
						case 4: return GPDT_MATRIX_4X4;
						default: return GPDT_UNKNOWN;
						}
					default: break;
					}
					break;
				default:
					return GPDT_UNKNOWN;
				}
			}

			if(type.getVectorSize() == 1)
			{
				switch(type.getBasicType())
				{
				case glslang::EbtFloat: return GPDT_FLOAT1;
				case glslang::EbtInt: return GPDT_INT1;
				case glslang::EbtUint: return GPDT_INT1;
				case glslang::EbtBool: return GPDT_BOOL;
				default: return GPDT_UNKNOWN;
				}
			}

			return GPDT_UNKNOWN;
		}

		GpuBufferFormat mapSamplerBasicType(const glslang::TSampler& sampler)
		{
			u32 vectorSize = sampler.vectorSize;
			switch(sampler.type)
			{
			case glslang::EbtFloat:
				switch(vectorSize)
				{
				case 1: return BF_32X1F;
				case 2: return BF_32X2F;
				case 3: return BF_32X3F;
				case 4: return BF_32X4F;
				default: return BF_UNKNOWN;
				}
			case glslang::EbtFloat16:
				switch(vectorSize)
				{
				case 1: return BF_16X1F;
				case 2: return BF_16X2F;
				case 4: return BF_16X4F;
				default: return BF_UNKNOWN;
				}
			case glslang::EbtInt16:
				switch(vectorSize)
				{
				case 1: return BF_16X1U;
				case 2: return BF_16X2U;
				case 4: return BF_16X4U;
				default: return BF_UNKNOWN;
				}
			case glslang::EbtUint16:
				switch(vectorSize)
				{
				case 1: return BF_16X1S;
				case 2: return BF_16X2S;
				case 4: return BF_16X4S;
				default: return BF_UNKNOWN;
				}
			case glslang::EbtInt8:
				switch(vectorSize)
				{
				case 1: return BF_8X1U;
				case 2: return BF_8X2U;
				case 4: return BF_8X4U;
				default: return BF_UNKNOWN;
				}
			case glslang::EbtUint8:
				switch(vectorSize)
				{
				case 1: return BF_8X1S;
				case 2: return BF_8X2S;
				case 4: return BF_8X4S;
				default: return BF_UNKNOWN;
				}
			case glslang::EbtInt:
				switch(vectorSize)
				{
				case 1: return BF_32X1S;
				case 2: return BF_32X2S;
				case 3: return BF_32X3S;
				case 4: return BF_32X4S;
				default: return BF_UNKNOWN;
				}
			case glslang::EbtUint:
				switch(vectorSize)
				{
				case 1: return BF_32X1U;
				case 2: return BF_32X2U;
				case 3: return BF_32X3U;
				case 4: return BF_32X4U;
				default: return BF_UNKNOWN;
				}
			default:
				return BF_UNKNOWN;
			}
		}

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
			i32 MatchesName(const String& name) const
			{
				if(!StringUtil::StartsWith(name, mName, false))
					return -1;

				u32 length = (u32)mName.size();
				return parsei32(name.substr(length));
			}

			/**	Returns the semantic of this attribute. */
			VertexElementSemantic GetSemantic() const { return mSemantic; }

		private:
			String mName;
			VertexElementSemantic mSemantic;
		};

		bool attribNameToElementSemantic(const String& name, VertexElementSemantic& semantic, u16& index)
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

		bool parseVertexAttributes(const glslang::TProgram* program, Vector<VertexElement>& elementList, String& log)
		{
			int numAttributes = program->getNumLiveAttributes();
			for(int i = 0; i < numAttributes; i++)
			{
				const glslang::TType* ttype = program->getAttributeTType(i);
				u32 location = ttype->getQualifier().layoutLocation;

				if(location == (u32)-1)
				{
					log = "Vertex attribute parsing error: Found a vertex attribute without a location "
						  "qualifier. Each attribute must have an explicitly defined location number.";

					return false;
				}

				const char* attribName = program->getAttributeName(i);

				VertexElementSemantic semantic = VES_POSITION;
				u16 index = 0;
				if(attribNameToElementSemantic(attribName, semantic, index))
				{
					VertexElementType type = mapGLSLangToVertexElemType(*ttype);
					if(type == VET_UNKNOWN)
						BS_LOG(Error, RenderBackend, "Cannot determine vertex input attribute type for attribute: {0}", attribName);

					elementList.push_back(VertexElement(0, location, type, semantic, index));
				}
				else
				{
					// Ignore built-in attributes
					if(memcmp(attribName, "gl_", 3) != 0)
					{
						BS_LOG(Error, RenderBackend, "Cannot determine vertex input attribute semantic for attribute: {0}", attribName);
					}
				}
			}

			return true;
		}

		void parseStruct(const glslang::TTypeList* typeList, u32& size)
		{
			for(auto iter = typeList->begin(); iter != typeList->end(); ++iter)
			{
				const glslang::TType* ttype = iter->type;

				if(ttype->getBasicType() == glslang::EbtStruct)
				{
					const glslang::TTypeList* childTypeList = ttype->getStruct();
					parseStruct(childTypeList, size);
				}
				else
				{
					u32 arraySize = 1;
					if(ttype->isArray())
						arraySize = (u32)ttype->getCumulativeArraySize();

					GpuParamDataType paramType = mapGLSLangToGpuParamDataType(*ttype);
					if(paramType == GPDT_UNKNOWN)
					{
						BS_LOG(Warning, RenderBackend, "Cannot determine type for uniform inside a struct.");
						continue;
					}

					u32 elemSize = VulkanUtility::CalcInterfaceBlockElementSizeAndOffset(paramType, arraySize, size);
					size += elemSize;
				}
			}
		}

		bool parseUniforms(const glslang::TProgram* program, GpuParamDesc& desc, String& log)
		{
			// Parse individual uniforms
			struct UniformInfo
			{
				u32 BufferOffset;
				u32 ArraySize;
			};

			UnorderedMap<String, UniformInfo> uniforms;

			int numUniforms = program->getNumLiveUniformVariables();
			for(int i = 0; i < numUniforms; i++)
			{
				const glslang::TType* ttype = program->getUniformTType(i);
				const glslang::TQualifier& qualifier = ttype->getQualifier();
				const char* name = program->getUniformName(i);

				if(ttype->getBasicType() == glslang::EbtSampler) // Object type
				{
					// Note: Even though the type is named EbtSampler, all object types are categorized under it (including non
					// sampled images and buffers)

					if(!qualifier.hasBinding())
					{
						log = "Uniform parsing error: Found an uniform without a binding qualifier. Each uniform must have an "
							  "explicitly defined binding number.";

						return false;
					}

					const glslang::TSampler& sampler = ttype->getSampler();

					GpuParamObjectDesc param;
					param.Name = name;
					param.Slot = qualifier.layoutBinding;
					param.Set = qualifier.layoutSet;
					param.Type = GPOT_UNKNOWN;
					param.ElementType = mapSamplerBasicType(sampler);

					if(param.Set == glslang::TQualifier::layoutSetEnd)
						param.Set = 0;

					if(sampler.isImage())
					{
						switch(sampler.dim)
						{
						case glslang::Esd1D: param.Type = sampler.isArrayed() ? GPOT_RWTEXTURE1DARRAY : GPOT_RWTEXTURE1D; break;
						case glslang::Esd2D:
							if(sampler.isArrayed())
								param.Type = sampler.isMultiSample() ? GPOT_RWTEXTURE2DMSARRAY : GPOT_RWTEXTURE2DARRAY;
							else
								param.Type = sampler.isMultiSample() ? GPOT_RWTEXTURE2DMS : GPOT_RWTEXTURE2D;
							break;
						case glslang::Esd3D: param.Type = GPOT_RWTEXTURE3D; break;
						case glslang::EsdBuffer: param.Type = GPOT_RWBYTE_BUFFER; break;
						default:
							break;
						}

						if(sampler.dim != glslang::EsdBuffer)
							desc.LoadStoreTextures[name] = param;
						else
							desc.Buffers[name] = param;
					}
					else
					{
						if(sampler.isPureSampler() || sampler.isCombined())
						{
							switch(sampler.dim)
							{
							case glslang::Esd1D: param.Type = GPOT_SAMPLER1D; break;
							default:
							case glslang::Esd2D: param.Type = sampler.isMultiSample() ? GPOT_SAMPLER2DMS : GPOT_SAMPLER2D; break;
							case glslang::Esd3D: param.Type = GPOT_SAMPLER3D; break;
							case glslang::EsdCube: param.Type = GPOT_SAMPLERCUBE; break;
							}

							desc.Samplers[name] = param;
						}

						if(!sampler.isPureSampler())
						{
							switch(sampler.dim)
							{
							case glslang::Esd1D: param.Type = sampler.isArrayed() ? GPOT_TEXTURE1DARRAY : GPOT_TEXTURE1D; break;
							case glslang::Esd2D:
								if(sampler.isArrayed())
									param.Type = sampler.isMultiSample() ? GPOT_TEXTURE2DMSARRAY : GPOT_TEXTURE2DARRAY;
								else
									param.Type = sampler.isMultiSample() ? GPOT_TEXTURE2DMS : GPOT_TEXTURE2D;
								break;
							case glslang::Esd3D: param.Type = GPOT_TEXTURE3D; break;
							case glslang::EsdCube: param.Type = sampler.isArrayed() ? GPOT_TEXTURECUBEARRAY : GPOT_TEXTURECUBE; break;
							case glslang::EsdBuffer: param.Type = GPOT_BYTE_BUFFER; break;
							default:
								break;
							}

							if(sampler.dim != glslang::EsdBuffer)
								desc.Textures[name] = param;
							else
								desc.Buffers[name] = param;
						}
					}

					if(param.Type == GPOT_UNKNOWN)
						BS_LOG(Error, RenderBackend, "Cannot determine type for uniform: {0}", name);
				}
				else
				{
					if(qualifier.storage == glslang::EvqUniform || qualifier.storage == glslang::EvqGlobal)
					{
						UniformInfo info;
						info.ArraySize = program->getUniformArraySize(i);
						info.BufferOffset = program->getUniformBufferOffset(i);

						uniforms[String(name)] = info;
					}
				}
			}

			// Parse uniform blocks
			int numBlocks = program->getNumLiveUniformBlocks();
			for(int i = 0; i < numBlocks; i++)
			{
				const glslang::TType* ttype = program->getUniformBlockTType(i);
				const glslang::TQualifier& qualifier = ttype->getQualifier();
				const char* name = program->getUniformBlockName(i);

				if(!qualifier.hasBinding())
				{
					log = "Uniform parsing error: Found a uniform block without a binding qualifier. Each uniform block must "
						  " have an explicitly defined binding number.";

					return false;
				}

				if(qualifier.storage == glslang::EvqBuffer) // Shared storage buffer
				{
					GpuParamObjectDesc param;
					param.Name = name;
					param.Slot = qualifier.layoutBinding;
					param.Set = qualifier.layoutSet;

					if(param.Set == glslang::TQualifier::layoutSetEnd)
						param.Set = 0;

					param.Type = GPOT_RWSTRUCTURED_BUFFER;
					desc.Buffers[name] = param;
				}
				else // Uniform buffer
				{
					int size = Math::DivideAndRoundUp(program->getUniformBlockSize(i), 16) * 16;

					GpuParamBlockDesc blockDesc;
					blockDesc.Name = name;
					blockDesc.BlockSize = size / 4;
					blockDesc.IsShareable = true;
					blockDesc.Slot = qualifier.layoutBinding;
					blockDesc.Set = qualifier.layoutSet;

					if(blockDesc.Set == glslang::TQualifier::layoutSetEnd)
						blockDesc.Set = 0;

					desc.ParamBlocks[name] = blockDesc;

					// Parse members of the uniform buffer
					const glslang::TTypeList* typeList = ttype->getStruct();
					if(typeList == nullptr)
						continue;

					u32 bufferOffset = 0;
					for(auto iter = typeList->begin(); iter != typeList->end(); ++iter)
					{
						const glslang::TType* paramTType = iter->type;
						String paramName = paramTType->getFieldName().c_str();

						GpuParamDataType paramType;
						u32 elementSize = 0;
						u32 arrayStride = 0;
						if(paramTType->getBasicType() == glslang::EbtStruct)
						{
							paramType = GPDT_STRUCT;

							const glslang::TTypeList* paramTypeList = paramTType->getStruct();
							parseStruct(paramTypeList, elementSize);

							// Struct alignment always a multiple of vec4
							arrayStride = Math::DivideAndRoundUp(elementSize, 4U) * 4;
						}
						else
						{
							paramType = mapGLSLangToGpuParamDataType(*paramTType);
						}

						if(paramType == GPDT_UNKNOWN)
						{
							BS_LOG(Warning, RenderBackend, "Cannot determine type for uniform: {0}", name);
							continue;
						}

						u32 arraySize = paramTType->isArray() ? paramTType->getCumulativeArraySize() : 1;
						if(paramType != GPDT_STRUCT)
						{
							const GpuParamDataTypeInfo& typeInfo = bs::GpuParams::PARAM_SIZES.Lookup[paramType];
							elementSize = typeInfo.Size / 4;

							// Array elements in std140 are always rounded to vec4
							if(arraySize > 1)
								arrayStride = Math::DivideAndRoundUp(elementSize, 4U) * 4;
							else
								arrayStride = elementSize;
						}

						u32 stride;
						if(paramTType->getBasicType() == glslang::EbtStruct)
						{
							// Structs are always aligned and rounded up to vec4
							stride = Math::DivideAndRoundUp(elementSize, 16U) * 4;
							bufferOffset = Math::DivideAndRoundUp(bufferOffset, 4U) * 4;
						}
						else
							stride = VulkanUtility::CalcInterfaceBlockElementSizeAndOffset(paramType, arraySize, bufferOffset);

						bool unusedMember = false;
						auto findIter = uniforms.find(paramName);
						if(findIter != uniforms.end()) // Likely unused and was optimized out
						{
							// Ensure our calculation matches the glslang provided one. We don't use glslang directly because
							// for some cases offset is not provided (e.g. structs that have members optimized out).
							const UniformInfo& uniformInfo = findIter->second;
							assert((uniformInfo.BufferOffset / 4) == bufferOffset);
						}
						else
						{
							// Ignore unused members. We never ignore structs because their members get reported as individual
							// uniforms rather than a whole struct, and we don't have the logic to check that case.
							if(paramType != GPDT_STRUCT)
								unusedMember = true;
						}

						if(!unusedMember)
						{
							GpuParamDataDesc paramDesc;
							paramDesc.Name = paramName;
							paramDesc.Type = paramType;
							paramDesc.ParamBlockSet = blockDesc.Set;
							paramDesc.ParamBlockSlot = blockDesc.Slot;
							paramDesc.ElementSize = elementSize;
							paramDesc.ArrayElementStride = arrayStride;
							paramDesc.ArraySize = arraySize;
							paramDesc.CpuMemOffset = bufferOffset;
							paramDesc.GpuMemOffset = bufferOffset;

							desc.Params[paramName] = paramDesc;
						}

						bufferOffset += stride * arraySize;
					}
				}
			}

			return true;
		}

		GLSLToSPIRV::GLSLToSPIRV()
		{
			glslang::InitializeProcess();
		}

		GLSLToSPIRV::~GLSLToSPIRV()
		{
			glslang::FinalizeProcess();
		}

		SPtr<GpuProgramBytecode> GLSLToSPIRV::Convert(const GPU_PROGRAM_DESC& desc)
		{
			TBuiltInResource resources = DefaultTBuiltInResource;
			glslang::TProgram* program = bs_new<glslang::TProgram>();

			EShLanguage glslType;
			switch(desc.Type)
			{
			case GPT_FRAGMENT_PROGRAM:
				glslType = EShLangFragment;
				break;
			case GPT_HULL_PROGRAM:
				glslType = EShLangTessControl;
				break;
			case GPT_DOMAIN_PROGRAM:
				glslType = EShLangTessEvaluation;
				break;
			case GPT_GEOMETRY_PROGRAM:
				glslType = EShLangGeometry;
				break;
			case GPT_VERTEX_PROGRAM:
				glslType = EShLangVertex;
				break;
			case GPT_COMPUTE_PROGRAM:
				glslType = EShLangCompute;
				break;
			default:
				break;
			}

			std::vector<u32> spirv;
			spv::SpvBuildLogger logger;
			std::string compileLog;

			const String& source = desc.Source;
			const char* sourceBytes = source.c_str();

			glslang::TShader* shader = bs_new<glslang::TShader>(glslType);
			shader->setEnvTarget(glslang::EShTargetSpv, glslang::EShTargetSpv_1_3);
			shader->setStrings(&sourceBytes, 1);
			shader->setEntryPoint("main");

			SPtr<GpuProgramBytecode> bytecode = bs_shared_ptr_new<GpuProgramBytecode>();
			bytecode->CompilerId = VULKAN_COMPILER_ID;
			bytecode->CompilerVersion = VULKAN_COMPILER_VERSION;

			EShMessages messages = (EShMessages)((int)EShMsgSpvRules | (int)EShMsgVulkanRules);
			if(!shader->parse(&resources, 450, false, messages))
			{
				bytecode->Messages = "Compile error: " + String(shader->getInfoLog());
				goto cleanup;
			}

			program->addShader(shader);

			if(!program->link(messages))
			{
				bytecode->Messages = "Link error: " + String(program->getInfoLog());
				goto cleanup;
			}

			program->mapIO();
			program->buildReflection();

			// Compile to SPIR-V
			GlslangToSpv(*program->getIntermediate(glslType), spirv, &logger);

			// Parse uniforms
			bytecode->ParamDesc = bs_shared_ptr_new<GpuParamDesc>();
			if(!parseUniforms(program, *bytecode->ParamDesc, bytecode->Messages))
				goto cleanup;

			// If vertex program, retrieve information about vertex inputs
			if(desc.Type == GPT_VERTEX_PROGRAM)
			{
				if(!parseVertexAttributes(program, bytecode->VertexInput, bytecode->Messages))
					goto cleanup;
			}

			bytecode->Instructions.Size = (u32)spirv.size() * sizeof(u32);
			bytecode->Instructions.Data = (u8*)bs_alloc(bytecode->Instructions.Size);

			memcpy(bytecode->Instructions.Data, spirv.data(), bytecode->Instructions.Size);

		cleanup:
			bs_delete(program);
			bs_delete(shader);

			return bytecode;
		}
	} // namespace ct
} // namespace bs
