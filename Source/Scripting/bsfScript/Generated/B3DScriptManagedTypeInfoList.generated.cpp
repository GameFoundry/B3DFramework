//************************************ B3D Framework - Copyright 2025 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DScriptManagedTypeInfoList.generated.h"
#include "B3DMonoMethod.h"
#include "B3DMonoClass.h"
#include "B3DMonoUtil.h"
#include "Reflection/B3DRTTIType.h"
#include "B3DScriptManagedTypeInfo.generated.h"

namespace b3d
{
	ScriptManagedTypeInfoList::ScriptManagedTypeInfoList(const TShared<ManagedTypeInfoList>& nativeObject)
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
		TShared<ManagedTypeInfo> tmp__output;
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

		TShared<ManagedTypeInfo> tmpvalue;
		ScriptManagedTypeInfoWrapperBase* scriptObjectWrappervalue;
		scriptObjectWrappervalue = (ScriptManagedTypeInfoWrapperBase*)ScriptManagedTypeInfo::GetScriptObjectWrapper(value);
		if(scriptObjectWrappervalue != nullptr)
			tmpvalue = std::static_pointer_cast<ManagedTypeInfo>(scriptObjectWrappervalue->GetBaseNativeObjectAsShared());
		static_cast<ManagedTypeInfoList*>(self->GetNativeObject())->ElementType = tmpvalue;
	}
}
