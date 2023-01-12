//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsPhysXD6Joint.h"
#include "BsFPhysXJoint.h"
#include "BsPhysX.h"
#include "BsPhysXRigidbody.h"
#include "PxRigidDynamic.h"

using namespace physx;

using namespace bs;

PxD6Axis::Enum ToPxAxis(D6JointAxis axis)
{
	switch(axis)
	{
	default:
	case D6JointAxis::X:
		return PxD6Axis::eX;
	case D6JointAxis::Y:
		return PxD6Axis::eY;
	case D6JointAxis::Z:
		return PxD6Axis::eZ;
	case D6JointAxis::Twist:
		return PxD6Axis::eTWIST;
	case D6JointAxis::SwingY:
		return PxD6Axis::eSWING1;
	case D6JointAxis::SwingZ:
		return PxD6Axis::eSWING2;
	}
}

PxD6Motion::Enum ToPxMotion(D6JointMotion motion)
{
	switch(motion)
	{
	default:
	case D6JointMotion::Free:
		return PxD6Motion::eFREE;
	case D6JointMotion::Limited:
		return PxD6Motion::eLIMITED;
	case D6JointMotion::Locked:
		return PxD6Motion::eLOCKED;
	}
}

PxD6Drive::Enum ToPxDrive(D6JointDriveType drive)
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
	case D6JointDriveType::Swing:
		return PxD6Drive::eSWING;
	case D6JointDriveType::Twist:
		return PxD6Drive::eTWIST;
	case D6JointDriveType::SLERP:
		return PxD6Drive::eSLERP;
	}
}

D6JointMotion FromPxMotion(PxD6Motion::Enum motion)
{
	switch(motion)
	{
	default:
	case PxD6Motion::eFREE:
		return D6JointMotion::Free;
	case PxD6Motion::eLIMITED:
		return D6JointMotion::Limited;
	case PxD6Motion::eLOCKED:
		return D6JointMotion::Locked;
	}
}

D6JointDriveType FromPxDrive(PxD6Drive::Enum drive)
{
	switch(drive)
	{
	default:
	case PxD6Drive::eX:
		return D6JointDriveType::X;
	case PxD6Drive::eY:
		return D6JointDriveType::Y;
	case PxD6Drive::eZ:
		return D6JointDriveType::Z;
	case PxD6Drive::eSWING:
		return D6JointDriveType::Swing;
	case PxD6Drive::eTWIST:
		return D6JointDriveType::Twist;
	case PxD6Drive::eSLERP:
		return D6JointDriveType::SLERP;
	}
}

PhysXD6Joint::PhysXD6Joint(PxPhysics* physx, const D6_JOINT_DESC& desc)
	: D6Joint(desc)
{
	PxRigidActor* actor0 = nullptr;
	if(desc.Bodies[0].Body != nullptr)
		actor0 = static_cast<PhysXRigidbody*>(desc.Bodies[0].Body)->GetInternalInternal();

	PxRigidActor* actor1 = nullptr;
	if(desc.Bodies[1].Body != nullptr)
		actor1 = static_cast<PhysXRigidbody*>(desc.Bodies[1].Body)->GetInternalInternal();

	PxTransform tfrm0 = ToPxTransform(desc.Bodies[0].Position, desc.Bodies[0].Rotation);
	PxTransform tfrm1 = ToPxTransform(desc.Bodies[1].Position, desc.Bodies[1].Rotation);

	PxD6Joint* joint = PxD6JointCreate(*physx, actor0, tfrm0, actor1, tfrm1);
	joint->userData = this;

	mInternal = B3DNew<FPhysXJoint>(joint, desc);

	// Calls to virtual methods are okay here
	for(u32 i = 0; i < (u32)D6JointAxis::Count; i++)
		SetMotion((D6JointAxis)i, desc.Motion[i]);

	for(u32 i = 0; i < (u32)D6JointDriveType::Count; i++)
		SetDrive((D6JointDriveType)i, desc.Drive[i]);

	SetLimitLinear(desc.LimitLinear);
	SetLimitTwist(desc.LimitTwist);
	SetLimitSwing(desc.LimitSwing);

	SetDriveTransform(desc.DrivePosition, desc.DriveRotation);
	SetDriveVelocity(desc.DriveLinearVelocity, desc.DriveAngularVelocity);
}

PhysXD6Joint::~PhysXD6Joint()
{
	B3DDelete(mInternal);
}

D6JointMotion PhysXD6Joint::GetMotion(D6JointAxis axis) const
{
	return FromPxMotion(GetInternal()->getMotion(ToPxAxis(axis)));
}

void PhysXD6Joint::SetMotion(D6JointAxis axis, D6JointMotion motion)
{
	GetInternal()->setMotion(ToPxAxis(axis), ToPxMotion(motion));
}

Radian PhysXD6Joint::GetTwist() const
{
	return Radian(GetInternal()->getTwist());
}

Radian PhysXD6Joint::GetSwingY() const
{
	return Radian(GetInternal()->getSwingYAngle());
}

Radian PhysXD6Joint::GetSwingZ() const
{
	return Radian(GetInternal()->getSwingZAngle());
}

LimitLinear PhysXD6Joint::GetLimitLinear() const
{
	PxJointLinearLimit pxLimit = GetInternal()->getLinearLimit();

	LimitLinear limit;
	limit.Extent = pxLimit.value;
	limit.ContactDist = pxLimit.contactDistance;
	limit.Restitution = pxLimit.restitution;
	limit.Spring.Stiffness = pxLimit.stiffness;
	limit.Spring.Damping = pxLimit.damping;

	return limit;
}

void PhysXD6Joint::SetLimitLinear(const LimitLinear& limit)
{
	PxJointLinearLimit pxLimit(GetPhysX().GetScale(), limit.Extent, limit.ContactDist);
	pxLimit.stiffness = limit.Spring.Stiffness;
	pxLimit.damping = limit.Spring.Damping;
	pxLimit.restitution = limit.Restitution;

	GetInternal()->setLinearLimit(pxLimit);
}

LimitAngularRange PhysXD6Joint::GetLimitTwist() const
{
	PxJointAngularLimitPair pxLimit = GetInternal()->getTwistLimit();

	LimitAngularRange limit;
	limit.Lower = pxLimit.lower;
	limit.Upper = pxLimit.upper;
	limit.ContactDist = pxLimit.contactDistance;
	limit.Restitution = pxLimit.restitution;
	limit.Spring.Stiffness = pxLimit.stiffness;
	limit.Spring.Damping = pxLimit.damping;

	return limit;
}

void PhysXD6Joint::SetLimitTwist(const LimitAngularRange& limit)
{
	PxJointAngularLimitPair pxLimit(limit.Lower.GetValueInRadians(), limit.Upper.GetValueInRadians(), limit.ContactDist);
	pxLimit.stiffness = limit.Spring.Stiffness;
	pxLimit.damping = limit.Spring.Damping;
	pxLimit.restitution = limit.Restitution;

	GetInternal()->setTwistLimit(pxLimit);
}

LimitConeRange PhysXD6Joint::GetLimitSwing() const
{
	PxJointLimitCone pxLimit = GetInternal()->getSwingLimit();

	LimitConeRange limit;
	limit.YLimitAngle = pxLimit.yAngle;
	limit.ZLimitAngle = pxLimit.zAngle;
	limit.ContactDist = pxLimit.contactDistance;
	limit.Restitution = pxLimit.restitution;
	limit.Spring.Stiffness = pxLimit.stiffness;
	limit.Spring.Damping = pxLimit.damping;

	return limit;
}

void PhysXD6Joint::SetLimitSwing(const LimitConeRange& limit)
{
	PxJointLimitCone pxLimit(limit.YLimitAngle.GetValueInRadians(), limit.ZLimitAngle.GetValueInRadians(), limit.ContactDist);
	pxLimit.stiffness = limit.Spring.Stiffness;
	pxLimit.damping = limit.Spring.Damping;
	pxLimit.restitution = limit.Restitution;

	GetInternal()->setSwingLimit(pxLimit);
}

D6JointDrive PhysXD6Joint::GetDrive(D6JointDriveType type) const
{
	PxD6JointDrive pxDrive = GetInternal()->getDrive(ToPxDrive(type));

	D6JointDrive drive;
	drive.Acceleration = pxDrive.flags & PxD6JointDriveFlag::eACCELERATION;
	drive.Stiffness = pxDrive.stiffness;
	drive.Damping = pxDrive.damping;
	drive.ForceLimit = pxDrive.forceLimit;

	return drive;
}

void PhysXD6Joint::SetDrive(D6JointDriveType type, const D6JointDrive& drive)
{
	PxD6JointDrive pxDrive;

	if(drive.Acceleration)
		pxDrive.flags = PxD6JointDriveFlag::eACCELERATION;

	pxDrive.stiffness = drive.Stiffness;
	pxDrive.damping = drive.Damping;
	pxDrive.forceLimit = drive.ForceLimit;

	GetInternal()->setDrive(ToPxDrive(type), pxDrive);
}

Vector3 PhysXD6Joint::GetDrivePosition() const
{
	return FromPxVector(GetInternal()->getDrivePosition().p);
}

Quaternion PhysXD6Joint::GetDriveRotation() const
{
	return FromPxQuaternion(GetInternal()->getDrivePosition().q);
}

void PhysXD6Joint::SetDriveTransform(const Vector3& position, const Quaternion& rotation)
{
	GetInternal()->setDrivePosition(ToPxTransform(position, rotation));
}

Vector3 PhysXD6Joint::GetDriveLinearVelocity() const
{
	PxVec3 linear;
	PxVec3 angular;

	GetInternal()->getDriveVelocity(linear, angular);
	return FromPxVector(linear);
}

Vector3 PhysXD6Joint::GetDriveAngularVelocity() const
{
	PxVec3 linear;
	PxVec3 angular;

	GetInternal()->getDriveVelocity(linear, angular);
	return FromPxVector(angular);
}

void PhysXD6Joint::SetDriveVelocity(const Vector3& linear, const Vector3& angular)
{
	GetInternal()->setDriveVelocity(ToPxVector(linear), ToPxVector(angular));
}

PxD6Joint* PhysXD6Joint::GetInternal() const
{
	FPhysXJoint* internal = static_cast<FPhysXJoint*>(mInternal);

	return static_cast<PxD6Joint*>(internal->GetInternalInternal());
}
