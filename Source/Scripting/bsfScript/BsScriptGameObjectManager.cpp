//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptGameObjectManager.h"
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
ScriptGameObjectManager::ScriptGameObjectManager()
{
	// Calls OnReset on all components after assembly reload happens
	mOnAssemblyReloadDoneConn = ScriptObjectManager::Instance().OnRefreshComplete.Connect(
		std::bind(&::bs::ScriptGameObjectManager::SendComponentResetEvents, this));
}

ScriptGameObjectManager::~ScriptGameObjectManager()
{
	mOnAssemblyReloadDoneConn.Disconnect();
}

ScriptManagedComponent* ScriptGameObjectManager::CreateManagedScriptComponent(MonoObject* existingInstance, const HManagedComponent& component)
{


	return nativeInstance;
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
