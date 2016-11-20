//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsPhysXHingeJoint.h"
#include "BsFPhysxJoint.h"
#include "BsPhysXRigidbody.h"
#include "PxRigidDynamic.h"

using namespace physx;

namespace bs
{
	PxRevoluteJointFlag::Enum toPxFlag(PhysXHingeJoint::Flag flag)
	{
		switch (flag)
		{
		case PhysXHingeJoint::Flag::Limit:
			return PxRevoluteJointFlag::eLIMIT_ENABLED;
		default:
		case PhysXHingeJoint::Flag::Drive:
			return PxRevoluteJointFlag::eDRIVE_ENABLED;
		}
	}

	PhysXHingeJoint::PhysXHingeJoint(PxPhysics* physx, const HINGE_JOINT_DESC& desc)
		:HingeJoint(desc)
	{
		PxRigidActor* actor0 = nullptr;
		if (desc.bodies[0].body != nullptr)
			actor0 = static_cast<PhysXRigidbody*>(desc.bodies[0].body)->_getInternal();

		PxRigidActor* actor1 = nullptr;
		if (desc.bodies[1].body != nullptr)
			actor1 = static_cast<PhysXRigidbody*>(desc.bodies[1].body)->_getInternal();

		PxTransform tfrm0 = toPxTransform(desc.bodies[0].position, desc.bodies[0].rotation);
		PxTransform tfrm1 = toPxTransform(desc.bodies[1].position, desc.bodies[1].rotation);

		PxRevoluteJoint* joint = PxRevoluteJointCreate(*physx, actor0, tfrm0, actor1, tfrm1);
		joint->userData = this;

		mInternal = bs_new<FPhysXJoint>(joint, desc);

		PxRevoluteJointFlags flags;

		if (((UINT32)desc.flag & (UINT32)Flag::Limit) != 0)
			flags |= PxRevoluteJointFlag::eLIMIT_ENABLED;

		if (((UINT32)desc.flag & (UINT32)Flag::Drive) != 0)
			flags |= PxRevoluteJointFlag::eDRIVE_ENABLED;

		joint->setRevoluteJointFlags(flags);

		// Must be set after global flags, as it will append to them.
		// Calls to virtual methods are okay here.
		setLimit(desc.limit);
		setDrive(desc.drive);
	}

	PhysXHingeJoint::~PhysXHingeJoint()
	{
		bs_delete(mInternal);
	}

	Radian PhysXHingeJoint::getAngle() const
	{
		return Radian(getInternal()->getAngle());
	}

	float PhysXHingeJoint::getSpeed() const
	{
		return getInternal()->getVelocity();
	}

	LimitAngularRange PhysXHingeJoint::getLimit() const
	{
		PxJointAngularLimitPair pxLimit = getInternal()->getLimit();

		LimitAngularRange limit;
		limit.lower = pxLimit.lower;
		limit.upper = pxLimit.upper;
		limit.contactDist = pxLimit.contactDistance;
		limit.restitution = pxLimit.restitution;
		limit.spring.stiffness = pxLimit.stiffness;
		limit.spring.damping = pxLimit.damping;

		return limit;
	}

	void PhysXHingeJoint::setLimit(const LimitAngularRange& limit)
	{
		PxJointAngularLimitPair pxLimit(limit.lower.valueRadians(), limit.upper.valueRadians(), limit.contactDist);
		pxLimit.stiffness = limit.spring.stiffness;
		pxLimit.damping = limit.spring.damping;
		pxLimit.restitution = limit.restitution;

		getInternal()->setLimit(pxLimit);
	}

	PhysXHingeJoint::Drive PhysXHingeJoint::getDrive() const
	{
		Drive drive;
		drive.speed = getInternal()->getDriveVelocity();
		drive.forceLimit = getInternal()->getDriveForceLimit();
		drive.gearRatio = getInternal()->getDriveGearRatio();
		drive.freeSpin = getInternal()->getRevoluteJointFlags() & PxRevoluteJointFlag::eDRIVE_FREESPIN;

		return drive;
	}

	void PhysXHingeJoint::setDrive(const Drive& drive)
	{
		getInternal()->setDriveVelocity(drive.speed);
		getInternal()->setDriveForceLimit(drive.forceLimit);
		getInternal()->setDriveGearRatio(drive.gearRatio);
		getInternal()->setRevoluteJointFlag(PxRevoluteJointFlag::eDRIVE_FREESPIN, drive.freeSpin);
	}

	void PhysXHingeJoint::setFlag(Flag flag, bool enabled)
	{
		getInternal()->setRevoluteJointFlag(toPxFlag(flag), enabled);
	}

	bool PhysXHingeJoint::hasFlag(Flag flag) const
	{
		return getInternal()->getRevoluteJointFlags() & toPxFlag(flag);
	}

	PxRevoluteJoint* PhysXHingeJoint::getInternal() const
	{
		FPhysXJoint* internal = static_cast<FPhysXJoint*>(mInternal);

		return static_cast<PxRevoluteJoint*>(internal->_getInternal());
	}
}