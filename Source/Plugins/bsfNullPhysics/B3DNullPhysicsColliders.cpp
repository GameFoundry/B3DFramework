//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DNullPhysicsColliders.h"
#include "B3DNullPhysicsRigidbody.h"

using namespace b3d;

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
	mInternal = B3DNew<FNullPhysicsCollider>(position, rotation);
}

NullPhysicsBoxCollider::~NullPhysicsBoxCollider()
{
	B3DDelete(mInternal);
}

NullPhysicsCapsuleCollider::NullPhysicsCapsuleCollider(const Vector3& position, const Quaternion& rotation, float radius, float halfHeight)
	: mRadius(radius), mHalfHeight(halfHeight)
{
	mInternal = B3DNew<FNullPhysicsCollider>(position, rotation);
}

NullPhysicsCapsuleCollider::~NullPhysicsCapsuleCollider()
{
	B3DDelete(mInternal);
}

NullPhysicsMeshCollider::NullPhysicsMeshCollider(const Vector3& position, const Quaternion& rotation)
{
	mInternal = B3DNew<FNullPhysicsCollider>(position, rotation);
}

NullPhysicsMeshCollider::~NullPhysicsMeshCollider()
{
	B3DDelete(mInternal);
}

NullPhysicsPlaneCollider::NullPhysicsPlaneCollider(const Vector3& position, const Quaternion& rotation)
{
	mInternal = B3DNew<FNullPhysicsCollider>(position, rotation);
}

NullPhysicsPlaneCollider::~NullPhysicsPlaneCollider()
{
	B3DDelete(mInternal);
}

NullPhysicsSphereCollider::NullPhysicsSphereCollider(const Vector3& position, const Quaternion& rotation, float radius)
	: mRadius(radius)
{
	mInternal = B3DNew<FNullPhysicsCollider>(position, rotation);
}

NullPhysicsSphereCollider::~NullPhysicsSphereCollider()
{
	B3DDelete(mInternal);
}
