//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsCHingeJoint.h"
#include "BsSceneObject.h"
#include "BsCHingeJointRTTI.h"

namespace bs
{
	CHingeJoint::CHingeJoint()
		: CJoint(mDesc)
	{ }

	CHingeJoint::CHingeJoint(const HSceneObject& parent)
		: CJoint(parent, mDesc)
	{
		setName("HingeJoint");
	}

	Radian CHingeJoint::getAngle() const
	{
		if (mInternal == nullptr)
			return Radian(0.0f);

		return _getInternal()->getAngle();
	}

	float CHingeJoint::getSpeed() const
	{
		if (mInternal == nullptr)
			return 0.0f;

		return _getInternal()->getSpeed();
	}

	LimitAngularRange CHingeJoint::getLimit() const
	{
		return mDesc.limit;
	}

	void CHingeJoint::setLimit(const LimitAngularRange& limit)
	{
		if (limit == mDesc.limit)
			return;

		mDesc.limit = limit;

		if (mInternal != nullptr)
			_getInternal()->setLimit(limit);
	}

	HingeJoint::Drive CHingeJoint::getDrive() const
	{
		return mDesc.drive;
	}

	void CHingeJoint::setDrive(const HingeJoint::Drive& drive)
	{
		if (drive == mDesc.drive)
			return;

		mDesc.drive = drive;

		if (mInternal != nullptr)
			_getInternal()->setDrive(drive);
	}

	void CHingeJoint::setFlag(HingeJoint::Flag flag, bool enabled)
	{
		bool isEnabled = ((UINT32)mDesc.flag & (UINT32)flag) != 0;
		if (isEnabled == enabled)
			return;

		if (enabled)
			mDesc.flag = (HingeJoint::Flag)((UINT32)mDesc.flag | (UINT32)flag);
		else
			mDesc.flag = (HingeJoint::Flag)((UINT32)mDesc.flag & ~(UINT32)flag);

		if (mInternal != nullptr)
			_getInternal()->setFlag(flag, enabled);
	}

	bool CHingeJoint::hasFlag(HingeJoint::Flag flag) const
	{
		return ((UINT32)mDesc.flag & (UINT32)flag) != 0;
	}

	SPtr<Joint> CHingeJoint::createInternal()
	{
		SPtr<Joint> joint = HingeJoint::create(mDesc);

		joint->_setOwner(PhysicsOwnerType::Component, this);
		return joint;
	}

	RTTITypeBase* CHingeJoint::getRTTIStatic()
	{
		return CHingeJointRTTI::instance();
	}

	RTTITypeBase* CHingeJoint::getRTTI() const
	{
		return CHingeJoint::getRTTIStatic();
	}
}