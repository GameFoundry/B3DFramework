//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsPhysXPrerequisites.h"
#include "BsCharacterController.h"
#include "PxPhysics.h"
#include "characterkinematic\PxCapsuleController.h"

namespace bs
{
	/** @addtogroup PhysX
	 *  @{
	 */

	/** PhysX specific implementation if a CharacterController. */
	class PhysXCharacterController : public CharacterController
								   , physx::PxUserControllerHitReport
		                           , physx::PxQueryFilterCallback
								   , physx::PxControllerFilterCallback
	{
	public:
		PhysXCharacterController(physx::PxControllerManager* manager, const CHAR_CONTROLLER_DESC& desc);
		~PhysXCharacterController();

		/** @copydoc CharacterController::move */
		CharacterCollisionFlags move(const Vector3& displacement) override;

		/** @copydoc CharacterController::getPosition */
		Vector3 getPosition() const override;

		/** @copydoc CharacterController::setPosition */
		void setPosition(const Vector3& position) override;

		/** @copydoc CharacterController::getFootPosition */
		Vector3 getFootPosition() const override;

		/** @copydoc CharacterController::setFootPosition */
		void setFootPosition(const Vector3& position) override;

		/** @copydoc CharacterController::getRadius */
		float getRadius() const override;

		/** @copydoc CharacterController::setRadius */
		void setRadius(float radius) override;

		/** @copydoc CharacterController::getHeight */
		float getHeight() const override;

		/** @copydoc CharacterController::setHeight */
		void setHeight(float height) override;

		/** @copydoc CharacterController::getUp */
		Vector3 getUp() const override;

		/** @copydoc CharacterController::setUp */
		void setUp(const Vector3& up) override;

		/** @copydoc CharacterController::getClimbingMode */
		CharacterClimbingMode getClimbingMode() const override;

		/** @copydoc CharacterController::setClimbingMode */
		void setClimbingMode(CharacterClimbingMode mode) override;

		/** @copydoc CharacterController::getNonWalkableMode */
		CharacterNonWalkableMode getNonWalkableMode() const override;

		/** @copydoc CharacterController::setNonWalkableMode */
		void setNonWalkableMode(CharacterNonWalkableMode mode) override;

		/** @copydoc CharacterController::getMinMoveDistance */
		float getMinMoveDistance() const override;

		/** @copydoc CharacterController::setMinMoveDistance */
		void setMinMoveDistance(float value) override;

		/** @copydoc CharacterController::getContactOffset */
		float getContactOffset() const override;

		/** @copydoc CharacterController::setContactOffset */
		void setContactOffset(float value) override;

		/** @copydoc CharacterController::getStepOffset */
		float getStepOffset() const override;

		/** @copydoc CharacterController::setStepOffset */
		void setStepOffset(float value) override;

		/** @copydoc CharacterController::getSlopeLimit */
		Radian getSlopeLimit() const override;

		/** @copydoc CharacterController::setSlopeLimit */
		void setSlopeLimit(Radian value) override;

	private:
		/** @copydoc physx::PxUserControllerHitReport::onShapeHit */
		void onShapeHit(const physx::PxControllerShapeHit& hit) override;

		/** @copydoc physx::PxUserControllerHitReport::onControllerHit */
		void onControllerHit(const physx::PxControllersHit& hit) override;

		/** @copydoc physx::PxUserControllerHitReport::onObstacleHit */
		void onObstacleHit(const physx::PxControllerObstacleHit& hit) override { /* Do nothing */ };

		/** @copydoc physx::PxQueryFilterCallback::preFilter */
		physx::PxQueryHitType::Enum preFilter(const physx::PxFilterData& filterData, const physx::PxShape* shape, 
			const physx::PxRigidActor* actor, physx::PxHitFlags& queryFlags) override;

		/** @copydoc physx::PxQueryFilterCallback::postFilter */
		physx::PxQueryHitType::Enum postFilter(const physx::PxFilterData& filterData, 
			const physx::PxQueryHit& hit) override;

		/** @copydoc physx::PxControllerFilterCallback::filter */
		bool filter(const physx::PxController& a, const physx::PxController& b) override;

		physx::PxCapsuleController* mController = nullptr;
		float mMinMoveDistance = 0.0f;
		float mLastMoveCall = 0.0f;
	};

	/** @} */
}