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
		PhysXSphereCollider(physx::PxPhysics* physx, physx::PxScene* scene, const Vector3& position, const Quaternion& rotation, float radius);
		~PhysXSphereCollider();

		void SetScale(const Vector3& scale) override;
		void SetRadius(float radius) override;
		float GetRadius() const override;

	private:
		/** Returns the PhysX collider implementation common to all colliders. */
		FPhysXCollider* GetInternal() const;

		/** Applies the sphere geometry to the internal object based on set radius and scale. */
		void ApplyGeometry();

		float mRadius;
	};

	/** @} */
} // namespace bs
