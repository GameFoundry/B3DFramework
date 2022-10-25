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

		/** @copydoc CharacterController::move */
		CharacterCollisionFlags Move(const Vector3& displacement) override;

		/** @copydoc CharacterController::getPosition */
		Vector3 GetPosition() const override;

		/** @copydoc CharacterController::setPosition */
		void SetPosition(const Vector3& position) override;

		/** @copydoc CharacterController::getFootPosition */
		Vector3 GetFootPosition() const override;

		/** @copydoc CharacterController::setFootPosition */
		void SetFootPosition(const Vector3& position) override;

		/** @copydoc CharacterController::getRadius */
		float GetRadius() const override;

		/** @copydoc CharacterController::setRadius */
		void SetRadius(float radius) override;

		/** @copydoc CharacterController::getHeight */
		float GetHeight() const override;

		/** @copydoc CharacterController::setHeight */
		void SetHeight(float height) override;

		/** @copydoc CharacterController::getUp */
		Vector3 GetUp() const override;

		/** @copydoc CharacterController::setUp */
		void SetUp(const Vector3& up) override;

		/** @copydoc CharacterController::getClimbingMode */
		CharacterClimbingMode GetClimbingMode() const override;

		/** @copydoc CharacterController::setClimbingMode */
		void SetClimbingMode(CharacterClimbingMode mode) override;

		/** @copydoc CharacterController::getNonWalkableMode */
		CharacterNonWalkableMode GetNonWalkableMode() const override;

		/** @copydoc CharacterController::setNonWalkableMode */
		void SetNonWalkableMode(CharacterNonWalkableMode mode) override;

		/** @copydoc CharacterController::getMinMoveDistance */
		float GetMinMoveDistance() const override;

		/** @copydoc CharacterController::setMinMoveDistance */
		void SetMinMoveDistance(float value) override;

		/** @copydoc CharacterController::getContactOffset */
		float GetContactOffset() const override;

		/** @copydoc CharacterController::setContactOffset */
		void SetContactOffset(float value) override;

		/** @copydoc CharacterController::getStepOffset */
		float GetStepOffset() const override;

		/** @copydoc CharacterController::setStepOffset */
		void SetStepOffset(float value) override;

		/** @copydoc CharacterController::getSlopeLimit */
		Radian GetSlopeLimit() const override;

		/** @copydoc CharacterController::setSlopeLimit */
		void SetSlopeLimit(Radian value) override;

	private:
		/** @copydoc physx::PxUserControllerHitReport::onShapeHit */
		void onShapeHit(const physx::PxControllerShapeHit& hit) override;

		/** @copydoc physx::PxUserControllerHitReport::onControllerHit */
		void onControllerHit(const physx::PxControllersHit& hit) override;

		/** @copydoc physx::PxUserControllerHitReport::onObstacleHit */
		void onObstacleHit(const physx::PxControllerObstacleHit& hit) override{ /* Do nothing */ };

		/** @copydoc physx::PxQueryFilterCallback::preFilter */
		physx::PxQueryHitType::Enum preFilter(const physx::PxFilterData& filterData, const physx::PxShape* shape, const physx::PxRigidActor* actor, physx::PxHitFlags& queryFlags) override;

		/** @copydoc physx::PxQueryFilterCallback::postFilter */
		physx::PxQueryHitType::Enum postFilter(const physx::PxFilterData& filterData, const physx::PxQueryHit& hit) override;

		/** @copydoc physx::PxControllerFilterCallback::filter */
		bool filter(const physx::PxController& a, const physx::PxController& b) override;

		physx::PxCapsuleController* mController = nullptr;
		float mMinMoveDistance = 0.0f;
		float mLastMoveCall = 0.0f;
	};

	/** @} */
} // namespace bs
