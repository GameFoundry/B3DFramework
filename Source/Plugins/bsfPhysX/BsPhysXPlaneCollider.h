//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsPhysXPrerequisites.h"
#include "Physics/BsPlaneCollider.h"
#include "PxPhysics.h"

namespace bs
{
	/** @addtogroup PhysX
	 *  @{
	 */

	/** PhysX implementation of the PlaneCollider. */
	class PhysXPlaneCollider : public PlaneCollider
	{
	public:
		PhysXPlaneCollider(physx::PxPhysics* physx, physx::PxScene* scene, const Vector3& position, 
			const Quaternion& rotation);
		~PhysXPlaneCollider();

	private:
		/** Returns the PhysX collider implementation common to all colliders. */
		FPhysXCollider* getInternal() const;
	};

	/** @} */
}