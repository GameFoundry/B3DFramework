//************************************ B3D Framework - Copyright 2026 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DPhysXDistanceJoint.h"
#include "B3DPhysXJoint.h"
#include "B3DPhysXRigidbody.h"
#include "PxRigidDynamic.h"

using namespace physx;
using namespace b3d;

PhysXDistanceJoint::PhysXDistanceJoint(PxPhysics* physx, Joint& owner, const DistanceJointCreateInformation& createInformation)
{
	PxRigidActor* actor0 = nullptr;
	if(createInformation.Bodies[0].Body.IsValid())
		actor0 = static_cast<PhysXRigidbody&>(createInformation.Bodies[0].Body->GetImplementation()).GetPxRigidDynamic();

	PxRigidActor* actor1 = nullptr;
	if(createInformation.Bodies[1].Body.IsValid())
		actor1 = static_cast<PhysXRigidbody&>(createInformation.Bodies[1].Body->GetImplementation()).GetPxRigidDynamic();

	PxTransform tfrm0 = ToPxTransform(createInformation.Bodies[0].Position, createInformation.Bodies[0].Rotation);
	PxTransform tfrm1 = ToPxTransform(createInformation.Bodies[1].Position, createInformation.Bodies[1].Rotation);

	PxDistanceJoint* joint = PxDistanceJointCreate(*physx, actor0, tfrm0, actor1, tfrm1);
	mInternal.Initialize(owner, *joint, createInformation);

	PhysXDistanceJoint::SetMinDistance(createInformation.MinDistance);
	PhysXDistanceJoint::SetMaxDistance(createInformation.MaxDistance);
	PhysXDistanceJoint::SetTolerance(createInformation.Tolerance);
	PhysXDistanceJoint::SetSpring(createInformation.Spring);

	PxDistanceJointFlags flags;

	if(createInformation.Flags.IsSet(DistanceJointFlag::MaxDistance))
		flags |= PxDistanceJointFlag::eMAX_DISTANCE_ENABLED;

	if(createInformation.Flags.IsSet(DistanceJointFlag::MinDistance))
		flags |= PxDistanceJointFlag::eMIN_DISTANCE_ENABLED;

	if(createInformation.Flags.IsSet(DistanceJointFlag::Spring))
		flags |= PxDistanceJointFlag::eSPRING_ENABLED;

	joint->setDistanceJointFlags(flags);
}

Spring PhysXDistanceJoint::GetSpring() const
{
	const float damping = GetPxDistanceJoint().getDamping();
	const float stiffness = GetPxDistanceJoint().getStiffness();

	return Spring(stiffness, damping);
}

void PhysXDistanceJoint::SetSpring(const Spring& value)
{
	GetPxDistanceJoint().setDamping(value.Damping);
	GetPxDistanceJoint().setStiffness(value.Stiffness);
}
