//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Components/B3DD6Joint.h"

#include "Physics/B3DPhysics.h"
#include "Scene/B3DSceneObject.h"
#include "RTTI/B3DD6JointRTTI.h"
#include "Scene/B3DSceneInstance.h"

using namespace b3d;

RTTIType* D6JointDrive::GetRttiStatic()
{
	return D6JointDriveRTTI::Instance();
}

RTTIType* D6JointDrive::GetRtti() const
{
	return GetRttiStatic();
}

D6Joint::D6Joint(const HSceneObject& parent)
	: Joint(parent, mInformation)
{
	SetName("D6Joint");
}

D6Joint::D6Joint()
	: D6Joint(nullptr)
{ }

void D6Joint::SetMotion(D6JointAxis axis, D6JointMotion motion)
{
	if(mInformation.Motion[(int)axis] == motion)
		return;

	mInformation.Motion[(int)axis] = motion;

	if(mImplementation != nullptr)
		GetImplementation().SetMotion(axis, motion);
}

Radian D6Joint::GetTwist() const
{
	if(mImplementation == nullptr)
		return Radian(0.0f);

	return GetImplementation().GetTwist();
}

Radian D6Joint::GetSwingY() const
{
	if(mImplementation == nullptr)
		return Radian(0.0f);

	return GetImplementation().GetSwingY();
}

Radian D6Joint::GetSwingZ() const
{
	if(mImplementation == nullptr)
		return Radian(0.0f);

	return GetImplementation().GetSwingZ();
}

void D6Joint::SetLimitLinear(const LimitLinear& limit)
{
	if(mInformation.LimitLinear == limit)
		return;

	mInformation.LimitLinear = limit;

	if(mImplementation != nullptr)
		GetImplementation().SetLimitLinear(limit);
}

void D6Joint::SetLimitTwist(const LimitAngularRange& limit)
{
	if(mInformation.LimitTwist == limit)
		return;

	mInformation.LimitTwist = limit;

	if(mImplementation != nullptr)
		GetImplementation().SetLimitTwist(limit);
}

void D6Joint::SetLimitSwing(const LimitConeRange& limit)
{
	if(mInformation.LimitSwing == limit)
		return;

	mInformation.LimitSwing = limit;

	if(mImplementation != nullptr)
		GetImplementation().SetLimitSwing(limit);
}

D6JointDrive D6Joint::GetDrive(D6JointDriveType type) const
{
	return mInformation.Drive[(int)type];
}

void D6Joint::SetDrive(D6JointDriveType type, const D6JointDrive& drive)
{
	if(mInformation.Drive[(int)type] == drive)
		return;

	mInformation.Drive[(int)type] = drive;

	if(mImplementation != nullptr)
		GetImplementation().SetDrive(type, drive);
}

void D6Joint::SetDriveTransform(const Vector3& position, const Quaternion& rotation)
{
	if(mInformation.DrivePosition == position && mInformation.DriveRotation == rotation)
		return;

	mInformation.DrivePosition = position;
	mInformation.DriveRotation = rotation;

	if(mImplementation != nullptr)
		GetImplementation().SetDriveTransform(position, rotation);
}

void D6Joint::SetDriveVelocity(const Vector3& linear, const Vector3& angular)
{
	if(mInformation.DriveLinearVelocity == linear && mInformation.DriveAngularVelocity == angular)
		return;

	mInformation.DriveLinearVelocity = linear;
	mInformation.DriveAngularVelocity = angular;

	if(mImplementation != nullptr)
		GetImplementation().SetDriveVelocity(linear, angular);
}

UPtr<IJointImplementation> D6Joint::CreateImplementation()
{
	const SPtr<SceneInstance>& scene = SO()->GetScene();
	return scene->GetPhysicsScene()->CreateD6Joint(*this, mInformation);
}

ID6JointImplementation& D6Joint::GetImplementation() const
{
	return static_cast<ID6JointImplementation&>(*mImplementation);
}

RTTIType* D6Joint::GetRttiStatic()
{
	return D6JointRTTI::Instance();
}

RTTIType* D6Joint::GetRtti() const
{
	return D6Joint::GetRttiStatic();
}
