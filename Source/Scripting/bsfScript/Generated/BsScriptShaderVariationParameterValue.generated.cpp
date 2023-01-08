//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptShaderVariationParameterValue.generated.h"
#include "BsMonoMethod.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"

namespace bs
{
	ScriptShaderVariationParameterValue::ScriptShaderVariationParameterValue(MonoObject* managedInstance)
		:ScriptObject(managedInstance)
	{ }

	void ScriptShaderVariationParameterValue::InitRuntimeData()
	{ }

	MonoObject*ScriptShaderVariationParameterValue::Box(const __ShaderVariationParameterValueInterop& value)
	{
		return MonoUtil::Box(metaData.ScriptClass->GetInternalClassInternal(), (void*)&value);
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
