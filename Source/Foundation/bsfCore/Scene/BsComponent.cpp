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
	if(HasGameObjectFlag(GameObjectFlag::QueuedForDestroy))
		return;

	if(!B3D_ENSURE(!HasGameObjectFlag(GameObjectFlag::Destroyed)))
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
	if(!B3D_ENSURE(!HasGameObjectFlag(GameObjectFlag::Destroyed)))
		return;

	if(!HasGameObjectFlag(GameObjectFlag::QueuedForDestroy))
	{
		if(HasGameObjectFlag(GameObjectFlag::Initialized))
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
	if(HasGameObjectFlag(GameObjectFlag::QueuedForDestroy))
		return;

	if(!B3D_ENSURE(!HasGameObjectFlag(GameObjectFlag::Destroyed)))
		return;

	if(HasGameObjectFlag(GameObjectFlag::Initialized))
	{
		HComponent thisComponentHandle = B3DStaticGameObjectCast<Component>(mThisHandle);
		GetSceneManager().NotifyComponentDestroyedInternal(thisComponentHandle, true);
	}

	GameObject::QueueForDestroy();
}

void Component::Initialize()
{
	SetGameObjectFlag(GameObjectFlag::Initialized);
}

RTTITypeBase* Component::GetRttiStatic()
{
	return ComponentRTTI::Instance();
}

RTTITypeBase* Component::GetRtti() const
{
	return Component::GetRttiStatic();
}
