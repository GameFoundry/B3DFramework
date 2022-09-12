//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Scene/BsComponent.h"
#include "Scene/BsSceneObject.h"
#include "Private/RTTI/BsComponentRTTI.h"

namespace bs
{
	Component::Component(HSceneObject parent)
		:mParent(std::move(parent))
	{
		setName("Component");
	}

	bool Component::TypeEquals(const Component& other)
	{
		return GetRTTI()->getRTTIId() == other.getRTTI()->getRTTIId();
	}

	bool Component::CalculateBounds(Bounds& bounds)
	{
		Vector3 position = SO()->getTransform().getPosition();

		bounds = Bounds(AABox(position, position), Sphere(position, 0.0f));
		return false;
	}

	void Component::Destroy(bool immediate)
	{
		SO()->destroyComponent(this, immediate);
	}

	void Component::DestroyInternal(GameObjectHandleBase& handle, bool immediate)
	{
		if (immediate)
			GameObjectManager::instance().unregisterObject(handle);
		else
			GameObjectManager::instance().queueForDestroy(handle);
	}

	RTTITypeBase* Component::getRTTIStatic()
	{
		return ComponentRTTI::Instance();
	}

	RTTITypeBase* Component::getRTTI() const
	{
		return Component::GetRTTIStatic();
	}
}
