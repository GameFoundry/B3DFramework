//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DHLSLToSPIRV.h"
#include "GpuBackend/B3DGpuParameterSet.h"
#include "GpuBackend/B3DGpuPushConstants.h"
#include "GpuBackend/B3DGpuBindlessHeap.h"
#include "GpuBackend/B3DGpuProgramParameterDescription.h"
#include "GpuBackend/B3DVertexDescription.h"
#include "GpuBackend/B3DGpuProgram.h"
#include "Material/B3DShaderParameterDescription.h"
#include "Math/B3DMath.h"
#include "Debug/B3DLog.h"
#include "String/B3DUnicode.h"

#include "spirv_cross/spirv_cross.hpp"
#include "spirv_cross/spirv_parser.hpp"

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

	/** Converts a SPIRVCross type into a B3D GPU data parameter type. HLSL matrix rows are SPIR-V matrix columns. */
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
				case 3: return GPDT_MATRIX_2X3;
				case 4: return GPDT_MATRIX_2X4;
				}
			}
			else if (type.columns == 3)
			{
				switch (type.vecsize)
				{
				case 2: return GPDT_MATRIX_3X2;
				case 3: return GPDT_MATRIX_3X3;
				case 4: return GPDT_MATRIX_3X4;
				}
			}
			else if (type.columns == 4)
			{
				switch (type.vecsize)
				{
				case 2: return GPDT_MATRIX_4X2;
				case 3: return GPDT_MATRIX_4X3;
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
				case 3: return GPDT_DOUBLE_MATRIX_2X3;
				case 4: return GPDT_DOUBLE_MATRIX_2X4;
				}
			}
			else if (type.columns == 3)
			{
				switch (type.vecsize)
				{
				case 2: return GPDT_DOUBLE_MATRIX_3X2;
				case 3: return GPDT_DOUBLE_MATRIX_3X3;
				case 4: return GPDT_DOUBLE_MATRIX_3X4;
				}
			}
			else if (type.columns == 4)
			{
				switch (type.vecsize)
				{
				case 2: return GPDT_DOUBLE_MATRIX_4X2;
				case 3: return GPDT_DOUBLE_MATRIX_4X3;
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
				case 3: return GPDT_HALF_MATRIX_2X3;
				case 4: return GPDT_HALF_MATRIX_2X4;
				}
			}
			else if (type.columns == 3)
			{
				switch (type.vecsize)
				{
				case 2: return GPDT_HALF_MATRIX_3X2;
				case 3: return GPDT_HALF_MATRIX_3X3;
				case 4: return GPDT_HALF_MATRIX_3X4;
				}
			}
			else if (type.columns == 4)
			{
				switch (type.vecsize)
				{
				case 2: return GPDT_HALF_MATRIX_4X2;
				case 3: return GPDT_HALF_MATRIX_4X3;
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
		uniformBufferInformation.Name = compiler.get_name(resource.id);
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
		objectInformation.Name = compiler.get_name(resource.id);
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
			objectInformation->Type = GPOT_TYPED_BUFFER;
			break;
		default:
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
			objectInformation->Type = GPOT_RWTYPED_BUFFER;
			break;
		default:
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

		// Read-only buffer qualifiers can decorate the block variable or its members.
		const bool isReadOnly = compiler.get_buffer_block_flags(resource.id).get(spv::DecorationNonWritable);
		objectInformation->Type = isReadOnly ? GPOT_STRUCTURED_BUFFER : GPOT_RWSTRUCTURED_BUFFER;

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
	bool ParseSPIRVCrossUniforms(spirv_cross::Compiler& compiler, GpuProgramParameterDescription& outParameterDescription, StringStream& outLog)
	{
		const spirv_cross::ShaderResources& sprivResources = compiler.get_shader_resources();

		// The descriptor heap arrays at the bindless set are bound by the device, not through parameters
		const auto fnIsBindlessHeap = [&compiler](const spirv_cross::Resource& resource)
		{
			return compiler.get_decoration(resource.id, spv::DecorationDescriptorSet) == kGpuBindlessParameterSet;
		};

		if (sprivResources.push_constant_buffers.size() > 1)
		{
			outLog << "SPIR-V declares more than one push-constant buffer.";
			return false;
		}

		if (!sprivResources.push_constant_buffers.empty())
		{
			const spirv_cross::SPIRType& pushConstantType = compiler.get_type(sprivResources.push_constant_buffers.front().base_type_id);
			const size_t pushConstantSize = compiler.get_declared_struct_size(pushConstantType);
			if (pushConstantSize == 0 || (pushConstantSize & 3u) != 0 || pushConstantSize > kMaxPushConstantSizeInBytes)
			{
				outLog << "SPIR-V push-constant buffer has an invalid reflected size of " << pushConstantSize
					<< " bytes; expected a non-zero, four-byte-aligned size no greater than " << kMaxPushConstantSizeInBytes << " bytes.";
				return false;
			}

			outParameterDescription.PushConstantBufferSize = (u32)pushConstantSize;
		}

		for (const auto& resource : sprivResources.uniform_buffers)
		{
			if (fnIsBindlessHeap(resource))
				continue;

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
			if (fnIsBindlessHeap(resource))
				continue;

			TOptional<GpuObjectParameterInformation> sampledImageInformation = ParseSPIRVCrossSampledTexture(compiler, resource, outLog);
			if (sampledImageInformation)
			{
				String name = sampledImageInformation->Name;
				if (sampledImageInformation->Type == GPOT_TYPED_BUFFER)
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
			if (fnIsBindlessHeap(resource))
				continue;

			TOptional<GpuObjectParameterInformation> sampledImageInformation = ParseSPIRVCrossSampledTexture(compiler, resource, outLog);
			if (sampledImageInformation)
			{
				String name = sampledImageInformation->Name;
				if (sampledImageInformation->Type == GPOT_TYPED_BUFFER)
					outParameterDescription.Buffers[std::move(name)] = std::move(sampledImageInformation.value());
				else
					outParameterDescription.SampledTextures[std::move(name)] = std::move(sampledImageInformation.value());
			}
		}

		// Storage texture or buffer (e.g. image2D/RWTexture2D, imageBuffer/RWBuffer).
		for (const auto& resource : sprivResources.storage_images)
		{
			if (fnIsBindlessHeap(resource))
				continue;

			TOptional<GpuObjectParameterInformation> sampledImageInformation = ParseSPIRVCrossStorageTexture(compiler, resource, outLog);
			if (sampledImageInformation)
			{
				String name = sampledImageInformation->Name;
				if (sampledImageInformation->Type == GPOT_RWTYPED_BUFFER)
					outParameterDescription.Buffers[std::move(name)] = std::move(sampledImageInformation.value());
				else
					outParameterDescription.StorageTextures[std::move(name)] = std::move(sampledImageInformation.value());
			}
		}

		// Samplers (sampler/SamplerState).
		for (const auto& resource : sprivResources.separate_samplers)
		{
			if (fnIsBindlessHeap(resource))
				continue;

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
			if (fnIsBindlessHeap(resource))
				continue;

			TOptional<GpuObjectParameterInformation> storageBufferInformation = ParseSPIRVCrossStorageBuffer(compiler, resource, outLog);
			if (storageBufferInformation)
			{
				String name = storageBufferInformation->Name;
				outParameterDescription.Buffers[std::move(name)] = std::move(storageBufferInformation.value());
			}
		}

		// Acceleration structures (RaytracingAccelerationStructure).
		for (const auto& resource : sprivResources.acceleration_structures)
		{
			if (fnIsBindlessHeap(resource))
				continue;

			const spirv_cross::SPIRType& type = compiler.get_type(resource.base_type_id);
			TOptional<GpuObjectParameterInformation> accelerationStructureInformation = ParseSPIRVCrossObjectCommon(compiler, resource, type, outLog);
			if (accelerationStructureInformation)
			{
				accelerationStructureInformation->Type = GPOT_ACCELERATION_STRUCTURE;
				accelerationStructureInformation->ElementType = BF_UNKNOWN;

				String name = accelerationStructureInformation->Name;
				outParameterDescription.AccelerationStructures[std::move(name)] = std::move(accelerationStructureInformation.value());
			}
		}

		return true;
	}

	/**	Holds an HLSL vertex program input semantic. */
	struct VertexAttributeSemantic
	{
		/** Constructs a new attribute from a name and a semantic that represents in which way is the attribute used. */
		VertexAttributeSemantic(const String& name, VertexElementSemantic semantic)
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
		static VertexAttributeSemantic attributes[] = {
			VertexAttributeSemantic("POSITION", VES_POSITION),
			VertexAttributeSemantic("NORMAL", VES_NORMAL),
			VertexAttributeSemantic("TANGENT", VES_TANGENT),
			VertexAttributeSemantic("BITANGENT", VES_BITANGENT),
			VertexAttributeSemantic("BINORMAL", VES_BITANGENT),
			VertexAttributeSemantic("TEXCOORD", VES_TEXCOORD),
			VertexAttributeSemantic("COLOR", VES_COLOR),
			VertexAttributeSemantic("BLENDWEIGHT", VES_BLEND_WEIGHTS),
			VertexAttributeSemantic("BLENDINDICES", VES_BLEND_INDICES)
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
			// DXC names the stage variable of a vertex input after its semantic, with a fixed prefix
			static const std::string kStageVariablePrefix = "in.var.";

			std::string attribName = compiler.get_name(input.id);
			if (attribName.compare(0, kStageVariablePrefix.size(), kStageVariablePrefix) == 0)
				attribName.erase(0, kStageVariablePrefix.size());

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

HLSLToSPIRV::HLSLToSPIRV(const char* compilerId, u32 compilerVersion)
	: DxcBytecodeCompiler(compilerId, compilerVersion)
{ }

spirv_cross::ParsedIR HLSLToSPIRV::ParseModule(const u32* words, size_t wordCount)
{
	spirv_cross::Parser parser(words, wordCount);
	parser.parse();

	spirv_cross::ParsedIR& module = parser.get_parsed_ir();
	module.for_each_typed_id<spirv_cross::SPIRVariable>([&module](u32 id, const spirv_cross::SPIRVariable& variable)
	{
		if(variable.storage != spv::StorageClassUniform && variable.storage != spv::StorageClassUniformConstant && variable.storage != spv::StorageClassStorageBuffer)
			return;

		for(auto& [entryPointId, entryPoint] : module.entry_points)
		{
			const auto found = std::find(entryPoint.interface_variables.begin(), entryPoint.interface_variables.end(), spirv_cross::VariableID(id));
			if(found == entryPoint.interface_variables.end())
				entryPoint.interface_variables.push_back(id);
		}
	});

	return std::move(module);
}

void HLSLToSPIRV::AddCompileArguments(Vector<WString>& outArguments) const
{
	const WString bindlessSet = UTF8::ToWide(ToString(kGpuBindlessParameterSet));

	// Register numbers are unique within a set across all register classes, so they're used as binding numbers directly
	outArguments.push_back(L"-spirv");
	outArguments.push_back(L"-fspv-target-env=vulkan1.3");
	outArguments.push_back(L"-fspv-preserve-bindings"); // Parameter sets must be identical regardless of which resources a stage reads.
	outArguments.push_back(L"-fvk-bind-sampler-heap");
	outArguments.push_back(L"0");
	outArguments.push_back(bindlessSet);
	outArguments.push_back(L"-fvk-bind-resource-heap");
	outArguments.push_back(L"1");
	outArguments.push_back(bindlessSet);
}

bool HLSLToSPIRV::ReflectBytecode(const GpuProgramCreateInformation& createInformation, IDxcResult& result, IDxcBlob& program, GpuProgramBytecode& outBytecode)
{
	spirv_cross::Compiler spirvCompiler(ParseModule((const u32*)program.GetBufferPointer(), program.GetBufferSize() / sizeof(u32)));

	// Parse uniforms via SPIRV-Cross reflection.
	outBytecode.ParameterDescription = B3DMakeShared<GpuProgramParameterDescription>();

	StringStream messageLog;
	if (!ParseSPIRVCrossUniforms(spirvCompiler, *outBytecode.ParameterDescription, messageLog))
	{
		outBytecode.Messages = messageLog.str();
		return false;
	}

	if(createInformation.ShaderReflection != nullptr && createInformation.ShaderReflection->Parameters != nullptr)
	{
		ShaderParameterDescription::IterateMatching(createInformation.ShaderReflection->Parameters->GetUniformBuffers(), outBytecode.ParameterDescription->UniformBuffers, [](const auto& source, auto& outTarget) { outTarget.UsesDynamicOffset = source.UsesDynamicOffset; });

		// SPIR-V declares byte address buffers as storage blocks, like structured buffers, so only the shader reflection tells them apart
		ShaderParameterDescription::IterateMatching(createInformation.ShaderReflection->Parameters->GetBufferParameters(), outBytecode.ParameterDescription->Buffers, [](const auto& source, auto& outTarget)
		{
			if(GpuObjectParameterTypeInformation::IsByteAddressBuffer(source.Type))
				outTarget.Type = source.Type;
		});
	}

	const ShaderEntryPointReflection& entryPointReflection = createInformation.GetEntryPointReflection();
	outBytecode.ParameterDescription->Features = entryPointReflection.Features;

	const String& messageLogString = messageLog.str();
	if (!messageLogString.empty())
		outBytecode.Messages += messageLogString;

	// Populate vertex input attributes from the SPIR-V stage variables.
	if (createInformation.Type == GPT_VERTEX_PROGRAM)
		return ParseVertexAttributes(spirvCompiler, outBytecode.VertexInput, outBytecode.Messages);

	return true;
}
