//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsScriptShader.h"
#include "BsScriptResourceManager.h"
#include "BsScriptMeta.h"
#include "BsMonoField.h"
#include "BsMonoClass.h"
#include "BsMonoArray.h"
#include "BsMonoManager.h"

namespace bs
{
	/**	Shader parameter types that can be handled using managed code. */
	enum class ShaderParameterType // Note: This must match C# ShaderParameterType enum
	{
		Float, Vector2, Vector3, Vector4, Color,
		Matrix3, Matrix4, Texture2D,
		Texture3D, TextureCube, Sampler
	};

	ScriptShader::ScriptShader(MonoObject* instance, const HShader& shader)
		:TScriptResource(instance, shader)
	{

	}

	void ScriptShader::initRuntimeData()
	{
		metaData.scriptClass->addInternalCall("Internal_GetShaderParameters", &ScriptShader::internal_GetShaderParameters);
	}

	void ScriptShader::internal_GetShaderParameters(ScriptShader* nativeInstance, MonoArray** outNames, 
		MonoArray** outTypes, MonoArray** outVisibility)
	{
		HShader shader = nativeInstance->getHandle();
		if (!shader.isLoaded())
		{
			ScriptArray names = ScriptArray::create<String>(0);
			ScriptArray types = ScriptArray::create<UINT32>(0);
			ScriptArray visibility = ScriptArray::create<bool>(0);

			*outNames = names.getInternal();
			*outTypes = types.getInternal();
			*outVisibility = visibility.getInternal();
			return;
		}

		const Map<String, SHADER_DATA_PARAM_DESC>& dataParams = shader->getDataParams();
		const Map<String, SHADER_OBJECT_PARAM_DESC>& textureParams = shader->getTextureParams();
		const Map<String, SHADER_OBJECT_PARAM_DESC>& samplerParams = shader->getSamplerParams();

		struct ParamInfo
		{
			String name;
			ShaderParameterType type;
			bool internal;
		};

		Vector<ParamInfo> paramInfos;

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


		UINT32 totalNumParams = (UINT32)paramInfos.size();

		ScriptArray names = ScriptArray::create<String>(totalNumParams);
		ScriptArray types = ScriptArray::create<UINT32>(totalNumParams);
		ScriptArray visibility = ScriptArray::create<bool>(totalNumParams);

		UINT32 idx = 0;
		for (auto& param : paramInfos)
		{
			names.set(idx, param.name);
			types.set(idx, param.type);
			visibility.set(idx, param.internal);

			idx++;
		}

		*outNames = names.getInternal();
		*outTypes = types.getInternal();
		*outVisibility = visibility.getInternal();
	}

	MonoObject* ScriptShader::createInstance()
	{
		return metaData.scriptClass->createInstance();
	}
}