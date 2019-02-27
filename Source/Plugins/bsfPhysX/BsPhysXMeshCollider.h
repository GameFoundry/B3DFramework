//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsPhysXPrerequisites.h"
#include "Physics/BsMeshCollider.h"
#include "PxPhysics.h"

namespace bs
{
	/** @addtogroup PhysX
	 *  @{
	 */

	/** PhysX implementation of a MeshCollider. */
	class PhysXMeshCollider : public MeshCollider
	{
	public:
		PhysXMeshCollider(physx::PxPhysics* physx, physx::PxScene* scene, const Vector3& position, 
			const Quaternion& rotation);
		~PhysXMeshCollider();

		/** @copydoc MeshCollider::setScale */
		void setScale(const Vector3& scale) override;

	private:
		/** Returns the PhysX collider implementation common to all colliders. */
		FPhysXCollider* getInternal() const;

		/** @copydoc MeshCollider::onMeshChanged */
		void onMeshChanged() override;

		/** Applies mesh geometry using the set mesh and scale. */
		void applyGeometry();

		/** Sets new geometry to the underlying shape. Rebuilds the shape if necessary. */
		void setGeometry(const physx::PxGeometry& geometry);
	};

	/** @} */
}