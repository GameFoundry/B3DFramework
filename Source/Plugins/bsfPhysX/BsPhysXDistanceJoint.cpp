//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsPhysXDistanceJoint.h"
#include "BsFPhysXJoint.h"
#include "BsPhysXRigidbody.h"
#include "PxRigidDynamic.h"

using namespace physx;

namespace bs
{
	PxDistanceJointFlag::Enum ToPxFlag(DistanceJointFlag flag)
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
			actor0 = static_cast<PhysXRigidbody*>(desc.bodies[0].body)->_getInternal();

		PxRigidActor* actor1 = nullptr;
		if (desc.bodies[1].body != nullptr)
			actor1 = static_cast<PhysXRigidbody*>(desc.bodies[1].body)->_getInternal();

		PxTransform tfrm0 = toPxTransform(desc.bodies[0].position, desc.bodies[0].rotation);
		PxTransform tfrm1 = toPxTransform(desc.bodies[1].position, desc.bodies[1].rotation);

		PxDistanceJoint* joint = PxDistanceJointCreate(*physx, actor0, tfrm0, actor1, tfrm1);
		joint->userData = this;

		mInternal = bs_new<FPhysXJoint>(joint, desc);

		// Calls to virtual methods are okay here
		setMinDistance(desc.minDistance);
		setMaxDistance(desc.maxDistance);
		setTolerance(desc.tolerance);
		setSpring(desc.spring);
		
		PxDistanceJointFlags flags;
		
		if(((UINT32)desc.flag & (UINT32)DistanceJointFlag::MaxDistance) != 0)
			flags |= PxDistanceJointFlag::eMAX_DISTANCE_ENABLED;

		if (((UINT32)desc.flag & (UINT32)DistanceJointFlag::MinDistance) != 0)
			flags |= PxDistanceJointFlag::eMIN_DISTANCE_ENABLED;

		if (((UINT32)desc.flag & (UINT32)DistanceJointFlag::Spring) != 0)
			flags |= PxDistanceJointFlag::eSPRING_ENABLED;

		joint->SetDistanceJointFlags(flags);
	}

	PhysXDistanceJoint::~PhysXDistanceJoint()
	{
		bs_delete(mInternal);
	}

	float PhysXDistanceJoint::GetDistance() const
	{
		return GetInternal()->GetDistance();
	}

	float PhysXDistanceJoint::GetMinDistance() const
	{
		return GetInternal()->GetMinDistance();
	}

	void PhysXDistanceJoint::SetMinDistance(float value)
	{
		getInternal()->SetMinDistance(value);
	}

	float PhysXDistanceJoint::GetMaxDistance() const
	{
		return GetInternal()->GetMaxDistance();
	}

	void PhysXDistanceJoint::SetMaxDistance(float value)
	{
		getInternal()->SetMaxDistance(value);
	}

	float PhysXDistanceJoint::GetTolerance() const
	{
		return GetInternal()->GetTolerance();
	}

	void PhysXDistanceJoint::SetTolerance(float value)
	{
		getInternal()->SetTolerance(value);
	}

	Spring PhysXDistanceJoint::GetSpring() const
	{
		float damping = getInternal()->GetDamping();
		float stiffness = getInternal()->GetStiffness();

		return Spring(stiffness, damping);
	}

	void PhysXDistanceJoint::SetSpring(const Spring& value)
	{
		getInternal()->SetDamping(value.damping);
		getInternal()->SetStiffness(value.stiffness);
	}

	void PhysXDistanceJoint::SetFlag(DistanceJointFlag flag, bool enabled)
	{
		getInternal()->SetDistanceJointFlag(toPxFlag(flag), enabled);
	}

	bool PhysXDistanceJoint::HasFlag(DistanceJointFlag flag) const
	{
		return GetInternal()->GetDistanceJointFlags() & toPxFlag(flag);
	}

	PxDistanceJoint* PhysXDistanceJoint::getInternal() const
	{
		FPhysXJoint* internal = static_cast<FPhysXJoint*>(mInternal);

		return static_cast<PxDistanceJoint*>(internal->_getInternal());
	}
}
