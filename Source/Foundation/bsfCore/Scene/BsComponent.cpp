//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Scene/BsComponent.h"
#include "BsSceneManager.h"
#include "Scene/BsSceneObject.h"
#include "Private/RTTI/BsComponentRTTI.h"

using namespace bs;

Component::Component(HSceneObject parent)
	: mParent(std::move(parent))
{
	SetName("Component");
}

bool Component::TypeEquals(const Component& other)
{
	return GetRtti()->GetRttiId() == other.GetRtti()->GetRttiId();
}

bool Component::CalculateBounds(Bounds& bounds)
{
	Vector3 position = SO()->GetTransform().GetPosition();

	bounds = Bounds(AABox(position, position), Sphere(position, 0.0f));
	return false;
}

void Component::Destroy(bool immediate)
{
	if(HasGameObjectFlag(GameObjectTransientFlag::QueuedForDestroy))
		return;

	if(!B3D_ENSURE(!HasGameObjectFlag(GameObjectTransientFlag::Destroyed)))
		return;

	HComponent thisComponentHandle = B3DStaticGameObjectCast<Component>(mThisHandle);
	mParent->RemoveComponent(thisComponentHandle);
	mParent = nullptr;

	if(immediate)
		DestroyImmediate();
	else
		QueueForDestroy();
}

void Component::DestroyImmediate()
{
	if(!B3D_ENSURE(!HasGameObjectFlag(GameObjectTransientFlag::Destroyed)))
		return;

	if(!HasGameObjectFlag(GameObjectTransientFlag::QueuedForDestroy))
	{
		if(HasGameObjectFlag(GameObjectTransientFlag::Initialized))
		{
			HComponent thisComponentHandle = B3DStaticGameObjectCast<Component>(mThisHandle);
			GetSceneManager().NotifyComponentDestroyedInternal(thisComponentHandle, true);
		}
	}

	mParent = nullptr;
	GameObject::DestroyImmediate();
}

void Component::QueueForDestroy()
{
	if(HasGameObjectFlag(GameObjectTransientFlag::QueuedForDestroy))
		return;

	if(!B3D_ENSURE(!HasGameObjectFlag(GameObjectTransientFlag::Destroyed)))
		return;

	if(HasGameObjectFlag(GameObjectTransientFlag::Initialized))
	{
		HComponent thisComponentHandle = B3DStaticGameObjectCast<Component>(mThisHandle);
		GetSceneManager().NotifyComponentDestroyedInternal(thisComponentHandle, true);
	}

	GameObject::QueueForDestroy();
}

void Component::Initialize()
{
	SetGameObjectFlag(GameObjectTransientFlag::Initialized);
}

bool Component::GetEnabled(bool self) const
{
	if(self)
		return !HasGameObjectFlag(GameObjectPersistentFlag::DisabledSelf);
	else
		return !HasGameObjectFlag(GameObjectTransientFlag::Disabled);
}

void Component::SetEnabled(bool enabled)
{
	const bool isEnabledSelf = !HasGameObjectFlag(GameObjectPersistentFlag::DisabledSelf);
	if(isEnabledSelf == enabled)
		return;

	if(enabled)
		UnsetGameObjectFlag(GameObjectPersistentFlag::DisabledSelf);
	else
		SetGameObjectFlag(GameObjectPersistentFlag::DisabledSelf);

	RefreshEnabledState();
}

void Component::RefreshEnabledState(bool triggerEvents)
{
	HComponent thisComponentHandle = B3DStaticGameObjectCast<Component>(mThisHandle);
	const bool isParentEnabled = mParent.IsValid() && mParent->GetActive();
	const bool isSelfEnabled = !HasGameObjectFlag(GameObjectPersistentFlag::DisabledSelf);
	const bool oldIsHierarchyEnabled = !HasGameObjectFlag(GameObjectTransientFlag::Disabled);
	const bool newIsHierarchyEnabled = isParentEnabled && isSelfEnabled;
	if(oldIsHierarchyEnabled != newIsHierarchyEnabled)
	{
		if(newIsHierarchyEnabled)
		{
			UnsetGameObjectFlag(GameObjectTransientFlag::Disabled);

			if(triggerEvents) // Note: Not sure this check makes sense, but keeping it to maintain behaviour from SceneObject. Same for below.
				GetSceneManager().NotifyComponentActivatedInternal(thisComponentHandle, triggerEvents);
		}
		else
		{
			SetGameObjectFlag(GameObjectTransientFlag::Disabled);

			if(triggerEvents)
				GetSceneManager().NotifyComponentDeactivatedInternal(thisComponentHandle, triggerEvents);
			
		}
	}
}

RTTITypeBase* Component::GetRttiStatic()
{
	return ComponentRTTI::Instance();
}

RTTITypeBase* Component::GetRtti() const
{
	return Component::GetRttiStatic();
}
