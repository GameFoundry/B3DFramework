//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptShaderParameter.generated.h"
#include "BsMonoMethod.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"

namespace bs
{
	ScriptShaderParameter::ScriptShaderParameter(MonoObject* managedInstance)
		:ScriptObject(managedInstance)
	{ }

	void ScriptShaderParameter::initRuntimeData()
	{ }

	MonoObject*ScriptShaderParameter::box(const __ShaderParameterInterop& value)
	{
		return MonoUtil::box(metaData.scriptClass->_getInternalClass(), (void*)&value);
	}

	__ShaderParameterInterop ScriptShaderParameter::unbox(MonoObject* value)
	{
		return *(__ShaderParameterInterop*)MonoUtil::unbox(value);
	}

	ShaderParameter ScriptShaderParameter::fromInterop(const __ShaderParameterInterop& value)
	{
		ShaderParameter output;
		String tmpname;
		tmpname = MonoUtil::monoToString(value.name);
		output.name = tmpname;
		String tmpidentifier;
		tmpidentifier = MonoUtil::monoToString(value.identifier);
		output.identifier = tmpidentifier;
		output.type = value.type;
		output.flags = value.flags;

		return output;
	}

	__ShaderParameterInterop ScriptShaderParameter::toInterop(const ShaderParameter& value)
	{
		__ShaderParameterInterop output;
		MonoString* tmpname;
		tmpname = MonoUtil::stringToMono(value.name);
		output.name = tmpname;
		MonoString* tmpidentifier;
		tmpidentifier = MonoUtil::stringToMono(value.identifier);
		output.identifier = tmpidentifier;
		output.type = value.type;
		output.flags = value.flags;

		return output;
	}

}
