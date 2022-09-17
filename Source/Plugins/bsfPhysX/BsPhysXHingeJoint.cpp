//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsPhysXHingeJoint.h"
#include "BsFPhysXJoint.h"
#include "BsPhysXRigidbody.h"
#include "PxRigidDynamic.h"

using namespace physx;

namespace bs
{
	PxRevoluteJointFlag::Enum toPxFlag(HingeJointFlag flag)
	{
		switch (flag)
		{
		case HingeJointFlag::Limit:
			return PxRevoluteJointFlag::eLIMIT_ENABLED;
		default:
		case HingeJointFlag::Drive:
			return PxRevoluteJointFlag::eDRIVE_ENABLED;
		}
	}

	PhysXHingeJoint::PhysXHingeJoint(PxPhysics* physx, const HINGE_JOINT_DESC& desc)
		:HingeJoint(desc)
	{
		PxRigidActor* actor0 = nullptr;
		if (desc.bodies[0].body != nullptr)
			actor0 = static_cast<PhysXRigidbody*>(desc.bodies[0].body)->GetInternalInternal();

		PxRigidActor* actor1 = nullptr;
		if (desc.bodies[1].body != nullptr)
			actor1 = static_cast<PhysXRigidbody*>(desc.bodies[1].body)->GetInternalInternal();

		PxTransform tfrm0 = toPxTransform(desc.bodies[0].position, desc.bodies[0].rotation);
		PxTransform tfrm1 = toPxTransform(desc.bodies[1].position, desc.bodies[1].rotation);

		PxRevoluteJoint* joint = PxRevoluteJointCreate(*physx, actor0, tfrm0, actor1, tfrm1);
		joint->userData = this;

		mInternal = bs_new<FPhysXJoint>(joint, desc);

		PxRevoluteJointFlags flags;

		if (((UINT32)desc.flag & (UINT32)HingeJointFlag::Limit) != 0)
			flags |= PxRevoluteJointFlag::eLIMIT_ENABLED;

		if (((UINT32)desc.flag & (UINT32)HingeJointFlag::Drive) != 0)
			flags |= PxRevoluteJointFlag::eDRIVE_ENABLED;

		joint->setRevoluteJointFlags(flags);

		// Must be set after global flags, as it will append to them.
		// Calls to virtual methods are okay here.
		SetLimit(desc.limit);
		SetDrive(desc.drive);
	}

	PhysXHingeJoint::~PhysXHingeJoint()
	{
		bs_delete(mInternal);
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
		limit.lower = pxLimit.lower;
		limit.upper = pxLimit.upper;
		limit.contactDist = pxLimit.contactDistance;
		limit.restitution = pxLimit.restitution;
		limit.spring.stiffness = pxLimit.stiffness;
		limit.spring.damping = pxLimit.damping;

		return limit;
	}

	void PhysXHingeJoint::SetLimit(const LimitAngularRange& limit)
	{
		PxJointAngularLimitPair pxLimit(limit.lower.ValueRadians(), limit.upper.ValueRadians(), limit.contactDist);
		pxLimit.stiffness = limit.spring.stiffness;
		pxLimit.damping = limit.spring.damping;
		pxLimit.restitution = limit.restitution;

		GetInternal()->setLimit(pxLimit);
	}

	HingeJointDrive PhysXHingeJoint::GetDrive() const
	{
		HingeJointDrive drive;
		drive.speed = GetInternal()->getDriveVelocity();
		drive.forceLimit = GetInternal()->getDriveForceLimit();
		drive.gearRatio = GetInternal()->getDriveGearRatio();
		drive.freeSpin = GetInternal()->getRevoluteJointFlags() & PxRevoluteJointFlag::eDRIVE_FREESPIN;

		return drive;
	}

	void PhysXHingeJoint::SetDrive(const HingeJointDrive& drive)
	{
		GetInternal()->setDriveVelocity(drive.speed);
		GetInternal()->setDriveForceLimit(drive.forceLimit);
		GetInternal()->setDriveGearRatio(drive.gearRatio);
		GetInternal()->setRevoluteJointFlag(PxRevoluteJointFlag::eDRIVE_FREESPIN, drive.freeSpin);
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
}
