//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DD3D12ShaderCompiler.h"
#include "GpuBackend/B3DGpuProgramParameterDescription.h"
#include "GpuBackend/B3DVertexDescription.h"

#include <d3dcompiler.h>
#include <d3d12shader.h>
#include <cctype>

#pragma comment(lib, "d3dcompiler.lib")

using namespace b3d;
using namespace b3d::render;

namespace
{
	/** Compiler ID for D3D12 HLSL compiler */
	static const String kD3D12CompilerId = "D3D12_DXC";
	static const u32 kD3D12CompilerVersion = 1;
}

const char* D3D12ShaderCompiler::GetShaderTarget(GpuProgramType type)
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

bool D3D12ShaderCompiler::CompileShader(const GpuProgramCreateInformation& desc, TShared<GpuProgramBytecode>& bytecode)
{
	const char* target = GetShaderTarget(desc.Type);
	if (!target)
	{
		if (bytecode)
			bytecode->Messages = "Unsupported shader type";
		return false;
	}

	// Set up compilation flags
	UINT compileFlags = 0;
#if B3D_BUILD_TYPE_DEVELOPMENT
	compileFlags |= D3DCOMPILE_DEBUG;
	compileFlags |= D3DCOMPILE_SKIP_OPTIMIZATION;
#else
	compileFlags |= D3DCOMPILE_OPTIMIZATION_LEVEL3;
#endif

	// Enable strict mode for better error checking
	compileFlags |= D3DCOMPILE_ENABLE_STRICTNESS;

	// Compile the shader
	ComPtr<ID3DBlob> shaderBlob;
	ComPtr<ID3DBlob> errorBlob;
	HRESULT hr = D3DCompile(
		desc.Source.c_str(),
		desc.Source.size(),
		desc.Name.c_str(),
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
			String errorMsg = String("Shader compilation failed:\n") +
				(const char*)errorBlob->GetBufferPointer();
			if (bytecode)
				bytecode->Messages = errorMsg;
			B3D_LOG(Error, LogRenderBackend, "Failed to compile shader '{0}':\n{1}", desc.Name, errorMsg);
		}
		else
		{
			String errorMsg = "Shader compilation failed with unknown error";
			if (bytecode)
				bytecode->Messages = errorMsg;
			B3D_LOG(Error, LogRenderBackend, "Failed to compile shader '{0}': {1}", desc.Name, errorMsg);
		}

		return false;
	}

	// Check for warnings
	if (errorBlob)
	{
		String warningMsg = String("Shader compiled with warnings:\n") +
			(const char*)errorBlob->GetBufferPointer();
		if (bytecode)
			bytecode->Messages = warningMsg;
		B3D_LOG(Warning, LogRenderBackend, "Shader '{0}' compiled with warnings:\n{1}", desc.Name, warningMsg);
	}
	else
	{
		if (bytecode)
			bytecode->Messages = "Shader compiled successfully";
	}

	// Create bytecode object if it doesn't exist
	if (!bytecode)
		bytecode = B3DMakeShared<GpuProgramBytecode>();

	// Store compiler information
	bytecode->CompilerId = kD3D12CompilerId;
	bytecode->CompilerVersion = kD3D12CompilerVersion;

	// Store the compiled bytecode
	u32 size = (u32)shaderBlob->GetBufferSize();
	u8* data = (u8*)B3DAllocate(size);
	memcpy(data, shaderBlob->GetBufferPointer(), size);

	bytecode->Instructions.Size = size;
	bytecode->Instructions.Data = data;

	// Perform shader reflection to extract parameter and vertex input information
	ReflectShader(shaderBlob.Get(), desc.Type, *bytecode);

	return true;
}

void D3D12ShaderCompiler::ReflectShader(ID3DBlob* shaderBlob, GpuProgramType type, GpuProgramBytecode& bytecode)
{
	// Create shader reflection interface
	ComPtr<ID3D12ShaderReflection> reflection;
	HRESULT hr = D3DReflect(
		shaderBlob->GetBufferPointer(),
		shaderBlob->GetBufferSize(),
		IID_PPV_ARGS(&reflection)
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

	// Reflect constant buffers
	ReflectConstantBuffers(reflection.Get(), shaderDesc, *bytecode.ParameterDescription);

	// Reflect bound resources (textures, samplers, UAVs)
	ReflectBoundResources(reflection.Get(), shaderDesc, *bytecode.ParameterDescription);

	// Reflect vertex input for vertex shaders
	if (type == GPT_VERTEX_PROGRAM)
	{
		ReflectVertexInput(reflection.Get(), shaderDesc, bytecode.VertexInput);
	}
}

void D3D12ShaderCompiler::ReflectConstantBuffers(ID3D12ShaderReflection* reflection, const D3D12_SHADER_DESC& shaderDesc,
	GpuProgramParameterDescription& paramDesc)
{
	for (u32 i = 0; i < shaderDesc.ConstantBuffers; i++)
	{
		ID3D12ShaderReflectionConstantBuffer* cbReflection = reflection->GetConstantBufferByIndex(i);
		D3D12_SHADER_BUFFER_DESC cbDesc;
		cbReflection->GetDesc(&cbDesc);

		// Get the resource binding information
		D3D12_SHADER_INPUT_BIND_DESC bindDesc;
		for (u32 j = 0; j < shaderDesc.BoundResources; j++)
		{
			reflection->GetResourceBindingDesc(j, &bindDesc);
			if (strcmp(bindDesc.Name, cbDesc.Name) == 0)
				break;
		}

		// Create uniform buffer information
		GpuDataParameterBlockInformation bufferInfo;
		bufferInfo.Name = cbDesc.Name;
		bufferInfo.Slot = bindDesc.BindPoint;
		bufferInfo.Set = bindDesc.Space; // Register space maps to descriptor set
		bufferInfo.Size = cbDesc.Size;

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
			GpuDataParameterInformation paramInfo;
			paramInfo.Name = varDesc.Name;
			paramInfo.ParamType = ConvertD3DTypeToGpuParamDataType(typeDesc);
			paramInfo.ElementSize = varDesc.Size;
			paramInfo.ParentUniformBufferSet = bufferInfo.Set;
			paramInfo.ParentUniformBufferSlot = bufferInfo.Slot;

			paramDesc.UniformBufferMembers[paramInfo.Name] = paramInfo;
		}

		paramDesc.UniformBuffers[bufferInfo.Name] = bufferInfo;
	}
}

void D3D12ShaderCompiler::ReflectBoundResources(ID3D12ShaderReflection* reflection, const D3D12_SHADER_DESC& shaderDesc,
	GpuProgramParameterDescription& paramDesc)
{
	for (u32 i = 0; i < shaderDesc.BoundResources; i++)
	{
		D3D12_SHADER_INPUT_BIND_DESC bindDesc;
		reflection->GetResourceBindingDesc(i, &bindDesc);

		GpuObjectParameterInformation paramInfo;
		paramInfo.Name = bindDesc.Name;
		paramInfo.Slot = bindDesc.BindPoint;
		paramInfo.Set = bindDesc.Space;

		switch (bindDesc.Type)
		{
		case D3D_SIT_TEXTURE:
			paramInfo.Type = GPOT_TEXTURE2D; // TODO: Detect actual texture type from dimension
			paramDesc.Textures[paramInfo.Name] = paramInfo;
			break;

		case D3D_SIT_SAMPLER:
			paramInfo.Type = GPOT_SAMPLER_STATE;
			paramDesc.Samplers[paramInfo.Name] = paramInfo;
			break;

		case D3D_SIT_UAV_RWTYPED:
		case D3D_SIT_UAV_RWSTRUCTURED:
		case D3D_SIT_UAV_RWBYTEADDRESS:
		case D3D_SIT_UAV_APPEND_STRUCTURED:
		case D3D_SIT_UAV_CONSUME_STRUCTURED:
		case D3D_SIT_UAV_RWSTRUCTURED_WITH_COUNTER:
			paramInfo.Type = GPOT_RW_TYPED_BUFFER; // TODO: Distinguish between buffer types
			paramDesc.LoadStoreTextures[paramInfo.Name] = paramInfo;
			break;

		case D3D_SIT_STRUCTURED:
		case D3D_SIT_BYTEADDRESS:
			paramInfo.Type = GPOT_BYTE_BUFFER;
			paramDesc.Buffers[paramInfo.Name] = paramInfo;
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

void D3D12ShaderCompiler::ReflectVertexInput(ID3D12ShaderReflection* reflection, const D3D12_SHADER_DESC& shaderDesc,
	Vector<VertexElement>& vertexInput)
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

		// Create vertex element
		VertexElement element(elementType, semantic, semanticIdx, 0, 0, paramDesc.SemanticIndex);
		vertexInput.push_back(element);
	}
}

GpuParamDataType D3D12ShaderCompiler::ConvertD3DTypeToGpuParamDataType(const D3D12_SHADER_TYPE_DESC& typeDesc)
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
		// No double types in GpuParamDataType, treat as float
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

bool D3D12ShaderCompiler::ParseSemanticName(const char* semanticName, VertexElementSemantic& semantic, u16& index)
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

VertexElementType D3D12ShaderCompiler::ConvertD3DSignatureToVertexType(const D3D12_SIGNATURE_PARAMETER_DESC& desc)
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
