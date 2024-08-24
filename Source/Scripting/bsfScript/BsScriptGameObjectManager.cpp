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

using namespace bs;
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

ScriptGameObjectBase* ScriptGameObjectManager::GetOrCreateScriptGameObject(const HGameObject& gameObject)
{
	if(B3DRTTIIsOfType<SceneObject>(gameObject.Get()))
		return GetOrCreateScriptSceneObject(B3DStaticGameObjectCast<SceneObject>(gameObject));

	const HComponent component = B3DStaticGameObjectCast<Component>(gameObject);
	if(B3DRTTIIsOfType<ManagedComponent>(component.Get()))
		return GetManagedScriptComponent(B3DStaticGameObjectCast<ManagedComponent>(component));

	return GetBuiltinScriptComponent(component);
}

ScriptManagedComponent* ScriptGameObjectManager::CreateManagedScriptComponent(MonoObject* existingInstance, const HManagedComponent& component)
{
	ScriptManagedComponent* const nativeInstance = new(B3DAllocate<ScriptManagedComponent>())
		ScriptManagedComponent(existingInstance, component);

	const UUID& id = component->GetId();
	mScriptComponents[id] = nativeInstance;

	return nativeInstance;
}

ScriptComponentBase* ScriptGameObjectManager::CreateBuiltinScriptComponent(const HComponent& component)
{
	u32 rttiId = component->GetRtti()->GetRttiId();
	BuiltinComponentInfo* info = ScriptAssemblyManager::Instance().GetBuiltinComponentInfo(rttiId);

	if(info == nullptr)
		return nullptr;

	ScriptComponentBase* nativeInstance = info->CreateCallback(component);
	nativeInstance->SetNativeHandle(B3DStaticGameObjectCast<GameObject>(component));

	const UUID& id = component->GetId();
	mScriptComponents[id] = nativeInstance;

	return nativeInstance;
}

ScriptComponentBase* ScriptGameObjectManager::GetBuiltinScriptComponent(const HComponent& component, bool createNonExisting)
{
	ScriptComponentBase* scriptComponent = GetScriptComponent(component.GetId());
	if(scriptComponent != nullptr)
		return scriptComponent;

	if(createNonExisting)
		return CreateBuiltinScriptComponent(component);

	return nullptr;
}

ScriptManagedComponent* ScriptGameObjectManager::GetManagedScriptComponent(const HManagedComponent& component) const
{
	auto findIter = mScriptComponents.find(component.GetId());
	if(findIter != mScriptComponents.end())
		return static_cast<ScriptManagedComponent*>(findIter->second);

	return nullptr;
}

ScriptComponentBase* ScriptGameObjectManager::GetScriptComponent(const UUID& id) const
{
	auto findIter = mScriptComponents.find(id);
	if(findIter != mScriptComponents.end())
		return findIter->second;

	return nullptr;
}

void ScriptGameObjectManager::DestroyScriptComponent(ScriptComponentBase* scriptComponent)
{
	const UUID& id = scriptComponent->GetNativeHandle().GetId();
	mScriptComponents.erase(id);

	B3DDelete(scriptComponent);
}

void ScriptGameObjectManager::SendComponentResetEvents()
{
	for(auto& scriptObjectEntry : mScriptComponents)
	{
		ScriptComponentBase* scriptComponent = scriptObjectEntry.second;
		HComponent component = scriptComponent->GetComponent();

		if(component->GetRtti()->GetRttiId() == TID_ManagedComponent)
		{
			HManagedComponent managedComponent = B3DStaticGameObjectCast<ManagedComponent>(component);
			if(!managedComponent.IsDestroyed())
				managedComponent->TriggerOnReset();
		}
	}
}

void ScriptGameObjectManager::OnGameObjectDestroyed(const HGameObject& go)
{
	const UUID& id = go.GetId();

	ScriptComponentBase* component = GetScriptComponent(id);
	if(component != nullptr)
	{
		component->NotifyDestroyedInternal();
		mScriptComponents.erase(id);
	}
}
