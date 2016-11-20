//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsPhysXFixedJoint.h"
#include "BsFPhysxJoint.h"
#include "BsPhysXRigidbody.h"
#include "extensions\PxFixedJoint.h"
#include "PxRigidDynamic.h"

using namespace physx;

namespace bs
{
	PhysXFixedJoint::PhysXFixedJoint(PxPhysics* physx, const FIXED_JOINT_DESC& desc)
		:FixedJoint(desc)
	{
		PxRigidActor* actor0 = nullptr;
		if (desc.bodies[0].body != nullptr)
			actor0 = static_cast<PhysXRigidbody*>(desc.bodies[0].body)->_getInternal();

		PxRigidActor* actor1 = nullptr;
		if (desc.bodies[1].body != nullptr)
			actor1 = static_cast<PhysXRigidbody*>(desc.bodies[1].body)->_getInternal();

		PxTransform tfrm0 = toPxTransform(desc.bodies[0].position, desc.bodies[0].rotation);
		PxTransform tfrm1 = toPxTransform(desc.bodies[1].position, desc.bodies[1].rotation);

		PxFixedJoint* joint = PxFixedJointCreate(*physx, actor0, tfrm0, actor1, tfrm1);
		joint->userData = this;

		mInternal = bs_new<FPhysXJoint>(joint, desc);
	}

	PhysXFixedJoint::~PhysXFixedJoint()
	{
		bs_delete(mInternal);
	}
}