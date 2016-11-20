//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsPhysXPrerequisites.h"
#include "BsPhysicsCommon.h"
#include "BsFCollider.h"
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
		explicit FPhysXCollider(physx::PxShape* shape);
		~FPhysXCollider();

		/** @copydoc FCollider::getPosition */
		Vector3 getPosition() const override;

		/** @copydoc FCollider::getRotation */
		Quaternion getRotation() const override;

		/** @copydoc FCollider::setTransform */
		void setTransform(const Vector3& pos, const Quaternion& rotation) override;

		/** @copydoc FCollider::setIsTrigger */
		void setIsTrigger(bool value) override;

		/** @copydoc FCollider::getIsTrigger */
		bool getIsTrigger() const override;

		/** @copydoc FCollider::setIsStatic */
		void setIsStatic(bool value) override;

		/** @copydoc FCollider::getIsStatic */
		bool getIsStatic() const override;

		/** @copydoc FCollider::setContactOffset */
		void setContactOffset(float value) override;

		/** @copydoc FCollider::getContactOffset */
		float getContactOffset() const override;

		/** @copydoc FCollider::setRestOffset */
		void setRestOffset(float value) override;

		/** @copydoc FCollider::getRestOffset */
		float getRestOffset() const override;

		/** @copydoc FCollider::setMaterial */
		void setMaterial(const HPhysicsMaterial& material) override;

		/** @copydoc FCollider::getLayer */
		UINT64 getLayer() const override;

		/** @copydoc FCollider::setLayer */
		void setLayer(UINT64 layer) override;

		/** @copydoc FCollider::getCollisionReportMode */
		CollisionReportMode getCollisionReportMode() const override;

		/** @copydoc FCollider::setCollisionReportMode */
		void setCollisionReportMode(CollisionReportMode mode) override;

		/** @copydoc FCollider::_setCCD */
		void _setCCD(bool enabled) override;

		/** Gets the internal PhysX shape that represents the collider. */
		physx::PxShape* _getShape() const { return mShape; }

		/** 
		 * Assigns a new shape the the collider. Old shape is released, and the new shape inherits any properties from the
		 * old shape, including parent, transform, flags and other.
		 */
		void _setShape(physx::PxShape* shape);
	protected:
		/** Updates shape filter data from stored values. */
		void updateFilter();

		physx::PxShape* mShape = nullptr;
		physx::PxRigidStatic* mStaticBody = nullptr;
		bool mIsTrigger = false;
		bool mIsStatic = true;
		UINT64 mLayer = 1;
		bool mCCD = false;
		CollisionReportMode mCollisionReportMode = CollisionReportMode::None;
	};

	/** @} */
}