//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Scene/B3DSceneObject.h"
#include "Scene/B3DSceneObjectFragments.h"

#include "Scene/B3DComponent.h"
#include "Scene/B3DSceneManager.h"
#include "Scene/B3DSceneInstance.h"
#include "Error/B3DException.h"
#include "Debug/B3DDebug.h"
#include "RTTI/B3DSceneObjectRTTI.h"
#include "Serialization/B3DBinarySerializer.h"
#include "FileSystem/B3DDataStream.h"
#include "Scene/B3DPrefabUtility.h"
#include "B3DApplication.h"
#include "B3DGameObjectCollection.h"
#include "Script/B3DIScriptObjectWrapper.h"

using namespace b3d;

SceneObject::SceneObject(const String& name, u32 flags)
	: GameObject(), mFlags(flags)
{
	SetName(name);
}

SceneObject::~SceneObject()
{
	if(!mThisHandle.IsDestroyed())
	{
		B3D_LOG(Warning, LogScene, "Object is being deleted without being destroyed first? {0}", mName);
		DestroyImmediate();
	}
}

HSceneObject SceneObject::Create(const String& name, u32 flags)
{
	const SPtr<SceneInstance>& mainScene = SceneManager::Instance().GetMainScene();
	return mainScene->CreateSceneObject(name, flags);
}

HSceneObject SceneObject::CreateInternal(const SPtr<GameObjectCollection>& ownerCollection, const String& name, u32 flags)
{
	const SPtr<SceneObject> sceneObject = SPtr<SceneObject>(new(B3DAllocate<SceneObject>()) SceneObject(name, flags), &B3DDelete<SceneObject>, StdAlloc<SceneObject>());
	sceneObject->mId = UUIDGenerator::GenerateRandom();

	sceneObject->CreateECSEntity(&ownerCollection->GetECSRegistry());
	sceneObject->mECSRegistry->AddTag<ecs::Movable>(sceneObject->mECSEntity);

	return CreateInternal(ownerCollection, sceneObject);
}

HSceneObject SceneObject::CreateInternal(const SPtr<GameObjectCollection>& ownerCollection, const SPtr<SceneObject>& sceneObject)
{
	if(!B3D_ENSURE(ownerCollection != nullptr))
		return HSceneObject();

	HSceneObject sceneObjectHandle =
		B3DStaticGameObjectCast<SceneObject>(ownerCollection->RegisterNewObject(sceneObject));
	sceneObjectHandle->mThisHandle = sceneObjectHandle;

	return sceneObjectHandle;
}

void SceneObject::Destroy(bool immediate)
{
	// Detach from parent immediately
	if(mParent != nullptr)
	{
		if(!mParent.IsDestroyed())
			mParent->RemoveChild(GetHandle());

		mParent = nullptr;
	}

	if(immediate)
		DestroyImmediate();
	else
		QueueForDestroy();
}

void SceneObject::QueueForDestroy()
{
	if(HasGameObjectFlag(GameObjectTransientFlag::QueuedForDestroy))
		return;

	if(!B3D_ENSURE(!HasGameObjectFlag(GameObjectTransientFlag::Destroyed)))
		return;

	// Important to queue components to destroy before the scene objects, as they will reference their parent during destruction
	for(const auto& component : mComponents)
		component->QueueForDestroy(false);

	for(const auto& child : mChildren)
		child->QueueForDestroy();

	GameObject::QueueForDestroy();
}

void SceneObject::CreateECSEntity(ecs::Registry* registry)
{
	B3D_ASSERT(mECSRegistry == nullptr && mECSEntity == ecs::kNullEntity);

	mECSRegistry = registry;
	mECSEntity = mECSRegistry->CreateEntity();
	mECSRegistry->AddComponent<ecs::LocalTransform>(mECSEntity, ecs::LocalTransform());
	mECSRegistry->AddComponent<ecs::WorldTransform>(mECSEntity, ecs::WorldTransform());
	mECSRegistry->AddComponent<ecs::HierarchyDepth>(mECSEntity, ecs::HierarchyDepth());
	mECSRegistry->AddComponent<ecs::Parent>(mECSEntity, ecs::Parent{ ecs::kNullEntity });
}

void SceneObject::DestroyImmediate()
{
	// If queued for destroy, children will be queued as well
	if(!HasGameObjectFlag(GameObjectTransientFlag::QueuedForDestroy))
	{
		for(auto it = mChildren.begin(); it != mChildren.end();)
		{
			(*it)->DestroyImmediate();

			it = mChildren.erase(it);
		}

		// It's important to remove the elements from the array as soon as they're destroyed, as OnDestroy callbacks
		// for components might query the SO's components, and we want to only return live ones
		for(auto it = mComponents.begin(); it != mComponents.end();)
		{
			(*it)->DestroyImmediate(false);

			it = mComponents.erase(it);
		}
	}

	mChildren.clear();
	mComponents.clear();

	if(mECSRegistry != nullptr && mECSEntity != ecs::kNullEntity)
	{
		mECSRegistry->EraseEntity(mECSEntity);
		mECSEntity = ecs::kNullEntity;
		mECSRegistry = nullptr;
	}

	GameObject::DestroyImmediate();
}

void SceneObject::SetOwnerCollection(const SPtr<GameObjectCollection>& collection)
{
	for(const auto& component : mComponents)
		component->SetOwnerCollection(collection);

	GameObject::SetOwnerCollection(collection);
}

bool SceneObject::IsPrefabInstanceRoot() const
{
	if(!IsPrefabInstance())
		return false;

	const HSceneObject& parent = GetParent();

	if(parent == nullptr)
		return true;

	if(GetPrefabResourceId() != parent->GetPrefabResourceId())
		return true;
	
	return false;
}

HSceneObject SceneObject::GetPrefabInstanceRoot() const
{
	if(mPrefabResourceId.Empty())
		return HSceneObject();

	HSceneObject currentObject = GetHandle();
	while(currentObject != nullptr)
	{
		if(currentObject->IsPrefabInstanceRoot())
			return currentObject;

		currentObject = currentObject->GetParent();
	}

	return currentObject;
}

void SceneObject::BreakPrefabLink()
{
	HSceneObject prefabInstanceRoot = GetPrefabInstanceRoot();
	if(prefabInstanceRoot == nullptr)
		return;

	prefabInstanceRoot->ClearPrefabDelta();
	PrefabUtility::ClearPrefabIds(prefabInstanceRoot);

	// If the parent is a prefab instance, we should become part of that prefab now
	const HSceneObject& parent = GetParent();
	if(parent && parent->IsPrefabInstance())
	{
		PrefabUtility::AssignPrefabResourceId(prefabInstanceRoot, parent->GetPrefabResourceId());
	}
}

bool SceneObject::HasFlag(SceneObjectFlag flag) const
{
	return mFlags.IsSet(flag);
}

void SceneObject::SetFlags(SceneObjectFlags flags)
{
	mFlags |= flags;

	for(auto& child : mChildren)
		child->SetFlags(flags);
}

void SceneObject::UnsetFlags(SceneObjectFlags flags)
{
	mFlags &= ~flags;

	for(auto& child : mChildren)
		child->UnsetFlags(flags);
}

void SceneObject::Initialize()
{
	Function<void(SceneObject*)> fnInitialize = [&fnInitialize](SceneObject* sceneObject)
	{
		if(sceneObject->GetScene() == nullptr)
		{
			B3D_LOG(Warning, LogScene, "Cannot instantiate scene object. No associated scene found.");
			return;
		}

		sceneObject->SetGameObjectFlag(GameObjectTransientFlag::Initialized);

		if(sceneObject->mParent == nullptr)
			sceneObject->SetParent(sceneObject->GetScene()->GetRoot());

		for(auto& component : sceneObject->mComponents)
		{
			if(!component->HasGameObjectFlag(GameObjectTransientFlag::Initialized))
				component->Initialize();
		}

		for(auto& child : sceneObject->mChildren)
			fnInitialize(child.Get());
	};

	const SPtr<SceneInstance>& scene = GetScene();
	Function<void(SceneObject*)> fnTriggerComponentCreatedEvents = [&fnTriggerComponentCreatedEvents, &scene](SceneObject* sceneObject)
	{
		if(!sceneObject->HasGameObjectFlag(GameObjectTransientFlag::Initialized))
			return;

		for(auto& component : sceneObject->mComponents)
			scene->NotifyComponentCreated(component, sceneObject->GetActive());

		for(auto& child : sceneObject->mChildren)
				fnTriggerComponentCreatedEvents(child.Get());
	};

	fnInitialize(this);
	fnTriggerComponentCreatedEvents(this);
}

/************************************************************************/
/* 								Transform	                     		*/
/************************************************************************/

bool SceneObject::IsMovable() const
{
	B3D_ASSERT(mECSRegistry != nullptr);
	return mECSRegistry->HasAllOf<ecs::Movable>(mECSEntity);
}

Transform& SceneObject::GetMutableLocalTransform()
{
	B3D_ASSERT(mECSRegistry != nullptr);
	return mECSRegistry->GetComponents<ecs::LocalTransform>(mECSEntity);
}

Transform& SceneObject::GetMutableWorldTransform()
{
	B3D_ASSERT(mECSRegistry != nullptr);
	return mECSRegistry->GetComponents<ecs::WorldTransform>(mECSEntity);
}

void SceneObject::UpdateHierarchyDepthFromParent()
{
	u16 hierarchyDepth = 0;
	if(mParent != nullptr)
	{
		B3D_ASSERT(mParent->mECSRegistry != nullptr);
		const ecs::HierarchyDepth& parentDepth = mParent->mECSRegistry->GetComponents<ecs::HierarchyDepth>(mParent->mECSEntity);
		hierarchyDepth = (u16)(parentDepth.Depth + 1);
	}

	UpdateHierarchyDepthRecursive(hierarchyDepth);
}

void SceneObject::UpdateHierarchyDepthRecursive(u16 hierarchyDepth)
{
	B3D_ASSERT(mECSRegistry != nullptr);
	mECSRegistry->GetComponents<ecs::HierarchyDepth>(mECSEntity).Depth = hierarchyDepth;

	const u16 childDepth = hierarchyDepth == 0xFFFF ? 0xFFFF : (u16)(hierarchyDepth + 1);
	for(auto& child : mChildren)
		child->UpdateHierarchyDepthRecursive(childDepth);
}

const Transform& SceneObject::GetLocalTransform() const
{
	B3D_ASSERT(mECSRegistry != nullptr);
	return mECSRegistry->GetComponents<ecs::LocalTransform>(mECSEntity);
}

const Transform& SceneObject::GetTransform() const
{
	B3D_ASSERT(mECSRegistry != nullptr);

	if(mECSRegistry->HasAllOf<ecs::TransformDirty>(mECSEntity))
		UpdateWorldTransform();

	return mECSRegistry->GetComponents<ecs::WorldTransform>(mECSEntity);
}

void SceneObject::UpdateWorldTransformIfDirty() const
{
	B3D_ASSERT(mECSRegistry != nullptr);

	if(mECSRegistry->HasAllOf<ecs::TransformDirty>(mECSEntity))
		UpdateWorldTransform();
}

void SceneObject::SetLocalTransform(const Transform& transform)
{
	B3D_ASSERT(mECSRegistry != nullptr);

	if(!IsMovable())
		return;

	GetMutableLocalTransform() = transform;
	NotifyTransformChanged(TCF_Transform);
}

void SceneObject::SetPosition(const Vector3& position)
{
	if(IsMovable())
	{
		GetMutableLocalTransform().SetPosition(position);
		NotifyTransformChanged(TCF_Transform);
	}
}

void SceneObject::SetRotation(const Quaternion& rotation)
{
	if(IsMovable())
	{
		GetMutableLocalTransform().SetRotation(rotation);
		NotifyTransformChanged(TCF_Transform);
	}
}

void SceneObject::SetScale(const Vector3& scale)
{
	if(IsMovable())
	{
		GetMutableLocalTransform().SetScale(scale);
		NotifyTransformChanged(TCF_Transform);
	}
}

void SceneObject::SetWorldPosition(const Vector3& position)
{
	if(!IsMovable())
		return;

	Transform& localTfrm = GetMutableLocalTransform();
	if(mParent != nullptr)
		localTfrm.SetWorldPosition(position, mParent->GetTransform());
	else
		localTfrm.SetPosition(position);

	NotifyTransformChanged(TCF_Transform);
}

void SceneObject::SetWorldRotation(const Quaternion& rotation)
{
	if(!IsMovable())
		return;

	Transform& localTfrm = GetMutableLocalTransform();
	if(mParent != nullptr)
		localTfrm.SetWorldRotation(rotation, mParent->GetTransform());
	else
		localTfrm.SetRotation(rotation);

	NotifyTransformChanged(TCF_Transform);
}

void SceneObject::SetWorldScale(const Vector3& scale)
{
	if(!IsMovable())
		return;

	Transform& localTfrm = GetMutableLocalTransform();
	if(mParent != nullptr)
		localTfrm.SetWorldScale(scale, mParent->GetTransform());
	else
		localTfrm.SetScale(scale);

	NotifyTransformChanged(TCF_Transform);
}

void SceneObject::LookAt(const Vector3& location, const Vector3& up)
{
	const Transform& worldTfrm = GetTransform();

	Vector3 forward = location - worldTfrm.GetPosition();

	Quaternion rotation = worldTfrm.GetRotation();
	rotation.LookRotation(forward, up);
	SetWorldRotation(rotation);
}

Matrix4 SceneObject::GetWorldMatrix() const
{
	return GetTransform().GetMatrix();
}

Matrix4 SceneObject::GetInvWorldMatrix() const
{
	return GetTransform().GetInvMatrix();
}

Matrix4 SceneObject::GetLocalMatrix() const
{
	return GetLocalTransform().GetMatrix();
}

void SceneObject::Move(const Vector3& vec)
{
	if(IsMovable())
	{
		GetMutableLocalTransform().Move(vec);
		NotifyTransformChanged(TCF_Transform);
	}
}

void SceneObject::MoveRelative(const Vector3& vec)
{
	if(IsMovable())
	{
		GetMutableLocalTransform().MoveRelative(vec);
		NotifyTransformChanged(TCF_Transform);
	}
}

void SceneObject::Rotate(const Vector3& axis, const Radian& angle)
{
	if(IsMovable())
	{
		GetMutableLocalTransform().Rotate(axis, angle);
		NotifyTransformChanged(TCF_Transform);
	}
}

void SceneObject::Rotate(const Quaternion& q)
{
	if(IsMovable())
	{
		GetMutableLocalTransform().Rotate(q);
		NotifyTransformChanged(TCF_Transform);
	}
}

void SceneObject::Roll(const Radian& angle)
{
	if(IsMovable())
	{
		GetMutableLocalTransform().Roll(angle);
		NotifyTransformChanged(TCF_Transform);
	}
}

void SceneObject::Yaw(const Radian& angle)
{
	if(IsMovable())
	{
		GetMutableLocalTransform().Yaw(angle);
		NotifyTransformChanged(TCF_Transform);
	}
}

void SceneObject::Pitch(const Radian& angle)
{
	if(IsMovable())
	{
		GetMutableLocalTransform().Pitch(angle);
		NotifyTransformChanged(TCF_Transform);
	}
}

void SceneObject::SetForward(const Vector3& forwardDir)
{
	const Transform& worldTfrm = GetTransform();

	Quaternion currentRotation = worldTfrm.GetRotation();
	currentRotation.LookRotation(forwardDir);
	SetWorldRotation(currentRotation);
}

void SceneObject::NotifyTransformChanged(TransformChangedFlags flags) const
{
	// If object is immovable, don't send transform changed events nor mark the transform dirty
	TransformChangedFlags componentFlags = flags;
	if(!IsMovable())
		componentFlags = (TransformChangedFlags)(componentFlags & ~TCF_Transform);
	else
	{
		mECSRegistry->AddTag<ecs::TransformDirty>(mECSEntity);
		mDirtyHash++;
	}

	// Only send component flags if we haven't removed them all
	if(componentFlags != 0)
	{
		const SPtr<SceneInstance>& scene = GetScene();
		for(auto& entry : mComponents)
		{
			if(entry->SupportsNotify(flags))
			{
				const bool alwaysRun = entry->HasFlag(ComponentFlag::AlwaysRun);
				if(alwaysRun || scene->IsRunning() || (flags & TCF_NotifyStopped) != 0)
					entry->OnTransformChanged(componentFlags);
			}
		}
	}

	// Mobility flag is only relevant for this scene object
	flags = (TransformChangedFlags)(flags & ~TCF_Mobility);
	if(flags != 0)
	{
		for(auto& entry : mChildren)
			entry->NotifyTransformChanged(flags);
	}
}

void SceneObject::UpdateWorldTransform() const
{
	B3D_ASSERT(mECSRegistry != nullptr);

	Transform& worldTfrm = const_cast<SceneObject*>(this)->GetMutableWorldTransform();
	worldTfrm = GetLocalTransform();

	// Don't allow movement from parent when not movable
	if(mParent != nullptr && mECSRegistry->HasAllOf<ecs::Movable>(mECSEntity))
		worldTfrm.MakeWorld(mParent->GetTransform());

	mECSRegistry->RemoveTag<ecs::TransformDirty>(mECSEntity);
}

/************************************************************************/
/* 								Hierarchy	                     		*/
/************************************************************************/

void SceneObject::SetParent(const HSceneObject& parent, bool keepWorldTransform)
{
	if(parent.IsDestroyed())
		return;

#if B3D_IS_ENGINE
	UUID originalPrefabResourceId = GetPrefabResourceId();
#endif

	if(!mECSRegistry->HasAllOf<ecs::Movable>(mECSEntity))
		keepWorldTransform = true;

	SetParentInternal(parent, keepWorldTransform);

#if B3D_IS_ENGINE
	if(GetApplication().IsEditor())
	{
		UUID newPrefabResourceId = GetPrefabResourceId();
		if(originalPrefabResourceId != newPrefabResourceId)
		{
			PrefabUtility::ClearPrefabIds(GetHandle());
			PrefabUtility::AssignPrefabResourceId(GetHandle(), newPrefabResourceId);
		}
	}
#endif
}

void SceneObject::SetParentInternal(const HSceneObject& parent, bool keepWorldTransform)
{
	if(mThisHandle == parent)
		return;

	if(mParent == nullptr || mParent != parent)
	{
		Transform worldTfrm;

		// Make sure the object keeps its world coordinates
		if(keepWorldTransform)
			worldTfrm = GetTransform();

		if(mParent != nullptr)
			mParent->RemoveChild(GetHandle());

		if(parent != nullptr)
		{
			SetScene(parent->GetScene());
			parent->AddChild(GetHandle());
		}
		else
			SetScene(nullptr);

		mParent = parent;

		ecs::Parent& parentFragment = mECSRegistry->GetOrAddComponent<ecs::Parent>(mECSEntity);
		if(parent != nullptr)
		{
			B3D_ASSERT(parent->mECSEntity != ecs::kNullEntity);
			parentFragment.Entity = parent->mECSEntity;
		}
		else
			parentFragment.Entity = ecs::kNullEntity;

		UpdateHierarchyDepthFromParent();

		if(keepWorldTransform && parent != nullptr)
		{
			Transform& localTfrm = GetMutableLocalTransform();
			localTfrm = worldTfrm;

			if(mParent != nullptr)
				localTfrm.MakeLocal(mParent->GetTransform());
		}

		if(const bool isInitialized = HasGameObjectFlag(GameObjectTransientFlag::Initialized))
			NotifyTransformChanged((TransformChangedFlags)(TCF_Parent | TCF_Transform));
	}
}

void SceneObject::ClearParent()
{
	if(mParent != nullptr)
		mParent->RemoveChild(GetHandle());

	SetScene(nullptr);
	mParent = nullptr;

	ecs::Parent& parentFragment = mECSRegistry->GetOrAddComponent<ecs::Parent>(mECSEntity);
	parentFragment.Entity = ecs::kNullEntity;

	UpdateHierarchyDepthFromParent();
}

void SceneObject::SetScene(const SPtr<SceneInstance>& scene)
{
	const SPtr<SceneInstance> currentScene = mParentScene.lock();
	if(currentScene == scene)
		return;

	mParentScene = scene;

	bool updatedRegistry = false;
	if(scene != nullptr)
	{
		const SPtr<GameObjectCollection>& sceneCollection = scene->GetGameObjectCollection();
		SetOwnerCollection(sceneCollection);

		ecs::Registry* sceneRegistry = &sceneCollection->GetECSRegistry();
		if(mECSRegistry == nullptr)
		{
			CreateECSEntity(sceneRegistry);
			mECSRegistry->AddTag<ecs::Movable>(mECSEntity);

			updatedRegistry = true;
		}
		else if(mECSRegistry != sceneRegistry)
		{
			// Migrate entity to new registry (e.g. persistent object moving between scenes)
			ecs::Registry* oldRegistry = mECSRegistry;
			ecs::Entity oldEntity = mECSEntity;

			// Read SceneObject's own data from old entity
			Transform localTfrm = GetLocalTransform();
			Transform worldTfrm = GetTransform();
			ObjectMobility mobility = GetMobility();
			ecs::HierarchyDepth hierarchyDepth = oldRegistry->GetComponents<ecs::HierarchyDepth>(oldEntity);

			// Create new entity in new registry (old entity still alive for component reads)
			mECSRegistry = sceneRegistry;
			mECSEntity = mECSRegistry->CreateEntity();
			mECSRegistry->AddComponent<ecs::LocalTransform>(mECSEntity, ecs::LocalTransform(localTfrm));
			mECSRegistry->AddComponent<ecs::WorldTransform>(mECSEntity, ecs::WorldTransform(worldTfrm));
			mECSRegistry->AddComponent<ecs::HierarchyDepth>(mECSEntity, hierarchyDepth);
			AddMobilityTag(mobility);

			// Notify components so they can migrate their ECS fragments
			// (old entity is still alive — components can read from it)
			for(auto& entry : mComponents)
				entry->OnSceneChanged(currentScene.get(), oldEntity);

			// Old entity no longer needed
			oldRegistry->EraseEntity(oldEntity);

			updatedRegistry = true;
		}

		// Update reference to the parent entity in the new registry
		if(updatedRegistry)
		{
			ecs::Parent& parent = mECSRegistry->AddComponent<ecs::Parent>(mECSEntity);
			if(mParent != nullptr)
			{
				B3D_ASSERT(mParent->mECSEntity != ecs::kNullEntity);
				parent.Entity = mParent->mECSEntity;
			}
			else
				parent.Entity = ecs::kNullEntity;
		}
	}
	else
	{
		// Note: We purposefully don't destroy the ECS entity here, as the scene object still belongs to the
		// original game object collection. We don't clear the original game object collection because the
		// scene object must belong to one. Scene object without a parent is only valid as a temporary state.
	}

	for(auto& child : mChildren)
		child->SetScene(scene);

	// Update child entities after they are registered in the new scene's registry
	if(scene != nullptr && updatedRegistry)
	{
		ecs::Children& children = mECSRegistry->AddComponent<ecs::Children>(mECSEntity);
		B3D_ASSERT(children.Entities.Empty());

		children.Entities.Reserve(mChildren.size());

		for(auto& child : mChildren)
		{
			B3D_ASSERT(child->mECSEntity != ecs::kNullEntity);
			children.Entities.Add(child->mECSEntity);
		}
	}
}

HSceneObject SceneObject::GetChild(u32 idx) const
{
	if(idx >= mChildren.size())
	{
		B3D_EXCEPT(InternalErrorException, "Child index out of range.");
	}

	return mChildren[idx];
}

int SceneObject::IndexOfChild(const HSceneObject& child) const
{
	for(int i = 0; i < (int)mChildren.size(); i++)
	{
		if(mChildren[i] == child)
			return i;
	}

	return -1;
}

void SceneObject::IterateHierarchy(const Function<bool(const HSceneObject&)>& onSceneObjectFound, const Function<void(const HComponent&)>& onComponentFound, bool visitSelf) const
{
	const Function<void(const HSceneObject&)> fnVisitComponentsAndChildren = [&onSceneObjectFound, &onComponentFound, &fnVisitComponentsAndChildren](const HSceneObject& sceneObject)
	{
		if(onSceneObjectFound != nullptr)
		{
			if(!onSceneObjectFound(sceneObject))
				return;
		}

		if(onComponentFound != nullptr)
		{
			for(auto& component : sceneObject->GetComponents())
				onComponentFound(component);
		}

		for(const HSceneObject& child : sceneObject->mChildren)
			fnVisitComponentsAndChildren(child);
	};

	if(visitSelf)
	{
		fnVisitComponentsAndChildren(GetHandle());
	}
	else
	{
		if(onComponentFound != nullptr)
		{
			for(auto& component : mComponents)
				onComponentFound(component);
		}

		for(const HSceneObject& child : mChildren)
			fnVisitComponentsAndChildren(child);
	}
}

void SceneObject::AddChild(const HSceneObject& object)
{
	mChildren.push_back(object);

	object->SetFlags(mFlags);

	ecs::Children& children = mECSRegistry->GetOrAddComponent<ecs::Children>(mECSEntity);

	B3D_ASSERT(object->mECSEntity != ecs::kNullEntity);
	children.Entities.Add(object->mECSEntity);
}

void SceneObject::RemoveChild(const HSceneObject& object)
{
	{
		auto result = find(mChildren.begin(), mChildren.end(), object);

		if(B3D_ENSURE(result != mChildren.end()))
			mChildren.erase(result);
	}

	// Remove from ECS
	{
		ecs::Children& children = mECSRegistry->GetComponents<ecs::Children>(mECSEntity);
		auto result = std::find(children.Entities.begin(), children.Entities.end(), object->mECSEntity);

		if(B3D_ENSURE(result != children.Entities.end()))
			children.Entities.erase(result);

		if(children.Entities.Empty())
			mECSRegistry->RemoveComponents<ecs::Children>(mECSEntity);
	}
}

HSceneObject SceneObject::FindPath(const String& path) const
{
	if(path.empty())
		return HSceneObject();

	String trimmedPath = StringUtility::Trim(path, "/");

	Vector<String> entries = StringUtility::Split(trimmedPath, "/");

	// Find scene object referenced by the path
	HSceneObject so = GetHandle();
	u32 pathIdx = 0;
	for(; pathIdx < (u32)entries.size(); pathIdx++)
	{
		String entry = entries[pathIdx];

		if(entry.empty())
			continue;

		// This character signifies not-a-scene-object. This is allowed to support
		// paths used by the scripting system (which can point to properties of
		// components on scene objects).
		if(entry[0] != '!')
			break;

		String childName = entry.substr(1, entry.size() - 1);
		so = so->FindChild(childName);

		if(so == nullptr)
			break;
	}

	return so;
}

HSceneObject SceneObject::FindChild(const String& name, bool recursive)
{
	for(auto& child : mChildren)
	{
		if(child->GetName() == name)
			return child;
	}

	if(recursive)
	{
		for(auto& child : mChildren)
		{
			HSceneObject foundObject = child->FindChild(name, true);
			if(foundObject != nullptr)
				return foundObject;
		}
	}

	return HSceneObject();
}

Vector<HSceneObject> SceneObject::FindChildren(const String& name, bool recursive)
{
	std::function<void(const HSceneObject&, Vector<HSceneObject>&)> findChildrenInternal =
		[&](const HSceneObject& so, Vector<HSceneObject>& output)
	{
		for(auto& child : so->mChildren)
		{
			if(child->GetName() == name)
				output.push_back(child);
		}

		if(recursive)
		{
			for(auto& child : so->mChildren)
				findChildrenInternal(child, output);
		}
	};

	Vector<HSceneObject> output;
	findChildrenInternal(GetHandle(), output);

	return output;
}

void SceneObject::SetActive(bool active)
{
	if(active)
		UnsetGameObjectFlag(GameObjectPersistentFlag::DisabledSelf);
	else
		SetGameObjectFlag(GameObjectPersistentFlag::DisabledSelf);

	SetActiveHierarchy(active);
}

void SceneObject::SetActiveHierarchy(bool active, bool triggerEvents)
{
	const bool oldEnabledHierarchy = !HasGameObjectFlag(GameObjectTransientFlag::Disabled);
	const bool enabledSelf = !HasGameObjectFlag(GameObjectPersistentFlag::DisabledSelf);
	const bool newEnabledHierarchy = active && enabledSelf;

	if(oldEnabledHierarchy != newEnabledHierarchy)
	{
		if(newEnabledHierarchy)
			UnsetGameObjectFlag(GameObjectTransientFlag::Disabled);
		else
			SetGameObjectFlag(GameObjectTransientFlag::Disabled);

		for(auto& component : mComponents)
			component->RefreshEnabledState(triggerEvents);
	}

	for(auto child : mChildren)
	{
		child->SetActiveHierarchy(newEnabledHierarchy, triggerEvents);
	}
}

bool SceneObject::GetActive(bool self) const
{
	if(self)
		return !HasGameObjectFlag(GameObjectPersistentFlag::DisabledSelf);
	else
		return !HasGameObjectFlag(GameObjectTransientFlag::Disabled);
}

ObjectMobility SceneObject::GetMobility() const
{
	if(mECSRegistry->HasAllOf<ecs::Immovable>(mECSEntity))
		return ObjectMobility::Immovable;

	if(mECSRegistry->HasAllOf<ecs::Static>(mECSEntity))
		return ObjectMobility::Static;

	return ObjectMobility::Movable;
}

void SceneObject::AddMobilityTag(ObjectMobility mobility)
{
	switch (mobility)
	{
	case ObjectMobility::Movable:
		mECSRegistry->AddTag<ecs::Movable>(mECSEntity);
		break;
	case ObjectMobility::Immovable:
		mECSRegistry->AddTag<ecs::Immovable>(mECSEntity);
		break;
	case ObjectMobility::Static:
		mECSRegistry->AddTag<ecs::Static>(mECSEntity);
		break;
	}
}

void SceneObject::SetMobility(ObjectMobility mobility)
{
	ObjectMobility currentMobility = GetMobility();
	if(currentMobility == mobility)
		return;

	mECSRegistry->RemoveTag<ecs::Movable>(mECSEntity);
	mECSRegistry->RemoveTag<ecs::Immovable>(mECSEntity);
	mECSRegistry->RemoveTag<ecs::Static>(mECSEntity);
	AddMobilityTag(mobility);

	// If mobility changed to movable, update both the mobility flag and transform, otherwise just mobility
	if(mobility == ObjectMobility::Movable)
		NotifyTransformChanged((TransformChangedFlags)(TCF_Transform | TCF_Mobility));
	else
		NotifyTransformChanged(TCF_Mobility);
}

HSceneObject SceneObject::Clone()
{
	HSceneObject cloneParent = GetParent();
	if(!cloneParent.IsValid())
	{
		const SPtr<SceneInstance>& sceneInstance = mParentScene.lock();
		if(sceneInstance != nullptr)
			cloneParent = sceneInstance->GetRoot();
	}

	if(!cloneParent.IsValid())
	{
		B3D_LOG(Error, LogScene, "Cannot clone scene object {0} ({1}). Cannot find a scene object to parent the clone to.", GetName(), GetId());
		return HSceneObject();
	}

	HSceneObject clone = Clone(GetOwnerCollection().lock(), false);
	clone->SetParent(cloneParent);

	if(HasGameObjectFlag(GameObjectTransientFlag::Initialized))
		clone->Initialize();

	return clone;
}

HSceneObject SceneObject::Clone(const SPtr<GameObjectCollection>& cloneOwnerCollection, bool preserveIds)
{
	if(!B3D_ENSURE(cloneOwnerCollection))
		return HSceneObject();

	SPtr<MemoryDataStream> stream = B3DMakeShared<MemoryDataStream>();
	BinarySerializer serializer;
	serializer.Encode(this, stream);

	B3D_ENSURE(!preserveIds || cloneOwnerCollection != mOwnerCollection.lock());

	RTTIOperationEngineContext rttiOperationContext;
	rttiOperationContext.PreserveGameObjectIds = preserveIds;
	rttiOperationContext.GameObjectCollection = cloneOwnerCollection;

	stream->Seek(0);
	SPtr<SceneObject> clone = std::static_pointer_cast<SceneObject>(serializer.Decode(stream, (u32)stream->Size(), rttiOperationContext));

	// Clear the parent of the clone, as it will belong to the original game object collection, which is not valid
	clone->mParent = nullptr;

	ecs::Parent& parentFragment = clone->mECSRegistry->GetOrAddComponent<ecs::Parent>(clone->mECSEntity);
	parentFragment.Entity = ecs::kNullEntity;

	return clone->GetHandle();
}

HSceneObject SceneObject::Clone(const SPtr<SceneInstance>& cloneSceneInstance, bool initialize, bool preserveIds)
{
	if(!B3D_ENSURE(cloneSceneInstance))
		return HSceneObject();

	SPtr<MemoryDataStream> stream = B3DMakeShared<MemoryDataStream>();
	BinarySerializer serializer;
	serializer.Encode(this, stream);

	B3D_ENSURE(!preserveIds || cloneSceneInstance->GetGameObjectCollection() != mOwnerCollection.lock());

	RTTIOperationEngineContext rttiOperationContext;
	rttiOperationContext.PreserveGameObjectIds = preserveIds;
	rttiOperationContext.GameObjectCollection = cloneSceneInstance->GetGameObjectCollection();

	stream->Seek(0);
	SPtr<SceneObject> clone = std::static_pointer_cast<SceneObject>(serializer.Decode(stream, (u32)stream->Size(), rttiOperationContext));
	clone->SetParent(cloneSceneInstance->GetRoot());

	if(initialize)
		clone->Initialize();

	return clone->GetHandle();
}

HComponent SceneObject::GetComponent(RTTIType* type) const
{
	if(type != Component::GetRttiStatic())
	{
		for(auto& entry : mComponents)
		{
			if(entry->GetRtti()->IsDerivedFrom(type))
				return entry;
		}
	}

	return HComponent();
}

void SceneObject::RemoveComponent(const HComponent& component)
{
	if(component == nullptr)
	{
		B3D_LOG(Warning, LogScene, "Trying to remove a null component");
		return;
	}

	if(auto found = std::find(mComponents.begin(), mComponents.end(), component); found != mComponents.end())
		mComponents.erase(found);
	else
		B3D_LOG(Warning, LogScene, "Trying to remove a component that doesn't exist on this SceneObject.");
}

HComponent SceneObject::AddComponent(u32 typeId)
{
	SPtr<IReflectable> newObj = B3DRTTICreate(typeId);

	if(!B3DRTTIIsSubclass<Component>(newObj.get()))
	{
		B3D_LOG(Error, LogScene, "Specified type is not a valid Component.");
		return HComponent();
	}

	SPtr<Component> component = std::static_pointer_cast<Component>(newObj);

	// Clean up the self-reference assigned by the RTTI system
	component->mRTTIData = nullptr;

	HComponent componentHandle = RegisterComponentWithOwnerCollection(component);
	InternalAddComponent(componentHandle, true);
	return componentHandle;
}

HComponent SceneObject::RegisterComponentWithOwnerCollection(const SPtr<Component>& component)
{
	component->SetId(UUIDGenerator::GenerateRandom());

	const SPtr<GameObjectCollection>& ownerCollection = mOwnerCollection.lock();
	if(!B3D_ENSURE(ownerCollection != nullptr))
		return HComponent();

	return B3DStaticGameObjectCast<Component>(ownerCollection->RegisterNewObject(component));
}

void SceneObject::InternalAddComponent(const SPtr<Component>& component, bool initialize)
{
	const SPtr<GameObjectCollection>& ownerCollection = mOwnerCollection.lock();
	if(!B3D_ENSURE(ownerCollection != nullptr))
		return;

	const HComponent& componentHandle = B3DStaticGameObjectCast<Component>(ownerCollection->GetObject(component->GetId()));
	InternalAddComponent(componentHandle, initialize);
}

void SceneObject::InternalAddComponent(const HComponent& component, bool initialize)
{
	component->mParent = GetHandle();
	component->mThisHandle = component;

	if(component->mId.Empty())
		component->mId = UUIDGenerator::GenerateRandom();

	mComponents.push_back(component);

	if(initialize && HasGameObjectFlag(GameObjectTransientFlag::Initialized))
	{
		component->Initialize();

		const SPtr<SceneInstance>& scene = GetScene();
		scene->NotifyComponentCreated(component, GetActive());
	}
}

RTTIType* SceneObject::GetRttiStatic()
{
	return SceneObjectRTTI::Instance();
}

RTTIType* SceneObject::GetRtti() const
{
	return SceneObject::GetRttiStatic();
}
