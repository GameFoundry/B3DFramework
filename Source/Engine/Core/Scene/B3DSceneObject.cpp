//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Scene/B3DSceneObject.h"

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
		B3D_LOG(Warning, Scene, "Object is being deleted without being destroyed first? {0}", mName);
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
			B3D_LOG(Warning, Scene, "Cannot instantiate scene object. No associated scene found.");
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

void SceneObject::SetLocalTransform(const Transform& transform)
{
	if(mMobility != ObjectMobility::Movable)
		return;

	mLocalTfrm = transform;
	NotifyTransformChanged(TCF_Transform);
}

void SceneObject::SetPosition(const Vector3& position)
{
	if(mMobility == ObjectMobility::Movable)
	{
		mLocalTfrm.SetPosition(position);
		NotifyTransformChanged(TCF_Transform);
	}
}

void SceneObject::SetRotation(const Quaternion& rotation)
{
	if(mMobility == ObjectMobility::Movable)
	{
		mLocalTfrm.SetRotation(rotation);
		NotifyTransformChanged(TCF_Transform);
	}
}

void SceneObject::SetScale(const Vector3& scale)
{
	if(mMobility == ObjectMobility::Movable)
	{
		mLocalTfrm.SetScale(scale);
		NotifyTransformChanged(TCF_Transform);
	}
}

void SceneObject::SetWorldPosition(const Vector3& position)
{
	if(mMobility != ObjectMobility::Movable)
		return;

	if(mParent != nullptr)
		mLocalTfrm.SetWorldPosition(position, mParent->GetTransform());
	else
		mLocalTfrm.SetPosition(position);

	NotifyTransformChanged(TCF_Transform);
}

void SceneObject::SetWorldRotation(const Quaternion& rotation)
{
	if(mMobility != ObjectMobility::Movable)
		return;

	if(mParent != nullptr)
		mLocalTfrm.SetWorldRotation(rotation, mParent->GetTransform());
	else
		mLocalTfrm.SetRotation(rotation);

	NotifyTransformChanged(TCF_Transform);
}

void SceneObject::SetWorldScale(const Vector3& scale)
{
	if(mMobility != ObjectMobility::Movable)
		return;

	if(mParent != nullptr)
		mLocalTfrm.SetWorldScale(scale, mParent->GetTransform());
	else
		mLocalTfrm.SetScale(scale);

	NotifyTransformChanged(TCF_Transform);
}

const Transform& SceneObject::GetTransform() const
{
	if(!IsCachedWorldTransformUpToDate())
		UpdateWorldTfrm();

	return mWorldTfrm;
}

void SceneObject::LookAt(const Vector3& location, const Vector3& up)
{
	const Transform& worldTfrm = GetTransform();

	Vector3 forward = location - worldTfrm.GetPosition();

	Quaternion rotation = worldTfrm.GetRotation();
	rotation.LookRotation(forward, up);
	SetWorldRotation(rotation);
}

const Matrix4& SceneObject::GetWorldMatrix() const
{
	if(!IsCachedWorldTransformUpToDate())
		UpdateWorldTfrm();

	return mCachedWorldTfrm;
}

Matrix4 SceneObject::GetInvWorldMatrix() const
{
	if(!IsCachedWorldTransformUpToDate())
		UpdateWorldTfrm();

	Matrix4 worldToLocal = mWorldTfrm.GetInvMatrix();
	return worldToLocal;
}

const Matrix4& SceneObject::GetLocalMatrix() const
{
	if(!IsCachedLocalTransformUpToDate())
		UpdateLocalTfrm();

	return mCachedLocalTfrm;
}

void SceneObject::Move(const Vector3& vec)
{
	if(mMobility == ObjectMobility::Movable)
	{
		mLocalTfrm.Move(vec);
		NotifyTransformChanged(TCF_Transform);
	}
}

void SceneObject::MoveRelative(const Vector3& vec)
{
	if(mMobility == ObjectMobility::Movable)
	{
		mLocalTfrm.MoveRelative(vec);
		NotifyTransformChanged(TCF_Transform);
	}
}

void SceneObject::Rotate(const Vector3& axis, const Radian& angle)
{
	if(mMobility == ObjectMobility::Movable)
	{
		mLocalTfrm.Rotate(axis, angle);
		NotifyTransformChanged(TCF_Transform);
	}
}

void SceneObject::Rotate(const Quaternion& q)
{
	if(mMobility == ObjectMobility::Movable)
	{
		mLocalTfrm.Rotate(q);
		NotifyTransformChanged(TCF_Transform);
	}
}

void SceneObject::Roll(const Radian& angle)
{
	if(mMobility == ObjectMobility::Movable)
	{
		mLocalTfrm.Roll(angle);
		NotifyTransformChanged(TCF_Transform);
	}
}

void SceneObject::Yaw(const Radian& angle)
{
	if(mMobility == ObjectMobility::Movable)
	{
		mLocalTfrm.Yaw(angle);
		NotifyTransformChanged(TCF_Transform);
	}
}

void SceneObject::Pitch(const Radian& angle)
{
	if(mMobility == ObjectMobility::Movable)
	{
		mLocalTfrm.Pitch(angle);
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

void SceneObject::UpdateTransformsIfDirty()
{
	if(!IsCachedLocalTransformUpToDate())
		UpdateLocalTfrm();

	if(!IsCachedWorldTransformUpToDate())
		UpdateWorldTfrm();
}

void SceneObject::NotifyTransformChanged(TransformChangedFlags flags) const
{
	// If object is immovable, don't send transform changed events nor mark the transform dirty
	TransformChangedFlags componentFlags = flags;
	if(mMobility != ObjectMobility::Movable)
		componentFlags = (TransformChangedFlags)(componentFlags & ~TCF_Transform);
	else
	{
		mDirtyFlags |= DirtyFlags::LocalTransformDirty | DirtyFlags::WorldTransformDirty;
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

void SceneObject::UpdateWorldTfrm() const
{
	mWorldTfrm = mLocalTfrm;

	// Don't allow movement from parent when not movable
	if(mParent != nullptr && mMobility == ObjectMobility::Movable)
	{
		mWorldTfrm.MakeWorld(mParent->GetTransform());

		mCachedWorldTfrm = mWorldTfrm.GetMatrix();
	}
	else
	{
		mCachedWorldTfrm = GetLocalMatrix();
	}

	mDirtyFlags &= ~DirtyFlags::WorldTransformDirty;
}

void SceneObject::UpdateLocalTfrm() const
{
	mCachedLocalTfrm = mLocalTfrm.GetMatrix();
	mDirtyFlags &= ~DirtyFlags::LocalTransformDirty;
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

	if(mMobility != ObjectMobility::Movable)
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
			parent->AddChild(GetHandle());
			SetScene(parent->GetScene(), true);
		}
		else
			SetScene(nullptr, true);

		mParent = parent;

		if(keepWorldTransform)
		{
			mLocalTfrm = worldTfrm;

			if(mParent != nullptr)
				mLocalTfrm.MakeLocal(mParent->GetTransform());
		}

		if(const bool isInitialized = HasGameObjectFlag(GameObjectTransientFlag::Initialized))
			NotifyTransformChanged((TransformChangedFlags)(TCF_Parent | TCF_Transform));
	}
}

void SceneObject::ClearParent()
{
	if(mParent != nullptr)
		mParent->RemoveChild(GetHandle());

	SetScene(nullptr, true);
	mParent = nullptr;
}

void SceneObject::SetScene(const SPtr<SceneInstance>& scene, bool recursive)
{
	const SPtr<SceneInstance> currentScene = mParentScene.lock();
	if(currentScene == scene)
		return;

	mParentScene = scene;

	if(scene != nullptr)
		SetOwnerCollection(scene->GetGameObjectCollection());

	if(recursive)
	{
		for(auto& child : mChildren)
			child->SetScene(scene, true);
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
}

void SceneObject::RemoveChild(const HSceneObject& object)
{
	auto result = find(mChildren.begin(), mChildren.end(), object);

	if(result != mChildren.end())
		mChildren.erase(result);
	else
		B3D_LOG(Warning, Scene, "Trying to remove a child but it's not a child of the transform.");
}

HSceneObject SceneObject::FindPath(const String& path) const
{
	if(path.empty())
		return HSceneObject();

	String trimmedPath = StringUtil::Trim(path, "/");

	Vector<String> entries = StringUtil::Split(trimmedPath, "/");

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

void SceneObject::SetMobility(ObjectMobility mobility)
{
	if(mMobility != mobility)
	{
		mMobility = mobility;

		// If mobility changed to movable, update both the mobility flag and transform, otherwise just mobility
		if(mMobility == ObjectMobility::Movable)
			NotifyTransformChanged((TransformChangedFlags)(TCF_Transform | TCF_Mobility));
		else
			NotifyTransformChanged(TCF_Mobility);
	}
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
		B3D_LOG(Error, Scene, "Cannot clone scene object {0} ({1}). Cannot find a scene object to parent the clone to.", GetName(), GetId());
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
		B3D_LOG(Warning, Scene, "Trying to remove a null component");
		return;
	}

	if(auto found = std::find(mComponents.begin(), mComponents.end(), component); found != mComponents.end())
		mComponents.erase(found);
	else
		B3D_LOG(Warning, Scene, "Trying to remove a component that doesn't exist on this SceneObject.");
}

HComponent SceneObject::AddComponent(u32 typeId)
{
	SPtr<IReflectable> newObj = B3DRTTICreate(typeId);

	if(!B3DRTTIIsSubclass<Component>(newObj.get()))
	{
		B3D_LOG(Error, Scene, "Specified type is not a valid Component.");
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
