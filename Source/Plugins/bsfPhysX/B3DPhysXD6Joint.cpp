//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DPhysXD6Joint.h"
#include "B3DPhysXJoint.h"
#include "B3DPhysX.h"
#include "B3DPhysXRigidbody.h"
#include "PxRigidDynamic.h"

using namespace physx;
using namespace b3d;

static PxD6Drive::Enum ToPxDrive(D6JointDriveType drive)
{
	switch(drive)
	{
	default:
	case D6JointDriveType::X:
		return PxD6Drive::eX;
	case D6JointDriveType::Y:
		return PxD6Drive::eY;
	case D6JointDriveType::Z:
		return PxD6Drive::eZ;
	case D6JointDriveType::Twist:
		return PxD6Drive::eTWIST;
	case D6JointDriveType::SwingY:
		return PxD6Drive::eSWING1;
	case D6JointDriveType::SwingZ:
		return PxD6Drive::eSWING2;
	case D6JointDriveType::SLERP:
		return PxD6Drive::eSLERP;
	}
}

/**
 * Checks if a drive belongs to the provided angular drive configuration. PhysX only exposes the drives of the active
 * configuration. Accessing any other is an error in checked builds and, because eSLERP shares its storage with eSWING2,
 * can overwrite an unrelated drive in others.
 */
static bool IsDriveAccessible(D6JointDriveType drive, PxD6AngularDriveConfig::Enum config)
{
	switch(drive)
	{
	default:
		return true;
	case D6JointDriveType::Twist:
	case D6JointDriveType::SwingY:
	case D6JointDriveType::SwingZ:
		return config == PxD6AngularDriveConfig::eSWING_TWIST;
	case D6JointDriveType::SLERP:
		return config == PxD6AngularDriveConfig::eSLERP;
	}
}

PhysXD6Joint::PhysXD6Joint(PxPhysics* physx, Joint& owner, const D6JointCreateInformation& createInformation)
{
	PxRigidActor* actor0 = nullptr;
	if(createInformation.Bodies[0].Body.IsValid())
		actor0 = static_cast<PhysXRigidbody&>(createInformation.Bodies[0].Body->GetImplementation()).GetPxRigidDynamic();

	PxRigidActor* actor1 = nullptr;
	if(createInformation.Bodies[1].Body.IsValid())
		actor1 = static_cast<PhysXRigidbody&>(createInformation.Bodies[1].Body->GetImplementation()).GetPxRigidDynamic();

	PxTransform tfrm0 = ToPxTransform(createInformation.Bodies[0].Position, createInformation.Bodies[0].Rotation);
	PxTransform tfrm1 = ToPxTransform(createInformation.Bodies[1].Position, createInformation.Bodies[1].Rotation);

	PxD6Joint* joint = PxD6JointCreate(*physx, actor0, tfrm0, actor1, tfrm1);
	mInternal.Initialize(owner, *joint, createInformation);

	for(u32 i = 0; i < (u32)D6JointAxis::Count; i++)
		PhysXD6Joint::SetMotion((D6JointAxis)i, createInformation.Motion[i]);

	PhysXD6Joint::SetAngularDriveMode(createInformation.AngularDriveMode, createInformation.Drive);

	PhysXD6Joint::SetLimitLinear(createInformation.LimitLinear);
	PhysXD6Joint::SetLimitTwist(createInformation.LimitTwist);
	PhysXD6Joint::SetLimitSwing(createInformation.LimitSwing);

	PhysXD6Joint::SetDriveTransform(createInformation.DrivePosition, createInformation.DriveRotation);
	PhysXD6Joint::SetDriveVelocity(createInformation.DriveLinearVelocity, createInformation.DriveAngularVelocity);
}

LimitLinear PhysXD6Joint::GetLimitLinear() const
{
	PxJointLinearLimit pxLimit = GetPxD6Joint().getDistanceLimit();

	LimitLinear limit;
	limit.Extent = pxLimit.value;
	limit.Restitution = pxLimit.restitution;
	limit.Spring.Stiffness = pxLimit.stiffness;
	limit.Spring.Damping = pxLimit.damping;

	return limit;
}

void PhysXD6Joint::SetLimitLinear(const LimitLinear& limit)
{
	PxJointLinearLimit pxLimit(limit.Extent);
	pxLimit.stiffness = limit.Spring.Stiffness;
	pxLimit.damping = limit.Spring.Damping;
	pxLimit.restitution = limit.Restitution;

	GetPxD6Joint().setDistanceLimit(pxLimit);
}

LimitAngularRange PhysXD6Joint::GetLimitTwist() const
{
	PxJointAngularLimitPair pxLimit = GetPxD6Joint().getTwistLimit();

	LimitAngularRange limit;
	limit.Lower = pxLimit.lower;
	limit.Upper = pxLimit.upper;
	limit.Restitution = pxLimit.restitution;
	limit.Spring.Stiffness = pxLimit.stiffness;
	limit.Spring.Damping = pxLimit.damping;

	return limit;
}

void PhysXD6Joint::SetLimitTwist(const LimitAngularRange& limit)
{
	PxJointAngularLimitPair pxLimit(limit.Lower.GetValueInRadians(), limit.Upper.GetValueInRadians());
	pxLimit.stiffness = limit.Spring.Stiffness;
	pxLimit.damping = limit.Spring.Damping;
	pxLimit.restitution = limit.Restitution;

	GetPxD6Joint().setTwistLimit(pxLimit);
}

LimitConeRange PhysXD6Joint::GetLimitSwing() const
{
	PxJointLimitCone pxLimit = GetPxD6Joint().getSwingLimit();

	LimitConeRange limit;
	limit.YLimitAngle = pxLimit.yAngle;
	limit.ZLimitAngle = pxLimit.zAngle;
	limit.Restitution = pxLimit.restitution;
	limit.Spring.Stiffness = pxLimit.stiffness;
	limit.Spring.Damping = pxLimit.damping;

	return limit;
}

void PhysXD6Joint::SetLimitSwing(const LimitConeRange& limit)
{
	PxJointLimitCone pxLimit(limit.YLimitAngle.GetValueInRadians(), limit.ZLimitAngle.GetValueInRadians());
	pxLimit.stiffness = limit.Spring.Stiffness;
	pxLimit.damping = limit.Spring.Damping;
	pxLimit.restitution = limit.Restitution;

	GetPxD6Joint().setSwingLimit(pxLimit);
}

void PhysXD6Joint::SetAngularDriveMode(D6JointAngularDriveMode mode, TArrayView<const D6JointDrive> drives)
{
	GetPxD6Joint().setAngularDriveConfig(ToPxAngularDriveMode(mode));

	// PhysX resets the drives of a configuration when it becomes active, and ignores those of inactive configurations
	for(u32 i = 0; i < (u32)D6JointDriveType::Count; i++)
		SetDrive((D6JointDriveType)i, drives[i]);
}

D6JointDrive PhysXD6Joint::GetDrive(D6JointDriveType type) const
{
	if(!IsDriveAccessible(type, GetPxD6Joint().getAngularDriveConfig()))
		return D6JointDrive();

	PxD6JointDrive pxDrive = GetPxD6Joint().getDrive(ToPxDrive(type));

	D6JointDrive drive;
	drive.Acceleration = pxDrive.flags & PxD6JointDriveFlag::eACCELERATION;
	drive.Stiffness = pxDrive.stiffness;
	drive.Damping = pxDrive.damping;
	drive.ForceLimit = pxDrive.forceLimit;

	return drive;
}

void PhysXD6Joint::SetDrive(D6JointDriveType type, const D6JointDrive& drive)
{
	if(!IsDriveAccessible(type, GetPxD6Joint().getAngularDriveConfig()))
		return;

	PxD6JointDrive pxDrive;

	if(drive.Acceleration)
		pxDrive.flags = PxD6JointDriveFlag::eACCELERATION;

	pxDrive.stiffness = drive.Stiffness;
	pxDrive.damping = drive.Damping;
	pxDrive.forceLimit = drive.ForceLimit;

	GetPxD6Joint().setDrive(ToPxDrive(type), pxDrive);
}

Vector3 PhysXD6Joint::GetDriveLinearVelocity() const
{
	PxVec3 linear;
	PxVec3 angular;

	GetPxD6Joint().getDriveVelocity(linear, angular);
	return FromPxVector(linear);
}

Vector3 PhysXD6Joint::GetDriveAngularVelocity() const
{
	PxVec3 linear;
	PxVec3 angular;

	GetPxD6Joint().getDriveVelocity(linear, angular);
	return FromPxVector(angular);
}
