//************************************ B3D Framework - Copyright 2025 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DScriptShaderVariationParameters.generated.h"
#include "B3DMonoMethod.h"
#include "B3DMonoClass.h"
#include "B3DMonoUtil.h"

namespace b3d
{
	ScriptShaderVariationParameters::ScriptShaderVariationParameters(const TShared<ShaderVariationParameters>& nativeObject)
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
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetI32", (void*)&ScriptShaderVariationParameters::InternalGetI32);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetUI32", (void*)&ScriptShaderVariationParameters::InternalGetUI32);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetFloat", (void*)&ScriptShaderVariationParameters::InternalGetFloat);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetBool", (void*)&ScriptShaderVariationParameters::InternalGetBool);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetI32", (void*)&ScriptShaderVariationParameters::InternalSetI32);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetU32", (void*)&ScriptShaderVariationParameters::InternalSetU32);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetFloat", (void*)&ScriptShaderVariationParameters::InternalSetFloat);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetBool", (void*)&ScriptShaderVariationParameters::InternalSetBool);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_RemoveParameter", (void*)&ScriptShaderVariationParameters::InternalRemoveParameter);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_HasParameter", (void*)&ScriptShaderVariationParameters::InternalHasParameter);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_ClearParameters", (void*)&ScriptShaderVariationParameters::InternalClearParameters);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetParameters", (void*)&ScriptShaderVariationParameters::InternalGetParameters);

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
		TShared<ShaderVariationParameters> nativeObject = B3DMakeShared<ShaderVariationParameters>();
		ScriptObjectWrapper::Create<ScriptShaderVariationParameters>(nativeObject, scriptObject);
	}

	int32_t ScriptShaderVariationParameters::InternalGetI32(ScriptShaderVariationParameters* self, MonoString* name)
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

	uint32_t ScriptShaderVariationParameters::InternalGetUI32(ScriptShaderVariationParameters* self, MonoString* name)
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

	void ScriptShaderVariationParameters::InternalSetI32(ScriptShaderVariationParameters* self, MonoString* name, int32_t value)
	{
		if(!self->IsNativeObjectValid())
			return;

		String tmpname;
		tmpname = MonoUtil::MonoToString(name);
		static_cast<ShaderVariationParameters*>(self->GetNativeObject())->SetI32(tmpname, value);
	}

	void ScriptShaderVariationParameters::InternalSetU32(ScriptShaderVariationParameters* self, MonoString* name, uint32_t value)
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

	void ScriptShaderVariationParameters::InternalRemoveParameter(ScriptShaderVariationParameters* self, MonoString* parameter)
	{
		if(!self->IsNativeObjectValid())
			return;

		String tmpparameter;
		tmpparameter = MonoUtil::MonoToString(parameter);
		static_cast<ShaderVariationParameters*>(self->GetNativeObject())->RemoveParameter(tmpparameter);
	}

	bool ScriptShaderVariationParameters::InternalHasParameter(ScriptShaderVariationParameters* self, MonoString* paramName)
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

	void ScriptShaderVariationParameters::InternalClearParameters(ScriptShaderVariationParameters* self)
	{
		if(!self->IsNativeObjectValid())
			return;

		static_cast<ShaderVariationParameters*>(self->GetNativeObject())->ClearParameters();
	}

	MonoArray* ScriptShaderVariationParameters::InternalGetParameters(ScriptShaderVariationParameters* self)
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
