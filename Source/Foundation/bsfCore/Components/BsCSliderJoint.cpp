//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Components/BsCSliderJoint.h"
#include "Scene/BsSceneObject.h"
#include "Components/BsCRigidbody.h"
#include "Private/RTTI/BsCSliderJointRTTI.h"
#include "Scene/BsSceneManager.h"

using namespace bs;

CSliderJoint::CSliderJoint()
	: CJoint(mDesc)
{
	SetName("SliderJoint");
}

CSliderJoint::CSliderJoint(const HSceneObject& parent)
	: CJoint(parent, mDesc)
{
	SetName("SliderJoint");
}

float CSliderJoint::GetPosition() const
{
	if(mInternal == nullptr)
		return 0.0f;

	return GetInternalInternal()->GetPosition();
}

float CSliderJoint::GetSpeed() const
{
	if(mInternal == nullptr)
		return 0.0f;

	return GetInternalInternal()->GetSpeed();
}

LimitLinearRange CSliderJoint::GetLimit() const
{
	return mDesc.Limit;
}

void CSliderJoint::SetLimit(const LimitLinearRange& limit)
{
	if(mDesc.Limit == limit)
		return;

	mDesc.Limit = limit;

	if(mInternal != nullptr)
		GetInternalInternal()->SetLimit(limit);
}

void CSliderJoint::SetFlag(SliderJointFlag flag, bool enabled)
{
	bool isEnabled = ((u32)mDesc.Flag & (u32)flag) != 0;
	if(isEnabled == enabled)
		return;

	if(enabled)
		mDesc.Flag = (SliderJointFlag)((u32)mDesc.Flag | (u32)flag);
	else
		mDesc.Flag = (SliderJointFlag)((u32)mDesc.Flag & ~(u32)flag);

	if(mInternal != nullptr)
		GetInternalInternal()->SetFlag(flag, enabled);
}

bool CSliderJoint::HasFlag(SliderJointFlag flag) const
{
	return ((u32)mDesc.Flag & (u32)flag) != 0;
}

SPtr<Joint> CSliderJoint::CreateInternal()
{
	const SPtr<SceneInstance>& scene = SO()->GetScene();
	SPtr<Joint> joint = SliderJoint::Create(*scene->GetPhysicsScene(), mDesc);

	joint->SetOwnerInternal(PhysicsOwnerType::Component, this);
	return joint;
}

void CSliderJoint::GetLocalTransform(JointBody body, Vector3& position, Quaternion& rotation)
{
	position = mPositions[(u32)body];
	rotation = mRotations[(u32)body];

	HRigidbody rigidbody = mBodies[(u32)body];
	const Transform& tfrm = SO()->GetTransform();
	if(rigidbody == nullptr) // Get world space transform if no relative to any body
	{
		Quaternion worldRot = tfrm.GetRotation();

		rotation = worldRot * rotation;
		position = worldRot.Rotate(position) + tfrm.GetPosition();
	}
	else
	{
		const Transform& rigidbodyTfrm = rigidbody->SO()->GetTransform();

		// Use only the offset for positioning, but for rotation use both the offset and target SO rotation.
		// (Needed because we need to rotate the joint SO in order to orient the slider direction, so we need an
		// additional transform that allows us to orient the object)
		position = rotation.Rotate(position);
		rotation = (rigidbodyTfrm.GetRotation() * rotation).Inverse() * tfrm.GetRotation();
	}
}

RTTITypeBase* CSliderJoint::GetRttiStatic()
{
	return CSliderJointRTTI::Instance();
}

RTTITypeBase* CSliderJoint::GetRtti() const
{
	return CSliderJoint::GetRttiStatic();
}
