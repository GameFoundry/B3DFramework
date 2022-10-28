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
		~PhysXCapsuleCollider() override;

		void SetScale(const Vector3& scale) override;
		void SetHalfHeight(float halfHeight) override;
		float GetHalfHeight() const override;
		void SetRadius(float radius) override;
		float GetRadius() const override;

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
