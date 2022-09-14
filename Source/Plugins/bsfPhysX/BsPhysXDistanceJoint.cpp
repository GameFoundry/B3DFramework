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

		joint->setDistanceJointFlags(flags);
	}

	PhysXDistanceJoint::~PhysXDistanceJoint()
	{
		bs_delete(mInternal);
	}

	float PhysXDistanceJoint::getDistance() const
	{
		return getInternal()->getDistance();
	}

	float PhysXDistanceJoint::getMinDistance() const
	{
		return getInternal()->getMinDistance();
	}

	void PhysXDistanceJoint::setMinDistance(float value)
	{
		getInternal()->setMinDistance(value);
	}

	float PhysXDistanceJoint::getMaxDistance() const
	{
		return getInternal()->getMaxDistance();
	}

	void PhysXDistanceJoint::setMaxDistance(float value)
	{
		getInternal()->setMaxDistance(value);
	}

	float PhysXDistanceJoint::getTolerance() const
	{
		return getInternal()->getTolerance();
	}

	void PhysXDistanceJoint::setTolerance(float value)
	{
		getInternal()->setTolerance(value);
	}

	Spring PhysXDistanceJoint::getSpring() const
	{
		float damping = getInternal()->getDamping();
		float stiffness = getInternal()->getStiffness();

		return Spring(stiffness, damping);
	}

	void PhysXDistanceJoint::setSpring(const Spring& value)
	{
		getInternal()->setDamping(value.damping);
		getInternal()->setStiffness(value.stiffness);
	}

	void PhysXDistanceJoint::setFlag(DistanceJointFlag flag, bool enabled)
	{
		getInternal()->setDistanceJointFlag(toPxFlag(flag), enabled);
	}

	bool PhysXDistanceJoint::hasFlag(DistanceJointFlag flag) const
	{
		return getInternal()->getDistanceJointFlags() & toPxFlag(flag);
	}

	PxDistanceJoint* PhysXDistanceJoint::getInternal() const
	{
		FPhysXJoint* internal = static_cast<FPhysXJoint*>(mInternal);

		return static_cast<PxDistanceJoint*>(internal->GetInternalInternal());
	}
}
