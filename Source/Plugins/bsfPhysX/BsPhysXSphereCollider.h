//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsPhysXPrerequisites.h"
#include "Physics/BsSphereCollider.h"
#include "PxPhysics.h"

namespace bs
{
	/** @addtogroup PhysX
	 *  @{
	 */

	/** PhysX implementation of a SphereCollider. */
	class PhysXSphereCollider : public SphereCollider
	{
	public:
		PhysXSphereCollider(physx::PxPhysics* physx, physx::PxScene* scene, const Vector3& position, 
			const Quaternion& rotation, float radius);
		~PhysXSphereCollider();

		/** @copydoc SphereCollider::setScale */
		void setScale(const Vector3& scale) override;

		/** @copydoc SphereCollider::setRadius */
		void setRadius(float radius) override;

		/** @copydoc SphereCollider::getRadius */
		float getRadius() const override;

	private:
		/** Returns the PhysX collider implementation common to all colliders. */
		FPhysXCollider* getInternal() const;

		/** Applies the sphere geometry to the internal object based on set radius and scale. */
		void applyGeometry();

		float mRadius;
	};

	/** @} */
}