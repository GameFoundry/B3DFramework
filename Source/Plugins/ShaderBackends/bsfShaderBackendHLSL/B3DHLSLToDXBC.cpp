//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DHLSLToDXBC.h"
#include "B3DHLSLShaderABI.h"
#include "GpuBackend/B3DGpuProgram.h"
#include "GpuBackend/B3DGpuProgramParameterDescription.h"
#include "GpuBackend/B3DVertexDescription.h"
#include "Debug/B3DLog.h"

#include <d3dcompiler.h>
#include <d3d12shader.h>
#include <wrl/client.h>
#include <algorithm>
#include <cctype>

using namespace b3d;
using namespace b3d::render;

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

	/** Parses an HLSL semantic name (e.g. "POSITION0") to a semantic type and index. */
	bool ParseSemanticName(const char* semanticName, VertexElementSemantic& semantic, u16& index)
	{
		String name(semanticName);

		// Extract trailing number if present
		index = 0;
		size_t i = name.length();
		while (i > 0 && isdigit(name[i - 1]))
			i--;

		if (i < name.length())
		{
			index = (u16)atoi(name.c_str() + i);
			name = name.substr(0, i);
		}

		// Convert to uppercase for comparison
		for (char& c : name)
			c = (char)toupper(c);

		if (name == "POSITION") semantic = VES_POSITION;
		else if (name == "NORMAL") semantic = VES_NORMAL;
		else if (name == "TANGENT") semantic = VES_TANGENT;
		else if (name == "BITANGENT" || name == "BINORMAL") semantic = VES_BITANGENT;
		else if (name == "COLOR") semantic = VES_COLOR;
		else if (name == "TEXCOORD") semantic = VES_TEXCOORD;
		else if (name == "BLENDINDICES") semantic = VES_BLEND_INDICES;
		else if (name == "BLENDWEIGHT") semantic = VES_BLEND_WEIGHTS;
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

	/** Reflects constant buffers and their members. */
	void ReflectConstantBuffers(ID3D12ShaderReflection* reflection, const D3D12_SHADER_DESC& shaderDesc, GpuProgramParameterDescription& paramDesc)
	{
		for (u32 i = 0; i < shaderDesc.ConstantBuffers; i++)
		{
			ID3D12ShaderReflectionConstantBuffer* cbReflection = reflection->GetConstantBufferByIndex(i);
			D3D12_SHADER_BUFFER_DESC cbDesc;
			cbReflection->GetDesc(&cbDesc);

			// Structured and byte-address buffers also appear in the constant buffer list (as resource bind
			// information blocks); they are reflected as bound resources instead
			if (cbDesc.Type != D3D_CT_CBUFFER)
				continue;

			// Get the resource binding information
			D3D12_SHADER_INPUT_BIND_DESC bindDesc;
			for (u32 j = 0; j < shaderDesc.BoundResources; j++)
			{
				reflection->GetResourceBindingDesc(j, &bindDesc);
				if (strcmp(bindDesc.Name, cbDesc.Name) == 0)
					break;
			}

			// Create uniform buffer information
			GpuUniformBufferInformation bufferInformation;
			bufferInformation.Name = cbDesc.Name;
			bufferInformation.Slot = MapRegisterToSlot(bindDesc.BindPoint, HLSLRegisterClass::ConstantBuffer);
			bufferInformation.Set = bindDesc.Space; // Register space maps to descriptor set
			bufferInformation.Size = cbDesc.Size / 4; // Core API expects size in multiples of 4 bytes
			// TODO(d3d12-port): Stage/IsShareable flags are not derivable from HLSL reflection alone; left at defaults.

			// Reflect constant buffer members
			for (u32 j = 0; j < cbDesc.Variables; j++)
			{
				ID3D12ShaderReflectionVariable* varReflection = cbReflection->GetVariableByIndex(j);
				D3D12_SHADER_VARIABLE_DESC varDesc;
				varReflection->GetDesc(&varDesc);

				ID3D12ShaderReflectionType* typeReflection = varReflection->GetType();
				D3D12_SHADER_TYPE_DESC typeDesc;
				typeReflection->GetDesc(&typeDesc);

				// Create parameter information
				GpuUniformBufferMemberInformation memberInformation;
				memberInformation.Name = varDesc.Name;
				memberInformation.Type = ConvertD3DTypeToGpuDataParameterType(typeDesc);
				memberInformation.ElementSize = varDesc.Size / 4; // Core API expects size in multiples of 4 bytes
				memberInformation.ArraySize = typeDesc.Elements > 0 ? typeDesc.Elements : 1;
				memberInformation.ArrayElementStride = 0; // TODO(d3d12-port): reflect array element stride if needed
				memberInformation.ParentUniformBufferSet = bufferInformation.Set;
				memberInformation.ParentUniformBufferSlot = bufferInformation.Slot;
				memberInformation.GpuOffset = varDesc.StartOffset / 4; // Core API expects offset in multiples of 4 bytes
				memberInformation.CpuOffset = varDesc.StartOffset / 4;

				paramDesc.UniformBufferMembers[memberInformation.Name] = memberInformation;
			}

			paramDesc.UniformBuffers[bufferInformation.Name] = bufferInformation;
		}
	}

	/** Reflects bound resources (textures, samplers, UAVs, etc.). */
	void ReflectBoundResources(ID3D12ShaderReflection* reflection, const D3D12_SHADER_DESC& shaderDesc, GpuProgramParameterDescription& paramDesc)
	{
		for (u32 i = 0; i < shaderDesc.BoundResources; i++)
		{
			D3D12_SHADER_INPUT_BIND_DESC bindDesc;
			reflection->GetResourceBindingDesc(i, &bindDesc);

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
				parameterInformation.Slot = MapRegisterToSlot(bindDesc.BindPoint, HLSLRegisterClass::UnorderedAccess);
				parameterInformation.Type = GPOT_RWSTRUCTURED_BUFFER;
				paramDesc.Buffers[parameterInformation.Name] = parameterInformation;
				break;

			case D3D_SIT_UAV_RWBYTEADDRESS:
				parameterInformation.Slot = MapRegisterToSlot(bindDesc.BindPoint, HLSLRegisterClass::UnorderedAccess);
				parameterInformation.Type = GPOT_RWBYTE_BUFFER;
				paramDesc.Buffers[parameterInformation.Name] = parameterInformation;
				break;

			case D3D_SIT_UAV_APPEND_STRUCTURED:
				parameterInformation.Slot = MapRegisterToSlot(bindDesc.BindPoint, HLSLRegisterClass::UnorderedAccess);
				parameterInformation.Type = GPOT_RWAPPEND_BUFFER;
				paramDesc.Buffers[parameterInformation.Name] = parameterInformation;
				break;

			case D3D_SIT_UAV_CONSUME_STRUCTURED:
				parameterInformation.Slot = MapRegisterToSlot(bindDesc.BindPoint, HLSLRegisterClass::UnorderedAccess);
				parameterInformation.Type = GPOT_RWCONSUME_BUFFER;
				paramDesc.Buffers[parameterInformation.Name] = parameterInformation;
				break;

			case D3D_SIT_UAV_RWSTRUCTURED_WITH_COUNTER:
				parameterInformation.Slot = MapRegisterToSlot(bindDesc.BindPoint, HLSLRegisterClass::UnorderedAccess);
				parameterInformation.Type = GPOT_RWSTRUCTURED_BUFFER_WITH_COUNTER;
				paramDesc.Buffers[parameterInformation.Name] = parameterInformation;
				break;

			case D3D_SIT_STRUCTURED:
				parameterInformation.Slot = MapRegisterToSlot(bindDesc.BindPoint, HLSLRegisterClass::ShaderResource);
				parameterInformation.Type = GPOT_STRUCTURED_BUFFER;
				paramDesc.Buffers[parameterInformation.Name] = parameterInformation;
				break;

			case D3D_SIT_BYTEADDRESS:
				parameterInformation.Slot = MapRegisterToSlot(bindDesc.BindPoint, HLSLRegisterClass::ShaderResource);
				parameterInformation.Type = GPOT_BYTE_BUFFER;
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
	void ReflectVertexInput(ID3D12ShaderReflection* reflection, const D3D12_SHADER_DESC& shaderDesc, Vector<VertexElement>& vertexInput)
	{
		vertexInput.clear();

		for (u32 i = 0; i < shaderDesc.InputParameters; i++)
		{
			D3D12_SIGNATURE_PARAMETER_DESC paramDesc;
			reflection->GetInputParameterDesc(i, &paramDesc);

			// Skip system values
			if (paramDesc.SystemValueType != D3D_NAME_UNDEFINED)
				continue;

			// Parse semantic name to get element semantic and index
			VertexElementSemantic semantic = VES_POSITION;
			u16 semanticIdx = 0;
			if (!ParseSemanticName(paramDesc.SemanticName, semantic, semanticIdx))
			{
				B3D_LOG(Warning, LogRenderBackend, "Unknown vertex semantic: {0}", paramDesc.SemanticName);
				continue;
			}

			// Convert component type to vertex element type
			VertexElementType elementType = ConvertD3DSignatureToVertexType(paramDesc);

			// Reflection reports the semantic base name without digits, with the index in a separate field, so
			// the index parsed from the name is always zero and paramDesc.SemanticIndex is authoritative
			VertexElement element(elementType, semantic, (u16)paramDesc.SemanticIndex);
			vertexInput.push_back(element);
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
		// Group the reflected bindings by register space (engine set), sorted by set then slot
		Map<u32, Vector<GpuDescriptorTableEntry>> setEntries;
		for (u32 i = 0; i < shaderDesc.BoundResources; i++)
		{
			D3D12_SHADER_INPUT_BIND_DESC bindDesc;
			reflection->GetResourceBindingDesc(i, &bindDesc);

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

		for (auto& [set, entries] : setEntries)
		{
			std::sort(entries.begin(), entries.end(), [](const GpuDescriptorTableEntry& lhs, const GpuDescriptorTableEntry& rhs)
			{
				return lhs.Slot < rhs.Slot;
			});

			GpuDescriptorTable table;
			table.Set = set;
			table.FirstEntry = (u32)layout.Entries.size();
			table.EntryCount = (u32)entries.size();
			layout.Tables.push_back(table);
			layout.Entries.insert(layout.Entries.end(), entries.begin(), entries.end());
		}
	}

	/** Performs shader reflection to extract parameter descriptions and vertex inputs. */
	void ReflectShader(ID3DBlob* shaderBlob, GpuProgramType type, GpuProgramBytecode& bytecode)
	{
		// Create shader reflection interface
		ComPtr<ID3D12ShaderReflection> reflection;
		HRESULT hr = D3DReflect(shaderBlob->GetBufferPointer(), shaderBlob->GetBufferSize(), IID_PPV_ARGS(&reflection)
		);

		if (FAILED(hr))
		{
			B3D_LOG(Warning, LogRenderBackend, "Failed to reflect shader");
			return;
		}

		// Get shader description
		D3D12_SHADER_DESC shaderDesc;
		reflection->GetDesc(&shaderDesc);

		// Create parameter description if not already created
		if (!bytecode.ParameterDescription)
			bytecode.ParameterDescription = B3DMakeShared<GpuProgramParameterDescription>();

		ReflectConstantBuffers(reflection.Get(), shaderDesc, *bytecode.ParameterDescription);
		ReflectBoundResources(reflection.Get(), shaderDesc, *bytecode.ParameterDescription);
		BuildResourceTableLayout(reflection.Get(), shaderDesc, bytecode);

		if (type == GPT_VERTEX_PROGRAM)
		{
			ReflectVertexInput(reflection.Get(), shaderDesc, bytecode.VertexInput);
		}
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

	// Set up compilation flags
	UINT compileFlags = 0;
#if B3D_BUILD_TYPE_DEVELOPMENT
	compileFlags |= D3DCOMPILE_DEBUG;
	compileFlags |= D3DCOMPILE_SKIP_OPTIMIZATION; // TODO - Likely want to enable this only under specific circumstances, not drive the development build perf. down always
#else
	compileFlags |= D3DCOMPILE_OPTIMIZATION_LEVEL3;
#endif

	// Enable strict mode for better error checking
	compileFlags |= D3DCOMPILE_ENABLE_STRICTNESS;
	compileFlags |= D3DCOMPILE_PACK_MATRIX_COLUMN_MAJOR;

	// The source name must be a valid filename-like string - the standard include handler derives the include
	// directory from it, and an empty name fails the whole compilation with ERROR_INVALID_NAME.
	const char* sourceName = !desc.Name.empty() ? desc.Name.c_str() : "unnamed_shader";

	// Compile the shader
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

	// Check for warnings
	if (errorBlob)
	{
		String warningMessage = String("Shader compiled with warnings:\n") + (const char*)errorBlob->GetBufferPointer();
		bytecode->Messages = warningMessage;
		B3D_LOG(Warning, LogRenderBackend, "Shader '{0}' compiled with warnings:\n{1}", desc.Name, warningMessage);
	}
	else
	{
		bytecode->Messages = "Shader compiled successfully";
	}

	// Store the compiled bytecode
	u32 size = (u32)shaderBlob->GetBufferSize();
	u8* data = (u8*)B3DAllocate(size);
	memcpy(data, shaderBlob->GetBufferPointer(), size);

	bytecode->Instructions.Size = size;
	bytecode->Instructions.Data = data;

	// Perform shader reflection to extract parameter and vertex input information
	ReflectShader(shaderBlob.Get(), desc.Type, *bytecode);

	return bytecode;
}
