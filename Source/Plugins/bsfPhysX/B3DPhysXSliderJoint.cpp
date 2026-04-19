//************************************ B3D Framework - Copyright 2026 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DPhysXSliderJoint.h"
#include "B3DPhysXJoint.h"
#include "B3DPhysX.h"
#include "B3DPhysXRigidbody.h"
#include "PxRigidDynamic.h"

using namespace physx;
using namespace b3d;

PhysXSliderJoint::PhysXSliderJoint(PxPhysics* physx, Joint& owner, const SliderJointCreateInformation& createInformation)
{
	PxRigidActor* actor0 = nullptr;
	if(createInformation.Bodies[0].Body.IsValid())
		actor0 = static_cast<PhysXRigidbody&>(createInformation.Bodies[0].Body->GetImplementation()).GetPxRigidDynamic();

	PxRigidActor* actor1 = nullptr;
	if(createInformation.Bodies[1].Body.IsValid())
		actor1 = static_cast<PhysXRigidbody&>(createInformation.Bodies[1].Body->GetImplementation()).GetPxRigidDynamic();

	PxTransform tfrm0 = ToPxTransform(createInformation.Bodies[0].Position, createInformation.Bodies[0].Rotation);
	PxTransform tfrm1 = ToPxTransform(createInformation.Bodies[1].Position, createInformation.Bodies[1].Rotation);

	PxPrismaticJoint* joint = PxPrismaticJointCreate(*physx, actor0, tfrm0, actor1, tfrm1);
	mInternal.Initialize(owner, *joint, createInformation);

	PxPrismaticJointFlags flags;

	if(createInformation.Flags.IsSet(SliderJointFlag::Limit))
		flags |= PxPrismaticJointFlag::eLIMIT_ENABLED;

	joint->setPrismaticJointFlags(flags);

	// Calls to virtual methods are okay here
	PhysXSliderJoint::SetLimit(createInformation.Limit);
}

LimitLinearRange PhysXSliderJoint::GetLimit() const
{
	PxJointLinearLimitPair pxLimit = GetPxPrismaticJoint().getLimit();

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
	PxJointLinearLimitPair pxLimit(GetPhysX().GetScale(), limit.Lower, limit.Upper, limit.ContactDist);
	pxLimit.stiffness = limit.Spring.Stiffness;
	pxLimit.damping = limit.Spring.Damping;
	pxLimit.restitution = limit.Restitution;

	GetPxPrismaticJoint().setLimit(pxLimit);
}
