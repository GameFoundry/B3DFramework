//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsPhysXPrerequisites.h"
#include "BsCapsuleCollider.h"
#include "PxPhysics.h"

namespace bs
{
	/** @addtogroup PhysX
	 *  @{
	 */

	 /** PhysX implementation of a CapsuleCollider. */
	class PhysXCapsuleCollider : public CapsuleCollider
	{
	public:
		PhysXCapsuleCollider(physx::PxPhysics* physx, const Vector3& position, const Quaternion& rotation, 
			float radius, float halfHeight);
		~PhysXCapsuleCollider();

		/** @copydoc CapsuleCollider::setScale() */
		void setScale(const Vector3& scale) override;

		/** @copydoc CapsuleCollider::setHalfHeight() */
		void setHalfHeight(float halfHeight) override;

		/** @copydoc CapsuleCollider::getHalfHeight() */
		float getHalfHeight() const override;

		/** @copydoc CapsuleCollider::setRadius() */
		void setRadius(float radius) override;

		/** @copydoc CapsuleCollider::getRadius() */
		float getRadius() const override;

	private:
		/** Returns the PhysX collider implementation common to all colliders. */
		FPhysXCollider* getInternal() const;

		/** Applies the capsule geometry to the internal object based on set radius, height and scale. */
		void applyGeometry();

		float mRadius;
		float mHalfHeight;
	};

	/** @} */
}