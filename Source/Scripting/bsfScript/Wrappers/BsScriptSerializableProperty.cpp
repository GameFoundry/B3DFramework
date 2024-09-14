//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Wrappers/BsScriptSerializableProperty.h"
#include "BsMonoField.h"
#include "BsMonoClass.h"
#include "BsMonoManager.h"
#include "BsMonoUtil.h"
#include "Serialization/BsManagedSerializableObjectInfo.h"
#include "Wrappers/BsScriptSerializableObject.h"
#include "Wrappers/BsScriptSerializableArray.h"
#include "Wrappers/BsScriptSerializableList.h"
#include "Wrappers/BsScriptSerializableDictionary.h"
#include "Serialization/BsScriptAssemblyManager.h"
#include "Serialization/BsManagedSerializableObject.h"
#include "Serialization/BsManagedSerializableArray.h"
#include "Serialization/BsManagedSerializableList.h"
#include "Serialization/BsManagedSerializableDictionary.h"
#include "Serialization/BsManagedSerializableField.h"

using namespace bs;
ScriptSerializableProperty::ScriptSerializableProperty(MonoObject* instance, const SPtr<ManagedSerializableTypeInfo>& typeInfo)
	: ScriptObject(instance), mTypeInfo(typeInfo)
{
}

void ScriptSerializableProperty::InitRuntimeData()
{
	metaData.ScriptClass->AddInternalCall("Internal_CreateInstance", (void*)&ScriptSerializableProperty::InternalCreateInstance);
	metaData.ScriptClass->AddInternalCall("Internal_CreateObject", (void*)&ScriptSerializableProperty::InternalCreateObject);
	metaData.ScriptClass->AddInternalCall("Internal_CreateArray", (void*)&ScriptSerializableProperty::InternalCreateArray);
	metaData.ScriptClass->AddInternalCall("Internal_CreateList", (void*)&ScriptSerializableProperty::InternalCreateList);
	metaData.ScriptClass->AddInternalCall("Internal_CreateDictionary", (void*)&ScriptSerializableProperty::InternalCreateDictionary);
	metaData.ScriptClass->AddInternalCall("Internal_CreateManagedObjectInstance", (void*)&ScriptSerializableProperty::InternalCreateManagedObjectInstance);
	metaData.ScriptClass->AddInternalCall("Internal_CreateManagedArrayInstance", (void*)&ScriptSerializableProperty::InternalCreateManagedArrayInstance);
	metaData.ScriptClass->AddInternalCall("Internal_CreateManagedListInstance", (void*)&ScriptSerializableProperty::InternalCreateManagedListInstance);
	metaData.ScriptClass->AddInternalCall("Internal_CreateManagedDictionaryInstance", (void*)&ScriptSerializableProperty::InternalCreateManagedDictionaryInstance);
}

MonoObject* ScriptSerializableProperty::Create(const SPtr<ManagedSerializableTypeInfo>& typeInfo)
{
	MonoObject* managedInstance = metaData.ScriptClass->CreateInstance();
	new(B3DAllocate<ScriptSerializableProperty>()) ScriptSerializableProperty(managedInstance, typeInfo);

	return managedInstance;
}

void ScriptSerializableProperty::InternalCreateInstance(MonoObject* instance, MonoReflectionType* reflType)
{
	if(reflType == nullptr)
		return;

	::MonoClass* monoClass = MonoUtil::GetClass(reflType);
	MonoClass* engineClass = MonoManager::Instance().FindClass(monoClass);

	SPtr<ManagedSerializableTypeInfo> typeInfo = ScriptAssemblyManager::Instance().GetTypeInfo(engineClass);
	if(typeInfo == nullptr)
	{
		B3D_LOG(Warning, Script, "Cannot create an instance of type \"{0}\", it is not marked as serializable.", engineClass->GetFullName());
		return;
	}

	new(B3DAllocate<ScriptSerializableProperty>()) ScriptSerializableProperty(instance, typeInfo);
}

MonoObject* ScriptSerializableProperty::InternalCreateObject(ScriptSerializableProperty* nativeInstance, MonoObject* managedInstance, MonoReflectionType* reflType)
{
	return ScriptSerializableObject::Create(nativeInstance, managedInstance, reflType);
}

MonoObject* ScriptSerializableProperty::InternalCreateArray(ScriptSerializableProperty* nativeInstance, MonoObject* managedInstance)
{
	return ScriptSerializableArray::Create(nativeInstance, managedInstance);
}

MonoObject* ScriptSerializableProperty::InternalCreateList(ScriptSerializableProperty* nativeInstance, MonoObject* managedInstance)
{
	return ScriptSerializableList::Create(nativeInstance, managedInstance);
}

MonoObject* ScriptSerializableProperty::InternalCreateDictionary(ScriptSerializableProperty* nativeInstance, MonoObject* managedInstance)
{
	return ScriptSerializableDictionary::Create(nativeInstance, managedInstance);
}

MonoObject* ScriptSerializableProperty::InternalCreateManagedObjectInstance(ScriptSerializableProperty* nativeInstance)
{
	SPtr<ManagedSerializableTypeInfoObject> objectTypeInfo = std::static_pointer_cast<ManagedSerializableTypeInfoObject>(nativeInstance->mTypeInfo);
	return ManagedSerializableObject::CreateManagedInstance(objectTypeInfo);
}

MonoObject* ScriptSerializableProperty::InternalCreateManagedArrayInstance(ScriptSerializableProperty* nativeInstance, MonoArray* sizes)
{
	ScriptArray scriptArray(sizes);

	Vector<u32> nativeSizes;
	u32 arrayLen = scriptArray.Size();
	for(u32 i = 0; i < arrayLen; i++)
		nativeSizes.push_back(scriptArray.Get<u32>(i));

	SPtr<ManagedSerializableTypeInfoArray> arrayTypeInfo = std::static_pointer_cast<ManagedSerializableTypeInfoArray>(nativeInstance->mTypeInfo);
	return ManagedSerializableArray::CreateManagedInstance(arrayTypeInfo, nativeSizes);
}

MonoObject* ScriptSerializableProperty::InternalCreateManagedListInstance(ScriptSerializableProperty* nativeInstance, int size)
{
	SPtr<ManagedSerializableTypeInfoList> listTypeInfo = std::static_pointer_cast<ManagedSerializableTypeInfoList>(nativeInstance->mTypeInfo);
	return ManagedSerializableList::CreateManagedInstance(listTypeInfo, size);
}

MonoObject* ScriptSerializableProperty::InternalCreateManagedDictionaryInstance(ScriptSerializableProperty* nativeInstance)
{
	SPtr<ManagedSerializableTypeInfoDictionary> dictTypeInfo = std::static_pointer_cast<ManagedSerializableTypeInfoDictionary>(nativeInstance->mTypeInfo);
	return ManagedSerializableDictionary::CreateManagedInstance(dictTypeInfo);
}
