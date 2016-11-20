//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsPhysXPrerequisites.h"
#include "BsBoxCollider.h"
#include "PxPhysics.h"

namespace bs
{
	/** @addtogroup PhysX
	 *  @{
	 */

	/** PhysX implementation of a BoxCollider. */
	class PhysXBoxCollider : public BoxCollider
	{
	public:
		PhysXBoxCollider(physx::PxPhysics* physx, const Vector3& position, const Quaternion& rotation, 
			const Vector3& extents);
		~PhysXBoxCollider();

		/** @copydoc BoxCollider::setScale */
		void setScale(const Vector3& scale) override;

		/** @copydoc BoxCollider::setExtents */
		void setExtents(const Vector3& extents) override;

		/** @copydoc BoxCollider::getExtents */
		Vector3 getExtents() const override;

	private:
		/** Returns the PhysX collider implementation common to all colliders. */
		FPhysXCollider* getInternal() const;

		/** Applies the sphere geometry to the internal object based on set extents and scale. */
		void applyGeometry();

		Vector3 mExtents;
	};

	/** @} */
}