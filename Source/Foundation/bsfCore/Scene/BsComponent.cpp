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
	if(immediate)
	{
		DestroyImmediate();
		return;
	}

	HComponent thisComponentHandle = B3DStaticGameObjectCast<Component>(mThisHandle);
	mParent->NotifyWillDestroyComponent(thisComponentHandle);

	// Queue for destroy
	SetGameObjectFlag(GameObjectFlag::QueuedForDestroy);

	const SPtr<GameObjectCollection>& ownerCollection = mOwnerCollection.lock();
	if(ownerCollection != nullptr) // Allowed to be null during GameObjectCollection destructor call
		ownerCollection->QueueForDestroy(mThisHandle);
}

void Component::DestroyImmediate()
{
	const SPtr<GameObjectCollection>& ownerCollection = mOwnerCollection.lock();

	HComponent thisComponentHandle = B3DStaticGameObjectCast<Component>(mThisHandle);

	const bool isInitialized = HasGameObjectFlag(GameObjectFlag::Initialized);
	if(isInitialized)
		GetSceneManager().NotifyComponentDestroyedInternal(thisComponentHandle, true);

	// If queued for destroy, parent will have already been notified
	if(!HasGameObjectFlag(GameObjectFlag::QueuedForDestroy))
		mParent->NotifyWillDestroyComponent(thisComponentHandle);

	if(ownerCollection != nullptr) // Allowed to be null during GameObjectCollection destructor call
		ownerCollection->UnregisterObject(mThisHandle, isInitialized);

	GameObject::DestroyImmediate();
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
