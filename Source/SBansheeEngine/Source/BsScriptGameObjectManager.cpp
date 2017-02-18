//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsScriptGameObjectManager.h"
#include "BsScriptGameObject.h"
#include "BsScriptComponent.h"
#include "BsScriptManagedComponent.h"
#include "BsScriptSceneObject.h"
#include "BsGameObjectManager.h"
#include "BsGameObject.h"
#include "BsComponent.h"
#include "BsManagedComponent.h"
#include "BsSceneObject.h"
#include "BsMonoManager.h"
#include "BsMonoAssembly.h"
#include "BsMonoClass.h"
#include "BsScriptAssemblyManager.h"
#include "BsScriptObjectManager.h"
#include "BsBuiltinComponentLookup.h"

using namespace std::placeholders;

namespace bs
{
	ScriptGameObjectManager::ScriptGameObjectEntry::ScriptGameObjectEntry()
		:instance(nullptr), isComponent(false)
	{ }

	ScriptGameObjectManager::ScriptGameObjectEntry::ScriptGameObjectEntry(ScriptGameObjectBase* instance, bool isComponent)
		:instance(instance), isComponent(isComponent)
	{ }

	ScriptGameObjectManager::ScriptGameObjectManager()
	{
		// Calls OnReset on all components after assembly reload happens
		mOnAssemblyReloadDoneConn = ScriptObjectManager::instance().onRefreshComplete.connect(
			std::bind(&ScriptGameObjectManager::sendComponentResetEvents, this));

		onGameObjectDestroyedConn = GameObjectManager::instance().onDestroyed.connect(
			std::bind(&ScriptGameObjectManager::onGameObjectDestroyed, this, _1));
	}

	ScriptGameObjectManager::~ScriptGameObjectManager()
	{
		mOnAssemblyReloadDoneConn.disconnect();
		onGameObjectDestroyedConn.disconnect();
	}

	ScriptSceneObject* ScriptGameObjectManager::getOrCreateScriptSceneObject(const HSceneObject& sceneObject)
	{
		ScriptSceneObject* so = getScriptSceneObject(sceneObject);
		if (so != nullptr)
			return so;

		return createScriptSceneObject(sceneObject);
	}

	ScriptSceneObject* ScriptGameObjectManager::createScriptSceneObject(const HSceneObject& sceneObject)
	{
		MonoClass* sceneObjectClass = ScriptAssemblyManager::instance().getSceneObjectClass();
		MonoObject* instance = sceneObjectClass->createInstance();

		return createScriptSceneObject(instance, sceneObject);
	}

	ScriptSceneObject* ScriptGameObjectManager::createScriptSceneObject(MonoObject* existingInstance, const HSceneObject& sceneObject)
	{
		ScriptSceneObject* so = getScriptSceneObject(sceneObject);
		if (so != nullptr)
			BS_EXCEPT(InvalidStateException, "Script object for this SceneObject already exists.");

		ScriptSceneObject* nativeInstance = new (bs_alloc<ScriptSceneObject>()) ScriptSceneObject(existingInstance, sceneObject);
		mScriptSceneObjects[sceneObject.getInstanceId()] = nativeInstance;

		return nativeInstance;
	}

	ScriptManagedComponent* ScriptGameObjectManager::createManagedScriptComponent(MonoObject* existingInstance,
																				  const HManagedComponent& component)
	{
		ScriptManagedComponent* nativeInstance = new (bs_alloc<ScriptManagedComponent>()) ScriptManagedComponent(existingInstance);
		nativeInstance->setNativeHandle(component);

		UINT64 instanceId = component->getInstanceId();
		mScriptComponents[instanceId] = nativeInstance;

		return nativeInstance;
	}

	ScriptComponentBase* ScriptGameObjectManager::createBuiltinScriptComponent(const HComponent& component)
	{
		UINT32 rttiId = component->getRTTI()->getRTTIId();
		BuiltinComponentInfo* info = ScriptAssemblyManager::instance().getBuiltinComponentInfo(rttiId);

		if (info == nullptr)
			return nullptr;

		ScriptComponentBase* nativeInstance = info->createCallback(component);
		nativeInstance->setNativeHandle(component);

		UINT64 instanceId = component->getInstanceId();
		mScriptComponents[instanceId] = nativeInstance;

		return nativeInstance;
	}

	ScriptComponentBase* ScriptGameObjectManager::getBuiltinScriptComponent(const HComponent& component)
	{
		ScriptComponentBase* scriptComponent = getScriptComponent(component.getInstanceId());
		if (scriptComponent != nullptr)
			return scriptComponent;

		return createBuiltinScriptComponent(component);
	}

	ScriptManagedComponent* ScriptGameObjectManager::getManagedScriptComponent(const HManagedComponent& component) const
	{
		auto findIter = mScriptComponents.find(component.getInstanceId());
		if (findIter != mScriptComponents.end())
			return static_cast<ScriptManagedComponent*>(findIter->second);

		return nullptr;
	}

	ScriptComponentBase* ScriptGameObjectManager::getScriptComponent(UINT64 instanceId) const
	{
		auto findIter = mScriptComponents.find(instanceId);
		if (findIter != mScriptComponents.end())
			return findIter->second;

		return nullptr;
	}

	ScriptSceneObject* ScriptGameObjectManager::getScriptSceneObject(const HSceneObject& sceneObject) const
	{
		auto findIter = mScriptSceneObjects.find(sceneObject.getInstanceId());
		if (findIter != mScriptSceneObjects.end())
			return findIter->second;

		return nullptr;
	}

	ScriptSceneObject* ScriptGameObjectManager::getScriptSceneObject(UINT64 instanceId) const
	{
		auto findIter = mScriptSceneObjects.find(instanceId);
		if (findIter != mScriptSceneObjects.end())
			return findIter->second;

		return nullptr;
	}

	ScriptGameObjectBase* ScriptGameObjectManager::getScriptGameObject(UINT64 instanceId) const
	{
		auto findIter = mScriptSceneObjects.find(instanceId);
		if (findIter != mScriptSceneObjects.end())
			return findIter->second;

		auto findIter2 = mScriptComponents.find(instanceId);
		if (findIter2 != mScriptComponents.end())
			return findIter2->second;

		return nullptr;
	}

	void ScriptGameObjectManager::destroyScriptSceneObject(ScriptSceneObject* sceneObject)
	{
		UINT64 instanceId = sceneObject->getNativeHandle().getInstanceId();
		mScriptSceneObjects.erase(instanceId);

		bs_delete(sceneObject);
	}

	void ScriptGameObjectManager::destroyScriptComponent(ScriptComponentBase* component)
	{
		UINT64 instanceId = component->getNativeHandle().getInstanceId();
		mScriptComponents.erase(instanceId);

		bs_delete(component);
	}

	void ScriptGameObjectManager::sendComponentResetEvents()
	{
		for (auto& scriptObjectEntry : mScriptComponents)
		{
			ScriptComponentBase* scriptComponent = scriptObjectEntry.second;
			HComponent component = scriptComponent->getComponent();

			if (component->getRTTI()->getRTTIId() == TID_ManagedComponent)
			{
				HManagedComponent managedComponent = static_object_cast<ManagedComponent>(component);
				if (!managedComponent.isDestroyed())
					managedComponent->triggerOnReset();
			}
		}
	}

	void ScriptGameObjectManager::onGameObjectDestroyed(const HGameObject& go)
	{
		UINT64 instanceId = go.getInstanceId();

		ScriptSceneObject* so = getScriptSceneObject(instanceId);
		if (so != nullptr)
		{
			so->_notifyDestroyed();
			mScriptSceneObjects.erase(instanceId);
		}

		ScriptComponentBase* component = getScriptComponent(instanceId);
		if(component != nullptr)
		{
			component->_notifyDestroyed();
			mScriptComponents.erase(instanceId);
		}
	}
}