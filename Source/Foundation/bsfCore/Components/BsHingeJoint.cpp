//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Components/BsHingeJoint.h"

#include "Physics/BsPhysics.h"
#include "Scene/BsSceneObject.h"
#include "Private/RTTI/BsHingeJointRTTI.h"
#include "Scene/BsSceneInstance.h"

using namespace b3d;

HingeJoint::HingeJoint(const HSceneObject& parent)
	: Joint(parent, mInformation)
{
	SetName("HingeJoint");
}

HingeJoint::HingeJoint()
	: HingeJoint(nullptr)
{ }

Radian HingeJoint::GetAngle() const
{
	if(mImplementation == nullptr)
		return Radian(0.0f);

	return GetImplementation().GetAngle();
}

float HingeJoint::GetSpeed() const
{
	if(mImplementation == nullptr)
		return 0.0f;

	return GetImplementation().GetSpeed();
}

void HingeJoint::SetLimit(const LimitAngularRange& limit)
{
	if(limit == mInformation.Limit)
		return;

	mInformation.Limit = limit;

	if(mImplementation != nullptr)
		GetImplementation().SetLimit(limit);
}

void HingeJoint::SetDrive(const HingeJointDrive& drive)
{
	if(drive == mInformation.Drive)
		return;

	mInformation.Drive = drive;

	if(mImplementation != nullptr)
		GetImplementation().SetDrive(drive);
}

void HingeJoint::SetFlag(HingeJointFlag flag, bool enabled)
{
	bool isEnabled = ((u32)mInformation.Flag & (u32)flag) != 0;
	if(isEnabled == enabled)
		return;

	if(enabled)
		mInformation.Flag = (HingeJointFlag)((u32)mInformation.Flag | (u32)flag);
	else
		mInformation.Flag = (HingeJointFlag)((u32)mInformation.Flag & ~(u32)flag);

	if(mImplementation != nullptr)
		GetImplementation().SetFlag(flag, enabled);
}

bool HingeJoint::HasFlag(HingeJointFlag flag) const
{
	return ((u32)mInformation.Flag & (u32)flag) != 0;
}

SPtr<IJointImplementation> HingeJoint::CreateImplementation()
{
	const SPtr<SceneInstance>& scene = SO()->GetScene();
	return scene->GetPhysicsScene()->CreateHingeJoint(*this, mInformation);
}

IHingeJointImplementation& HingeJoint::GetImplementation() const
{
	return static_cast<IHingeJointImplementation&>(*mImplementation);
}

RTTIType* HingeJoint::GetRttiStatic()
{
	return HingeJointRTTI::Instance();
}

RTTIType* HingeJoint::GetRtti() const
{
	return HingeJoint::GetRttiStatic();
}
