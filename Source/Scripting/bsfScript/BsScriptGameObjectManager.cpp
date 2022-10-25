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
		: Instance(instance), IsComponent(isComponent)
	{}

	ScriptGameObjectManager::ScriptGameObjectManager()
	{
		// Calls OnReset on all components after assembly reload happens
		mOnAssemblyReloadDoneConn = ScriptObjectManager::Instance().OnRefreshComplete.Connect(
			std::bind(&::bs::ScriptGameObjectManager::SendComponentResetEvents, this));

		onGameObjectDestroyedConn = GameObjectManager::Instance().OnDestroyed.Connect(
			std::bind(&::bs::ScriptGameObjectManager::OnGameObjectDestroyed, this, _1));
	}

	ScriptGameObjectManager::~ScriptGameObjectManager()
	{
		mOnAssemblyReloadDoneConn.Disconnect();
		onGameObjectDestroyedConn.Disconnect();
	}

	ScriptSceneObject* ScriptGameObjectManager::GetOrCreateScriptSceneObject(const HSceneObject& sceneObject)
	{
		ScriptSceneObject* so = GetScriptSceneObject(sceneObject);
		if(so != nullptr)
			return so;

		return CreateScriptSceneObject(sceneObject);
	}

	ScriptSceneObject* ScriptGameObjectManager::CreateScriptSceneObject(const HSceneObject& sceneObject)
	{
		MonoClass* sceneObjectClass = ScriptAssemblyManager::Instance().GetBuiltinClasses().SceneObjectClass;
		MonoObject* instance = sceneObjectClass->CreateInstance();

		return CreateScriptSceneObject(instance, sceneObject);
	}

	ScriptSceneObject* ScriptGameObjectManager::CreateScriptSceneObject(MonoObject* existingInstance, const HSceneObject& sceneObject)
	{
		ScriptSceneObject* so = GetScriptSceneObject(sceneObject);
		if(so != nullptr)
			BS_EXCEPT(InvalidStateException, "Script object for this SceneObject already exists.");

		ScriptSceneObject* nativeInstance = new(bs_alloc<ScriptSceneObject>()) ScriptSceneObject(existingInstance, sceneObject);
		mScriptSceneObjects[sceneObject.GetInstanceId()] = nativeInstance;

		return nativeInstance;
	}

	ScriptManagedComponent* ScriptGameObjectManager::CreateManagedScriptComponent(MonoObject* existingInstance, const HManagedComponent& component)
	{
		ScriptManagedComponent* nativeInstance = new(bs_alloc<ScriptManagedComponent>())
			ScriptManagedComponent(existingInstance, component);

		u64 instanceId = component->GetInstanceId();
		mScriptComponents[instanceId] = nativeInstance;

		return nativeInstance;
	}

	ScriptComponentBase* ScriptGameObjectManager::CreateBuiltinScriptComponent(const HComponent& component)
	{
		u32 rttiId = component->GetRtti()->GetRttiId();
		BuiltinComponentInfo* info = ScriptAssemblyManager::Instance().GetBuiltinComponentInfo(rttiId);

		if(info == nullptr)
			return nullptr;

		ScriptComponentBase* nativeInstance = info->CreateCallback(component);
		nativeInstance->SetNativeHandle(static_object_cast<GameObject>(component));

		u64 instanceId = component->GetInstanceId();
		mScriptComponents[instanceId] = nativeInstance;

		return nativeInstance;
	}

	ScriptComponentBase* ScriptGameObjectManager::GetBuiltinScriptComponent(const HComponent& component, bool createNonExisting)
	{
		ScriptComponentBase* scriptComponent = GetScriptComponent(component.GetInstanceId());
		if(scriptComponent != nullptr)
			return scriptComponent;

		if(createNonExisting)
			return CreateBuiltinScriptComponent(component);

		return nullptr;
	}

	ScriptManagedComponent* ScriptGameObjectManager::GetManagedScriptComponent(const HManagedComponent& component) const
	{
		auto findIter = mScriptComponents.find(component.GetInstanceId());
		if(findIter != mScriptComponents.end())
			return static_cast<ScriptManagedComponent*>(findIter->second);

		return nullptr;
	}

	ScriptComponentBase* ScriptGameObjectManager::GetScriptComponent(u64 instanceId) const
	{
		auto findIter = mScriptComponents.find(instanceId);
		if(findIter != mScriptComponents.end())
			return findIter->second;

		return nullptr;
	}

	ScriptSceneObject* ScriptGameObjectManager::GetScriptSceneObject(const HSceneObject& sceneObject) const
	{
		auto findIter = mScriptSceneObjects.find(sceneObject.GetInstanceId());
		if(findIter != mScriptSceneObjects.end())
			return findIter->second;

		return nullptr;
	}

	ScriptSceneObject* ScriptGameObjectManager::GetScriptSceneObject(u64 instanceId) const
	{
		auto findIter = mScriptSceneObjects.find(instanceId);
		if(findIter != mScriptSceneObjects.end())
			return findIter->second;

		return nullptr;
	}

	ScriptGameObjectBase* ScriptGameObjectManager::GetScriptGameObject(u64 instanceId) const
	{
		auto findIter = mScriptSceneObjects.find(instanceId);
		if(findIter != mScriptSceneObjects.end())
			return findIter->second;

		auto findIter2 = mScriptComponents.find(instanceId);
		if(findIter2 != mScriptComponents.end())
			return findIter2->second;

		return nullptr;
	}

	void ScriptGameObjectManager::DestroyScriptSceneObject(ScriptSceneObject* sceneObject)
	{
		u64 instanceId = sceneObject->GetNativeHandle().GetInstanceId();
		mScriptSceneObjects.erase(instanceId);

		bs_delete(sceneObject);
	}

	void ScriptGameObjectManager::DestroyScriptComponent(ScriptComponentBase* component)
	{
		u64 instanceId = component->GetNativeHandle().GetInstanceId();
		mScriptComponents.erase(instanceId);

		bs_delete(component);
	}

	void ScriptGameObjectManager::SendComponentResetEvents()
	{
		for(auto& scriptObjectEntry : mScriptComponents)
		{
			ScriptComponentBase* scriptComponent = scriptObjectEntry.second;
			HComponent component = scriptComponent->GetComponent();

			if(component->GetRtti()->GetRttiId() == TID_ManagedComponent)
			{
				HManagedComponent managedComponent = static_object_cast<ManagedComponent>(component);
				if(!managedComponent.IsDestroyed())
					managedComponent->TriggerOnReset();
			}
		}
	}

	void ScriptGameObjectManager::OnGameObjectDestroyed(const HGameObject& go)
	{
		u64 instanceId = go.GetInstanceId();

		ScriptSceneObject* so = GetScriptSceneObject(instanceId);
		if(so != nullptr)
		{
			so->NotifyDestroyedInternal();
			mScriptSceneObjects.erase(instanceId);
		}

		ScriptComponentBase* component = GetScriptComponent(instanceId);
		if(component != nullptr)
		{
			component->NotifyDestroyedInternal();
			mScriptComponents.erase(instanceId);
		}
	}
} // namespace bs
