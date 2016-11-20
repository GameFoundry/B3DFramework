//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsCapsuleCollider.h"
#include "BsPhysics.h"

namespace bs
{
	CapsuleCollider::CapsuleCollider()
	{ }

	SPtr<CapsuleCollider> CapsuleCollider::create(float radius, float halfHeight, 
		const Vector3& position, const Quaternion& rotation)
	{
		return Physics::instance().createCapsuleCollider(radius, halfHeight, position, rotation);
	}
}