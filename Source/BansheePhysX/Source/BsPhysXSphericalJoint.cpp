//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsPhysXSphericalJoint.h"
#include "BsFPhysxJoint.h"
#include "BsPhysXRigidbody.h"
#include "PxRigidDynamic.h"

using namespace physx;

namespace BansheeEngine
{
	PxSphericalJointFlag::Enum toPxFlag(PhysXSphericalJoint::Flag flag)
	{
		switch (flag)
		{
		default:
		case PhysXSphericalJoint::Flag::Limit:
			return PxSphericalJointFlag::eLIMIT_ENABLED;
		}
	}

	PhysXSphericalJoint::PhysXSphericalJoint(PxPhysics* physx, const SPHERICAL_JOINT_DESC& desc)
		:SphericalJoint(desc)
	{
		PxRigidActor* actor0 = nullptr;
		if (desc.bodies[0].body != nullptr)
			actor0 = static_cast<PhysXRigidbody*>(desc.bodies[0].body)->_getInternal();

		PxRigidActor* actor1 = nullptr;
		if (desc.bodies[1].body != nullptr)
			actor1 = static_cast<PhysXRigidbody*>(desc.bodies[1].body)->_getInternal();

		PxTransform tfrm0 = toPxTransform(desc.bodies[0].position, desc.bodies[0].rotation);
		PxTransform tfrm1 = toPxTransform(desc.bodies[1].position, desc.bodies[1].rotation);

		PxSphericalJoint* joint = PxSphericalJointCreate(*physx, actor0, tfrm0, actor1, tfrm1);
		joint->userData = this;

		mInternal = bs_new<FPhysXJoint>(joint, desc);

		PxSphericalJointFlags flags;

		if (((UINT32)desc.flag & (UINT32)Flag::Limit) != 0)
			flags |= PxSphericalJointFlag::eLIMIT_ENABLED;

		joint->setSphericalJointFlags(flags);

		// Calls to virtual methods are okay here
		setLimit(desc.limit);
	}

	PhysXSphericalJoint::~PhysXSphericalJoint()
	{
		bs_delete(mInternal);
	}

	LimitConeRange PhysXSphericalJoint::getLimit() const
	{
		PxJointLimitCone pxLimit = getInternal()->getLimitCone();

		LimitConeRange limit;
		limit.yLimitAngle = pxLimit.yAngle;
		limit.zLimitAngle = pxLimit.zAngle;
		limit.contactDist = pxLimit.contactDistance;
		limit.restitution = pxLimit.restitution;
		limit.spring.stiffness = pxLimit.stiffness;
		limit.spring.damping = pxLimit.damping;

		return limit;
	}

	void PhysXSphericalJoint::setLimit(const LimitConeRange& limit)
	{
		PxJointLimitCone pxLimit(limit.yLimitAngle.valueRadians(), limit.zLimitAngle.valueRadians(), limit.contactDist);
		pxLimit.stiffness = limit.spring.stiffness;
		pxLimit.damping = limit.spring.damping;
		pxLimit.restitution = limit.restitution;

		getInternal()->setLimitCone(pxLimit);
	}

	void PhysXSphericalJoint::setFlag(Flag flag, bool enabled)
	{
		getInternal()->setSphericalJointFlag(toPxFlag(flag), enabled);
	}

	bool PhysXSphericalJoint::hasFlag(Flag flag) const
	{
		return getInternal()->getSphericalJointFlags() & toPxFlag(flag);
	}

	PxSphericalJoint* PhysXSphericalJoint::getInternal() const
	{
		FPhysXJoint* internal = static_cast<FPhysXJoint*>(mInternal);

		return static_cast<PxSphericalJoint*>(internal->_getInternal());
	}
}