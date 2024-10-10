//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Wrappers/BsScriptSerializableDictionary.h"
#include "BsMonoField.h"
#include "BsMonoClass.h"
#include "BsMonoManager.h"
#include "BsMonoUtil.h"
#include "Serialization/BsScriptAssemblyManager.h"
#include "Serialization/BsManagedSerializableObjectInfo.h"
#include "Wrappers/BsScriptSerializableProperty.h"

using namespace bs;
ScriptSerializableDictionary::ScriptSerializableDictionary(MonoObject* instance, const SPtr<ManagedTypeInfoDictionary>& typeInfo)
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
	SPtr<ManagedTypeInfoDictionary> dictTypeInfo =
		std::static_pointer_cast<ManagedTypeInfoDictionary>(native->GetTypeInfo());

	MonoReflectionType* internalKeyType = MonoUtil::GetType(dictTypeInfo->KeyType->GetMonoClass());
	MonoReflectionType* internalValueType = MonoUtil::GetType(dictTypeInfo->ValueType->GetMonoClass());

	void* params[3] = { internalKeyType, internalValueType, managed };
	MonoObject* managedInstance = metaData.ScriptClass->CreateInstance(params, 3);

	new(B3DAllocate<ScriptSerializableDictionary>()) ScriptSerializableDictionary(managedInstance, dictTypeInfo);
	return managedInstance;
}

MonoObject* ScriptSerializableDictionary::InternalCreateKeyProperty(ScriptSerializableDictionary* nativeInstance)
{
	return ScriptSerializableProperty::Create(nativeInstance->mTypeInfo->KeyType);
}

MonoObject* ScriptSerializableDictionary::InternalCreateValueProperty(ScriptSerializableDictionary* nativeInstance)
{
	return ScriptSerializableProperty::Create(nativeInstance->mTypeInfo->ValueType);
}
