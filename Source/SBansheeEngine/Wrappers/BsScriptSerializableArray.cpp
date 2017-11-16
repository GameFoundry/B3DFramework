//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "Wrappers/BsScriptSerializableArray.h"
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
	ScriptSerializableArray::ScriptSerializableArray(MonoObject* instance, const SPtr<ManagedSerializableTypeInfoArray>& typeInfo)
		:ScriptObject(instance), mTypeInfo(typeInfo)
	{

	}

	void ScriptSerializableArray::initRuntimeData()
	{
		metaData.scriptClass->addInternalCall("Internal_CreateProperty", (void*)&ScriptSerializableArray::internal_createProperty);
	}

	MonoObject* ScriptSerializableArray::create(const ScriptSerializableProperty* native, MonoObject* managed)
	{
		SPtr<ManagedSerializableTypeInfoArray> arrayTypeInfo = 
			std::static_pointer_cast<ManagedSerializableTypeInfoArray>(native->getTypeInfo());

		MonoReflectionType* internalElementType = MonoUtil::getType(arrayTypeInfo->mElementType->getMonoClass());

		void* params[2] = { internalElementType, managed };
		MonoObject* managedInstance = metaData.scriptClass->createInstance(params, 2);

		new (bs_alloc<ScriptSerializableArray>()) ScriptSerializableArray(managedInstance, arrayTypeInfo);

		return managedInstance;
	}

	MonoObject* ScriptSerializableArray::internal_createProperty(ScriptSerializableArray* nativeInstance)
	{
		return ScriptSerializableProperty::create(nativeInstance->mTypeInfo->mElementType);
	}
}