//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsNullPhysicsCharacterController.h"
#include "Utility/BsTime.h"
#include "BsNullPhysics.h"
#include "Components/BsCCollider.h"

namespace bs
{
	NullPhysicsCharacterController::NullPhysicsCharacterController(const CHAR_CONTROLLER_DESC& desc)
		:CharacterController(desc), mDesc(desc)
	{ }

	CharacterCollisionFlags NullPhysicsCharacterController::Move(const Vector3& displacement)
	{
		mDesc.position += displacement;

		return CharacterCollisionFlags();
	}
}
