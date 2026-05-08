//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Components/B3DHingeJoint.h"

#include "Physics/B3DPhysics.h"
#include "Scene/B3DSceneObject.h"
#include "RTTI/B3DHingeJointRTTI.h"
#include "Scene/B3DSceneInstance.h"

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

UPtr<IJointImplementation> HingeJoint::CreateImplementation()
{
	const TShared<SceneInstance>& scene = SO()->GetScene();
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
