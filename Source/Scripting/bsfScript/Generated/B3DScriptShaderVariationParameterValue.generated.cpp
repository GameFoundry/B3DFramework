//********************************* B3D Framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DScriptShaderVariationParameterValue.generated.h"
#include "B3DMonoMethod.h"
#include "B3DMonoClass.h"
#include "B3DMonoUtil.h"

namespace b3d
{
	ScriptShaderVariationParameterValue::ScriptShaderVariationParameterValue()
	{ }

	MonoObject* ScriptShaderVariationParameterValue::Box(const __ShaderVariationParameterValueInterop& value)
	{
		return MonoUtil::Box(sInteropMetaData.ScriptClass->GetInternalClass(), (void*)&value);
	}

	__ShaderVariationParameterValueInterop ScriptShaderVariationParameterValue::Unbox(MonoObject* value)
	{
		return *(__ShaderVariationParameterValueInterop*)MonoUtil::Unbox(value);
	}

	ShaderVariationParameterValue ScriptShaderVariationParameterValue::FromInterop(const __ShaderVariationParameterValueInterop& value)
	{
		ShaderVariationParameterValue output;
		String tmpName;
		tmpName = MonoUtil::MonoToString(value.Name);
		output.Name = tmpName;
		output.Value = value.Value;

		return output;
	}

	__ShaderVariationParameterValueInterop ScriptShaderVariationParameterValue::ToInterop(const ShaderVariationParameterValue& value)
	{
		__ShaderVariationParameterValueInterop output;
		MonoString* tmpName;
		tmpName = MonoUtil::StringToMono(value.Name);
		output.Name = tmpName;
		output.Value = value.Value;

		return output;
	}

}
