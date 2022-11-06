//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
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

	void ScriptShaderParameter::InitRuntimeData()
	{ }

	MonoObject*ScriptShaderParameter::Box(const __ShaderParameterInterop& value)
	{
		return MonoUtil::Box(metaData.ScriptClass->GetInternalClassInternal(), (void*)&value);
	}

	__ShaderParameterInterop ScriptShaderParameter::Unbox(MonoObject* value)
	{
		return *(__ShaderParameterInterop*)MonoUtil::Unbox(value);
	}

	ShaderParameter ScriptShaderParameter::FromInterop(const __ShaderParameterInterop& value)
	{
		ShaderParameter output;
		String tmpName;
		tmpName = MonoUtil::MonoToString(value.Name);
		output.Name = tmpName;
		String tmpIdentifier;
		tmpIdentifier = MonoUtil::MonoToString(value.Identifier);
		output.Identifier = tmpIdentifier;
		output.Type = value.Type;
		output.Flags = value.Flags;

		return output;
	}

	__ShaderParameterInterop ScriptShaderParameter::ToInterop(const ShaderParameter& value)
	{
		__ShaderParameterInterop output;
		MonoString* tmpName;
		tmpName = MonoUtil::StringToMono(value.Name);
		output.Name = tmpName;
		MonoString* tmpIdentifier;
		tmpIdentifier = MonoUtil::StringToMono(value.Identifier);
		output.Identifier = tmpIdentifier;
		output.Type = value.Type;
		output.Flags = value.Flags;

		return output;
	}

}
