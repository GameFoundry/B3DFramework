//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "Wrappers/BsScriptSerializedDiff.h"
#include "BsScriptMeta.h"
#include "BsMonoClass.h"
#include "Serialization/BsManagedSerializableObject.h"
#include "Wrappers/BsScriptSerializedObject.h"
#include "Serialization/BsManagedSerializableDiff.h"

namespace bs
{
	ScriptSerializedDiff::ScriptSerializedDiff(MonoObject* instance, const SPtr<ManagedSerializableDiff>& obj)
		: ScriptObject(instance), mSerializedDiff(obj)
	{

	}

	void ScriptSerializedDiff::initRuntimeData()
	{
		metaData.scriptClass->addInternalCall("Internal_CreateDiff", (void*)&ScriptSerializedDiff::internal_CreateDiff);
		metaData.scriptClass->addInternalCall("Internal_ApplyDiff", (void*)&ScriptSerializedDiff::internal_ApplyDiff);
	}

	MonoObject* ScriptSerializedDiff::internal_CreateDiff(ScriptSerializedObject* oldObj, MonoObject* newObj)
	{
		SPtr<ManagedSerializableObject> oldSerializedObject = oldObj->getInternal();
		SPtr<ManagedSerializableObject> newSerializedObject = ManagedSerializableObject::createFromExisting(newObj);

		if (oldSerializedObject == nullptr || newSerializedObject == nullptr)
			return nullptr;

		oldSerializedObject->serialize();
		newSerializedObject->serialize();

		SPtr<ManagedSerializableDiff> diff = ManagedSerializableDiff::create(oldSerializedObject, newSerializedObject);

		MonoObject* instance = metaData.scriptClass->createInstance();
		new (bs_alloc<ScriptSerializedDiff>()) ScriptSerializedDiff(instance, diff);

		return instance;
	}

	void ScriptSerializedDiff::internal_ApplyDiff(ScriptSerializedDiff* thisPtr, MonoObject* obj)
	{
		SPtr<ManagedSerializableObject> serializedObject = ManagedSerializableObject::createFromExisting(obj);
		if (serializedObject == nullptr)
			return;

		thisPtr->mSerializedDiff->apply(serializedObject);
	}
}