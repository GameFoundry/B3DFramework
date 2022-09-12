//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptGameObjectManager.h"
#include "Wrappers/BsScriptGameObject.h"
#include "Wrappers/BsScriptComponent.h"
#include "Wrappers/BsScriptManagedComponent.h"
#include "Wrappers/BsScriptSceneObject.h"
#include "Scene/BsGameObjectManager.h"
#include "Scene/BsGameObject.h"
#include "Scene/BsComponent.h"
#include "BsManagedComponent.h"
#include "Scene/BsSceneObject.h"
#include "BsMonoManager.h"
#include "BsMonoAssembly.h"
#include "BsMonoClass.h"
#include "Serialization/BsScriptAssemblyManager.h"
#include "BsScriptObjectManager.h"

using namespace std::placeholders;

namespace bs
{
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
		mOnAssemblyReloadDoneConn.Disconnect();
		onGameObjectDestroyedConn.Disconnect();
	}

	ScriptSceneObject* ScriptGameObjectManager::getOrCreateScriptSceneObject(const HSceneObject& sceneObject)
	{
		ScriptSceneObject* so = getScriptSceneObject(sceneObject);
		if (so != nullptr)
			return so;

		return CreateScriptSceneObject(sceneObject);
	}

	ScriptSceneObject* ScriptGameObjectManager::createScriptSceneObject(const HSceneObject& sceneObject)
	{
		MonoClass* sceneObjectClass = ScriptAssemblyManager::instance().GetBuiltinClasses().sceneObjectClass;
		MonoObject* instance = sceneObjectClass->CreateInstance();

		return CreateScriptSceneObject(instance, sceneObject);
	}

	ScriptSceneObject* ScriptGameObjectManager::createScriptSceneObject(MonoObject* existingInstance, const HSceneObject& sceneObject)
	{
		ScriptSceneObject* so = getScriptSceneObject(sceneObject);
		if (so != nullptr)
			BS_EXCEPT(InvalidStateException, "Script object for this SceneObject already exists.");

		ScriptSceneObject* nativeInstance = new (bs_alloc<ScriptSceneObject>()) ScriptSceneObject(existingInstance, sceneObject);
		mScriptSceneObjects[sceneObject.GetInstanceId()] = nativeInstance;

		return nativeInstance;
	}

	ScriptManagedComponent* ScriptGameObjectManager::createManagedScriptComponent(MonoObject* existingInstance,
																				  const HManagedComponent& component)
	{
		ScriptManagedComponent* nativeInstance = new (bs_alloc<ScriptManagedComponent>())
			ScriptManagedComponent(existingInstance, component);

		UINT64 instanceId = component->GetInstanceId();
		mScriptComponents[instanceId] = nativeInstance;

		return nativeInstance;
	}

	ScriptComponentBase* ScriptGameObjectManager::createBuiltinScriptComponent(const HComponent& component)
	{
		UINT32 rttiId = component->GetRTTI()->getRTTIId();
		BuiltinComponentInfo* info = ScriptAssemblyManager::instance().GetBuiltinComponentInfo(rttiId);

		if (info == nullptr)
			return nullptr;

		ScriptComponentBase* nativeInstance = info->CreateCallback(component);
		nativeInstance->SetNativeHandle(static_object_cast<GameObject>(component));

		UINT64 instanceId = component->GetInstanceId();
		mScriptComponents[instanceId] = nativeInstance;

		return nativeInstance;
	}

	ScriptComponentBase* ScriptGameObjectManager::getBuiltinScriptComponent(const HComponent& component, bool createNonExisting)
	{
		ScriptComponentBase* scriptComponent = getScriptComponent(component.GetInstanceId());
		if (scriptComponent != nullptr)
			return scriptComponent;

		if(createNonExisting)
			return CreateBuiltinScriptComponent(component);

		return nullptr;
	}

	ScriptManagedComponent* ScriptGameObjectManager::getManagedScriptComponent(const HManagedComponent& component) const
	{
		auto findIter = mScriptComponents.Find(component.getInstanceId());
		if (findIter != mScriptComponents.End())
			return static_cast<ScriptManagedComponent*>(findIter->second);

		return nullptr;
	}

	ScriptComponentBase* ScriptGameObjectManager::getScriptComponent(UINT64 instanceId) const
	{
		auto findIter = mScriptComponents.Find(instanceId);
		if (findIter != mScriptComponents.End())
			return findIter->second;

		return nullptr;
	}

	ScriptSceneObject* ScriptGameObjectManager::getScriptSceneObject(const HSceneObject& sceneObject) const
	{
		auto findIter = mScriptSceneObjects.Find(sceneObject.getInstanceId());
		if (findIter != mScriptSceneObjects.End())
			return findIter->second;

		return nullptr;
	}

	ScriptSceneObject* ScriptGameObjectManager::getScriptSceneObject(UINT64 instanceId) const
	{
		auto findIter = mScriptSceneObjects.Find(instanceId);
		if (findIter != mScriptSceneObjects.End())
			return findIter->second;

		return nullptr;
	}

	ScriptGameObjectBase* ScriptGameObjectManager::getScriptGameObject(UINT64 instanceId) const
	{
		auto findIter = mScriptSceneObjects.Find(instanceId);
		if (findIter != mScriptSceneObjects.End())
			return findIter->second;

		auto findIter2 = mScriptComponents.Find(instanceId);
		if (findIter2 != mScriptComponents.End())
			return findIter2->second;

		return nullptr;
	}

	void ScriptGameObjectManager::DestroyScriptSceneObject(ScriptSceneObject* sceneObject)
	{
		UINT64 instanceId = sceneObject->GetNativeHandle().GetInstanceId();
		mScriptSceneObjects.Erase(instanceId);

		bs_delete(sceneObject);
	}

	void ScriptGameObjectManager::DestroyScriptComponent(ScriptComponentBase* component)
	{
		UINT64 instanceId = component->GetNativeHandle().GetInstanceId();
		mScriptComponents.Erase(instanceId);

		bs_delete(component);
	}

	void ScriptGameObjectManager::SendComponentResetEvents()
	{
		for (auto& scriptObjectEntry : mScriptComponents)
		{
			ScriptComponentBase* scriptComponent = scriptObjectEntry.second;
			HComponent component = scriptComponent->GetComponent();

			if (component->GetRTTI()->getRTTIId() == TID_ManagedComponent)
			{
				HManagedComponent managedComponent = static_object_cast<ManagedComponent>(component);
				if (!managedComponent.IsDestroyed())
					managedComponent->TriggerOnReset();
			}
		}
	}

	void ScriptGameObjectManager::OnGameObjectDestroyed(const HGameObject& go)
	{
		UINT64 instanceId = go.GetInstanceId();

		ScriptSceneObject* so = getScriptSceneObject(instanceId);
		if (so != nullptr)
		{
			so->_notifyDestroyed();
			mScriptSceneObjects.Erase(instanceId);
		}

		ScriptComponentBase* component = getScriptComponent(instanceId);
		if(component != nullptr)
		{
			component->_notifyDestroyed();
			mScriptComponents.Erase(instanceId);
		}
	}
}
