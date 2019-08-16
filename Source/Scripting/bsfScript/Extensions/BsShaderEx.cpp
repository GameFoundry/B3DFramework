//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Extensions/BsShaderEx.h"
#include "CoreThread/BsCoreThread.h"

namespace bs
{
	Vector<ShaderParameter> ShaderEx::getParameters(const HShader& thisPtr)
	{
		if (!thisPtr.isLoaded())
			return Vector<ShaderParameter>();

		const Map<String, SHADER_DATA_PARAM_DESC>& dataParams = thisPtr->getDataParams();
		const Map<String, SHADER_OBJECT_PARAM_DESC>& textureParams = thisPtr->getTextureParams();
		const Map<String, SHADER_OBJECT_PARAM_DESC>& samplerParams = thisPtr->getSamplerParams();
		const Vector<SHADER_PARAM_ATTRIBUTE> attributes = thisPtr->getParamAttributes();

		Vector<ShaderParameter> paramInfos;
		auto parseParam = [&paramInfos, &attributes](const String& identifier, ShaderParameterType type, bool isInternal,
			UINT32 attribIdx)
		{
			ShaderParameter output;
			output.identifier = identifier;
			output.type = type;
			output.flags = isInternal ? ShaderParameterFlag::Internal : ShaderParameterFlag::None;

			while (attribIdx != (UINT32)-1)
			{
				const SHADER_PARAM_ATTRIBUTE& attrib = attributes[attribIdx];
				if (attrib.type == ShaderParamAttributeType::Name)
					output.name = attrib.value;
				else if (attrib.type == ShaderParamAttributeType::HideInInspector)
					output.flags |= ShaderParameterFlag::HideInInspector;
				else if (attrib.type == ShaderParamAttributeType::HDR)
					output.flags |= ShaderParameterFlag::HDR;

				attribIdx = attrib.nextParamIdx;
			}

			if(output.name.empty())
				output.name = output.identifier;

			paramInfos.push_back(output);
		};

		// TODO - Ignoring int, bool, struct and non-square matrices
		// TODO - Ignoring buffers and load/store textures
		for (auto& param : dataParams)
		{
			ShaderParameterType type;
			bool isValidType = false;
			bool isInternal = !param.second.rendererSemantic.empty();
			switch (param.second.type)
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

			if (isValidType)
				parseParam(param.first, type, isInternal, param.second.attribIdx);
		}

		for (auto& param : textureParams)
		{
			ShaderParameterType type;
			bool isValidType = false;
			bool isInternal = !param.second.rendererSemantic.empty();
			switch (param.second.type)
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

			if (isValidType)
				parseParam(param.first, type, isInternal, param.second.attribIdx);
		}

		for (auto& param : samplerParams)
		{
			ShaderParameterType type = ShaderParameterType::Sampler;
			bool isInternal = !param.second.rendererSemantic.empty();

			parseParam(param.first, type, isInternal, param.second.attribIdx);
		}

		return paramInfos;
	}
}
