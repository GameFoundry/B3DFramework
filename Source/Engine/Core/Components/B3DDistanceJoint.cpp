//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Components/B3DDistanceJoint.h"

#include "Physics/B3DPhysics.h"
#include "Scene/B3DSceneObject.h"
#include "RTTI/B3DDistanceJointRTTI.h"
#include "Scene/B3DSceneInstance.h"

using namespace b3d;

DistanceJoint::DistanceJoint(const HSceneObject& parent)
	: Joint(parent, mInformation)
{
	SetName("DistanceJoint");
}

DistanceJoint::DistanceJoint()
	: DistanceJoint(nullptr)
{ }

float DistanceJoint::GetDistance() const
{
	if(mImplementation == nullptr)
		return 0.0f;

	return GetImplementation().GetDistance();
}

void DistanceJoint::SetMinDistance(float value)
{
	if(mInformation.MinDistance == value)
		return;

	mInformation.MinDistance = value;

	if(mImplementation != nullptr)
		GetImplementation().SetMinDistance(value);
}

void DistanceJoint::SetMaxDistance(float value)
{
	if(mInformation.MaxDistance == value)
		return;

	mInformation.MaxDistance = value;

	if(mImplementation != nullptr)
		GetImplementation().SetMaxDistance(value);
}

void DistanceJoint::SetTolerance(float value)
{
	if(mInformation.Tolerance == value)
		return;

	mInformation.Tolerance = value;

	if(mImplementation != nullptr)
		GetImplementation().SetTolerance(value);
}

void DistanceJoint::SetSpring(const Spring& value)
{
	if(mInformation.Spring == value)
		return;

	mInformation.Spring = value;

	if(mImplementation != nullptr)
		GetImplementation().SetSpring(value);
}

void DistanceJoint::SetFlag(DistanceJointFlag flag, bool enabled)
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

UPtr<IJointImplementation> DistanceJoint::CreateImplementation()
{
	const TShared<SceneInstance>& scene = SO()->GetScene();
	return scene->GetPhysicsScene()->CreateDistanceJoint(*this, mInformation);
}

IDistanceJointImplementation& DistanceJoint::GetImplementation() const
{
	return static_cast<IDistanceJointImplementation&>(*mImplementation);
}

RTTIType* DistanceJoint::GetRttiStatic()
{
	return DistanceJointRTTI::Instance();
}

RTTIType* DistanceJoint::GetRtti() const
{
	return DistanceJoint::GetRttiStatic();
}
