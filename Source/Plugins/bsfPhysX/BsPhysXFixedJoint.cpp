//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsPhysXFixedJoint.h"
#include "BsFPhysXJoint.h"
#include "BsPhysXRigidbody.h"
#include "extensions/PxFixedJoint.h"
#include "PxRigidDynamic.h"

using namespace physx;

using namespace b3d;

PhysXFixedJoint::PhysXFixedJoint(PxPhysics* physx, const FixedJointCreateInformation& createInformation)
{
	PxRigidActor* actor0 = nullptr;
	if(createInformation.Bodies[0].Body != nullptr)
		actor0 = static_cast<PhysXRigidbody&>(createInformation.Bodies[0].Body->GetImplementation()).GetPxRigidDynamic();

	PxRigidActor* actor1 = nullptr;
	if(createInformation.Bodies[1].Body != nullptr)
		actor1 = static_cast<PhysXRigidbody&>(createInformation.Bodies[1].Body->GetImplementation()).GetPxRigidDynamic();

	PxTransform tfrm0 = ToPxTransform(createInformation.Bodies[0].Position, createInformation.Bodies[0].Rotation);
	PxTransform tfrm1 = ToPxTransform(createInformation.Bodies[1].Position, createInformation.Bodies[1].Rotation);

	PxFixedJoint* joint = PxFixedJointCreate(*physx, actor0, tfrm0, actor1, tfrm1);
	joint->userData = this;

	mInternal.Initialize(*joint, createInformation);
}
