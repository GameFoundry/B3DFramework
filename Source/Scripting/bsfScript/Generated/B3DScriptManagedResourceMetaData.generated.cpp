//************************************ B3D Framework - Copyright 2025 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DScriptManagedResourceMetaData.generated.h"
#include "B3DMonoMethod.h"
#include "B3DMonoClass.h"
#include "B3DMonoUtil.h"

namespace b3d
{
	ScriptManagedResourceMetaData::ScriptManagedResourceMetaData(const TShared<ManagedResourceMetaData>& nativeObject)
		:TScriptReflectableWrapper(nativeObject)
	{
		RegisterEvents();
	}

	ScriptManagedResourceMetaData::~ScriptManagedResourceMetaData()
	{
		UnregisterEvents();
	}

	void ScriptManagedResourceMetaData::SetupScriptBindings()
	{
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetTypeNamespace", (void*)&ScriptManagedResourceMetaData::InternalGetTypeNamespace);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetTypeNamespace", (void*)&ScriptManagedResourceMetaData::InternalSetTypeNamespace);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetTypeName", (void*)&ScriptManagedResourceMetaData::InternalGetTypeName);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetTypeName", (void*)&ScriptManagedResourceMetaData::InternalSetTypeName);

	}

	MonoObject* ScriptManagedResourceMetaData::CreateScriptObject(bool construct)
	{
		bool dummy = false;
		void* ctorParams[1] = { &dummy };

		if(construct)
			return sInteropMetaData.ScriptClass->CreateInstance("bool", ctorParams);

		return sInteropMetaData.ScriptClass->CreateInstance(false);
	}
	MonoString* ScriptManagedResourceMetaData::InternalGetTypeNamespace(ScriptManagedResourceMetaData* self)
	{
		String tmp__output;
		if(!self->IsNativeObjectValid())
			return {};

		tmp__output = static_cast<ManagedResourceMetaData*>(self->GetNativeObject())->TypeNamespace;

		MonoString* __output;
		__output = MonoUtil::StringToMono(tmp__output);

		return __output;
	}

	void ScriptManagedResourceMetaData::InternalSetTypeNamespace(ScriptManagedResourceMetaData* self, MonoString* value)
	{
		if(!self->IsNativeObjectValid())
			return;

		String tmpvalue;
		tmpvalue = MonoUtil::MonoToString(value);
		static_cast<ManagedResourceMetaData*>(self->GetNativeObject())->TypeNamespace = tmpvalue;
	}

	MonoString* ScriptManagedResourceMetaData::InternalGetTypeName(ScriptManagedResourceMetaData* self)
	{
		String tmp__output;
		if(!self->IsNativeObjectValid())
			return {};

		tmp__output = static_cast<ManagedResourceMetaData*>(self->GetNativeObject())->TypeName;

		MonoString* __output;
		__output = MonoUtil::StringToMono(tmp__output);

		return __output;
	}

	void ScriptManagedResourceMetaData::InternalSetTypeName(ScriptManagedResourceMetaData* self, MonoString* value)
	{
		if(!self->IsNativeObjectValid())
			return;

		String tmpvalue;
		tmpvalue = MonoUtil::MonoToString(value);
		static_cast<ManagedResourceMetaData*>(self->GetNativeObject())->TypeName = tmpvalue;
	}
}
