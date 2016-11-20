//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsPlaneCollider.h"
#include "BsPhysics.h"

namespace bs
{
	PlaneCollider::PlaneCollider()
	{ }

	SPtr<PlaneCollider> PlaneCollider::create(const Vector3& position, const Quaternion& rotation)
	{
		return Physics::instance().createPlaneCollider(position, rotation);
	}
}