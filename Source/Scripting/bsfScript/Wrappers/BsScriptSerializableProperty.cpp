//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Wrappers/BsScriptSerializableProperty.h"
#include "BsScriptMeta.h"
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

namespace bs
{
	ScriptSerializableProperty::ScriptSerializableProperty(MonoObject* instance, const SPtr<ManagedSerializableTypeInfo>& typeInfo)
		:ScriptObject(instance), mTypeInfo(typeInfo)
	{

	}

	void ScriptSerializableProperty::initRuntimeData()
	{
		metaData.scriptClass->addInternalCall("Internal_CreateInstance", (void*)&ScriptSerializableProperty::internal_CreateInstance);
		metaData.scriptClass->addInternalCall("Internal_CreateObject", (void*)&ScriptSerializableProperty::internal_createObject);
		metaData.scriptClass->addInternalCall("Internal_CreateArray", (void*)&ScriptSerializableProperty::internal_createArray);
		metaData.scriptClass->addInternalCall("Internal_CreateList", (void*)&ScriptSerializableProperty::internal_createList);
		metaData.scriptClass->addInternalCall("Internal_CreateDictionary", (void*)&ScriptSerializableProperty::internal_createDictionary);
		metaData.scriptClass->addInternalCall("Internal_CreateManagedObjectInstance", (void*)&ScriptSerializableProperty::internal_createManagedObjectInstance);
		metaData.scriptClass->addInternalCall("Internal_CreateManagedArrayInstance", (void*)&ScriptSerializableProperty::internal_createManagedArrayInstance);
		metaData.scriptClass->addInternalCall("Internal_CreateManagedListInstance", (void*)&ScriptSerializableProperty::internal_createManagedListInstance);
		metaData.scriptClass->addInternalCall("Internal_CreateManagedDictionaryInstance", (void*)&ScriptSerializableProperty::internal_createManagedDictionaryInstance);
	}

	MonoObject* ScriptSerializableProperty::create(const SPtr<ManagedSerializableTypeInfo>& typeInfo)
	{
		MonoObject* managedInstance = metaData.scriptClass->createInstance();
		new (bs_alloc<ScriptSerializableProperty>()) ScriptSerializableProperty(managedInstance, typeInfo);

		return managedInstance;
	}

	void ScriptSerializableProperty::internal_CreateInstance(MonoObject* instance, MonoReflectionType* reflType)
	{
		if (reflType == nullptr)
			return;

		::MonoClass* monoClass = MonoUtil::getClass(reflType);
		MonoClass* engineClass = MonoManager::instance().findClass(monoClass);

		SPtr<ManagedSerializableTypeInfo> typeInfo = ScriptAssemblyManager::instance().getTypeInfo(engineClass);
		if (typeInfo == nullptr)
		{
			BS_LOG(Warning, Script, "Cannot create an instance of type \"{0}\", it is not marked as serializable.",
				engineClass->getFullName());
			return;
		}

		new (bs_alloc<ScriptSerializableProperty>()) ScriptSerializableProperty(instance, typeInfo);
	}

	MonoObject* ScriptSerializableProperty::internal_createObject(ScriptSerializableProperty* nativeInstance,
		MonoObject* managedInstance, MonoReflectionType* reflType)
	{
		return ScriptSerializableObject::create(nativeInstance, managedInstance, reflType);
	}

	MonoObject* ScriptSerializableProperty::internal_createArray(ScriptSerializableProperty* nativeInstance, MonoObject* managedInstance)
	{
		return ScriptSerializableArray::create(nativeInstance, managedInstance);
	}

	MonoObject* ScriptSerializableProperty::internal_createList(ScriptSerializableProperty* nativeInstance, MonoObject* managedInstance)
	{
		return ScriptSerializableList::create(nativeInstance, managedInstance);
	}

	MonoObject* ScriptSerializableProperty::internal_createDictionary(ScriptSerializableProperty* nativeInstance, MonoObject* managedInstance)
	{
		return ScriptSerializableDictionary::create(nativeInstance, managedInstance);
	}

	MonoObject* ScriptSerializableProperty::internal_createManagedObjectInstance(ScriptSerializableProperty* nativeInstance)
	{
		SPtr<ManagedSerializableTypeInfoObject> objectTypeInfo = std::static_pointer_cast<ManagedSerializableTypeInfoObject>(nativeInstance->mTypeInfo);
		return ManagedSerializableObject::createManagedInstance(objectTypeInfo);
	}

	MonoObject* ScriptSerializableProperty::internal_createManagedArrayInstance(ScriptSerializableProperty* nativeInstance, MonoArray* sizes)
	{
		ScriptArray scriptArray(sizes);

		Vector<UINT32> nativeSizes;
		UINT32 arrayLen = scriptArray.size();
		for (UINT32 i = 0; i < arrayLen; i++)
			nativeSizes.push_back(scriptArray.get<UINT32>(i));

		SPtr<ManagedSerializableTypeInfoArray> arrayTypeInfo = std::static_pointer_cast<ManagedSerializableTypeInfoArray>(nativeInstance->mTypeInfo);
		return ManagedSerializableArray::createManagedInstance(arrayTypeInfo, nativeSizes);
	}

	MonoObject* ScriptSerializableProperty::internal_createManagedListInstance(ScriptSerializableProperty* nativeInstance, int size)
	{
		SPtr<ManagedSerializableTypeInfoList> listTypeInfo = std::static_pointer_cast<ManagedSerializableTypeInfoList>(nativeInstance->mTypeInfo);
		return ManagedSerializableList::createManagedInstance(listTypeInfo, size);
	}

	MonoObject* ScriptSerializableProperty::internal_createManagedDictionaryInstance(ScriptSerializableProperty* nativeInstance)
	{
		SPtr<ManagedSerializableTypeInfoDictionary> dictTypeInfo = std::static_pointer_cast<ManagedSerializableTypeInfoDictionary>(nativeInstance->mTypeInfo);
		return ManagedSerializableDictionary::createManagedInstance(dictTypeInfo);
	}
}
