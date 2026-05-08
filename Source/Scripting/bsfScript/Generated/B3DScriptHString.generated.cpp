//************************************ B3D Framework - Copyright 2025 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DScriptHString.generated.h"
#include "B3DMonoMethod.h"
#include "B3DMonoClass.h"
#include "B3DMonoUtil.h"
#include "../../../Engine/Core/Localization/B3DHString.h"

namespace b3d
{
	ScriptLocString::ScriptLocString(const TShared<HString>& nativeObject)
		:TScriptNonReflectableWrapper(nativeObject)
	{
		RegisterEvents();
	}

	ScriptLocString::~ScriptLocString()
	{
		UnregisterEvents();
	}

	void ScriptLocString::SetupScriptBindings()
	{
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_HString", (void*)&ScriptLocString::InternalHString);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_HString0", (void*)&ScriptLocString::InternalHString0);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_HString1", (void*)&ScriptLocString::InternalHString1);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_HString2", (void*)&ScriptLocString::InternalHString2);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetValue", (void*)&ScriptLocString::InternalGetValue);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetParameter", (void*)&ScriptLocString::InternalSetParameter);

	}

	MonoObject* ScriptLocString::CreateScriptObject(bool construct)
	{
		bool dummy = false;
		void* ctorParams[1] = { &dummy };

		if(construct)
			return sInteropMetaData.ScriptClass->CreateInstance("bool", ctorParams);

		return sInteropMetaData.ScriptClass->CreateInstance(false);
	}
	void ScriptLocString::InternalHString(MonoObject* scriptObject, MonoString* identifier, uint32_t stringTableId)
	{
		String tmpidentifier;
		tmpidentifier = MonoUtil::MonoToString(identifier);
		TShared<HString> nativeObject = B3DMakeShared<HString>(tmpidentifier, stringTableId);
		ScriptObjectWrapper::Create<ScriptLocString>(nativeObject, scriptObject);
	}

	void ScriptLocString::InternalHString0(MonoObject* scriptObject, MonoString* identifier, MonoString* defaultString, uint32_t stringTableId)
	{
		String tmpidentifier;
		tmpidentifier = MonoUtil::MonoToString(identifier);
		String tmpdefaultString;
		tmpdefaultString = MonoUtil::MonoToString(defaultString);
		TShared<HString> nativeObject = B3DMakeShared<HString>(tmpidentifier, tmpdefaultString, stringTableId);
		ScriptObjectWrapper::Create<ScriptLocString>(nativeObject, scriptObject);
	}

	void ScriptLocString::InternalHString1(MonoObject* scriptObject, uint32_t stringTableId)
	{
		TShared<HString> nativeObject = B3DMakeShared<HString>(stringTableId);
		ScriptObjectWrapper::Create<ScriptLocString>(nativeObject, scriptObject);
	}

	void ScriptLocString::InternalHString2(MonoObject* scriptObject)
	{
		TShared<HString> nativeObject = B3DMakeShared<HString>();
		ScriptObjectWrapper::Create<ScriptLocString>(nativeObject, scriptObject);
	}

	MonoString* ScriptLocString::InternalGetValue(ScriptLocString* self)
	{
		String tmp__output;
		if(!self->IsNativeObjectValid())
			return {};

		tmp__output = static_cast<HString*>(self->GetNativeObject())->GetValue();

		MonoString* __output;
		__output = MonoUtil::StringToMono(tmp__output);

		return __output;
	}

	void ScriptLocString::InternalSetParameter(ScriptLocString* self, uint32_t index, MonoString* value)
	{
		if(!self->IsNativeObjectValid())
			return;

		String tmpvalue;
		tmpvalue = MonoUtil::MonoToString(value);
		static_cast<HString*>(self->GetNativeObject())->SetParameter(index, tmpvalue);
	}
}
