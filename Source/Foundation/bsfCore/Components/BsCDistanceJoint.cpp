//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Components/BsCDistanceJoint.h"
#include "Scene/BsSceneObject.h"
#include "Private/RTTI/BsCDistanceJointRTTI.h"
#include "Scene/BsSceneInstance.h"

using namespace b3d;

CDistanceJoint::CDistanceJoint()
	: CJoint(mDesc)
{
	SetName("DistanceJoint");
}

CDistanceJoint::CDistanceJoint(const HSceneObject& parent)
	: CJoint(parent, mDesc)
{
	SetName("DistanceJoint");
}

float CDistanceJoint::GetDistance() const
{
	if(mInternal == nullptr)
		return 0.0f;

	return GetInternalInternal()->GetDistance();
}

float CDistanceJoint::GetMinDistance() const
{
	return mDesc.MinDistance;
}

void CDistanceJoint::SetMinDistance(float value)
{
	if(mDesc.MinDistance == value)
		return;

	mDesc.MinDistance = value;

	if(mInternal != nullptr)
		GetInternalInternal()->SetMinDistance(value);
}

float CDistanceJoint::GetMaxDistance() const
{
	return mDesc.MaxDistance;
}

void CDistanceJoint::SetMaxDistance(float value)
{
	if(mDesc.MaxDistance == value)
		return;

	mDesc.MaxDistance = value;

	if(mInternal != nullptr)
		GetInternalInternal()->SetMaxDistance(value);
}

float CDistanceJoint::GetTolerance() const
{
	return mDesc.Tolerance;
}

void CDistanceJoint::SetTolerance(float value)
{
	if(mDesc.Tolerance == value)
		return;

	mDesc.Tolerance = value;

	if(mInternal != nullptr)
		GetInternalInternal()->SetTolerance(value);
}

Spring CDistanceJoint::GetSpring() const
{
	return mDesc.Spring;
}

void CDistanceJoint::SetSpring(const Spring& value)
{
	if(mDesc.Spring == value)
		return;

	mDesc.Spring = value;

	if(mInternal != nullptr)
		GetInternalInternal()->SetSpring(value);
}

void CDistanceJoint::SetFlag(DistanceJointFlag flag, bool enabled)
{
	bool isEnabled = ((u32)mDesc.Flag & (u32)flag) != 0;
	if(isEnabled == enabled)
		return;

	if(enabled)
		mDesc.Flag = (DistanceJointFlag)((u32)mDesc.Flag | (u32)flag);
	else
		mDesc.Flag = (DistanceJointFlag)((u32)mDesc.Flag & ~(u32)flag);

	if(mInternal != nullptr)
		GetInternalInternal()->SetFlag(flag, enabled);
}

bool CDistanceJoint::HasFlag(DistanceJointFlag flag) const
{
	return ((u32)mDesc.Flag & (u32)flag) != 0;
}

SPtr<Joint> CDistanceJoint::CreateInternal()
{
	const SPtr<SceneInstance>& scene = SO()->GetScene();
	SPtr<Joint> joint = DistanceJoint::Create(*scene->GetPhysicsScene(), mDesc);

	joint->SetOwnerInternal(PhysicsOwnerType::Component, this);
	return joint;
}

RTTIType* CDistanceJoint::GetRttiStatic()
{
	return CDistanceJointRTTI::Instance();
}

RTTIType* CDistanceJoint::GetRtti() const
{
	return CDistanceJoint::GetRttiStatic();
}
