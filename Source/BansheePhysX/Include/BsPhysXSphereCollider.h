//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsPhysXPrerequisites.h"
#include "BsSphereCollider.h"
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
		PhysXSphereCollider(physx::PxPhysics* physx, const Vector3& position, const Quaternion& rotation, float radius);
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