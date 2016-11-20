//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsPhysXSliderJoint.h"
#include "BsFPhysxJoint.h"
#include "BsPhysX.h"
#include "BsPhysXRigidbody.h"
#include "PxRigidDynamic.h"

using namespace physx;

namespace bs
{
	PxPrismaticJointFlag::Enum toPxFlag(PhysXSliderJoint::Flag flag)
	{
		switch (flag)
		{
		default:
		case PhysXSliderJoint::Flag::Limit:
			return PxPrismaticJointFlag::eLIMIT_ENABLED;
		}
	}

	PhysXSliderJoint::PhysXSliderJoint(PxPhysics* physx, const SLIDER_JOINT_DESC& desc)
		:SliderJoint(desc)
	{
		PxRigidActor* actor0 = nullptr;
		if (desc.bodies[0].body != nullptr)
			actor0 = static_cast<PhysXRigidbody*>(desc.bodies[0].body)->_getInternal();

		PxRigidActor* actor1 = nullptr;
		if (desc.bodies[1].body != nullptr)
			actor1 = static_cast<PhysXRigidbody*>(desc.bodies[1].body)->_getInternal();

		PxTransform tfrm0 = toPxTransform(desc.bodies[0].position, desc.bodies[0].rotation);
		PxTransform tfrm1 = toPxTransform(desc.bodies[1].position, desc.bodies[1].rotation);

		PxPrismaticJoint* joint = PxPrismaticJointCreate(*physx, actor0, tfrm0, actor1, tfrm1);
		joint->userData = this;

		mInternal = bs_new<FPhysXJoint>(joint, desc);

		PxPrismaticJointFlags flags;

		if (((UINT32)desc.flag & (UINT32)Flag::Limit) != 0)
			flags |= PxPrismaticJointFlag::eLIMIT_ENABLED;

		joint->setPrismaticJointFlags(flags);

		// Calls to virtual methods are okay here
		setLimit(desc.limit);
	}

	PhysXSliderJoint::~PhysXSliderJoint()
	{
		bs_delete(mInternal);
	}

	float PhysXSliderJoint::getPosition() const
	{
		return getInternal()->getPosition();
	}

	float PhysXSliderJoint::getSpeed() const
	{
		return getInternal()->getVelocity();
	}

	LimitLinearRange PhysXSliderJoint::getLimit() const
	{
		PxJointLinearLimitPair pxLimit = getInternal()->getLimit();

		LimitLinearRange limit;
		limit.lower = pxLimit.lower;
		limit.upper = pxLimit.upper;
		limit.contactDist = pxLimit.contactDistance;
		limit.restitution = pxLimit.restitution;
		limit.spring.stiffness = pxLimit.stiffness;
		limit.spring.damping = pxLimit.damping;

		return limit;
	}

	void PhysXSliderJoint::setLimit(const LimitLinearRange& limit)
	{
		PxJointLinearLimitPair pxLimit(gPhysX().getScale(), limit.lower, limit.upper, limit.contactDist);
		pxLimit.stiffness = limit.spring.stiffness;
		pxLimit.damping = limit.spring.damping;
		pxLimit.restitution = limit.restitution;

		getInternal()->setLimit(pxLimit);
	}

	void PhysXSliderJoint::setFlag(Flag flag, bool enabled)
	{
		getInternal()->setPrismaticJointFlag(toPxFlag(flag), enabled);
	}

	bool PhysXSliderJoint::hasFlag(Flag flag) const
	{
		return getInternal()->getPrismaticJointFlags() & toPxFlag(flag);
	}

	PxPrismaticJoint* PhysXSliderJoint::getInternal() const
	{
		FPhysXJoint* internal = static_cast<FPhysXJoint*>(mInternal);

		return static_cast<PxPrismaticJoint*>(internal->_getInternal());
	}
}