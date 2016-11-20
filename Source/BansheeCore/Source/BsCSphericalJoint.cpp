//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsCSphericalJoint.h"
#include "BsSceneObject.h"
#include "BsCSphericalJointRTTI.h"

namespace bs
{
	CSphericalJoint::CSphericalJoint()
		: CJoint(mDesc)
	{ }

	CSphericalJoint::CSphericalJoint(const HSceneObject& parent)
		: CJoint(parent, mDesc)
	{
		setName("SphericalJoint");
	}

	LimitConeRange CSphericalJoint::getLimit() const
	{
		return mDesc.limit;
	}

	void CSphericalJoint::setLimit(const LimitConeRange& limit)
	{
		if (limit == mDesc.limit)
			return;

		mDesc.limit = limit;

		if (mInternal != nullptr)
			_getInternal()->setLimit(limit);
	}
	
	void CSphericalJoint::setFlag(SphericalJoint::Flag flag, bool enabled)
	{
		bool isEnabled = ((UINT32)mDesc.flag & (UINT32)flag) != 0;
		if (isEnabled == enabled)
			return;

		if (enabled)
			mDesc.flag = (SphericalJoint::Flag)((UINT32)mDesc.flag | (UINT32)flag);
		else
			mDesc.flag = (SphericalJoint::Flag)((UINT32)mDesc.flag & ~(UINT32)flag);

		if (mInternal != nullptr)
			_getInternal()->setFlag(flag, enabled);
	}

	bool CSphericalJoint::hasFlag(SphericalJoint::Flag flag) const
	{
		return ((UINT32)mDesc.flag & (UINT32)flag) != 0;
	}

	SPtr<Joint> CSphericalJoint::createInternal()
	{
		SPtr<Joint> joint = SphericalJoint::create(mDesc);

		joint->_setOwner(PhysicsOwnerType::Component, this);
		return joint;
	}

	RTTITypeBase* CSphericalJoint::getRTTIStatic()
	{
		return CSphericalJointRTTI::instance();
	}

	RTTITypeBase* CSphericalJoint::getRTTI() const
	{
		return CSphericalJoint::getRTTIStatic();
	}
}