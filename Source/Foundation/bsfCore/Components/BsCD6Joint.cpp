//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Components/BsCD6Joint.h"
#include "Scene/BsSceneObject.h"
#include "Private/RTTI/BsCD6JointRTTI.h"
#include "Scene/BsSceneManager.h"

namespace bs
{
	CD6Joint::CD6Joint()
		: CJoint(mDesc)
	{
		setName("D6Joint");
	}

	CD6Joint::CD6Joint(const HSceneObject& parent)
		: CJoint(parent, mDesc)
	{
		setName("D6Joint");
	}

	D6JointMotion CD6Joint::GetMotion(D6JointAxis axis) const
	{
		return mDesc.motion[(int)axis];
	}

	void CD6Joint::SetMotion(D6JointAxis axis, D6JointMotion motion)
	{
		if (mDesc.motion[(int)axis] == motion)
			return;

		mDesc.motion[(int)axis] = motion;

		if (mInternal != nullptr)
			_getInternal()->SetMotion(axis, motion);
	}

	Radian CD6Joint::GetTwist() const
	{
		if (mInternal == nullptr)
			return Radian(0.0f);

		return _getInternal()->GetTwist();
	}

	Radian CD6Joint::GetSwingY() const
	{
		if (mInternal == nullptr)
			return Radian(0.0f);

		return _getInternal()->GetSwingY();
	}

	Radian CD6Joint::GetSwingZ() const
	{
		if (mInternal == nullptr)
			return Radian(0.0f);

		return _getInternal()->GetSwingZ();
	}

	LimitLinear CD6Joint::GetLimitLinear() const
	{
		return mDesc.limitLinear;
	}

	void CD6Joint::SetLimitLinear(const LimitLinear& limit)
	{
		if (mDesc.limitLinear == limit)
			return;

		mDesc.limitLinear = limit;

		if (mInternal != nullptr)
			_getInternal()->SetLimitLinear(limit);
	}

	LimitAngularRange CD6Joint::GetLimitTwist() const
	{
		return mDesc.limitTwist;
	}

	void CD6Joint::SetLimitTwist(const LimitAngularRange& limit)
	{
		if (mDesc.limitTwist == limit)
			return;

		mDesc.limitTwist = limit;

		if (mInternal != nullptr)
			_getInternal()->SetLimitTwist(limit);
	}

	LimitConeRange CD6Joint::GetLimitSwing() const
	{
		return mDesc.limitSwing;
	}

	void CD6Joint::SetLimitSwing(const LimitConeRange& limit)
	{
		if (mDesc.limitSwing == limit)
			return;

		mDesc.limitSwing = limit;

		if (mInternal != nullptr)
			_getInternal()->SetLimitSwing(limit);
	}

	D6JointDrive CD6Joint::GetDrive(D6JointDriveType type) const
	{
		return mDesc.drive[(int)type];
	}

	void CD6Joint::SetDrive(D6JointDriveType type, const D6JointDrive& drive)
	{
		if (mDesc.drive[(int)type] == drive)
			return;

		mDesc.drive[(int)type] = drive;

		if (mInternal != nullptr)
			_getInternal()->SetDrive(type, drive);
	}

	Vector3 CD6Joint::GetDrivePosition() const
	{
		return mDesc.drivePosition;
	}

	Quaternion CD6Joint::GetDriveRotation() const
	{
		return mDesc.driveRotation;
	}

	void CD6Joint::SetDriveTransform(const Vector3& position, const Quaternion& rotation)
	{
		if (mDesc.drivePosition == position && mDesc.driveRotation == rotation)
			return;

		mDesc.drivePosition = position;
		mDesc.driveRotation = rotation;

		if (mInternal != nullptr)
			_getInternal()->SetDriveTransform(position, rotation);
	}

	Vector3 CD6Joint::GetDriveLinearVelocity() const
	{
		return mDesc.driveLinearVelocity;
	}

	Vector3 CD6Joint::GetDriveAngularVelocity() const
	{
		return mDesc.driveAngularVelocity;
	}

	void CD6Joint::SetDriveVelocity(const Vector3& linear, const Vector3& angular)
	{
		if (mDesc.driveLinearVelocity == linear && mDesc.driveAngularVelocity == angular)
			return;

		mDesc.driveLinearVelocity = linear;
		mDesc.driveAngularVelocity = angular;

		if (mInternal != nullptr)
			_getInternal()->SetDriveVelocity(linear, angular);
	}

	SPtr<Joint> CD6Joint::CreateInternal()
	{
		const SPtr<SceneInstance>& scene = SO()->GetScene();
		SPtr<Joint> joint = D6Joint::create(*scene->GetPhysicsScene(), mDesc);

		joint->_setOwner(PhysicsOwnerType::Component, this);
		return joint;
	}

	RTTITypeBase* CD6Joint::getRTTIStatic()
	{
		return CD6JointRTTI::Instance();
	}

	RTTITypeBase* CD6Joint::getRTTI() const
	{
		return CD6Joint::GetRTTIStatic();
	}
}
