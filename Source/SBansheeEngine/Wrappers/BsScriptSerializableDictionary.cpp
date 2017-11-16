//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
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
		:ScriptObject(instance), mTypeInfo(typeInfo)
	{

	}

	void ScriptSerializableDictionary::initRuntimeData()
	{
		metaData.scriptClass->addInternalCall("Internal_CreateKeyProperty", (void*)&ScriptSerializableDictionary::internal_createKeyProperty);
		metaData.scriptClass->addInternalCall("Internal_CreateValueProperty", (void*)&ScriptSerializableDictionary::internal_createValueProperty);
	}

	MonoObject* ScriptSerializableDictionary::create(const ScriptSerializableProperty* native, MonoObject* managed)
	{
		SPtr<ManagedSerializableTypeInfoDictionary> dictTypeInfo = 
			std::static_pointer_cast<ManagedSerializableTypeInfoDictionary>(native->getTypeInfo());

		MonoReflectionType* internalKeyType = MonoUtil::getType(dictTypeInfo->mKeyType->getMonoClass());
		MonoReflectionType* internalValueType = MonoUtil::getType(dictTypeInfo->mValueType->getMonoClass());

		void* params[3] = { internalKeyType, internalValueType, managed };
		MonoObject* managedInstance = metaData.scriptClass->createInstance(params, 3);

		new (bs_alloc<ScriptSerializableDictionary>()) ScriptSerializableDictionary(managedInstance, dictTypeInfo);
		return managedInstance;
	}

	MonoObject* ScriptSerializableDictionary::internal_createKeyProperty(ScriptSerializableDictionary* nativeInstance)
	{
		return ScriptSerializableProperty::create(nativeInstance->mTypeInfo->mKeyType);
	}

	MonoObject* ScriptSerializableDictionary::internal_createValueProperty(ScriptSerializableDictionary* nativeInstance)
	{
		return ScriptSerializableProperty::create(nativeInstance->mTypeInfo->mValueType);
	}
}