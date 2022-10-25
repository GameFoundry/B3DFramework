//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptShaderVariationParamValue.generated.h"
#include "BsMonoMethod.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"

namespace bs
{
	ScriptShaderVariationParamValue::ScriptShaderVariationParamValue(MonoObject* managedInstance)
		: ScriptObject(managedInstance)
	{}

	void ScriptShaderVariationParamValue::InitRuntimeData()
	{}

	MonoObject* ScriptShaderVariationParamValue::Box(const __ShaderVariationParamValueInterop& value)
	{
		return MonoUtil::Box(metaData.ScriptClass->GetInternalClassInternal(), (void*)&value);
	}

	__ShaderVariationParamValueInterop ScriptShaderVariationParamValue::Unbox(MonoObject* value)
	{
		return *(__ShaderVariationParamValueInterop*)MonoUtil::Unbox(value);
	}

	ShaderVariationParamValue ScriptShaderVariationParamValue::FromInterop(const __ShaderVariationParamValueInterop& value)
	{
		ShaderVariationParamValue output;
		String tmpName;
		tmpName = MonoUtil::MonoToString(value.Name);
		output.Name = tmpName;
		output.Value = value.Value;

		return output;
	}

	__ShaderVariationParamValueInterop ScriptShaderVariationParamValue::ToInterop(const ShaderVariationParamValue& value)
	{
		__ShaderVariationParamValueInterop output;
		MonoString* tmpName;
		tmpName = MonoUtil::StringToMono(value.Name);
		output.Name = tmpName;
		output.Value = value.Value;

		return output;
	}

} // namespace bs
