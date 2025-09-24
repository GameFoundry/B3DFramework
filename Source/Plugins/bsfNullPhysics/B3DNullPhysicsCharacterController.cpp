//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DNullPhysicsCharacterController.h"
#include "Utility/B3DTime.h"
#include "B3DNullPhysics.h"
#include "Components/B3DCCollider.h"

using namespace b3d;

NullPhysicsCharacterController::NullPhysicsCharacterController(const CHAR_CONTROLLER_DESC& desc)
	: CharacterController(desc), mDesc(desc)
{}

CharacterCollisionFlags NullPhysicsCharacterController::Move(const Vector3& displacement)
{
	mDesc.Position += displacement;

	return CharacterCollisionFlags();
}
