//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptManagedTypeInfoList.generated.h"
#include "BsMonoMethod.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"
#include "Reflection/BsRTTIType.h"
#include "BsScriptManagedTypeInfo.generated.h"

namespace bs
{
	ScriptManagedTypeInfoList::ScriptManagedTypeInfoList(const SPtr<ManagedTypeInfoList>& nativeObject)
		:TScriptReflectableWrapper(nativeObject)
	{
		RegisterEvents();
	}

	ScriptManagedTypeInfoList::~ScriptManagedTypeInfoList()
	{
		UnregisterEvents();
	}

	void ScriptManagedTypeInfoList::SetupScriptBindings()
	{
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetElementType", (void*)&ScriptManagedTypeInfoList::InternalGetElementType);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetElementType", (void*)&ScriptManagedTypeInfoList::InternalSetElementType);

	}

	MonoObject* ScriptManagedTypeInfoList::CreateScriptObject(bool construct)
	{
		bool dummy = false;
		void* ctorParams[1] = { &dummy };

		if(construct)
			return sInteropMetaData.ScriptClass->CreateInstance("bool", ctorParams);

		return sInteropMetaData.ScriptClass->CreateInstance(false);
	}
	MonoObject* ScriptManagedTypeInfoList::InternalGetElementType(ScriptManagedTypeInfoList* self)
	{
		SPtr<ManagedTypeInfo> tmp__output;
		if(!self->IsNativeObjectValid())
			return {};

		tmp__output = static_cast<ManagedTypeInfoList*>(self->GetNativeObject())->ElementType;

		MonoObject* __output;
		__output = ScriptManagedTypeInfo::GetOrCreateScriptObject(tmp__output);

		return __output;
	}

	void ScriptManagedTypeInfoList::InternalSetElementType(ScriptManagedTypeInfoList* self, MonoObject* value)
	{
		if(!self->IsNativeObjectValid())
			return;

		SPtr<ManagedTypeInfo> tmpvalue;
		ScriptManagedTypeInfoWrapperBase* scriptObjectWrappervalue;
		scriptObjectWrappervalue = (ScriptManagedTypeInfoWrapperBase*)ScriptManagedTypeInfo::GetScriptObjectWrapper(value);
		if(scriptObjectWrappervalue != nullptr)
			tmpvalue = std::static_pointer_cast<ManagedTypeInfo>(scriptObjectWrappervalue->GetBaseNativeObjectAsShared());
		static_cast<ManagedTypeInfoList*>(self->GetNativeObject())->ElementType = tmpvalue;
	}
}
