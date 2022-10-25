//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Wrappers/BsScriptSerializableDictionary.h"
#include "BsScriptMeta.h"
#include "BsMonoField.h"
#include "BsMonoClass.h"
#include "BsMonoManager.h"
#include "BsMonoUtil.h"
#include "Serialization/BsScriptAssemblyManager.h"
#include "Serialization/BsManagedSerializableObjectInfo.h"
#include "Wrappers/BsScriptSerializableProperty.h"

namespace bs
{
	ScriptSerializableDictionary::ScriptSerializableDictionary(MonoObject* instance, const SPtr<ManagedSerializableTypeInfoDictionary>& typeInfo)
		: ScriptObject(instance), mTypeInfo(typeInfo)
	{
	}

	void ScriptSerializableDictionary::InitRuntimeData()
	{
		metaData.ScriptClass->AddInternalCall("Internal_CreateKeyProperty", (void*)&ScriptSerializableDictionary::InternalCreateKeyProperty);
		metaData.ScriptClass->AddInternalCall("Internal_CreateValueProperty", (void*)&ScriptSerializableDictionary::InternalCreateValueProperty);
	}

	MonoObject* ScriptSerializableDictionary::Create(const ScriptSerializableProperty* native, MonoObject* managed)
	{
		SPtr<ManagedSerializableTypeInfoDictionary> dictTypeInfo =
			std::static_pointer_cast<ManagedSerializableTypeInfoDictionary>(native->GetTypeInfo());

		MonoReflectionType* internalKeyType = MonoUtil::GetType(dictTypeInfo->MKeyType->GetMonoClass());
		MonoReflectionType* internalValueType = MonoUtil::GetType(dictTypeInfo->MValueType->GetMonoClass());

		void* params[3] = { internalKeyType, internalValueType, managed };
		MonoObject* managedInstance = metaData.ScriptClass->CreateInstance(params, 3);

		new(bs_alloc<ScriptSerializableDictionary>()) ScriptSerializableDictionary(managedInstance, dictTypeInfo);
		return managedInstance;
	}

	MonoObject* ScriptSerializableDictionary::InternalCreateKeyProperty(ScriptSerializableDictionary* nativeInstance)
	{
		return ScriptSerializableProperty::Create(nativeInstance->mTypeInfo->MKeyType);
	}

	MonoObject* ScriptSerializableDictionary::InternalCreateValueProperty(ScriptSerializableDictionary* nativeInstance)
	{
		return ScriptSerializableProperty::Create(nativeInstance->mTypeInfo->MValueType);
	}
} // namespace bs
