//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsCDistanceJoint.h"
#include "BsSceneObject.h"
#include "BsCDistanceJointRTTI.h"

namespace bs
{
	CDistanceJoint::CDistanceJoint()
		: CJoint(mDesc)
	{ }

	CDistanceJoint::CDistanceJoint(const HSceneObject& parent)
		: CJoint(parent, mDesc)
	{
		setName("DistanceJoint");
	}

	float CDistanceJoint::getDistance() const
	{
		if (mInternal == nullptr)
			return 0.0f;

		return _getInternal()->getDistance();
	}

	float CDistanceJoint::getMinDistance() const
	{
		return mDesc.minDistance;
	}

	void CDistanceJoint::setMinDistance(float value)
	{
		if (mDesc.minDistance == value)
			return;

		mDesc.minDistance = value;

		if (mInternal != nullptr)
			_getInternal()->setMinDistance(value);
	}

	float CDistanceJoint::getMaxDistance() const
	{
		return mDesc.maxDistance;
	}

	void CDistanceJoint::setMaxDistance(float value)
	{
		if (mDesc.maxDistance == value)
			return;

		mDesc.maxDistance = value;

		if (mInternal != nullptr)
			_getInternal()->setMaxDistance(value);
	}

	float CDistanceJoint::getTolerance() const
	{
		return mDesc.tolerance;
	}

	void CDistanceJoint::setTolerance(float value)
	{
		if (mDesc.tolerance == value)
			return;

		mDesc.tolerance = value;

		if (mInternal != nullptr)
			_getInternal()->setTolerance(value);
	}

	Spring CDistanceJoint::getSpring() const
	{
		return mDesc.spring;
	}

	void CDistanceJoint::setSpring(const Spring& value)
	{
		if (mDesc.spring == value)
			return;

		mDesc.spring = value;

		if(mInternal != nullptr)
			_getInternal()->setSpring(value);
	}

	void CDistanceJoint::setFlag(DistanceJoint::Flag flag, bool enabled)
	{
		bool isEnabled = ((UINT32)mDesc.flag & (UINT32)flag) != 0;
		if (isEnabled == enabled)
			return;

		if (enabled)
			mDesc.flag = (DistanceJoint::Flag)((UINT32)mDesc.flag | (UINT32)flag);
		else
			mDesc.flag = (DistanceJoint::Flag)((UINT32)mDesc.flag & ~(UINT32)flag);

		if (mInternal != nullptr)
			_getInternal()->setFlag(flag, enabled);
	}

	bool CDistanceJoint::hasFlag(DistanceJoint::Flag flag) const
	{
		return ((UINT32)mDesc.flag & (UINT32)flag) != 0;
	}

	SPtr<Joint> CDistanceJoint::createInternal()
	{
		SPtr<Joint> joint = DistanceJoint::create(mDesc);

		joint->_setOwner(PhysicsOwnerType::Component, this);
		return joint;
	}

	RTTITypeBase* CDistanceJoint::getRTTIStatic()
	{
		return CDistanceJointRTTI::instance();
	}

	RTTITypeBase* CDistanceJoint::getRTTI() const
	{
		return CDistanceJoint::getRTTIStatic();
	}
}