//************************************ B3D Framework - Copyright 2026 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DNullPhysicsPrerequisites.h"
#include "Components/B3DRigidbody.h"
#include "Math/B3DVector3.h"
#include "Math/B3DQuaternion.h"

namespace b3d
{
	/** @addtogroup NullPhysics
	 *  @{
	 */

	/** Null implementation of IRigidbodyImplementation. */
	class NullPhysicsRigidbody : public IRigidbodyImplementation
	{
	public:
		NullPhysicsRigidbody(Rigidbody& owner);
		~NullPhysicsRigidbody() override = default;

		void Move(const Vector3& position) override { mPosition = position; }
		void Rotate(const Quaternion& rotation) override { mRotation = rotation; }
		void SetMass(float mass) override {}
		void SetIsKinematic(bool kinematic) override {}
		bool IsSleeping() const override { return false; }
		void Sleep() override {}
		void WakeUp() override {}
		void SetSleepThreshold(float threshold) override {}
		void SetUseGravity(bool gravity) override {}
		void SetVelocity(const Vector3& velocity) override {}
		Vector3 GetVelocity() const override { return Vector3::kZero; }
		void SetAngularVelocity(const Vector3& velocity) override {}
		Vector3 GetAngularVelocity() const override { return Vector3::kZero; }
		void SetDrag(float drag) override {}
		void SetAngularDrag(float drag) override {}
		void SetInertiaTensor(const Vector3& tensor) override {}
		Vector3 GetInertiaTensor() const override { return Vector3::kZero; }
		void SetMaxAngularVelocity(float velocity) override {}
		void SetFlags(RigidbodyFlags flags) override {}
		void AddForce(const Vector3& force, ForceMode mode) override {}
		void AddTorque(const Vector3& torque, ForceMode mode) override {}
		void AddForceAtPoint(const Vector3& force, const Vector3& position, PointForceMode mode) override {}
		Vector3 GetVelocityAtPoint(const Vector3& point) const override { return Vector3::kZero; }
		void UpdateMassDistribution(bool autoMassEnabled) override {}
		void SetTransform(const Vector3& position, const Quaternion& rotation) override;
		void GetTransform(Vector3& outPosition, Quaternion& outRotation) override;
		void SetCenterOfMass(const Vector3& position, const Quaternion& rotation) override {}
		void GetCenterOfMass(Vector3& outPosition, Quaternion& outRotation) override;
		void SetSolverIterationCounts(u32 positionCount, u32 velocityCount) override {}
		void AddToScene(PhysicsScene& scene) override {}
		void RemoveFromScene() override {}
		void AttachShape(const SPtr<ColliderShape>& shape) override {}
		void DetachShape(const SPtr<ColliderShape>& shape) override {}

	private:
		Vector3 mPosition = Vector3::kZero;
		Quaternion mRotation = Quaternion::kIdentity;
	};

	/** @} */
} // namespace b3d
