//************************************ B3D Framework - Copyright 2025 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DScriptManagedTypeInfoEnum.generated.h"
#include "B3DMonoMethod.h"
#include "B3DMonoClass.h"
#include "B3DMonoUtil.h"

namespace b3d
{
	ScriptManagedTypeInfoEnum::ScriptManagedTypeInfoEnum(const TShared<ManagedTypeInfoEnum>& nativeObject)
		:TScriptReflectableWrapper(nativeObject)
	{
		RegisterEvents();
	}

	ScriptManagedTypeInfoEnum::~ScriptManagedTypeInfoEnum()
	{
		UnregisterEvents();
	}

	void ScriptManagedTypeInfoEnum::SetupScriptBindings()
	{
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetUnderlyingType", (void*)&ScriptManagedTypeInfoEnum::InternalGetUnderlyingType);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetUnderlyingType", (void*)&ScriptManagedTypeInfoEnum::InternalSetUnderlyingType);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetTypeNamespace", (void*)&ScriptManagedTypeInfoEnum::InternalGetTypeNamespace);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetTypeNamespace", (void*)&ScriptManagedTypeInfoEnum::InternalSetTypeNamespace);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetTypeName", (void*)&ScriptManagedTypeInfoEnum::InternalGetTypeName);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetTypeName", (void*)&ScriptManagedTypeInfoEnum::InternalSetTypeName);

	}

	MonoObject* ScriptManagedTypeInfoEnum::CreateScriptObject(bool construct)
	{
		bool dummy = false;
		void* ctorParams[1] = { &dummy };

		if(construct)
			return sInteropMetaData.ScriptClass->CreateInstance("bool", ctorParams);

		return sInteropMetaData.ScriptClass->CreateInstance(false);
	}
	ManagedPrimitiveType ScriptManagedTypeInfoEnum::InternalGetUnderlyingType(ScriptManagedTypeInfoEnum* self)
	{
		ManagedPrimitiveType tmp__output;
		if(!self->IsNativeObjectValid())
			return {};

		tmp__output = static_cast<ManagedTypeInfoEnum*>(self->GetNativeObject())->UnderlyingType;

		ManagedPrimitiveType __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptManagedTypeInfoEnum::InternalSetUnderlyingType(ScriptManagedTypeInfoEnum* self, ManagedPrimitiveType value)
	{
		if(!self->IsNativeObjectValid())
			return;

		static_cast<ManagedTypeInfoEnum*>(self->GetNativeObject())->UnderlyingType = value;
	}

	MonoString* ScriptManagedTypeInfoEnum::InternalGetTypeNamespace(ScriptManagedTypeInfoEnum* self)
	{
		String tmp__output;
		if(!self->IsNativeObjectValid())
			return {};

		tmp__output = static_cast<ManagedTypeInfoEnum*>(self->GetNativeObject())->TypeNamespace;

		MonoString* __output;
		__output = MonoUtil::StringToMono(tmp__output);

		return __output;
	}

	void ScriptManagedTypeInfoEnum::InternalSetTypeNamespace(ScriptManagedTypeInfoEnum* self, MonoString* value)
	{
		if(!self->IsNativeObjectValid())
			return;

		String tmpvalue;
		tmpvalue = MonoUtil::MonoToString(value);
		static_cast<ManagedTypeInfoEnum*>(self->GetNativeObject())->TypeNamespace = tmpvalue;
	}

	MonoString* ScriptManagedTypeInfoEnum::InternalGetTypeName(ScriptManagedTypeInfoEnum* self)
	{
		String tmp__output;
		if(!self->IsNativeObjectValid())
			return {};

		tmp__output = static_cast<ManagedTypeInfoEnum*>(self->GetNativeObject())->TypeName;

		MonoString* __output;
		__output = MonoUtil::StringToMono(tmp__output);

		return __output;
	}

	void ScriptManagedTypeInfoEnum::InternalSetTypeName(ScriptManagedTypeInfoEnum* self, MonoString* value)
	{
		if(!self->IsNativeObjectValid())
			return;

		String tmpvalue;
		tmpvalue = MonoUtil::MonoToString(value);
		static_cast<ManagedTypeInfoEnum*>(self->GetNativeObject())->TypeName = tmpvalue;
	}
}
