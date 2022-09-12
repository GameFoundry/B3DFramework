//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsFPhysXJoint.h"
#include "BsPhysXRigidbody.h"
#include "Physics/BsJoint.h"
#include "PxRigidDynamic.h"

using namespace physx;

namespace bs
{
	PxJointActorIndex::Enum ToJointActor(JointBody body)
	{
		if (body == JointBody::Target)
			return PxJointActorIndex::eACTOR0;

		return PxJointActorIndex::eACTOR1;
	}

	FPhysXJoint::FPhysXJoint(physx::PxJoint* joint, const JOINT_DESC& desc)
		:FJoint(desc), mJoint(joint)
	{
		mJoint->SetBreakForce(desc.breakForce, desc.breakTorque);
		mJoint->SetConstraintFlag(PxConstraintFlag::eCOLLISION_ENABLED, desc.enableCollision);
	}

	FPhysXJoint::~FPhysXJoint()
	{
		mJoint->userData = nullptr;
		mJoint->Release();
	}

	Rigidbody* FPhysXJoint::getBody(JointBody body) const
	{
		PxRigidActor* actorA = nullptr;
		PxRigidActor* actorB = nullptr;

		mJoint->GetActors(actorA, actorB);

		PxRigidActor* wantedActor = body == JointBody::Target ? actorA : actorB;
		if (wantedActor == nullptr)
			return nullptr;

		return (Rigidbody*)wantedActor->userData;
	}

	void FPhysXJoint::SetBody(JointBody body, Rigidbody* value)
	{
		PxRigidActor* actorA = nullptr;
		PxRigidActor* actorB = nullptr;

		mJoint->GetActors(actorA, actorB);

		PxRigidActor* actor = nullptr;
		if (value != nullptr)
			actor = static_cast<PhysXRigidbody*>(value)->_getInternal();

		if (body == JointBody::Target)
			actorA = actor;
		else
			actorB = actor;

		mJoint->SetActors(actorA, actorB);
	}

	Vector3 FPhysXJoint::GetPosition(JointBody body) const
	{
		PxVec3 position = mJoint->GetLocalPose(toJointActor(body)).p;

		return FromPxVector(position);
	}

	Quaternion FPhysXJoint::GetRotation(JointBody body) const
	{
		PxQuat rotation = mJoint->GetLocalPose(toJointActor(body)).q;

		return FromPxQuaternion(rotation);
	}

	void FPhysXJoint::SetTransform(JointBody body, const Vector3& position, const Quaternion& rotation)
	{
		PxTransform transform = toPxTransform(position, rotation);

		mJoint->SetLocalPose(toJointActor(body), transform);
	}

	float FPhysXJoint::GetBreakForce() const
	{
		float force = 0.0f;
		float torque = 0.0f;

		mJoint->GetBreakForce(force, torque);
		return force;
	}

	void FPhysXJoint::SetBreakForce(float force)
	{
		float dummy = 0.0f;
		float torque = 0.0f;

		mJoint->GetBreakForce(dummy, torque);
		mJoint->SetBreakForce(force, torque);
	}

	float FPhysXJoint::GetBreakTorque() const
	{
		float force = 0.0f;
		float torque = 0.0f;

		mJoint->GetBreakForce(force, torque);
		return torque;
	}

	void FPhysXJoint::SetBreakTorque(float torque)
	{
		float force = 0.0f;
		float dummy = 0.0f;

		mJoint->GetBreakForce(force, dummy);
		mJoint->SetBreakForce(force, torque);
	}

	bool FPhysXJoint::GetEnableCollision() const
	{
		return mJoint->GetConstraintFlags() & PxConstraintFlag::eCOLLISION_ENABLED;
	}

	void FPhysXJoint::SetEnableCollision(bool value)
	{
		mJoint->SetConstraintFlag(PxConstraintFlag::eCOLLISION_ENABLED, value);
	}
}
