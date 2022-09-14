//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptShaderVariationParamValue.generated.h"
#include "BsMonoMethod.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"

namespace bs
{
	ScriptShaderVariationParamValue::ScriptShaderVariationParamValue(MonoObject* managedInstance)
		:ScriptObject(managedInstance)
	{ }

	void ScriptShaderVariationParamValue::initRuntimeData()
	{ }

	MonoObject*ScriptShaderVariationParamValue::box(const __ShaderVariationParamValueInterop& value)
	{
		return MonoUtil::box(metaData.scriptClass->GetInternalClassInternal(), (void*)&value);
	}

	__ShaderVariationParamValueInterop ScriptShaderVariationParamValue::unbox(MonoObject* value)
	{
		return *(__ShaderVariationParamValueInterop*)MonoUtil::unbox(value);
	}

	ShaderVariationParamValue ScriptShaderVariationParamValue::fromInterop(const __ShaderVariationParamValueInterop& value)
	{
		ShaderVariationParamValue output;
		String tmpname;
		tmpname = MonoUtil::monoToString(value.name);
		output.name = tmpname;
		output.value = value.value;

		return output;
	}

	__ShaderVariationParamValueInterop ScriptShaderVariationParamValue::toInterop(const ShaderVariationParamValue& value)
	{
		__ShaderVariationParamValueInterop output;
		MonoString* tmpname;
		tmpname = MonoUtil::stringToMono(value.name);
		output.name = tmpname;
		output.value = value.value;

		return output;
	}

}
