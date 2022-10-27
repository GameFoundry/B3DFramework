//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsPhysXFixedJoint.h"
#include "BsFPhysXJoint.h"
#include "BsPhysXRigidbody.h"
#include "extensions/PxFixedJoint.h"
#include "PxRigidDynamic.h"

using namespace physx;

using namespace bs;

PhysXFixedJoint::PhysXFixedJoint(PxPhysics* physx, const FIXED_JOINT_DESC& desc)
	: FixedJoint(desc)
{
	PxRigidActor* actor0 = nullptr;
	if(desc.Bodies[0].Body != nullptr)
		actor0 = static_cast<PhysXRigidbody*>(desc.Bodies[0].Body)->GetInternalInternal();

	PxRigidActor* actor1 = nullptr;
	if(desc.Bodies[1].Body != nullptr)
		actor1 = static_cast<PhysXRigidbody*>(desc.Bodies[1].Body)->GetInternalInternal();

	PxTransform tfrm0 = toPxTransform(desc.Bodies[0].Position, desc.Bodies[0].Rotation);
	PxTransform tfrm1 = toPxTransform(desc.Bodies[1].Position, desc.Bodies[1].Rotation);

	PxFixedJoint* joint = PxFixedJointCreate(*physx, actor0, tfrm0, actor1, tfrm1);
	joint->userData = this;

	mInternal = bs_new<FPhysXJoint>(joint, desc);
}

PhysXFixedJoint::~PhysXFixedJoint()
{
	bs_delete(mInternal);
}
