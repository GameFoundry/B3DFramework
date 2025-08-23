//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsPhysXSphericalJoint.h"
#include "BsPhysXJoint.h"
#include "BsPhysXRigidbody.h"
#include "PxRigidDynamic.h"

using namespace physx;
using namespace b3d;

PhysXSphericalJoint::PhysXSphericalJoint(PxPhysics* physx, Joint& owner, const SphericalJointCreateInformation& createInformation)
{
	PxRigidActor* actor0 = nullptr;
	if(createInformation.Bodies[0].Body.IsValid())
		actor0 = static_cast<PhysXRigidbody&>(createInformation.Bodies[0].Body->GetImplementation()).GetPxRigidDynamic();

	PxRigidActor* actor1 = nullptr;
	if(createInformation.Bodies[1].Body.IsValid())
		actor1 = static_cast<PhysXRigidbody&>(createInformation.Bodies[1].Body->GetImplementation()).GetPxRigidDynamic();

	PxTransform tfrm0 = ToPxTransform(createInformation.Bodies[0].Position, createInformation.Bodies[0].Rotation);
	PxTransform tfrm1 = ToPxTransform(createInformation.Bodies[1].Position, createInformation.Bodies[1].Rotation);

	PxSphericalJoint* joint = PxSphericalJointCreate(*physx, actor0, tfrm0, actor1, tfrm1);
	mInternal.Initialize(owner, *joint, createInformation);

	PxSphericalJointFlags flags;

	if(((u32)createInformation.Flag & (u32)SphericalJointFlag::Limit) != 0)
		flags |= PxSphericalJointFlag::eLIMIT_ENABLED;

	joint->setSphericalJointFlags(flags);

	// Calls to virtual methods are okay here
	PhysXSphericalJoint::SetLimit(createInformation.Limit);
}

LimitConeRange PhysXSphericalJoint::GetLimit() const
{
	PxJointLimitCone pxLimit = GetPxSphericalJoint().getLimitCone();

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

	GetPxSphericalJoint().setLimitCone(pxLimit);
}
