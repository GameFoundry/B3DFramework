//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Components/B3DSliderJoint.h"
#include "Scene/B3DSceneObject.h"
#include "Components/B3DRigidbody.h"
#include "Physics/B3DPhysics.h"
#include "RTTI/B3DSliderJointRTTI.h"
#include "Scene/B3DSceneInstance.h"

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
	bool isEnabled = mInformation.Flags.IsSet(flag);
	if(isEnabled == enabled)
		return;

	if(enabled)
		mInformation.Flags.Set(flag);
	else
		mInformation.Flags.Unset(flag);

	if(mImplementation != nullptr)
		GetImplementation().SetFlag(flag, enabled);
}

UPtr<IJointImplementation> SliderJoint::CreateImplementation()
{
	const TShared<SceneInstance>& scene = SO()->GetScene();
	return scene->GetPhysicsScene()->CreateSliderJoint(*this, mInformation);
}

void SliderJoint::CalculateLocalBodyTransform(JointBody body, Vector3& position, Quaternion& rotation)
{
	position = mInformation.Bodies[(u32)body].Position;
	rotation = mInformation.Bodies[(u32)body].Rotation;

	HRigidbody rigidbody = mInformation.Bodies[(u32)body].Body;
	const Transform& transform = SO()->GetTransform();
	if(rigidbody == nullptr) // Get world space transform if no relative to any body
	{
		Quaternion worldRotation = transform.GetRotation();

		rotation = worldRotation * rotation;
		position = worldRotation.Rotate(position) + transform.GetPosition();
	}
	else
	{
		const Transform& rigidbodyTransform = rigidbody->SO()->GetTransform();

		// Use only the offset for positioning, but for rotation use both the offset and target SO rotation.
		// (Needed because we need to rotate the joint SO in order to orient the slider direction, so we need an
		// additional transform that allows us to orient the object)
		position = rotation.Rotate(position);
		rotation = (rigidbodyTransform.GetRotation() * rotation).Inverse() * transform.GetRotation();
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
