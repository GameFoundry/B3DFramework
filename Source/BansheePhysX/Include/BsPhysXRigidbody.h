//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsPhysXPrerequisites.h"
#include "BsRigidbody.h"
#include "BsVector3.h"
#include "BsQuaternion.h"
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
		void move(const Vector3& position) override;

		/** @copydoc Rigidbody::rotate */
		void rotate(const Quaternion& rotation) override;

		/** @copydoc Rigidbody::getPosition */
		Vector3 getPosition() const override;

		/** @copydoc Rigidbody::getRotation */
		Quaternion getRotation() const override;

		/** @copydoc Rigidbody::setTransform */
		void setTransform(const Vector3& pos, const Quaternion& rot) override;

		/** @copydoc Rigidbody::setMass */
		void setMass(float mass) override;

		/** @copydoc Rigidbody::getMass */
		float getMass() const override;

		/** @copydoc Rigidbody::setIsKinematic */
		void setIsKinematic(bool kinematic) override;

		/** @copydoc Rigidbody::getIsKinematic */
		bool getIsKinematic() const override;

		/** @copydoc Rigidbody::isSleeping */
		bool isSleeping() const override;

		/** @copydoc Rigidbody::sleep */
		void sleep() override;

		/** @copydoc Rigidbody::wakeUp */
		void wakeUp() override;

		/** @copydoc Rigidbody::setSleepThreshold */
		void setSleepThreshold(float threshold) override;

		/** @copydoc Rigidbody::getSleepThreshold */
		float getSleepThreshold() const override;

		/** @copydoc Rigidbody::setUseGravity */
		void setUseGravity(bool gravity) override;

		/** @copydoc Rigidbody::getUseGravity */
		bool getUseGravity() const override;

		/** @copydoc Rigidbody::setVelocity */
		void setVelocity(const Vector3& velocity) override;

		/** @copydoc Rigidbody::getVelocity */
		Vector3 getVelocity() const override;

		/** @copydoc Rigidbody::setAngularVelocity */
		void setAngularVelocity(const Vector3& velocity) override;

		/** @copydoc Rigidbody::getAngularVelocity */
		Vector3 getAngularVelocity() const override;

		/** @copydoc Rigidbody::setDrag */
		void setDrag(float drag) override;

		/** @copydoc Rigidbody::getDrag */
		float getDrag() const override;

		/** @copydoc Rigidbody::setAngularDrag */
		void setAngularDrag(float drag) override;

		/** @copydoc Rigidbody::getAngularDrag */
		float getAngularDrag() const override;

		/** @copydoc Rigidbody::setInertiaTensor */
		void setInertiaTensor(const Vector3& tensor) override;

		/** @copydoc Rigidbody::getInertiaTensor */
		Vector3 getInertiaTensor() const override;

		/** @copydoc Rigidbody::setMaxAngularVelocity */
		void setMaxAngularVelocity(float maxVelocity) override;

		/** @copydoc Rigidbody::getMaxAngularVelocity */
		float getMaxAngularVelocity() const override;

		/** @copydoc Rigidbody::setCenterOfMass */
		void setCenterOfMass(const Vector3& position, const Quaternion& rotation) override;

		/** @copydoc Rigidbody::getCenterOfMassPosition */
		Vector3 getCenterOfMassPosition() const override;

		/** @copydoc Rigidbody::getCenterOfMassRotation */
		Quaternion getCenterOfMassRotation() const override;

		/** @copydoc Rigidbody::setPositionSolverCount */
		void setPositionSolverCount(UINT32 count) override;

		/** @copydoc Rigidbody::getPositionSolverCount */
		UINT32 getPositionSolverCount() const override;

		/** @copydoc Rigidbody::setVelocitySolverCount */
		void setVelocitySolverCount(UINT32 count) override;

		/** @copydoc Rigidbody::getVelocitySolverCount */
		UINT32 getVelocitySolverCount() const override;

		/** @copydoc Rigidbody::setFlags */
		void setFlags(Flag flags) override;

		/** @copydoc Rigidbody::addForce */
		void addForce(const Vector3& force, ForceMode mode = ForceMode::Force) override;

		/** @copydoc Rigidbody::addTorque */
		void addTorque(const Vector3& torque, ForceMode mode = ForceMode::Force) override;

		/** @copydoc Rigidbody::addForceAtPoint */
		void addForceAtPoint(const Vector3& force, const Vector3& position,
			PointForceMode mode = PointForceMode::Force) override;

		/** @copydoc Rigidbody::getVelocityAtPoint */
		Vector3 getVelocityAtPoint(const Vector3& point) const override;

		/** @copydoc Rigidbody::addCollider */
		void addCollider(FCollider* collider) override;

		/** @copydoc Rigidbody::removeCollider */
		void removeCollider(FCollider* collider) override;

		/** @copydoc Rigidbody::removeColliders */
		void removeColliders() override;

		/** @copydoc Rigidbody::updateMassDistribution */
		void updateMassDistribution() override;
		
		/** Returns the internal PhysX dynamic actor. */
		physx::PxRigidDynamic* _getInternal() const { return mInternal; }

	private:
		physx::PxRigidDynamic* mInternal;
	};

	/** @} */
}