//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsComponent.h"
#include "BsSceneObject.h"
#include "BsComponentRTTI.h"

namespace BansheeEngine
{
	Component::Component()
		:mNotifyFlags(TCF_None)
	{ }

	Component::Component(const HSceneObject& parent)
		:mNotifyFlags(TCF_None), mParent(parent)
	{
		setName("Component");
	}

	Component::~Component()
	{

	}

	bool Component::typeEquals(const Component& other)
	{
		return getRTTI()->getRTTIId() == other.getRTTI()->getRTTIId();
	}

	bool Component::calculateBounds(Bounds& bounds)
	{
		Vector3 position = SO()->getWorldPosition();

		bounds = Bounds(AABox(position, position), Sphere(position, 0.0f));
		return false;
	}

	void Component::destroy(bool immediate)
	{
		SO()->destroyComponent(this, immediate);
	}

	void Component::destroyInternal(GameObjectHandleBase& handle, bool immediate)
	{
		if (immediate)
			GameObjectManager::instance().unregisterObject(handle);
		else
			GameObjectManager::instance().queueForDestroy(handle);
	}

	RTTITypeBase* Component::getRTTIStatic()
	{
		return ComponentRTTI::instance();
	}

	RTTITypeBase* Component::getRTTI() const
	{
		return Component::getRTTIStatic();
	}
}