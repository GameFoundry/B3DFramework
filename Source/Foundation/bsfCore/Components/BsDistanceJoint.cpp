//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Components/BsDistanceJoint.h"

#include "Physics/BsPhysics.h"
#include "Scene/BsSceneObject.h"
#include "Private/RTTI/BsDistanceJointRTTI.h"
#include "Scene/BsSceneInstance.h"

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
	bool isEnabled = ((u32)mInformation.Flag & (u32)flag) != 0;
	if(isEnabled == enabled)
		return;

	if(enabled)
		mInformation.Flag = (DistanceJointFlag)((u32)mInformation.Flag | (u32)flag);
	else
		mInformation.Flag = (DistanceJointFlag)((u32)mInformation.Flag & ~(u32)flag);

	if(mImplementation != nullptr)
		GetImplementation().SetFlag(flag, enabled);
}

bool DistanceJoint::HasFlag(DistanceJointFlag flag) const
{
	return ((u32)mInformation.Flag & (u32)flag) != 0;
}

SPtr<IJointImplementation> DistanceJoint::CreateImplementation()
{
	const SPtr<SceneInstance>& scene = SO()->GetScene();
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
