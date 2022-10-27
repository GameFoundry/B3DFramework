//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsD3D11HLSLParamParser.h"
#include "BsD3D11Mappings.h"
#include "RenderAPI/BsGpuParamDesc.h"
#include "Error/BsException.h"
#include "Debug/BsDebug.h"
#include "Math/BsMath.h"

using namespace bs;
using namespace bs::ct;

void D3D11HLSLParamParser::Parse(ID3DBlob* microcode, GpuProgramType type, GpuParamDesc& desc, Vector<VertexElement>* inputParams)
{
	const char* commentString = nullptr;
	ID3DBlob* pIDisassembly = nullptr;
	char* pDisassembly = nullptr;

	HRESULT hr = D3DDisassemble((UINT*)microcode->GetBufferPointer(), microcode->GetBufferSize(), D3D_DISASM_ENABLE_COLOR_CODE, commentString, &pIDisassembly);

	const char* assemblyCode = static_cast<const char*>(pIDisassembly->GetBufferPointer());

	if(FAILED(hr))
		BS_EXCEPT(RenderingAPIException, "Unable to disassemble shader.");

	ID3D11ShaderReflection* shaderReflection;
	hr = D3DReflect((void*)microcode->GetBufferPointer(), microcode->GetBufferSize(), IID_ID3D11ShaderReflection, (void**)&shaderReflection);

	if(FAILED(hr))
		BS_EXCEPT(RenderingAPIException, "Cannot reflect D3D11 high-level shader.");

	D3D11_SHADER_DESC shaderDesc;
	hr = shaderReflection->GetDesc(&shaderDesc);

	if(FAILED(hr))
		BS_EXCEPT(RenderingAPIException, "Cannot reflect D3D11 high-level shader.");

	if(inputParams != nullptr)
	{
		D3D11_SIGNATURE_PARAMETER_DESC inputParamDesc;
		for(u32 i = 0; i < shaderDesc.InputParameters; i++)
		{
			hr = shaderReflection->GetInputParameterDesc(i, &inputParamDesc);

			if(FAILED(hr))
				BS_EXCEPT(RenderingAPIException, "Cannot get input param desc with index: " + toString(i));

			// We don't care about system value semantics
			if(StringUtil::StartsWith(String(inputParamDesc.SemanticName), "sv_"))
				continue;

			inputParams->push_back(VertexElement(inputParamDesc.Stream, inputParamDesc.Register, D3D11Mappings::GetInputType(inputParamDesc.ComponentType), D3D11Mappings::Get(inputParamDesc.SemanticName), inputParamDesc.SemanticIndex));
		}
	}

	for(u32 i = 0; i < shaderDesc.BoundResources; i++)
	{
		D3D11_SHADER_INPUT_BIND_DESC bindingDesc;
		hr = shaderReflection->GetResourceBindingDesc(i, &bindingDesc);

		if(FAILED(hr))
			BS_EXCEPT(RenderingAPIException, "Cannot get resource binding desc with index: " + toString(i));

		ParseResource(bindingDesc, type, desc);
	}

	for(u32 i = 0; i < shaderDesc.ConstantBuffers; i++)
	{
		ID3D11ShaderReflectionConstantBuffer* shaderReflectionConstantBuffer;
		shaderReflectionConstantBuffer = shaderReflection->GetConstantBufferByIndex(i);

		ParseBuffer(shaderReflectionConstantBuffer, desc);
	}

	shaderReflection->Release();
}

void D3D11HLSLParamParser::ParseResource(D3D11_SHADER_INPUT_BIND_DESC& resourceDesc, GpuProgramType type, GpuParamDesc& desc)
{
	for(u32 i = 0; i < resourceDesc.BindCount; i++)
	{
		if(resourceDesc.Type == D3D_SIT_CBUFFER || resourceDesc.Type == D3D_SIT_TBUFFER)
		{
			GpuParamBlockDesc blockDesc;
			blockDesc.Name = resourceDesc.Name;
			blockDesc.Slot = resourceDesc.BindPoint + i;
			blockDesc.Set = MapParameterToSet(type, ParamType::ConstantBuffer);
			blockDesc.BlockSize = 0; // Calculated manually as we add parameters

			if(strcmp(resourceDesc.Name, "$Globals") == 0 || strcmp(resourceDesc.Name, "$Param") == 0) // Special buffers, as defined by DX11 docs
				blockDesc.IsShareable = false;
			else
				blockDesc.IsShareable = true;

			desc.ParamBlocks.insert(std::make_pair(blockDesc.Name, blockDesc));
		}
		else
		{
			GpuParamObjectDesc memberDesc;
			memberDesc.Name = resourceDesc.Name;
			memberDesc.Slot = resourceDesc.BindPoint + i;
			memberDesc.Type = GPOT_UNKNOWN;

			switch(resourceDesc.Type)
			{
			case D3D_SIT_SAMPLER:
				memberDesc.Type = GPOT_SAMPLER2D; // Actual dimension of the sampler doesn't matter
				memberDesc.Set = MapParameterToSet(type, ParamType::Sampler);

				desc.Samplers.insert(std::make_pair(memberDesc.Name, memberDesc));
				break;
			case D3D_SIT_TEXTURE:
				{
					bool isTexture = true;
					switch(resourceDesc.Dimension)
					{
					case D3D_SRV_DIMENSION_TEXTURE1D:
						memberDesc.Type = GPOT_TEXTURE1D;
						break;
					case D3D_SRV_DIMENSION_TEXTURE1DARRAY:
						memberDesc.Type = GPOT_TEXTURE1DARRAY;
						break;
					case D3D_SRV_DIMENSION_TEXTURE2D:
						memberDesc.Type = GPOT_TEXTURE2D;
						break;
					case D3D_SRV_DIMENSION_TEXTURE2DARRAY:
						memberDesc.Type = GPOT_TEXTURE2DARRAY;
						break;
					case D3D_SRV_DIMENSION_TEXTURE3D:
						memberDesc.Type = GPOT_TEXTURE3D;
						break;
					case D3D_SRV_DIMENSION_TEXTURECUBE:
						memberDesc.Type = GPOT_TEXTURECUBE;
						break;
					case D3D_SRV_DIMENSION_TEXTURECUBEARRAY:
						memberDesc.Type = GPOT_TEXTURECUBEARRAY;
						break;
					case D3D_SRV_DIMENSION_TEXTURE2DMS:
						memberDesc.Type = GPOT_TEXTURE2DMS;
						break;
					case D3D_SRV_DIMENSION_TEXTURE2DMSARRAY:
						memberDesc.Type = GPOT_TEXTURE2DMSARRAY;
						break;
					case D3D_SRV_DIMENSION_BUFFER:
						memberDesc.Type = GPOT_BYTE_BUFFER;
						isTexture = false;
						break;
					default:
						BS_LOG(Warning, RenderBackend, "Skipping texture because it has unsupported dimension: {0}", resourceDesc.Dimension);
					}

					if(memberDesc.Type != GPOT_UNKNOWN)
					{
						memberDesc.Set = MapParameterToSet(type, ParamType::Texture);

						if(isTexture)
							desc.Textures.insert(std::make_pair(memberDesc.Name, memberDesc));
						else
							desc.Buffers.insert(std::make_pair(memberDesc.Name, memberDesc));
					}
				}
				break;
			case D3D_SIT_STRUCTURED:
				memberDesc.Type = GPOT_STRUCTURED_BUFFER;
				memberDesc.Set = MapParameterToSet(type, ParamType::Texture);

				desc.Buffers.insert(std::make_pair(memberDesc.Name, memberDesc));
				break;
			case D3D_SIT_BYTEADDRESS:
				memberDesc.Type = GPOT_BYTE_BUFFER;
				memberDesc.Set = MapParameterToSet(type, ParamType::Texture);

				desc.Buffers.insert(std::make_pair(memberDesc.Name, memberDesc));
				break;
			case D3D11_SIT_UAV_RWTYPED:
				{
					memberDesc.Set = MapParameterToSet(type, ParamType::UAV);

					switch(resourceDesc.Dimension)
					{
					case D3D_SRV_DIMENSION_TEXTURE1D:
						memberDesc.Type = GPOT_RWTEXTURE1D;
						desc.LoadStoreTextures.insert(std::make_pair(memberDesc.Name, memberDesc));
						break;
					case D3D_SRV_DIMENSION_TEXTURE1DARRAY:
						memberDesc.Type = GPOT_RWTEXTURE1DARRAY;
						desc.LoadStoreTextures.insert(std::make_pair(memberDesc.Name, memberDesc));
						break;
					case D3D_SRV_DIMENSION_TEXTURE2D:
						memberDesc.Type = GPOT_RWTEXTURE2D;
						desc.LoadStoreTextures.insert(std::make_pair(memberDesc.Name, memberDesc));
						break;
					case D3D_SRV_DIMENSION_TEXTURE2DARRAY:
						memberDesc.Type = GPOT_RWTEXTURE2DARRAY;
						desc.LoadStoreTextures.insert(std::make_pair(memberDesc.Name, memberDesc));
						break;
					case D3D_SRV_DIMENSION_TEXTURE3D:
						memberDesc.Type = GPOT_RWTEXTURE3D;
						desc.LoadStoreTextures.insert(std::make_pair(memberDesc.Name, memberDesc));
						break;
					case D3D_SRV_DIMENSION_TEXTURE2DMS:
						memberDesc.Type = GPOT_RWTEXTURE2DMS;
						desc.LoadStoreTextures.insert(std::make_pair(memberDesc.Name, memberDesc));
						break;
					case D3D_SRV_DIMENSION_TEXTURE2DMSARRAY:
						memberDesc.Type = GPOT_RWTEXTURE2DMSARRAY;
						desc.LoadStoreTextures.insert(std::make_pair(memberDesc.Name, memberDesc));
						break;
					case D3D_SRV_DIMENSION_BUFFER:
						memberDesc.Type = GPOT_RWTYPED_BUFFER;
						desc.Buffers.insert(std::make_pair(memberDesc.Name, memberDesc));
						break;
					default:
						BS_LOG(Warning, RenderBackend, "Skipping typed UAV because it has unsupported dimension: {0}", resourceDesc.Dimension);
					}

					break;
				}
			case D3D11_SIT_UAV_RWSTRUCTURED:
				memberDesc.Type = GPOT_RWSTRUCTURED_BUFFER;
				memberDesc.Set = MapParameterToSet(type, ParamType::UAV);

				desc.Buffers.insert(std::make_pair(memberDesc.Name, memberDesc));
				break;
			case D3D11_SIT_UAV_RWBYTEADDRESS:
				memberDesc.Type = GPOT_RWBYTE_BUFFER;
				memberDesc.Set = MapParameterToSet(type, ParamType::UAV);

				desc.Buffers.insert(std::make_pair(memberDesc.Name, memberDesc));
				break;
			case D3D_SIT_UAV_APPEND_STRUCTURED:
				memberDesc.Type = GPOT_RWAPPEND_BUFFER;
				memberDesc.Set = MapParameterToSet(type, ParamType::UAV);

				desc.Buffers.insert(std::make_pair(memberDesc.Name, memberDesc));
				break;
			case D3D_SIT_UAV_CONSUME_STRUCTURED:
				memberDesc.Type = GPOT_RWCONSUME_BUFFER;
				memberDesc.Set = MapParameterToSet(type, ParamType::UAV);

				desc.Buffers.insert(std::make_pair(memberDesc.Name, memberDesc));
				break;
			case D3D_SIT_UAV_RWSTRUCTURED_WITH_COUNTER:
				memberDesc.Type = GPOT_RWSTRUCTURED_BUFFER_WITH_COUNTER;
				memberDesc.Set = MapParameterToSet(type, ParamType::UAV);

				desc.Buffers.insert(std::make_pair(memberDesc.Name, memberDesc));
				break;
			default:
				BS_LOG(Warning, RenderBackend, "Skipping resource because it has unsupported type: {0}", resourceDesc.Type);
			}
		}
	}
}

void D3D11HLSLParamParser::ParseBuffer(ID3D11ShaderReflectionConstantBuffer* bufferReflection, GpuParamDesc& desc)
{
	D3D11_SHADER_BUFFER_DESC constantBufferDesc;
	HRESULT hr = bufferReflection->GetDesc(&constantBufferDesc);
	if(FAILED(hr))
		BS_EXCEPT(RenderingAPIException, "Failed to retrieve HLSL constant buffer description.");

	if(constantBufferDesc.Type != D3D_CT_CBUFFER && constantBufferDesc.Type != D3D_CT_TBUFFER)
	{
		// Not supported (most likely a constant buffer used internally by DX)
		return;
	}

	GpuParamBlockDesc& blockDesc = desc.ParamBlocks[constantBufferDesc.Name];

	for(u32 j = 0; j < constantBufferDesc.Variables; j++)
	{
		ID3D11ShaderReflectionVariable* varRef = bufferReflection->GetVariableByIndex(j);
		D3D11_SHADER_VARIABLE_DESC varDesc;
		hr = varRef->GetDesc(&varDesc);

		if(FAILED(hr))
			BS_EXCEPT(RenderingAPIException, "Failed to retrieve HLSL constant buffer variable description.");

		ID3D11ShaderReflectionType* varRefType = varRef->GetType();
		D3D11_SHADER_TYPE_DESC varTypeDesc;
		varRefType->GetDesc(&varTypeDesc);

		ParseVariable(varTypeDesc, varDesc, desc, blockDesc);
	}

	blockDesc.BlockSize = constantBufferDesc.Size / 4;
}

void D3D11HLSLParamParser::ParseVariable(D3D11_SHADER_TYPE_DESC& varTypeDesc, D3D11_SHADER_VARIABLE_DESC& varDesc, GpuParamDesc& desc, GpuParamBlockDesc& paramBlock)
{
	GpuParamDataDesc memberDesc;
	memberDesc.Name = varDesc.Name;
	memberDesc.ParamBlockSlot = paramBlock.Slot;
	memberDesc.ParamBlockSet = paramBlock.Set;
	memberDesc.ArraySize = varTypeDesc.Elements == 0 ? 1 : varTypeDesc.Elements;
	memberDesc.GpuMemOffset = varDesc.StartOffset / 4;
	memberDesc.CpuMemOffset = varDesc.StartOffset / 4;

	// Determine individual element size in the array
	if(memberDesc.ArraySize > 1)
	{
		// Find array element size (reported size is total size of array, minus unused register slots)
		int totalArraySize = (varDesc.Size / 4);

		int totalSlotsUsedByArray = Math::DivideAndRoundUp(totalArraySize, 4) * 4;
		int unusedSlotsInArray = totalSlotsUsedByArray - totalArraySize;

		memberDesc.ArrayElementStride = totalSlotsUsedByArray / memberDesc.ArraySize;
		memberDesc.ElementSize = memberDesc.ArrayElementStride - unusedSlotsInArray;
	}
	else
	{
		memberDesc.ElementSize = varDesc.Size / 4; // Stored in multiples of 4
		memberDesc.ArrayElementStride = memberDesc.ElementSize;
	}

	switch(varTypeDesc.Class)
	{
	case D3D_SVC_SCALAR:
		{
			switch(varTypeDesc.Type)
			{
			case D3D_SVT_BOOL:
				memberDesc.Type = GPDT_BOOL;
				break;
			case D3D_SVT_INT:
			case D3D_SVT_UINT:
				memberDesc.Type = GPDT_INT1;
				break;
			case D3D_SVT_FLOAT:
				memberDesc.Type = GPDT_FLOAT1;
				break;
			default:
				BS_LOG(Warning, RenderBackend, "Skipping variable because it has unsupported type: {0}", varTypeDesc.Type);
			}
		}
		break;
	case D3D_SVC_VECTOR:
		{
			switch(varTypeDesc.Type)
			{
			case D3D_SVT_UINT:
			case D3D_SVT_INT:
				{
					switch(varTypeDesc.Columns)
					{
					case 1:
						memberDesc.Type = GPDT_INT1;
						break;
					case 2:
						memberDesc.Type = GPDT_INT2;
						break;
					case 3:
						memberDesc.Type = GPDT_INT3;
						break;
					case 4:
						memberDesc.Type = GPDT_INT4;
						break;
					}
				}

				break;
			case D3D_SVT_FLOAT:
				{
					switch(varTypeDesc.Columns)
					{
					case 1:
						memberDesc.Type = GPDT_FLOAT1;
						break;
					case 2:
						memberDesc.Type = GPDT_FLOAT2;
						break;
					case 3:
						memberDesc.Type = GPDT_FLOAT3;
						break;
					case 4:
						memberDesc.Type = GPDT_FLOAT4;
						break;
					}
				}

				break;
			}
		}
		break;
	case D3D_SVC_MATRIX_COLUMNS:
	case D3D_SVC_MATRIX_ROWS:
		switch(varTypeDesc.Rows)
		{
		case 2:
			switch(varTypeDesc.Columns)
			{
			case 2:
				memberDesc.Type = GPDT_MATRIX_2X2;
				break;
			case 3:
				memberDesc.Type = GPDT_MATRIX_2X3;
				break;
			case 4:
				memberDesc.Type = GPDT_MATRIX_2X4;
				break;
			}
			break;
		case 3:
			switch(varTypeDesc.Columns)
			{
			case 2:
				memberDesc.Type = GPDT_MATRIX_3X2;
				break;
			case 3:
				memberDesc.Type = GPDT_MATRIX_3X3;
				break;
			case 4:
				memberDesc.Type = GPDT_MATRIX_3X4;
				break;
			}
			break;
		case 4:
			switch(varTypeDesc.Columns)
			{
			case 2:
				memberDesc.Type = GPDT_MATRIX_4X2;
				break;
			case 3:
				memberDesc.Type = GPDT_MATRIX_4X3;
				break;
			case 4:
				memberDesc.Type = GPDT_MATRIX_4X4;
				break;
			}
			break;
		}
		break;
	case D3D_SVC_STRUCT:
		memberDesc.Type = GPDT_STRUCT;
		break;
	default:
		BS_LOG(Warning, RenderBackend, "Skipping variable because it has unsupported class: {0}", varTypeDesc.Class);
	}

	desc.Params.insert(std::make_pair(memberDesc.Name, memberDesc));
}

u32 D3D11HLSLParamParser::MapParameterToSet(GpuProgramType progType, ParamType paramType)
{
	u32 progTypeIdx = (u32)progType;
	u32 paramTypeIdx = (u32)paramType;

	return progTypeIdx * (u32)ParamType::Count + paramTypeIdx;
}
