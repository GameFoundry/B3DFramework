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

		/** @copydoc CharacterController::move */
		CharacterCollisionFlags Move(const Vector3& displacement) override;

		/** @copydoc CharacterController::getPosition */
		Vector3 GetPosition() const override { return mDesc.Position; }

		/** @copydoc CharacterController::setPosition */
		void SetPosition(const Vector3& position) override { mDesc.Position = position; }

		/** @copydoc CharacterController::getFootPosition */
		Vector3 GetFootPosition() const override { return mDesc.Position; }

		/** @copydoc CharacterController::setFootPosition */
		void SetFootPosition(const Vector3& position) override { mDesc.Position = position; }

		/** @copydoc CharacterController::getRadius */
		float GetRadius() const override { return mDesc.Radius; }

		/** @copydoc CharacterController::setRadius */
		void SetRadius(float radius) override { mDesc.Radius = radius; }

		/** @copydoc CharacterController::getHeight */
		float GetHeight() const override { return mDesc.Height; }

		/** @copydoc CharacterController::setHeight */
		void SetHeight(float height) override { mDesc.Height = height; }

		/** @copydoc CharacterController::getUp */
		Vector3 GetUp() const override { return mDesc.Up; }

		/** @copydoc CharacterController::setUp */
		void SetUp(const Vector3& up) override { mDesc.Up = up; }

		/** @copydoc CharacterController::getClimbingMode */
		CharacterClimbingMode GetClimbingMode() const override { return mDesc.ClimbingMode; }

		/** @copydoc CharacterController::setClimbingMode */
		void SetClimbingMode(CharacterClimbingMode mode) override { mDesc.ClimbingMode = mode; }

		/** @copydoc CharacterController::getNonWalkableMode */
		CharacterNonWalkableMode GetNonWalkableMode() const override { return mDesc.NonWalkableMode; }

		/** @copydoc CharacterController::setNonWalkableMode */
		void SetNonWalkableMode(CharacterNonWalkableMode mode) override { mDesc.NonWalkableMode = mode; }

		/** @copydoc CharacterController::getMinMoveDistance */
		float GetMinMoveDistance() const override { return mDesc.MinMoveDistance; }

		/** @copydoc CharacterController::setMinMoveDistance */
		void SetMinMoveDistance(float value) override { mDesc.MinMoveDistance = value; }

		/** @copydoc CharacterController::getContactOffset */
		float GetContactOffset() const override { return mDesc.ContactOffset; }

		/** @copydoc CharacterController::setContactOffset */
		void SetContactOffset(float value) override { mDesc.ContactOffset = value; }

		/** @copydoc CharacterController::getStepOffset */
		float GetStepOffset() const override { return mDesc.StepOffset; }

		/** @copydoc CharacterController::setStepOffset */
		void SetStepOffset(float value) override { mDesc.StepOffset = value; }

		/** @copydoc CharacterController::getSlopeLimit */
		Radian GetSlopeLimit() const override { return mDesc.SlopeLimit; }

		/** @copydoc CharacterController::setSlopeLimit */
		void SetSlopeLimit(Radian value) override { mDesc.SlopeLimit = value; }

	private:
		CHAR_CONTROLLER_DESC mDesc;
	};

	/** @} */
} // namespace bs
