//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Extensions/BsShaderEx.h"
#include "CoreThread/BsCoreThread.h"

namespace bs
{
	Vector<ShaderParameter> ShaderEx::GetParameters(const HShader& thisPtr)
	{
		if(!thisPtr.IsLoaded())
			return Vector<ShaderParameter>();

		const Map<String, SHADER_DATA_PARAM_DESC>& dataParams = thisPtr->GetDataParams();
		const Map<String, SHADER_OBJECT_PARAM_DESC>& textureParams = thisPtr->GetTextureParams();
		const Map<String, SHADER_OBJECT_PARAM_DESC>& samplerParams = thisPtr->GetSamplerParams();
		const Vector<SHADER_PARAM_ATTRIBUTE> attributes = thisPtr->GetParamAttributes();

		Vector<ShaderParameter> paramInfos;
		auto parseParam = [&paramInfos, &attributes](const String& identifier, ShaderParameterType type, bool isInternal, u32 attribIdx)
		{
			ShaderParameter output;
			output.Identifier = identifier;
			output.Type = type;
			output.Flags = isInternal ? ShaderParameterFlag::Internal : ShaderParameterFlag::None;

			while(attribIdx != (u32)-1)
			{
				const SHADER_PARAM_ATTRIBUTE& attrib = attributes[attribIdx];
				if(attrib.Type == ShaderParamAttributeType::Name)
					output.Name = attrib.Value;
				else if(attrib.Type == ShaderParamAttributeType::HideInInspector)
					output.Flags |= ShaderParameterFlag::HideInInspector;
				else if(attrib.Type == ShaderParamAttributeType::HDR)
					output.Flags |= ShaderParameterFlag::HDR;

				attribIdx = attrib.NextParamIdx;
			}

			if(output.Name.empty())
				output.Name = output.Identifier;

			paramInfos.push_back(output);
		};

		// TODO - Ignoring int, bool, struct and non-square matrices
		// TODO - Ignoring buffers and load/store textures
		for(auto& param : dataParams)
		{
			ShaderParameterType type;
			bool isValidType = false;
			bool isInternal = !param.second.RendererSemantic.Empty();
			switch(param.second.Type)
			{
			case GPDT_FLOAT1:
				type = ShaderParameterType::Float;
				isValidType = true;
				break;
			case GPDT_FLOAT2:
				type = ShaderParameterType::Vector2;
				isValidType = true;
				break;
			case GPDT_FLOAT3:
				type = ShaderParameterType::Vector3;
				isValidType = true;
				break;
			case GPDT_FLOAT4:
				type = ShaderParameterType::Vector4;
				isValidType = true;
				break;
			case GPDT_MATRIX_3X3:
				type = ShaderParameterType::Matrix3;
				isValidType = true;
				break;
			case GPDT_MATRIX_4X4:
				type = ShaderParameterType::Matrix4;
				isValidType = true;
				break;
			case GPDT_COLOR:
				type = ShaderParameterType::Color;
				isValidType = true;
				break;
			default:
				break;
			}

			if(isValidType)
				parseParam(param.first, type, isInternal, param.second.AttribIdx);
		}

		for(auto& param : textureParams)
		{
			ShaderParameterType type;
			bool isValidType = false;
			bool isInternal = !param.second.RendererSemantic.Empty();
			switch(param.second.Type)
			{
			case GPOT_TEXTURE2D:
			case GPOT_TEXTURE2DMS:
				type = ShaderParameterType::Texture2D;
				isValidType = true;
				break;
			case GPOT_TEXTURE3D:
				type = ShaderParameterType::Texture3D;
				isValidType = true;
				break;
			case GPOT_TEXTURECUBE:
				type = ShaderParameterType::TextureCube;
				isValidType = true;
				break;
			default:
				break;
			}

			if(isValidType)
				parseParam(param.first, type, isInternal, param.second.AttribIdx);
		}

		for(auto& param : samplerParams)
		{
			ShaderParameterType type = ShaderParameterType::Sampler;
			bool isInternal = !param.second.RendererSemantic.Empty();

			parseParam(param.first, type, isInternal, param.second.AttribIdx);
		}

		return paramInfos;
	}
} // namespace bs
