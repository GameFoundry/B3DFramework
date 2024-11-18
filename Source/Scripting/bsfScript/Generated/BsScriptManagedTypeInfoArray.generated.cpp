//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptManagedTypeInfoArray.generated.h"
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
	ScriptManagedTypeInfoArray::ScriptManagedTypeInfoArray(const SPtr<ManagedTypeInfoArray>& nativeObject)
		:TScriptReflectableWrapper(nativeObject)
	{
		RegisterEvents();
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
		SPtr<ManagedTypeInfo> tmp__output;
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

		SPtr<ManagedTypeInfo> tmpvalue;
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
