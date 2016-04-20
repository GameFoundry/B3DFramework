//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsCSliderJoint.h"
#include "BsSceneObject.h"
#include "BsCRigidbody.h"
#include "BsCSliderJointRTTI.h"

namespace BansheeEngine
{
	CSliderJoint::CSliderJoint()
		: CJoint(mDesc)
	{ }

	CSliderJoint::CSliderJoint(const HSceneObject& parent)
		: CJoint(parent, mDesc)
	{
		setName("SliderJoint");
	}

	float CSliderJoint::getPosition() const
	{
		if (mInternal == nullptr)
			return 0.0f;

		return _getInternal()->getPosition();
	}

	float CSliderJoint::getSpeed() const
	{
		if (mInternal == nullptr)
			return 0.0f;

		return _getInternal()->getSpeed();
	}

	LimitLinearRange CSliderJoint::getLimit() const
	{
		return mDesc.limit;
	}

	void CSliderJoint::setLimit(const LimitLinearRange& limit)
	{
		if (mDesc.limit == limit)
			return;

		mDesc.limit = limit;

		if (mInternal != nullptr)
			_getInternal()->setLimit(limit);
	}

	void CSliderJoint::setFlag(SliderJoint::Flag flag, bool enabled)
	{
		bool isEnabled = ((UINT32)mDesc.flag & (UINT32)flag) != 0;
		if (isEnabled == enabled)
			return;

		if (enabled)
			mDesc.flag = (SliderJoint::Flag)((UINT32)mDesc.flag | (UINT32)flag);
		else
			mDesc.flag = (SliderJoint::Flag)((UINT32)mDesc.flag & ~(UINT32)flag);

		if (mInternal != nullptr)
			_getInternal()->setFlag(flag, enabled);
	}

	bool CSliderJoint::hasFlag(SliderJoint::Flag flag) const
	{
		return ((UINT32)mDesc.flag & (UINT32)flag) != 0;
	}

	SPtr<Joint> CSliderJoint::createInternal()
	{
		SPtr<Joint> joint = SliderJoint::create(mDesc);

		joint->_setOwner(PhysicsOwnerType::Component, this);
		return joint;
	}

	void CSliderJoint::getLocalTransform(JointBody body, Vector3& position, Quaternion& rotation)
	{
		position = mPositions[(int)body];
		rotation = mRotations[(int)body];

		HRigidbody rigidbody = mBodies[(int)body];
		if (rigidbody == nullptr) // Get world space transform if no relative to any body
		{
			Quaternion worldRot = SO()->getWorldRotation();

			rotation = worldRot*rotation;
			position = worldRot.rotate(position) + SO()->getWorldPosition();
		}
		else
		{
			// Use only the offset for positioning, but for rotation use both the offset and target SO rotation.
			// (Needed because we need to rotate the joint SO in order to orient the slider direction, so we need an
			// additional transform that allows us to orient the object)
			position = rotation.rotate(position);
			rotation = (rigidbody->SO()->getWorldRotation()*rotation).inverse()*SO()->getWorldRotation();
		}
	}

	RTTITypeBase* CSliderJoint::getRTTIStatic()
	{
		return CSliderJointRTTI::instance();
	}

	RTTITypeBase* CSliderJoint::getRTTI() const
	{
		return CSliderJoint::getRTTIStatic();
	}
}