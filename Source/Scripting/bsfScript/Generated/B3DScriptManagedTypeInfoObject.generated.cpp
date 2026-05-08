//************************************ B3D Framework - Copyright 2025 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DScriptManagedTypeInfoObject.generated.h"
#include "B3DMonoMethod.h"
#include "B3DMonoClass.h"
#include "B3DMonoUtil.h"

namespace b3d
{
	ScriptManagedTypeInfoObject::ScriptManagedTypeInfoObject(const TShared<ManagedTypeInfoObject>& nativeObject)
		:TScriptReflectableWrapper(nativeObject)
	{
		RegisterEvents();
	}

	ScriptManagedTypeInfoObject::~ScriptManagedTypeInfoObject()
	{
		UnregisterEvents();
	}

	void ScriptManagedTypeInfoObject::SetupScriptBindings()
	{
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetTypeNamespace", (void*)&ScriptManagedTypeInfoObject::InternalGetTypeNamespace);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetTypeNamespace", (void*)&ScriptManagedTypeInfoObject::InternalSetTypeNamespace);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetTypeName", (void*)&ScriptManagedTypeInfoObject::InternalGetTypeName);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetTypeName", (void*)&ScriptManagedTypeInfoObject::InternalSetTypeName);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetIsValueType", (void*)&ScriptManagedTypeInfoObject::InternalGetIsValueType);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetIsValueType", (void*)&ScriptManagedTypeInfoObject::InternalSetIsValueType);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetTypeRTTIId", (void*)&ScriptManagedTypeInfoObject::InternalGetTypeRTTIId);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetTypeRTTIId", (void*)&ScriptManagedTypeInfoObject::InternalSetTypeRTTIId);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetMetaDataFlags", (void*)&ScriptManagedTypeInfoObject::InternalGetMetaDataFlags);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetMetaDataFlags", (void*)&ScriptManagedTypeInfoObject::InternalSetMetaDataFlags);

	}

	MonoObject* ScriptManagedTypeInfoObject::CreateScriptObject(bool construct)
	{
		bool dummy = false;
		void* ctorParams[1] = { &dummy };

		if(construct)
			return sInteropMetaData.ScriptClass->CreateInstance("bool", ctorParams);

		return sInteropMetaData.ScriptClass->CreateInstance(false);
	}
	MonoString* ScriptManagedTypeInfoObject::InternalGetTypeNamespace(ScriptManagedTypeInfoObject* self)
	{
		String tmp__output;
		if(!self->IsNativeObjectValid())
			return {};

		tmp__output = static_cast<ManagedTypeInfoObject*>(self->GetNativeObject())->TypeNamespace;

		MonoString* __output;
		__output = MonoUtil::StringToMono(tmp__output);

		return __output;
	}

	void ScriptManagedTypeInfoObject::InternalSetTypeNamespace(ScriptManagedTypeInfoObject* self, MonoString* value)
	{
		if(!self->IsNativeObjectValid())
			return;

		String tmpvalue;
		tmpvalue = MonoUtil::MonoToString(value);
		static_cast<ManagedTypeInfoObject*>(self->GetNativeObject())->TypeNamespace = tmpvalue;
	}

	MonoString* ScriptManagedTypeInfoObject::InternalGetTypeName(ScriptManagedTypeInfoObject* self)
	{
		String tmp__output;
		if(!self->IsNativeObjectValid())
			return {};

		tmp__output = static_cast<ManagedTypeInfoObject*>(self->GetNativeObject())->TypeName;

		MonoString* __output;
		__output = MonoUtil::StringToMono(tmp__output);

		return __output;
	}

	void ScriptManagedTypeInfoObject::InternalSetTypeName(ScriptManagedTypeInfoObject* self, MonoString* value)
	{
		if(!self->IsNativeObjectValid())
			return;

		String tmpvalue;
		tmpvalue = MonoUtil::MonoToString(value);
		static_cast<ManagedTypeInfoObject*>(self->GetNativeObject())->TypeName = tmpvalue;
	}

	bool ScriptManagedTypeInfoObject::InternalGetIsValueType(ScriptManagedTypeInfoObject* self)
	{
		bool tmp__output;
		if(!self->IsNativeObjectValid())
			return {};

		tmp__output = static_cast<ManagedTypeInfoObject*>(self->GetNativeObject())->IsValueType;

		bool __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptManagedTypeInfoObject::InternalSetIsValueType(ScriptManagedTypeInfoObject* self, bool value)
	{
		if(!self->IsNativeObjectValid())
			return;

		static_cast<ManagedTypeInfoObject*>(self->GetNativeObject())->IsValueType = value;
	}

	uint32_t ScriptManagedTypeInfoObject::InternalGetTypeRTTIId(ScriptManagedTypeInfoObject* self)
	{
		uint32_t tmp__output;
		if(!self->IsNativeObjectValid())
			return {};

		tmp__output = static_cast<ManagedTypeInfoObject*>(self->GetNativeObject())->TypeRTTIId;

		uint32_t __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptManagedTypeInfoObject::InternalSetTypeRTTIId(ScriptManagedTypeInfoObject* self, uint32_t value)
	{
		if(!self->IsNativeObjectValid())
			return;

		static_cast<ManagedTypeInfoObject*>(self->GetNativeObject())->TypeRTTIId = value;
	}

	ManagedObjectMetaDataFlag ScriptManagedTypeInfoObject::InternalGetMetaDataFlags(ScriptManagedTypeInfoObject* self)
	{
		Flags<ManagedObjectMetaDataFlag> tmp__output;
		if(!self->IsNativeObjectValid())
			return {};

		tmp__output = static_cast<ManagedTypeInfoObject*>(self->GetNativeObject())->MetaDataFlags;

		ManagedObjectMetaDataFlag __output;
		__output = (ManagedObjectMetaDataFlag)(uint32_t)tmp__output;

		return __output;
	}

	void ScriptManagedTypeInfoObject::InternalSetMetaDataFlags(ScriptManagedTypeInfoObject* self, ManagedObjectMetaDataFlag value)
	{
		if(!self->IsNativeObjectValid())
			return;

		static_cast<ManagedTypeInfoObject*>(self->GetNativeObject())->MetaDataFlags = value;
	}
}
