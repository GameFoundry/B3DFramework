//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "Wrappers/BsScriptSerializableList.h"
#include "BsScriptMeta.h"
#include "BsMonoField.h"
#include "BsMonoClass.h"
#include "BsMonoManager.h"
#include "Serialization/BsScriptAssemblyManager.h"
#include "Serialization/BsManagedSerializableObjectInfo.h"
#include "Wrappers/BsScriptSerializableProperty.h"
#include "BsMonoUtil.h"

namespace bs
{
	ScriptSerializableList::ScriptSerializableList(MonoObject* instance, const SPtr<ManagedSerializableTypeInfoList>& typeInfo)
		:ScriptObject(instance), mTypeInfo(typeInfo)
	{

	}

	void ScriptSerializableList::initRuntimeData()
	{
		metaData.scriptClass->addInternalCall("Internal_CreateProperty", (void*)&ScriptSerializableList::internal_createProperty);
	}

	MonoObject* ScriptSerializableList::create(const ScriptSerializableProperty* native, MonoObject* managed)
	{
		SPtr<ManagedSerializableTypeInfoList> listTypeInfo = 
			std::static_pointer_cast<ManagedSerializableTypeInfoList>(native->getTypeInfo());
		MonoReflectionType* internalElementType = MonoUtil::getType(listTypeInfo->mElementType->getMonoClass());

		void* params[2] = { internalElementType, managed };
		MonoObject* managedInstance = metaData.scriptClass->createInstance(params, 2);

		new (bs_alloc<ScriptSerializableList>()) ScriptSerializableList(managedInstance, listTypeInfo);
		return managedInstance;
	}

	MonoObject* ScriptSerializableList::internal_createProperty(ScriptSerializableList* nativeInstance)
	{
		return ScriptSerializableProperty::create(nativeInstance->mTypeInfo->mElementType);
	}
}