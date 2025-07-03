//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Components/BsCD6Joint.h"
#include "Scene/BsSceneObject.h"
#include "Private/RTTI/BsCD6JointRTTI.h"
#include "Scene/BsSceneInstance.h"

using namespace b3d;

CD6Joint::CD6Joint()
	: CJoint(mDesc)
{
	SetName("D6Joint");
}

CD6Joint::CD6Joint(const HSceneObject& parent)
	: CJoint(parent, mDesc)
{
	SetName("D6Joint");
}

D6JointMotion CD6Joint::GetMotion(D6JointAxis axis) const
{
	return mDesc.Motion[(int)axis];
}

void CD6Joint::SetMotion(D6JointAxis axis, D6JointMotion motion)
{
	if(mDesc.Motion[(int)axis] == motion)
		return;

	mDesc.Motion[(int)axis] = motion;

	if(mInternal != nullptr)
		GetInternalInternal()->SetMotion(axis, motion);
}

Radian CD6Joint::GetTwist() const
{
	if(mInternal == nullptr)
		return Radian(0.0f);

	return GetInternalInternal()->GetTwist();
}

Radian CD6Joint::GetSwingY() const
{
	if(mInternal == nullptr)
		return Radian(0.0f);

	return GetInternalInternal()->GetSwingY();
}

Radian CD6Joint::GetSwingZ() const
{
	if(mInternal == nullptr)
		return Radian(0.0f);

	return GetInternalInternal()->GetSwingZ();
}

LimitLinear CD6Joint::GetLimitLinear() const
{
	return mDesc.LimitLinear;
}

void CD6Joint::SetLimitLinear(const LimitLinear& limit)
{
	if(mDesc.LimitLinear == limit)
		return;

	mDesc.LimitLinear = limit;

	if(mInternal != nullptr)
		GetInternalInternal()->SetLimitLinear(limit);
}

LimitAngularRange CD6Joint::GetLimitTwist() const
{
	return mDesc.LimitTwist;
}

void CD6Joint::SetLimitTwist(const LimitAngularRange& limit)
{
	if(mDesc.LimitTwist == limit)
		return;

	mDesc.LimitTwist = limit;

	if(mInternal != nullptr)
		GetInternalInternal()->SetLimitTwist(limit);
}

LimitConeRange CD6Joint::GetLimitSwing() const
{
	return mDesc.LimitSwing;
}

void CD6Joint::SetLimitSwing(const LimitConeRange& limit)
{
	if(mDesc.LimitSwing == limit)
		return;

	mDesc.LimitSwing = limit;

	if(mInternal != nullptr)
		GetInternalInternal()->SetLimitSwing(limit);
}

D6JointDrive CD6Joint::GetDrive(D6JointDriveType type) const
{
	return mDesc.Drive[(int)type];
}

void CD6Joint::SetDrive(D6JointDriveType type, const D6JointDrive& drive)
{
	if(mDesc.Drive[(int)type] == drive)
		return;

	mDesc.Drive[(int)type] = drive;

	if(mInternal != nullptr)
		GetInternalInternal()->SetDrive(type, drive);
}

Vector3 CD6Joint::GetDrivePosition() const
{
	return mDesc.DrivePosition;
}

Quaternion CD6Joint::GetDriveRotation() const
{
	return mDesc.DriveRotation;
}

void CD6Joint::SetDriveTransform(const Vector3& position, const Quaternion& rotation)
{
	if(mDesc.DrivePosition == position && mDesc.DriveRotation == rotation)
		return;

	mDesc.DrivePosition = position;
	mDesc.DriveRotation = rotation;

	if(mInternal != nullptr)
		GetInternalInternal()->SetDriveTransform(position, rotation);
}

Vector3 CD6Joint::GetDriveLinearVelocity() const
{
	return mDesc.DriveLinearVelocity;
}

Vector3 CD6Joint::GetDriveAngularVelocity() const
{
	return mDesc.DriveAngularVelocity;
}

void CD6Joint::SetDriveVelocity(const Vector3& linear, const Vector3& angular)
{
	if(mDesc.DriveLinearVelocity == linear && mDesc.DriveAngularVelocity == angular)
		return;

	mDesc.DriveLinearVelocity = linear;
	mDesc.DriveAngularVelocity = angular;

	if(mInternal != nullptr)
		GetInternalInternal()->SetDriveVelocity(linear, angular);
}

SPtr<Joint> CD6Joint::CreateInternal()
{
	const SPtr<SceneInstance>& scene = SO()->GetScene();
	SPtr<Joint> joint = D6Joint::Create(*scene->GetPhysicsScene(), mDesc);

	joint->SetOwnerInternal(PhysicsOwnerType::Component, this);
	return joint;
}

RTTIType* CD6Joint::GetRttiStatic()
{
	return CD6JointRTTI::Instance();
}

RTTIType* CD6Joint::GetRtti() const
{
	return CD6Joint::GetRttiStatic();
}
