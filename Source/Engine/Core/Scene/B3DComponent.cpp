//************************************ B3D Framework - Copyright 2026 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Scene/B3DComponent.h"
#include "Scene/B3DSceneInstance.h"
#include "Scene/B3DSceneObject.h"
#include "ECS/B3DRegistry.h"
#include "RTTI/B3DComponentRTTI.h"

using namespace b3d;

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

	bounds = Bounds(position, Vector3::kZero, 0.0f);
	return false;
}

void Component::Destroy(bool immediate)
{
	if(HasGameObjectFlag(GameObjectTransientFlag::QueuedForDestroy))
		return;

	if(!B3D_ENSURE(!HasGameObjectFlag(GameObjectTransientFlag::Destroyed)))
		return;

	if(immediate)
		DestroyImmediate(true);
	else
		QueueForDestroy(true);
}

void Component::DestroyImmediate(bool removeFromParent)
{
	if(!B3D_ENSURE(!HasGameObjectFlag(GameObjectTransientFlag::Destroyed)))
		return;

	if(!HasGameObjectFlag(GameObjectTransientFlag::QueuedForDestroy))
	{
		if(HasGameObjectFlag(GameObjectTransientFlag::Initialized))
		{
			HComponent thisComponentHandle = B3DStaticGameObjectCast<Component>(mThisHandle);

			const SPtr<SceneInstance>& scene = SceneObject()->GetScene();
			if(B3D_ENSURE(scene != nullptr))
				scene->NotifyComponentDestroyed(thisComponentHandle, true);
		}
	}

	if(removeFromParent)
	{
		HComponent thisComponentHandle = B3DStaticGameObjectCast<Component>(mThisHandle);
		mParent->RemoveComponent(thisComponentHandle);
	}

	mParent = nullptr;
	GameObject::DestroyImmediate();
}

void Component::QueueForDestroy(bool removeFromParent)
{
	if(HasGameObjectFlag(GameObjectTransientFlag::QueuedForDestroy))
		return;

	if(!B3D_ENSURE(!HasGameObjectFlag(GameObjectTransientFlag::Destroyed)))
		return;

	if(HasGameObjectFlag(GameObjectTransientFlag::Initialized))
	{
		HComponent thisComponentHandle = B3DStaticGameObjectCast<Component>(mThisHandle);

		const SPtr<SceneInstance>& scene = SceneObject()->GetScene();
		if(B3D_ENSURE(scene != nullptr))
			scene->NotifyComponentDestroyed(thisComponentHandle, true);
	}

	if(removeFromParent)
	{
		HComponent thisComponentHandle = B3DStaticGameObjectCast<Component>(mThisHandle);
		mParent->RemoveComponent(thisComponentHandle);
	}

	mParent = nullptr;
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
			{
				const SPtr<SceneInstance>& scene = SceneObject()->GetScene();
				scene->NotifyComponentActivated(thisComponentHandle, triggerEvents);
			}
		}
		else
		{
			SetGameObjectFlag(GameObjectTransientFlag::Disabled);

			if(triggerEvents)
			{
				const SPtr<SceneInstance>& scene = SceneObject()->GetScene();
				scene->NotifyComponentDeactivated(thisComponentHandle, triggerEvents);
			}
			
		}
	}
}

RTTIType* Component::GetRttiStatic()
{
	return ComponentRTTI::Instance();
}

RTTIType* Component::GetRtti() const
{
	return Component::GetRttiStatic();
}
