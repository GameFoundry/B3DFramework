//************************************ B3D Framework - Copyright 2025 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DScriptManagedTypeInfo.generated.h"
#include "B3DMonoMethod.h"
#include "B3DMonoClass.h"
#include "B3DMonoUtil.h"
#include "Reflection/B3DRTTIType.h"
#include "B3DScriptManagedTypeInfo.generated.h"

namespace b3d
{
	ScriptManagedTypeInfo::ScriptManagedTypeInfo(const TShared<ManagedTypeInfo>& nativeObject)
		:TScriptReflectableWrapper(nativeObject)
	{
		RegisterEvents();
	}

	ScriptManagedTypeInfo::~ScriptManagedTypeInfo()
	{
		UnregisterEvents();
	}

	void ScriptManagedTypeInfo::SetupScriptBindings()
	{
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_Matches", (void*)&ScriptManagedTypeInfo::InternalMatches);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_IsTypeLoaded", (void*)&ScriptManagedTypeInfo::InternalIsTypeLoaded);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetReflectionType", (void*)&ScriptManagedTypeInfo::InternalGetReflectionType);

	}

	MonoObject* ScriptManagedTypeInfo::CreateScriptObject(bool construct)
	{
		bool dummy = false;
		void* ctorParams[1] = { &dummy };

		if(construct)
			return sInteropMetaData.ScriptClass->CreateInstance("bool", ctorParams);

		return sInteropMetaData.ScriptClass->CreateInstance(false);
	}
	bool ScriptManagedTypeInfo::InternalMatches(ScriptManagedTypeInfoWrapperBase* self, MonoObject* typeInfo)
	{
		bool tmp__output;
		if(!self->IsNativeObjectValid())
			return {};

		TShared<ManagedTypeInfo> tmptypeInfo;
		ScriptManagedTypeInfoWrapperBase* scriptObjectWrappertypeInfo;
		scriptObjectWrappertypeInfo = (ScriptManagedTypeInfoWrapperBase*)ScriptManagedTypeInfo::GetScriptObjectWrapper(typeInfo);
		if(scriptObjectWrappertypeInfo != nullptr)
			tmptypeInfo = std::static_pointer_cast<ManagedTypeInfo>(scriptObjectWrappertypeInfo->GetBaseNativeObjectAsShared());
		tmp__output = static_cast<ManagedTypeInfo*>(self->GetNativeObject())->Matches(tmptypeInfo);

		bool __output;
		__output = tmp__output;

		return __output;
	}

	bool ScriptManagedTypeInfo::InternalIsTypeLoaded(ScriptManagedTypeInfoWrapperBase* self)
	{
		bool tmp__output;
		if(!self->IsNativeObjectValid())
			return {};

		tmp__output = static_cast<ManagedTypeInfo*>(self->GetNativeObject())->IsTypeLoaded();

		bool __output;
		__output = tmp__output;

		return __output;
	}

	MonoReflectionType* ScriptManagedTypeInfo::InternalGetReflectionType(ScriptManagedTypeInfoWrapperBase* self)
	{
		_MonoReflectionType* tmp__output = nullptr;
		if(!self->IsNativeObjectValid())
			return {};

		tmp__output = static_cast<ManagedTypeInfo*>(self->GetNativeObject())->GetReflectionType();

		MonoReflectionType* __output;
		__output = tmp__output;

		return __output;
	}
}
