//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsNullPhysicsPrerequisites.h"
#include "Physics/BsRigidbody.h"
#include "Math/BsVector3.h"
#include "Math/BsQuaternion.h"

namespace bs
{
	/** @addtogroup NullPhysics
	 *  @{
	 */

	/** Null implementation of a Rigidbody. */
	class NullPhysicsRigidbody : public Rigidbody
	{
	public:
		NullPhysicsRigidbody(const HSceneObject& linkedSO);
		~NullPhysicsRigidbody() = default;

		/** @copydoc Rigidbody::move */
		void Move(const Vector3& position);

		/** @copydoc Rigidbody::rotate */
		void Rotate(const Quaternion& rotation);

		/** @copydoc Rigidbody::getPosition */
		Vector3 GetPosition() const;

		/** @copydoc Rigidbody::getRotation */
		Quaternion GetRotation() const;

		/** @copydoc Rigidbody::setTransform */
		void SetTransform(const Vector3& pos, const Quaternion& rot);

		/** @copydoc Rigidbody::setMass */
		void SetMass(float mass) { mMass = mass; }

		/** @copydoc Rigidbody::getMass */
		float GetMass() const { return mMass; }

		/** @copydoc Rigidbody::setIsKinematic */
		void SetIsKinematic(bool kinematic) { mIsKinematic = kinematic; }

		/** @copydoc Rigidbody::getIsKinematic */
		bool GetIsKinematic() const { return mIsKinematic; }

		/** @copydoc Rigidbody::isSleeping */
		bool IsSleeping() const { return false; }

		/** @copydoc Rigidbody::sleep */
		void Sleep() {}

		/** @copydoc Rigidbody::wakeUp */
		void WakeUp() {}

		/** @copydoc Rigidbody::setSleepThreshold */
		void SetSleepThreshold(float threshold) { mSleepThreshold = threshold; }

		/** @copydoc Rigidbody::getSleepThreshold */
		float GetSleepThreshold() const { return mSleepThreshold; }

		/** @copydoc Rigidbody::setUseGravity */
		void SetUseGravity(bool gravity) { mUseGravity = gravity; }

		/** @copydoc Rigidbody::getUseGravity */
		bool GetUseGravity() const { return mUseGravity; }

		/** @copydoc Rigidbody::setVelocity */
		void SetVelocity(const Vector3& velocity) { mVelocity = velocity; }

		/** @copydoc Rigidbody::getVelocity */
		Vector3 GetVelocity() const { return mVelocity; }

		/** @copydoc Rigidbody::setAngularVelocity */
		void SetAngularVelocity(const Vector3& velocity) { mAngularVelocity = velocity; }

		/** @copydoc Rigidbody::getAngularVelocity */
		Vector3 GetAngularVelocity() const override { return mAngularVelocity; }

		/** @copydoc Rigidbody::setDrag */
		void SetDrag(float drag) override { mDrag = drag; }

		/** @copydoc Rigidbody::getDrag */
		float GetDrag() const override { return mDrag; }

		/** @copydoc Rigidbody::setAngularDrag */
		void SetAngularDrag(float drag) override { mAngularDrag = drag; }

		/** @copydoc Rigidbody::getAngularDrag */
		float GetAngularDrag() const override { return mAngularDrag; }

		/** @copydoc Rigidbody::setInertiaTensor */
		void SetInertiaTensor(const Vector3& tensor) override { mInertiaTensor = tensor; }

		/** @copydoc Rigidbody::getInertiaTensor */
		Vector3 GetInertiaTensor() const override { return mInertiaTensor; }

		/** @copydoc Rigidbody::setMaxAngularVelocity */
		void SetMaxAngularVelocity(float maxVelocity) override { mMaxAngularVelocity = maxVelocity; }

		/** @copydoc Rigidbody::getMaxAngularVelocity */
		float GetMaxAngularVelocity() const override { return mMaxAngularVelocity; }

		/** @copydoc Rigidbody::setCenterOfMass */
		void SetCenterOfMass(const Vector3& position, const Quaternion& rotation) override;

		/** @copydoc Rigidbody::getCenterOfMassPosition */
		Vector3 GetCenterOfMassPosition() const override { return mCenterOfMassPosition; }

		/** @copydoc Rigidbody::getCenterOfMassRotation */
		Quaternion GetCenterOfMassRotation() const override { return mCenterOfMassRotation; }

		/** @copydoc Rigidbody::setPositionSolverCount */
		void SetPositionSolverCount(u32 count) override { mPositionSolverCount = count; }

		/** @copydoc Rigidbody::getPositionSolverCount */
		u32 GetPositionSolverCount() const override { return mPositionSolverCount; }

		/** @copydoc Rigidbody::setVelocitySolverCount */
		void SetVelocitySolverCount(u32 count) override { mVelocitySolverCount = count; }

		/** @copydoc Rigidbody::getVelocitySolverCount */
		u32 GetVelocitySolverCount() const override { return mVelocitySolverCount; }

		/** @copydoc Rigidbody::addForce */
		void AddForce(const Vector3& force, ForceMode mode = ForceMode::Force) override {}

		/** @copydoc Rigidbody::addTorque */
		void AddTorque(const Vector3& torque, ForceMode mode = ForceMode::Force) override {}

		/** @copydoc Rigidbody::addForceAtPoint */
		void AddForceAtPoint(const Vector3& force, const Vector3& position, PointForceMode mode = PointForceMode::Force) override {}

		/** @copydoc Rigidbody::getVelocityAtPoint */
		Vector3 GetVelocityAtPoint(const Vector3& point) const override { return Vector3::ZERO; }

		/** @copydoc Rigidbody::addCollider */
		void AddCollider(Collider* collider) override {}

		/** @copydoc Rigidbody::removeCollider */
		void RemoveCollider(Collider* collider) override {}

		/** @copydoc Rigidbody::removeColliders */
		void RemoveColliders() override {}

	private:
		Vector3 mPosition = Vector3::ZERO;
		Quaternion mRotation = Quaternion::IDENTITY;
		float mMass = 0.0f;
		bool mIsKinematic = false;
		float mSleepThreshold = 0.1f;
		bool mUseGravity = false;
		Vector3 mVelocity = Vector3::ZERO;
		Vector3 mAngularVelocity = Vector3::ZERO;
		float mDrag = 0.0f;
		float mAngularDrag = 0.0f;
		Vector3 mInertiaTensor = Vector3::ZERO;
		float mMaxAngularVelocity = std::numeric_limits<float>::max();
		Vector3 mCenterOfMassPosition = Vector3::ZERO;
		Quaternion mCenterOfMassRotation = Quaternion::IDENTITY;
		u32 mPositionSolverCount = 0;
		u32 mVelocitySolverCount = 0;
	};

	/** @} */
} // namespace bs
