//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsScriptSerializedObject.h"
#include "BsScriptMeta.h"
#include "BsMonoClass.h"
#include "BsScriptComponent.h"
#include "BsScriptManagedResource.h"
#include "BsManagedComponent.h"
#include "BsManagedResource.h"
#include "BsManagedSerializableObject.h"
#include "BsScriptAssemblyManager.h"

namespace BansheeEngine
{
	ScriptSerializedObject::ScriptSerializedObject(MonoObject* instance, const SPtr<ManagedSerializableObject>& obj)
		: ScriptObject(instance), mSerializedObject(obj)
	{

	}

	void ScriptSerializedObject::initRuntimeData()
	{
		metaData.scriptClass->addInternalCall("Internal_CreateComponent", &ScriptSerializedObject::internal_CreateComponent);
		metaData.scriptClass->addInternalCall("Internal_CreateResource", &ScriptSerializedObject::internal_CreateResource);
		metaData.scriptClass->addInternalCall("Internal_CreateGeneric", &ScriptSerializedObject::internal_CreateGeneric);
		metaData.scriptClass->addInternalCall("Internal_Deserialize", &ScriptSerializedObject::internal_Deserialize);
	}

	MonoObject* ScriptSerializedObject::internal_CreateComponent(ScriptComponent* componentPtr)
	{
		HManagedComponent component = componentPtr->getHandle();
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

		serializedObject->deserialize();
		MonoObject* managedInstance = serializedObject->getManagedInstance();

		// Note: Technically I could just clear managed data without a full serialization since I know nothing has changed
		serializedObject->serialize(); 

		return managedInstance;
	}
}