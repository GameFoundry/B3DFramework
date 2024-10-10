//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptManagedTypeUtility.generated.h"
#include "BsMonoMethod.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"
#include "../BsManagedTypeUtility.h"
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
	ScriptManagedTypeUtility::ScriptManagedTypeUtility()
		:TScriptTypeDefinition()
	{
	}

	void ScriptManagedTypeUtility::SetupScriptBindings()
	{
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetTypeInfo", (void*)&ScriptManagedTypeUtility::InternalGetTypeInfo);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_CloneObject", (void*)&ScriptManagedTypeUtility::InternalCloneObject);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_CreateObjectOfType", (void*)&ScriptManagedTypeUtility::InternalCreateObjectOfType);

	}

	MonoObject* ScriptManagedTypeUtility::InternalGetTypeInfo(MonoObject* scriptObject)
	{
		SPtr<ManagedTypeInfo> tmp__output;
		_MonoObject* tmpscriptObject;
		tmpscriptObject = scriptObject;
		tmp__output = ManagedTypeUtility::GetTypeInfo(tmpscriptObject);

		MonoObject* __output;
		__output = ScriptManagedTypeInfo::GetOrCreateScriptObject(tmp__output);

		return __output;
	}

	MonoObject* ScriptManagedTypeUtility::InternalCloneObject(MonoObject* original)
	{
		_MonoObject* tmp__output;
		_MonoObject* tmporiginal;
		tmporiginal = original;
		tmp__output = ManagedTypeUtility::CloneObject(tmporiginal);

		MonoObject* __output;
		__output = tmp__output;

		return __output;
	}

	MonoObject* ScriptManagedTypeUtility::InternalCreateObjectOfType(MonoReflectionType* type)
	{
		_MonoObject* tmp__output;
		_MonoReflectionType* tmptype;
		tmptype = type;
		tmp__output = ManagedTypeUtility::CreateObjectOfType(tmptype);

		MonoObject* __output;
		__output = tmp__output;

		return __output;
	}
}
