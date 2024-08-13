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
	MonoObject* ScriptShader::InternalGetRef(ScriptShader* self)
	{
		return self->GetRRef();
	}

	MonoArray* ScriptShader::InternalGetVariationParams(ScriptShader* self)
	{
		Vector<ShaderVariationParameterInformation> nativeArray__output;
		nativeArray__output = self->GetHandle()->GetVariationParams();

		MonoArray* __output;
		int elementCount__output = (int)nativeArray__output.size();
		ScriptArray scriptArray__output = ScriptArray::Create<ScriptShaderVariationParameterInformation>(elementCount__output);
		for(int elementIndex = 0; elementIndex < elementCount__output; elementIndex++)
		{
			scriptArray__output.Set(elementIndex, ScriptShaderVariationParameterInformation::ToInterop(nativeArray__output[elementIndex]));
		}
		__output = scriptArray__output.GetInternal();

		return __output;
	}

	MonoArray* ScriptShader::InternalGetParameters(ScriptShader* self)
	{
		Vector<ShaderParameter> nativeArray__output;
		nativeArray__output = ShaderEx::GetParameters(self->GetHandle());

		MonoArray* __output;
		int elementCount__output = (int)nativeArray__output.size();
		ScriptArray scriptArray__output = ScriptArray::Create<ScriptShaderParameter>(elementCount__output);
		for(int elementIndex = 0; elementIndex < elementCount__output; elementIndex++)
		{
			scriptArray__output.Set(elementIndex, ScriptShaderParameter::ToInterop(nativeArray__output[elementIndex]));
		}
		__output = scriptArray__output.GetInternal();

		return __output;
	}
}
