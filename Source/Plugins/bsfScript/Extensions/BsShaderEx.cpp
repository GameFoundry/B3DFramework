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

		Vector<ShaderParameter> paramInfos;

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
				paramInfos.push_back({ param.first, type, isInternal });
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
				paramInfos.push_back({ param.first, type, isInternal });
		}

		for (auto& param : samplerParams)
		{
			ShaderParameterType type = ShaderParameterType::Sampler;
			bool isInternal = !param.second.rendererSemantic.empty();
			paramInfos.push_back({ param.first, type, isInternal });
		}

		return paramInfos;
	}
}
