//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "Wrappers/BsScriptSerializedObject.h"
#include "BsScriptMeta.h"
#include "BsMonoClass.h"
#include "Wrappers/BsScriptManagedComponent.h"
#include "Wrappers/BsScriptManagedResource.h"
#include "BsManagedComponent.h"
#include "BsManagedResource.h"
#include "Serialization/BsManagedSerializableObject.h"
#include "Serialization/BsScriptAssemblyManager.h"
#include "Reflection/BsRTTIType.h"

namespace bs
{
	ScriptSerializedObject::ScriptSerializedObject(MonoObject* instance, const SPtr<ManagedSerializableObject>& obj)
		: ScriptObject(instance), mSerializedObject(obj)
	{

	}

	void ScriptSerializedObject::initRuntimeData()
	{
		metaData.scriptClass->addInternalCall("Internal_CreateComponent", (void*)&ScriptSerializedObject::internal_CreateComponent);
		metaData.scriptClass->addInternalCall("Internal_CreateResource", (void*)&ScriptSerializedObject::internal_CreateResource);
		metaData.scriptClass->addInternalCall("Internal_CreateGeneric", (void*)&ScriptSerializedObject::internal_CreateGeneric);
		metaData.scriptClass->addInternalCall("Internal_Deserialize", (void*)&ScriptSerializedObject::internal_Deserialize);
	}

	MonoObject* ScriptSerializedObject::internal_CreateComponent(ScriptManagedComponent* componentPtr)
	{
		HManagedComponent component = static_object_cast<ManagedComponent>(componentPtr->getHandle());
		if (component.isDestroyed())
			return nullptr;

		MonoObject* managedInstance = component->getManagedInstance();
		SPtr<ManagedSerializableObject> serializedObject = ManagedSerializableObject::createFromExisting(managedInstance);
		if (serializedObject == nullptr)
			return nullptr;

		serializedObject->serialize();

		MonoObject* instance = metaData.scriptClass->createInstance();
		new (bs_alloc<ScriptSerializedObject>()) ScriptSerializedObject(instance, serializedObject);

		return instance;
	}

	MonoObject* ScriptSerializedObject::internal_CreateResource(ScriptManagedResource* resourcePtr)
	{
		HManagedResource resource = resourcePtr->getHandle();
		if (!resource.isLoaded())
			return nullptr;

		MonoObject* managedInstance = resource->getManagedInstance();
		SPtr<ManagedSerializableObject> serializedObject = ManagedSerializableObject::createFromExisting(managedInstance);
		if (serializedObject == nullptr)
			return nullptr;

		serializedObject->serialize();

		MonoObject* instance = metaData.scriptClass->createInstance();
		new (bs_alloc<ScriptSerializedObject>()) ScriptSerializedObject(instance, serializedObject);

		return instance;
	}

	MonoObject* ScriptSerializedObject::internal_CreateGeneric(MonoObject* obj)
	{
		if (obj == nullptr)
			return nullptr;

		SPtr<ManagedSerializableObject> serializedObject = ManagedSerializableObject::createFromExisting(obj);
		if (serializedObject == nullptr)
			return nullptr;

		serializedObject->serialize();

		MonoObject* instance = metaData.scriptClass->createInstance();
		new (bs_alloc<ScriptSerializedObject>()) ScriptSerializedObject(instance, serializedObject);

		return instance;
	}

	MonoObject* ScriptSerializedObject::internal_Deserialize(ScriptSerializedObject* thisPtr)
	{
		SPtr<ManagedSerializableObject> serializedObject = thisPtr->mSerializedObject;
		if (serializedObject == nullptr)
			return nullptr;

		return serializedObject->deserialize();
	}
}