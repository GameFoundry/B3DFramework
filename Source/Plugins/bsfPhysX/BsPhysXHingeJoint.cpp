//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsPhysXHingeJoint.h"
#include "BsFPhysXJoint.h"
#include "BsPhysXRigidbody.h"
#include "PxRigidDynamic.h"

using namespace physx;
using namespace b3d;

PhysXHingeJoint::PhysXHingeJoint(PxPhysics* physx, const HingeJointCreateInformation& createInformation)
{
	PxRigidActor* actor0 = nullptr;
	if(createInformation.Bodies[0].Body != nullptr)
		actor0 = static_cast<PhysXRigidbody&>(createInformation.Bodies[0].Body->GetImplementation()).GetPxRigidDynamic();

	PxRigidActor* actor1 = nullptr;
	if(createInformation.Bodies[1].Body != nullptr)
		actor1 = static_cast<PhysXRigidbody&>(createInformation.Bodies[1].Body->GetImplementation()).GetPxRigidDynamic();

	PxTransform tfrm0 = ToPxTransform(createInformation.Bodies[0].Position, createInformation.Bodies[0].Rotation);
	PxTransform tfrm1 = ToPxTransform(createInformation.Bodies[1].Position, createInformation.Bodies[1].Rotation);

	PxRevoluteJoint* joint = PxRevoluteJointCreate(*physx, actor0, tfrm0, actor1, tfrm1);
	joint->userData = this;

	mInternal.Initialize(*joint, createInformation);

	PxRevoluteJointFlags flags;

	if(((u32)createInformation.Flag & (u32)HingeJointFlag::Limit) != 0)
		flags |= PxRevoluteJointFlag::eLIMIT_ENABLED;

	if(((u32)createInformation.Flag & (u32)HingeJointFlag::Drive) != 0)
		flags |= PxRevoluteJointFlag::eDRIVE_ENABLED;

	joint->setRevoluteJointFlags(flags);

	// Must be set after global flags, as it will append to them.
	// Calls to virtual methods are okay here.
	PhysXHingeJoint::SetLimit(createInformation.Limit);
	PhysXHingeJoint::SetDrive(createInformation.Drive);
}

LimitAngularRange PhysXHingeJoint::GetLimit() const
{
	PxJointAngularLimitPair pxLimit = GetPxRevoluteJoint().getLimit();

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
	PxJointAngularLimitPair pxLimit(limit.Lower.GetValueInRadians(), limit.Upper.GetValueInRadians(), limit.ContactDist);
	pxLimit.stiffness = limit.Spring.Stiffness;
	pxLimit.damping = limit.Spring.Damping;
	pxLimit.restitution = limit.Restitution;

	GetPxRevoluteJoint().setLimit(pxLimit);
}

HingeJointDrive PhysXHingeJoint::GetDrive() const
{
	HingeJointDrive drive;
	drive.Speed = GetPxRevoluteJoint().getDriveVelocity();
	drive.ForceLimit = GetPxRevoluteJoint().getDriveForceLimit();
	drive.GearRatio = GetPxRevoluteJoint().getDriveGearRatio();
	drive.FreeSpin = GetPxRevoluteJoint().getRevoluteJointFlags() & PxRevoluteJointFlag::eDRIVE_FREESPIN;

	return drive;
}

void PhysXHingeJoint::SetDrive(const HingeJointDrive& drive)
{
	GetPxRevoluteJoint().setDriveVelocity(drive.Speed);
	GetPxRevoluteJoint().setDriveForceLimit(drive.ForceLimit);
	GetPxRevoluteJoint().setDriveGearRatio(drive.GearRatio);
	GetPxRevoluteJoint().setRevoluteJointFlag(PxRevoluteJointFlag::eDRIVE_FREESPIN, drive.FreeSpin);
}
