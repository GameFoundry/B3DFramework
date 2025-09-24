//********************************* B3D Framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptShaderVariationParameters.generated.h"
#include "BsMonoMethod.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"

namespace b3d
{
	ScriptShaderVariationParameters::ScriptShaderVariationParameters(const SPtr<ShaderVariationParameters>& nativeObject)
		:TScriptReflectableWrapper(nativeObject)
	{
		RegisterEvents();
	}

	ScriptShaderVariationParameters::~ScriptShaderVariationParameters()
	{
		UnregisterEvents();
	}

	void ScriptShaderVariationParameters::SetupScriptBindings()
	{
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_ShaderVariationParameters", (void*)&ScriptShaderVariationParameters::InternalShaderVariationParameters);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetInt", (void*)&ScriptShaderVariationParameters::InternalGetInt);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetUInt", (void*)&ScriptShaderVariationParameters::InternalGetUInt);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetFloat", (void*)&ScriptShaderVariationParameters::InternalGetFloat);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetBool", (void*)&ScriptShaderVariationParameters::InternalGetBool);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetInt", (void*)&ScriptShaderVariationParameters::InternalSetInt);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetUInt", (void*)&ScriptShaderVariationParameters::InternalSetUInt);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetFloat", (void*)&ScriptShaderVariationParameters::InternalSetFloat);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetBool", (void*)&ScriptShaderVariationParameters::InternalSetBool);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_RemoveParam", (void*)&ScriptShaderVariationParameters::InternalRemoveParam);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_HasParam", (void*)&ScriptShaderVariationParameters::InternalHasParam);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_ClearParams", (void*)&ScriptShaderVariationParameters::InternalClearParams);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetParamNames", (void*)&ScriptShaderVariationParameters::InternalGetParamNames);

	}

	MonoObject* ScriptShaderVariationParameters::CreateScriptObject(bool construct)
	{
		bool dummy = false;
		void* ctorParams[1] = { &dummy };

		if(construct)
			return sInteropMetaData.ScriptClass->CreateInstance("bool", ctorParams);

		return sInteropMetaData.ScriptClass->CreateInstance(false);
	}
	void ScriptShaderVariationParameters::InternalShaderVariationParameters(MonoObject* scriptObject)
	{
		SPtr<ShaderVariationParameters> nativeObject = B3DMakeShared<ShaderVariationParameters>();
		ScriptObjectWrapper::Create<ScriptShaderVariationParameters>(nativeObject, scriptObject);
	}

	int32_t ScriptShaderVariationParameters::InternalGetInt(ScriptShaderVariationParameters* self, MonoString* name)
	{
		int32_t tmp__output;
		if(!self->IsNativeObjectValid())
			return {};

		String tmpname;
		tmpname = MonoUtil::MonoToString(name);
		tmp__output = static_cast<ShaderVariationParameters*>(self->GetNativeObject())->GetI32(tmpname);

		int32_t __output;
		__output = tmp__output;

		return __output;
	}

	uint32_t ScriptShaderVariationParameters::InternalGetUInt(ScriptShaderVariationParameters* self, MonoString* name)
	{
		uint32_t tmp__output;
		if(!self->IsNativeObjectValid())
			return {};

		String tmpname;
		tmpname = MonoUtil::MonoToString(name);
		tmp__output = static_cast<ShaderVariationParameters*>(self->GetNativeObject())->GetUI32(tmpname);

		uint32_t __output;
		__output = tmp__output;

		return __output;
	}

	float ScriptShaderVariationParameters::InternalGetFloat(ScriptShaderVariationParameters* self, MonoString* name)
	{
		float tmp__output;
		if(!self->IsNativeObjectValid())
			return {};

		String tmpname;
		tmpname = MonoUtil::MonoToString(name);
		tmp__output = static_cast<ShaderVariationParameters*>(self->GetNativeObject())->GetFloat(tmpname);

		float __output;
		__output = tmp__output;

		return __output;
	}

	bool ScriptShaderVariationParameters::InternalGetBool(ScriptShaderVariationParameters* self, MonoString* name)
	{
		bool tmp__output;
		if(!self->IsNativeObjectValid())
			return {};

		String tmpname;
		tmpname = MonoUtil::MonoToString(name);
		tmp__output = static_cast<ShaderVariationParameters*>(self->GetNativeObject())->GetBool(tmpname);

		bool __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptShaderVariationParameters::InternalSetInt(ScriptShaderVariationParameters* self, MonoString* name, int32_t value)
	{
		if(!self->IsNativeObjectValid())
			return;

		String tmpname;
		tmpname = MonoUtil::MonoToString(name);
		static_cast<ShaderVariationParameters*>(self->GetNativeObject())->SetI32(tmpname, value);
	}

	void ScriptShaderVariationParameters::InternalSetUInt(ScriptShaderVariationParameters* self, MonoString* name, uint32_t value)
	{
		if(!self->IsNativeObjectValid())
			return;

		String tmpname;
		tmpname = MonoUtil::MonoToString(name);
		static_cast<ShaderVariationParameters*>(self->GetNativeObject())->SetU32(tmpname, value);
	}

	void ScriptShaderVariationParameters::InternalSetFloat(ScriptShaderVariationParameters* self, MonoString* name, float value)
	{
		if(!self->IsNativeObjectValid())
			return;

		String tmpname;
		tmpname = MonoUtil::MonoToString(name);
		static_cast<ShaderVariationParameters*>(self->GetNativeObject())->SetFloat(tmpname, value);
	}

	void ScriptShaderVariationParameters::InternalSetBool(ScriptShaderVariationParameters* self, MonoString* name, bool value)
	{
		if(!self->IsNativeObjectValid())
			return;

		String tmpname;
		tmpname = MonoUtil::MonoToString(name);
		static_cast<ShaderVariationParameters*>(self->GetNativeObject())->SetBool(tmpname, value);
	}

	void ScriptShaderVariationParameters::InternalRemoveParam(ScriptShaderVariationParameters* self, MonoString* paramName)
	{
		if(!self->IsNativeObjectValid())
			return;

		String tmpparamName;
		tmpparamName = MonoUtil::MonoToString(paramName);
		static_cast<ShaderVariationParameters*>(self->GetNativeObject())->RemoveParameter(tmpparamName);
	}

	bool ScriptShaderVariationParameters::InternalHasParam(ScriptShaderVariationParameters* self, MonoString* paramName)
	{
		bool tmp__output;
		if(!self->IsNativeObjectValid())
			return {};

		String tmpparamName;
		tmpparamName = MonoUtil::MonoToString(paramName);
		tmp__output = static_cast<ShaderVariationParameters*>(self->GetNativeObject())->HasParameter(tmpparamName);

		bool __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptShaderVariationParameters::InternalClearParams(ScriptShaderVariationParameters* self)
	{
		if(!self->IsNativeObjectValid())
			return;

		static_cast<ShaderVariationParameters*>(self->GetNativeObject())->ClearParameters();
	}

	MonoArray* ScriptShaderVariationParameters::InternalGetParamNames(ScriptShaderVariationParameters* self)
	{
		Vector<String> nativeArray__output;
		if(!self->IsNativeObjectValid())
			return {};

		nativeArray__output = static_cast<ShaderVariationParameters*>(self->GetNativeObject())->GetParameters();

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
