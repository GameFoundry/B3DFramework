//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DPhysXPrerequisites.h"
#include "Components/B3DRigidbody.h"
#include "Math/B3DVector3.h"
#include "Math/B3DQuaternion.h"
#include "PxPhysics.h"

namespace b3d
{
	/** @addtogroup PhysX
	 *  @{
	 */

	/** PhysX implementation of a Rigidbody. */
	class PhysXRigidbody : public IRigidbodyImplementation
	{
	public:
		PhysXRigidbody(Rigidbody& owner);
		~PhysXRigidbody() override;

		void Move(const Vector3& position) override;
		void Rotate(const Quaternion& rotation) override;
		void SetTransform(const Vector3& position, const Quaternion& rotation) override;
		void GetTransform(Vector3& outPosition, Quaternion& outRotation) override;
		void SetMass(float mass) override;
		void SetIsKinematic(bool kinematic) override;
		bool IsSleeping() const override;
		void Sleep() override;
		void WakeUp() override;
		void SetSleepThreshold(float threshold) override;
		void SetUseGravity(bool gravity) override;
		void SetVelocity(const Vector3& velocity) override;
		Vector3 GetVelocity() const override;
		void SetAngularVelocity(const Vector3& velocity) override;
		Vector3 GetAngularVelocity() const override;
		void SetDrag(float drag) override;
		void SetAngularDrag(float drag) override;
		void SetInertiaTensor(const Vector3& tensor) override;
		Vector3 GetInertiaTensor() const override;
		void SetMaxAngularVelocity(float maxVelocity) override;
		void SetCenterOfMass(const Vector3& position, const Quaternion& rotation) override;
		void GetCenterOfMass(Vector3& outPosition, Quaternion& outRotation) override;
		void SetSolverIterationCounts(u32 positionCount, u32 velocityCount) override;
		void SetFlags(RigidbodyFlags flags) override;
		void AddForce(const Vector3& force, ForceMode mode = ForceMode::Force) override;
		void AddTorque(const Vector3& torque, ForceMode mode = ForceMode::Force) override;
		void AddForceAtPoint(const Vector3& force, const Vector3& position, PointForceMode mode = PointForceMode::Force) override;
		Vector3 GetVelocityAtPoint(const Vector3& point) const override;
		void UpdateMassDistribution(bool autoMassEnabled) override;
		void AttachShape(const TShared<ColliderShape>& shape) override;
		void DetachShape(const TShared<ColliderShape>& shape) override;
		void AddToScene(PhysicsScene& scene) override;
		void RemoveFromScene() override;

		/** Returns the internal PhysX dynamic actor. */
		physx::PxRigidDynamic* GetPxRigidDynamic() const { return mPxRigidDynamic; }

	private:
		physx::PxScene* mPxScene = nullptr;
		physx::PxRigidDynamic* mPxRigidDynamic = nullptr;
	};

	/** @} */
} // namespace b3d
