//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DHLSLToDXIL.h"
#include "B3DHLSLShaderABI.h"
#include "GpuBackend/B3DGpuPushConstants.h"
#include "GpuBackend/B3DGpuProgram.h"
#include "GpuBackend/B3DGpuProgramParameterDescription.h"
#include "GpuBackend/B3DVertexDescription.h"
#include "Material/B3DShaderParameterDescription.h"
#include "Material/B3DShaderReflection.h"
#include "Debug/B3DLog.h"
#include "Math/B3DMath.h"
#include "String/B3DStringFormat.h"
#include "String/B3DUnicode.h"

#include <d3d12shader.h>
#include <algorithm>
#include <cctype>

using namespace b3d;
using namespace b3d::render;

namespace
{
	using Microsoft::WRL::ComPtr;

	/** Returns true when a reflected binding is the reserved push-constant carrier. */
	bool IsPushConstantBufferBinding(const D3D12_SHADER_INPUT_BIND_DESC& bindingDescription)
	{
		return bindingDescription.Type == D3D_SIT_CBUFFER && bindingDescription.BindPoint == kPushConstantHlslRegister && bindingDescription.Space == kPushConstantHlslRegisterSpace;
	}

	/** Returns the occupied byte range of a constant buffer without including the carrier's trailing register padding. */
	u32 GetConstantBufferSize(ID3D12ShaderReflectionConstantBuffer* reflection, const D3D12_SHADER_BUFFER_DESC& description)
	{
		u32 size = 0;
		for (u32 variableIndex = 0; variableIndex < description.Variables; variableIndex++)
		{
			D3D12_SHADER_VARIABLE_DESC variableDescription;
			reflection->GetVariableByIndex(variableIndex)->GetDesc(&variableDescription);
			size = std::max(size, variableDescription.StartOffset + variableDescription.Size);
		}

		return size;
	}

	/** Converts a D3D shader variable type to the matching engine data parameter type. */
	GpuDataParameterType ConvertD3DTypeToGpuDataParameterType(const D3D12_SHADER_TYPE_DESC& typeDesc)
	{
		switch (typeDesc.Type)
		{
		case D3D_SVT_FLOAT:
			if (typeDesc.Columns == 1 && typeDesc.Rows == 1) return GPDT_FLOAT1;
			if (typeDesc.Columns == 2 && typeDesc.Rows == 1) return GPDT_FLOAT2;
			if (typeDesc.Columns == 3 && typeDesc.Rows == 1) return GPDT_FLOAT3;
			if (typeDesc.Columns == 4 && typeDesc.Rows == 1) return GPDT_FLOAT4;
			if (typeDesc.Columns == 2 && typeDesc.Rows == 2) return GPDT_MATRIX_2X2;
			if (typeDesc.Columns == 3 && typeDesc.Rows == 3) return GPDT_MATRIX_3X3;
			if (typeDesc.Columns == 4 && typeDesc.Rows == 4) return GPDT_MATRIX_4X4;
			if (typeDesc.Columns == 4 && typeDesc.Rows == 3) return GPDT_MATRIX_4X3;
			if (typeDesc.Columns == 3 && typeDesc.Rows == 4) return GPDT_MATRIX_3X4;
			if (typeDesc.Columns == 2 && typeDesc.Rows == 3) return GPDT_MATRIX_2X3;
			if (typeDesc.Columns == 3 && typeDesc.Rows == 2) return GPDT_MATRIX_3X2;
			if (typeDesc.Columns == 2 && typeDesc.Rows == 4) return GPDT_MATRIX_2X4;
			if (typeDesc.Columns == 4 && typeDesc.Rows == 2) return GPDT_MATRIX_4X2;
			break;

		case D3D_SVT_INT:
			if (typeDesc.Columns == 1 && typeDesc.Rows == 1) return GPDT_INT1;
			if (typeDesc.Columns == 2 && typeDesc.Rows == 1) return GPDT_INT2;
			if (typeDesc.Columns == 3 && typeDesc.Rows == 1) return GPDT_INT3;
			if (typeDesc.Columns == 4 && typeDesc.Rows == 1) return GPDT_INT4;
			break;

		case D3D_SVT_UINT:
			if (typeDesc.Columns == 1 && typeDesc.Rows == 1) return GPDT_INT1; // No separate uint types
			if (typeDesc.Columns == 2 && typeDesc.Rows == 1) return GPDT_INT2;
			if (typeDesc.Columns == 3 && typeDesc.Rows == 1) return GPDT_INT3;
			if (typeDesc.Columns == 4 && typeDesc.Rows == 1) return GPDT_INT4;
			break;

		case D3D_SVT_BOOL:
			return GPDT_BOOL;

		case D3D_SVT_DOUBLE:
			// No double types in GpuDataParameterType, treat as float
			if (typeDesc.Columns == 1 && typeDesc.Rows == 1) return GPDT_FLOAT1;
			if (typeDesc.Columns == 2 && typeDesc.Rows == 1) return GPDT_FLOAT2;
			if (typeDesc.Columns == 3 && typeDesc.Rows == 1) return GPDT_FLOAT3;
			if (typeDesc.Columns == 4 && typeDesc.Rows == 1) return GPDT_FLOAT4;
			break;

		default:
			break;
		}

		return GPDT_UNKNOWN;
	}

	/**
	 * Parses an HLSL semantic name (e.g. "POSITION0") into the matching engine vertex element semantic, ignoring any
	 * trailing semantic index. Returns false for semantics the engine does not model.
	 */
	bool ParseSemanticName(const char* semanticName, VertexElementSemantic& outSemantic)
	{
		String name(semanticName);

		size_t baseNameLength = name.length();
		while (baseNameLength > 0 && isdigit(name[baseNameLength - 1]))
			baseNameLength--;

		name = name.substr(0, baseNameLength);

		for (char& character : name)
			character = (char)toupper(character);

		if (name == "POSITION") outSemantic = VES_POSITION;
		else if (name == "NORMAL") outSemantic = VES_NORMAL;
		else if (name == "TANGENT") outSemantic = VES_TANGENT;
		else if (name == "BITANGENT" || name == "BINORMAL") outSemantic = VES_BITANGENT;
		else if (name == "COLOR") outSemantic = VES_COLOR;
		else if (name == "TEXCOORD") outSemantic = VES_TEXCOORD;
		else if (name == "BLENDINDICES") outSemantic = VES_BLEND_INDICES;
		else if (name == "BLENDWEIGHT") outSemantic = VES_BLEND_WEIGHTS;
		else
			return false;

		return true;
	}

	/** Converts a D3D signature parameter to a vertex element type. */
	VertexElementType ConvertD3DSignatureToVertexType(const D3D12_SIGNATURE_PARAMETER_DESC& desc)
	{
		u32 componentCount = 0;
		if (desc.Mask & 0x1) componentCount++;
		if (desc.Mask & 0x2) componentCount++;
		if (desc.Mask & 0x4) componentCount++;
		if (desc.Mask & 0x8) componentCount++;

		switch (desc.ComponentType)
		{
		case D3D_REGISTER_COMPONENT_FLOAT32:
			switch (componentCount)
			{
			case 1: return VET_FLOAT1;
			case 2: return VET_FLOAT2;
			case 3: return VET_FLOAT3;
			case 4: return VET_FLOAT4;
			}
			break;

		case D3D_REGISTER_COMPONENT_SINT32:
			switch (componentCount)
			{
			case 1: return VET_INT1;
			case 2: return VET_INT2;
			case 3: return VET_INT3;
			case 4: return VET_INT4;
			}
			break;

		case D3D_REGISTER_COMPONENT_UINT32:
			switch (componentCount)
			{
			case 1: return VET_UINT1;
			case 2: return VET_UINT2;
			case 3: return VET_UINT3;
			case 4: return VET_UINT4;
			}
			break;

		default:
			break;
		}

		return VET_UNKNOWN;
	}

	/** Maps a read-only SRV dimension to the matching engine texture object type. */
	GpuParameterObjectType GetTextureObjectType(D3D_SRV_DIMENSION dimension)
	{
		switch (dimension)
		{
		case D3D_SRV_DIMENSION_TEXTURE1D:			return GPOT_TEXTURE1D;
		case D3D_SRV_DIMENSION_TEXTURE1DARRAY:		return GPOT_TEXTURE1DARRAY;
		case D3D_SRV_DIMENSION_TEXTURE2D:			return GPOT_TEXTURE2D;
		case D3D_SRV_DIMENSION_TEXTURE2DARRAY:		return GPOT_TEXTURE2DARRAY;
		case D3D_SRV_DIMENSION_TEXTURE2DMS:			return GPOT_TEXTURE2DMS;
		case D3D_SRV_DIMENSION_TEXTURE2DMSARRAY:	return GPOT_TEXTURE2DMSARRAY;
		case D3D_SRV_DIMENSION_TEXTURE3D:			return GPOT_TEXTURE3D;
		case D3D_SRV_DIMENSION_TEXTURECUBE:			return GPOT_TEXTURECUBE;
		case D3D_SRV_DIMENSION_TEXTURECUBEARRAY:	return GPOT_TEXTURECUBEARRAY;
		default:									return GPOT_TEXTURE2D;
		}
	}

	/** Maps a read-write UAV dimension to the matching engine texture object type. */
	GpuParameterObjectType GetRWTextureObjectType(D3D_SRV_DIMENSION dimension)
	{
		switch (dimension)
		{
		case D3D_SRV_DIMENSION_TEXTURE1D:			return GPOT_RWTEXTURE1D;
		case D3D_SRV_DIMENSION_TEXTURE1DARRAY:		return GPOT_RWTEXTURE1DARRAY;
		case D3D_SRV_DIMENSION_TEXTURE2D:			return GPOT_RWTEXTURE2D;
		case D3D_SRV_DIMENSION_TEXTURE2DARRAY:		return GPOT_RWTEXTURE2DARRAY;
		case D3D_SRV_DIMENSION_TEXTURE2DMS:			return GPOT_RWTEXTURE2DMS;
		case D3D_SRV_DIMENSION_TEXTURE2DMSARRAY:	return GPOT_RWTEXTURE2DMSARRAY;
		case D3D_SRV_DIMENSION_TEXTURE3D:			return GPOT_RWTEXTURE3D;
		default:									return GPOT_RWTEXTURE2D;
		}
	}

	/** Maps a structured/byte-address buffer binding type to the matching engine buffer object type. */
	GpuParameterObjectType GetBufferObjectType(D3D_SHADER_INPUT_TYPE type)
	{
		switch (type)
		{
		case D3D_SIT_UAV_RWSTRUCTURED:					return GPOT_RWSTRUCTURED_BUFFER;
		case D3D_SIT_UAV_RWBYTEADDRESS:					return GPOT_RWBYTE_ADDRESS_BUFFER;
		case D3D_SIT_UAV_APPEND_STRUCTURED:				return GPOT_RWAPPEND_BUFFER;
		case D3D_SIT_UAV_CONSUME_STRUCTURED:			return GPOT_RWCONSUME_BUFFER;
		case D3D_SIT_UAV_RWSTRUCTURED_WITH_COUNTER:		return GPOT_RWSTRUCTURED_BUFFER_WITH_COUNTER;
		case D3D_SIT_STRUCTURED:						return GPOT_STRUCTURED_BUFFER;
		case D3D_SIT_BYTEADDRESS:						return GPOT_BYTE_ADDRESS_BUFFER;
		default:										return GPOT_UNKNOWN;
		}
	}

	/**
	 * Returns the distance between two consecutive array elements of a uniform buffer member, in multiples of 4 bytes.
	 *
	 * HLSL reflection exposes no array stride, so it is derived from the member's total size. Constant buffer array
	 * elements always begin on a 16-byte boundary, and the reported size leaves out the trailing padding of the final
	 * element, so the per-element size is rounded up to a whole 16-byte row instead of being divided out exactly.
	 * Members that are not arrays report their own size as the stride, matching the other shader backends.
	 *
	 * @param	totalSizeBytes	Size of the whole member, every array element included, in bytes.
	 * @param	elementCount	Number of array elements, or 0 if the member is not an array.
	 */
	u32 GetArrayElementStride(u32 totalSizeBytes, u32 elementCount)
	{
		const u32 totalSize = totalSizeBytes / 4;
		if (elementCount <= 1)
			return totalSize;

		// Truncating the division is safe: the true stride is a whole number of 16-byte rows, and the quotient can
		// only fall short of it by the last element's missing padding, which is less than one row.
		return Math::CeilToMultiple(totalSize / elementCount, 4u);
	}

	/**
	 * Reflects constant buffers and their members.
	 *
	 * @tparam	TReflection		ID3D12ShaderReflection for a shader, or ID3D12FunctionReflection for a library function.
	 * @tparam	TDescription	Description of @p reflection: D3D12_SHADER_DESC or D3D12_FUNCTION_DESC.
	 */
	template<class TReflection, class TDescription>
	bool ReflectConstantBuffers(TReflection* reflection, const TDescription& shaderDesc, GpuProgramParameterDescription& parameterDescription, String& outMessages)
	{
		for (u32 bufferIndex = 0; bufferIndex < shaderDesc.ConstantBuffers; bufferIndex++)
		{
			ID3D12ShaderReflectionConstantBuffer* constantBufferReflection = reflection->GetConstantBufferByIndex(bufferIndex);
			D3D12_SHADER_BUFFER_DESC cbDesc;
			constantBufferReflection->GetDesc(&cbDesc);

			// Structured and byte-address buffers also appear in the constant buffer list (as resource bind
			// information blocks); they are reflected as bound resources instead
			if (cbDesc.Type != D3D_CT_CBUFFER)
				continue;

			// The register and space live on the resource binding of the same name, not on the buffer reflection
			D3D12_SHADER_INPUT_BIND_DESC bindDesc;
			for (u32 resourceIndex = 0; resourceIndex < shaderDesc.BoundResources; resourceIndex++)
			{
				reflection->GetResourceBindingDesc(resourceIndex, &bindDesc);
				if (strcmp(bindDesc.Name, cbDesc.Name) == 0)
					break;
			}

			if (IsPushConstantBufferBinding(bindDesc))
			{
				const u32 pushConstantBufferSize = GetConstantBufferSize(constantBufferReflection, cbDesc);
				if (pushConstantBufferSize == 0 || (pushConstantBufferSize & 3u) != 0 || pushConstantBufferSize > kMaxPushConstantSizeInBytes)
				{
					outMessages = StringUtility::Format("Push-constant buffer '{0}' has an invalid reflected size of {1} bytes; expected a non-zero, four-byte-aligned size no greater than {2} bytes.",
						cbDesc.Name, pushConstantBufferSize, kMaxPushConstantSizeInBytes);
					return false;
				}

				parameterDescription.PushConstantBufferSize = pushConstantBufferSize;
				continue;
			}

			GpuUniformBufferInformation bufferInformation;
			bufferInformation.Name = cbDesc.Name;
			bufferInformation.Slot = MapRegisterToSlot(bindDesc.BindPoint, HLSLRegisterClass::ConstantBuffer);
			bufferInformation.Set = bindDesc.Space; // Register space maps to descriptor set
			bufferInformation.Size = cbDesc.Size / 4; // Core API expects size in multiples of 4 bytes
			bufferInformation.IsShareable = true;
			// TODO(d3d12-port): Stage flags are not derivable from HLSL reflection alone; left at the default.

			for (u32 variableIndex = 0; variableIndex < cbDesc.Variables; variableIndex++)
			{
				ID3D12ShaderReflectionVariable* variableReflection = constantBufferReflection->GetVariableByIndex(variableIndex);
				D3D12_SHADER_VARIABLE_DESC varDesc;
				variableReflection->GetDesc(&varDesc);

				ID3D12ShaderReflectionType* typeReflection = variableReflection->GetType();
				D3D12_SHADER_TYPE_DESC typeDesc;
				typeReflection->GetDesc(&typeDesc);

				GpuUniformBufferMemberInformation memberInformation;
				memberInformation.Name = varDesc.Name;
				memberInformation.Type = ConvertD3DTypeToGpuDataParameterType(typeDesc);
				memberInformation.ElementSize = varDesc.Size / 4; // Core API expects size in multiples of 4 bytes
				memberInformation.ArraySize = typeDesc.Elements > 0 ? typeDesc.Elements : 1;
				memberInformation.ArrayElementStride = GetArrayElementStride(varDesc.Size, typeDesc.Elements);
				memberInformation.ParentUniformBufferSet = bufferInformation.Set;
				memberInformation.ParentUniformBufferSlot = bufferInformation.Slot;
				memberInformation.GpuOffset = varDesc.StartOffset / 4; // Core API expects offset in multiples of 4 bytes
				memberInformation.CpuOffset = varDesc.StartOffset / 4;

				parameterDescription.UniformBufferMembers[memberInformation.Name] = memberInformation;
			}

			parameterDescription.UniformBuffers[bufferInformation.Name] = bufferInformation;
		}

		return true;
	}

	/** Reflects bound resources (textures, samplers, UAVs, etc.). See ReflectConstantBuffers() for the template parameters. */
	template<class TReflection, class TDescription>
	void ReflectBoundResources(TReflection* reflection, const TDescription& shaderDesc, GpuProgramParameterDescription& paramDesc)
	{
		for (u32 resourceIndex = 0; resourceIndex < shaderDesc.BoundResources; resourceIndex++)
		{
			D3D12_SHADER_INPUT_BIND_DESC bindDesc;
			reflection->GetResourceBindingDesc(resourceIndex, &bindDesc);

			GpuObjectParameterInformation parameterInformation;
			parameterInformation.Name = bindDesc.Name;
			parameterInformation.Set = bindDesc.Space;
			parameterInformation.ArraySize = std::max(bindDesc.BindCount, 1u);

			switch (bindDesc.Type)
			{
			case D3D_SIT_TEXTURE:
				parameterInformation.Slot = MapRegisterToSlot(bindDesc.BindPoint, HLSLRegisterClass::ShaderResource);
				if (bindDesc.Dimension == D3D_SRV_DIMENSION_BUFFER)
				{
					parameterInformation.Type = GPOT_TYPED_BUFFER;
					paramDesc.Buffers[parameterInformation.Name] = parameterInformation;
				}
				else
				{
					parameterInformation.Type = GetTextureObjectType(bindDesc.Dimension);
					paramDesc.SampledTextures[parameterInformation.Name] = parameterInformation;
				}
				break;

			case D3D_SIT_SAMPLER:
				// TODO(d3d12-port): No dedicated sampler-state object type exists; use a representative sampler type.
				parameterInformation.Slot = MapRegisterToSlot(bindDesc.BindPoint, HLSLRegisterClass::Sampler);
				parameterInformation.Type = GPOT_SAMPLER2D;
				paramDesc.Samplers[parameterInformation.Name] = parameterInformation;
				break;

			case D3D_SIT_UAV_RWTYPED:
				parameterInformation.Slot = MapRegisterToSlot(bindDesc.BindPoint, HLSLRegisterClass::UnorderedAccess);
				if (bindDesc.Dimension == D3D_SRV_DIMENSION_BUFFER)
				{
					parameterInformation.Type = GPOT_RWTYPED_BUFFER;
					paramDesc.Buffers[parameterInformation.Name] = parameterInformation;
				}
				else
				{
					parameterInformation.Type = GetRWTextureObjectType(bindDesc.Dimension);
					paramDesc.StorageTextures[parameterInformation.Name] = parameterInformation;
				}
				break;

			case D3D_SIT_UAV_RWSTRUCTURED:
			case D3D_SIT_UAV_RWBYTEADDRESS:
			case D3D_SIT_UAV_APPEND_STRUCTURED:
			case D3D_SIT_UAV_CONSUME_STRUCTURED:
			case D3D_SIT_UAV_RWSTRUCTURED_WITH_COUNTER:
				parameterInformation.Slot = MapRegisterToSlot(bindDesc.BindPoint, HLSLRegisterClass::UnorderedAccess);
				parameterInformation.Type = GetBufferObjectType(bindDesc.Type);
				paramDesc.Buffers[parameterInformation.Name] = parameterInformation;
				break;

			case D3D_SIT_STRUCTURED:
			case D3D_SIT_BYTEADDRESS:
				parameterInformation.Slot = MapRegisterToSlot(bindDesc.BindPoint, HLSLRegisterClass::ShaderResource);
				parameterInformation.Type = GetBufferObjectType(bindDesc.Type);
				paramDesc.Buffers[parameterInformation.Name] = parameterInformation;
				break;

			case D3D_SIT_RTACCELERATIONSTRUCTURE:
				parameterInformation.Slot = MapRegisterToSlot(bindDesc.BindPoint, HLSLRegisterClass::ShaderResource);
				parameterInformation.Type = GPOT_ACCELERATION_STRUCTURE;
				paramDesc.AccelerationStructures[parameterInformation.Name] = parameterInformation;
				break;

			case D3D_SIT_CBUFFER:
				// Already handled in ReflectConstantBuffers
				break;

			default:
				B3D_LOG(Warning, LogRenderBackend, "Unknown resource type in shader reflection: {0}", (u32)bindDesc.Type);
				break;
			}
		}
	}

	/** Reflects vertex input attributes for vertex shaders. */
	void ReflectVertexInput(ID3D12ShaderReflection* reflection, const D3D12_SHADER_DESC& shaderDesc, Vector<VertexElement>& outVertexInput)
	{
		outVertexInput.clear();

		for (u32 parameterIndex = 0; parameterIndex < shaderDesc.InputParameters; parameterIndex++)
		{
			D3D12_SIGNATURE_PARAMETER_DESC paramDesc;
			reflection->GetInputParameterDesc(parameterIndex, &paramDesc);

			// Skip system values
			if (paramDesc.SystemValueType != D3D_NAME_UNDEFINED)
				continue;

			VertexElementSemantic semantic = VES_POSITION;
			if (!ParseSemanticName(paramDesc.SemanticName, semantic))
			{
				B3D_LOG(Warning, LogRenderBackend, "Unknown vertex semantic: {0}", paramDesc.SemanticName);
				continue;
			}

			VertexElementType elementType = ConvertD3DSignatureToVertexType(paramDesc);
			outVertexInput.push_back(VertexElement(elementType, semantic, (u16)paramDesc.SemanticIndex));
		}
	}

	/**
	 * Classifies a reflected resource binding into its engine parameter type and HLSL register class. Returns false
	 * for binding types the engine does not model (the caller skips those).
	 */
	bool ClassifyBoundResource(const D3D12_SHADER_INPUT_BIND_DESC& bindDesc, GpuParameterType& outType, HLSLRegisterClass& outRegisterClass)
	{
		switch (bindDesc.Type)
		{
		case D3D_SIT_CBUFFER:
			outType = GpuParameterType::UniformBuffer;
			outRegisterClass = HLSLRegisterClass::ConstantBuffer;
			return true;

		case D3D_SIT_TEXTURE:
			// Buffer<T> reflects as TEXTURE with a BUFFER dimension and is modeled as a (read-only) storage buffer
			outType = bindDesc.Dimension == D3D_SRV_DIMENSION_BUFFER ? GpuParameterType::StorageBuffer : GpuParameterType::SampledTexture;
			outRegisterClass = HLSLRegisterClass::ShaderResource;
			return true;

		case D3D_SIT_SAMPLER:
			outType = GpuParameterType::Sampler;
			outRegisterClass = HLSLRegisterClass::Sampler;
			return true;

		case D3D_SIT_UAV_RWTYPED:
			// RWBuffer<T> reflects as UAV_RWTYPED with a BUFFER dimension and is modeled as a storage buffer
			outType = bindDesc.Dimension == D3D_SRV_DIMENSION_BUFFER ? GpuParameterType::StorageBuffer : GpuParameterType::StorageTexture;
			outRegisterClass = HLSLRegisterClass::UnorderedAccess;
			return true;

		case D3D_SIT_UAV_RWSTRUCTURED:
		case D3D_SIT_UAV_RWBYTEADDRESS:
		case D3D_SIT_UAV_APPEND_STRUCTURED:
		case D3D_SIT_UAV_CONSUME_STRUCTURED:
		case D3D_SIT_UAV_RWSTRUCTURED_WITH_COUNTER:
			outType = GpuParameterType::StorageBuffer;
			outRegisterClass = HLSLRegisterClass::UnorderedAccess;
			return true;

		case D3D_SIT_STRUCTURED:
		case D3D_SIT_BYTEADDRESS:
			outType = GpuParameterType::StorageBuffer;
			outRegisterClass = HLSLRegisterClass::ShaderResource;
			return true;

		case D3D_SIT_RTACCELERATIONSTRUCTURE:
			outType = GpuParameterType::AccelerationStructure;
			outRegisterClass = HLSLRegisterClass::ShaderResource;
			return true;

		default:
			return false;
		}
	}

	/**
	 * Builds the program's resource-table layout from the reflected resource bindings: a root table holding one
	 * sub-table per referenced register space (engine set), each listing the space's bindings sorted by slot.
	 *
	 * Unlike the Metal argument-buffer layouts, D3D12 has no compiler-chosen descriptor packing - registers
	 * are the shader-side contract and the backend decides the root-signature packing when the pipeline layout is
	 * created. All offsets and sizes are therefore left at 0 (driver-managed); the table records which resources the
	 * stage binds per set (type, slot, descriptor count). Note the bindings are those the stage actually USES - the
	 * D3D shader compiler strips unused declarations from reflection - so tables of different stages sharing a set
	 * are merged by slot when the pipeline parameter layout is created.
	 *
	 * See ReflectConstantBuffers() for the template parameters.
	 */
	template<class TReflection, class TDescription>
	void BuildResourceTableLayout(TReflection* reflection, const TDescription& shaderDesc, GpuProgramBytecode& bytecode)
	{
		// Group the reflected bindings by register space (engine set). Map keeps the sets ordered; the entries within
		// each set are sorted by slot further below.
		Map<u32, Vector<GpuDescriptorTableEntry>> setEntries;
		for (u32 resourceIndex = 0; resourceIndex < shaderDesc.BoundResources; resourceIndex++)
		{
			D3D12_SHADER_INPUT_BIND_DESC bindDesc;
			reflection->GetResourceBindingDesc(resourceIndex, &bindDesc);
			if (IsPushConstantBufferBinding(bindDesc))
				continue;

			GpuParameterType type;
			HLSLRegisterClass registerClass;
			if (!ClassifyBoundResource(bindDesc, type, registerClass))
				continue;

			GpuDescriptorTableEntry entry;
			entry.Kind = GpuDescriptorEntryKind::Resource;
			entry.Type = type;
			entry.Slot = MapRegisterToSlot(bindDesc.BindPoint, registerClass);
			entry.DescriptorCount = std::max(bindDesc.BindCount, 1u);
			setEntries[bindDesc.Space].push_back(entry);
		}

		bytecode.ResourceTableLayout = B3DMakeShared<GpuResourceTableLayout>();
		if (setEntries.empty())
			return;

		GpuResourceTableLayout& layout = *bytecode.ResourceTableLayout;

		GpuDescriptorTable root;
		root.FirstEntry = 0;
		root.EntryCount = (u32)setEntries.size();
		layout.Tables.push_back(root);

		for (u32 tableIndex = 0; tableIndex < (u32)setEntries.size(); tableIndex++)
		{
			GpuDescriptorTableEntry entry;
			entry.Kind = GpuDescriptorEntryKind::SubTable;
			entry.TableIndex = tableIndex + 1;
			layout.Entries.push_back(entry);
		}

		auto fnCompareBySlot = [](const GpuDescriptorTableEntry& lhs, const GpuDescriptorTableEntry& rhs) { return lhs.Slot < rhs.Slot; };

		for (auto& [set, entries] : setEntries)
		{
			std::sort(entries.begin(), entries.end(), fnCompareBySlot);

			GpuDescriptorTable table;
			table.Set = set;
			table.FirstEntry = (u32)layout.Entries.size();
			table.EntryCount = (u32)entries.size();
			layout.Tables.push_back(table);
			layout.Entries.insert(layout.Entries.end(), entries.begin(), entries.end());
		}
	}

	/**
	 * Adds resources that were stripped out from @p parameterDescription by the D3D shader compiler, due to them being unused.
	 * We use the output from the high-level shader compiler to detect this case and re-add the missing entries, so we can have
	 * a stable set of bindings for all backends.
	 */
	void AddUnusedDeclarations(const ShaderParameterDescription& declarations, GpuProgramParameterDescription& parameterDescription)
	{
		UnorderedSet<u64> reflectedBindings;
		auto fnBindingKey = [](u32 set, u32 slot) { return ((u64)set << 32) | slot; };

		for (const auto& entry : parameterDescription.UniformBuffers)
			reflectedBindings.insert(fnBindingKey(entry.second.Set, entry.second.Slot));

		for (const Map<String, GpuObjectParameterInformation>* objects : { &parameterDescription.Samplers, &parameterDescription.SampledTextures, &parameterDescription.StorageTextures, &parameterDescription.Buffers, &parameterDescription.AccelerationStructures })
		{
			for (const auto& entry : *objects)
				reflectedBindings.insert(fnBindingKey(entry.second.Set, entry.second.Slot));
		}

		for (const auto& entry : declarations.GetUniformBuffers())
		{
			const ShaderUniformBufferInformation& declaration = entry.second;
			if (declaration.Set == ~0u || declaration.Slot == ~0u)
				continue;

			const u32 slot = MapRegisterToSlot(declaration.Slot, HLSLRegisterClass::ConstantBuffer);
			if (!reflectedBindings.insert(fnBindingKey(declaration.Set, slot)).second)
				continue;

			GpuUniformBufferInformation bufferInformation;
			bufferInformation.Name = declaration.Name;
			bufferInformation.Set = declaration.Set;
			bufferInformation.Slot = slot;
			bufferInformation.Size = 0;
			bufferInformation.IsShareable = true;
			bufferInformation.UsesDynamicOffset = declaration.UsesDynamicOffset;
			parameterDescription.UniformBuffers[bufferInformation.Name] = bufferInformation;
		}

		auto fnAddObjects = [&](const Map<String, ShaderObjectParameterInformation>& objects)
		{
			for (const auto& entry : objects)
			{
				const ShaderObjectParameterInformation& declaration = entry.second;
				if (declaration.Set == ~0u || declaration.Slot == ~0u)
					continue;

				HLSLRegisterClass registerClass;
				Map<String, GpuObjectParameterInformation>* target;
				if (GpuObjectParameterTypeInformation::IsSampler(declaration.Type))
				{
					registerClass = HLSLRegisterClass::Sampler;
					target = &parameterDescription.Samplers;
				}
				else if (GpuObjectParameterTypeInformation::IsReadWriteTexture(declaration.Type))
				{
					registerClass = HLSLRegisterClass::UnorderedAccess;
					target = &parameterDescription.StorageTextures;
				}
				else if (GpuObjectParameterTypeInformation::IsTexture(declaration.Type))
				{
					registerClass = HLSLRegisterClass::ShaderResource;
					target = &parameterDescription.SampledTextures;
				}
				else if (GpuObjectParameterTypeInformation::IsReadWriteBuffer(declaration.Type))
				{
					registerClass = HLSLRegisterClass::UnorderedAccess;
					target = &parameterDescription.Buffers;
				}
				else if (declaration.Type == GPOT_ACCELERATION_STRUCTURE)
				{
					registerClass = HLSLRegisterClass::ShaderResource;
					target = &parameterDescription.AccelerationStructures;
				}
				else
				{
					registerClass = HLSLRegisterClass::ShaderResource;
					target = &parameterDescription.Buffers;
				}

				const u32 slot = MapRegisterToSlot(declaration.Slot, registerClass);
				if (!reflectedBindings.insert(fnBindingKey(declaration.Set, slot)).second)
					continue;

				GpuObjectParameterInformation parameterInformation;
				parameterInformation.Name = declaration.GpuVariableName;
				parameterInformation.Type = registerClass == HLSLRegisterClass::Sampler ? GPOT_SAMPLER2D : declaration.Type;
				parameterInformation.Set = declaration.Set;
				parameterInformation.Slot = slot;
				parameterInformation.ArraySize = std::max(declaration.ArraySize, 1u);
				(*target)[parameterInformation.Name] = parameterInformation;
			}
		};

		fnAddObjects(declarations.GetTextureParameters());
		fnAddObjects(declarations.GetBufferParameters());
		fnAddObjects(declarations.GetSamplerParameters());
	}

	/** Returns the DXIL shader kind of a ray tracing program type. */
	D3D12_SHADER_VERSION_TYPE GetRayTracingShaderKind(GpuProgramType type)
	{
		switch (type)
		{
		case GPT_RAY_GENERATION_PROGRAM: return D3D12_SHVER_RAY_GENERATION_SHADER;
		case GPT_MISS_PROGRAM: return D3D12_SHVER_MISS_SHADER;
		case GPT_CLOSEST_HIT_PROGRAM: return D3D12_SHVER_CLOSEST_HIT_SHADER;
		case GPT_ANY_HIT_PROGRAM: return D3D12_SHVER_ANY_HIT_SHADER;
		case GPT_INTERSECTION_PROGRAM: return D3D12_SHVER_INTERSECTION_SHADER;
		case GPT_CALLABLE_PROGRAM: return D3D12_SHVER_CALLABLE_SHADER;
		default:
			B3D_ASSERT(false);
			return D3D12_SHVER_RESERVED0;
		}
	}

	/** Reflects the parameters and resource table layout of a ray tracing program compiled as a DXIL library. */
	bool ReflectLibrary(IDxcUtils* utilities, const DxcBuffer& reflectionBuffer, GpuProgramType type, GpuProgramBytecode& bytecode)
	{
		ComPtr<ID3D12LibraryReflection> reflection;
		HRESULT hr = utilities->CreateReflection(&reflectionBuffer, IID_PPV_ARGS(&reflection));
		if (FAILED(hr))
		{
			bytecode.Messages = "Failed to reflect compiled HLSL library.";
			B3D_LOG(Error, LogRenderBackend, "{0}", bytecode.Messages);
			return false;
		}

		D3D12_LIBRARY_DESC libraryDesc;
		reflection->GetDesc(&libraryDesc);

		// The program's function is the one whose kind matches its type, whatever else the library exports
		const D3D12_SHADER_VERSION_TYPE shaderKind = GetRayTracingShaderKind(type);
		ID3D12FunctionReflection* functionReflection = nullptr;
		D3D12_FUNCTION_DESC functionDesc;
		u32 matchingFunctionCount = 0;
		for (u32 functionIndex = 0; functionIndex < libraryDesc.FunctionCount; functionIndex++)
		{
			ID3D12FunctionReflection* candidate = reflection->GetFunctionByIndex((INT)functionIndex);
			D3D12_FUNCTION_DESC candidateDesc;
			if (candidate == nullptr || FAILED(candidate->GetDesc(&candidateDesc)) || D3D12_SHVER_GET_TYPE(candidateDesc.Version) != (UINT)shaderKind)
				continue;

			functionReflection = candidate;
			functionDesc = candidateDesc;
			matchingFunctionCount++;
		}

		if (matchingFunctionCount != 1)
		{
			bytecode.Messages = StringUtility::Format("Compiled HLSL library exports {0} {1} shaders, expected exactly one.", matchingFunctionCount, GetProgramTypeName(type));
			B3D_LOG(Error, LogRenderBackend, "{0}", bytecode.Messages);
			return false;
		}

		if (!ReflectConstantBuffers(functionReflection, functionDesc, *bytecode.ParameterDescription, bytecode.Messages))
			return false;

		ReflectBoundResources(functionReflection, functionDesc, *bytecode.ParameterDescription);
		BuildResourceTableLayout(functionReflection, functionDesc, bytecode);

		return true;
	}

	/** Performs shader reflection to extract parameter descriptions, vertex inputs, and execution requirements. */
	bool ReflectShader(IDxcUtils* utilities, IDxcResult* compilationResult, GpuProgramType type, GpuProgramBytecode& bytecode)
	{
		ComPtr<IDxcBlob> reflectionBlob;
		HRESULT hr = compilationResult->GetOutput(DXC_OUT_REFLECTION, IID_PPV_ARGS(&reflectionBlob), nullptr);
		if(FAILED(hr) || reflectionBlob == nullptr)
		{
			bytecode.Messages = "Failed to retrieve compiled HLSL shader reflection.";
			B3D_LOG(Error, LogRenderBackend, "{0}", bytecode.Messages);
			return false;
		}

		DxcBuffer reflectionBuffer;
		reflectionBuffer.Ptr = reflectionBlob->GetBufferPointer();
		reflectionBuffer.Size = reflectionBlob->GetBufferSize();
		reflectionBuffer.Encoding = 0;

		if (!bytecode.ParameterDescription)
			bytecode.ParameterDescription = B3DMakeShared<GpuProgramParameterDescription>();

		if (IsRayTracingProgramType(type))
		{
			return ReflectLibrary(utilities, reflectionBuffer, type, bytecode);
		}
		else
		{
			ComPtr<ID3D12ShaderReflection> reflection;
			hr = utilities->CreateReflection(&reflectionBuffer, IID_PPV_ARGS(&reflection));
			if (FAILED(hr))
			{
				bytecode.Messages = "Failed to reflect compiled HLSL shader.";
				B3D_LOG(Error, LogRenderBackend, "{0}", bytecode.Messages);
				return false;
			}

			D3D12_SHADER_DESC shaderDesc;
			reflection->GetDesc(&shaderDesc);

			if (!ReflectConstantBuffers(reflection.Get(), shaderDesc, *bytecode.ParameterDescription, bytecode.Messages))
				return false;

			ReflectBoundResources(reflection.Get(), shaderDesc, *bytecode.ParameterDescription);
			BuildResourceTableLayout(reflection.Get(), shaderDesc, bytecode);

			if (type == GPT_VERTEX_PROGRAM)
				ReflectVertexInput(reflection.Get(), shaderDesc, bytecode.VertexInput);
			else if(type == GPT_COMPUTE_PROGRAM)
				reflection->GetThreadGroupSize(&bytecode.ThreadGroupSize[0], &bytecode.ThreadGroupSize[1], &bytecode.ThreadGroupSize[2]);

			return true;
		}
	}
} // namespace

HLSLToDXIL::HLSLToDXIL(const char* compilerId, u32 compilerVersion)
	: DxcBytecodeCompiler(compilerId, compilerVersion)
{ }

bool HLSLToDXIL::ReflectBytecode(const GpuProgramCreateInformation& createInformation, IDxcResult& result, IDxcBlob& program, GpuProgramBytecode& outBytecode)
{
	if(outBytecode.Messages.empty())
		outBytecode.Messages = "Shader compiled successfully";

	if (!ReflectShader(mUtilities.Get(), &result, createInformation.Type, outBytecode))
		return false;

	if(createInformation.ShaderReflection != nullptr && createInformation.ShaderReflection->Parameters != nullptr)
	{
		ShaderParameterDescription::IterateMatching(createInformation.ShaderReflection->Parameters->GetUniformBuffers(), outBytecode.ParameterDescription->UniformBuffers, [](const auto& source, auto& outTarget) { outTarget.UsesDynamicOffset = source.UsesDynamicOffset; }, [](u32 slot) { return MapRegisterToSlot(slot, HLSLRegisterClass::ConstantBuffer); });
		AddUnusedDeclarations(*createInformation.ShaderReflection->Parameters, *outBytecode.ParameterDescription);
	}

	// DXIL reflection doesn't report descriptor heap accesses
	const GpuProgramEntryPointReflection& entryPointReflection = createInformation.GetEntryPointReflection();
	outBytecode.ParameterDescription->Features = entryPointReflection.Features;

	return true;
}
