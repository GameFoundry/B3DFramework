//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsPhysXDistanceJoint.h"
#include "BsFPhysXJoint.h"
#include "BsPhysXRigidbody.h"
#include "PxRigidDynamic.h"

using namespace physx;

namespace bs
{
	PxDistanceJointFlag::Enum toPxFlag(DistanceJointFlag flag)
	{
		switch (flag)
		{
		case DistanceJointFlag::MaxDistance:
			return PxDistanceJointFlag::eMAX_DISTANCE_ENABLED;
		case DistanceJointFlag::MinDistance:
			return PxDistanceJointFlag::eMIN_DISTANCE_ENABLED;
		default:
		case DistanceJointFlag::Spring:
			return PxDistanceJointFlag::eSPRING_ENABLED;
		}
	}

	PhysXDistanceJoint::PhysXDistanceJoint(PxPhysics* physx, const DISTANCE_JOINT_DESC& desc)
		:DistanceJoint(desc)
	{
		PxRigidActor* actor0 = nullptr;
		if (desc.bodies[0].body != nullptr)
			actor0 = static_cast<PhysXRigidbody*>(desc.bodies[0].body)->GetInternalInternal();

		PxRigidActor* actor1 = nullptr;
		if (desc.bodies[1].body != nullptr)
			actor1 = static_cast<PhysXRigidbody*>(desc.bodies[1].body)->GetInternalInternal();

		PxTransform tfrm0 = toPxTransform(desc.bodies[0].position, desc.bodies[0].rotation);
		PxTransform tfrm1 = toPxTransform(desc.bodies[1].position, desc.bodies[1].rotation);

		PxDistanceJoint* joint = PxDistanceJointCreate(*physx, actor0, tfrm0, actor1, tfrm1);
		joint->userData = this;

		mInternal = bs_new<FPhysXJoint>(joint, desc);

		// Calls to virtual methods are okay here
		SetMinDistance(desc.minDistance);
		SetMaxDistance(desc.maxDistance);
		SetTolerance(desc.tolerance);
		SetSpring(desc.spring);
		
		PxDistanceJointFlags flags;
		
		if(((UINT32)desc.flag & (UINT32)DistanceJointFlag::MaxDistance) != 0)
			flags |= PxDistanceJointFlag::eMAX_DISTANCE_ENABLED;

		if (((UINT32)desc.flag & (UINT32)DistanceJointFlag::MinDistance) != 0)
			flags |= PxDistanceJointFlag::eMIN_DISTANCE_ENABLED;

		if (((UINT32)desc.flag & (UINT32)DistanceJointFlag::Spring) != 0)
			flags |= PxDistanceJointFlag::eSPRING_ENABLED;

		joint->setDistanceJointFlags(flags);
	}

	PhysXDistanceJoint::~PhysXDistanceJoint()
	{
		bs_delete(mInternal);
	}

	float PhysXDistanceJoint::GetDistance() const
	{
		return GetInternal()->getDistance();
	}

	float PhysXDistanceJoint::GetMinDistance() const
	{
		return GetInternal()->getMinDistance();
	}

	void PhysXDistanceJoint::SetMinDistance(float value)
	{
		GetInternal()->setMinDistance(value);
	}

	float PhysXDistanceJoint::GetMaxDistance() const
	{
		return GetInternal()->getMaxDistance();
	}

	void PhysXDistanceJoint::SetMaxDistance(float value)
	{
		GetInternal()->setMaxDistance(value);
	}

	float PhysXDistanceJoint::GetTolerance() const
	{
		return GetInternal()->getTolerance();
	}

	void PhysXDistanceJoint::SetTolerance(float value)
	{
		GetInternal()->setTolerance(value);
	}

	Spring PhysXDistanceJoint::GetSpring() const
	{
		float damping = GetInternal()->getDamping();
		float stiffness = GetInternal()->getStiffness();

		return Spring(stiffness, damping);
	}

	void PhysXDistanceJoint::SetSpring(const Spring& value)
	{
		GetInternal()->setDamping(value.damping);
		GetInternal()->setStiffness(value.stiffness);
	}

	void PhysXDistanceJoint::SetFlag(DistanceJointFlag flag, bool enabled)
	{
		GetInternal()->setDistanceJointFlag(toPxFlag(flag), enabled);
	}

	bool PhysXDistanceJoint::HasFlag(DistanceJointFlag flag) const
	{
		return GetInternal()->getDistanceJointFlags() & toPxFlag(flag);
	}

	PxDistanceJoint* PhysXDistanceJoint::GetInternal() const
	{
		FPhysXJoint* internal = static_cast<FPhysXJoint*>(mInternal);

		return static_cast<PxDistanceJoint*>(internal->GetInternalInternal());
	}
}
