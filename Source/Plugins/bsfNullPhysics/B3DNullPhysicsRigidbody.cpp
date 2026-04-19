//************************************ B3D Framework - Copyright 2026 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DNullPhysicsRigidbody.h"

using namespace b3d;

NullPhysicsRigidbody::NullPhysicsRigidbody(Rigidbody& owner)
{}

void NullPhysicsRigidbody::SetTransform(const Vector3& position, const Quaternion& rotation)
{
	mPosition = position;
	mRotation = rotation;
}

void NullPhysicsRigidbody::GetTransform(Vector3& outPosition, Quaternion& outRotation)
{
	outPosition = mPosition;
	outRotation = mRotation;
}

void NullPhysicsRigidbody::GetCenterOfMass(Vector3& outPosition, Quaternion& outRotation)
{
	outPosition = Vector3::kZero;
	outRotation = Quaternion::kIdentity;
}
