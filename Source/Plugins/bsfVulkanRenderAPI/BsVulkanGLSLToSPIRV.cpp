//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsVulkanGLSLToSPIRV.h"
#include "BsVulkanGpuProgram.h"
#include "BsVulkanUtility.h"
#include "RenderAPI/BsGpuParameters.h"
#include "RenderAPI/BsGpuProgramParameterDescription.h"
#include "Math/BsMath.h"

#define AMD_EXTENSIONS
#define NV_EXTENSIONS
#include "glslang/Public/ShaderLang.h"
#include "glslang/Include/Types.h"
#include "glslang/SPIRV/GlslangToSpv.h"
#include "glslang/Include/revision.h"
#include "spirv_cross/spirv_cross.hpp"
#include "Utility/BsScopeGuard.h"

using namespace bs;
using namespace bs::ct;

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

static VertexElementType MapGLSLangToVertexElemType(const glslang::TType& type)
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
		case glslang::EbtFloat16:
			switch(vectorSize)
			{
			case 2: return VET_HALF2;
			case 3: return VET_HALF3;
			case 4: return VET_HALF4;
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
		case glslang::EbtFloat16: return VET_HALF1;
		case glslang::EbtInt: return VET_INT1;
		case glslang::EbtUint: return VET_UINT1;
		default: return VET_UNKNOWN;
		}
	}

	return VET_UNKNOWN;
}

static GpuDataParameterType SPIRVCrossTypeToGpuDataParameterType(const spirv_cross::SPIRType& type)
{
	if(type.basetype == spirv_cross::SPIRType::Struct)
		return GPDT_STRUCT;

	if(type.basetype == spirv_cross::SPIRType::Float)
	{
		if(type.columns == 1)
		{
			switch(type.vecsize)
			{
			case 1: return GPDT_FLOAT1;
			case 2: return GPDT_FLOAT2;
			case 3: return GPDT_FLOAT3;
			case 4: return GPDT_FLOAT4;
			}
		}
		else if(type.columns == 2)
		{
			switch(type.vecsize)
			{
			case 2: return GPDT_MATRIX_2X2;
			case 3: return GPDT_MATRIX_3X2;
			case 4: return GPDT_MATRIX_4X2;
			}
		}
		else if(type.columns == 3)
		{
			switch(type.vecsize)
			{
			case 2: return GPDT_MATRIX_2X3;
			case 3: return GPDT_MATRIX_3X3;
			case 4: return GPDT_MATRIX_4X3;
			}
		}
		else if(type.columns == 4)
		{
			switch(type.vecsize)
			{
			case 2: return GPDT_MATRIX_2X3;
			case 3: return GPDT_MATRIX_3X4;
			case 4: return GPDT_MATRIX_4X4;
			}
		}
	}
	else if(type.basetype == spirv_cross::SPIRType::Double)
	{
		if(type.columns == 1)
		{
			switch(type.vecsize)
			{
			case 1: return GPDT_DOUBLE1;
			case 2: return GPDT_DOUBLE2;
			case 3: return GPDT_DOUBLE3;
			case 4: return GPDT_DOUBLE4;
			}
		}
		else if(type.columns == 2)
		{
			switch(type.vecsize)
			{
			case 2: return GPDT_DOUBLE_MATRIX_2X2;
			case 3: return GPDT_DOUBLE_MATRIX_3X2;
			case 4: return GPDT_DOUBLE_MATRIX_4X2;
			}
		}
		else if(type.columns == 3)
		{
			switch(type.vecsize)
			{
			case 2: return GPDT_DOUBLE_MATRIX_2X3;
			case 3: return GPDT_DOUBLE_MATRIX_3X3;
			case 4: return GPDT_DOUBLE_MATRIX_4X3;
			}
		}
		else if(type.columns == 4)
		{
			switch(type.vecsize)
			{
			case 2: return GPDT_DOUBLE_MATRIX_2X3;
			case 3: return GPDT_DOUBLE_MATRIX_3X4;
			case 4: return GPDT_DOUBLE_MATRIX_4X4;
			}
		}
	}
	else if(type.basetype == spirv_cross::SPIRType::Half)
	{
		if(type.columns == 1)
		{
			switch(type.vecsize)
			{
			case 1: return GPDT_HALF1;
			case 2: return GPDT_HALF2;
			case 3: return GPDT_HALF3;
			case 4: return GPDT_HALF4;
			}
		}
		else if(type.columns == 2)
		{
			switch(type.vecsize)
			{
			case 2: return GPDT_HALF_MATRIX_2X2;
			case 3: return GPDT_HALF_MATRIX_3X2;
			case 4: return GPDT_HALF_MATRIX_4X2;
			}
		}
		else if(type.columns == 3)
		{
			switch(type.vecsize)
			{
			case 2: return GPDT_HALF_MATRIX_2X3;
			case 3: return GPDT_HALF_MATRIX_3X3;
			case 4: return GPDT_HALF_MATRIX_4X3;
			}
		}
		else if(type.columns == 4)
		{
			switch(type.vecsize)
			{
			case 2: return GPDT_HALF_MATRIX_2X3;
			case 3: return GPDT_HALF_MATRIX_3X4;
			case 4: return GPDT_HALF_MATRIX_4X4;
			}
		}
	}
	else if(type.basetype == spirv_cross::SPIRType::Int)
	{
		if(type.columns == 1)
		{
			switch(type.vecsize)
			{
			case 1: return GPDT_INT1;
			case 2: return GPDT_INT2;
			case 3: return GPDT_INT3;
			case 4: return GPDT_INT4;
			}
		}
	}
	else if(type.basetype == spirv_cross::SPIRType::UInt)
	{
		if(type.columns == 1)
		{
			switch(type.vecsize)
			{
			case 1: return GPDT_UINT1;
			case 2: return GPDT_UINT2;
			case 3: return GPDT_UINT3;
			case 4: return GPDT_UINT4;
			}
		}
	}
	else if(type.basetype == spirv_cross::SPIRType::Boolean)
	{
		if(type.columns == 1 && type.vecsize == 1)
			return GPDT_BOOL;
	}

	return GPDT_UNKNOWN;
}

static GpuBufferFormat SPIRVCrossTypeToBufferFormat(const spirv_cross::SPIRType& type)
{
	switch(type.basetype)
	{
	case spirv_cross::SPIRType::Float:
		switch(type.vecsize)
		{
		case 1: return BF_32X1F;
		case 2: return BF_32X2F;
		case 3: return BF_32X3F;
		case 4: return BF_32X4F;
		default: return BF_UNKNOWN;
		}
	case spirv_cross::SPIRType::Double:
		switch(type.vecsize)
		{
		case 1: return BF_64X1F;
		case 2: return BF_64X2F;
		case 3: return BF_64X3F;
		case 4: return BF_64X4F;
		default: return BF_UNKNOWN;
		}
	case spirv_cross::SPIRType::Half:

		switch(type.vecsize)
		{
		case 1: return BF_16X1F;
		case 2: return BF_16X2F;
		case 4: return BF_16X4F;
		default: return BF_UNKNOWN;
		}
	case spirv_cross::SPIRType::UShort:
		switch(type.vecsize)
		{
		case 1: return BF_16X1U;
		case 2: return BF_16X2U;
		case 4: return BF_16X4U;
		default: return BF_UNKNOWN;
		}
	case spirv_cross::SPIRType::Short:
		switch(type.vecsize)
		{
		case 1: return BF_16X1S;
		case 2: return BF_16X2S;
		case 4: return BF_16X4S;
		default: return BF_UNKNOWN;
		}
	case spirv_cross::SPIRType::UByte:
		switch(type.vecsize)
		{
		case 1: return BF_8X1U;
		case 2: return BF_8X2U;
		case 4: return BF_8X4U;
		default: return BF_UNKNOWN;
		}
	case spirv_cross::SPIRType::SByte:
		switch(type.vecsize)
		{
		case 1: return BF_8X1S;
		case 2: return BF_8X2S;
		case 4: return BF_8X4S;
		default: return BF_UNKNOWN;
		}
	case spirv_cross::SPIRType::Int:
		switch(type.vecsize)
		{
		case 1: return BF_32X1S;
		case 2: return BF_32X2S;
		case 3: return BF_32X3S;
		case 4: return BF_32X4S;
		default: return BF_UNKNOWN;
		}
	case spirv_cross::SPIRType::UInt:
		switch(type.vecsize)
		{
		case 1: return BF_32X1U;
		case 2: return BF_32X2U;
		case 3: return BF_32X3U;
		case 4: return BF_32X4U;
		default: return BF_UNKNOWN;
		}
	case spirv_cross::SPIRType::Int64:
		switch(type.vecsize)
		{
		case 1: return BF_64X1S;
		case 2: return BF_64X2S;
		case 3: return BF_64X3S;
		case 4: return BF_64X4S;
		default: return BF_UNKNOWN;
		}
	case spirv_cross::SPIRType::UInt64:
		switch(type.vecsize)
		{
		case 1: return BF_64X1U;
		case 2: return BF_64X2U;
		case 3: return BF_64X3U;
		case 4: return BF_64X4U;
		default: return BF_UNKNOWN;
		}
	default:
		return BF_UNKNOWN;
	}
}

static GpuDataParameterType MapGLSLangToGpuParamDataType(const glslang::TType& type)
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
		case glslang::EbtDouble:
			switch(vectorSize)
			{
			case 2: return GPDT_DOUBLE2;
			case 3: return GPDT_DOUBLE3;
			case 4: return GPDT_DOUBLE4;
			default: return GPDT_UNKNOWN;
			}
		case glslang::EbtFloat16:
			switch(vectorSize)
			{
			case 2: return GPDT_HALF2;
			case 3: return GPDT_HALF3;
			case 4: return GPDT_HALF4;
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
			case 2: return GPDT_UINT2;
			case 3: return GPDT_UINT3;
			case 4: return GPDT_UINT4;
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
		case glslang::EbtFloat16:
			switch(type.getMatrixCols())
			{
			case 2:
				switch(type.getMatrixRows())
				{
				case 2: return GPDT_HALF_MATRIX_2X2;
				case 3: return GPDT_HALF_MATRIX_3X2;
				case 4: return GPDT_HALF_MATRIX_4X2;
				default: return GPDT_UNKNOWN;
				}
			case 3:
				switch(type.getMatrixRows())
				{
				case 2: return GPDT_HALF_MATRIX_2X3;
				case 3: return GPDT_HALF_MATRIX_3X3;
				case 4: return GPDT_HALF_MATRIX_4X3;
				default: return GPDT_UNKNOWN;
				}
			case 4:
				switch(type.getMatrixRows())
				{
				case 2: return GPDT_HALF_MATRIX_2X4;
				case 3: return GPDT_HALF_MATRIX_3X4;
				case 4: return GPDT_HALF_MATRIX_4X4;
				default: return GPDT_UNKNOWN;
				}
			default: break;
			}
			break;
		case glslang::EbtDouble:
			switch(type.getMatrixCols())
			{
			case 2:
				switch(type.getMatrixRows())
				{
				case 2: return GPDT_DOUBLE_MATRIX_2X2;
				case 3: return GPDT_DOUBLE_MATRIX_3X2;
				case 4: return GPDT_DOUBLE_MATRIX_4X2;
				default: return GPDT_UNKNOWN;
				}
			case 3:
				switch(type.getMatrixRows())
				{
				case 2: return GPDT_DOUBLE_MATRIX_2X3;
				case 3: return GPDT_DOUBLE_MATRIX_3X3;
				case 4: return GPDT_DOUBLE_MATRIX_4X3;
				default: return GPDT_UNKNOWN;
				}
			case 4:
				switch(type.getMatrixRows())
				{
				case 2: return GPDT_DOUBLE_MATRIX_2X4;
				case 3: return GPDT_DOUBLE_MATRIX_3X4;
				case 4: return GPDT_DOUBLE_MATRIX_4X4;
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
		case glslang::EbtDouble: return GPDT_DOUBLE1;
		case glslang::EbtFloat16: return GPDT_HALF1;
		case glslang::EbtInt: return GPDT_INT1;
		case glslang::EbtUint: return GPDT_UINT1;
		case glslang::EbtBool: return GPDT_BOOL;
		default: return GPDT_UNKNOWN;
		}
	}

	return GPDT_UNKNOWN;
}

static GpuBufferFormat MapSamplerBasicType(const glslang::TSampler& sampler)
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
	case glslang::EbtDouble:
		switch(vectorSize)
		{
		case 1: return BF_64X1F;
		case 2: return BF_64X2F;
		case 3: return BF_64X3F;
		case 4: return BF_64X4F;
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
	case glslang::EbtInt64:
		switch(vectorSize)
		{
		case 1: return BF_64X1S;
		case 2: return BF_64X2S;
		case 3: return BF_64X3S;
		case 4: return BF_64X4S;
		default: return BF_UNKNOWN;
		}
	case glslang::EbtUint64:
		switch(vectorSize)
		{
		case 1: return BF_64X1U;
		case 2: return BF_64X2U;
		case 3: return BF_64X3U;
		case 4: return BF_64X4U;
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
		return Parsei32(name.substr(length));
	}

	/**	Returns the semantic of this attribute. */
	VertexElementSemantic GetSemantic() const { return mSemantic; }

private:
	String mName;
	VertexElementSemantic mSemantic;
};

static bool AttributeNameToElementSemantic(const String& name, VertexElementSemantic& semantic, u16& index)
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

static bool ParseVertexAttributes(const glslang::TProgram* program, Vector<VertexElement>& elementList, String& log)
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
		if(AttributeNameToElementSemantic(attribName, semantic, index))
		{
			VertexElementType type = MapGLSLangToVertexElemType(*ttype);
			if(type == VET_UNKNOWN)
				B3D_LOG(Error, RenderBackend, "Cannot determine vertex input attribute type for attribute: {0}", attribName);

			elementList.push_back(VertexElement(type, semantic, index, 0, 0, location));
		}
		else
		{
			// Ignore built-in attributes
			if(memcmp(attribName, "gl_", 3) != 0)
			{
				B3D_LOG(Error, RenderBackend, "Cannot determine vertex input attribute semantic for attribute: {0}", attribName);
			}
		}
	}

	return true;
}

static void ParseStruct(const glslang::TTypeList* typeList, u32& size)
{
	for(auto iter = typeList->begin(); iter != typeList->end(); ++iter)
	{
		const glslang::TType* ttype = iter->type;

		if(ttype->getBasicType() == glslang::EbtStruct)
		{
			const glslang::TTypeList* childTypeList = ttype->getStruct();
			ParseStruct(childTypeList, size);
		}
		else
		{
			u32 arraySize = 1;
			if(ttype->isArray())
				arraySize = (u32)ttype->getCumulativeArraySize();

			GpuDataParameterType paramType = MapGLSLangToGpuParamDataType(*ttype);
			if(paramType == GPDT_UNKNOWN)
			{
				B3D_LOG(Warning, RenderBackend, "Cannot determine type for uniform inside a struct.");
				continue;
			}

			u32 elemSize = VulkanUtility::CalcInterfaceBlockElementSizeAndOffset(paramType, arraySize, size);
			size += elemSize;
		}
	}
}

static bool ParseUniforms(const glslang::TProgram* program, GpuProgramParameterDescription& desc, String& log)
{
	// Parse individual uniforms
	struct UniformInfo
	{
		u32 BufferOffset;
		u32 ArraySize;
	};

	UnorderedMap<String, UniformInfo> uniforms;

	int numUniforms = program->getNumUniformVariables();
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

			GpuObjectParameterInformation param;
			param.Name = name;
			param.Slot = qualifier.layoutBinding;
			param.Set = qualifier.layoutSet;
			param.Type = GPOT_UNKNOWN;
			param.ElementType = MapSamplerBasicType(sampler);

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
					desc.StorageTextures[name] = param;
				else
					desc.Buffers[name] = param;

				if(ttype->getArraySizes() != nullptr)
					param.ArraySize = (u32)ttype->getCumulativeArraySize();
				else
					param.ArraySize = 1;
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
						desc.SampledTextures[name] = param;
					else
						desc.Buffers[name] = param;

					if(ttype->getArraySizes() != nullptr)
						param.ArraySize = (u32)ttype->getCumulativeArraySize();
					else
						param.ArraySize = 1;
				}
			}

			if(param.Type == GPOT_UNKNOWN)
				B3D_LOG(Error, RenderBackend, "Cannot determine type for uniform: {0}", name);
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
	int numBlocks = program->getNumUniformBlocks();
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
			GpuObjectParameterInformation param;
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

			GpuDataParameterBlockInformation blockDesc;
			blockDesc.Name = name;
			blockDesc.BlockSize = size / 4;
			blockDesc.IsShareable = true;
			blockDesc.Slot = qualifier.layoutBinding;
			blockDesc.Set = qualifier.layoutSet;

			if(blockDesc.Set == glslang::TQualifier::layoutSetEnd)
				blockDesc.Set = 0;

			desc.UniformBuffers[name] = blockDesc;

			// Parse members of the uniform buffer
			const glslang::TTypeList* typeList = ttype->getStruct();
			if(typeList == nullptr)
				continue;

			u32 bufferOffset = 0;
			for(auto iter = typeList->begin(); iter != typeList->end(); ++iter)
			{
				const glslang::TType* paramTType = iter->type;
				String paramName = paramTType->getFieldName().c_str();

				GpuDataParameterType paramType;
				u32 elementSize = 0;
				u32 arrayStride = 0;
				if(paramTType->getBasicType() == glslang::EbtStruct)
				{
					paramType = GPDT_STRUCT;

					const glslang::TTypeList* paramTypeList = paramTType->getStruct();
					ParseStruct(paramTypeList, elementSize);

					// Struct alignment always a multiple of vec4
					arrayStride = Math::DivideAndRoundUp(elementSize, 4U) * 4;
				}
				else
				{
					paramType = MapGLSLangToGpuParamDataType(*paramTType);
				}

				if(paramType == GPDT_UNKNOWN)
				{
					B3D_LOG(Warning, RenderBackend, "Cannot determine type for uniform: {0}", name);
					continue;
				}

				u32 arraySize = paramTType->isArray() ? paramTType->getCumulativeArraySize() : 1;
				if(paramType != GPDT_STRUCT)
				{
					const GpuDataParameterTypeInformation& typeInfo = bs::GpuParameters::kParamSizes.Lookup[paramType];
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
					stride = Math::DivideAndRoundUp(elementSize, 4U) * 4;
					bufferOffset = Math::DivideAndRoundUp(bufferOffset, 4U) * 4;
				}
				else if(paramTType->isMatrix())
				{
					// Matrices get rounded up to vec4
					const GpuDataParameterTypeInformation& typeInfo = bs::GpuParameters::kParamSizes.Lookup[paramType];

					stride = Math::DivideAndRoundUp(typeInfo.BaseTypeSize * typeInfo.NumColumns / 4U, 4U) * 4 * typeInfo.NumRows;
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
					B3D_ASSERT((uniformInfo.BufferOffset / 4) == bufferOffset);
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
					GpuDataParameterInformation paramDesc;
					paramDesc.Name = paramName;
					paramDesc.Type = paramType;
					paramDesc.ParamBlockSet = blockDesc.Set;
					paramDesc.ParamBlockSlot = blockDesc.Slot;
					paramDesc.ElementSize = elementSize;
					paramDesc.ArrayElementStride = arrayStride;
					paramDesc.ArraySize = arraySize;
					paramDesc.CpuOffset = bufferOffset;
					paramDesc.GpuOffset = bufferOffset;


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

static Optional<GpuDataParameterInformation> ParseSPIRVCrossStructMember(spirv_cross::Compiler& compiler, const spirv_cross::SPIRType& structType, u32 memberIndex, StringStream& outLog)
{
	const spirv_cross::SPIRType& memberType = compiler.get_type(structType.member_types[memberIndex]);
	u32 memberSize = (u32)compiler.get_declared_struct_member_size(structType, memberIndex);
	u32 memberOffset = compiler.type_struct_member_offset(structType, memberIndex);

	if(!B3D_ENSURE(memberSize % 4 == 0))
		return {};

	if(!B3D_ENSURE(memberOffset % 4 == 0))
		return {};

	memberSize /= 4;
	memberOffset /= 4;

	u32 arrayStride = memberSize;
	if(!memberType.array.empty())
	{
		arrayStride = compiler.type_struct_member_array_stride(structType, memberIndex);

		if(!B3D_ENSURE(arrayStride % 4 == 0))
			return {};

		arrayStride /= 4;
	}

	const std::string& name = compiler.get_member_name(structType.self, memberIndex);

	GpuDataParameterInformation memberInformation;
	memberInformation.Name = name;
	memberInformation.Type = SPIRVCrossTypeToGpuDataParameterType(memberType);
	memberInformation.ParamBlockSet = 0; // Must be assigned by the caller
	memberInformation.ParamBlockSlot = 0; // Must be assigned by the caller
	memberInformation.ElementSize = memberSize;
	memberInformation.ArrayElementStride = arrayStride;
	memberInformation.ArraySize = memberType.array.empty() ? 1 : memberType.array[0]; // Not supporting array of arrays
	memberInformation.CpuOffset = memberOffset;
	memberInformation.GpuOffset = memberOffset;

	if(memberInformation.Type == GPDT_UNKNOWN)
	{
		outLog << StringUtil::Format("Warning: Failed parsing uniform. Cannot determine type for uniform: {0}.\n", memberInformation.Name);
		return {};
	}

	return memberInformation;
}
static Optional<GpuDataParameterBlockInformation> ParseSPIRVCrossUniformBuffer(spirv_cross::Compiler& compiler, const spirv_cross::Resource& resource, StringStream& outLog)
{
	if(!compiler.get_decoration_bitset(resource.id).get(spv::DecorationBinding))
	{
		outLog << StringUtil::Format("Warning: Failed parsing uniform {0}. Uniform has no slot binding assigned. Each uniform must have a slot binding otherwise it cannot be assigned.", resource.name.c_str());
		return {};
	}

	if(!compiler.get_decoration_bitset(resource.id).get(spv::DecorationDescriptorSet))
	{
		outLog << StringUtil::Format("Warning: Failed parsing uniform {0}. Uniform has no set binding assigned. Each uniform must have a set binding otherwise it cannot be assigned.", resource.name.c_str());
		return {};
	}

	const spirv_cross::SPIRType& type = compiler.get_type(resource.base_type_id);
	const u32 bufferSize = (u32)compiler.get_declared_struct_size(type);

	GpuDataParameterBlockInformation uniformBufferInformation;
	uniformBufferInformation.Name = resource.name;
	uniformBufferInformation.BlockSize = Math::CeilToMultiple(bufferSize / 4u, 4u); // Round to multiple of 16 bytes
	uniformBufferInformation.IsShareable = true;
	uniformBufferInformation.Slot = compiler.get_decoration(resource.id, spv::DecorationBinding);
	uniformBufferInformation.Set = compiler.get_decoration(resource.id, spv::DecorationDescriptorSet);

	return uniformBufferInformation;
}

/** Parses common object information (name, binding) from a SPIRVCross resource. Helper to be used by other parsing methods. */
static Optional<GpuObjectParameterInformation> ParseSPIRVCrossObjectCommon(spirv_cross::Compiler& compiler, const spirv_cross::Resource& resource, const spirv_cross::SPIRType& type, StringStream& outLog)
{
	if(!compiler.get_decoration_bitset(resource.id).get(spv::DecorationBinding))
	{
		outLog << StringUtil::Format("Warning: Failed parsing uniform {0}. Uniform has no slot binding assigned. Each uniform must have a slot binding otherwise it cannot be assigned.", resource.name.c_str());
		return {};
	}

	if(!compiler.get_decoration_bitset(resource.id).get(spv::DecorationDescriptorSet))
	{
		outLog << StringUtil::Format("Warning: Failed parsing uniform {0}. Uniform has no set binding assigned. Each uniform must have a set binding otherwise it cannot be assigned.", resource.name.c_str());
		return {};
	}

	if(type.array.size() > 1)
	{
		outLog << StringUtil::Format("Warning: Failed parsing uniform {0}. Multi-dimension arrays are not supported.", resource.name.c_str());
		return {};
	}

	GpuObjectParameterInformation objectInformation;
	objectInformation.Type = GPOT_UNKNOWN;
	objectInformation.Name = resource.name;
	objectInformation.Slot = compiler.get_decoration(resource.id, spv::DecorationBinding);
	objectInformation.Set = compiler.get_decoration(resource.id, spv::DecorationDescriptorSet);
	objectInformation.ArraySize = type.array.empty() ? 1 : type.array[0];

	return objectInformation;
}

static Optional<GpuObjectParameterInformation> ParseSPIRVCrossSampledTexture(spirv_cross::Compiler& compiler, const spirv_cross::Resource& resource, StringStream& outLog)
{
	const spirv_cross::SPIRType& type = compiler.get_type(resource.base_type_id);
	Optional<GpuObjectParameterInformation> objectInformation = ParseSPIRVCrossObjectCommon(compiler, resource, type, outLog);
	if(!objectInformation)
		return {};

	const spirv_cross::SPIRType::ImageType imageTypeInformation = type.image;
	if(!B3D_ENSURE(imageTypeInformation.sampled == 1)) // 1 means sampled images, 2 means load/store, 0 means nothing
		return {};

	switch(type.image.dim)
	{
	case spv::Dim1D:
		objectInformation->Type = imageTypeInformation.arrayed ? GPOT_TEXTURE1DARRAY : GPOT_TEXTURE1D;
		break;
	case spv::Dim2D:
		if(imageTypeInformation.ms)
			objectInformation->Type = imageTypeInformation.arrayed ? GPOT_TEXTURE2DMSARRAY : GPOT_TEXTURE2DMS;
		else
			objectInformation->Type = imageTypeInformation.arrayed ? GPOT_TEXTURE2DARRAY : GPOT_TEXTURE2D;
		break;
	case spv::Dim3D:
		objectInformation->Type = GPOT_TEXTURE3D;
		break;
	case spv::DimCube:
		objectInformation->Type = imageTypeInformation.arrayed ? GPOT_TEXTURECUBEARRAY : GPOT_TEXTURECUBE;
		break;
	case spv::DimBuffer:
		objectInformation->Type = GPOT_BYTE_BUFFER;
		break;
	}

	if(objectInformation->Type == GPOT_UNKNOWN)
	{
		outLog << StringUtil::Format("Warning: Failed parsing uniform. Cannot determine type for uniform: {0}.\n", objectInformation->Type);
		return {};
	}

	if(type.image.type != 0)
	{
		const spirv_cross::SPIRType& elementType = compiler.get_type(type.image.type);
		objectInformation->ElementType = SPIRVCrossTypeToBufferFormat(elementType);
	}
	else
	{
		objectInformation->ElementType = BF_UNKNOWN;
	}

	return objectInformation;
}

static Optional<GpuObjectParameterInformation> ParseSPIRVCrossSampler(spirv_cross::Compiler& compiler, const spirv_cross::Resource& resource, StringStream& outLog)
{
	const spirv_cross::SPIRType& type = compiler.get_type(resource.base_type_id);
	Optional<GpuObjectParameterInformation> objectInformation = ParseSPIRVCrossObjectCommon(compiler, resource, type, outLog);
	if(!objectInformation)
		return {};

	objectInformation->Type = GPOT_SAMPLER2D;
	objectInformation->ElementType = BF_UNKNOWN;

	return objectInformation;
}

static Optional<GpuObjectParameterInformation> ParseSPIRVCrossStorageTexture(spirv_cross::Compiler& compiler, const spirv_cross::Resource& resource, StringStream& outLog)
{
	const spirv_cross::SPIRType& type = compiler.get_type(resource.base_type_id);
	Optional<GpuObjectParameterInformation> objectInformation = ParseSPIRVCrossObjectCommon(compiler, resource, type, outLog);
	if(!objectInformation)
		return {};

	const spirv_cross::SPIRType::ImageType imageTypeInformation = type.image;
	if(!B3D_ENSURE(imageTypeInformation.sampled == 2)) // 1 means sampled images, 2 means load/store, 0 means nothing
		return {};

	switch(type.image.dim)
	{
	case spv::Dim1D:
		objectInformation->Type = imageTypeInformation.arrayed ? GPOT_RWTEXTURE1DARRAY : GPOT_RWTEXTURE1D;
		break;
	case spv::Dim2D:
		if(imageTypeInformation.ms)
			objectInformation->Type = imageTypeInformation.arrayed ? GPOT_RWTEXTURE2DMSARRAY : GPOT_RWTEXTURE2DMS;
		else
			objectInformation->Type = imageTypeInformation.arrayed ? GPOT_RWTEXTURE2DARRAY : GPOT_RWTEXTURE2D;
		break;
	case spv::Dim3D:
		objectInformation->Type = GPOT_RWTEXTURE3D;
		break;
	case spv::DimBuffer:
		objectInformation->Type = GPOT_RWBYTE_BUFFER;
		break;
	}

	if(objectInformation->Type == GPOT_UNKNOWN)
	{
		outLog << StringUtil::Format("Warning: Failed parsing uniform. Cannot determine type for uniform: {0}.\n", objectInformation->Type);
		return {};
	}

	if(type.image.type != 0)
	{
		const spirv_cross::SPIRType& elementType = compiler.get_type(type.image.type);
		objectInformation->ElementType = SPIRVCrossTypeToBufferFormat(elementType);
	}
	else
	{
		objectInformation->ElementType = BF_UNKNOWN;
	}

	return objectInformation;
}

static Optional<GpuObjectParameterInformation> ParseSPIRVCrossStorageBuffer(spirv_cross::Compiler& compiler, const spirv_cross::Resource& resource, StringStream& outLog)
{
	const spirv_cross::SPIRType& type = compiler.get_type(resource.base_type_id);
	Optional<GpuObjectParameterInformation> objectInformation = ParseSPIRVCrossObjectCommon(compiler, resource, type, outLog);
	if(!objectInformation)
		return {};

	const spirv_cross::SPIRType::ImageType imageTypeInformation = type.image;
	objectInformation->Type = GPOT_RWSTRUCTURED_BUFFER;

	if(type.image.type != 0)
	{
		const spirv_cross::SPIRType& elementType = compiler.get_type(type.image.type);
		objectInformation->ElementType = SPIRVCrossTypeToBufferFormat(elementType);
	}
	else
	{
		objectInformation->ElementType = BF_UNKNOWN;
	}

	return objectInformation;
}

static void ParseSPIRVCrossUniforms(spirv_cross::Compiler& compiler, GpuProgramParameterDescription& outParameterDescription, StringStream& outLog)
{
	const spirv_cross::ShaderResources& sprivResources = compiler.get_shader_resources();
	for(const auto& resource : sprivResources.uniform_buffers)
	{
		Optional<GpuDataParameterBlockInformation> uniformBufferInformation = ParseSPIRVCrossUniformBuffer(compiler, resource, outLog);
		if(!uniformBufferInformation)
			continue;

		const spirv_cross::SPIRType& type = compiler.get_type(resource.base_type_id);
		const u32 memberCount = (u32)type.member_types.size();
		for(u32 memberIndex = 0; memberIndex < memberCount; memberIndex++)
		{
			Optional<GpuDataParameterInformation> memberInformation = ParseSPIRVCrossStructMember(compiler, type, memberIndex, outLog);
			if(!memberInformation)
				continue;

			memberInformation->ParamBlockSet = uniformBufferInformation->Set;
			memberInformation->ParamBlockSlot = uniformBufferInformation->Slot;

			outParameterDescription.UniformBufferMembers[memberInformation->Name] = std::move(memberInformation.value());
		}

		outParameterDescription.UniformBuffers[uniformBufferInformation->Name] = std::move(uniformBufferInformation.value());
	}

	// Combined texture/sampler (e.g. sampler2D)
	for(const auto& resource : sprivResources.sampled_images)
	{
		Optional<GpuObjectParameterInformation> sampledImageInformation = ParseSPIRVCrossSampledTexture(compiler, resource, outLog);
		if(sampledImageInformation)
		{
			if(sampledImageInformation->Type == GPOT_BYTE_BUFFER)
				outParameterDescription.Buffers[sampledImageInformation->Name] = std::move(sampledImageInformation.value());
			else
				outParameterDescription.SampledTextures[sampledImageInformation->Name] = std::move(sampledImageInformation.value());
		}

		Optional<GpuObjectParameterInformation> samplerInformation = ParseSPIRVCrossSampler(compiler, resource, outLog);
		if(samplerInformation)
			outParameterDescription.Samplers[samplerInformation->Name] = std::move(samplerInformation.value());
	}

	// Sampled texture or buffer (e.g. texture2D/Texture2D, samplerBuffer/Buffer)
	for(const auto& resource : sprivResources.separate_images)
	{
		Optional<GpuObjectParameterInformation> sampledImageInformation = ParseSPIRVCrossSampledTexture(compiler, resource, outLog);
		if(sampledImageInformation)
		{
			if(sampledImageInformation->Type == GPOT_BYTE_BUFFER)
				outParameterDescription.Buffers[sampledImageInformation->Name] = std::move(sampledImageInformation.value());
			else
				outParameterDescription.SampledTextures[sampledImageInformation->Name] = std::move(sampledImageInformation.value());
		}
	}

	// Storage texture or buffer (e.g. image2D/RWTexture2D, imageBuffer/RWBuffer)
	for(const auto& resource : sprivResources.storage_images)
	{
		Optional<GpuObjectParameterInformation> sampledImageInformation = ParseSPIRVCrossStorageTexture(compiler, resource, outLog);
		if(sampledImageInformation)
		{
			if(sampledImageInformation->Type == GPOT_RWBYTE_BUFFER)
				outParameterDescription.Buffers[sampledImageInformation->Name] = std::move(sampledImageInformation.value());
			else
				outParameterDescription.StorageTextures[sampledImageInformation->Name] = std::move(sampledImageInformation.value());
		}
	}

	// Samplers (sampler/SamplerState)
	for(const auto& resource : sprivResources.separate_samplers)
	{
		Optional<GpuObjectParameterInformation> samplerInformation = ParseSPIRVCrossSampler(compiler, resource, outLog);
		if(samplerInformation)
			outParameterDescription.Samplers[samplerInformation->Name] = std::move(samplerInformation.value());
	}

	// Structured buffers (e.g. buffer/StructuredBuffer/RWStructuredBuffer)
	for(const auto& resource : sprivResources.storage_buffers)
	{
		Optional<GpuObjectParameterInformation> storageBufferInformation = ParseSPIRVCrossStorageBuffer(compiler, resource, outLog);
		if(storageBufferInformation)
			outParameterDescription.Buffers[storageBufferInformation->Name] = std::move(storageBufferInformation.value());
	}
}

SPtr<GpuProgramBytecode> GLSLToSPIRV::Convert(const GpuProgramCreateInformation& desc)
{
	TBuiltInResource resources = DefaultTBuiltInResource;
	glslang::TProgram program;

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

	glslang::TShader shader(glslType);
	shader.setEnvTarget(glslang::EShTargetSpv, glslang::EShTargetSpv_1_3);
	shader.setStrings(&sourceBytes, 1);
	shader.setEntryPoint("main");

	SPtr<GpuProgramBytecode> bytecode = B3DMakeShared<GpuProgramBytecode>();
	bytecode->CompilerId = VULKAN_COMPILER_ID;
	bytecode->CompilerVersion = VULKAN_COMPILER_VERSION;

	EShMessages messages = (EShMessages)((int)EShMsgSpvRules | (int)EShMsgVulkanRules);
	if(!shader.parse(&resources, 450, false, messages))
	{
		bytecode->Messages = "Compile error: " + String(shader.getInfoLog());
		return bytecode;
	}

	program.addShader(&shader);

	if(!program.link(messages))
	{
		bytecode->Messages = "Link error: " + String(program.getInfoLog());
		return bytecode;
	}

	program.mapIO();
	program.buildReflection();

	// Compile to SPIR-V
	GlslangToSpv(*program.getIntermediate(glslType), spirv, &logger);

	spirv_cross::Compiler spirvCompiler(spirv);

	// Parse uniforms
	bytecode->ParameterDescription = B3DMakeShared<GpuProgramParameterDescription>();
	//if(!ParseUniforms(&program, *bytecode->ParameterDescription, bytecode->Messages))
	//	return bytecode;

	StringStream messageLog;
	ParseSPIRVCrossUniforms(spirvCompiler, *bytecode->ParameterDescription, messageLog);

	const String& messageLogString = messageLog.str();
	if(!messageLogString.empty())
		bytecode->Messages += messageLogString;

	// If vertex program, retrieve information about vertex inputs
	if(desc.Type == GPT_VERTEX_PROGRAM)
	{
		if(!ParseVertexAttributes(&program, bytecode->VertexInput, bytecode->Messages))
			return bytecode;
	}

	bytecode->Instructions.Size = (u32)spirv.size() * sizeof(u32);
	bytecode->Instructions.Data = (u8*)B3DAllocate(bytecode->Instructions.Size);

	memcpy(bytecode->Instructions.Data, spirv.data(), bytecode->Instructions.Size);

	return bytecode;
}
