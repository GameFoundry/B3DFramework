//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsNullPhysicsPrerequisites.h"
#include "Physics/BsCharacterController.h"

namespace bs
{
	/** @addtogroup NullPhysics
	 *  @{
	 */

	/** Null implementation if a CharacterController. */
	class NullPhysicsCharacterController : public CharacterController
	{
	public:
		NullPhysicsCharacterController(const CHAR_CONTROLLER_DESC& desc);
		~NullPhysicsCharacterController() = default;

		CharacterCollisionFlags Move(const Vector3& displacement) override;
		Vector3 GetPosition() const override { return mDesc.Position; }
		void SetPosition(const Vector3& position) override { mDesc.Position = position; }
		Vector3 GetFootPosition() const override { return mDesc.Position; }
		void SetFootPosition(const Vector3& position) override { mDesc.Position = position; }
		float GetRadius() const override { return mDesc.Radius; }
		void SetRadius(float radius) override { mDesc.Radius = radius; }
		float GetHeight() const override { return mDesc.Height; }
		void SetHeight(float height) override { mDesc.Height = height; }
		Vector3 GetUp() const override { return mDesc.Up; }
		void SetUp(const Vector3& up) override { mDesc.Up = up; }
		CharacterClimbingMode GetClimbingMode() const override { return mDesc.ClimbingMode; }
		void SetClimbingMode(CharacterClimbingMode mode) override { mDesc.ClimbingMode = mode; }
		CharacterNonWalkableMode GetNonWalkableMode() const override { return mDesc.NonWalkableMode; }
		void SetNonWalkableMode(CharacterNonWalkableMode mode) override { mDesc.NonWalkableMode = mode; }
		float GetMinMoveDistance() const override { return mDesc.MinMoveDistance; }
		void SetMinMoveDistance(float value) override { mDesc.MinMoveDistance = value; }
		float GetContactOffset() const override { return mDesc.ContactOffset; }
		void SetContactOffset(float value) override { mDesc.ContactOffset = value; }
		float GetStepOffset() const override { return mDesc.StepOffset; }
		void SetStepOffset(float value) override { mDesc.StepOffset = value; }
		Radian GetSlopeLimit() const override { return mDesc.SlopeLimit; }
		void SetSlopeLimit(Radian value) override { mDesc.SlopeLimit = value; }

	private:
		CHAR_CONTROLLER_DESC mDesc;
	};

	/** @} */
} // namespace bs
