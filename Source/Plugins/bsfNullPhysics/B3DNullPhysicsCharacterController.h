//************************************ B3D Framework - Copyright 2026 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DNullPhysicsPrerequisites.h"
#include "Components/B3DCharacterController.h"

namespace b3d
{
	/** @addtogroup NullPhysics
	 *  @{
	 */

	/** Null implementation of ICharacterControllerImplementation. */
	class NullPhysicsCharacterController : public ICharacterControllerImplementation
	{
	public:
		NullPhysicsCharacterController(CharacterController& owner, const CharacterControllerCreateInformation& createInformation);
		~NullPhysicsCharacterController() override = default;

		CharacterCollisionFlags Move(const Vector3& displacement) override { return CharacterCollisionFlags(); }
		Vector3 GetPosition() const override { return mPosition; }
		void SetPosition(const Vector3& position) override { mPosition = position; }
		Vector3 GetFootPosition() const override { return mPosition; }
		void SetFootPosition(const Vector3& position) override { mPosition = position; }
		float GetRadius() const override { return mRadius; }
		void SetRadius(float radius) override { mRadius = radius; }
		float GetHeight() const override { return mHeight; }
		void SetHeight(float height) override { mHeight = height; }
		Vector3 GetUp() const override { return mUp; }
		void SetUp(const Vector3& up) override { mUp = up; }
		CharacterClimbingMode GetClimbingMode() const override { return mClimbingMode; }
		void SetClimbingMode(CharacterClimbingMode mode) override { mClimbingMode = mode; }
		CharacterNonWalkableMode GetNonWalkableMode() const override { return mNonWalkableMode; }
		void SetNonWalkableMode(CharacterNonWalkableMode mode) override { mNonWalkableMode = mode; }
		float GetMinMoveDistance() const override { return mMinMoveDistance; }
		void SetMinMoveDistance(float value) override { mMinMoveDistance = value; }
		float GetContactOffset() const override { return mContactOffset; }
		void SetContactOffset(float value) override { mContactOffset = value; }
		float GetStepOffset() const override { return mStepOffset; }
		void SetStepOffset(float value) override { mStepOffset = value; }
		Radian GetSlopeLimit() const override { return mSlopeLimit; }
		void SetSlopeLimit(Radian value) override { mSlopeLimit = value; }

	private:
		Vector3 mPosition = Vector3::kZero;
		float mRadius = 0.0f;
		float mHeight = 0.0f;
		Vector3 mUp = Vector3::kUnitY;
		CharacterClimbingMode mClimbingMode = CharacterClimbingMode::Normal;
		CharacterNonWalkableMode mNonWalkableMode = CharacterNonWalkableMode::Prevent;
		float mMinMoveDistance = 0.0f;
		float mContactOffset = 0.0f;
		float mStepOffset = 0.0f;
		Radian mSlopeLimit = Radian(0.0f);
	};

	/** @} */
} // namespace b3d
