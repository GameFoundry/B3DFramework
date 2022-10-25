//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsPhysXPrerequisites.h"
#include "Physics/BsCapsuleCollider.h"
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
		PhysXCapsuleCollider(physx::PxPhysics* physx, physx::PxScene* scene, const Vector3& position, const Quaternion& rotation, float radius, float halfHeight);
		~PhysXCapsuleCollider();

		/** @copydoc CapsuleCollider::setScale() */
		void SetScale(const Vector3& scale);

		/** @copydoc CapsuleCollider::setHalfHeight() */
		void SetHalfHeight(float halfHeight);

		/** @copydoc CapsuleCollider::getHalfHeight() */
		float GetHalfHeight() const;

		/** @copydoc CapsuleCollider::setRadius() */
		void SetRadius(float radius);

		/** @copydoc CapsuleCollider::getRadius() */
		float GetRadius() const;

	private:
		/** Returns the PhysX collider implementation common to all colliders. */
		FPhysXCollider* GetInternal() const;

		/** Applies the capsule geometry to the internal object based on set radius, height and scale. */
		void ApplyGeometry();

		float mRadius;
		float mHalfHeight;
	};

	/** @} */
} // namespace bs
