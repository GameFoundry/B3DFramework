//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Scene/BsSceneManager.h"

#include "BsGameObjectCollection.h"
#include "Scene/BsSceneObject.h"
#include "Scene/BsComponent.h"
#include "Renderer/BsRenderable.h"
#include "Renderer/BsCamera.h"
#include "Renderer/BsLight.h"
#include "RenderAPI/BsViewport.h"
#include "Scene/BsGameObjectManager.h"
#include "RenderAPI/BsRenderTarget.h"
#include "Renderer/BsLightProbeVolume.h"
#include "Scene/BsSceneActor.h"
#include "Scene/BsPrefab.h"
#include "Physics/BsPhysics.h"

using namespace bs;

enum ListType
{
	NoList = 0,
	ActiveList = 1,
	InactiveList = 2,
	UninitializedList = 3
};

struct ScopeToggle
{
	ScopeToggle(bool& val)
		: val(val) { val = true; }

	~ScopeToggle() { val = false; }

private:
	bool& val;
};

SceneInstance::SceneInstance(ConstructPrivately dummy, const String& name, const HSceneObject& root, const UUID& associatedResourceId, const SPtr<PhysicsScene>& physicsScene)
	: mName(name), mRoot(root), mAssociatedResourceId(associatedResourceId), mPhysicsScene(physicsScene), mGameObjectCollection(root->GetOwnerCollection())
{}

SceneInstance::~SceneInstance()
{
	GetSceneManager().NotifySceneInstanceDestroyed(this);
}

void SceneInstance::SetRoot(const HSceneObject& newRoot)
{
	if(newRoot == nullptr)
		return;

	HSceneObject oldRoot = mRoot;
	const u32 childCount = oldRoot->GetChildCount();

	// Make sure to keep persistent objects
	{
		FrameScope frameScope;
		FrameVector<HSceneObject> toMove;
		for(u32 i = 0; i < childCount; i++)
		{
			HSceneObject child = oldRoot->GetChild(i);

			if(child->HasFlag(SceneObjectFlag::Persistent))
				toMove.push_back(child);
		}

		for(auto& entry : toMove)
			entry->SetParent(newRoot, false);
	}

	oldRoot->Destroy();
	mGameObjectCollection->DestroyQueuedObjects();

	mRoot = newRoot;
	mRoot->ClearParent();
	mRoot->SetScene(shared_from_this());
	mGameObjectCollection = newRoot->GetOwnerCollection().lock();
}

HSceneObject SceneInstance::CreateSceneObject(const String& name)
{
	HSceneObject newSceneObject = SceneObject::CreateInternal(mGameObjectCollection, name);
	newSceneObject->SetParent(mRoot, false);
	newSceneObject->Initialize();

	return newSceneObject;
}

SPtr<SceneInstance> SceneInstance::Create(const String& name)
{
	const SPtr<GameObjectCollection>& gameObjectCollection = GameObjectCollection::Create();
	HSceneObject root = SceneObject::CreateInternal(gameObjectCollection, "Root");

	SPtr<SceneInstance> sceneInstance = B3DMakeShared<SceneInstance>(ConstructPrivately(), name, root, UUID::kEmpty, GetPhysics().CreatePhysicsScene());
	root->SetScene(sceneInstance, false);

	SceneManager::Instance().NotifySceneInstanceCreated(sceneInstance);
	root->Initialize();

	return sceneInstance;
}

SPtr<SceneInstance> SceneInstance::Create(const String& name, const HSceneObject& root)
{
	return Create(name, root, UUID::kEmpty);
}

SPtr<SceneInstance> SceneInstance::Create(const String& name, const HSceneObject& root, const UUID& associatedResourceId)
{
	const SPtr<GameObjectCollection>& gameObjectCollection = root->GetOwnerCollection().lock();
	if(!B3D_ENSURE(gameObjectCollection != nullptr))
		return nullptr;

	SPtr<SceneInstance> sceneInstance = B3DMakeShared<SceneInstance>(ConstructPrivately(), name, root, associatedResourceId, GetPhysics().CreatePhysicsScene());
	root->SetScene(sceneInstance, true);

	SceneManager::Instance().NotifySceneInstanceCreated(sceneInstance);
	return sceneInstance;
}

SceneManager::SceneManager()
{
}

SceneManager::~SceneManager()
{
	mMainScene->mPhysicsScene = nullptr;

	if(mMainScene->mRoot != nullptr && !mMainScene->mRoot.IsDestroyed())
		mMainScene->mRoot->Destroy(true);

	// Clear strong references to SceneInstance before destructor exits, as their destructors may call into the SceneManager, so we must sure
	// data remains valid
	mMainScene = nullptr;
}

void SceneManager::OnStartUp()
{
	mMainScene = SceneInstance::Create("Main");
	mMainScene->mRoot->SetScene(mMainScene);
}

void SceneManager::ClearMainScene(bool forceAll)
{
	const u32 childCount = mMainScene->mRoot->GetChildCount();

	u32 childIndex = 0;
	for(u32 i = 0; i < childCount; i++)
	{
		HSceneObject child = mMainScene->mRoot->GetChild(childIndex);

		if(forceAll || !child->HasFlag(SceneObjectFlag::Persistent))
			child->Destroy();
		else
			childIndex++;
	}

	const SPtr<GameObjectCollection>& gameObjectCollection = mMainScene->GetGameObjectCollection();
	if(B3D_ENSURE(gameObjectCollection != nullptr))
		gameObjectCollection->DestroyQueuedObjects();

	UUID oldMainSceneResourceId = mMainScene->GetAssociatedResourceId();

	HSceneObject newRoot = SceneObject::CreateInternal(gameObjectCollection, "SceneRoot");
	mMainScene->SetRoot(newRoot);
	mMainScene->SetAssociatedResourceId(UUID::kEmpty);

	OnMainSceneUnloaded(oldMainSceneResourceId);
}

void SceneManager::LoadMainScene(const HPrefab& scene)
{
	HSceneObject root = scene->Instantiate(mMainScene, true);
	mMainScene->SetRoot(root);
	mMainScene->SetAssociatedResourceId(scene->GetId());

	OnMainSceneLoaded(scene->GetId());
}

void SceneManager::BindActorInternal(const SPtr<SceneActor>& actor, const HSceneObject& so)
{
	mBoundActors[actor.get()] = BoundActorData(actor, so);
	actor->UpdateStateInternal(*so, true);
}

void SceneManager::UnbindActorInternal(const SPtr<SceneActor>& actor)
{
	mBoundActors.erase(actor.get());
}

HSceneObject SceneManager::GetActorSOInternal(const SPtr<SceneActor>& actor) const
{
	auto iterFind = mBoundActors.find(actor.get());
	if(iterFind != mBoundActors.end())
		return iterFind->second.So;

	return HSceneObject();
}

void SceneManager::RegisterCameraInternal(const SPtr<Camera>& camera)
{
	mCameras[camera.get()] = camera;
}

void SceneManager::UnregisterCameraInternal(const SPtr<Camera>& camera)
{
	mCameras.erase(camera.get());

	auto iterFind = std::find_if(mMainCameras.begin(), mMainCameras.end(), [&](const SPtr<Camera>& x)
								 { return x == camera; });

	if(iterFind != mMainCameras.end())
		mMainCameras.erase(iterFind);
}

void SceneManager::NotifyMainCameraStateChangedInternal(const SPtr<Camera>& camera)
{
	auto iterFind = std::find_if(mMainCameras.begin(), mMainCameras.end(), [&](const SPtr<Camera>& entry)
								 { return entry == camera; });

	SPtr<Viewport> viewport = camera->GetViewport();
	if(camera->IsMain())
	{
		if(iterFind == mMainCameras.end())
			mMainCameras.push_back(mCameras[camera.get()]);

		viewport->SetTarget(mMainRT);
	}
	else
	{
		if(iterFind != mMainCameras.end())
			mMainCameras.erase(iterFind);

		if(viewport->GetTarget() == mMainRT)
			viewport->SetTarget(nullptr);
	}
}

void SceneManager::UpdateCoreObjectTransformsInternal()
{
	for(auto& entry : mBoundActors)
		entry.second.Actor->UpdateStateInternal(*entry.second.So);
}

SPtr<Camera> SceneManager::GetMainCamera() const
{
	if(mMainCameras.size() > 0)
		return mMainCameras[0];

	return nullptr;
}

HSceneObject SceneManager::GetMainCameraSceneObject() const
{
	SPtr<Camera> camera = GetMainCamera();
	if(camera == nullptr)
		return HSceneObject();

	return GetActorSOInternal(camera);
}

void SceneManager::SetMainRenderTarget(const SPtr<RenderTarget>& rt)
{
	if(mMainRT == rt)
		return;

	mMainRTResizedConn.Disconnect();

	if(rt != nullptr)
		mMainRTResizedConn = rt->OnResized.Connect(std::bind(&SceneManager::OnMainRenderTargetResized, this));

	mMainRT = rt;

	float aspect = 1.0f;
	if(rt != nullptr)
	{
		auto& rtProps = rt->GetProperties();
		aspect = rtProps.Width / (float)rtProps.Height;
	}

	for(auto& entry : mMainCameras)
	{
		entry->GetViewport()->SetTarget(rt);
		entry->SetAspectRatio(aspect);
	}
}

void SceneManager::SetComponentState(ComponentState state)
{
	if(mDisableStateChange)
	{
		B3D_LOG(Warning, Scene, "Component state cannot be changed from the calling locating. "
							   "Are you calling it from Component callbacks?");
		return;
	}

	if(mComponentState == state)
		return;

	ComponentState oldState = mComponentState;

	// Make sure to change the state before calling any callbacks, so callbacks can query the state
	mComponentState = state;

	// Make sure the per-state lists are up-to-date
	ProcessStateChanges();

	ScopeToggle toggle(mDisableStateChange);

	// Wake up all components with onInitialize/onEnable events if moving to running or paused state
	if(state == ComponentState::Running || state == ComponentState::Paused)
	{
		if(oldState == ComponentState::Stopped)
		{
			// Disable, and then re-enable components that have an AlwaysRun flag
			for(auto& entry : mActiveComponents)
			{
				if(entry->GetEnabled())
				{
					entry->OnDisabled();
					entry->OnEnabled();
				}
			}

			// Process any state changes queued by the component callbacks
			ProcessStateChanges();

			// Trigger enable on all components that don't have AlwaysRun flag (at this point those will be all
			// inactive components that have active scene object parents)
			for(auto& entry : mInactiveComponents)
			{
				if(entry->GetEnabled())
				{
					entry->OnEnabled();

					if(state == ComponentState::Running)
						mStateChanges.emplace_back(entry, ComponentStateEventType::Activated);
				}
			}

			// Process any state changes queued by the component callbacks
			ProcessStateChanges();

			// Initialize and enable uninitialized components
			for(auto& entry : mUninitializedComponents)
			{
				entry->OnBeginPlay();

				if(entry->GetEnabled())
				{
					entry->OnEnabled();
					mStateChanges.emplace_back(entry, ComponentStateEventType::Activated);
				}
				else
					mStateChanges.emplace_back(entry, ComponentStateEventType::Deactivated);
			}

			// Process any state changes queued by the component callbacks
			ProcessStateChanges();
		}
	}

	// Stop updates on all active components
	if(state == ComponentState::Paused || state == ComponentState::Stopped)
	{
		// Trigger onDisable events if stopping
		if(state == ComponentState::Stopped)
		{
			for(const auto& component : mActiveComponents)
			{
				const bool alwaysRun = component->HasFlag(ComponentFlag::AlwaysRun);

				component->OnDisabled();

				if(alwaysRun && component->GetEnabled())
					component->OnEnabled();
			}
		}

		// Move from active to inactive list
		for(i32 i = 0; i < (i32)mActiveComponents.size(); i++)
		{
			// Note: Purposely not a reference since the list changes in the add/remove methods below
			const HComponent component = mActiveComponents[i];

			const bool alwaysRun = component->HasFlag(ComponentFlag::AlwaysRun);
			if(alwaysRun && component->GetEnabled())
				continue;

			RemoveFromStateList(component);
			AddToStateList(component, InactiveList);

			i--; // Keep the same index next iteration to process the component we just swapped
		}
	}
}

void SceneManager::NotifyComponentCreatedInternal(const HComponent& component, bool parentActive)
{
	// Note: This method must remain reentrant (in case the callbacks below trigger component state changes)

	// Queue the change before any callbacks trigger, as the callbacks could trigger their own changes and they should
	// be in order
	mStateChanges.emplace_back(component, ComponentStateEventType::Created);
	ScopeToggle toggle(mDisableStateChange);

	component->OnCreated();

	const bool alwaysRun = component->HasFlag(ComponentFlag::AlwaysRun);
	if(alwaysRun || mComponentState != ComponentState::Stopped)
	{
		component->OnBeginPlay();

		if(parentActive && component->GetEnabled(true))
			component->OnEnabled();
	}
}

void SceneManager::NotifyComponentActivatedInternal(const HComponent& component, bool triggerEvent)
{
	// Note: This method must remain reentrant (in case the callbacks below trigger component state changes)

	// Queue the change before any callbacks trigger, as the callbacks could trigger their own changes and they should
	// be in order
	mStateChanges.emplace_back(component, ComponentStateEventType::Activated);
	ScopeToggle toggle(mDisableStateChange);

	const bool alwaysRun = component->HasFlag(ComponentFlag::AlwaysRun);
	if(alwaysRun || mComponentState != ComponentState::Stopped)
	{
		if(triggerEvent)
			component->OnEnabled();
	}
}

void SceneManager::NotifyComponentDeactivatedInternal(const HComponent& component, bool triggerEvent)
{
	// Note: This method must remain reentrant (in case the callbacks below trigger component state changes)

	// Queue the change before any callbacks trigger, as the callbacks could trigger their own changes and they should
	// be in order
	mStateChanges.emplace_back(component, ComponentStateEventType::Deactivated);
	ScopeToggle toggle(mDisableStateChange);

	const bool alwaysRun = component->HasFlag(ComponentFlag::AlwaysRun);
	if(alwaysRun || mComponentState != ComponentState::Stopped)
	{
		if(triggerEvent)
			component->OnDisabled();
	}
}

void SceneManager::NotifyComponentDestroyedInternal(const HComponent& component, bool immediate)
{
	// Note: This method must remain reentrant (in case the callbacks below trigger component state changes)

	// Queue the change before any callbacks trigger, as the callbacks could trigger their own changes and they should
	// be in order
	if(!immediate)
	{
		// If destruction is immediate no point in queuing state change since it will be ignored anyway
		mStateChanges.emplace_back(component, ComponentStateEventType::Destroyed);
	}

	ScopeToggle toggle(mDisableStateChange);

	const bool alwaysRun = component->HasFlag(ComponentFlag::AlwaysRun);
	const bool isEnabled = component->GetEnabled() && (alwaysRun || mComponentState != ComponentState::Stopped);

	if(isEnabled)
		component->OnDisabled();

	component->OnDestroyed();

	if(immediate)
	{
		// Since the state change wasn't queued, remove the component from the list right away. Its expected the caller
		// knows what is he doing.

		u32 existingListType;
		u32 existingIdx;
		DecodeComponentId(component->GetSceneManagerId(), existingIdx, existingListType);

		if(existingListType != 0)
			RemoveFromStateList(component);
	}
}

void SceneManager::NotifySceneInstanceCreated(const SPtr<SceneInstance>& sceneInstance)
{
	if(!B3D_ENSURE(sceneInstance != nullptr))
		return;

	mSceneInstances[sceneInstance.get()] = sceneInstance;
}

void SceneManager::NotifySceneInstanceDestroyed(SceneInstance* sceneInstance)
{
	auto found = mSceneInstances.find(sceneInstance);
	if(B3D_ENSURE(found != mSceneInstances.end()))
		mSceneInstances.erase(found);
}

void SceneManager::AddToStateList(const HComponent& component, u32 listType)
{
	if(listType == 0)
		return;

	Vector<HComponent>& list = *mComponentsPerState[listType - 1];

	const auto idx = (u32)list.size();
	list.push_back(component);

	component->SetSceneManagerId(EncodeComponentId(idx, listType));
}

void SceneManager::RemoveFromStateList(const HComponent& component)
{
	u32 listType;
	u32 idx;
	DecodeComponentId(component->GetSceneManagerId(), idx, listType);

	if(listType == 0)
		return;

	Vector<HComponent>& list = *mComponentsPerState[listType - 1];

	u32 lastIdx;
	DecodeComponentId(list.back()->GetSceneManagerId(), lastIdx, listType);

	B3D_ASSERT(list[idx] == component);

	if(idx != lastIdx)
	{
		std::swap(list[idx], list[lastIdx]);
		list[idx]->SetSceneManagerId(EncodeComponentId(idx, listType));
	}

	list.erase(list.end() - 1);
}

void SceneManager::ProcessStateChanges()
{
	const bool isStopped = mComponentState == ComponentState::Stopped;

	for(auto& entry : mStateChanges)
	{
		const HComponent& component = entry.Obj;

		// Must check queued state, because the component can be destroyed while in the state list otherwise
		if(component.IsDestroyed(true))
			continue;

		u32 existingListType;
		u32 existingIdx;
		DecodeComponentId(component->GetSceneManagerId(), existingIdx, existingListType);

		const bool alwaysRun = component->HasFlag(ComponentFlag::AlwaysRun);
		const bool isEnabled = component->GetEnabled();

		u32 listType = 0;
		switch(entry.Type)
		{
		case ComponentStateEventType::Created:
			if(alwaysRun || !isStopped)
				listType = isEnabled ? ActiveList : InactiveList;
			else
				listType = UninitializedList;
			break;
		case ComponentStateEventType::Activated:
		case ComponentStateEventType::Deactivated:
			if(alwaysRun || !isStopped)
				listType = isEnabled ? ActiveList : InactiveList;
			else
				listType = (existingListType == UninitializedList) ? UninitializedList : InactiveList;
			break;
		case ComponentStateEventType::Destroyed:
			listType = 0;
			break;
		default: break;
		}

		if(existingListType == listType)
			continue;

		if(existingListType != 0)
			RemoveFromStateList(component);

		AddToStateList(component, listType);
	}

	mStateChanges.clear();
}

u32 SceneManager::EncodeComponentId(u32 idx, u32 type)
{
	B3D_ASSERT(idx <= (0x3FFFFFFF));

	return (type << 30) | idx;
}

void SceneManager::DecodeComponentId(u32 id, u32& idx, u32& type)
{
	idx = id & 0x3FFFFFFF;
	type = id >> 30;
}

bool SceneManager::IsComponentOfType(const HComponent& component, u32 rttiId)
{
	return component->GetRtti()->GetRttiId() == rttiId;
}

void SceneManager::UpdateInternal()
{
	ProcessStateChanges();

	// Note: Eventually perform updates based on component types and/or on component priority. Right now we just
	// iterate in an undefined order, but it wouldn't be hard to change it.

	ScopeToggle toggle(mDisableStateChange);
	for(auto& entry : mActiveComponents)
		entry->Update();

	GameObjectManager::Instance().DestroyQueuedObjects();
}

void SceneManager::FixedUpdateInternal()
{
	ProcessStateChanges();

	ScopeToggle toggle(mDisableStateChange);
	for(auto& entry : mActiveComponents)
		entry->FixedUpdate();
}

void SceneManager::AddToMainScene(const HSceneObject& node)
{
	if(mMainScene->GetRoot())
		node->SetParent(mMainScene->GetRoot());
}

void SceneManager::OnMainRenderTargetResized()
{
	auto& rtProps = mMainRT->GetProperties();
	float aspect = rtProps.Width / (float)rtProps.Height;

	for(auto& entry : mMainCameras)
		entry->SetAspectRatio(aspect);
}

namespace bs
{
SceneManager& GetSceneManager()
{
	return SceneManager::Instance();
}
} // namespace bs
