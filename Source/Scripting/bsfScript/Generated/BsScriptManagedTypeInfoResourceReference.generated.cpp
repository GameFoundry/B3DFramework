//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptManagedTypeInfoResourceReference.generated.h"
#include "BsMonoMethod.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"
#include "Reflection/BsRTTIType.h"
#include "BsScriptManagedTypeInfo.generated.h"

namespace bs
{
	ScriptManagedTypeInfoResourceReference::ScriptManagedTypeInfoResourceReference(const SPtr<ManagedTypeInfoResourceReference>& nativeObject)
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
		SPtr<ManagedTypeInfo> tmp__output;
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

		SPtr<ManagedTypeInfo> tmpvalue;
		ScriptManagedTypeInfoWrapperBase* scriptObjectWrappervalue;
		scriptObjectWrappervalue = (ScriptManagedTypeInfoWrapperBase*)ScriptManagedTypeInfo::GetScriptObjectWrapper(value);
		if(scriptObjectWrappervalue != nullptr)
			tmpvalue = std::static_pointer_cast<ManagedTypeInfo>(scriptObjectWrappervalue->GetBaseNativeObjectAsShared());
		static_cast<ManagedTypeInfoResourceReference*>(self->GetNativeObject())->ResourceType = tmpvalue;
	}
}
