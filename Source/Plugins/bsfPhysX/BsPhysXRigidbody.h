//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsPhysXPrerequisites.h"
#include "Physics/BsRigidbody.h"
#include "Math/BsVector3.h"
#include "Math/BsQuaternion.h"
#include "PxPhysics.h"

namespace bs
{
	/** @addtogroup PhysX
	 *  @{
	 */

	/** PhysX implementation of a Rigidbody. */
	class PhysXRigidbody : public Rigidbody
	{
	public:
		PhysXRigidbody(physx::PxPhysics* physx, physx::PxScene* scene, const HSceneObject& linkedSO);
		~PhysXRigidbody();

		void Move(const Vector3& position) override;
		void Rotate(const Quaternion& rotation) override;
		Vector3 GetPosition() const override;
		Quaternion GetRotation() const override;
		void SetTransform(const Vector3& pos, const Quaternion& rot) override;
		void SetMass(float mass) override;
		float GetMass() const override;
		void SetIsKinematic(bool kinematic) override;
		bool GetIsKinematic() const override;
		bool IsSleeping() const override;
		void Sleep() override;
		void WakeUp() override;
		void SetSleepThreshold(float threshold) override;
		float GetSleepThreshold() const override;
		void SetUseGravity(bool gravity) override;
		bool GetUseGravity() const override;
		void SetVelocity(const Vector3& velocity) override;
		Vector3 GetVelocity() const override;
		void SetAngularVelocity(const Vector3& velocity) override;
		Vector3 GetAngularVelocity() const override;
		void SetDrag(float drag) override;
		float GetDrag() const override;
		void SetAngularDrag(float drag) override;
		float GetAngularDrag() const override;
		void SetInertiaTensor(const Vector3& tensor) override;
		Vector3 GetInertiaTensor() const override;
		void SetMaxAngularVelocity(float maxVelocity) override;
		float GetMaxAngularVelocity() const override;
		void SetCenterOfMass(const Vector3& position, const Quaternion& rotation) override;
		Vector3 GetCenterOfMassPosition() const override;
		Quaternion GetCenterOfMassRotation() const override;
		void SetPositionSolverCount(u32 count) override;
		u32 GetPositionSolverCount() const override;
		void SetVelocitySolverCount(u32 count) override;
		u32 GetVelocitySolverCount() const override;
		void SetFlags(RigidbodyFlag flags) override;
		void AddForce(const Vector3& force, ForceMode mode = ForceMode::Force) override;
		void AddTorque(const Vector3& torque, ForceMode mode = ForceMode::Force) override;
		void AddForceAtPoint(const Vector3& force, const Vector3& position, PointForceMode mode = PointForceMode::Force) override;
		Vector3 GetVelocityAtPoint(const Vector3& point) const override;
		void AddCollider(Collider* collider) override;
		void RemoveCollider(Collider* collider) override;
		void RemoveColliders() override;
		void UpdateMassDistribution() override;

		/** Returns the internal PhysX dynamic actor. */
		physx::PxRigidDynamic* GetInternalInternal() const { return mInternal; }

	private:
		physx::PxRigidDynamic* mInternal;
	};

	/** @} */
} // namespace bs
