//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsScriptSerializableProperty.h"
#include "BsScriptMeta.h"
#include "BsMonoField.h"
#include "BsMonoClass.h"
#include "BsMonoManager.h"
#include "BsMonoUtil.h"
#include "BsManagedSerializableObjectInfo.h"
#include "BsScriptSerializableObject.h"
#include "BsScriptSerializableArray.h"
#include "BsScriptSerializableList.h"
#include "BsScriptSerializableDictionary.h"
#include "BsScriptAssemblyManager.h"
#include "BsManagedSerializableObject.h"
#include "BsManagedSerializableArray.h"
#include "BsManagedSerializableList.h"
#include "BsManagedSerializableDictionary.h"
#include "BsManagedSerializableField.h"
#include "BsMemorySerializer.h"

namespace bs
{
	ScriptSerializableProperty::ScriptSerializableProperty(MonoObject* instance, const SPtr<ManagedSerializableTypeInfo>& typeInfo)
		:ScriptObject(instance), mTypeInfo(typeInfo)
	{

	}

	void ScriptSerializableProperty::initRuntimeData()
	{
		metaData.scriptClass->addInternalCall("Internal_CreateInstance", &ScriptSerializableProperty::internal_CreateInstance);
		metaData.scriptClass->addInternalCall("Internal_CreateObject", &ScriptSerializableProperty::internal_createObject);
		metaData.scriptClass->addInternalCall("Internal_CreateArray", &ScriptSerializableProperty::internal_createArray);
		metaData.scriptClass->addInternalCall("Internal_CreateList", &ScriptSerializableProperty::internal_createList);
		metaData.scriptClass->addInternalCall("Internal_CreateDictionary", &ScriptSerializableProperty::internal_createDictionary);
		metaData.scriptClass->addInternalCall("Internal_CreateManagedObjectInstance", &ScriptSerializableProperty::internal_createManagedObjectInstance);
		metaData.scriptClass->addInternalCall("Internal_CreateManagedArrayInstance", &ScriptSerializableProperty::internal_createManagedArrayInstance);
		metaData.scriptClass->addInternalCall("Internal_CreateManagedListInstance", &ScriptSerializableProperty::internal_createManagedListInstance);
		metaData.scriptClass->addInternalCall("Internal_CreateManagedDictionaryInstance", &ScriptSerializableProperty::internal_createManagedDictionaryInstance);
	}

	ScriptSerializableProperty* ScriptSerializableProperty::create(const SPtr<ManagedSerializableTypeInfo>& typeInfo)
	{
		MonoObject* managedInstance = metaData.scriptClass->createInstance();

		ScriptSerializableProperty* nativeInstance = new (bs_alloc<ScriptSerializableProperty>()) ScriptSerializableProperty(managedInstance, typeInfo);

		return nativeInstance;
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
			LOGWRN("Cannot create an instance of type \"" +
				engineClass->getFullName() + "\", it is not marked as serializable.");
			return;
		}

		new (bs_alloc<ScriptSerializableProperty>()) ScriptSerializableProperty(instance, typeInfo);
	}

	MonoObject* ScriptSerializableProperty::internal_createObject(ScriptSerializableProperty* nativeInstance)
	{
		ScriptSerializableObject* newObject = ScriptSerializableObject::create(nativeInstance);

		return newObject->getManagedInstance();
	}

	MonoObject* ScriptSerializableProperty::internal_createArray(ScriptSerializableProperty* nativeInstance)
	{
		ScriptSerializableArray* newObject = ScriptSerializableArray::create(nativeInstance);

		return newObject->getManagedInstance();
	}

	MonoObject* ScriptSerializableProperty::internal_createList(ScriptSerializableProperty* nativeInstance)
	{
		ScriptSerializableList* newObject = ScriptSerializableList::create(nativeInstance);

		return newObject->getManagedInstance();
	}

	MonoObject* ScriptSerializableProperty::internal_createDictionary(ScriptSerializableProperty* nativeInstance)
	{
		ScriptSerializableDictionary* newObject = ScriptSerializableDictionary::create(nativeInstance);

		return newObject->getManagedInstance();
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