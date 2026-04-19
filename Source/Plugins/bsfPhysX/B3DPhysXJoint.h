//************************************ B3D Framework - Copyright 2026 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DPhysXPrerequisites.h"
#include "Components/B3DJoint.h"
#include "B3DPhysXRigidbody.h"
#include "extensions/PxJoint.h"
#include "PxRigidDynamic.h"

namespace b3d
{
	/** @addtogroup PhysX
	 *  @{
	 */

	/** Converts an engine joint body flag into a PhysX variant. */
	inline physx::PxJointActorIndex::Enum ToJointActor(JointBody body)
	{
		if(body == JointBody::Target)
			return physx::PxJointActorIndex::eACTOR0;

		return physx::PxJointActorIndex::eACTOR1;
	}

	/** Common functionality for all PhysX joint types. */
	class PhysXJoint
	{
	public:
		PhysXJoint() = default;
		~PhysXJoint();

		/** Initializes the object. Must be called before performing any other operations on the object. */
		void Initialize(Joint& owner, physx::PxJoint& pxJoint, const JointCreateInformation& createInformation);

		/** @copydoc IJointImplementation::SetBody */
		void SetBody(JointBody body, Rigidbody* value);

		/** @copydoc IJointImplementation::GetBody */
		Rigidbody* GetBody(JointBody body) const;

		/** @copydoc IJointImplementation::GetPosition */
		Vector3 GetPosition(JointBody body) const;

		/** @copydoc IJointImplementation::GetRotation */
		Quaternion GetRotation(JointBody body) const;

		/** @copydoc IJointImplementation::SetTransform */
		void SetTransform(JointBody body, const Vector3& position, const Quaternion& rotation);

		/** @copydoc IJointImplementation::SetBreakForce */
		void SetBreakForce(float force);

		/** @copydoc IJointImplementation::GetBreakForce */
		float GetBreakForce() const;

		/** @copydoc IJointImplementation::SetBreakTorque */
		void SetBreakTorque(float torque);

		/** @copydoc IJointImplementation::GetBreakTorque */
		float GetBreakTorque() const;

		/** @copydoc IJointImplementation::GetEnableCollision */
		bool GetEnableCollision() const;

		/** @copydoc IJointImplementation::SetEnableCollision */
		void SetEnableCollision(bool value);

		/** Gets the internal PhysX joint object. */
		physx::PxJoint& GetPxJoint() const { return *mPxJoint; }

	protected:
		physx::PxJoint* mPxJoint = nullptr;
	};

	inline void PhysXJoint::Initialize(Joint& owner, physx::PxJoint& pxJoint, const JointCreateInformation& createInformation)
	{
		mPxJoint = &pxJoint;
		mPxJoint->userData = &owner;
		mPxJoint->setBreakForce(createInformation.BreakForce, createInformation.BreakTorque);
		mPxJoint->setConstraintFlag(physx::PxConstraintFlag::eCOLLISION_ENABLED, createInformation.EnableCollision);
	}

	inline PhysXJoint::~PhysXJoint()
	{
		mPxJoint->userData = nullptr;
		mPxJoint->release();
	}

	inline Rigidbody* PhysXJoint::GetBody(JointBody body) const
	{
		physx::PxRigidActor* actorA = nullptr;
		physx::PxRigidActor* actorB = nullptr;

		mPxJoint->getActors(actorA, actorB);

		physx::PxRigidActor* wantedActor = body == JointBody::Target ? actorA : actorB;
		if(wantedActor == nullptr)
			return nullptr;

		return (Rigidbody*)wantedActor->userData;
	}

	inline void PhysXJoint::SetBody(JointBody body, Rigidbody* value)
	{
		physx::PxRigidActor* actorA = nullptr;
		physx::PxRigidActor* actorB = nullptr;

		mPxJoint->getActors(actorA, actorB);

		physx::PxRigidActor* actor = nullptr;
		if(value != nullptr)
			actor = static_cast<PhysXRigidbody&>(value->GetImplementation()).GetPxRigidDynamic();

		if(body == JointBody::Target)
			actorA = actor;
		else
			actorB = actor;

		mPxJoint->setActors(actorA, actorB);
	}

	inline Vector3 PhysXJoint::GetPosition(JointBody body) const
	{
		physx::PxVec3 position = mPxJoint->getLocalPose(ToJointActor(body)).p;

		return FromPxVector(position);
	}

	inline Quaternion PhysXJoint::GetRotation(JointBody body) const
	{
		physx::PxQuat rotation = mPxJoint->getLocalPose(ToJointActor(body)).q;

		return FromPxQuaternion(rotation);
	}

	inline void PhysXJoint::SetTransform(JointBody body, const Vector3& position, const Quaternion& rotation)
	{
		physx::PxTransform transform = ToPxTransform(position, rotation);

		mPxJoint->setLocalPose(ToJointActor(body), transform);
	}

	inline float PhysXJoint::GetBreakForce() const
	{
		float force = 0.0f;
		float torque = 0.0f;

		mPxJoint->getBreakForce(force, torque);
		return force;
	}

	inline void PhysXJoint::SetBreakForce(float force)
	{
		float dummy = 0.0f;
		float torque = 0.0f;

		mPxJoint->getBreakForce(dummy, torque);
		mPxJoint->setBreakForce(force, torque);
	}

	inline float PhysXJoint::GetBreakTorque() const
	{
		float force = 0.0f;
		float torque = 0.0f;

		mPxJoint->getBreakForce(force, torque);
		return torque;
	}

	inline void PhysXJoint::SetBreakTorque(float torque)
	{
		float force = 0.0f;
		float dummy = 0.0f;

		mPxJoint->getBreakForce(force, dummy);
		mPxJoint->setBreakForce(force, torque);
	}

	inline bool PhysXJoint::GetEnableCollision() const
	{
		return mPxJoint->getConstraintFlags() & physx::PxConstraintFlag::eCOLLISION_ENABLED;
	}

	inline void PhysXJoint::SetEnableCollision(bool value)
	{
		mPxJoint->setConstraintFlag(physx::PxConstraintFlag::eCOLLISION_ENABLED, value);
	}

	/** @} */
} // namespace b3d
