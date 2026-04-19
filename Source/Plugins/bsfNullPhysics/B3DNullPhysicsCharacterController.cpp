//************************************ B3D Framework - Copyright 2026 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DNullPhysicsCharacterController.h"

using namespace b3d;

NullPhysicsCharacterController::NullPhysicsCharacterController(CharacterController& owner, const CharacterControllerCreateInformation& createInformation)
	: mPosition(createInformation.Position)
	, mRadius(createInformation.Radius)
	, mHeight(createInformation.Height)
	, mUp(createInformation.Up)
	, mClimbingMode(createInformation.ClimbingMode)
	, mNonWalkableMode(createInformation.NonWalkableMode)
	, mMinMoveDistance(createInformation.MinMoveDistance)
	, mContactOffset(createInformation.ContactOffset)
	, mStepOffset(createInformation.StepOffset)
	, mSlopeLimit(createInformation.SlopeLimit)
{}
