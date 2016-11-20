//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsSphereCollider.h"
#include "BsPhysics.h"

namespace bs
{
	SphereCollider::SphereCollider()
	{ }

	SPtr<SphereCollider> SphereCollider::create(float radius, const Vector3& position, const Quaternion& rotation)
	{
		return Physics::instance().createSphereCollider(radius, position, rotation);
	}
}