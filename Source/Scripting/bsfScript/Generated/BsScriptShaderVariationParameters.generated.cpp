//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptShaderVariationParameters.generated.h"
#include "BsMonoMethod.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"

namespace bs
{
	ScriptShaderVariationParameters::ScriptShaderVariationParameters(MonoObject* managedInstance, const SPtr<ShaderVariationParameters>& value)
		:TScriptReflectable(managedInstance, value)
	{
	}

	void ScriptShaderVariationParameters::InitRuntimeData()
	{
		metaData.ScriptClass->AddInternalCall("Internal_ShaderVariationParameters", (void*)&ScriptShaderVariationParameters::InternalShaderVariationParameters);
		metaData.ScriptClass->AddInternalCall("Internal_GetInt", (void*)&ScriptShaderVariationParameters::InternalGetInt);
		metaData.ScriptClass->AddInternalCall("Internal_GetUInt", (void*)&ScriptShaderVariationParameters::InternalGetUInt);
		metaData.ScriptClass->AddInternalCall("Internal_GetFloat", (void*)&ScriptShaderVariationParameters::InternalGetFloat);
		metaData.ScriptClass->AddInternalCall("Internal_GetBool", (void*)&ScriptShaderVariationParameters::InternalGetBool);
		metaData.ScriptClass->AddInternalCall("Internal_SetInt", (void*)&ScriptShaderVariationParameters::InternalSetInt);
		metaData.ScriptClass->AddInternalCall("Internal_SetUInt", (void*)&ScriptShaderVariationParameters::InternalSetUInt);
		metaData.ScriptClass->AddInternalCall("Internal_SetFloat", (void*)&ScriptShaderVariationParameters::InternalSetFloat);
		metaData.ScriptClass->AddInternalCall("Internal_SetBool", (void*)&ScriptShaderVariationParameters::InternalSetBool);
		metaData.ScriptClass->AddInternalCall("Internal_RemoveParam", (void*)&ScriptShaderVariationParameters::InternalRemoveParam);
		metaData.ScriptClass->AddInternalCall("Internal_HasParam", (void*)&ScriptShaderVariationParameters::InternalHasParam);
		metaData.ScriptClass->AddInternalCall("Internal_ClearParams", (void*)&ScriptShaderVariationParameters::InternalClearParams);
		metaData.ScriptClass->AddInternalCall("Internal_GetParamNames", (void*)&ScriptShaderVariationParameters::InternalGetParamNames);

	}

	MonoObject* ScriptShaderVariationParameters::Create(const SPtr<ShaderVariationParameters>& value)
	{
		if(value == nullptr) return nullptr; 

		bool dummy = false;
		void* ctorParams[1] = { &dummy };

		MonoObject* managedInstance = metaData.ScriptClass->CreateInstance("bool", ctorParams);
		new (B3DAllocate<ScriptShaderVariationParameters>()) ScriptShaderVariationParameters(managedInstance, value);
		return managedInstance;
	}
	void ScriptShaderVariationParameters::InternalShaderVariationParameters(MonoObject* managedInstance)
	{
		SPtr<ShaderVariationParameters> nativeObject = B3DMakeShared<ShaderVariationParameters>();
		new (B3DAllocate<ScriptShaderVariationParameters>())ScriptShaderVariationParameters(managedInstance, nativeObject);
	}

	int32_t ScriptShaderVariationParameters::InternalGetInt(ScriptShaderVariationParameters* self, MonoString* name)
	{
		int32_t tmp__output;
		String tmpname;
		tmpname = MonoUtil::MonoToString(name);
		tmp__output = self->GetInternal()->GetInt(tmpname);

		int32_t __output;
		__output = tmp__output;

		return __output;
	}

	uint32_t ScriptShaderVariationParameters::InternalGetUInt(ScriptShaderVariationParameters* self, MonoString* name)
	{
		uint32_t tmp__output;
		String tmpname;
		tmpname = MonoUtil::MonoToString(name);
		tmp__output = self->GetInternal()->GetUInt(tmpname);

		uint32_t __output;
		__output = tmp__output;

		return __output;
	}

	float ScriptShaderVariationParameters::InternalGetFloat(ScriptShaderVariationParameters* self, MonoString* name)
	{
		float tmp__output;
		String tmpname;
		tmpname = MonoUtil::MonoToString(name);
		tmp__output = self->GetInternal()->GetFloat(tmpname);

		float __output;
		__output = tmp__output;

		return __output;
	}

	bool ScriptShaderVariationParameters::InternalGetBool(ScriptShaderVariationParameters* self, MonoString* name)
	{
		bool tmp__output;
		String tmpname;
		tmpname = MonoUtil::MonoToString(name);
		tmp__output = self->GetInternal()->GetBool(tmpname);

		bool __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptShaderVariationParameters::InternalSetInt(ScriptShaderVariationParameters* self, MonoString* name, int32_t value)
	{
		String tmpname;
		tmpname = MonoUtil::MonoToString(name);
		self->GetInternal()->SetInt(tmpname, value);
	}

	void ScriptShaderVariationParameters::InternalSetUInt(ScriptShaderVariationParameters* self, MonoString* name, uint32_t value)
	{
		String tmpname;
		tmpname = MonoUtil::MonoToString(name);
		self->GetInternal()->SetUInt(tmpname, value);
	}

	void ScriptShaderVariationParameters::InternalSetFloat(ScriptShaderVariationParameters* self, MonoString* name, float value)
	{
		String tmpname;
		tmpname = MonoUtil::MonoToString(name);
		self->GetInternal()->SetFloat(tmpname, value);
	}

	void ScriptShaderVariationParameters::InternalSetBool(ScriptShaderVariationParameters* self, MonoString* name, bool value)
	{
		String tmpname;
		tmpname = MonoUtil::MonoToString(name);
		self->GetInternal()->SetBool(tmpname, value);
	}

	void ScriptShaderVariationParameters::InternalRemoveParam(ScriptShaderVariationParameters* self, MonoString* paramName)
	{
		String tmpparamName;
		tmpparamName = MonoUtil::MonoToString(paramName);
		self->GetInternal()->RemoveParam(tmpparamName);
	}

	bool ScriptShaderVariationParameters::InternalHasParam(ScriptShaderVariationParameters* self, MonoString* paramName)
	{
		bool tmp__output;
		String tmpparamName;
		tmpparamName = MonoUtil::MonoToString(paramName);
		tmp__output = self->GetInternal()->HasParam(tmpparamName);

		bool __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptShaderVariationParameters::InternalClearParams(ScriptShaderVariationParameters* self)
	{
		self->GetInternal()->ClearParams();
	}

	MonoArray* ScriptShaderVariationParameters::InternalGetParamNames(ScriptShaderVariationParameters* self)
	{
		Vector<String> nativeArray__output;
		nativeArray__output = self->GetInternal()->GetParamNames();

		MonoArray* __output;
		int elementCount__output = (int)nativeArray__output.size();
		ScriptArray scriptArray__output = ScriptArray::Create<String>(elementCount__output);
		for(int elementIndex = 0; elementIndex < elementCount__output; elementIndex++)
		{
			scriptArray__output.Set(elementIndex, nativeArray__output[elementIndex]);
		}
		__output = scriptArray__output.GetInternal();

		return __output;
	}
}
