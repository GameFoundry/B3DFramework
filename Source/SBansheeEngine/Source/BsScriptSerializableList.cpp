//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsScriptSerializableList.h"
#include "BsScriptMeta.h"
#include "BsMonoField.h"
#include "BsMonoClass.h"
#include "BsMonoManager.h"
#include "BsScriptAssemblyManager.h"
#include "BsManagedSerializableObjectInfo.h"
#include "BsScriptSerializableProperty.h"
#include "BsMonoUtil.h"

namespace bs
{
	ScriptSerializableList::ScriptSerializableList(MonoObject* instance, const SPtr<ManagedSerializableTypeInfoList>& typeInfo)
		:ScriptObject(instance), mTypeInfo(typeInfo)
	{

	}

	void ScriptSerializableList::initRuntimeData()
	{
		metaData.scriptClass->addInternalCall("Internal_CreateProperty", &ScriptSerializableList::internal_createProperty);
	}

	ScriptSerializableList* ScriptSerializableList::create(const ScriptSerializableProperty* parentProperty)
	{
		SPtr<ManagedSerializableTypeInfoList> listTypeInfo = std::static_pointer_cast<ManagedSerializableTypeInfoList>(parentProperty->getTypeInfo());
		MonoReflectionType* internalElementType = MonoUtil::getType(listTypeInfo->mElementType->getMonoClass());

		void* params[2] = { internalElementType, parentProperty->getManagedInstance() };
		MonoObject* managedInstance = metaData.scriptClass->createInstance(params, 2);

		ScriptSerializableList* nativeInstance = new (bs_alloc<ScriptSerializableList>()) ScriptSerializableList(managedInstance, listTypeInfo);

		return nativeInstance;
	}

	MonoObject* ScriptSerializableList::internal_createProperty(ScriptSerializableList* nativeInstance)
	{
		ScriptSerializableProperty* newProperty = ScriptSerializableProperty::create(nativeInstance->mTypeInfo->mElementType);

		return newProperty->getManagedInstance();
	}
}