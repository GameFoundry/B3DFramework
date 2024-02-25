//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Scene/BsComponent.h"
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
	SO()->DestroyComponent(this, immediate);
}

void Component::DestroyInternal(GameObjectHandleBase& handle, bool immediate)
{
	const SPtr<GameObjectCollection>& ownerCollection = mOwnerCollection.lock();

	if(immediate)
	{
		if(B3D_ENSURE(ownerCollection != nullptr))
			ownerCollection->UnregisterObject(handle, SO()->IsInstantiated());
		else
			GameObjectManager::Instance().UnregisterObject(handle);
	}
	else
	{
		if(B3D_ENSURE(ownerCollection != nullptr))
			ownerCollection->QueueForDestroy(handle);
		else
			GameObjectManager::Instance().QueueForDestroy(handle);
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
