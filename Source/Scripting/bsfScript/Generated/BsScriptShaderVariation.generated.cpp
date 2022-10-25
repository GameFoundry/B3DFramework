//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptShaderVariation.generated.h"
#include "BsMonoMethod.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"

namespace bs
{
	ScriptShaderVariation::ScriptShaderVariation(MonoObject* managedInstance, const SPtr<ShaderVariation>& value)
		: TScriptReflectable(managedInstance, value)
	{
	}

	void ScriptShaderVariation::InitRuntimeData()
	{
		metaData.ScriptClass->AddInternalCall("Internal_ShaderVariation", (void*)&ScriptShaderVariation::InternalShaderVariation);
		metaData.ScriptClass->AddInternalCall("Internal_GetInt", (void*)&ScriptShaderVariation::InternalGetInt);
		metaData.ScriptClass->AddInternalCall("Internal_GetUInt", (void*)&ScriptShaderVariation::InternalGetUInt);
		metaData.ScriptClass->AddInternalCall("Internal_GetFloat", (void*)&ScriptShaderVariation::InternalGetFloat);
		metaData.ScriptClass->AddInternalCall("Internal_GetBool", (void*)&ScriptShaderVariation::InternalGetBool);
		metaData.ScriptClass->AddInternalCall("Internal_SetInt", (void*)&ScriptShaderVariation::InternalSetInt);
		metaData.ScriptClass->AddInternalCall("Internal_SetUInt", (void*)&ScriptShaderVariation::InternalSetUInt);
		metaData.ScriptClass->AddInternalCall("Internal_SetFloat", (void*)&ScriptShaderVariation::InternalSetFloat);
		metaData.ScriptClass->AddInternalCall("Internal_SetBool", (void*)&ScriptShaderVariation::InternalSetBool);
		metaData.ScriptClass->AddInternalCall("Internal_RemoveParam", (void*)&ScriptShaderVariation::InternalRemoveParam);
		metaData.ScriptClass->AddInternalCall("Internal_HasParam", (void*)&ScriptShaderVariation::InternalHasParam);
		metaData.ScriptClass->AddInternalCall("Internal_ClearParams", (void*)&ScriptShaderVariation::InternalClearParams);
		metaData.ScriptClass->AddInternalCall("Internal_GetParamNames", (void*)&ScriptShaderVariation::InternalGetParamNames);
	}

	MonoObject* ScriptShaderVariation::Create(const SPtr<ShaderVariation>& value)
	{
		if(value == nullptr) return nullptr;

		bool dummy = false;
		void* ctorParams[1] = { &dummy };

		MonoObject* managedInstance = metaData.ScriptClass->CreateInstance("bool", ctorParams);
		new(bs_alloc<ScriptShaderVariation>()) ScriptShaderVariation(managedInstance, value);
		return managedInstance;
	}

	void ScriptShaderVariation::InternalShaderVariation(MonoObject* managedInstance)
	{
		SPtr<ShaderVariation> instance = bs_shared_ptr_new<ShaderVariation>();
		new(bs_alloc<ScriptShaderVariation>()) ScriptShaderVariation(managedInstance, instance);
	}

	int32_t ScriptShaderVariation::InternalGetInt(ScriptShaderVariation* thisPtr, MonoString* name)
	{
		int32_t tmp__output;
		String tmpname;
		tmpname = MonoUtil::MonoToString(name);
		tmp__output = thisPtr->GetInternal()->GetInt(tmpname);

		int32_t __output;
		__output = tmp__output;

		return __output;
	}

	uint32_t ScriptShaderVariation::InternalGetUInt(ScriptShaderVariation* thisPtr, MonoString* name)
	{
		uint32_t tmp__output;
		String tmpname;
		tmpname = MonoUtil::MonoToString(name);
		tmp__output = thisPtr->GetInternal()->GetUInt(tmpname);

		uint32_t __output;
		__output = tmp__output;

		return __output;
	}

	float ScriptShaderVariation::InternalGetFloat(ScriptShaderVariation* thisPtr, MonoString* name)
	{
		float tmp__output;
		String tmpname;
		tmpname = MonoUtil::MonoToString(name);
		tmp__output = thisPtr->GetInternal()->GetFloat(tmpname);

		float __output;
		__output = tmp__output;

		return __output;
	}

	bool ScriptShaderVariation::InternalGetBool(ScriptShaderVariation* thisPtr, MonoString* name)
	{
		bool tmp__output;
		String tmpname;
		tmpname = MonoUtil::MonoToString(name);
		tmp__output = thisPtr->GetInternal()->GetBool(tmpname);

		bool __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptShaderVariation::InternalSetInt(ScriptShaderVariation* thisPtr, MonoString* name, int32_t value)
	{
		String tmpname;
		tmpname = MonoUtil::MonoToString(name);
		thisPtr->GetInternal()->SetInt(tmpname, value);
	}

	void ScriptShaderVariation::InternalSetUInt(ScriptShaderVariation* thisPtr, MonoString* name, uint32_t value)
	{
		String tmpname;
		tmpname = MonoUtil::MonoToString(name);
		thisPtr->GetInternal()->SetUInt(tmpname, value);
	}

	void ScriptShaderVariation::InternalSetFloat(ScriptShaderVariation* thisPtr, MonoString* name, float value)
	{
		String tmpname;
		tmpname = MonoUtil::MonoToString(name);
		thisPtr->GetInternal()->SetFloat(tmpname, value);
	}

	void ScriptShaderVariation::InternalSetBool(ScriptShaderVariation* thisPtr, MonoString* name, bool value)
	{
		String tmpname;
		tmpname = MonoUtil::MonoToString(name);
		thisPtr->GetInternal()->SetBool(tmpname, value);
	}

	void ScriptShaderVariation::InternalRemoveParam(ScriptShaderVariation* thisPtr, MonoString* paramName)
	{
		String tmpparamName;
		tmpparamName = MonoUtil::MonoToString(paramName);
		thisPtr->GetInternal()->RemoveParam(tmpparamName);
	}

	bool ScriptShaderVariation::InternalHasParam(ScriptShaderVariation* thisPtr, MonoString* paramName)
	{
		bool tmp__output;
		String tmpparamName;
		tmpparamName = MonoUtil::MonoToString(paramName);
		tmp__output = thisPtr->GetInternal()->HasParam(tmpparamName);

		bool __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptShaderVariation::InternalClearParams(ScriptShaderVariation* thisPtr)
	{
		thisPtr->GetInternal()->ClearParams();
	}

	MonoArray* ScriptShaderVariation::InternalGetParamNames(ScriptShaderVariation* thisPtr)
	{
		Vector<String> vec__output;
		vec__output = thisPtr->GetInternal()->GetParamNames();

		MonoArray* __output;
		int arraySize__output = (int)vec__output.size();
		ScriptArray array__output = ScriptArray::Create<String>(arraySize__output);
		for(int i = 0; i < arraySize__output; i++)
		{
			array__output.Set(i, vec__output[i]);
		}
		__output = array__output.GetInternal();

		return __output;
	}
} // namespace bs
