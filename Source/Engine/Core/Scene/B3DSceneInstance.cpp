//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Scene/B3DSceneInstance.h"

#include "B3DApplication.h"
#include "B3DGameObjectCollection.h"
#include "B3DScene.h"
#include "B3DSceneManager.h"
#include "Components/B3DCamera.h"
#include "Scene/B3DSceneObject.h"
#include "Scene/B3DTransformSystem.h"
#include "Scene/B3DComponent.h"
#include "Components/B3DRenderable.h"
#include "GpuBackend/B3DViewport.h"
#include "Scene/B3DGameObjectManager.h"
#include "GpuBackend/B3DRenderTarget.h"
#include "Scene/B3DPrefab.h"
#include "Physics/B3DPhysics.h"
#include "Renderer/B3DRendererScene.h"
#include "Particles/B3DParticleScene.h"
#include "Profiling/B3DProfilerCPU.h"

#if B3D_WITH_EDITOR
#include "RTTI/B3DIEditorSceneInstanceRTTI.h"
#endif

using namespace b3d;

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

void SceneInstanceComponents::SetComponentState(ComponentState state)
{
	if(mDisableStateChange)
	{
		B3D_LOG(Warning, LogScene, "Component state cannot be changed from the calling locating. "
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

void SceneInstanceComponents::NotifyComponentCreated(const HComponent& component, bool parentActive)
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

void SceneInstanceComponents::NotifyComponentActivated(const HComponent& component, bool triggerEvent)
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

void SceneInstanceComponents::NotifyComponentDeactivated(const HComponent& component, bool triggerEvent)
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

void SceneInstanceComponents::NotifyComponentDestroyed(const HComponent& component, bool immediate)
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

void SceneInstanceComponents::Update()
{
	ProcessStateChanges();

	// Note: Eventually perform updates based on component types and/or on component priority. Right now we just
	// iterate in an undefined order, but it wouldn't be hard to change it.

	ScopeToggle toggle(mDisableStateChange);
	for(auto& entry : mActiveComponents)
		entry->Update();
}

void SceneInstanceComponents::FixedUpdate()
{
	ProcessStateChanges();

	ScopeToggle toggle(mDisableStateChange);
	for(auto& entry : mActiveComponents)
		entry->FixedUpdate();
}

void SceneInstanceComponents::AddToStateList(const HComponent& component, u32 listType)
{
	if(listType == 0)
		return;

	Vector<HComponent>& list = *mComponentsPerState[listType - 1];

	const auto idx = (u32)list.size();
	list.push_back(component);

	component->SetSceneManagerId(EncodeComponentId(idx, listType));
}

void SceneInstanceComponents::RemoveFromStateList(const HComponent& component)
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

void SceneInstanceComponents::ProcessStateChanges()
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

u32 SceneInstanceComponents::EncodeComponentId(u32 idx, u32 type)
{
	B3D_ASSERT(idx <= (0x3FFFFFFF));

	return (type << 30) | idx;
}

void SceneInstanceComponents::DecodeComponentId(u32 id, u32& idx, u32& type)
{
	idx = id & 0x3FFFFFFF;
	type = id >> 30;
}

bool SceneInstanceComponents::IsComponentOfType(const HComponent& component, u32 rttiId)
{
	return component->GetRtti()->GetRttiId() == rttiId;
}

#if B3D_WITH_EDITOR
RTTIType* IEditorSceneInstance::GetRttiStatic()
{
	return IEditorSceneInstanceRTTI::Instance();
}

RTTIType* IEditorSceneInstance::GetRtti() const
{
	return IEditorSceneInstance::GetRttiStatic();
}
#endif

SceneInstance::SceneInstance(ConstructPrivately dummy, const String& name, const HSceneObject& root, const UUID& associatedResourceId)
	: mName(name), mRoot(root), mAssociatedResourceId(associatedResourceId), mPhysicsScene(GetPhysics().CreatePhysicsScene()), mRendererScene(RendererScene::Create()), mAnimationScene(AnimationScene::Create()), mParticleScene(ParticleScene::Create()), mGameObjectCollection(root->GetOwnerCollection())
{
}

SceneInstance::~SceneInstance()
{
	if(mGameObjectCollection != nullptr)
		B3D_ASSERT(mGameObjectCollection->GetObjectCount() == 0);

	GetSceneManager().NotifySceneInstanceDestroyed(this);
}

void SceneInstance::Initialize()
{
	CoreObject::Initialize();

	mAnimationScene->SetOwner(std::static_pointer_cast<SceneInstance>(GetShared()));
	mParticleScene->SetOwner(std::static_pointer_cast<SceneInstance>(GetShared()));
	mRendererScene->SetOwner(std::static_pointer_cast<SceneInstance>(GetShared()));
}

void SceneInstance::Destroy()
{
	if(!mRoot.IsDestroyed())
		mRoot->Destroy();

	if(mGameObjectCollection != nullptr)
		mGameObjectCollection->DestroyQueuedObjects();

	CoreObject::Destroy();
}

void SceneInstance::FixedUpdate()
{
	u64 fixedUpdateStep;
	const u32 iterationCount = mTime.GetFixedUpdateStep(fixedUpdateStep);

	const float stepSeconds = fixedUpdateStep / 1000000.0f;
	for(u32 i = 0; i < iterationCount; i++)
	{
		ecs::TransformSystem::Update(mGameObjectCollection->GetECSRegistry());
		PROFILE_CALL(SceneInstanceComponents::FixedUpdate(), "Scene fixed update");
		PROFILE_CALL(mPhysicsScene->FixedUpdate(stepSeconds), "Physics simulation");

		mTime.AdvanceFixedUpdate(fixedUpdateStep);
	}
}

void SceneInstance::Update()
{
	ecs::TransformSystem::Update(mGameObjectCollection->GetECSRegistry());
	SceneInstanceComponents::Update();
	mGameObjectCollection->DestroyQueuedObjects();

	mPhysicsScene->Update();
}

void SceneInstance::SetRoot(const HSceneObject& newRoot)
{
	if(newRoot == nullptr)
		return;

	HSceneObject oldRoot = mRoot;
	TShared<GameObjectCollection> oldGameObjectCollection = mGameObjectCollection;

	// Must be set before mRoot->SetScene, as it will retrieve the game object collection from the scene instance
	mGameObjectCollection = newRoot->GetOwnerCollection().lock();

	mRoot = newRoot;
	mRoot->ClearParent();
	mRoot->SetScene(std::static_pointer_cast<SceneInstance>(GetShared()));

	const u32 childCount = oldRoot->GetChildCount();

	// Make sure to keep persistent objects
	{
		FrameAllocatorScope frameScope;
		FrameVector<HSceneObject> toMove;
		for(u32 i = 0; i < childCount; i++)
		{
			HSceneObject child = oldRoot->GetChild(i);

			if(child->HasFlag(SceneObjectFlag::RuntimePersistent))
				toMove.push_back(child);
		}

		for(auto& entry : toMove)
			entry->SetParent(newRoot, false);
	}

	oldRoot->Destroy();
	oldGameObjectCollection->DestroyQueuedObjects();
}

void SceneInstance::Clear(bool forceAll)
{
	const u32 childCount = mRoot->GetChildCount();

	u32 childIndex = 0;
	for(u32 i = 0; i < childCount; i++)
	{
		HSceneObject child = mRoot->GetChild(childIndex);

		if(forceAll || !child->HasFlag(SceneObjectFlag::RuntimePersistent))
			child->Destroy();
		else
			childIndex++;
	}

	const TShared<GameObjectCollection>& gameObjectCollection = GetGameObjectCollection();
	if(B3D_ENSURE(gameObjectCollection != nullptr))
		gameObjectCollection->DestroyQueuedObjects();

	HSceneObject newRoot = SceneObject::CreateInternal(gameObjectCollection, "SceneRoot");
	SetRoot(newRoot);
	SetAssociatedResourceId(UUID::kEmpty);
}

void SceneInstance::RegisterCamera(const HCamera& camera)
{
	mCameras[camera.GetId()] = camera;
}

void SceneInstance::UnregisterCamera(const HCamera& camera)
{
	mCameras.erase(camera.GetId());

	auto iterFind = std::find_if(mMainCameras.begin(), mMainCameras.end(), [&](const HCamera& x)
								 { return x == camera; });

	if(iterFind != mMainCameras.end())
		mMainCameras.erase(iterFind);
}

void SceneInstance::NotifyMainCameraStateChanged(const HCamera& camera)
{
	auto iterFind = std::find_if(mMainCameras.begin(), mMainCameras.end(), [&](const HCamera& entry)
								 { return entry == camera; });

	TShared<Viewport> viewport = camera->GetViewport();
	if(camera->IsMain())
	{
		if(iterFind == mMainCameras.end())
			mMainCameras.push_back(mCameras[camera.GetId()]);

		viewport->SetTarget(mPrimaryRenderTarget);
	}
	else
	{
		if(iterFind != mMainCameras.end())
			mMainCameras.erase(iterFind);

		if(viewport->GetTarget() == mPrimaryRenderTarget)
			viewport->SetTarget(nullptr);
	}
}

HCamera SceneInstance::GetMainCamera() const
{
	if(mMainCameras.size() > 0)
		return mMainCameras[0];

	return nullptr;
}

void SceneInstance::SetMainCameraRenderTarget(const TShared<RenderTarget>& renderTarget)
{
	if(mPrimaryRenderTarget == renderTarget)
		return;

	mMainRenderTargetResizedHandle.Disconnect();

	if(renderTarget != nullptr)
		mMainRenderTargetResizedHandle = renderTarget->OnResized.Connect([this]() { OnMainRenderTargetResized(); });

	mPrimaryRenderTarget = renderTarget;

	float aspect = 1.0f;
	if(renderTarget != nullptr)
	{
		auto& rtProps = renderTarget->GetProperties();
		aspect = rtProps.Width / (float)rtProps.Height;
	}

	for(auto& entry : mMainCameras)
	{
		entry->GetViewport()->SetTarget(renderTarget);
		entry->SetAspectRatio(aspect);
	}
}

void SceneInstance::OnMainRenderTargetResized()
{
	auto& rtProps = mPrimaryRenderTarget->GetProperties();
	float aspect = rtProps.Width / (float)rtProps.Height;

	for(auto& entry : mMainCameras)
		entry->SetAspectRatio(aspect);
}

HSceneObject SceneInstance::CreateSceneObject(const String& name, u32 flags)
{
	HSceneObject newSceneObject = SceneObject::CreateInternal(mGameObjectCollection, name, flags);
	newSceneObject->SetParent(mRoot, false);
	newSceneObject->Initialize();

	return newSceneObject;
}

TShared<SceneInstance> SceneInstance::Create(const String& name)
{
	const TShared<GameObjectCollection>& gameObjectCollection = GameObjectCollection::Create();
	HSceneObject root = SceneObject::CreateInternal(gameObjectCollection, "Root");

	TShared<SceneInstance> sceneInstance = B3DMakeShared<SceneInstance>(ConstructPrivately(), name, root, UUID::kEmpty);
	root->SetScene(sceneInstance);

	SceneManager::Instance().NotifySceneInstanceCreated(sceneInstance);
	root->Initialize();

	sceneInstance->SetShared(sceneInstance);
	sceneInstance->Initialize();

	// Apply fixed timestep if set via command-line
	const u64 fixedDeltaTimeUs = ::GetTime().GetFixedDeltaTimeUs();
	sceneInstance->GetTime().SetFixedDeltaTimeUs(fixedDeltaTimeUs);

	return sceneInstance;
}

TShared<SceneInstance> SceneInstance::Create(const String& name, const HSceneObject& root)
{
	return Create(name, root, UUID::kEmpty);
}

TShared<SceneInstance> SceneInstance::Create(const String& name, const HSceneObject& root, const UUID& associatedResourceId)
{
	const TShared<GameObjectCollection>& gameObjectCollection = root->GetOwnerCollection().lock();
	if(!B3D_ENSURE(gameObjectCollection != nullptr))
		return nullptr;

	TShared<SceneInstance> sceneInstance = B3DMakeShared<SceneInstance>(ConstructPrivately(), name, root, associatedResourceId);
	root->SetScene(sceneInstance);

	SceneManager::Instance().NotifySceneInstanceCreated(sceneInstance);

	sceneInstance->SetShared(sceneInstance);
	sceneInstance->Initialize();

	// Apply fixed timestep if set via command-line
	const u64 fixedDeltaTimeUs = ::GetTime().GetFixedDeltaTimeUs();
	sceneInstance->GetTime().SetFixedDeltaTimeUs(fixedDeltaTimeUs);

	return sceneInstance;
}

TShared<render::RenderProxy> SceneInstance::CreateRenderProxy() const
{
	const TShared<render::RendererScene>& rendererSceneProxy = B3DGetRenderProxy(mRendererScene);

	render::SceneInstance* renderProxy = new(B3DAllocate<render::SceneInstance>()) render::SceneInstance(GetInternalId(), rendererSceneProxy);
	TShared<render::SceneInstance> renderProxyShared = B3DMakeSharedFromExisting<render::SceneInstance>(renderProxy);
	renderProxyShared->SetShared(renderProxyShared);

	return renderProxyShared;
}
