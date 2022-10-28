//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsPhysXPrerequisites.h"
#include "Physics/BsCharacterController.h"
#include "PxPhysics.h"
#include "characterkinematic/PxCapsuleController.h"

namespace bs
{
	/** @addtogroup PhysX
	 *  @{
	 */

	/** PhysX specific implementation if a CharacterController. */
	class PhysXCharacterController : public CharacterController, physx::PxUserControllerHitReport, physx::PxQueryFilterCallback, physx::PxControllerFilterCallback
	{
	public:
		PhysXCharacterController(physx::PxControllerManager* manager, const CHAR_CONTROLLER_DESC& desc);
		~PhysXCharacterController();

		CharacterCollisionFlags Move(const Vector3& displacement) override;
		Vector3 GetPosition() const override;
		void SetPosition(const Vector3& position) override;
		Vector3 GetFootPosition() const override;
		void SetFootPosition(const Vector3& position) override;
		float GetRadius() const override;
		void SetRadius(float radius) override;
		float GetHeight() const override;
		void SetHeight(float height) override;
		Vector3 GetUp() const override;
		void SetUp(const Vector3& up) override;
		CharacterClimbingMode GetClimbingMode() const override;
		void SetClimbingMode(CharacterClimbingMode mode) override;
		CharacterNonWalkableMode GetNonWalkableMode() const override;
		void SetNonWalkableMode(CharacterNonWalkableMode mode) override;
		float GetMinMoveDistance() const override;
		void SetMinMoveDistance(float value) override;
		float GetContactOffset() const override;
		void SetContactOffset(float value) override;
		float GetStepOffset() const override;
		void SetStepOffset(float value) override;
		Radian GetSlopeLimit() const override;
		void SetSlopeLimit(Radian value) override;

	private:
		void onShapeHit(const physx::PxControllerShapeHit& hit) override;
		void onControllerHit(const physx::PxControllersHit& hit) override;
		void onObstacleHit(const physx::PxControllerObstacleHit& hit) override { /* Do nothing */ }

		physx::PxQueryHitType::Enum preFilter(const physx::PxFilterData& filterData, const physx::PxShape* shape, const physx::PxRigidActor* actor, physx::PxHitFlags& queryFlags) override;
		physx::PxQueryHitType::Enum postFilter(const physx::PxFilterData& filterData, const physx::PxQueryHit& hit) override;
		bool filter(const physx::PxController& a, const physx::PxController& b) override;

		physx::PxCapsuleController* mController = nullptr;
		float mMinMoveDistance = 0.0f;
		float mLastMoveCall = 0.0f;
	};

	/** @} */
} // namespace bs
