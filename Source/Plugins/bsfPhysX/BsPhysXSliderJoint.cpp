//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsPhysXSliderJoint.h"
#include "BsFPhysXJoint.h"
#include "BsPhysX.h"
#include "BsPhysXRigidbody.h"
#include "PxRigidDynamic.h"

using namespace physx;

namespace bs
{
	PxPrismaticJointFlag::Enum ToPxFlag(SliderJointFlag flag)
	{
		switch (flag)
		{
		default:
		case SliderJointFlag::Limit:
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

		if (((UINT32)desc.flag & (UINT32)SliderJointFlag::Limit) != 0)
			flags |= PxPrismaticJointFlag::eLIMIT_ENABLED;

		joint->SetPrismaticJointFlags(flags);

		// Calls to virtual methods are okay here
		setLimit(desc.limit);
	}

	PhysXSliderJoint::~PhysXSliderJoint()
	{
		bs_delete(mInternal);
	}

	float PhysXSliderJoint::GetPosition() const
	{
		return GetInternal()->GetPosition();
	}

	float PhysXSliderJoint::GetSpeed() const
	{
		return GetInternal()->GetVelocity();
	}

	LimitLinearRange PhysXSliderJoint::GetLimit() const
	{
		PxJointLinearLimitPair pxLimit = getInternal()->GetLimit();

		LimitLinearRange limit;
		limit.lower = pxLimit.lower;
		limit.upper = pxLimit.upper;
		limit.contactDist = pxLimit.contactDistance;
		limit.restitution = pxLimit.restitution;
		limit.spring.stiffness = pxLimit.stiffness;
		limit.spring.damping = pxLimit.damping;

		return limit;
	}

	void PhysXSliderJoint::SetLimit(const LimitLinearRange& limit)
	{
		PxJointLinearLimitPair PxLimit(gPhysX().GetScale(), limit.lower, limit.upper, limit.contactDist);
		pxLimit.stiffness = limit.spring.stiffness;
		pxLimit.damping = limit.spring.damping;
		pxLimit.restitution = limit.restitution;

		getInternal()->SetLimit(pxLimit);
	}

	void PhysXSliderJoint::SetFlag(SliderJointFlag flag, bool enabled)
	{
		getInternal()->SetPrismaticJointFlag(toPxFlag(flag), enabled);
	}

	bool PhysXSliderJoint::HasFlag(SliderJointFlag flag) const
	{
		return GetInternal()->GetPrismaticJointFlags() & toPxFlag(flag);
	}

	PxPrismaticJoint* PhysXSliderJoint::getInternal() const
	{
		FPhysXJoint* internal = static_cast<FPhysXJoint*>(mInternal);

		return static_cast<PxPrismaticJoint*>(internal->_getInternal());
	}
}
