//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptManagedTypeInfo.generated.h"
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
	ScriptManagedTypeInfo::ScriptManagedTypeInfo(const SPtr<ManagedTypeInfo>& nativeObject)
		:TScriptReflectableWrapper(nativeObject)
	{
		RegisterEvents();
	}

	void ScriptManagedTypeInfo::SetupScriptBindings()
	{
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_Matches", (void*)&ScriptManagedTypeInfo::InternalMatches);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_IsTypeLoaded", (void*)&ScriptManagedTypeInfo::InternalIsTypeLoaded);

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
		SPtr<ManagedTypeInfo> tmptypeInfo;
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
		tmp__output = static_cast<ManagedTypeInfo*>(self->GetNativeObject())->IsTypeLoaded();

		bool __output;
		__output = tmp__output;

		return __output;
	}
}
