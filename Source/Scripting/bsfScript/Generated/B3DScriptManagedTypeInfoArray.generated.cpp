//************************************ B3D Framework - Copyright 2025 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DScriptManagedTypeInfoArray.generated.h"
#include "B3DMonoMethod.h"
#include "B3DMonoClass.h"
#include "B3DMonoUtil.h"
#include "Reflection/B3DRTTIType.h"
#include "B3DScriptManagedTypeInfo.generated.h"

namespace b3d
{
	ScriptManagedTypeInfoArray::ScriptManagedTypeInfoArray(const TShared<ManagedTypeInfoArray>& nativeObject)
		:TScriptReflectableWrapper(nativeObject)
	{
		RegisterEvents();
	}

	ScriptManagedTypeInfoArray::~ScriptManagedTypeInfoArray()
	{
		UnregisterEvents();
	}

	void ScriptManagedTypeInfoArray::SetupScriptBindings()
	{
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetElementType", (void*)&ScriptManagedTypeInfoArray::InternalGetElementType);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetElementType", (void*)&ScriptManagedTypeInfoArray::InternalSetElementType);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetRank", (void*)&ScriptManagedTypeInfoArray::InternalGetRank);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetRank", (void*)&ScriptManagedTypeInfoArray::InternalSetRank);

	}

	MonoObject* ScriptManagedTypeInfoArray::CreateScriptObject(bool construct)
	{
		bool dummy = false;
		void* ctorParams[1] = { &dummy };

		if(construct)
			return sInteropMetaData.ScriptClass->CreateInstance("bool", ctorParams);

		return sInteropMetaData.ScriptClass->CreateInstance(false);
	}
	MonoObject* ScriptManagedTypeInfoArray::InternalGetElementType(ScriptManagedTypeInfoArray* self)
	{
		TShared<ManagedTypeInfo> tmp__output;
		if(!self->IsNativeObjectValid())
			return {};

		tmp__output = static_cast<ManagedTypeInfoArray*>(self->GetNativeObject())->ElementType;

		MonoObject* __output;
		__output = ScriptManagedTypeInfo::GetOrCreateScriptObject(tmp__output);

		return __output;
	}

	void ScriptManagedTypeInfoArray::InternalSetElementType(ScriptManagedTypeInfoArray* self, MonoObject* value)
	{
		if(!self->IsNativeObjectValid())
			return;

		TShared<ManagedTypeInfo> tmpvalue;
		ScriptManagedTypeInfoWrapperBase* scriptObjectWrappervalue;
		scriptObjectWrappervalue = (ScriptManagedTypeInfoWrapperBase*)ScriptManagedTypeInfo::GetScriptObjectWrapper(value);
		if(scriptObjectWrappervalue != nullptr)
			tmpvalue = std::static_pointer_cast<ManagedTypeInfo>(scriptObjectWrappervalue->GetBaseNativeObjectAsShared());
		static_cast<ManagedTypeInfoArray*>(self->GetNativeObject())->ElementType = tmpvalue;
	}

	uint32_t ScriptManagedTypeInfoArray::InternalGetRank(ScriptManagedTypeInfoArray* self)
	{
		uint32_t tmp__output;
		if(!self->IsNativeObjectValid())
			return {};

		tmp__output = static_cast<ManagedTypeInfoArray*>(self->GetNativeObject())->Rank;

		uint32_t __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptManagedTypeInfoArray::InternalSetRank(ScriptManagedTypeInfoArray* self, uint32_t value)
	{
		if(!self->IsNativeObjectValid())
			return;

		static_cast<ManagedTypeInfoArray*>(self->GetNativeObject())->Rank = value;
	}
}
