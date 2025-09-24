//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DNullPhysicsPrerequisites.h"
#include "Physics/B3DRigidbody.h"
#include "Math/B3DVector3.h"
#include "Math/B3DQuaternion.h"

namespace b3d
{
	/** @addtogroup NullPhysics
	 *  @{
	 */

	/** Null implementation of a Rigidbody. */
	class NullPhysicsRigidbody : public Rigidbody
	{
	public:
		NullPhysicsRigidbody(const HSceneObject& linkedSO);
		~NullPhysicsRigidbody() override = default;

		void Move(const Vector3& position) override;
		void Rotate(const Quaternion& rotation) override;
		Vector3 GetPosition() const override;
		Quaternion GetRotation() const override;
		void SetTransform(const Vector3& pos, const Quaternion& rot) override;
		void SetMass(float mass) override { mMass = mass; }
		float GetMass() const override { return mMass; }
		void SetIsKinematic(bool kinematic) override { mIsKinematic = kinematic; }
		bool GetIsKinematic() const override { return mIsKinematic; }
		bool IsSleeping() const override { return false; }
		void Sleep() override {}
		void WakeUp() override {}
		void SetSleepThreshold(float threshold) override { mSleepThreshold = threshold; }
		float GetSleepThreshold() const override { return mSleepThreshold; }
		void SetUseGravity(bool gravity) override { mUseGravity = gravity; }
		bool GetUseGravity() const override { return mUseGravity; }
		void SetVelocity(const Vector3& velocity) override { mVelocity = velocity; }
		Vector3 GetVelocity() const override { return mVelocity; }
		void SetAngularVelocity(const Vector3& velocity) override { mAngularVelocity = velocity; }
		Vector3 GetAngularVelocity() const override { return mAngularVelocity; }
		void SetDrag(float drag) override { mDrag = drag; }
		float GetDrag() const override { return mDrag; }
		void SetAngularDrag(float drag) override { mAngularDrag = drag; }
		float GetAngularDrag() const override { return mAngularDrag; }
		void SetInertiaTensor(const Vector3& tensor) override { mInertiaTensor = tensor; }
		Vector3 GetInertiaTensor() const override { return mInertiaTensor; }
		void SetMaxAngularVelocity(float maxVelocity) override { mMaxAngularVelocity = maxVelocity; }
		float GetMaxAngularVelocity() const override { return mMaxAngularVelocity; }
		void SetCenterOfMass(const Vector3& position, const Quaternion& rotation) override;
		Vector3 GetCenterOfMassPosition() const override { return mCenterOfMassPosition; }
		Quaternion GetCenterOfMassRotation() const override { return mCenterOfMassRotation; }
		void SetPositionSolverCount(u32 count) override { mPositionSolverCount = count; }
		u32 GetPositionSolverCount() const override { return mPositionSolverCount; }
		void SetVelocitySolverCount(u32 count) override { mVelocitySolverCount = count; }
		u32 GetVelocitySolverCount() const override { return mVelocitySolverCount; }
		void AddForce(const Vector3& force, ForceMode mode = ForceMode::Force) override {}
		void AddTorque(const Vector3& torque, ForceMode mode = ForceMode::Force) override {}
		void AddForceAtPoint(const Vector3& force, const Vector3& position, PointForceMode mode = PointForceMode::Force) override {}
		Vector3 GetVelocityAtPoint(const Vector3& point) const override { return Vector3::kZero; }
		void AddCollider(Collider* collider) override {}
		void RemoveCollider(Collider* collider) override {}
		void RemoveColliders() override {}

	private:
		Vector3 mPosition = Vector3::kZero;
		Quaternion mRotation = Quaternion::kIdentity;
		float mMass = 0.0f;
		bool mIsKinematic = false;
		float mSleepThreshold = 0.1f;
		bool mUseGravity = false;
		Vector3 mVelocity = Vector3::kZero;
		Vector3 mAngularVelocity = Vector3::kZero;
		float mDrag = 0.0f;
		float mAngularDrag = 0.0f;
		Vector3 mInertiaTensor = Vector3::kZero;
		float mMaxAngularVelocity = std::numeric_limits<float>::max();
		Vector3 mCenterOfMassPosition = Vector3::kZero;
		Quaternion mCenterOfMassRotation = Quaternion::kIdentity;
		u32 mPositionSolverCount = 0;
		u32 mVelocitySolverCount = 0;
	};

	/** @} */
} // namespace b3d
