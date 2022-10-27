//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsPhysXDistanceJoint.h"
#include "BsFPhysXJoint.h"
#include "BsPhysXRigidbody.h"
#include "PxRigidDynamic.h"

using namespace physx;

using namespace bs;

PxDistanceJointFlag::Enum toPxFlag(DistanceJointFlag flag)
{
	switch(flag)
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
	: DistanceJoint(desc)
{
	PxRigidActor* actor0 = nullptr;
	if(desc.Bodies[0].Body != nullptr)
		actor0 = static_cast<PhysXRigidbody*>(desc.Bodies[0].Body)->GetInternalInternal();

	PxRigidActor* actor1 = nullptr;
	if(desc.Bodies[1].Body != nullptr)
		actor1 = static_cast<PhysXRigidbody*>(desc.Bodies[1].Body)->GetInternalInternal();

	PxTransform tfrm0 = toPxTransform(desc.Bodies[0].Position, desc.Bodies[0].Rotation);
	PxTransform tfrm1 = toPxTransform(desc.Bodies[1].Position, desc.Bodies[1].Rotation);

	PxDistanceJoint* joint = PxDistanceJointCreate(*physx, actor0, tfrm0, actor1, tfrm1);
	joint->userData = this;

	mInternal = bs_new<FPhysXJoint>(joint, desc);

	// Calls to virtual methods are okay here
	SetMinDistance(desc.MinDistance);
	SetMaxDistance(desc.MaxDistance);
	SetTolerance(desc.Tolerance);
	SetSpring(desc.Spring);

	PxDistanceJointFlags flags;

	if(((u32)desc.Flag & (u32)DistanceJointFlag::MaxDistance) != 0)
		flags |= PxDistanceJointFlag::eMAX_DISTANCE_ENABLED;

	if(((u32)desc.Flag & (u32)DistanceJointFlag::MinDistance) != 0)
		flags |= PxDistanceJointFlag::eMIN_DISTANCE_ENABLED;

	if(((u32)desc.Flag & (u32)DistanceJointFlag::Spring) != 0)
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
	GetInternal()->setDamping(value.Damping);
	GetInternal()->setStiffness(value.Stiffness);
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
