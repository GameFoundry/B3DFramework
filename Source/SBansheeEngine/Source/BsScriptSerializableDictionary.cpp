//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsScriptSerializableDictionary.h"
#include "BsScriptMeta.h"
#include "BsMonoField.h"
#include "BsMonoClass.h"
#include "BsMonoManager.h"
#include "BsMonoUtil.h"
#include "BsScriptAssemblyManager.h"
#include "BsManagedSerializableObjectInfo.h"
#include "BsScriptSerializableProperty.h"

namespace bs
{
	ScriptSerializableDictionary::ScriptSerializableDictionary(MonoObject* instance, const SPtr<ManagedSerializableTypeInfoDictionary>& typeInfo)
		:ScriptObject(instance), mTypeInfo(typeInfo)
	{

	}

	void ScriptSerializableDictionary::initRuntimeData()
	{
		metaData.scriptClass->addInternalCall("Internal_CreateKeyProperty", &ScriptSerializableDictionary::internal_createKeyProperty);
		metaData.scriptClass->addInternalCall("Internal_CreateValueProperty", &ScriptSerializableDictionary::internal_createValueProperty);
	}

	ScriptSerializableDictionary* ScriptSerializableDictionary::create(const ScriptSerializableProperty* parentProperty)
	{
		SPtr<ManagedSerializableTypeInfoDictionary> dictTypeInfo = std::static_pointer_cast<ManagedSerializableTypeInfoDictionary>(parentProperty->getTypeInfo());

		MonoReflectionType* internalKeyType = MonoUtil::getType(dictTypeInfo->mKeyType->getMonoClass());
		MonoReflectionType* internalValueType = MonoUtil::getType(dictTypeInfo->mValueType->getMonoClass());

		void* params[3] = { internalKeyType, internalValueType, parentProperty->getManagedInstance() };
		MonoObject* managedInstance = metaData.scriptClass->createInstance(params, 3);

		ScriptSerializableDictionary* nativeInstance = new (bs_alloc<ScriptSerializableDictionary>()) ScriptSerializableDictionary(managedInstance, dictTypeInfo);

		return nativeInstance;
	}

	MonoObject* ScriptSerializableDictionary::internal_createKeyProperty(ScriptSerializableDictionary* nativeInstance)
	{
		ScriptSerializableProperty* newProperty = ScriptSerializableProperty::create(nativeInstance->mTypeInfo->mKeyType);

		return newProperty->getManagedInstance();
	}

	MonoObject* ScriptSerializableDictionary::internal_createValueProperty(ScriptSerializableDictionary* nativeInstance)
	{
		ScriptSerializableProperty* newProperty = ScriptSerializableProperty::create(nativeInstance->mTypeInfo->mValueType);

		return newProperty->getManagedInstance();
	}
}