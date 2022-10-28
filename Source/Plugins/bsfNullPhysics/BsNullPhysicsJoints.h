//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsNullPhysicsPrerequisites.h"
#include "Physics/BsJoint.h"
#include "Physics/BsFJoint.h"
#include "Physics/BsD6Joint.h"
#include "Physics/BsDistanceJoint.h"
#include "Physics/BsFixedJoint.h"
#include "Physics/BsHingeJoint.h"
#include "Physics/BsSliderJoint.h"
#include "Physics/BsSphericalJoint.h"

namespace bs
{
	/** @addtogroup NullPhysics
	 *  @{
	 */

	/** Null implementation of an FJoint. */
	class FNullPhysicsJoint : public FJoint
	{
	public:
		FNullPhysicsJoint(const JOINT_DESC& desc);
		~FNullPhysicsJoint() override = default;

		Rigidbody* GetBody(JointBody body) const override { return mDesc.Bodies[(int)body].Body; }
		void SetBody(JointBody body, Rigidbody* value) override { mDesc.Bodies[(int)body].Body = value; }
		Vector3 GetPosition(JointBody body) const override { return mDesc.Bodies[(int)body].Position; }
		Quaternion GetRotation(JointBody body) const override { return mDesc.Bodies[(int)body].Rotation; }
		void SetTransform(JointBody body, const Vector3& position, const Quaternion& rotation) override;
		float GetBreakForce() const override { return mDesc.BreakForce; }
		void SetBreakForce(float force) override { mDesc.BreakForce = force; }
		float GetBreakTorque() const override { return mDesc.BreakTorque; }
		void SetBreakTorque(float torque) override { mDesc.BreakTorque = torque; }
		bool GetEnableCollision() const override { return mDesc.EnableCollision; }
		void SetEnableCollision(bool value) override { mDesc.EnableCollision = value; }

	private:
		JOINT_DESC mDesc;
	};

	/** NullPhysics implementation of a D6 joint. */
	class NullPhysicsD6Joint : public D6Joint
	{
	public:
		NullPhysicsD6Joint(const D6_JOINT_DESC& desc);
		~NullPhysicsD6Joint() override;

		D6JointMotion GetMotion(D6JointAxis axis) const override { return mDesc.Motion[(int)axis]; }
		void SetMotion(D6JointAxis axis, D6JointMotion motion) override { mDesc.Motion[(int)axis] = motion; }
		Radian GetTwist() const override { return Radian(0.0f); }
		Radian GetSwingY() const override { return Radian(0.0f); }
		Radian GetSwingZ() const override { return Radian(0.0f); }
		LimitLinear GetLimitLinear() const override { return mDesc.LimitLinear; }
		void SetLimitLinear(const LimitLinear& limit) override { mDesc.LimitLinear = limit; }
		LimitAngularRange GetLimitTwist() const override { return mDesc.LimitTwist; }
		void SetLimitTwist(const LimitAngularRange& limit) override { mDesc.LimitTwist = limit; }
		LimitConeRange GetLimitSwing() const override { return mDesc.LimitSwing; }
		void SetLimitSwing(const LimitConeRange& limit) override { mDesc.LimitSwing = limit; }
		D6JointDrive GetDrive(D6JointDriveType type) const override { return mDesc.Drive[(int)type]; }
		void SetDrive(D6JointDriveType type, const D6JointDrive& drive) override { mDesc.Drive[(int)type] = drive; }
		Vector3 GetDrivePosition() const override { return mDesc.DrivePosition; }
		Quaternion GetDriveRotation() const override { return mDesc.DriveRotation; }
		void SetDriveTransform(const Vector3& position, const Quaternion& rotation) override;
		Vector3 GetDriveLinearVelocity() const override { return mDesc.DriveLinearVelocity; }
		Vector3 GetDriveAngularVelocity() const override { return mDesc.DriveAngularVelocity; }
		void SetDriveVelocity(const Vector3& linear, const Vector3& angular) override;

	private:
		D6_JOINT_DESC mDesc;
	};

	/** Null implementation of a DistanceJoint */
	class NullPhysicsDistanceJoint : public DistanceJoint
	{
	public:
		NullPhysicsDistanceJoint(const DISTANCE_JOINT_DESC& desc);
		~NullPhysicsDistanceJoint() override;

		float GetDistance() const override { return 0.0f; }
		float GetMinDistance() const override { return mDesc.MinDistance; }
		void SetMinDistance(float value) override { mDesc.MinDistance = value; }
		float GetMaxDistance() const override { return mDesc.MaxDistance; }
		void SetMaxDistance(float value) override { mDesc.MaxDistance = value; }
		float GetTolerance() const override { return mDesc.Tolerance; }
		void SetTolerance(float value) override { mDesc.Tolerance = value; }
		Spring GetSpring() const override { return mDesc.Spring; }
		void SetSpring(const Spring& value) override { mDesc.Spring = value; }
		void SetFlag(DistanceJointFlag flag, bool enabled) override;
		bool HasFlag(DistanceJointFlag flag) const override;

	private:
		DISTANCE_JOINT_DESC mDesc;
	};

	/** Null implementation of a FixedJoint. */
	class NullPhysicsFixedJoint : public FixedJoint
	{
	public:
		NullPhysicsFixedJoint(const FIXED_JOINT_DESC& desc);
		~NullPhysicsFixedJoint() override;
	};

	/** Null implementation of a HingeJoint. */
	class NullPhysicsHingeJoint : public HingeJoint
	{
	public:
		NullPhysicsHingeJoint(const HINGE_JOINT_DESC& desc);
		~NullPhysicsHingeJoint() override;

		Radian GetAngle() const override { return Radian(0.0f); }
		float GetSpeed() const override { return 0.0f; }
		LimitAngularRange GetLimit() const override { return mDesc.Limit; }
		void SetLimit(const LimitAngularRange& limit) override { mDesc.Limit = limit; }
		HingeJointDrive GetDrive() const override { return mDesc.Drive; }
		void SetDrive(const HingeJointDrive& drive) override { mDesc.Drive = drive; }
		void SetFlag(HingeJointFlag flag, bool enabled) override;
		bool HasFlag(HingeJointFlag flag) const override;

	private:
		HINGE_JOINT_DESC mDesc;
	};

	/** Null implementation of a SliderJoint. */
	class NullPhysicsSliderJoint : public SliderJoint
	{
	public:
		NullPhysicsSliderJoint(const SLIDER_JOINT_DESC& desc);
		~NullPhysicsSliderJoint() override;

		float GetPosition() const override { return 0.0f; }
		float GetSpeed() const override { return 0.0f; }
		LimitLinearRange GetLimit() const override { return mDesc.Limit; }
		void SetLimit(const LimitLinearRange& limit) override { mDesc.Limit = limit; }
		void SetFlag(SliderJointFlag flag, bool enabled) override;
		bool HasFlag(SliderJointFlag flag) const override;

	private:
		SLIDER_JOINT_DESC mDesc;
	};

	/** Null implementation of a SphericalJoint. */
	class NullPhysicsSphericalJoint : public SphericalJoint
	{
	public:
		NullPhysicsSphericalJoint(const SPHERICAL_JOINT_DESC& desc);
		~NullPhysicsSphericalJoint() override;

		LimitConeRange GetLimit() const override { return mDesc.Limit; }
		void SetLimit(const LimitConeRange& limit) override { mDesc.Limit = limit; }
		void SetFlag(SphericalJointFlag flag, bool enabled) override;
		bool HasFlag(SphericalJointFlag flag) const override;

	private:
		SPHERICAL_JOINT_DESC mDesc;
	};

	/** @} */
} // namespace bs
