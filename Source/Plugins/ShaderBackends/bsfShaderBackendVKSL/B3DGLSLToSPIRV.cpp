//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DGLSLToSPIRV.h"
#include "GpuBackend/B3DGpuParameterSet.h"
#include "GpuBackend/B3DGpuProgramParameterDescription.h"
#include "GpuBackend/B3DVertexDescription.h"
#include "GpuBackend/B3DGpuProgram.h"
#include "Math/B3DMath.h"
#include "Debug/B3DLog.h"

#include "glslang/Public/ShaderLang.h"
#include "glslang/Public/ResourceLimits.h"
#include "glslang/SPIRV/GlslangToSpv.h"
#include "spirv_cross/spirv_cross.hpp"

using namespace b3d;
using namespace b3d::render;

namespace
{
	/** Converts a SPIRV-Cross vertex stage-input type into a B3D vertex element type. */
	VertexElementType MapSPIRVCrossToVertexElementType(const spirv_cross::SPIRType& type)
	{
		const u32 vectorSize = type.vecsize;

		switch (type.basetype)
		{
		case spirv_cross::SPIRType::Float:
			switch (vectorSize)
			{
			case 1: return VET_FLOAT1;
			case 2: return VET_FLOAT2;
			case 3: return VET_FLOAT3;
			case 4: return VET_FLOAT4;
			default: return VET_UNKNOWN;
			}
		case spirv_cross::SPIRType::Half:
			switch (vectorSize)
			{
			case 1: return VET_HALF1;
			case 2: return VET_HALF2;
			case 3: return VET_HALF3;
			case 4: return VET_HALF4;
			default: return VET_UNKNOWN;
			}
		case spirv_cross::SPIRType::Int:
			switch (vectorSize)
			{
			case 1: return VET_INT1;
			case 2: return VET_INT2;
			case 3: return VET_INT3;
			case 4: return VET_INT4;
			default: return VET_UNKNOWN;
			}
		case spirv_cross::SPIRType::UInt:
			switch (vectorSize)
			{
			case 1: return VET_UINT1;
			case 2: return VET_UINT2;
			case 3: return VET_UINT3;
			case 4: return VET_UINT4;
			default: return VET_UNKNOWN;
			}
		default:
			return VET_UNKNOWN;
		}
	}

	/** Converts a SPIRVCross type into a B3D GPU data parameter type. */
	GpuDataParameterType SPIRVCrossTypeToGpuDataParameterType(const spirv_cross::SPIRType& type)
	{
		if (type.basetype == spirv_cross::SPIRType::Struct)
			return GPDT_STRUCT;

		if (type.basetype == spirv_cross::SPIRType::Float)
		{
			if (type.columns == 1)
			{
				switch (type.vecsize)
				{
				case 1: return GPDT_FLOAT1;
				case 2: return GPDT_FLOAT2;
				case 3: return GPDT_FLOAT3;
				case 4: return GPDT_FLOAT4;
				}
			}
			else if (type.columns == 2)
			{
				switch (type.vecsize)
				{
				case 2: return GPDT_MATRIX_2X2;
				case 3: return GPDT_MATRIX_3X2;
				case 4: return GPDT_MATRIX_4X2;
				}
			}
			else if (type.columns == 3)
			{
				switch (type.vecsize)
				{
				case 2: return GPDT_MATRIX_2X3;
				case 3: return GPDT_MATRIX_3X3;
				case 4: return GPDT_MATRIX_4X3;
				}
			}
			else if (type.columns == 4)
			{
				switch (type.vecsize)
				{
				case 2: return GPDT_MATRIX_2X4;
				case 3: return GPDT_MATRIX_3X4;
				case 4: return GPDT_MATRIX_4X4;
				}
			}
		}
		else if (type.basetype == spirv_cross::SPIRType::Double)
		{
			if (type.columns == 1)
			{
				switch (type.vecsize)
				{
				case 1: return GPDT_DOUBLE1;
				case 2: return GPDT_DOUBLE2;
				case 3: return GPDT_DOUBLE3;
				case 4: return GPDT_DOUBLE4;
				}
			}
			else if (type.columns == 2)
			{
				switch (type.vecsize)
				{
				case 2: return GPDT_DOUBLE_MATRIX_2X2;
				case 3: return GPDT_DOUBLE_MATRIX_3X2;
				case 4: return GPDT_DOUBLE_MATRIX_4X2;
				}
			}
			else if (type.columns == 3)
			{
				switch (type.vecsize)
				{
				case 2: return GPDT_DOUBLE_MATRIX_2X3;
				case 3: return GPDT_DOUBLE_MATRIX_3X3;
				case 4: return GPDT_DOUBLE_MATRIX_4X3;
				}
			}
			else if (type.columns == 4)
			{
				switch (type.vecsize)
				{
				case 2: return GPDT_DOUBLE_MATRIX_2X4;
				case 3: return GPDT_DOUBLE_MATRIX_3X4;
				case 4: return GPDT_DOUBLE_MATRIX_4X4;
				}
			}
		}
		else if (type.basetype == spirv_cross::SPIRType::Half)
		{
			if (type.columns == 1)
			{
				switch (type.vecsize)
				{
				case 1: return GPDT_HALF1;
				case 2: return GPDT_HALF2;
				case 3: return GPDT_HALF3;
				case 4: return GPDT_HALF4;
				}
			}
			else if (type.columns == 2)
			{
				switch (type.vecsize)
				{
				case 2: return GPDT_HALF_MATRIX_2X2;
				case 3: return GPDT_HALF_MATRIX_3X2;
				case 4: return GPDT_HALF_MATRIX_4X2;
				}
			}
			else if (type.columns == 3)
			{
				switch (type.vecsize)
				{
				case 2: return GPDT_HALF_MATRIX_2X3;
				case 3: return GPDT_HALF_MATRIX_3X3;
				case 4: return GPDT_HALF_MATRIX_4X3;
				}
			}
			else if (type.columns == 4)
			{
				switch (type.vecsize)
				{
				case 2: return GPDT_HALF_MATRIX_2X4;
				case 3: return GPDT_HALF_MATRIX_3X4;
				case 4: return GPDT_HALF_MATRIX_4X4;
				}
			}
		}
		else if (type.basetype == spirv_cross::SPIRType::Int)
		{
			if (type.columns == 1)
			{
				switch (type.vecsize)
				{
				case 1: return GPDT_INT1;
				case 2: return GPDT_INT2;
				case 3: return GPDT_INT3;
				case 4: return GPDT_INT4;
				}
			}
		}
		else if (type.basetype == spirv_cross::SPIRType::UInt)
		{
			if (type.columns == 1)
			{
				switch (type.vecsize)
				{
				case 1: return GPDT_UINT1;
				case 2: return GPDT_UINT2;
				case 3: return GPDT_UINT3;
				case 4: return GPDT_UINT4;
				}
			}
		}
		else if (type.basetype == spirv_cross::SPIRType::Boolean)
		{
			if (type.columns == 1 && type.vecsize == 1)
				return GPDT_BOOL;
		}

		return GPDT_UNKNOWN;
	}

	/** Converts a SPIRVCross type into a B3D buffer format. */
	GpuBufferFormat SPIRVCrossTypeToBufferFormat(const spirv_cross::SPIRType& type)
	{
		switch (type.basetype)
		{
		case spirv_cross::SPIRType::Float:
			switch (type.vecsize)
			{
			case 1: return BF_32X1F;
			case 2: return BF_32X2F;
			case 3: return BF_32X3F;
			case 4: return BF_32X4F;
			default: return BF_UNKNOWN;
			}
		case spirv_cross::SPIRType::Double:
			switch (type.vecsize)
			{
			case 1: return BF_64X1F;
			case 2: return BF_64X2F;
			case 3: return BF_64X3F;
			case 4: return BF_64X4F;
			default: return BF_UNKNOWN;
			}
		case spirv_cross::SPIRType::Half:
			switch (type.vecsize)
			{
			case 1: return BF_16X1F;
			case 2: return BF_16X2F;
			case 4: return BF_16X4F;
			default: return BF_UNKNOWN;
			}
		case spirv_cross::SPIRType::UShort:
			switch (type.vecsize)
			{
			case 1: return BF_16X1U;
			case 2: return BF_16X2U;
			case 4: return BF_16X4U;
			default: return BF_UNKNOWN;
			}
		case spirv_cross::SPIRType::Short:
			switch (type.vecsize)
			{
			case 1: return BF_16X1S;
			case 2: return BF_16X2S;
			case 4: return BF_16X4S;
			default: return BF_UNKNOWN;
			}
		case spirv_cross::SPIRType::UByte:
			switch (type.vecsize)
			{
			case 1: return BF_8X1U;
			case 2: return BF_8X2U;
			case 4: return BF_8X4U;
			default: return BF_UNKNOWN;
			}
		case spirv_cross::SPIRType::SByte:
			switch (type.vecsize)
			{
			case 1: return BF_8X1S;
			case 2: return BF_8X2S;
			case 4: return BF_8X4S;
			default: return BF_UNKNOWN;
			}
		case spirv_cross::SPIRType::Int:
			switch (type.vecsize)
			{
			case 1: return BF_32X1S;
			case 2: return BF_32X2S;
			case 3: return BF_32X3S;
			case 4: return BF_32X4S;
			default: return BF_UNKNOWN;
			}
		case spirv_cross::SPIRType::UInt:
			switch (type.vecsize)
			{
			case 1: return BF_32X1U;
			case 2: return BF_32X2U;
			case 3: return BF_32X3U;
			case 4: return BF_32X4U;
			default: return BF_UNKNOWN;
			}
		case spirv_cross::SPIRType::Int64:
			switch (type.vecsize)
			{
			case 1: return BF_64X1S;
			case 2: return BF_64X2S;
			case 3: return BF_64X3S;
			case 4: return BF_64X4S;
			default: return BF_UNKNOWN;
			}
		case spirv_cross::SPIRType::UInt64:
			switch (type.vecsize)
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

	/** Parses a single struct member from a SPIRVCross type. Returns null if parsing failed and logs an error into the provided log stream, otherwise returns the parsed data. */
	TOptional<GpuUniformBufferMemberInformation> ParseSPIRVCrossStructMember(spirv_cross::Compiler& compiler, const spirv_cross::SPIRType& structType, u32 memberIndex, StringStream& outLog)
	{
		const spirv_cross::SPIRType& memberType = compiler.get_type(structType.member_types[memberIndex]);
		u32 memberSize = (u32)compiler.get_declared_struct_member_size(structType, memberIndex);
		u32 memberOffset = compiler.type_struct_member_offset(structType, memberIndex);

		if (!B3D_ENSURE(memberSize % 4 == 0))
			return {};

		if (!B3D_ENSURE(memberOffset % 4 == 0))
			return {};

		memberSize /= 4;
		memberOffset /= 4;

		u32 arrayStride = memberSize;
		if (!memberType.array.empty())
		{
			arrayStride = compiler.type_struct_member_array_stride(structType, memberIndex);

			if (!B3D_ENSURE(arrayStride % 4 == 0))
				return {};

			arrayStride /= 4;
		}

		const std::string& name = compiler.get_member_name(structType.self, memberIndex);

		GpuUniformBufferMemberInformation memberInformation;
		memberInformation.Name = name;
		memberInformation.Type = SPIRVCrossTypeToGpuDataParameterType(memberType);
		memberInformation.ParentUniformBufferSet = 0; // Must be assigned by the caller
		memberInformation.ParentUniformBufferSlot = 0; // Must be assigned by the caller
		memberInformation.ElementSize = memberSize;
		memberInformation.ArrayElementStride = arrayStride;
		memberInformation.ArraySize = memberType.array.empty() ? 1 : memberType.array[0]; // Not supporting array of arrays
		memberInformation.CpuOffset = memberOffset;
		memberInformation.GpuOffset = memberOffset;

		if (memberInformation.Type == GPDT_UNKNOWN)
		{
			outLog << StringUtility::Format("Warning: Failed parsing uniform. Cannot determine type for uniform: {0}.\n", memberInformation.Name);
			return {};
		}

		return memberInformation;
	}

	/** Parses a uniform buffer uniform from a SPIRVCross type. Returns null if parsing failed and logs an error into the provided log stream, otherwise returns the parsed data. */
	TOptional<GpuUniformBufferInformation> ParseSPIRVCrossUniformBuffer(spirv_cross::Compiler& compiler, const spirv_cross::Resource& resource, StringStream& outLog)
	{
		if (!compiler.get_decoration_bitset(resource.id).get(spv::DecorationBinding))
		{
			outLog << StringUtility::Format("Warning: Failed parsing uniform {0}. Uniform has no slot binding assigned.", resource.name.c_str());
			return {};
		}

		if (!compiler.get_decoration_bitset(resource.id).get(spv::DecorationDescriptorSet))
		{
			outLog << StringUtility::Format("Warning: Failed parsing uniform {0}. Uniform has no set binding assigned.", resource.name.c_str());
			return {};
		}

		const spirv_cross::SPIRType& type = compiler.get_type(resource.base_type_id);
		const u32 bufferSize = (u32)compiler.get_declared_struct_size(type);

		GpuUniformBufferInformation uniformBufferInformation;
		uniformBufferInformation.Name = resource.name;
		uniformBufferInformation.Size = Math::CeilToMultiple(bufferSize / 4u, 4u); // Round to multiple of 16 bytes
		uniformBufferInformation.IsShareable = true;
		uniformBufferInformation.Slot = compiler.get_decoration(resource.id, spv::DecorationBinding);
		uniformBufferInformation.Set = compiler.get_decoration(resource.id, spv::DecorationDescriptorSet);

		return uniformBufferInformation;
	}

	/**
	 * Parses common object information (name, binding, array size) from a SPIRVCross resource and its type. Helper to be used by other parsing methods.
	 * Returns null if parsing failed and logs an error into the provided log stream, otherwise returns the parsed data.
	 */
	TOptional<GpuObjectParameterInformation> ParseSPIRVCrossObjectCommon(spirv_cross::Compiler& compiler, const spirv_cross::Resource& resource, const spirv_cross::SPIRType& type, StringStream& outLog)
	{
		if (!compiler.get_decoration_bitset(resource.id).get(spv::DecorationBinding))
		{
			outLog << StringUtility::Format("Warning: Failed parsing uniform {0}. Uniform has no slot binding assigned.", resource.name.c_str());
			return {};
		}

		if (!compiler.get_decoration_bitset(resource.id).get(spv::DecorationDescriptorSet))
		{
			outLog << StringUtility::Format("Warning: Failed parsing uniform {0}. Uniform has no set binding assigned.", resource.name.c_str());
			return {};
		}

		if (type.array.size() > 1)
		{
			outLog << StringUtility::Format("Warning: Failed parsing uniform {0}. Multi-dimension arrays are not supported.", resource.name.c_str());
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

	/** Parses a sampled texture uniform from a SPIRVCross type. Returns null if parsing failed and logs an error into the provided log stream, otherwise returns the parsed data. */
	TOptional<GpuObjectParameterInformation> ParseSPIRVCrossSampledTexture(spirv_cross::Compiler& compiler, const spirv_cross::Resource& resource, StringStream& outLog)
	{
		const spirv_cross::SPIRType& type = compiler.get_type(resource.base_type_id);
		TOptional<GpuObjectParameterInformation> objectInformation = ParseSPIRVCrossObjectCommon(compiler, resource, type, outLog);
		if (!objectInformation)
			return {};

		const spirv_cross::SPIRType::ImageType imageTypeInformation = type.image;
		if (!B3D_ENSURE(imageTypeInformation.sampled == 1)) // 1 means sampled images, 2 means load/store, 0 means nothing
			return {};

		switch (type.image.dim)
		{
		case spv::Dim1D:
			objectInformation->Type = imageTypeInformation.arrayed ? GPOT_TEXTURE1DARRAY : GPOT_TEXTURE1D;
			break;
		case spv::Dim2D:
			if (imageTypeInformation.ms)
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

		if (objectInformation->Type == GPOT_UNKNOWN)
		{
			outLog << StringUtility::Format("Warning: Failed parsing uniform. Cannot determine type for uniform: {0}.\n", objectInformation->Type);
			return {};
		}

		if (type.image.type != 0)
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

	/** Parses a sampler uniform from a SPIRVCross type. Returns null if parsing failed and logs an error into the provided log stream, otherwise returns the parsed data. */
	TOptional<GpuObjectParameterInformation> ParseSPIRVCrossSampler(spirv_cross::Compiler& compiler, const spirv_cross::Resource& resource, StringStream& outLog)
	{
		const spirv_cross::SPIRType& type = compiler.get_type(resource.base_type_id);
		TOptional<GpuObjectParameterInformation> objectInformation = ParseSPIRVCrossObjectCommon(compiler, resource, type, outLog);
		if (!objectInformation)
			return {};

		objectInformation->Type = GPOT_SAMPLER2D;
		objectInformation->ElementType = BF_UNKNOWN;

		return objectInformation;
	}

	/** Parses a storage texture uniform from a SPIRVCross type. Returns null if parsing failed and logs an error into the provided log stream, otherwise returns the parsed data. */
	TOptional<GpuObjectParameterInformation> ParseSPIRVCrossStorageTexture(spirv_cross::Compiler& compiler, const spirv_cross::Resource& resource, StringStream& outLog)
	{
		const spirv_cross::SPIRType& type = compiler.get_type(resource.base_type_id);
		TOptional<GpuObjectParameterInformation> objectInformation = ParseSPIRVCrossObjectCommon(compiler, resource, type, outLog);
		if (!objectInformation)
			return {};

		const spirv_cross::SPIRType::ImageType imageTypeInformation = type.image;
		if (!B3D_ENSURE(imageTypeInformation.sampled == 2))
			return {};

		switch (type.image.dim)
		{
		case spv::Dim1D:
			objectInformation->Type = imageTypeInformation.arrayed ? GPOT_RWTEXTURE1DARRAY : GPOT_RWTEXTURE1D;
			break;
		case spv::Dim2D:
			if (imageTypeInformation.ms)
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

		if (objectInformation->Type == GPOT_UNKNOWN)
		{
			outLog << StringUtility::Format("Warning: Failed parsing uniform. Cannot determine type for uniform: {0}.\n", objectInformation->Type);
			return {};
		}

		if (type.image.type != 0)
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

	/** Parses a storage buffer uniform from a SPIRVCross type. Returns null if parsing failed and logs an error into the provided log stream, otherwise returns the parsed data. */
	TOptional<GpuObjectParameterInformation> ParseSPIRVCrossStorageBuffer(spirv_cross::Compiler& compiler, const spirv_cross::Resource& resource, StringStream& outLog)
	{
		const spirv_cross::SPIRType& type = compiler.get_type(resource.base_type_id);
		TOptional<GpuObjectParameterInformation> objectInformation = ParseSPIRVCrossObjectCommon(compiler, resource, type, outLog);
		if (!objectInformation)
			return {};

#if 0 // SPIRV-Cross reports read-only qualifier even when its not there
		objectInformation->Type = type.image.access == spv::AccessQualifierReadOnly ? GPOT_STRUCTURED_BUFFER : GPOT_RWSTRUCTURED_BUFFER;
#else
		objectInformation->Type = GPOT_RWSTRUCTURED_BUFFER;
#endif

		if (type.image.type != 0)
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

	/** Parses all uniforms as reflected by the provided SPIRVCross compiler. Appends the out data into @p outParameterDescription, and logs any warnings in @p outLog. */
	void ParseSPIRVCrossUniforms(spirv_cross::Compiler& compiler, GpuProgramParameterDescription& outParameterDescription, StringStream& outLog)
	{
		const spirv_cross::ShaderResources& sprivResources = compiler.get_shader_resources();
		for (const auto& resource : sprivResources.uniform_buffers)
		{
			TOptional<GpuUniformBufferInformation> uniformBufferInformation = ParseSPIRVCrossUniformBuffer(compiler, resource, outLog);
			if (!uniformBufferInformation)
				continue;

			const spirv_cross::SPIRType& type = compiler.get_type(resource.base_type_id);
			const u32 memberCount = (u32)type.member_types.size();
			for (u32 memberIndex = 0; memberIndex < memberCount; memberIndex++)
			{
				TOptional<GpuUniformBufferMemberInformation> memberInformation = ParseSPIRVCrossStructMember(compiler, type, memberIndex, outLog);
				if (!memberInformation)
					continue;

				memberInformation->ParentUniformBufferSet = uniformBufferInformation->Set;
				memberInformation->ParentUniformBufferSlot = uniformBufferInformation->Slot;

				// Note: Name must be captured before moving to avoid use-after-move (evaluation order is unsequenced)
				String memberName = memberInformation->Name;
				outParameterDescription.UniformBufferMembers[std::move(memberName)] = std::move(memberInformation.value());
			}

			String bufferName = uniformBufferInformation->Name;
			outParameterDescription.UniformBuffers[std::move(bufferName)] = std::move(uniformBufferInformation.value());
		}

		// Combined texture/sampler (e.g. sampler2D).
		for (const auto& resource : sprivResources.sampled_images)
		{
			TOptional<GpuObjectParameterInformation> sampledImageInformation = ParseSPIRVCrossSampledTexture(compiler, resource, outLog);
			if (sampledImageInformation)
			{
				String name = sampledImageInformation->Name;
				if (sampledImageInformation->Type == GPOT_BYTE_BUFFER)
					outParameterDescription.Buffers[std::move(name)] = std::move(sampledImageInformation.value());
				else
					outParameterDescription.SampledTextures[std::move(name)] = std::move(sampledImageInformation.value());
			}

			TOptional<GpuObjectParameterInformation> samplerInformation = ParseSPIRVCrossSampler(compiler, resource, outLog);
			if (samplerInformation)
			{
				String name = samplerInformation->Name;
				outParameterDescription.Samplers[std::move(name)] = std::move(samplerInformation.value());
			}
		}

		// Sampled texture or buffer (e.g. texture2D/Texture2D, samplerBuffer/Buffer).
		for (const auto& resource : sprivResources.separate_images)
		{
			TOptional<GpuObjectParameterInformation> sampledImageInformation = ParseSPIRVCrossSampledTexture(compiler, resource, outLog);
			if (sampledImageInformation)
			{
				String name = sampledImageInformation->Name;
				if (sampledImageInformation->Type == GPOT_BYTE_BUFFER)
					outParameterDescription.Buffers[std::move(name)] = std::move(sampledImageInformation.value());
				else
					outParameterDescription.SampledTextures[std::move(name)] = std::move(sampledImageInformation.value());
			}
		}

		// Storage texture or buffer (e.g. image2D/RWTexture2D, imageBuffer/RWBuffer).
		for (const auto& resource : sprivResources.storage_images)
		{
			TOptional<GpuObjectParameterInformation> sampledImageInformation = ParseSPIRVCrossStorageTexture(compiler, resource, outLog);
			if (sampledImageInformation)
			{
				String name = sampledImageInformation->Name;
				if (sampledImageInformation->Type == GPOT_RWBYTE_BUFFER)
					outParameterDescription.Buffers[std::move(name)] = std::move(sampledImageInformation.value());
				else
					outParameterDescription.StorageTextures[std::move(name)] = std::move(sampledImageInformation.value());
			}
		}

		// Samplers (sampler/SamplerState).
		for (const auto& resource : sprivResources.separate_samplers)
		{
			TOptional<GpuObjectParameterInformation> samplerInformation = ParseSPIRVCrossSampler(compiler, resource, outLog);
			if (samplerInformation)
			{
				String name = samplerInformation->Name;
				outParameterDescription.Samplers[std::move(name)] = std::move(samplerInformation.value());
			}
		}

		// Structured buffers (e.g. buffer/StructuredBuffer/RWStructuredBuffer).
		for (const auto& resource : sprivResources.storage_buffers)
		{
			TOptional<GpuObjectParameterInformation> storageBufferInformation = ParseSPIRVCrossStorageBuffer(compiler, resource, outLog);
			if (storageBufferInformation)
			{
				String name = storageBufferInformation->Name;
				outParameterDescription.Buffers[std::move(name)] = std::move(storageBufferInformation.value());
			}
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
			if (!StringUtility::StartsWith(name, mName, false))
				return -1;

			u32 length = (u32)mName.size();
			return ParseI32(name.substr(length));
		}

		/**	Returns the semantic of this attribute. */
		VertexElementSemantic GetSemantic() const { return mSemantic; }

	private:
		String mName;
		VertexElementSemantic mSemantic;
	};

	bool AttributeNameToElementSemantic(const String& name, VertexElementSemantic& semantic, u16& index)
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

		static const u32 kAttributeCount = sizeof(attributes) / sizeof(attributes[0]);
		for (u32 i = 0; i < kAttributeCount; i++)
		{
			i32 attribIndex = attributes[i].MatchesName(name);
			if (attribIndex != -1)
			{
				index = attribIndex;
				semantic = attributes[i].GetSemantic();
				return true;
			}
		}

		return false;
	}

	bool ParseVertexAttributes(spirv_cross::Compiler& compiler, Vector<VertexElement>& elementList, String& log)
	{
		const spirv_cross::ShaderResources resources = compiler.get_shader_resources();
		for (const spirv_cross::Resource& input : resources.stage_inputs)
		{
			const std::string& attribName = input.name;

			// Built-in inputs (gl_VertexIndex etc.) carry no user semantic and are typically reflected
			// separately, but guard against them just in case.
			if (attribName.compare(0, 3, "gl_") == 0)
				continue;

			if (!compiler.has_decoration(input.id, spv::DecorationLocation))
			{
				log = "Vertex attribute parsing error: Found a vertex attribute without a location "
					  "qualifier. Each attribute must have an explicitly defined location number.";

				return false;
			}

			const u32 location = compiler.get_decoration(input.id, spv::DecorationLocation);

			VertexElementSemantic semantic = VES_POSITION;
			u16 index = 0;
			if (AttributeNameToElementSemantic(attribName.c_str(), semantic, index))
			{
				const spirv_cross::SPIRType& type = compiler.get_type(input.base_type_id);
				const VertexElementType elementType = MapSPIRVCrossToVertexElementType(type);
				if (elementType == VET_UNKNOWN)
					B3D_LOG(Error, LogRenderBackend, "Cannot determine vertex input attribute type for attribute: {0}", attribName.c_str());

				elementList.push_back(VertexElement(elementType, semantic, index, 0, 0, location));
			}
			else
			{
				B3D_LOG(Error, LogRenderBackend, "Cannot determine vertex input attribute semantic for attribute: {0}", attribName.c_str());
			}
		}

		return true;
	}
} // namespace

GLSLToSPIRV::GLSLToSPIRV(const char* compilerId, u32 compilerVersion)
	: mCompilerId(compilerId), mCompilerVersion(compilerVersion)
{
	glslang::InitializeProcess();
}

GLSLToSPIRV::~GLSLToSPIRV()
{
	glslang::FinalizeProcess();
}

bool GLSLToSPIRV::IsUpToDate(const GpuProgramBytecode& bytecode) const
{
	return bytecode.CompilerId == mCompilerId && bytecode.CompilerVersion == mCompilerVersion;
}

TShared<GpuProgramBytecode> GLSLToSPIRV::CompileBytecode(const GpuProgramCreateInformation& desc)
{
	const TBuiltInResource& resources = *GetDefaultResources();
	glslang::TProgram program;

	EShLanguage glslType = EShLangVertex;
	switch (desc.Type)
	{
	case GPT_FRAGMENT_PROGRAM: glslType = EShLangFragment; break;
	case GPT_HULL_PROGRAM:     glslType = EShLangTessControl; break;
	case GPT_DOMAIN_PROGRAM:   glslType = EShLangTessEvaluation; break;
	case GPT_GEOMETRY_PROGRAM: glslType = EShLangGeometry; break;
	case GPT_VERTEX_PROGRAM:   glslType = EShLangVertex; break;
	case GPT_COMPUTE_PROGRAM:  glslType = EShLangCompute; break;
	default: break;
	}

	std::vector<u32> spirv;
	spv::SpvBuildLogger logger;

	const String& source = desc.Source;
	const char* sourceBytes = source.c_str();

	glslang::TShader shader(glslType);
	shader.setEnvInput(glslang::EShSourceGlsl, glslType, glslang::EShClientVulkan, 100);
	shader.setEnvClient(glslang::EShClientVulkan, glslang::EShTargetVulkan_1_1);
	shader.setEnvTarget(glslang::EShTargetSpv, glslang::EShTargetSpv_1_3);
	shader.setStrings(&sourceBytes, 1);
	shader.setEntryPoint("main");

	TShared<GpuProgramBytecode> bytecode = B3DMakeShared<GpuProgramBytecode>();
	bytecode->CompilerId = mCompilerId;
	bytecode->CompilerVersion = mCompilerVersion;

	EShMessages messages = (EShMessages)((int)EShMsgSpvRules | (int)EShMsgVulkanRules);
	if (!shader.parse(&resources, 450, false, messages))
	{
		bytecode->Messages = "Compile error: " + String(shader.getInfoLog());
		return bytecode;
	}

	program.addShader(&shader);

	if (!program.link(messages))
	{
		bytecode->Messages = "Link error: " + String(program.getInfoLog());
		return bytecode;
	}

	program.mapIO();

	glslang::SpvOptions spvOptions;
	spvOptions.disableOptimizer = false;
	spvOptions.optimizeSize = false;

	GlslangToSpv(*program.getIntermediate(glslType), spirv, &logger, &spvOptions);

	spirv_cross::Compiler spirvCompiler(spirv);

	// Parse uniforms via SPIRV-Cross reflection.
	bytecode->ParameterDescription = B3DMakeShared<GpuProgramParameterDescription>();

	StringStream messageLog;
	ParseSPIRVCrossUniforms(spirvCompiler, *bytecode->ParameterDescription, messageLog);

	const String& messageLogString = messageLog.str();
	if (!messageLogString.empty())
		bytecode->Messages += messageLogString;

	// Populate vertex input attributes from the glslang reflection.
	if (desc.Type == GPT_VERTEX_PROGRAM)
	{
		if (!ParseVertexAttributes(spirvCompiler, bytecode->VertexInput, bytecode->Messages))
			return bytecode;
	}

	bytecode->Instructions.Size = (u32)spirv.size() * sizeof(u32);
	bytecode->Instructions.Data = (u8*)B3DAllocate(bytecode->Instructions.Size);

	memcpy(bytecode->Instructions.Data, spirv.data(), bytecode->Instructions.Size);

	return bytecode;
}
