//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsD3D11HLSLParamParser.h"
#include "BsD3D11Mappings.h"
#include "BsGpuParamDesc.h"
#include "BsException.h"
#include "BsDebug.h"

namespace bs { namespace ct
{
	void D3D11HLSLParamParser::parse(ID3DBlob* microcode, GpuProgramType type, GpuParamDesc& desc, 
		List<VertexElement>* inputParams)
	{
		const char* commentString = nullptr;
		ID3DBlob* pIDisassembly = nullptr;
		char* pDisassembly = nullptr;

		HRESULT hr = D3DDisassemble((UINT*)microcode->GetBufferPointer(), 
			microcode->GetBufferSize(), D3D_DISASM_ENABLE_COLOR_CODE, commentString, &pIDisassembly);

		const char* assemblyCode =  static_cast<const char*>(pIDisassembly->GetBufferPointer());

		if (FAILED(hr))
			BS_EXCEPT(RenderingAPIException, "Unable to disassemble shader.");

		ID3D11ShaderReflection* shaderReflection;
		hr = D3DReflect((void*)microcode->GetBufferPointer(), microcode->GetBufferSize(),
			IID_ID3D11ShaderReflection, (void**)&shaderReflection);

		if (FAILED(hr))
			BS_EXCEPT(RenderingAPIException, "Cannot reflect D3D11 high-level shader.");

		D3D11_SHADER_DESC shaderDesc;
		hr = shaderReflection->GetDesc(&shaderDesc);

		if (FAILED(hr))
			BS_EXCEPT(RenderingAPIException, "Cannot reflect D3D11 high-level shader.");

		if (inputParams != nullptr)
		{
			D3D11_SIGNATURE_PARAMETER_DESC inputParamDesc;
			for (UINT32 i = 0; i < shaderDesc.InputParameters; i++)
			{
				hr = shaderReflection->GetInputParameterDesc(i, &inputParamDesc);

				if (FAILED(hr))
					BS_EXCEPT(RenderingAPIException, "Cannot get input param desc with index: " + toString(i));

				// We don't care about system value semantics
				if (StringUtil::startsWith(String(inputParamDesc.SemanticName), "sv_"))
					continue;

				inputParams->push_back(VertexElement(inputParamDesc.Stream, inputParamDesc.Register,
					D3D11Mappings::getInputType(inputParamDesc.ComponentType), D3D11Mappings::get(inputParamDesc.SemanticName), inputParamDesc.SemanticIndex));
			}
		}

		for(UINT32 i = 0; i < shaderDesc.BoundResources; i++)
		{
			D3D11_SHADER_INPUT_BIND_DESC bindingDesc;
			hr = shaderReflection->GetResourceBindingDesc(i, &bindingDesc);

			if (FAILED(hr))
				BS_EXCEPT(RenderingAPIException, "Cannot get resource binding desc with index: " + toString(i));

			parseResource(bindingDesc, type, desc);
		}

		for(UINT32 i = 0; i < shaderDesc.ConstantBuffers; i++)
		{
			ID3D11ShaderReflectionConstantBuffer* shaderReflectionConstantBuffer;
			shaderReflectionConstantBuffer = shaderReflection->GetConstantBufferByIndex(i);
			
			parseBuffer(shaderReflectionConstantBuffer, desc);
		}

		shaderReflection->Release();
	}

	void D3D11HLSLParamParser::parseResource(D3D11_SHADER_INPUT_BIND_DESC& resourceDesc, GpuProgramType type, 
		GpuParamDesc& desc)
	{
		for(UINT32 i = 0; i < resourceDesc.BindCount; i++)
		{
			if(resourceDesc.Type == D3D_SIT_CBUFFER || resourceDesc.Type == D3D_SIT_TBUFFER)
			{
				GpuParamBlockDesc blockDesc;
				blockDesc.name = resourceDesc.Name;
				blockDesc.slot = resourceDesc.BindPoint + i;
				blockDesc.set = mapParameterToSet(type, ParamType::ConstantBuffer);
				blockDesc.blockSize = 0; // Calculated manually as we add parameters

				if(strcmp(resourceDesc.Name, "$Globals") == 0 || strcmp(resourceDesc.Name, "$Param") == 0) // Special buffers, as defined by DX11 docs
					blockDesc.isShareable = false;
				else
					blockDesc.isShareable = true;

				desc.paramBlocks.insert(std::make_pair(blockDesc.name, blockDesc));
			}
			else
			{
				GpuParamObjectDesc memberDesc;
				memberDesc.name = resourceDesc.Name;
				memberDesc.slot = resourceDesc.BindPoint + i;
				memberDesc.type = GPOT_UNKNOWN;

				switch(resourceDesc.Type)
				{
				case D3D_SIT_SAMPLER:
					memberDesc.type = GPOT_SAMPLER2D; // Actual dimension of the sampler doesn't matter
					memberDesc.set = mapParameterToSet(type, ParamType::Sampler);

					desc.samplers.insert(std::make_pair(memberDesc.name, memberDesc));
					break;
				case D3D_SIT_TEXTURE:
					switch(resourceDesc.Dimension)
					{
					case D3D_SRV_DIMENSION_TEXTURE1D:
					case D3D_SRV_DIMENSION_TEXTURE1DARRAY:
						memberDesc.type = GPOT_TEXTURE1D;
						break;
					case D3D_SRV_DIMENSION_TEXTURE2D:
					case D3D_SRV_DIMENSION_TEXTURE2DARRAY:
						memberDesc.type = GPOT_TEXTURE2D;
						break;
					case D3D_SRV_DIMENSION_TEXTURE3D:
						memberDesc.type = GPOT_TEXTURE3D;
						break;
					case D3D_SRV_DIMENSION_TEXTURECUBE:
					case D3D_SRV_DIMENSION_TEXTURECUBEARRAY:
						memberDesc.type = GPOT_TEXTURECUBE;
						break;
					case D3D_SRV_DIMENSION_TEXTURE2DMS:
					case D3D_SRV_DIMENSION_TEXTURE2DMSARRAY:
						memberDesc.type = GPOT_TEXTURE2DMS;
						break;
					default:
						LOGWRN("Skipping texture because it has unsupported dimension: " + toString(resourceDesc.Dimension));
					}

					if (memberDesc.type != GPOT_UNKNOWN)
					{
						memberDesc.set = mapParameterToSet(type, ParamType::Texture);
						desc.textures.insert(std::make_pair(memberDesc.name, memberDesc));
					}
					break;
				case D3D_SIT_STRUCTURED:
					memberDesc.type = GPOT_STRUCTURED_BUFFER;
					memberDesc.set = mapParameterToSet(type, ParamType::Texture);

					desc.buffers.insert(std::make_pair(memberDesc.name, memberDesc));
					break;
				case D3D_SIT_BYTEADDRESS:
					memberDesc.type = GPOT_BYTE_BUFFER;
					memberDesc.set = mapParameterToSet(type, ParamType::Texture);

					desc.buffers.insert(std::make_pair(memberDesc.name, memberDesc));
					break;
				case D3D11_SIT_UAV_RWTYPED:
				{
					memberDesc.set = mapParameterToSet(type, ParamType::UAV);

					switch (resourceDesc.Dimension)
					{
					case D3D_SRV_DIMENSION_TEXTURE1D:
					case D3D_SRV_DIMENSION_TEXTURE1DARRAY:
						memberDesc.type = GPOT_RWTEXTURE1D;
						desc.loadStoreTextures.insert(std::make_pair(memberDesc.name, memberDesc));
						break;
					case D3D_SRV_DIMENSION_TEXTURE2D:
					case D3D_SRV_DIMENSION_TEXTURE2DARRAY:
						memberDesc.type = GPOT_RWTEXTURE2D;
						desc.loadStoreTextures.insert(std::make_pair(memberDesc.name, memberDesc));
						break;
					case D3D_SRV_DIMENSION_TEXTURE3D:
						memberDesc.type = GPOT_RWTEXTURE3D;
						desc.loadStoreTextures.insert(std::make_pair(memberDesc.name, memberDesc));
						break;
					case D3D_SRV_DIMENSION_TEXTURE2DMS:
					case D3D_SRV_DIMENSION_TEXTURE2DMSARRAY:
						memberDesc.type = GPOT_RWTEXTURE2DMS;
						desc.loadStoreTextures.insert(std::make_pair(memberDesc.name, memberDesc));
						break;
					case D3D_SRV_DIMENSION_BUFFER:
						memberDesc.type = GPOT_RWTYPED_BUFFER;
						desc.buffers.insert(std::make_pair(memberDesc.name, memberDesc));
						break;
					default:
						LOGWRN("Skipping typed UAV because it has unsupported dimension: " + toString(resourceDesc.Dimension));
					}

					break;
				}
				case D3D11_SIT_UAV_RWSTRUCTURED:
					memberDesc.type = GPOT_RWSTRUCTURED_BUFFER;
					memberDesc.set = mapParameterToSet(type, ParamType::UAV);

					desc.buffers.insert(std::make_pair(memberDesc.name, memberDesc));
					break;
				case D3D11_SIT_UAV_RWBYTEADDRESS:
					memberDesc.type = GPOT_RWBYTE_BUFFER;
					memberDesc.set = mapParameterToSet(type, ParamType::UAV);

					desc.buffers.insert(std::make_pair(memberDesc.name, memberDesc));
					break;
				case D3D_SIT_UAV_APPEND_STRUCTURED:
					memberDesc.type = GPOT_RWAPPEND_BUFFER;
					memberDesc.set = mapParameterToSet(type, ParamType::UAV);

					desc.buffers.insert(std::make_pair(memberDesc.name, memberDesc));
					break;
				case D3D_SIT_UAV_CONSUME_STRUCTURED:
					memberDesc.type = GPOT_RWCONSUME_BUFFER;
					memberDesc.set = mapParameterToSet(type, ParamType::UAV);

					desc.buffers.insert(std::make_pair(memberDesc.name, memberDesc));
					break;
				case D3D_SIT_UAV_RWSTRUCTURED_WITH_COUNTER:
					memberDesc.type = GPOT_RWSTRUCTURED_BUFFER_WITH_COUNTER;
					memberDesc.set = mapParameterToSet(type, ParamType::UAV);

					desc.buffers.insert(std::make_pair(memberDesc.name, memberDesc));
					break;
				default:
					LOGWRN("Skipping resource because it has unsupported type: " + toString(resourceDesc.Type));
				}
			}
		}
	}

	void D3D11HLSLParamParser::parseBuffer(ID3D11ShaderReflectionConstantBuffer* bufferReflection, 
		GpuParamDesc& desc)
	{
		D3D11_SHADER_BUFFER_DESC constantBufferDesc;
		HRESULT hr = bufferReflection->GetDesc(&constantBufferDesc);
		if (FAILED(hr))
			BS_EXCEPT(RenderingAPIException, "Failed to retrieve HLSL constant buffer description.");

		if(constantBufferDesc.Type != D3D_CT_CBUFFER && constantBufferDesc.Type != D3D_CT_TBUFFER)
		{
			// Not supported (most likely a constant buffer used internally by DX)
			return;
		}

		GpuParamBlockDesc& blockDesc = desc.paramBlocks[constantBufferDesc.Name];

		for(UINT32 j = 0; j < constantBufferDesc.Variables; j++)
		{
			ID3D11ShaderReflectionVariable* varRef = bufferReflection->GetVariableByIndex(j);
			D3D11_SHADER_VARIABLE_DESC varDesc;
			hr = varRef->GetDesc(&varDesc);

			if (FAILED(hr))
				BS_EXCEPT(RenderingAPIException, "Failed to retrieve HLSL constant buffer variable description.");

			ID3D11ShaderReflectionType* varRefType = varRef->GetType();
			D3D11_SHADER_TYPE_DESC varTypeDesc;
			varRefType->GetDesc(&varTypeDesc);

			parseVariable(varTypeDesc, varDesc, desc, blockDesc);
		}

		blockDesc.blockSize = constantBufferDesc.Size / 4;
	}

	void D3D11HLSLParamParser::parseVariable(D3D11_SHADER_TYPE_DESC& varTypeDesc, D3D11_SHADER_VARIABLE_DESC& varDesc, 
		GpuParamDesc& desc, GpuParamBlockDesc& paramBlock)
	{
		GpuParamDataDesc memberDesc;
		memberDesc.name = varDesc.Name;
		memberDesc.paramBlockSlot = paramBlock.slot;
		memberDesc.paramBlockSet = paramBlock.set;
		memberDesc.arraySize = varTypeDesc.Elements == 0 ? 1 : varTypeDesc.Elements;
		memberDesc.gpuMemOffset = varDesc.StartOffset / 4;
		memberDesc.cpuMemOffset = varDesc.StartOffset / 4;
		
		// Determine individual element size in the array
		if(memberDesc.arraySize > 1)
		{
			// Find array element size (reported size is total size of array, minus unused register slots)
			int totalArraySize = (varDesc.Size / 4);

			int totalSlotsUsedByArray = (totalArraySize / 4 + 1) * 4;
			int unusedSlotsInArray = totalSlotsUsedByArray - totalArraySize;

			memberDesc.arrayElementStride = totalSlotsUsedByArray / memberDesc.arraySize;
			memberDesc.elementSize = memberDesc.arrayElementStride - unusedSlotsInArray;
		}
		else
		{
			memberDesc.elementSize = varDesc.Size / 4; // Stored in multiples of 4
			memberDesc.arrayElementStride = memberDesc.elementSize;
		}
			
		switch(varTypeDesc.Class)
		{
		case D3D_SVC_SCALAR:
			{
				switch(varTypeDesc.Type)
				{
				case D3D_SVT_BOOL:
					memberDesc.type = GPDT_BOOL;
					break;
				case D3D_SVT_INT:
				case D3D_SVT_UINT:
					memberDesc.type = GPDT_INT1;
					break;
				case D3D_SVT_FLOAT:
					memberDesc.type = GPDT_FLOAT1;
					break;
				default:
					LOGWRN("Skipping variable because it has unsupported type: " + toString(varTypeDesc.Type));
				}
			}
			break;
		case D3D_SVC_VECTOR:
			{
				switch(varTypeDesc.Type)
				{
				case D3D_SVT_INT:
					{
						switch(varTypeDesc.Columns)
						{
						case 1:
							memberDesc.type = GPDT_INT1;
							break;
						case 2:
							memberDesc.type = GPDT_INT2;
							break;
						case 3:
							memberDesc.type = GPDT_INT3;
							break;
						case 4:
							memberDesc.type = GPDT_INT4;
							break;
						}
					}
					
					break;
				case D3D_SVT_FLOAT:
					{
						switch(varTypeDesc.Columns)
						{
						case 1:
							memberDesc.type = GPDT_FLOAT1;
							break;
						case 2:
							memberDesc.type = GPDT_FLOAT2;
							break;
						case 3:
							memberDesc.type = GPDT_FLOAT3;
							break;
						case 4:
							memberDesc.type = GPDT_FLOAT4;
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
					memberDesc.type = GPDT_MATRIX_2X2;
					break;
				case 3:
					memberDesc.type = GPDT_MATRIX_2X3;
					break;
				case 4:
					memberDesc.type = GPDT_MATRIX_2X4;
					break;
				}
				break;
			case 3:
				switch(varTypeDesc.Columns)
				{
				case 2:
					memberDesc.type = GPDT_MATRIX_3X2;
					break;
				case 3:
					memberDesc.type = GPDT_MATRIX_3X3;
					break;
				case 4:
					memberDesc.type = GPDT_MATRIX_3X4;
					break;
				}
				break;
			case 4:
				switch(varTypeDesc.Columns)
				{
				case 2:
					memberDesc.type = GPDT_MATRIX_4X2;
					break;
				case 3:
					memberDesc.type = GPDT_MATRIX_4X3;
					break;
				case 4:
					memberDesc.type = GPDT_MATRIX_4X4;
					break;
				}
				break;
			}
			break;
		case D3D_SVC_STRUCT:
			memberDesc.type = GPDT_STRUCT;
			break;
		default:
			LOGWRN("Skipping variable because it has unsupported class: " + toString(varTypeDesc.Class));
		}

		desc.params.insert(std::make_pair(memberDesc.name, memberDesc));
	}

	UINT32 D3D11HLSLParamParser::mapParameterToSet(GpuProgramType progType, ParamType paramType)
	{
		UINT32 progTypeIdx = (UINT32)progType;
		UINT32 paramTypeIdx = (UINT32)paramType;
		
		return progTypeIdx * (UINT32)ParamType::Count + paramTypeIdx;
	}
}}