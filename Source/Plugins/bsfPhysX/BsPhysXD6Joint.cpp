//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsPhysXD6Joint.h"
#include "BsFPhysXJoint.h"
#include "BsPhysX.h"
#include "BsPhysXRigidbody.h"
#include "PxRigidDynamic.h"

using namespace physx;

namespace bs
{
	PxD6Axis::Enum toPxAxis(D6JointAxis axis)
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

	PxD6Motion::Enum toPxMotion(D6JointMotion motion)
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

	PxD6Drive::Enum toPxDrive(D6JointDriveType drive)
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

	D6JointMotion fromPxMotion(PxD6Motion::Enum motion)
	{
		switch (motion)
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


	D6JointDriveType fromPxDrive(PxD6Drive::Enum drive)
	{
		switch (drive)
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
		:D6Joint(desc)
	{
		PxRigidActor* actor0 = nullptr;
		if (desc.bodies[0].body != nullptr)
			actor0 = static_cast<PhysXRigidbody*>(desc.bodies[0].body)->GetInternalInternal();

		PxRigidActor* actor1 = nullptr;
		if (desc.bodies[1].body != nullptr)
			actor1 = static_cast<PhysXRigidbody*>(desc.bodies[1].body)->GetInternalInternal();

		PxTransform tfrm0 = toPxTransform(desc.bodies[0].position, desc.bodies[0].rotation);
		PxTransform tfrm1 = toPxTransform(desc.bodies[1].position, desc.bodies[1].rotation);

		PxD6Joint* joint = PxD6JointCreate(*physx, actor0, tfrm0, actor1, tfrm1);
		joint->userData = this;

		mInternal = bs_new<FPhysXJoint>(joint, desc);

		// Calls to virtual methods are okay here
		for (UINT32 i = 0; i < (UINT32)D6JointAxis::Count; i++)
			setMotion((D6JointAxis)i, desc.motion[i]);

		for (UINT32 i = 0; i < (UINT32)D6JointDriveType::Count; i++)
			setDrive((D6JointDriveType)i, desc.drive[i]);

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

	D6JointMotion PhysXD6Joint::getMotion(D6JointAxis axis) const
	{
		return fromPxMotion(getInternal()->getMotion(toPxAxis(axis)));
	}

	void PhysXD6Joint::setMotion(D6JointAxis axis, D6JointMotion motion)
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

	D6JointDrive PhysXD6Joint::getDrive(D6JointDriveType type) const
	{
		PxD6JointDrive pxDrive = getInternal()->getDrive(toPxDrive(type));

		D6JointDrive drive;
		drive.acceleration = pxDrive.flags & PxD6JointDriveFlag::eACCELERATION;
		drive.stiffness = pxDrive.stiffness;
		drive.damping = pxDrive.damping;
		drive.forceLimit = pxDrive.forceLimit;

		return drive;
	}

	void PhysXD6Joint::setDrive(D6JointDriveType type, const D6JointDrive& drive)
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

		return static_cast<PxD6Joint*>(internal->GetInternalInternal());
	}
}
