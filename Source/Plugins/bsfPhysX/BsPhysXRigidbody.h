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

		/** @copydoc Rigidbody::move */
		void Move(const Vector3& position) override;

		/** @copydoc Rigidbody::rotate */
		void Rotate(const Quaternion& rotation) override;

		/** @copydoc Rigidbody::getPosition */
		Vector3 GetPosition() const override;

		/** @copydoc Rigidbody::getRotation */
		Quaternion GetRotation() const override;

		/** @copydoc Rigidbody::setTransform */
		void SetTransform(const Vector3& pos, const Quaternion& rot) override;

		/** @copydoc Rigidbody::setMass */
		void SetMass(float mass) override;

		/** @copydoc Rigidbody::getMass */
		float GetMass() const override;

		/** @copydoc Rigidbody::setIsKinematic */
		void SetIsKinematic(bool kinematic) override;

		/** @copydoc Rigidbody::getIsKinematic */
		bool GetIsKinematic() const override;

		/** @copydoc Rigidbody::isSleeping */
		bool IsSleeping() const override;

		/** @copydoc Rigidbody::sleep */
		void Sleep() override;

		/** @copydoc Rigidbody::wakeUp */
		void WakeUp() override;

		/** @copydoc Rigidbody::setSleepThreshold */
		void SetSleepThreshold(float threshold) override;

		/** @copydoc Rigidbody::getSleepThreshold */
		float GetSleepThreshold() const override;

		/** @copydoc Rigidbody::setUseGravity */
		void SetUseGravity(bool gravity) override;

		/** @copydoc Rigidbody::getUseGravity */
		bool GetUseGravity() const override;

		/** @copydoc Rigidbody::setVelocity */
		void SetVelocity(const Vector3& velocity) override;

		/** @copydoc Rigidbody::getVelocity */
		Vector3 GetVelocity() const override;

		/** @copydoc Rigidbody::setAngularVelocity */
		void SetAngularVelocity(const Vector3& velocity) override;

		/** @copydoc Rigidbody::getAngularVelocity */
		Vector3 GetAngularVelocity() const override;

		/** @copydoc Rigidbody::setDrag */
		void SetDrag(float drag) override;

		/** @copydoc Rigidbody::getDrag */
		float GetDrag() const override;

		/** @copydoc Rigidbody::setAngularDrag */
		void SetAngularDrag(float drag) override;

		/** @copydoc Rigidbody::getAngularDrag */
		float GetAngularDrag() const override;

		/** @copydoc Rigidbody::setInertiaTensor */
		void SetInertiaTensor(const Vector3& tensor) override;

		/** @copydoc Rigidbody::getInertiaTensor */
		Vector3 GetInertiaTensor() const override;

		/** @copydoc Rigidbody::setMaxAngularVelocity */
		void SetMaxAngularVelocity(float maxVelocity) override;

		/** @copydoc Rigidbody::getMaxAngularVelocity */
		float GetMaxAngularVelocity() const override;

		/** @copydoc Rigidbody::setCenterOfMass */
		void SetCenterOfMass(const Vector3& position, const Quaternion& rotation) override;

		/** @copydoc Rigidbody::getCenterOfMassPosition */
		Vector3 GetCenterOfMassPosition() const override;

		/** @copydoc Rigidbody::getCenterOfMassRotation */
		Quaternion GetCenterOfMassRotation() const override;

		/** @copydoc Rigidbody::setPositionSolverCount */
		void SetPositionSolverCount(u32 count) override;

		/** @copydoc Rigidbody::getPositionSolverCount */
		u32 GetPositionSolverCount() const override;

		/** @copydoc Rigidbody::setVelocitySolverCount */
		void SetVelocitySolverCount(u32 count) override;

		/** @copydoc Rigidbody::getVelocitySolverCount */
		u32 GetVelocitySolverCount() const override;

		/** @copydoc Rigidbody::setFlags */
		void SetFlags(RigidbodyFlag flags) override;

		/** @copydoc Rigidbody::addForce */
		void AddForce(const Vector3& force, ForceMode mode = ForceMode::Force) override;

		/** @copydoc Rigidbody::addTorque */
		void AddTorque(const Vector3& torque, ForceMode mode = ForceMode::Force) override;

		/** @copydoc Rigidbody::addForceAtPoint */
		void AddForceAtPoint(const Vector3& force, const Vector3& position, PointForceMode mode = PointForceMode::Force) override;

		/** @copydoc Rigidbody::getVelocityAtPoint */
		Vector3 GetVelocityAtPoint(const Vector3& point) const override;

		/** @copydoc Rigidbody::addCollider */
		void AddCollider(Collider* collider) override;

		/** @copydoc Rigidbody::removeCollider */
		void RemoveCollider(Collider* collider) override;

		/** @copydoc Rigidbody::removeColliders */
		void RemoveColliders() override;

		/** @copydoc Rigidbody::updateMassDistribution */
		void UpdateMassDistribution() override;

		/** Returns the internal PhysX dynamic actor. */
		physx::PxRigidDynamic* GetInternalInternal() const { return mInternal; }

	private:
		physx::PxRigidDynamic* mInternal;
	};

	/** @} */
} // namespace bs
