//************************************ B3D Framework - Copyright 2026 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DPhysXPrerequisites.h"
#include "Physics/B3DPhysicsCommon.h"
#include "Physics/B3DColliderShape.h"
#include "PxRigidStatic.h"

namespace b3d
{
	/** @addtogroup PhysX
	 *  @{
	 */

	/** PhysX implementation of ColliderShape. */
	class PhysXColliderShape : public ColliderShape
	{
		using Super = ColliderShape;
	public:
		~PhysXColliderShape() override;

		ColliderShapeType GetType() const override { return mShapeType; }

		void SetPosition(const Vector3& position) override;
		void SetRotation(const Quaternion& rotation) override;
		void SetScale(const Vector3& scale) override;
		void SetContactOffset(float value) override;
		void SetRestOffset(float value) override;
		void SetMaterial(const HPhysicsMaterial& material) override;
		void SetIsTrigger(bool value) override;
		void SetLayer(u64 layer) override;
		void SetCollisionReportMode(CollisionReportMode mode) override;
		void SetContinuousCollisionDetection(bool value) override;

		void SetShape(const PlaneColliderShapeInformation& information) override;
		void SetShape(const BoxColliderShapeInformation& information) override;
		void SetShape(const SphereColliderShapeInformation& information) override;
		void SetShape(const CapsuleColliderShapeInformation& information) override;
		void SetShape(const MeshColliderShapeInformation& information) override;

		/** Returns the underlying PhysX shape object. */
		physx::PxShape* GetPxShape() const { return mShape; }

	protected:
		/**
		 * Changes the underlying shape geometry. A new shape will be created if it doesn't already exist. If shape exists
		 * but geometry type doesn't match, the shape will be re-created.
		 */
		void SetGeometry(const physx::PxGeometry& geometry);

		/** Sets shape filter data from stored values. */
		void SetFilter();

		/** Recreates the underlying shape using the currently set properties. */
		void RecreateShape();

		/** Destroys the currently assigned shape, if any. */
		void DestroyShape();

		void UpdateTransform() override;

		physx::PxShape* mShape = nullptr;
		ColliderShapeType mShapeType = ColliderShapeType::Plane;
		Vector3 mWorldSpaceScale = Vector3::kOne;
	};

	/** @} */
} // namespace b3d
