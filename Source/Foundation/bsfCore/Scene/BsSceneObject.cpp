//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Scene/BsSceneObject.h"
#include "Scene/BsComponent.h"
#include "Scene/BsSceneManager.h"
#include "Error/BsException.h"
#include "Debug/BsDebug.h"
#include "Private/RTTI/BsSceneObjectRTTI.h"
#include "Serialization/BsBinarySerializer.h"
#include "FileSystem/BsDataStream.h"
#include "Scene/BsGameObjectManager.h"
#include "Scene/BsPrefabUtility.h"
#include "Math/BsMatrix3.h"
#include "BsCoreApplication.h"

using namespace bs;

SceneObject::SceneObject(const String& name, u32 flags)
	: GameObject(), mFlags(flags)
{
	SetName(name);
}

SceneObject::~SceneObject()
{
	if(!mThisHandle.IsDestroyed())
	{
		BS_LOG(Warning, Scene, "Object is being deleted without being destroyed first? {0}", mName);
		DestroyInternal(mThisHandle, true);
	}
}

HSceneObject SceneObject::Create(const String& name, u32 flags)
{
	HSceneObject newObject = CreateInternal(name, flags);

	if(newObject->IsInstantiated())
		GetSceneManager().RegisterNewSo(newObject);

	return newObject;
}

HSceneObject SceneObject::CreateInternal(const String& name, u32 flags)
{
	SPtr<SceneObject> sceneObjectPtr = SPtr<SceneObject>(new(bs_alloc<SceneObject>()) SceneObject(name, flags), &bs_delete<SceneObject>, StdAlloc<SceneObject>());
	sceneObjectPtr->mUUID = UUIDGenerator::GenerateRandom();

	HSceneObject sceneObject = static_object_cast<SceneObject>(
		GameObjectManager::Instance().RegisterObject(sceneObjectPtr));
	sceneObject->mThisHandle = sceneObject;

	return sceneObject;
}

HSceneObject SceneObject::CreateInternal(const SPtr<SceneObject>& soPtr)
{
	HSceneObject sceneObject = static_object_cast<SceneObject>(
		GameObjectManager::Instance().RegisterObject(soPtr));
	sceneObject->mThisHandle = sceneObject;

	return sceneObject;
}

void SceneObject::Destroy(bool immediate)
{
	// Parent is our owner, so when his reference to us is removed, delete might be called.
	// So make sure this is the last thing we do.
	if(mParent != nullptr)
	{
		if(!mParent.IsDestroyed())
			mParent->RemoveChild(mThisHandle);

		mParent = nullptr;
	}

	DestroyInternal(mThisHandle, immediate);
}

void SceneObject::DestroyInternal(GameObjectHandleBase& handle, bool immediate)
{
	if(immediate)
	{
		for(auto iter = mChildren.begin(); iter != mChildren.end(); ++iter)
			(*iter)->DestroyInternal(*iter, true);

		mChildren.clear();

		// It's important to remove the elements from the array as soon as they're destroyed, as OnDestroy callbacks
		// for components might query the SO's components, and we want to only return live ones
		while(!mComponents.empty())
		{
			HComponent component = mComponents.back();
			component->SetIsDestroyedInternal();

			if(IsInstantiated())
				GetSceneManager().NotifyComponentDestroyedInternal(component, immediate);

			component->DestroyInternal(component, true);
			mComponents.erase(mComponents.end() - 1);
		}

		GameObjectManager::Instance().UnregisterObject(handle);
	}
	else
		GameObjectManager::Instance().QueueForDestroy(handle);
}

void SceneObject::SetInstanceDataInternal(GameObjectInstanceDataPtr& other)
{
	GameObject::SetInstanceDataInternal(other);

	// Instance data changed, so make sure to refresh the handles to reflect that
	SPtr<SceneObject> thisPtr = mThisHandle.GetInternalPtr();
	mThisHandle.SetHandleDataInternal(thisPtr);
}

UUID SceneObject::GetPrefabLink(bool onlyDirect) const
{
	const SceneObject* curObj = this;

	while(curObj != nullptr)
	{
		if(!curObj->mPrefabLinkUUID.Empty())
			return curObj->mPrefabLinkUUID;

		if(curObj->mParent != nullptr && !onlyDirect)
			curObj = curObj->mParent.Get();
		else
			curObj = nullptr;
	}

	return UUID::kEmpty;
}

HSceneObject SceneObject::GetPrefabParent() const
{
	HSceneObject curObj = mThisHandle;

	while(curObj != nullptr)
	{
		if(!curObj->mPrefabLinkUUID.Empty())
			return curObj;

		if(curObj->mParent != nullptr)
			curObj = curObj->mParent;
		else
			curObj = nullptr;
	}

	return curObj;
}

void SceneObject::BreakPrefabLink()
{
	SceneObject* rootObj = this;

	while(rootObj != nullptr)
	{
		if(!rootObj->mPrefabLinkUUID.Empty())
			break;

		if(rootObj->mParent != nullptr)
			rootObj = rootObj->mParent.Get();
		else
			rootObj = nullptr;
	}

	if(rootObj != nullptr)
	{
		rootObj->mPrefabLinkUUID = UUID::kEmpty;
		rootObj->mPrefabDiff = nullptr;
		PrefabUtility::ClearPrefabIds(rootObj->GetHandle(), true, false);
	}
}

bool SceneObject::HasFlag(u32 flag) const
{
	return (mFlags & flag) != 0;
}

void SceneObject::SetFlagsInternal(u32 flags)
{
	mFlags |= flags;

	for(auto& child : mChildren)
		child->SetFlagsInternal(flags);
}

void SceneObject::UnsetFlagsInternal(u32 flags)
{
	mFlags &= ~flags;

	for(auto& child : mChildren)
		child->UnsetFlagsInternal(flags);
}

void SceneObject::InstantiateInternal(bool prefabOnly)
{
	std::function<void(SceneObject*)> instantiateRecursive = [&](SceneObject* obj)
	{
		obj->mFlags &= ~SOF_DontInstantiate;

		if(obj->mParent == nullptr)
			GetSceneManager().RegisterNewSo(obj->mThisHandle);

		for(auto& component : obj->mComponents)
			component->InstantiateInternal();

		for(auto& child : obj->mChildren)
		{
			if(!prefabOnly || child->mPrefabLinkUUID.Empty())
				instantiateRecursive(child.Get());
		}
	};

	std::function<void(SceneObject*)> triggerEventsRecursive = [&](SceneObject* obj)
	{
		for(auto& component : obj->mComponents)
			GetSceneManager().NotifyComponentCreatedInternal(component, obj->GetActive());

		for(auto& child : obj->mChildren)
		{
			if(!prefabOnly || child->mPrefabLinkUUID.Empty())
				triggerEventsRecursive(child.Get());
		}
	};

	instantiateRecursive(this);
	triggerEventsRecursive(this);
}

/************************************************************************/
/* 								Transform	                     		*/
/************************************************************************/

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
	if(!IsCachedWorldTfrmUpToDate())
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
	if(!IsCachedWorldTfrmUpToDate())
		UpdateWorldTfrm();

	return mCachedWorldTfrm;
}

Matrix4 SceneObject::GetInvWorldMatrix() const
{
	if(!IsCachedWorldTfrmUpToDate())
		UpdateWorldTfrm();

	Matrix4 worldToLocal = mWorldTfrm.GetInvMatrix();
	return worldToLocal;
}

const Matrix4& SceneObject::GetLocalMatrix() const
{
	if(!IsCachedLocalTfrmUpToDate())
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
	if(!IsCachedLocalTfrmUpToDate())
		UpdateLocalTfrm();

	if(!IsCachedWorldTfrmUpToDate())
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
		mDirtyFlags |= DirtyFlags::LocalTfrmDirty | DirtyFlags::WorldTfrmDirty;
		mDirtyHash++;
	}

	// Only send component flags if we haven't removed them all
	if(componentFlags != 0)
	{
		for(auto& entry : mComponents)
		{
			if(entry->SupportsNotify(flags))
			{
				bool alwaysRun = entry->HasFlag(ComponentFlag::AlwaysRun);
				if(alwaysRun || GetSceneManager().IsRunning())
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

	mDirtyFlags &= ~DirtyFlags::WorldTfrmDirty;
}

void SceneObject::UpdateLocalTfrm() const
{
	mCachedLocalTfrm = mLocalTfrm.GetMatrix();
	mDirtyFlags &= ~DirtyFlags::LocalTfrmDirty;
}

/************************************************************************/
/* 								Hierarchy	                     		*/
/************************************************************************/

void SceneObject::SetParent(const HSceneObject& parent, bool keepWorldTransform)
{
	if(parent.IsDestroyed())
		return;

#if BS_IS_BANSHEE3D
	UUID originalPrefab = GetPrefabLink();
#endif

	if(mMobility != ObjectMobility::Movable)
		keepWorldTransform = true;

	SetParentInternal(parent, keepWorldTransform);

#if BS_IS_BANSHEE3D
	if(GetCoreApplication().IsEditor())
	{
		UUID newPrefab = GetPrefabLink();
		if(originalPrefab != newPrefab)
			PrefabUtility::ClearPrefabIds(mThisHandle);
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
			mParent->RemoveChild(mThisHandle);

		if(parent != nullptr)
		{
			parent->AddChild(mThisHandle);
			SetScene(parent->mParentScene);
		}
		else
			SetScene(nullptr);

		mParent = parent;

		if(keepWorldTransform)
		{
			mLocalTfrm = worldTfrm;

			if(mParent != nullptr)
				mLocalTfrm.MakeLocal(mParent->GetTransform());
		}

		bool isInstantiated = (mFlags & SOF_DontInstantiate) == 0;
		if(isInstantiated)
			NotifyTransformChanged((TransformChangedFlags)(TCF_Parent | TCF_Transform));
	}
}

const SPtr<SceneInstance>& SceneObject::GetScene() const
{
	if(mParentScene)
		return mParentScene;

	BS_LOG(Warning, Scene, "Attempting to access a scene of a SceneObject with no scene, returning main scene instead.");
	return GetSceneManager().GetMainScene();
}

void SceneObject::SetScene(const SPtr<SceneInstance>& scene)
{
	if(mParentScene == scene)
		return;

	mParentScene = scene;

	for(auto& child : mChildren)
		child->SetScene(scene);
}

HSceneObject SceneObject::GetChild(u32 idx) const
{
	if(idx >= mChildren.size())
	{
		BS_EXCEPT(InternalErrorException, "Child index out of range.");
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

void SceneObject::AddChild(const HSceneObject& object)
{
	mChildren.push_back(object);

	object->SetFlagsInternal(mFlags);
}

void SceneObject::RemoveChild(const HSceneObject& object)
{
	auto result = find(mChildren.begin(), mChildren.end(), object);

	if(result != mChildren.end())
		mChildren.erase(result);
	else
	{
		BS_EXCEPT(InternalErrorException, "Trying to remove a child but it's not a child of the transform.");
	}
}

HSceneObject SceneObject::FindPath(const String& path) const
{
	if(path.empty())
		return HSceneObject();

	String trimmedPath = path;
	StringUtil::Trim(trimmedPath, "/");

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
	findChildrenInternal(mThisHandle, output);

	return output;
}

void SceneObject::SetActive(bool active)
{
	mActiveSelf = active;
	SetActiveHierarchy(active);
}

void SceneObject::SetActiveHierarchy(bool active, bool triggerEvents)
{
	bool activeHierarchy = active && mActiveSelf;

	if(mActiveHierarchy != activeHierarchy)
	{
		mActiveHierarchy = activeHierarchy;

		if(triggerEvents)
		{
			if(activeHierarchy)
			{
				for(auto& component : mComponents)
					GetSceneManager().NotifyComponentActivatedInternal(component, triggerEvents);
			}
			else
			{
				for(auto& component : mComponents)
					GetSceneManager().NotifyComponentDeactivatedInternal(component, triggerEvents);
			}
		}
	}

	for(auto child : mChildren)
	{
		child->SetActiveHierarchy(mActiveHierarchy, triggerEvents);
	}
}

bool SceneObject::GetActive(bool self) const
{
	if(self)
		return mActiveSelf;
	else
		return mActiveHierarchy;
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

HSceneObject SceneObject::Clone(bool instantiate, bool preserveUUIDs)
{
	const bool isInstantiated = !HasFlag(SOF_DontInstantiate);

	if(!instantiate)
		SetFlagsInternal(SOF_DontInstantiate);
	else
		UnsetFlagsInternal(SOF_DontInstantiate);

	SPtr<MemoryDataStream> stream = bs_shared_ptr_new<MemoryDataStream>();
	BinarySerializer serializer;
	serializer.Encode(this, stream);

	int flags = GODM_RestoreExternal | GODM_UseNewIds;
	if(!preserveUUIDs)
		flags |= GODM_UseNewUUID;

	CoreSerializationContext serzContext;
	serzContext.GoState = bs_shared_ptr_new<GameObjectDeserializationState>(flags);

	stream->Seek(0);
	SPtr<SceneObject> cloneObj = std::static_pointer_cast<SceneObject>(
		serializer.Decode(stream, (u32)stream->Size(), BinarySerializerFlag::None, &serzContext));

	if(isInstantiated)
		UnsetFlagsInternal(SOF_DontInstantiate);
	else
		SetFlagsInternal(SOF_DontInstantiate);

	return cloneObj->mThisHandle;
}

HComponent SceneObject::GetComponent(RTTITypeBase* type) const
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

void SceneObject::DestroyComponent(const HComponent component, bool immediate)
{
	if(component == nullptr)
	{
		BS_LOG(Warning, Scene, "Trying to remove a null component");
		return;
	}

	auto iter = std::find(mComponents.begin(), mComponents.end(), component);

	if(iter != mComponents.end())
	{
		(*iter)->SetIsDestroyedInternal();

		if(IsInstantiated())
			GetSceneManager().NotifyComponentDestroyedInternal(*iter, immediate);

		(*iter)->DestroyInternal(*iter, immediate);
		mComponents.erase(iter);
	}
	else
		BS_LOG(Warning, Scene, "Trying to remove a component that doesn't exist on this SceneObject.");
}

void SceneObject::DestroyComponent(Component* component, bool immediate)
{
	auto iterFind = std::find_if(mComponents.begin(), mComponents.end(), [component](const HComponent& x)
								 {
			if(x.IsDestroyed())
				return false;

			return x.GetHandleDataInternal()->MPtr->Object.get() == component; });

	if(iterFind != mComponents.end())
	{
		DestroyComponent(*iterFind, immediate);
	}
}

HComponent SceneObject::AddComponent(u32 typeId)
{
	SPtr<IReflectable> newObj = rtti_create(typeId);

	if(!rtti_is_subclass<Component>(newObj.get()))
	{
		BS_LOG(Error, Scene, "Specified type is not a valid Component.");
		return HComponent();
	}

	SPtr<Component> componentPtr = std::static_pointer_cast<Component>(newObj);

	// Clean up the self-reference assigned by the RTTI system
	componentPtr->mRTTIData = nullptr;

	HComponent newComponent = static_object_cast<Component>(GameObjectManager::Instance().RegisterObject(componentPtr));
	newComponent->mParent = mThisHandle;

	AddAndInitializeComponent(newComponent);
	return newComponent;
}

void SceneObject::AddComponentInternal(const SPtr<Component>& component)
{
	HComponent newComponent = static_object_cast<Component>(
		GameObjectManager::Instance().GetObject(component->GetInstanceId()));
	newComponent->mParent = mThisHandle;
	newComponent->mThisHandle = newComponent;

	mComponents.push_back(newComponent);
}

void SceneObject::AddAndInitializeComponent(const HComponent& component)
{
	component->mThisHandle = component;

	if(component->mUUID.Empty())
		component->mUUID = UUIDGenerator::GenerateRandom();

	mComponents.push_back(component);

	if(IsInstantiated())
	{
		component->InstantiateInternal();

		GetSceneManager().NotifyComponentCreatedInternal(component, GetActive());
	}
}

void SceneObject::AddAndInitializeComponent(const SPtr<Component>& component)
{
	HComponent newComponent = static_object_cast<Component>(
		GameObjectManager::Instance().GetObject(component->GetInstanceId()));
	newComponent->mParent = mThisHandle;

	AddAndInitializeComponent(newComponent);
}

RTTITypeBase* SceneObject::GetRttiStatic()
{
	return SceneObjectRTTI::Instance();
}

RTTITypeBase* SceneObject::GetRtti() const
{
	return SceneObject::GetRttiStatic();
}
