//************************************ B3D Framework - Copyright 2025 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DScriptManagedTypeInfoReference.generated.h"
#include "B3DMonoMethod.h"
#include "B3DMonoClass.h"
#include "B3DMonoUtil.h"

namespace b3d
{
	ScriptManagedTypeInfoReference::ScriptManagedTypeInfoReference(const TShared<ManagedTypeInfoReference>& nativeObject)
		:TScriptReflectableWrapper(nativeObject)
	{
		RegisterEvents();
	}

	ScriptManagedTypeInfoReference::~ScriptManagedTypeInfoReference()
	{
		UnregisterEvents();
	}

	void ScriptManagedTypeInfoReference::SetupScriptBindings()
	{
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetReferenceType", (void*)&ScriptManagedTypeInfoReference::InternalGetReferenceType);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetReferenceType", (void*)&ScriptManagedTypeInfoReference::InternalSetReferenceType);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetTypeRTTIId", (void*)&ScriptManagedTypeInfoReference::InternalGetTypeRTTIId);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetTypeRTTIId", (void*)&ScriptManagedTypeInfoReference::InternalSetTypeRTTIId);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetTypeNamespace", (void*)&ScriptManagedTypeInfoReference::InternalGetTypeNamespace);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetTypeNamespace", (void*)&ScriptManagedTypeInfoReference::InternalSetTypeNamespace);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetTypeName", (void*)&ScriptManagedTypeInfoReference::InternalGetTypeName);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetTypeName", (void*)&ScriptManagedTypeInfoReference::InternalSetTypeName);

	}

	MonoObject* ScriptManagedTypeInfoReference::CreateScriptObject(bool construct)
	{
		bool dummy = false;
		void* ctorParams[1] = { &dummy };

		if(construct)
			return sInteropMetaData.ScriptClass->CreateInstance("bool", ctorParams);

		return sInteropMetaData.ScriptClass->CreateInstance(false);
	}
	ManagedReferenceType ScriptManagedTypeInfoReference::InternalGetReferenceType(ScriptManagedTypeInfoReference* self)
	{
		ManagedReferenceType tmp__output;
		if(!self->IsNativeObjectValid())
			return {};

		tmp__output = static_cast<ManagedTypeInfoReference*>(self->GetNativeObject())->ReferenceType;

		ManagedReferenceType __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptManagedTypeInfoReference::InternalSetReferenceType(ScriptManagedTypeInfoReference* self, ManagedReferenceType value)
	{
		if(!self->IsNativeObjectValid())
			return;

		static_cast<ManagedTypeInfoReference*>(self->GetNativeObject())->ReferenceType = value;
	}

	uint32_t ScriptManagedTypeInfoReference::InternalGetTypeRTTIId(ScriptManagedTypeInfoReference* self)
	{
		uint32_t tmp__output;
		if(!self->IsNativeObjectValid())
			return {};

		tmp__output = static_cast<ManagedTypeInfoReference*>(self->GetNativeObject())->TypeRTTIId;

		uint32_t __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptManagedTypeInfoReference::InternalSetTypeRTTIId(ScriptManagedTypeInfoReference* self, uint32_t value)
	{
		if(!self->IsNativeObjectValid())
			return;

		static_cast<ManagedTypeInfoReference*>(self->GetNativeObject())->TypeRTTIId = value;
	}

	MonoString* ScriptManagedTypeInfoReference::InternalGetTypeNamespace(ScriptManagedTypeInfoReference* self)
	{
		String tmp__output;
		if(!self->IsNativeObjectValid())
			return {};

		tmp__output = static_cast<ManagedTypeInfoReference*>(self->GetNativeObject())->TypeNamespace;

		MonoString* __output;
		__output = MonoUtil::StringToMono(tmp__output);

		return __output;
	}

	void ScriptManagedTypeInfoReference::InternalSetTypeNamespace(ScriptManagedTypeInfoReference* self, MonoString* value)
	{
		if(!self->IsNativeObjectValid())
			return;

		String tmpvalue;
		tmpvalue = MonoUtil::MonoToString(value);
		static_cast<ManagedTypeInfoReference*>(self->GetNativeObject())->TypeNamespace = tmpvalue;
	}

	MonoString* ScriptManagedTypeInfoReference::InternalGetTypeName(ScriptManagedTypeInfoReference* self)
	{
		String tmp__output;
		if(!self->IsNativeObjectValid())
			return {};

		tmp__output = static_cast<ManagedTypeInfoReference*>(self->GetNativeObject())->TypeName;

		MonoString* __output;
		__output = MonoUtil::StringToMono(tmp__output);

		return __output;
	}

	void ScriptManagedTypeInfoReference::InternalSetTypeName(ScriptManagedTypeInfoReference* self, MonoString* value)
	{
		if(!self->IsNativeObjectValid())
			return;

		String tmpvalue;
		tmpvalue = MonoUtil::MonoToString(value);
		static_cast<ManagedTypeInfoReference*>(self->GetNativeObject())->TypeName = tmpvalue;
	}
}
