//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Components/BsCDistanceJoint.h"
#include "Scene/BsSceneObject.h"
#include "Private/RTTI/BsCDistanceJointRTTI.h"
#include "Scene/BsSceneManager.h"

namespace bs
{
	CDistanceJoint::CDistanceJoint()
		: CJoint(mDesc)
	{
		setName("DistanceJoint");
	}

	CDistanceJoint::CDistanceJoint(const HSceneObject& parent)
		: CJoint(parent, mDesc)
	{
		setName("DistanceJoint");
	}

	float CDistanceJoint::GetDistance() const
	{
		if (mInternal == nullptr)
			return 0.0f;

		return _getInternal()->GetDistance();
	}

	float CDistanceJoint::GetMinDistance() const
	{
		return mDesc.minDistance;
	}

	void CDistanceJoint::SetMinDistance(float value)
	{
		if (mDesc.minDistance == value)
			return;

		mDesc.minDistance = value;

		if (mInternal != nullptr)
			_getInternal()->SetMinDistance(value);
	}

	float CDistanceJoint::GetMaxDistance() const
	{
		return mDesc.maxDistance;
	}

	void CDistanceJoint::SetMaxDistance(float value)
	{
		if (mDesc.maxDistance == value)
			return;

		mDesc.maxDistance = value;

		if (mInternal != nullptr)
			_getInternal()->SetMaxDistance(value);
	}

	float CDistanceJoint::GetTolerance() const
	{
		return mDesc.tolerance;
	}

	void CDistanceJoint::SetTolerance(float value)
	{
		if (mDesc.tolerance == value)
			return;

		mDesc.tolerance = value;

		if (mInternal != nullptr)
			_getInternal()->SetTolerance(value);
	}

	Spring CDistanceJoint::GetSpring() const
	{
		return mDesc.spring;
	}

	void CDistanceJoint::SetSpring(const Spring& value)
	{
		if (mDesc.spring == value)
			return;

		mDesc.spring = value;

		if(mInternal != nullptr)
			_getInternal()->SetSpring(value);
	}

	void CDistanceJoint::SetFlag(DistanceJointFlag flag, bool enabled)
	{
		bool isEnabled = ((UINT32)mDesc.flag & (UINT32)flag) != 0;
		if (isEnabled == enabled)
			return;

		if (enabled)
			mDesc.flag = (DistanceJointFlag)((UINT32)mDesc.flag | (UINT32)flag);
		else
			mDesc.flag = (DistanceJointFlag)((UINT32)mDesc.flag & ~(UINT32)flag);

		if (mInternal != nullptr)
			_getInternal()->SetFlag(flag, enabled);
	}

	bool CDistanceJoint::HasFlag(DistanceJointFlag flag) const
	{
		return ((UINT32)mDesc.flag & (UINT32)flag) != 0;
	}

	SPtr<Joint> CDistanceJoint::CreateInternal()
	{
		const SPtr<SceneInstance>& scene = SO()->GetScene();
		SPtr<Joint> joint = DistanceJoint::create(*scene->GetPhysicsScene(), mDesc);

		joint->_setOwner(PhysicsOwnerType::Component, this);
		return joint;
	}

	RTTITypeBase* CDistanceJoint::getRTTIStatic()
	{
		return CDistanceJointRTTI::Instance();
	}

	RTTITypeBase* CDistanceJoint::getRTTI() const
	{
		return CDistanceJoint::GetRTTIStatic();
	}
}
