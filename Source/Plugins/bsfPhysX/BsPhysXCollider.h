//************************************ bs::framework - Copyright 2025 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsPhysXPrerequisites.h"
#include "Physics/BsPhysicsCommon.h"
#include "Physics/BsCollider.h"
#include "PxRigidStatic.h"

namespace bs
{
	class PhysXScene;
	/** @addtogroup PhysX
	 *  @{
	 */

	/** PhysX implementation of Collider. */
	class PhysXCollider : public Collider
	{
	public:
		PhysXCollider(PhysXScene& physicsScene, const Vector3& position = Vector3::kZero, const Quaternion& rotation = Quaternion::kIdentity, const Vector3& scale = Vector3::kOne);
		~PhysXCollider() override;

		void SetShapes(const TArrayView<SPtr<ColliderShape>>& shapes) override;
		void SetRigidbody(Rigidbody* rigidbody) override;
		void UpdateTransform() override;

		/**
		 * Returns the underlying PhysX object that represents a static collider. Only available of the collider is not
		 * attached to a rigidbody (in which case its managed by rigidbody's dynamic actor).
		 */
		physx::PxRigidStatic* GetPxRigidStatic() const { return mStaticBody; }
	protected:
		/** Creates a static body for the collider. This should be called for colliders not attached to rigidbodies. */
		void CreateStaticBody();

		/** Destroys a static body on the collider, if one exists. */
		void DestroyStaticBody();

		physx::PxRigidStatic* mStaticBody = nullptr;
	};

	/** @} */
} // namespace bs
