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

	void ScriptShaderVariationParamValue::InitRuntimeData()
	{ }

	MonoObject*ScriptShaderVariationParamValue::box(const __ShaderVariationParamValueInterop& value)
	{
		return MonoUtil::Box(metaData.scriptClass->_getInternalClass(), (void*)&value);
	}

	__ShaderVariationParamValueInterop ScriptShaderVariationParamValue::Unbox(MonoObject* value)
	{
		return *(__ShaderVariationParamValueInterop*)MonoUtil::unbox(value);
	}

	ShaderVariationParamValue ScriptShaderVariationParamValue::FromInterop(const __ShaderVariationParamValueInterop& value)
	{
		ShaderVariationParamValue output;
		String tmpname;
		tmpname = MonoUtil::monoToString(value.name);
		output.name = tmpname;
		output.value = value.value;

		return output;
	}

	__ShaderVariationParamValueInterop ScriptShaderVariationParamValue::ToInterop(const ShaderVariationParamValue& value)
	{
		__ShaderVariationParamValueInterop output;
		MonoString* tmpname;
		tmpname = MonoUtil::stringToMono(value.name);
		output.name = tmpname;
		output.value = value.value;

		return output;
	}

}
