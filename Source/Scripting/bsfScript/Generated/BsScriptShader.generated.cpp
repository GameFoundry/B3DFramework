//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptShader.generated.h"
#include "BsMonoMethod.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"
#include "../../../Foundation/bsfCore/Material/BsShader.h"
#include "BsScriptShaderVariationParameterInformation.generated.h"
#include "BsScriptShaderParameter.generated.h"
#include "../Extensions/BsShaderEx.h"

namespace bs
{
	ScriptShader::ScriptShader(MonoObject* managedInstance, const TResourceHandle<Shader>& value)
		:TScriptResource(managedInstance, value)
	{
	}

	void ScriptShader::InitRuntimeData()
	{
		metaData.ScriptClass->AddInternalCall("Internal_GetRef", (void*)&ScriptShader::InternalGetRef);
		metaData.ScriptClass->AddInternalCall("Internal_GetVariationParams", (void*)&ScriptShader::InternalGetVariationParams);
		metaData.ScriptClass->AddInternalCall("Internal_GetParameters", (void*)&ScriptShader::InternalGetParameters);

	}

	 MonoObject*ScriptShader::CreateInstance()
	{
		bool dummy = false;
		void* ctorParams[1] = { &dummy };

		return metaData.ScriptClass->CreateInstance("bool", ctorParams);
	}
	MonoObject* ScriptShader::InternalGetRef(ScriptShader* thisPtr)
	{
		return thisPtr->GetRRef();
	}

	MonoArray* ScriptShader::InternalGetVariationParams(ScriptShader* thisPtr)
	{
		Vector<ShaderVariationParameterInformation> vec__output;
		vec__output = thisPtr->GetHandle()->GetVariationParams();

		MonoArray* __output;
		int arraySize__output = (int)vec__output.size();
		ScriptArray array__output = ScriptArray::Create<ScriptShaderVariationParameterInformation>(arraySize__output);
		for(int i = 0; i < arraySize__output; i++)
		{
			array__output.Set(i, ScriptShaderVariationParameterInformation::ToInterop(vec__output[i]));
		}
		__output = array__output.GetInternal();

		return __output;
	}

	MonoArray* ScriptShader::InternalGetParameters(ScriptShader* thisPtr)
	{
		Vector<ShaderParameter> vec__output;
		vec__output = ShaderEx::GetParameters(thisPtr->GetHandle());

		MonoArray* __output;
		int arraySize__output = (int)vec__output.size();
		ScriptArray array__output = ScriptArray::Create<ScriptShaderParameter>(arraySize__output);
		for(int i = 0; i < arraySize__output; i++)
		{
			array__output.Set(i, ScriptShaderParameter::ToInterop(vec__output[i]));
		}
		__output = array__output.GetInternal();

		return __output;
	}
}
