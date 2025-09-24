//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DNullPhysicsRigidbody.h"
#include "Physics/B3DCollider.h"
#include "B3DNullPhysicsColliders.h"
#include "Scene/B3DSceneObject.h"
#include "Physics/B3DPhysics.h"
#include "B3DNullPhysics.h"

using namespace b3d;

NullPhysicsRigidbody::NullPhysicsRigidbody(const HSceneObject& linkedSO)
	: Rigidbody(linkedSO)
{
	mPosition = linkedSO->GetTransform().GetPosition();
	mRotation = linkedSO->GetTransform().GetRotation();
}

void NullPhysicsRigidbody::Move(const Vector3& position)
{
	SetTransform(position, GetRotation());
}

void NullPhysicsRigidbody::Rotate(const Quaternion& rotation)
{
	SetTransform(GetPosition(), rotation);
}

Vector3 NullPhysicsRigidbody::GetPosition() const
{
	return mPosition;
}

Quaternion NullPhysicsRigidbody::GetRotation() const
{
	return mRotation;
}

void NullPhysicsRigidbody::SetTransform(const Vector3& pos, const Quaternion& rot)
{
	mPosition = pos;
	mRotation = rot;
}

void NullPhysicsRigidbody::SetCenterOfMass(const Vector3& position, const Quaternion& rotation)
{
	mCenterOfMassPosition = position;
	mCenterOfMassRotation = rotation;
}
