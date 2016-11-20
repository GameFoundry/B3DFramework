//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsPhysXD6Joint.h"
#include "BsFPhysxJoint.h"
#include "BsPhysX.h"
#include "BsPhysXRigidbody.h"
#include "PxRigidDynamic.h"

using namespace physx;

namespace bs
{
	PxD6Axis::Enum toPxAxis(PhysXD6Joint::Axis axis)
	{
		switch(axis)
		{
		default:
		case PhysXD6Joint::Axis::X:
			return PxD6Axis::eX;
		case PhysXD6Joint::Axis::Y:
			return PxD6Axis::eY;
		case PhysXD6Joint::Axis::Z:
			return PxD6Axis::eZ;
		case PhysXD6Joint::Axis::Twist:
			return PxD6Axis::eTWIST;
		case PhysXD6Joint::Axis::SwingY:
			return PxD6Axis::eSWING1;
		case PhysXD6Joint::Axis::SwingZ:
			return PxD6Axis::eSWING2;
		}
	}

	PxD6Motion::Enum toPxMotion(PhysXD6Joint::Motion motion)
	{
		switch(motion)
		{
		default:
		case PhysXD6Joint::Motion::Free:
			return PxD6Motion::eFREE;
		case PhysXD6Joint::Motion::Limited:
			return PxD6Motion::eLIMITED;
		case PhysXD6Joint::Motion::Locked:
			return PxD6Motion::eLOCKED;
		}
	}

	PxD6Drive::Enum toPxDrive(PhysXD6Joint::DriveType drive)
	{
		switch(drive)
		{
		default:
		case PhysXD6Joint::DriveType::X:
			return PxD6Drive::eX;
		case PhysXD6Joint::DriveType::Y:
			return PxD6Drive::eY;
		case PhysXD6Joint::DriveType::Z:
			return PxD6Drive::eZ;
		case PhysXD6Joint::DriveType::Swing:
			return PxD6Drive::eSWING;
		case PhysXD6Joint::DriveType::Twist:
			return PxD6Drive::eTWIST;
		case PhysXD6Joint::DriveType::SLERP:
			return PxD6Drive::eSLERP;
		}
	}

	PhysXD6Joint::Motion fromPxMotion(PxD6Motion::Enum motion)
	{
		switch (motion)
		{
		default:
		case PxD6Motion::eFREE:
			return PhysXD6Joint::Motion::Free;
		case PxD6Motion::eLIMITED:
			return PhysXD6Joint::Motion::Limited;
		case PxD6Motion::eLOCKED:
			return PhysXD6Joint::Motion::Locked;
		}
	}


	PhysXD6Joint::DriveType fromPxDrive(PxD6Drive::Enum drive)
	{
		switch (drive)
		{
		default:
		case PxD6Drive::eX:
			return PhysXD6Joint::DriveType::X;
		case PxD6Drive::eY:
			return PhysXD6Joint::DriveType::Y;
		case PxD6Drive::eZ:
			return PhysXD6Joint::DriveType::Z;
		case PxD6Drive::eSWING:
			return PhysXD6Joint::DriveType::Swing;
		case PxD6Drive::eTWIST:
			return PhysXD6Joint::DriveType::Twist;
		case PxD6Drive::eSLERP:
			return PhysXD6Joint::DriveType::SLERP;
		}
	}

	PhysXD6Joint::PhysXD6Joint(PxPhysics* physx, const D6_JOINT_DESC& desc)
		:D6Joint(desc)
	{
		PxRigidActor* actor0 = nullptr;
		if (desc.bodies[0].body != nullptr)
			actor0 = static_cast<PhysXRigidbody*>(desc.bodies[0].body)->_getInternal();

		PxRigidActor* actor1 = nullptr;
		if (desc.bodies[1].body != nullptr)
			actor1 = static_cast<PhysXRigidbody*>(desc.bodies[1].body)->_getInternal();

		PxTransform tfrm0 = toPxTransform(desc.bodies[0].position, desc.bodies[0].rotation);
		PxTransform tfrm1 = toPxTransform(desc.bodies[1].position, desc.bodies[1].rotation);

		PxD6Joint* joint = PxD6JointCreate(*physx, actor0, tfrm0, actor1, tfrm1);
		joint->userData = this;

		mInternal = bs_new<FPhysXJoint>(joint, desc);

		// Calls to virtual methods are okay here
		for (UINT32 i = 0; i < (UINT32)Axis::Count; i++)
			setMotion((Axis)i, desc.motion[i]);

		for (UINT32 i = 0; i < (UINT32)DriveType::Count; i++)
			setDrive((DriveType)i, desc.drive[i]);

		setLimitLinear(desc.limitLinear);
		setLimitTwist(desc.limitTwist);
		setLimitSwing(desc.limitSwing);

		setDriveTransform(desc.drivePosition, desc.driveRotation);
		setDriveVelocity(desc.driveLinearVelocity, desc.driveAngularVelocity);
	}

	PhysXD6Joint::~PhysXD6Joint()
	{
		bs_delete(mInternal);
	}

	PhysXD6Joint::Motion PhysXD6Joint::getMotion(Axis axis) const
	{
		return fromPxMotion(getInternal()->getMotion(toPxAxis(axis)));
	}

	void PhysXD6Joint::setMotion(Axis axis, Motion motion)
	{
		getInternal()->setMotion(toPxAxis(axis), toPxMotion(motion));
	}

	Radian PhysXD6Joint::getTwist() const
	{
		return Radian(getInternal()->getTwist());
	}

	Radian PhysXD6Joint::getSwingY() const
	{
		return Radian(getInternal()->getSwingYAngle());
	}

	Radian PhysXD6Joint::getSwingZ() const
	{
		return Radian(getInternal()->getSwingZAngle());
	}

	LimitLinear PhysXD6Joint::getLimitLinear() const
	{
		PxJointLinearLimit pxLimit = getInternal()->getLinearLimit();

		LimitLinear limit;
		limit.extent = pxLimit.value;
		limit.contactDist = pxLimit.contactDistance;
		limit.restitution = pxLimit.restitution;
		limit.spring.stiffness = pxLimit.stiffness;
		limit.spring.damping = pxLimit.damping;

		return limit;
	}

	void PhysXD6Joint::setLimitLinear(const LimitLinear& limit)
	{
		PxJointLinearLimit pxLimit(gPhysX().getScale(), limit.extent, limit.contactDist);
		pxLimit.stiffness = limit.spring.stiffness;
		pxLimit.damping = limit.spring.damping;
		pxLimit.restitution = limit.restitution;

		getInternal()->setLinearLimit(pxLimit);
	}

	LimitAngularRange PhysXD6Joint::getLimitTwist() const
	{
		PxJointAngularLimitPair pxLimit = getInternal()->getTwistLimit();

		LimitAngularRange limit;
		limit.lower = pxLimit.lower;
		limit.upper = pxLimit.upper;
		limit.contactDist = pxLimit.contactDistance;
		limit.restitution = pxLimit.restitution;
		limit.spring.stiffness = pxLimit.stiffness;
		limit.spring.damping = pxLimit.damping;

		return limit;
	}

	void PhysXD6Joint::setLimitTwist(const LimitAngularRange& limit)
	{
		PxJointAngularLimitPair pxLimit(limit.lower.valueRadians(), limit.upper.valueRadians(), limit.contactDist);
		pxLimit.stiffness = limit.spring.stiffness;
		pxLimit.damping = limit.spring.damping;
		pxLimit.restitution = limit.restitution;

		getInternal()->setTwistLimit(pxLimit);
	}

	LimitConeRange PhysXD6Joint::getLimitSwing() const
	{
		PxJointLimitCone pxLimit = getInternal()->getSwingLimit();

		LimitConeRange limit;
		limit.yLimitAngle = pxLimit.yAngle;
		limit.zLimitAngle = pxLimit.zAngle;
		limit.contactDist = pxLimit.contactDistance;
		limit.restitution = pxLimit.restitution;
		limit.spring.stiffness = pxLimit.stiffness;
		limit.spring.damping = pxLimit.damping;

		return limit;
	}

	void PhysXD6Joint::setLimitSwing(const LimitConeRange& limit)
	{
		PxJointLimitCone pxLimit(limit.yLimitAngle.valueRadians(), limit.zLimitAngle.valueRadians(), limit.contactDist);
		pxLimit.stiffness = limit.spring.stiffness;
		pxLimit.damping = limit.spring.damping;
		pxLimit.restitution = limit.restitution;

		getInternal()->setSwingLimit(pxLimit);
	}

	PhysXD6Joint::Drive PhysXD6Joint::getDrive(DriveType type) const
	{
		PxD6JointDrive pxDrive = getInternal()->getDrive(toPxDrive(type));

		Drive drive;
		drive.acceleration = pxDrive.flags & PxD6JointDriveFlag::eACCELERATION;
		drive.stiffness = pxDrive.stiffness;
		drive.damping = pxDrive.damping;
		drive.forceLimit = pxDrive.forceLimit;

		return drive;
	}

	void PhysXD6Joint::setDrive(DriveType type, const Drive& drive)
	{
		PxD6JointDrive pxDrive;

		if(drive.acceleration)
			pxDrive.flags = PxD6JointDriveFlag::eACCELERATION;

		pxDrive.stiffness = drive.stiffness;
		pxDrive.damping = drive.damping;
		pxDrive.forceLimit = drive.forceLimit;

		getInternal()->setDrive(toPxDrive(type), pxDrive);
	}

	Vector3 PhysXD6Joint::getDrivePosition() const
	{
		return fromPxVector(getInternal()->getDrivePosition().p);
	}

	Quaternion PhysXD6Joint::getDriveRotation() const
	{
		return fromPxQuaternion(getInternal()->getDrivePosition().q);
	}

	void PhysXD6Joint::setDriveTransform(const Vector3& position, const Quaternion& rotation)
	{
		getInternal()->setDrivePosition(toPxTransform(position, rotation));
	}

	Vector3 PhysXD6Joint::getDriveLinearVelocity() const
	{
		PxVec3 linear;
		PxVec3 angular;

		getInternal()->getDriveVelocity(linear, angular);
		return fromPxVector(linear);
	}

	Vector3 PhysXD6Joint::getDriveAngularVelocity() const
	{
		PxVec3 linear;
		PxVec3 angular;

		getInternal()->getDriveVelocity(linear, angular);
		return fromPxVector(angular);
	}

	void PhysXD6Joint::setDriveVelocity(const Vector3& linear, const Vector3& angular)
	{
		getInternal()->setDriveVelocity(toPxVector(linear), toPxVector(angular));
	}

	PxD6Joint* PhysXD6Joint::getInternal() const
	{
		FPhysXJoint* internal = static_cast<FPhysXJoint*>(mInternal);

		return static_cast<PxD6Joint*>(internal->_getInternal());
	}
}