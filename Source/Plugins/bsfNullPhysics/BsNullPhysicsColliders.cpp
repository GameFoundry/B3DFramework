//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsNullPhysicsColliders.h"
#include "BsNullPhysicsRigidbody.h"

namespace bs
{
	FNullPhysicsCollider::FNullPhysicsCollider(const Vector3& position, const Quaternion& rotation)
		: mPosition(position), mRotation(rotation)
	{}

	void FNullPhysicsCollider::SetTransform(const Vector3& pos, const Quaternion& rotation)
	{
		mPosition = pos;
		mRotation = rotation;
	}

	NullPhysicsBoxCollider::NullPhysicsBoxCollider(const Vector3& position, const Quaternion& rotation, const Vector3& extents)
		: mExtents(extents)
	{
		mInternal = bs_new<FNullPhysicsCollider>(position, rotation);
	}

	NullPhysicsBoxCollider::~NullPhysicsBoxCollider()
	{
		bs_delete(mInternal);
	}

	NullPhysicsCapsuleCollider::NullPhysicsCapsuleCollider(const Vector3& position, const Quaternion& rotation, float radius, float halfHeight)
		: mRadius(radius), mHalfHeight(halfHeight)
	{
		mInternal = bs_new<FNullPhysicsCollider>(position, rotation);
	}

	NullPhysicsCapsuleCollider::~NullPhysicsCapsuleCollider()
	{
		bs_delete(mInternal);
	}

	NullPhysicsMeshCollider::NullPhysicsMeshCollider(const Vector3& position, const Quaternion& rotation)
	{
		mInternal = bs_new<FNullPhysicsCollider>(position, rotation);
	}

	NullPhysicsMeshCollider::~NullPhysicsMeshCollider()
	{
		bs_delete(mInternal);
	}

	NullPhysicsPlaneCollider::NullPhysicsPlaneCollider(const Vector3& position, const Quaternion& rotation)
	{
		mInternal = bs_new<FNullPhysicsCollider>(position, rotation);
	}

	NullPhysicsPlaneCollider::~NullPhysicsPlaneCollider()
	{
		bs_delete(mInternal);
	}

	NullPhysicsSphereCollider::NullPhysicsSphereCollider(const Vector3& position, const Quaternion& rotation, float radius)
		: mRadius(radius)
	{
		mInternal = bs_new<FNullPhysicsCollider>(position, rotation);
	}

	NullPhysicsSphereCollider::~NullPhysicsSphereCollider()
	{
		bs_delete(mInternal);
	}
} // namespace bs
