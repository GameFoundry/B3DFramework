//************************************ B3D Framework - Copyright 2026 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DPhysXPrerequisites.h"
#include "Components/B3DCharacterController.h"
#include "PxPhysics.h"
#include "characterkinematic/PxCapsuleController.h"

namespace b3d
{
	/** @addtogroup PhysX
	 *  @{
	 */

	/** Converts from engine vector to PhysX extension vector. */
	inline physx::PxExtendedVec3 ToPxExtVector(const Vector3& input)
	{
		return physx::PxExtendedVec3(input.X, input.Y, input.Z);
	}

	/** Converts from PhysX extension vector to engine vector. */
	inline Vector3 FromPxExtVector(const physx::PxExtendedVec3& input)
	{
		return Vector3((float)input.x, (float)input.y, (float)input.z);
	}

	/** Converts an engine enum to a PhysX one. */
	inline physx::PxCapsuleClimbingMode::Enum ToPxEnum(CharacterClimbingMode value)
	{
		return value == CharacterClimbingMode::Normal
			? physx::PxCapsuleClimbingMode::eEASY
			: physx::PxCapsuleClimbingMode::eCONSTRAINED;
	}

	/** Converts a PhysX enum to an engine one. */
	inline CharacterClimbingMode FromPxEnum(physx::PxCapsuleClimbingMode::Enum value)
	{
		return value == physx::PxCapsuleClimbingMode::eEASY
			? CharacterClimbingMode::Normal
			: CharacterClimbingMode::Constrained;
	}

	/** Converts an engine enum to a PhysX one. */
	inline physx::PxControllerNonWalkableMode::Enum ToPxEnum(CharacterNonWalkableMode value)
	{
		return value == CharacterNonWalkableMode::Prevent
			? physx::PxControllerNonWalkableMode::ePREVENT_CLIMBING
			: physx::PxControllerNonWalkableMode::ePREVENT_CLIMBING_AND_FORCE_SLIDING;
	}

	/** Converts a PhysX enum to an engine one. */
	inline CharacterNonWalkableMode FromPxEnum(physx::PxControllerNonWalkableMode::Enum value)
	{
		return value == physx::PxControllerNonWalkableMode::ePREVENT_CLIMBING
			? CharacterNonWalkableMode::Prevent
			: CharacterNonWalkableMode::PreventAndSlide;
	}

	/** PhysX specific implementation if a CharacterController. */
	class PhysXCharacterController : public ICharacterControllerImplementation, physx::PxUserControllerHitReport, physx::PxQueryFilterCallback, physx::PxControllerFilterCallback
	{
	public:
		PhysXCharacterController(physx::PxControllerManager* manager, CharacterController& owner, const CharacterControllerCreateInformation& createInformation);
		~PhysXCharacterController() override;

		CharacterCollisionFlags Move(const Vector3& displacement) override;
		Vector3 GetPosition() const override { return FromPxExtVector(mController->getPosition()); } 
		void SetPosition(const Vector3& position) override { mController->setPosition(ToPxExtVector(position)); }
		Vector3 GetFootPosition() const override { return FromPxExtVector(mController->getFootPosition()); }
		void SetFootPosition(const Vector3& position) override { mController->setFootPosition(ToPxExtVector(position)); }
		float GetRadius() const override { return mController->getRadius(); }
		void SetRadius(float radius) override { mController->setRadius(radius); }
		float GetHeight() const override { return mController->getHeight(); }
		void SetHeight(float height) override { mController->setHeight(height); }
		Vector3 GetUp() const override { return FromPxVector(mController->getUpDirection()); }
		void SetUp(const Vector3& up) override { mController->setUpDirection(ToPxVector(up)); }
		CharacterClimbingMode GetClimbingMode() const override { return FromPxEnum(mController->getClimbingMode()); }
		void SetClimbingMode(CharacterClimbingMode mode) override { mController->setClimbingMode(ToPxEnum(mode)); }
		CharacterNonWalkableMode GetNonWalkableMode() const override { return FromPxEnum(mController->getNonWalkableMode()); }
		void SetNonWalkableMode(CharacterNonWalkableMode mode) override { mController->setNonWalkableMode(ToPxEnum(mode)); }
		float GetMinMoveDistance() const override { return mMinMoveDistance; }
		void SetMinMoveDistance(float value) override { mMinMoveDistance = value; }
		float GetContactOffset() const override { return mController->getContactOffset(); }
		void SetContactOffset(float value) override { mController->setContactOffset(value); }
		float GetStepOffset() const override { return mController->getStepOffset(); }
		void SetStepOffset(float value) override { mController->setStepOffset(value); }
		Radian GetSlopeLimit() const override { return Radian(mController->getSlopeLimit()); }
		void SetSlopeLimit(Radian value) override { mController->setSlopeLimit(value.GetValueInRadians()); }

	private:
		void onShapeHit(const physx::PxControllerShapeHit& hit) override;
		void onControllerHit(const physx::PxControllersHit& hit) override;
		void onObstacleHit(const physx::PxControllerObstacleHit& hit) override { /* Do nothing */ }

		physx::PxQueryHitType::Enum preFilter(const physx::PxFilterData& filterData, const physx::PxShape* shape, const physx::PxRigidActor* actor, physx::PxHitFlags& queryFlags) override;
		physx::PxQueryHitType::Enum postFilter(const physx::PxFilterData& filterData, const physx::PxQueryHit& hit) override;
		bool filter(const physx::PxController& a, const physx::PxController& b) override;

		CharacterController& mOwner;
		physx::PxCapsuleController* mController = nullptr;
		float mMinMoveDistance = 0.0f;
		float mLastMoveCall = 0.0f;
	};

	/** @} */
} // namespace b3d
