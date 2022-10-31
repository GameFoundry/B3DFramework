//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsPhysXHingeJoint.h"
#include "BsFPhysXJoint.h"
#include "BsPhysXRigidbody.h"
#include "PxRigidDynamic.h"

using namespace physx;

using namespace bs;

PxRevoluteJointFlag::Enum toPxFlag(HingeJointFlag flag)
{
	switch(flag)
	{
	case HingeJointFlag::Limit:
		return PxRevoluteJointFlag::eLIMIT_ENABLED;
	default:
	case HingeJointFlag::Drive:
		return PxRevoluteJointFlag::eDRIVE_ENABLED;
	}
}

PhysXHingeJoint::PhysXHingeJoint(PxPhysics* physx, const HINGE_JOINT_DESC& desc)
	: HingeJoint(desc)
{
	PxRigidActor* actor0 = nullptr;
	if(desc.Bodies[0].Body != nullptr)
		actor0 = static_cast<PhysXRigidbody*>(desc.Bodies[0].Body)->GetInternalInternal();

	PxRigidActor* actor1 = nullptr;
	if(desc.Bodies[1].Body != nullptr)
		actor1 = static_cast<PhysXRigidbody*>(desc.Bodies[1].Body)->GetInternalInternal();

	PxTransform tfrm0 = toPxTransform(desc.Bodies[0].Position, desc.Bodies[0].Rotation);
	PxTransform tfrm1 = toPxTransform(desc.Bodies[1].Position, desc.Bodies[1].Rotation);

	PxRevoluteJoint* joint = PxRevoluteJointCreate(*physx, actor0, tfrm0, actor1, tfrm1);
	joint->userData = this;

	mInternal = B3DNew<FPhysXJoint>(joint, desc);

	PxRevoluteJointFlags flags;

	if(((u32)desc.Flag & (u32)HingeJointFlag::Limit) != 0)
		flags |= PxRevoluteJointFlag::eLIMIT_ENABLED;

	if(((u32)desc.Flag & (u32)HingeJointFlag::Drive) != 0)
		flags |= PxRevoluteJointFlag::eDRIVE_ENABLED;

	joint->setRevoluteJointFlags(flags);

	// Must be set after global flags, as it will append to them.
	// Calls to virtual methods are okay here.
	SetLimit(desc.Limit);
	SetDrive(desc.Drive);
}

PhysXHingeJoint::~PhysXHingeJoint()
{
	B3DDelete(mInternal);
}

Radian PhysXHingeJoint::GetAngle() const
{
	return Radian(GetInternal()->getAngle());
}

float PhysXHingeJoint::GetSpeed() const
{
	return GetInternal()->getVelocity();
}

LimitAngularRange PhysXHingeJoint::GetLimit() const
{
	PxJointAngularLimitPair pxLimit = GetInternal()->getLimit();

	LimitAngularRange limit;
	limit.Lower = pxLimit.lower;
	limit.Upper = pxLimit.upper;
	limit.ContactDist = pxLimit.contactDistance;
	limit.Restitution = pxLimit.restitution;
	limit.Spring.Stiffness = pxLimit.stiffness;
	limit.Spring.Damping = pxLimit.damping;

	return limit;
}

void PhysXHingeJoint::SetLimit(const LimitAngularRange& limit)
{
	PxJointAngularLimitPair pxLimit(limit.Lower.ValueRadians(), limit.Upper.ValueRadians(), limit.ContactDist);
	pxLimit.stiffness = limit.Spring.Stiffness;
	pxLimit.damping = limit.Spring.Damping;
	pxLimit.restitution = limit.Restitution;

	GetInternal()->setLimit(pxLimit);
}

HingeJointDrive PhysXHingeJoint::GetDrive() const
{
	HingeJointDrive drive;
	drive.Speed = GetInternal()->getDriveVelocity();
	drive.ForceLimit = GetInternal()->getDriveForceLimit();
	drive.GearRatio = GetInternal()->getDriveGearRatio();
	drive.FreeSpin = GetInternal()->getRevoluteJointFlags() & PxRevoluteJointFlag::eDRIVE_FREESPIN;

	return drive;
}

void PhysXHingeJoint::SetDrive(const HingeJointDrive& drive)
{
	GetInternal()->setDriveVelocity(drive.Speed);
	GetInternal()->setDriveForceLimit(drive.ForceLimit);
	GetInternal()->setDriveGearRatio(drive.GearRatio);
	GetInternal()->setRevoluteJointFlag(PxRevoluteJointFlag::eDRIVE_FREESPIN, drive.FreeSpin);
}

void PhysXHingeJoint::SetFlag(HingeJointFlag flag, bool enabled)
{
	GetInternal()->setRevoluteJointFlag(toPxFlag(flag), enabled);
}

bool PhysXHingeJoint::HasFlag(HingeJointFlag flag) const
{
	return GetInternal()->getRevoluteJointFlags() & toPxFlag(flag);
}

PxRevoluteJoint* PhysXHingeJoint::GetInternal() const
{
	FPhysXJoint* internal = static_cast<FPhysXJoint*>(mInternal);

	return static_cast<PxRevoluteJoint*>(internal->GetInternalInternal());
}
