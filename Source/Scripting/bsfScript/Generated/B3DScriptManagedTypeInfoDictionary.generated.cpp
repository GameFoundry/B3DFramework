//************************************ B3D Framework - Copyright 2025 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DScriptManagedTypeInfoDictionary.generated.h"
#include "B3DMonoMethod.h"
#include "B3DMonoClass.h"
#include "B3DMonoUtil.h"
#include "Reflection/B3DRTTIType.h"
#include "B3DScriptManagedTypeInfo.generated.h"

namespace b3d
{
	ScriptManagedTypeInfoDictionary::ScriptManagedTypeInfoDictionary(const TShared<ManagedTypeInfoDictionary>& nativeObject)
		:TScriptReflectableWrapper(nativeObject)
	{
		RegisterEvents();
	}

	ScriptManagedTypeInfoDictionary::~ScriptManagedTypeInfoDictionary()
	{
		UnregisterEvents();
	}

	void ScriptManagedTypeInfoDictionary::SetupScriptBindings()
	{
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetKeyType", (void*)&ScriptManagedTypeInfoDictionary::InternalGetKeyType);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetKeyType", (void*)&ScriptManagedTypeInfoDictionary::InternalSetKeyType);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetValueType", (void*)&ScriptManagedTypeInfoDictionary::InternalGetValueType);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetValueType", (void*)&ScriptManagedTypeInfoDictionary::InternalSetValueType);

	}

	MonoObject* ScriptManagedTypeInfoDictionary::CreateScriptObject(bool construct)
	{
		bool dummy = false;
		void* ctorParams[1] = { &dummy };

		if(construct)
			return sInteropMetaData.ScriptClass->CreateInstance("bool", ctorParams);

		return sInteropMetaData.ScriptClass->CreateInstance(false);
	}
	MonoObject* ScriptManagedTypeInfoDictionary::InternalGetKeyType(ScriptManagedTypeInfoDictionary* self)
	{
		TShared<ManagedTypeInfo> tmp__output;
		if(!self->IsNativeObjectValid())
			return {};

		tmp__output = static_cast<ManagedTypeInfoDictionary*>(self->GetNativeObject())->KeyType;

		MonoObject* __output;
		__output = ScriptManagedTypeInfo::GetOrCreateScriptObject(tmp__output);

		return __output;
	}

	void ScriptManagedTypeInfoDictionary::InternalSetKeyType(ScriptManagedTypeInfoDictionary* self, MonoObject* value)
	{
		if(!self->IsNativeObjectValid())
			return;

		TShared<ManagedTypeInfo> tmpvalue;
		ScriptManagedTypeInfoWrapperBase* scriptObjectWrappervalue;
		scriptObjectWrappervalue = (ScriptManagedTypeInfoWrapperBase*)ScriptManagedTypeInfo::GetScriptObjectWrapper(value);
		if(scriptObjectWrappervalue != nullptr)
			tmpvalue = std::static_pointer_cast<ManagedTypeInfo>(scriptObjectWrappervalue->GetBaseNativeObjectAsShared());
		static_cast<ManagedTypeInfoDictionary*>(self->GetNativeObject())->KeyType = tmpvalue;
	}

	MonoObject* ScriptManagedTypeInfoDictionary::InternalGetValueType(ScriptManagedTypeInfoDictionary* self)
	{
		TShared<ManagedTypeInfo> tmp__output;
		if(!self->IsNativeObjectValid())
			return {};

		tmp__output = static_cast<ManagedTypeInfoDictionary*>(self->GetNativeObject())->ValueType;

		MonoObject* __output;
		__output = ScriptManagedTypeInfo::GetOrCreateScriptObject(tmp__output);

		return __output;
	}

	void ScriptManagedTypeInfoDictionary::InternalSetValueType(ScriptManagedTypeInfoDictionary* self, MonoObject* value)
	{
		if(!self->IsNativeObjectValid())
			return;

		TShared<ManagedTypeInfo> tmpvalue;
		ScriptManagedTypeInfoWrapperBase* scriptObjectWrappervalue;
		scriptObjectWrappervalue = (ScriptManagedTypeInfoWrapperBase*)ScriptManagedTypeInfo::GetScriptObjectWrapper(value);
		if(scriptObjectWrappervalue != nullptr)
			tmpvalue = std::static_pointer_cast<ManagedTypeInfo>(scriptObjectWrappervalue->GetBaseNativeObjectAsShared());
		static_cast<ManagedTypeInfoDictionary*>(self->GetNativeObject())->ValueType = tmpvalue;
	}
}
