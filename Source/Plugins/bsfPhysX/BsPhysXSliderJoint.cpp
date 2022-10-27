//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsPhysXSliderJoint.h"
#include "BsFPhysXJoint.h"
#include "BsPhysX.h"
#include "BsPhysXRigidbody.h"
#include "PxRigidDynamic.h"

using namespace physx;

using namespace bs;

PxPrismaticJointFlag::Enum toPxFlag(SliderJointFlag flag)
{
	switch(flag)
	{
	default:
	case SliderJointFlag::Limit:
		return PxPrismaticJointFlag::eLIMIT_ENABLED;
	}
}

PhysXSliderJoint::PhysXSliderJoint(PxPhysics* physx, const SLIDER_JOINT_DESC& desc)
	: SliderJoint(desc)
{
	PxRigidActor* actor0 = nullptr;
	if(desc.Bodies[0].Body != nullptr)
		actor0 = static_cast<PhysXRigidbody*>(desc.Bodies[0].Body)->GetInternalInternal();

	PxRigidActor* actor1 = nullptr;
	if(desc.Bodies[1].Body != nullptr)
		actor1 = static_cast<PhysXRigidbody*>(desc.Bodies[1].Body)->GetInternalInternal();

	PxTransform tfrm0 = toPxTransform(desc.Bodies[0].Position, desc.Bodies[0].Rotation);
	PxTransform tfrm1 = toPxTransform(desc.Bodies[1].Position, desc.Bodies[1].Rotation);

	PxPrismaticJoint* joint = PxPrismaticJointCreate(*physx, actor0, tfrm0, actor1, tfrm1);
	joint->userData = this;

	mInternal = bs_new<FPhysXJoint>(joint, desc);

	PxPrismaticJointFlags flags;

	if(((u32)desc.Flag & (u32)SliderJointFlag::Limit) != 0)
		flags |= PxPrismaticJointFlag::eLIMIT_ENABLED;

	joint->setPrismaticJointFlags(flags);

	// Calls to virtual methods are okay here
	SetLimit(desc.Limit);
}

PhysXSliderJoint::~PhysXSliderJoint()
{
	bs_delete(mInternal);
}

float PhysXSliderJoint::GetPosition() const
{
	return GetInternal()->getPosition();
}

float PhysXSliderJoint::GetSpeed() const
{
	return GetInternal()->getVelocity();
}

LimitLinearRange PhysXSliderJoint::GetLimit() const
{
	PxJointLinearLimitPair pxLimit = GetInternal()->getLimit();

	LimitLinearRange limit;
	limit.Lower = pxLimit.lower;
	limit.Upper = pxLimit.upper;
	limit.ContactDist = pxLimit.contactDistance;
	limit.Restitution = pxLimit.restitution;
	limit.Spring.Stiffness = pxLimit.stiffness;
	limit.Spring.Damping = pxLimit.damping;

	return limit;
}

void PhysXSliderJoint::SetLimit(const LimitLinearRange& limit)
{
	PxJointLinearLimitPair pxLimit(gPhysX().GetScale(), limit.Lower, limit.Upper, limit.ContactDist);
	pxLimit.stiffness = limit.Spring.Stiffness;
	pxLimit.damping = limit.Spring.Damping;
	pxLimit.restitution = limit.Restitution;

	GetInternal()->setLimit(pxLimit);
}

void PhysXSliderJoint::SetFlag(SliderJointFlag flag, bool enabled)
{
	GetInternal()->setPrismaticJointFlag(toPxFlag(flag), enabled);
}

bool PhysXSliderJoint::HasFlag(SliderJointFlag flag) const
{
	return GetInternal()->getPrismaticJointFlags() & toPxFlag(flag);
}

PxPrismaticJoint* PhysXSliderJoint::GetInternal() const
{
	FPhysXJoint* internal = static_cast<FPhysXJoint*>(mInternal);

	return static_cast<PxPrismaticJoint*>(internal->GetInternalInternal());
}
