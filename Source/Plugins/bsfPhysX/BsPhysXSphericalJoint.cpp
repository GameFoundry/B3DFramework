//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsPhysXSphericalJoint.h"
#include "BsFPhysXJoint.h"
#include "BsPhysXRigidbody.h"
#include "PxRigidDynamic.h"

using namespace physx;

using namespace bs;

PxSphericalJointFlag::Enum ToPxFlag(SphericalJointFlag flag)
{
	switch(flag)
	{
	default:
	case SphericalJointFlag::Limit:
		return PxSphericalJointFlag::eLIMIT_ENABLED;
	}
}

PhysXSphericalJoint::PhysXSphericalJoint(PxPhysics* physx, const SPHERICAL_JOINT_DESC& desc)
	: SphericalJoint(desc)
{
	PxRigidActor* actor0 = nullptr;
	if(desc.Bodies[0].Body != nullptr)
		actor0 = static_cast<PhysXRigidbody*>(desc.Bodies[0].Body)->GetInternalInternal();

	PxRigidActor* actor1 = nullptr;
	if(desc.Bodies[1].Body != nullptr)
		actor1 = static_cast<PhysXRigidbody*>(desc.Bodies[1].Body)->GetInternalInternal();

	PxTransform tfrm0 = ToPxTransform(desc.Bodies[0].Position, desc.Bodies[0].Rotation);
	PxTransform tfrm1 = ToPxTransform(desc.Bodies[1].Position, desc.Bodies[1].Rotation);

	PxSphericalJoint* joint = PxSphericalJointCreate(*physx, actor0, tfrm0, actor1, tfrm1);
	joint->userData = this;

	mInternal = B3DNew<FPhysXJoint>(joint, desc);

	PxSphericalJointFlags flags;

	if(((u32)desc.Flag & (u32)SphericalJointFlag::Limit) != 0)
		flags |= PxSphericalJointFlag::eLIMIT_ENABLED;

	joint->setSphericalJointFlags(flags);

	// Calls to virtual methods are okay here
	SetLimit(desc.Limit);
}

PhysXSphericalJoint::~PhysXSphericalJoint()
{
	B3DDelete(mInternal);
}

LimitConeRange PhysXSphericalJoint::GetLimit() const
{
	PxJointLimitCone pxLimit = GetInternal()->getLimitCone();

	LimitConeRange limit;
	limit.YLimitAngle = pxLimit.yAngle;
	limit.ZLimitAngle = pxLimit.zAngle;
	limit.ContactDist = pxLimit.contactDistance;
	limit.Restitution = pxLimit.restitution;
	limit.Spring.Stiffness = pxLimit.stiffness;
	limit.Spring.Damping = pxLimit.damping;

	return limit;
}

void PhysXSphericalJoint::SetLimit(const LimitConeRange& limit)
{
	PxJointLimitCone pxLimit(limit.YLimitAngle.GetValueInRadians(), limit.ZLimitAngle.GetValueInRadians(), limit.ContactDist);
	pxLimit.stiffness = limit.Spring.Stiffness;
	pxLimit.damping = limit.Spring.Damping;
	pxLimit.restitution = limit.Restitution;

	GetInternal()->setLimitCone(pxLimit);
}

void PhysXSphericalJoint::SetFlag(SphericalJointFlag flag, bool enabled)
{
	GetInternal()->setSphericalJointFlag(ToPxFlag(flag), enabled);
}

bool PhysXSphericalJoint::HasFlag(SphericalJointFlag flag) const
{
	return GetInternal()->getSphericalJointFlags() & ToPxFlag(flag);
}

PxSphericalJoint* PhysXSphericalJoint::GetInternal() const
{
	FPhysXJoint* internal = static_cast<FPhysXJoint*>(mInternal);

	return static_cast<PxSphericalJoint*>(internal->GetInternalInternal());
}
