//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsPhysXPrerequisites.h"
#include "Physics/BsPhysicsCommon.h"
#include "Physics/BsFCollider.h"
#include "PxRigidStatic.h"

namespace bs
{
	/** @addtogroup PhysX
	 *  @{
	 */

	/** PhysX implementation of FCollider. */
	class FPhysXCollider : public FCollider
	{
	public:
		explicit FPhysXCollider(physx::PxScene* scene, physx::PxShape* shape);
		~FPhysXCollider();

		Vector3 GetPosition() const override;
		Quaternion GetRotation() const override;
		void SetTransform(const Vector3& pos, const Quaternion& rotation) override;
		void SetIsTrigger(bool value) override;
		bool GetIsTrigger() const override;
		void SetIsStatic(bool value) override;
		bool GetIsStatic() const override;
		void SetContactOffset(float value) override;
		float GetContactOffset() const override;
		void SetRestOffset(float value) override;
		float GetRestOffset() const override;
		void SetMaterial(const HPhysicsMaterial& material) override;
		u64 GetLayer() const override;
		void SetLayer(u64 layer) override;
		CollisionReportMode GetCollisionReportMode() const override;
		void SetCollisionReportMode(CollisionReportMode mode) override;
		void SetCCDInternal(bool enabled) override;

		/** Gets the internal PhysX shape that represents the collider. */
		physx::PxShape* GetShapeInternal() const { return mShape; }

		/**
		 * Assigns a new shape the the collider. Old shape is released, and the new shape inherits any properties from the
		 * old shape, including parent, transform, flags and other.
		 */
		void SetShapeInternal(physx::PxShape* shape);

	protected:
		/** Updates shape filter data from stored values. */
		void UpdateFilter();

		physx::PxScene* mScene = nullptr;
		physx::PxShape* mShape = nullptr;
		physx::PxRigidStatic* mStaticBody = nullptr;
		bool mIsTrigger = false;
		bool mIsStatic = true;
		u64 mLayer = 1;
		bool mCCD = false;
		CollisionReportMode mCollisionReportMode = CollisionReportMode::None;
	};

	/** @} */
} // namespace bs
