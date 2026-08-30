//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DHLSLToDXBC.h"
#include "B3DHLSLShaderABI.h"
#include "GpuBackend/B3DGpuPushConstants.h"
#include "GpuBackend/B3DGpuProgram.h"
#include "GpuBackend/B3DGpuProgramParameterDescription.h"
#include "GpuBackend/B3DVertexDescription.h"
#include "Debug/B3DLog.h"
#include "Math/B3DMath.h"
#include "String/B3DStringFormat.h"
#include "Utility/B3DConfigVariable.h"

#include <d3dcompiler.h>
#include <d3d12shader.h>
#include <wrl/client.h>
#include <algorithm>
#include <cctype>

using namespace b3d;
using namespace b3d::render;

namespace b3d
{
	TConfigVariable<bool> gSkipShaderOptimization("d3d12.SkipShaderOptimization",
		"Compile HLSL shaders with optimizations disabled and debug info attached, so they can be stepped through in a "
		"shader debugger. Slows down shader execution significantly and makes FXC report spurious 'potentially "
		"uninitialized variable' warnings for functions with more than one return statement.", false,
		ConfigVariableFlag::ReadOnly);
}

namespace
{
	using Microsoft::WRL::ComPtr;

	/** Converts GPU program type to HLSL shader model target. Returns null for unsupported program types. */
	const char* GetShaderTarget(GpuProgramType type)
	{
		switch (type)
		{
		case GPT_VERTEX_PROGRAM:
			return "vs_5_1";
		case GPT_FRAGMENT_PROGRAM:
			return "ps_5_1";
		case GPT_GEOMETRY_PROGRAM:
			return "gs_5_1";
		case GPT_HULL_PROGRAM:
			return "hs_5_1";
		case GPT_DOMAIN_PROGRAM:
			return "ds_5_1";
		case GPT_COMPUTE_PROGRAM:
			return "cs_5_1";
		default:
			return nullptr;
		}
	}

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
		case D3D_SIT_UAV_RWBYTEADDRESS:					return GPOT_RWBYTE_BUFFER;
		case D3D_SIT_UAV_APPEND_STRUCTURED:				return GPOT_RWAPPEND_BUFFER;
		case D3D_SIT_UAV_CONSUME_STRUCTURED:			return GPOT_RWCONSUME_BUFFER;
		case D3D_SIT_UAV_RWSTRUCTURED_WITH_COUNTER:		return GPOT_RWSTRUCTURED_BUFFER_WITH_COUNTER;
		case D3D_SIT_STRUCTURED:						return GPOT_STRUCTURED_BUFFER;
		case D3D_SIT_BYTEADDRESS:
		default:										return GPOT_BYTE_BUFFER;
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

	/** Reflects constant buffers and their members. */
	bool ReflectConstantBuffers(ID3D12ShaderReflection* reflection, const D3D12_SHADER_DESC& shaderDesc, GpuProgramParameterDescription& parameterDescription, String& outMessages)
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

	/** Reflects bound resources (textures, samplers, UAVs, etc.). */
	void ReflectBoundResources(ID3D12ShaderReflection* reflection, const D3D12_SHADER_DESC& shaderDesc, GpuProgramParameterDescription& paramDesc)
	{
		for (u32 resourceIndex = 0; resourceIndex < shaderDesc.BoundResources; resourceIndex++)
		{
			D3D12_SHADER_INPUT_BIND_DESC bindDesc;
			reflection->GetResourceBindingDesc(resourceIndex, &bindDesc);

			GpuObjectParameterInformation parameterInformation;
			parameterInformation.Name = bindDesc.Name;
			parameterInformation.Set = bindDesc.Space;

			switch (bindDesc.Type)
			{
			case D3D_SIT_TEXTURE:
				parameterInformation.Slot = MapRegisterToSlot(bindDesc.BindPoint, HLSLRegisterClass::ShaderResource);
				if (bindDesc.Dimension == D3D_SRV_DIMENSION_BUFFER)
				{
					// Buffer<T> - read-only typed buffer. TODO(d3d12-port): no dedicated read-only typed buffer object
					// type exists; byte buffer is the closest read-only buffer representation.
					parameterInformation.Type = GPOT_BYTE_BUFFER;
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
	 */
	void BuildResourceTableLayout(ID3D12ShaderReflection* reflection, const D3D12_SHADER_DESC& shaderDesc, GpuProgramBytecode& bytecode)
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

	/** Performs shader reflection to extract parameter descriptions and vertex inputs. */
	bool ReflectShader(ID3DBlob* shaderBlob, GpuProgramType type, GpuProgramBytecode& bytecode)
	{
		ComPtr<ID3D12ShaderReflection> reflection;
		HRESULT hr = D3DReflect(shaderBlob->GetBufferPointer(), shaderBlob->GetBufferSize(), IID_PPV_ARGS(&reflection));

		if (FAILED(hr))
		{
			bytecode.Messages = "Failed to reflect compiled HLSL shader.";
			B3D_LOG(Error, LogRenderBackend, "{0}", bytecode.Messages);
			return false;
		}

		D3D12_SHADER_DESC shaderDesc;
		reflection->GetDesc(&shaderDesc);

		if (!bytecode.ParameterDescription)
			bytecode.ParameterDescription = B3DMakeShared<GpuProgramParameterDescription>();

		if (!ReflectConstantBuffers(reflection.Get(), shaderDesc, *bytecode.ParameterDescription, bytecode.Messages))
			return false;

		ReflectBoundResources(reflection.Get(), shaderDesc, *bytecode.ParameterDescription);
		BuildResourceTableLayout(reflection.Get(), shaderDesc, bytecode);

		if (type == GPT_VERTEX_PROGRAM)
			ReflectVertexInput(reflection.Get(), shaderDesc, bytecode.VertexInput);

		return true;
	}
} // namespace

HLSLToDXBC::HLSLToDXBC(const char* compilerId, u32 compilerVersion)
	: mCompilerId(compilerId), mCompilerVersion(compilerVersion)
{
}

bool HLSLToDXBC::IsUpToDate(const GpuProgramBytecode& bytecode) const
{
	return bytecode.CompilerId == mCompilerId && bytecode.CompilerVersion == mCompilerVersion;
}

TShared<GpuProgramBytecode> HLSLToDXBC::CompileBytecode(const GpuProgramCreateInformation& desc)
{
	TShared<GpuProgramBytecode> bytecode = B3DMakeShared<GpuProgramBytecode>();
	bytecode->CompilerId = mCompilerId;
	bytecode->CompilerVersion = mCompilerVersion;

	const char* target = GetShaderTarget(desc.Type);
	if (!target)
	{
		bytecode->Messages = "Unsupported shader type";
		return bytecode;
	}

	// Unoptimized shaders are opt-in even in development builds: they cost a lot of GPU time, and FXC's flow analysis
	// reports a false 'potentially uninitialized variable (<function>)' for every function with an early return once
	// optimizations are off, which buries the real warnings.
	bool skipOptimization = false;
#if B3D_BUILD_TYPE_DEVELOPMENT
	skipOptimization = gSkipShaderOptimization.Get();
#endif

	UINT compileFlags = D3DCOMPILE_ENABLE_STRICTNESS | D3DCOMPILE_PACK_MATRIX_COLUMN_MAJOR;
	compileFlags |= skipOptimization ? (D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION) : D3DCOMPILE_OPTIMIZATION_LEVEL3;

	// The source name must be a valid filename-like string - the standard include handler derives the include
	// directory from it, and an empty name fails the whole compilation with ERROR_INVALID_NAME.
	const char* sourceName = !desc.Name.empty() ? desc.Name.c_str() : "unnamed_shader";

	ComPtr<ID3DBlob> shaderBlob;
	ComPtr<ID3DBlob> errorBlob;
	HRESULT hr = D3DCompile(
		desc.Source.c_str(),
		desc.Source.size(),
		sourceName,
		nullptr, // No defines for now
		D3D_COMPILE_STANDARD_FILE_INCLUDE,
		desc.EntryPoint.c_str(),
		target,
		compileFlags,
		0, // Effect flags (not used for shaders)
		&shaderBlob,
		&errorBlob
	);

	if (FAILED(hr))
	{
		if (errorBlob)
		{
			String errorMessage = String("Shader compilation failed:\n") + (const char*)errorBlob->GetBufferPointer();
			bytecode->Messages = errorMessage;

			// Append the generated source with line numbers so cross-compilation errors can be diagnosed from the
			// log alone (the source only exists in memory, so the compiler's file/line references point nowhere)
			StringStream numberedSource;
			u32 lineNumber = 1;
			size_t lineStart = 0;
			while (lineStart <= desc.Source.size())
			{
				size_t lineEnd = desc.Source.find('\n', lineStart);
				if (lineEnd == String::npos)
					lineEnd = desc.Source.size();

				numberedSource << lineNumber << ": " << desc.Source.substr(lineStart, lineEnd - lineStart) << "\n";
				lineStart = lineEnd + 1;
				lineNumber++;
			}

			B3D_LOG(Error, LogRenderBackend, "Failed to compile shader '{0}':\n{1}\nGenerated source:\n{2}",
				desc.Name, errorMessage, numberedSource.str());
		}
		else
		{
			String errorMessage = "Shader compilation failed with unknown error";
			bytecode->Messages = errorMessage;
			B3D_LOG(Error, LogRenderBackend, "Failed to compile shader '{0}': {1} (hr={2}, entryPoint='{3}', target='{4}', sourceLength={5})",
				desc.Name, errorMessage, (u32)hr, desc.EntryPoint, target, (u64)desc.Source.size());
		}

		return bytecode;
	}

	// A non-null error blob on success carries warnings
	if (errorBlob)
	{
		String warningMessage = String("Shader compiled with warnings:\n") + (const char*)errorBlob->GetBufferPointer();
		bytecode->Messages = warningMessage;
		B3D_LOG(Warning, LogRenderBackend, "Shader '{0}' compiled with warnings:\n{1}", desc.Name, warningMessage);
	}
	else
		bytecode->Messages = "Shader compiled successfully";

	if (!ReflectShader(shaderBlob.Get(), desc.Type, *bytecode))
		return bytecode;

	const u32 bytecodeSize = (u32)shaderBlob->GetBufferSize();
	bytecode->Instructions.Data = (u8*)B3DAllocate(bytecodeSize);
	bytecode->Instructions.Size = bytecodeSize;
	memcpy(bytecode->Instructions.Data, shaderBlob->GetBufferPointer(), bytecodeSize);

	return bytecode;
}
