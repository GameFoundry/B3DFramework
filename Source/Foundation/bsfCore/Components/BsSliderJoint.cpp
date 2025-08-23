//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Components/BsCSliderJoint.h"
#include "Scene/BsSceneObject.h"
#include "Components/BsRigidbody.h"
#include "Physics/BsPhysics.h"
#include "Private/RTTI/BsCSliderJointRTTI.h"
#include "Scene/BsSceneInstance.h"

using namespace b3d;

SliderJoint::SliderJoint(const HSceneObject& parent)
	: Joint(parent, mInformation)
{
	SetName("SliderJoint");
}

SliderJoint::SliderJoint()
	: SliderJoint(nullptr)
{ }

float SliderJoint::GetPosition() const
{
	if(mImplementation == nullptr)
		return 0.0f;

	return GetImplementation().GetPosition();
}

float SliderJoint::GetSpeed() const
{
	if(mImplementation == nullptr)
		return 0.0f;

	return GetImplementation().GetSpeed();
}

void SliderJoint::SetLimit(const LimitLinearRange& limit)
{
	if(mInformation.Limit == limit)
		return;

	mInformation.Limit = limit;

	if(mImplementation != nullptr)
		GetImplementation().SetLimit(limit);
}

void SliderJoint::SetFlag(SliderJointFlag flag, bool enabled)
{
	bool isEnabled = ((u32)mInformation.Flag & (u32)flag) != 0;
	if(isEnabled == enabled)
		return;

	if(enabled)
		mInformation.Flag = (SliderJointFlag)((u32)mInformation.Flag | (u32)flag);
	else
		mInformation.Flag = (SliderJointFlag)((u32)mInformation.Flag & ~(u32)flag);

	if(mImplementation != nullptr)
		GetImplementation().SetFlag(flag, enabled);
}

bool SliderJoint::HasFlag(SliderJointFlag flag) const
{
	return ((u32)mInformation.Flag & (u32)flag) != 0;
}

SPtr<IJointImplementation> SliderJoint::CreateImplementation()
{
	const SPtr<SceneInstance>& scene = SO()->GetScene();
	return scene->GetPhysicsScene()->CreateSliderJoint(*this, mInformation);
}

void SliderJoint::CalculateLocalBodyTransform(JointBody body, Vector3& position, Quaternion& rotation)
{
	position = mInformation.Bodies[(u32)body].Position;
	rotation = mInformation.Bodies[(u32)body].Rotation;

	HRigidbody rigidbody = mInformation.Bodies[(u32)body].Body;
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

ISliderJointImplementation& SliderJoint::GetImplementation() const
{
	return static_cast<ISliderJointImplementation&>(*mImplementation);
}

RTTIType* SliderJoint::GetRttiStatic()
{
	return SliderJointRTTI::Instance();
}

RTTIType* SliderJoint::GetRtti() const
{
	return SliderJoint::GetRttiStatic();
}
