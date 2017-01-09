//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsVulkanGpuProgram.h"
#include "BsVulkanRenderAPI.h"
#include "BsVulkanDevice.h"
#include "BsVulkanUtility.h"
#include "BsGpuParams.h"
#include "BsGpuParamDesc.h"
#include "BsGpuProgramManager.h"
#include "BsVertexDeclaration.h"
#include "BsHardwareBufferManager.h"
#include "BsRenderStats.h"
#include "BsFileSystem.h"
#include "BsDataStream.h"

#define AMD_EXTENSIONS
#include "glslang/Public/ShaderLang.h"
#include "glslang/Include/Types.h"
#include "SPIRV/GlslangToSpv.h"
#include "SPIRV/Logger.h"

namespace bs { namespace ct
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
		/* .limits = */{
		/* .nonInductiveForLoops = */ 1,
		/* .whileLoops = */ 1,
		/* .doWhileLoops = */ 1,
		/* .generalUniformIndexing = */ 1,
		/* .generalAttributeMatrixVectorIndexing = */ 1,
		/* .generalVaryingIndexing = */ 1,
		/* .generalSamplerIndexing = */ 1,
		/* .generalVariableIndexing = */ 1,
		/* .generalConstantMatrixVectorIndexing = */ 1,
		} };

	VertexElementType mapGLSLangToVertexElemType(const glslang::TType& type)
	{
		if (type.isVector()) 
		{
			UINT32 vectorSize = type.getVectorSize();

			switch (type.getBasicType())
			{
			case glslang::EbtFloat:
				switch(vectorSize)
				{
				case 2:		return VET_FLOAT2;
				case 3:		return VET_FLOAT3;
				case 4:		return VET_FLOAT4;
				default:	return VET_UNKNOWN;
				}
			case glslang::EbtInt:
				switch (vectorSize)
				{
				case 2:		return VET_INT2;
				case 3:		return VET_INT3;
				case 4:		return VET_INT4;
				default:	return VET_UNKNOWN;
				}
			case glslang::EbtUint:
				switch (vectorSize)
				{
				case 2:		return VET_UINT2;
				case 3:		return VET_UINT3;
				case 4:		return VET_UINT4;
				default:	return VET_UNKNOWN;
				}
			default:            
				return VET_UNKNOWN;
			}
		}

		if (type.getVectorSize() == 1) 
		{
			switch (type.getBasicType()) 
			{
				case glslang::EbtFloat:      return VET_FLOAT1;
				case glslang::EbtInt:        return VET_INT1;
				case glslang::EbtUint:       return VET_UINT1;
				default:			         return VET_UNKNOWN;
			}
		}

		return VET_UNKNOWN;
	}

	GpuParamDataType mapGLSLangToGpuParamDataType(const glslang::TType& type)
	{
		if (type.getBasicType() == glslang::EbtStruct)
			return GPDT_STRUCT;

		if (type.isVector())
		{
			UINT32 vectorSize = type.getVectorSize();

			switch (type.getBasicType())
			{
			case glslang::EbtFloat:
				switch (vectorSize)
				{
				case 2:		return GPDT_FLOAT2;
				case 3:		return GPDT_FLOAT3;
				case 4:		return GPDT_FLOAT4;
				default:	return GPDT_UNKNOWN;
				}
			case glslang::EbtInt:
				switch (vectorSize)
				{
				case 2:		return GPDT_INT2;
				case 3:		return GPDT_INT3;
				case 4:		return GPDT_INT4;
				default:	return GPDT_UNKNOWN;
				}
			case glslang::EbtUint:
				switch (vectorSize)
				{
				case 2:		return GPDT_INT2;
				case 3:		return GPDT_INT3;
				case 4:		return GPDT_INT4;
				default:	return GPDT_UNKNOWN;
				}
			default:        
				return GPDT_UNKNOWN;
			}
		}

		if (type.isMatrix()) 
		{
			switch (type.getBasicType()) 
			{
			case glslang::EbtFloat:
				switch (type.getMatrixCols()) 
				{
				case 2:
					switch (type.getMatrixRows()) 
					{
						case 2:    return GPDT_MATRIX_2X2;
						case 3:    return GPDT_MATRIX_3X2;
						case 4:    return GPDT_MATRIX_4X2;
						default:   return GPDT_UNKNOWN;
					}
				case 3:
					switch (type.getMatrixRows()) 
					{
						case 2:    return GPDT_MATRIX_2X3;
						case 3:    return GPDT_MATRIX_3X3;
						case 4:    return GPDT_MATRIX_4X3;
						default:   return GPDT_UNKNOWN;
					}
				case 4:
					switch (type.getMatrixRows()) 
					{
						case 2:    return GPDT_MATRIX_2X4;
						case 3:    return GPDT_MATRIX_3X4;
						case 4:    return GPDT_MATRIX_4X4;
						default:   return GPDT_UNKNOWN;
					}
				}
			default:
				return GPDT_UNKNOWN;
			}
		}

		if (type.getVectorSize() == 1)
		{
			switch (type.getBasicType())
			{
			case glslang::EbtFloat:     return GPDT_FLOAT1;
			case glslang::EbtInt:       return GPDT_INT1;
			case glslang::EbtUint:      return GPDT_INT1;
			case glslang::EbtBool:      return GPDT_BOOL;
			default:					return GPDT_UNKNOWN;
			}
		}

		return GPDT_UNKNOWN;
	}

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
		INT32 matchesName(const String& name) const
		{
			if (!StringUtil::startsWith(name, mName, false))
				return -1;

			UINT32 length = (UINT32)mName.size();
			return parseINT32(name.substr(length));
		}

		/**	Returns the semantic of this attribute. */
		VertexElementSemantic getSemantic() const { return mSemantic; }

	private:
		String mName;
		VertexElementSemantic mSemantic;
	};

	bool attribNameToElementSemantic(const String& name, VertexElementSemantic& semantic, UINT16& index)
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
			GLSLAttribute("bs_blendindices", VES_BLEND_INDICES)
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

	bool parseVertexAttributes(const glslang::TProgram* program, List<VertexElement>& elementList, String& log)
	{
		int numAttributes = program->getNumLiveAttributes();
		for (int i = 0; i < numAttributes; i++)
		{
			const glslang::TType* ttype = program->getAttributeTType(i);
			UINT32 location = ttype->getQualifier().layoutLocation;

			if (location == -1)
			{
				log = "Vertex attribute parsing error: Found a vertex attribute without a location "
					"qualifier. Each attribute must have an explicitly defined location number.";

				return false;
			}

			const char* attribName = program->getAttributeName(i);

			VertexElementSemantic semantic = VES_POSITION;
			UINT16 index = 0;
			if (attribNameToElementSemantic(attribName, semantic, index))
			{
				VertexElementType type = mapGLSLangToVertexElemType(*ttype);
				if (type == VET_UNKNOWN)
					LOGERR("Cannot determine vertex input attribute type for attribute: " + String(attribName));

				elementList.push_back(VertexElement(0, location, type, semantic, index));
			}
			else
			{
				// Ignore built-in attributes
				if (memcmp(attribName, "gl_", 3) != 0)
					LOGERR("Cannot determine vertex input attribute semantic for attribute: " + String(attribName));
			}
		}

		return true;
	}

	bool parseUniforms(const glslang::TProgram* program, GpuParamDesc& desc, String& log)
	{
		// Parse uniform blocks
		UnorderedMap<UINT32, String> uniformBlockMap;
		int numBlocks = program->getNumLiveUniformBlocks();
		for (int i = 0; i < numBlocks; i++)
		{
			const glslang::TType* ttype = program->getUniformBlockTType(i);

			if (!ttype->getQualifier().hasBinding())
			{
				log = "Uniform parsing error: Found a uniform block without a binding qualifier. Each uniform block must "
					" have an explicitly defined binding number.";

				return false;
			}

			const char* name = program->getUniformBlockName(i);
			int size = program->getUniformBlockSize(i); 

			GpuParamBlockDesc param;
			param.name = name;
			param.blockSize = size / 4;
			param.isShareable = true;
			param.slot = ttype->getQualifier().layoutBinding;
			param.set = ttype->getQualifier().layoutSet;

			if (param.set == glslang::TQualifier::layoutSetEnd)
				param.set = 0;

			desc.paramBlocks[name] = param;
			uniformBlockMap[i] = name;
		}

		// Parse individual uniforms
		int numUniforms = program->getNumLiveUniformVariables();
		for (int i = 0; i < numUniforms; i++)
		{
			const glslang::TType* ttype = program->getUniformTType(i);
			const char* name = program->getUniformName(i);

			if (ttype->getBasicType() == glslang::EbtSampler) // Object type
			{
				// Note: Even though the type is named EbtSampler, all object types are categorized under it (including non
				// sampled images and buffers)

				if (!ttype->getQualifier().hasBinding())
				{
					log = "Uniform parsing error: Found an uniform without a binding qualifier. Each uniform must have an "
						"explicitly defined binding number.";

					return false;
				}

				const glslang::TSampler& sampler = ttype->getSampler();

				GpuParamObjectDesc param;
				param.name = name;
				param.slot = ttype->getQualifier().layoutBinding;
				param.set = ttype->getQualifier().layoutSet;

				if (param.set == glslang::TQualifier::layoutSetEnd)
					param.set = 0;

				if (sampler.isImage())
				{
					switch (sampler.dim)
					{
					case glslang::Esd1D:		param.type = GPOT_RWTEXTURE1D; break;
					case glslang::Esd2D:		param.type = sampler.isMultiSample() ? GPOT_RWTEXTURE2DMS : GPOT_RWTEXTURE2D; break;
					case glslang::Esd3D:		param.type = GPOT_RWTEXTURE3D; break;
					case glslang::EsdBuffer:	param.type = GPOT_RWSTRUCTURED_BUFFER; break;
					}

					if(sampler.dim != glslang::EsdBuffer)
						desc.loadStoreTextures[name] = param;
					else
						desc.buffers[name] = param;
				}
				else
				{
					switch (sampler.dim)
					{
					case glslang::Esd1D:		param.type = GPOT_SAMPLER1D; break;
					case glslang::Esd2D:		param.type = sampler.isMultiSample() ? GPOT_SAMPLER2DMS : GPOT_SAMPLER2D; break;
					case glslang::Esd3D:		param.type = GPOT_SAMPLER3D; break;
					case glslang::EsdCube:		param.type = GPOT_SAMPLERCUBE; break;
					}

					desc.samplers[name] = param;

					if (!sampler.isPureSampler())
					{
						switch (sampler.dim)
						{
						case glslang::Esd1D:		param.type = GPOT_TEXTURE1D; break;
						case glslang::Esd2D:		param.type = sampler.isMultiSample() ? GPOT_TEXTURE2DMS : GPOT_TEXTURE2D; break;
						case glslang::Esd3D:		param.type = GPOT_TEXTURE3D; break;
						case glslang::EsdCube:		param.type = GPOT_TEXTURECUBE; break;
						case glslang::EsdBuffer:	param.type = GPOT_STRUCTURED_BUFFER; break;
						}

						if (sampler.dim != glslang::EsdBuffer)
							desc.textures[name] = param;
						else
							desc.buffers[name] = param;
					}
				}

				if(param.type == GPOT_UNKNOWN)
					LOGERR("Cannot determine type for uniform: " + String(name));
			}
			else
			{
				if(ttype->getBasicType() == glslang::EbtStruct)
				{
					// Not handling structs at the moment
				}
				else
				{
					GpuParamDataType paramType = mapGLSLangToGpuParamDataType(*ttype);

					if (paramType == GPDT_UNKNOWN)
					{
						LOGWRN("Cannot determine type for uniform: " + String(name));
						continue;
					}

					int blockIdx = program->getUniformBlockIndex(i);
					auto iterFind = uniformBlockMap.find(blockIdx);
					if (iterFind == uniformBlockMap.end())
						LOGERR("Uniform is referencing a uniform block that doesn't exist: " + String(name));

					const GpuParamBlockDesc& paramBlockDesc = desc.paramBlocks[iterFind->second];
					const GpuParamDataTypeInfo& typeInfo = GpuParams::PARAM_SIZES.lookup[paramType];
					int bufferOffset = program->getUniformBufferOffset(i) / 4;

					GpuParamDataDesc param;
					param.name = name;
					param.type = paramType;
					param.paramBlockSet = paramBlockDesc.set;
					param.paramBlockSlot = paramBlockDesc.slot;
					param.elementSize = typeInfo.size / 4;
					param.arrayElementStride = param.elementSize;
					param.arraySize = program->getUniformArraySize(i);
					param.cpuMemOffset = bufferOffset;
					param.gpuMemOffset = bufferOffset;

					desc.params[name] = param;
				}
			}
		}

		return true;
	}

	VulkanShaderModule::VulkanShaderModule(VulkanResourceManager* owner, VkShaderModule module)
		:VulkanResource(owner, true), mModule(module)
	{ }

	VulkanShaderModule::~VulkanShaderModule()
	{
		vkDestroyShaderModule(mOwner->getDevice().getLogical(), mModule, gVulkanAllocator);
	}

	VulkanGpuProgram::VulkanGpuProgram(const GPU_PROGRAM_DESC& desc, GpuDeviceFlags deviceMask)
		: GpuProgramCore(desc, deviceMask), mDeviceMask(deviceMask), mModules()
	{

	}

	VulkanGpuProgram::~VulkanGpuProgram()
	{
		for (UINT32 i = 0; i < BS_MAX_DEVICES; i++)
		{
			if (mModules[i] != nullptr)
				mModules[i]->destroy();
		}

		BS_INC_RENDER_STAT_CAT(ResDestroyed, RenderStatObject_GpuProgram);
	}

	void VulkanGpuProgram::initialize()
	{
		if (!isSupported())
		{
			mIsCompiled = false;
			mCompileError = "Specified program is not supported by the current render system.";

			GpuProgramCore::initialize();
			return;
		}
		
		TBuiltInResource resources = DefaultTBuiltInResource;
		glslang::TProgram* program = new glslang::TProgram;

		EShLanguage glslType;
		switch(mProperties.getType())
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
		}

		std::vector<UINT32> spirv;
		spv::SpvBuildLogger logger;
		std::string compileLog;

		const String& source = mProperties.getSource();
		const char* sourceBytes = source.c_str();

		glslang::TShader* shader = new glslang::TShader(glslType);
		shader->setStrings(&sourceBytes, 1);
		shader->setEntryPoint("main");

		EShMessages messages = (EShMessages)((int)EShMsgSpvRules | (int)EShMsgVulkanRules);
		if (!shader->parse(&resources, 450, false, messages))
		{
			mIsCompiled = false;
			mCompileError = "Compile error: " + String(shader->getInfoLog());

			goto cleanup;
		}

		program->addShader(shader);

		if (!program->link(messages))
		{
			mIsCompiled = false;
			mCompileError = "Link error: " + String(program->getInfoLog());

			goto cleanup;
		}

		program->mapIO();
		program->buildReflection();

		// Compile to SPIR-V
		GlslangToSpv(*program->getIntermediate(glslType), spirv, &logger);

		// Parse uniforms
		if(!parseUniforms(program, *mParametersDesc, mCompileError))
		{
			mIsCompiled = false;
			goto cleanup;
		}

		// If vertex program, retrieve information about vertex inputs
		if (mProperties.getType() == GPT_VERTEX_PROGRAM)
		{
			List<VertexElement> elementList;
					
			if (parseVertexAttributes(program, elementList, mCompileError))
				mInputDeclaration = HardwareBufferManager::instance().createVertexDeclaration(elementList, mDeviceMask);
			else
			{
				mIsCompiled = false;
				goto cleanup;
			}
		}

		// Create Vulkan module
		VkShaderModuleCreateInfo moduleCI;
		moduleCI.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
		moduleCI.pNext = nullptr;
		moduleCI.flags = 0;
		moduleCI.codeSize = spirv.size() * sizeof(UINT32);
		moduleCI.pCode = spirv.data();

		VulkanRenderAPI& rapi = static_cast<VulkanRenderAPI&>(RenderAPI::instance());

		VulkanDevice* devices[BS_MAX_DEVICES];
		VulkanUtility::getDevices(rapi, mDeviceMask, devices);

		for (UINT32 i = 0; i < BS_MAX_DEVICES; i++)
		{
			if (devices[i] != nullptr)
			{
				VkDevice vkDevice = devices[i]->getLogical();
				VulkanResourceManager& rescManager = devices[i]->getResourceManager();

				VkShaderModule shaderModule;
				VkResult result = vkCreateShaderModule(vkDevice, &moduleCI, gVulkanAllocator, &shaderModule);
				assert(result == VK_SUCCESS);

				mModules[i] = rescManager.create<VulkanShaderModule>(shaderModule);
			}
		}

		mIsCompiled = true;

cleanup:
		delete program;
		delete shader;

		BS_INC_RENDER_STAT_CAT(ResCreated, RenderStatObject_GpuProgram);

		GpuProgramCore::initialize();
	}
}}