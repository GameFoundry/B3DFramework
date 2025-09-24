//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DNullPhysicsJoints.h"
#include "B3DNullPhysicsRigidbody.h"

using namespace b3d;

FNullPhysicsJoint::FNullPhysicsJoint(const JOINT_DESC& desc)
	: FJoint(desc), mDesc(desc)
{}

void FNullPhysicsJoint::SetTransform(JointBody body, const Vector3& position, const Quaternion& rotation)
{
	mDesc.Bodies[(int)body].Position = position;
	mDesc.Bodies[(int)body].Rotation = rotation;
}

NullPhysicsD6Joint::NullPhysicsD6Joint(const D6_JOINT_DESC& desc)
	: D6Joint(desc)
{
	mInternal = B3DNew<FNullPhysicsJoint>(desc);
}

NullPhysicsD6Joint::~NullPhysicsD6Joint()
{
	B3DDelete(mInternal);
}

void NullPhysicsD6Joint::SetDriveTransform(const Vector3& position, const Quaternion& rotation)
{
	mDesc.DrivePosition = position;
	mDesc.DriveRotation = rotation;
}

void NullPhysicsD6Joint::SetDriveVelocity(const Vector3& linear, const Vector3& angular)
{
	mDesc.DriveLinearVelocity = linear;
	mDesc.DriveAngularVelocity = angular;
}

NullPhysicsDistanceJoint::NullPhysicsDistanceJoint(const DISTANCE_JOINT_DESC& desc)
	: DistanceJoint(desc)
{
	mInternal = B3DNew<FNullPhysicsJoint>(desc);
}

NullPhysicsDistanceJoint::~NullPhysicsDistanceJoint()
{
	B3DDelete(mInternal);
}

void NullPhysicsDistanceJoint::SetFlag(DistanceJointFlag flag, bool enabled)
{
	int flags = (int)mDesc.Flag;
	int newFlag = (int)flag;

	if(enabled)
		flags |= newFlag;
	else
		flags &= ~newFlag;

	mDesc.Flag = (DistanceJointFlag)flags;
}

bool NullPhysicsDistanceJoint::HasFlag(DistanceJointFlag flag) const
{
	return ((int)mDesc.Flag & (int)flag) != 0;
}

NullPhysicsFixedJoint::NullPhysicsFixedJoint(const FIXED_JOINT_DESC& desc)
	: FixedJoint(desc)
{
	mInternal = B3DNew<FNullPhysicsJoint>(desc);
}

NullPhysicsFixedJoint::~NullPhysicsFixedJoint()
{
	B3DDelete(mInternal);
}

NullPhysicsHingeJoint::NullPhysicsHingeJoint(const HINGE_JOINT_DESC& desc)
	: HingeJoint(desc), mDesc(desc)
{
	mInternal = B3DNew<FNullPhysicsJoint>(desc);
}

NullPhysicsHingeJoint::~NullPhysicsHingeJoint()
{
	B3DDelete(mInternal);
}

void NullPhysicsHingeJoint::SetFlag(HingeJointFlag flag, bool enabled)
{
	int flags = (int)mDesc.Flag;
	int newFlag = (int)flag;

	if(enabled)
		flags |= newFlag;
	else
		flags &= ~newFlag;

	mDesc.Flag = (HingeJointFlag)flags;
}

bool NullPhysicsHingeJoint::HasFlag(HingeJointFlag flag) const
{
	return ((int)mDesc.Flag & (int)flag) != 0;
}

NullPhysicsSliderJoint::NullPhysicsSliderJoint(const SLIDER_JOINT_DESC& desc)
	: SliderJoint(desc), mDesc(desc)
{
	mInternal = B3DNew<FNullPhysicsJoint>(desc);
}

NullPhysicsSliderJoint::~NullPhysicsSliderJoint()
{
	B3DDelete(mInternal);
}

void NullPhysicsSliderJoint::SetFlag(SliderJointFlag flag, bool enabled)
{
	int flags = (int)mDesc.Flag;
	int newFlag = (int)flag;

	if(enabled)
		flags |= newFlag;
	else
		flags &= ~newFlag;

	mDesc.Flag = (SliderJointFlag)flags;
}

bool NullPhysicsSliderJoint::HasFlag(SliderJointFlag flag) const
{
	return ((int)mDesc.Flag & (int)flag) != 0;
}

NullPhysicsSphericalJoint::NullPhysicsSphericalJoint(const SPHERICAL_JOINT_DESC& desc)
	: SphericalJoint(desc), mDesc(desc)
{
	mInternal = B3DNew<FNullPhysicsJoint>(desc);
}

NullPhysicsSphericalJoint::~NullPhysicsSphericalJoint()
{
	B3DDelete(mInternal);
}

void NullPhysicsSphericalJoint::SetFlag(SphericalJointFlag flag, bool enabled)
{
	int flags = (int)mDesc.Flag;
	int newFlag = (int)flag;

	if(enabled)
		flags |= newFlag;
	else
		flags &= ~newFlag;

	mDesc.Flag = (SphericalJointFlag)flags;
}

bool NullPhysicsSphericalJoint::HasFlag(SphericalJointFlag flag) const
{
	return ((int)mDesc.Flag & (int)flag) != 0;
}
