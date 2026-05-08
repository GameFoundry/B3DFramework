//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DPhysXPrerequisites.h"
#include "Physics/B3DPhysicsCommon.h"
#include "Components/B3DCollider.h"
#include "PxRigidStatic.h"

namespace b3d
{
	class PhysXScene;
	/** @addtogroup PhysX
	 *  @{
	 */

	/** PhysX implementation of Collider. */
	class PhysXCollider : public IColliderImplementation
	{
	public:
		PhysXCollider();
		~PhysXCollider() override;

		void AddToScene(PhysicsScene& scene) override;
		void RemoveFromScene() override;

		void AttachShape(const TShared<ColliderShape>& shape) override;
		void DetachShape(const TShared<ColliderShape>& shape) override;
		void SetTransform(const Vector3& position, const Quaternion& rotation) override;

		/**
		 * Returns the underlying PhysX object that represents a static collider. Only available of the collider is not
		 * attached to a rigidbody (in which case its managed by rigidbody's dynamic actor).
		 */
		physx::PxRigidStatic* GetPxRigidStatic() const { return mPxRigidStatic; }
	protected:
		physx::PxScene* mPxScene = nullptr;
		physx::PxRigidStatic* mPxRigidStatic = nullptr;
	};

	/** @} */
} // namespace b3d
