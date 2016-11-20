//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsBoxCollider.h"
#include "BsPhysics.h"

namespace bs
{
	BoxCollider::BoxCollider()
	{ }

	SPtr<BoxCollider> BoxCollider::create(const Vector3& extents,
		const Vector3& position, const Quaternion& rotation)
	{
		return Physics::instance().createBoxCollider(extents, position, rotation);
	}
}