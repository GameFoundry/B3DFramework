//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsPhysXSphericalJoint.h"
#include "BsFPhysXJoint.h"
#include "BsPhysXRigidbody.h"
#include "PxRigidDynamic.h"

using namespace physx;

namespace bs
{
	PxSphericalJointFlag::Enum toPxFlag(SphericalJointFlag flag)
	{
		switch (flag)
		{
		default:
		case SphericalJointFlag::Limit:
			return PxSphericalJointFlag::eLIMIT_ENABLED;
		}
	}

	PhysXSphericalJoint::PhysXSphericalJoint(PxPhysics* physx, const SPHERICAL_JOINT_DESC& desc)
		:SphericalJoint(desc)
	{
		PxRigidActor* actor0 = nullptr;
		if (desc.bodies[0].body != nullptr)
			actor0 = static_cast<PhysXRigidbody*>(desc.bodies[0].body)->GetInternalInternal();

		PxRigidActor* actor1 = nullptr;
		if (desc.bodies[1].body != nullptr)
			actor1 = static_cast<PhysXRigidbody*>(desc.bodies[1].body)->GetInternalInternal();

		PxTransform tfrm0 = toPxTransform(desc.bodies[0].position, desc.bodies[0].rotation);
		PxTransform tfrm1 = toPxTransform(desc.bodies[1].position, desc.bodies[1].rotation);

		PxSphericalJoint* joint = PxSphericalJointCreate(*physx, actor0, tfrm0, actor1, tfrm1);
		joint->userData = this;

		mInternal = bs_new<FPhysXJoint>(joint, desc);

		PxSphericalJointFlags flags;

		if (((UINT32)desc.flag & (UINT32)SphericalJointFlag::Limit) != 0)
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

	void PhysXSphericalJoint::setFlag(SphericalJointFlag flag, bool enabled)
	{
		getInternal()->setSphericalJointFlag(toPxFlag(flag), enabled);
	}

	bool PhysXSphericalJoint::hasFlag(SphericalJointFlag flag) const
	{
		return getInternal()->getSphericalJointFlags() & toPxFlag(flag);
	}

	PxSphericalJoint* PhysXSphericalJoint::getInternal() const
	{
		FPhysXJoint* internal = static_cast<FPhysXJoint*>(mInternal);

		return static_cast<PxSphericalJoint*>(internal->GetInternalInternal());
	}
}
