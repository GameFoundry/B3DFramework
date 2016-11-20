//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsPhysXPrerequisites.h"
#include "BsMeshCollider.h"
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
		PhysXMeshCollider(physx::PxPhysics* physx, const Vector3& position, const Quaternion& rotation);
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