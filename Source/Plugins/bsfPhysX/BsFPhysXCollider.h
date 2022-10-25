//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsPhysXPrerequisites.h"
#include "Physics/BsPhysicsCommon.h"
#include "Physics/BsFCollider.h"
#include "PxRigidStatic.h"

namespace bs
{
	/** @addtogroup PhysX
	 *  @{
	 */

	/** PhysX implementation of FCollider. */
	class FPhysXCollider : public FCollider
	{
	public:
		explicit FPhysXCollider(physx::PxScene* scene, physx::PxShape* shape);
		~FPhysXCollider();

		/** @copydoc FCollider::getPosition */
		Vector3 GetPosition() const override;

		/** @copydoc FCollider::getRotation */
		Quaternion GetRotation() const override;

		/** @copydoc FCollider::setTransform */
		void SetTransform(const Vector3& pos, const Quaternion& rotation) override;

		/** @copydoc FCollider::setIsTrigger */
		void SetIsTrigger(bool value) override;

		/** @copydoc FCollider::getIsTrigger */
		bool GetIsTrigger() const override;

		/** @copydoc FCollider::setIsStatic */
		void SetIsStatic(bool value) override;

		/** @copydoc FCollider::getIsStatic */
		bool GetIsStatic() const override;

		/** @copydoc FCollider::setContactOffset */
		void SetContactOffset(float value) override;

		/** @copydoc FCollider::getContactOffset */
		float GetContactOffset() const override;

		/** @copydoc FCollider::setRestOffset */
		void SetRestOffset(float value) override;

		/** @copydoc FCollider::getRestOffset */
		float GetRestOffset() const override;

		/** @copydoc FCollider::setMaterial */
		void SetMaterial(const HPhysicsMaterial& material) override;

		/** @copydoc FCollider::getLayer */
		u64 GetLayer() const override;

		/** @copydoc FCollider::setLayer */
		void SetLayer(u64 layer) override;

		/** @copydoc FCollider::getCollisionReportMode */
		CollisionReportMode GetCollisionReportMode() const override;

		/** @copydoc FCollider::setCollisionReportMode */
		void SetCollisionReportMode(CollisionReportMode mode) override;

		/** @copydoc FCollider::_setCCD */
		void SetCCDInternal(bool enabled) override;

		/** Gets the internal PhysX shape that represents the collider. */
		physx::PxShape* GetShapeInternal() const { return mShape; }

		/**
		 * Assigns a new shape the the collider. Old shape is released, and the new shape inherits any properties from the
		 * old shape, including parent, transform, flags and other.
		 */
		void SetShapeInternal(physx::PxShape* shape);

	protected:
		/** Updates shape filter data from stored values. */
		void UpdateFilter();

		physx::PxScene* mScene = nullptr;
		physx::PxShape* mShape = nullptr;
		physx::PxRigidStatic* mStaticBody = nullptr;
		bool mIsTrigger = false;
		bool mIsStatic = true;
		u64 mLayer = 1;
		bool mCCD = false;
		CollisionReportMode mCollisionReportMode = CollisionReportMode::None;
	};

	/** @} */
} // namespace bs
