//************************************ B3D Framework - Copyright 2025 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DScriptManagedTypeInfoResourceReference.generated.h"
#include "B3DMonoMethod.h"
#include "B3DMonoClass.h"
#include "B3DMonoUtil.h"
#include "Reflection/B3DRTTIType.h"
#include "B3DScriptManagedTypeInfo.generated.h"

namespace b3d
{
	ScriptManagedTypeInfoResourceReference::ScriptManagedTypeInfoResourceReference(const TShared<ManagedTypeInfoResourceReference>& nativeObject)
		:TScriptReflectableWrapper(nativeObject)
	{
		RegisterEvents();
	}

	ScriptManagedTypeInfoResourceReference::~ScriptManagedTypeInfoResourceReference()
	{
		UnregisterEvents();
	}

	void ScriptManagedTypeInfoResourceReference::SetupScriptBindings()
	{
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetResourceType", (void*)&ScriptManagedTypeInfoResourceReference::InternalGetResourceType);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetResourceType", (void*)&ScriptManagedTypeInfoResourceReference::InternalSetResourceType);

	}

	MonoObject* ScriptManagedTypeInfoResourceReference::CreateScriptObject(bool construct)
	{
		bool dummy = false;
		void* ctorParams[1] = { &dummy };

		if(construct)
			return sInteropMetaData.ScriptClass->CreateInstance("bool", ctorParams);

		return sInteropMetaData.ScriptClass->CreateInstance(false);
	}
	MonoObject* ScriptManagedTypeInfoResourceReference::InternalGetResourceType(ScriptManagedTypeInfoResourceReference* self)
	{
		TShared<ManagedTypeInfo> tmp__output;
		if(!self->IsNativeObjectValid())
			return {};

		tmp__output = static_cast<ManagedTypeInfoResourceReference*>(self->GetNativeObject())->ResourceType;

		MonoObject* __output;
		__output = ScriptManagedTypeInfo::GetOrCreateScriptObject(tmp__output);

		return __output;
	}

	void ScriptManagedTypeInfoResourceReference::InternalSetResourceType(ScriptManagedTypeInfoResourceReference* self, MonoObject* value)
	{
		if(!self->IsNativeObjectValid())
			return;

		TShared<ManagedTypeInfo> tmpvalue;
		ScriptManagedTypeInfoWrapperBase* scriptObjectWrappervalue;
		scriptObjectWrappervalue = (ScriptManagedTypeInfoWrapperBase*)ScriptManagedTypeInfo::GetScriptObjectWrapper(value);
		if(scriptObjectWrappervalue != nullptr)
			tmpvalue = std::static_pointer_cast<ManagedTypeInfo>(scriptObjectWrappervalue->GetBaseNativeObjectAsShared());
		static_cast<ManagedTypeInfoResourceReference*>(self->GetNativeObject())->ResourceType = tmpvalue;
	}
}
