//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptManagedTypeInfoDictionary.generated.h"
#include "BsMonoMethod.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"
#include "Reflection/BsRTTIType.h"
#include "BsScriptManagedTypeInfo.generated.h"
#include "../Serialization/BsManagedTypeInfo.h"
#include "BsScriptManagedTypeInfoDictionary.generated.h"
#include "../Serialization/BsManagedTypeInfo.h"
#include "BsScriptManagedTypeInfoResourceReference.generated.h"
#include "../Serialization/BsManagedTypeInfo.h"
#include "BsScriptManagedTypeInfoReference.generated.h"
#include "../Serialization/BsManagedTypeInfo.h"
#include "BsScriptManagedTypeInfoPrimitive.generated.h"
#include "../Serialization/BsManagedTypeInfo.h"
#include "BsScriptManagedTypeInfoArray.generated.h"
#include "../Serialization/BsManagedTypeInfo.h"
#include "BsScriptManagedTypeInfoEnum.generated.h"
#include "../Serialization/BsManagedTypeInfo.h"
#include "BsScriptManagedTypeInfoObject.generated.h"
#include "../Serialization/BsManagedTypeInfo.h"
#include "BsScriptManagedTypeInfoList.generated.h"

namespace bs
{
	ScriptManagedTypeInfoDictionary::ScriptManagedTypeInfoDictionary(const SPtr<ManagedTypeInfoDictionary>& nativeObject)
		:TScriptReflectableWrapper(nativeObject)
	{
		RegisterEvents();
	}

	void ScriptManagedTypeInfoDictionary::SetupScriptBindings()
	{
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetKeyType", (void*)&ScriptManagedTypeInfoDictionary::InternalGetKeyType);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetKeyType", (void*)&ScriptManagedTypeInfoDictionary::InternalSetKeyType);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetValueType", (void*)&ScriptManagedTypeInfoDictionary::InternalGetValueType);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetValueType", (void*)&ScriptManagedTypeInfoDictionary::InternalSetValueType);

	}

	MonoObject* ScriptManagedTypeInfoDictionary::CreateScriptObject(bool construct)
	{
		bool dummy = false;
		void* ctorParams[1] = { &dummy };

		if(construct)
			return sInteropMetaData.ScriptClass->CreateInstance("bool", ctorParams);

		return sInteropMetaData.ScriptClass->CreateInstance(false);
	}
	MonoObject* ScriptManagedTypeInfoDictionary::InternalGetKeyType(ScriptManagedTypeInfoDictionary* self)
	{
		SPtr<ManagedTypeInfo> tmp__output;
		tmp__output = static_cast<ManagedTypeInfoDictionary*>(self->GetNativeObject())->KeyType;

		MonoObject* __output;
		__output = ScriptManagedTypeInfo::GetOrCreateScriptObject(tmp__output);

		return __output;
	}

	void ScriptManagedTypeInfoDictionary::InternalSetKeyType(ScriptManagedTypeInfoDictionary* self, MonoObject* value)
	{
		SPtr<ManagedTypeInfo> tmpvalue;
		ScriptManagedTypeInfoWrapperBase* scriptObjectWrappervalue;
		scriptObjectWrappervalue = (ScriptManagedTypeInfoWrapperBase*)ScriptManagedTypeInfo::GetScriptObjectWrapper(value);
		if(scriptObjectWrappervalue != nullptr)
			tmpvalue = std::static_pointer_cast<ManagedTypeInfo>(scriptObjectWrappervalue->GetBaseNativeObjectAsShared());
		static_cast<ManagedTypeInfoDictionary*>(self->GetNativeObject())->KeyType = tmpvalue;
	}

	MonoObject* ScriptManagedTypeInfoDictionary::InternalGetValueType(ScriptManagedTypeInfoDictionary* self)
	{
		SPtr<ManagedTypeInfo> tmp__output;
		tmp__output = static_cast<ManagedTypeInfoDictionary*>(self->GetNativeObject())->ValueType;

		MonoObject* __output;
		__output = ScriptManagedTypeInfo::GetOrCreateScriptObject(tmp__output);

		return __output;
	}

	void ScriptManagedTypeInfoDictionary::InternalSetValueType(ScriptManagedTypeInfoDictionary* self, MonoObject* value)
	{
		SPtr<ManagedTypeInfo> tmpvalue;
		ScriptManagedTypeInfoWrapperBase* scriptObjectWrappervalue;
		scriptObjectWrappervalue = (ScriptManagedTypeInfoWrapperBase*)ScriptManagedTypeInfo::GetScriptObjectWrapper(value);
		if(scriptObjectWrappervalue != nullptr)
			tmpvalue = std::static_pointer_cast<ManagedTypeInfo>(scriptObjectWrappervalue->GetBaseNativeObjectAsShared());
		static_cast<ManagedTypeInfoDictionary*>(self->GetNativeObject())->ValueType = tmpvalue;
	}
}
