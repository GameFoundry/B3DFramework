//************************************ B3D Framework - Copyright 2026 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DNullPhysicsPrerequisites.h"
#include "Components/B3DJoint.h"
#include "Components/B3DFixedJoint.h"
#include "Components/B3DDistanceJoint.h"
#include "Components/B3DHingeJoint.h"
#include "Components/B3DSphericalJoint.h"
#include "Components/B3DSliderJoint.h"
#include "Components/B3DD6Joint.h"

namespace b3d
{
	/** @addtogroup NullPhysics
	 *  @{
	 */

	/** Null implementation of IJointImplementation base methods. */
	class NullPhysicsJointBase
	{
	public:
		void SetBody(JointBody body, Rigidbody* value) {}
		Rigidbody* GetBody(JointBody body) const { return nullptr; }
		Vector3 GetPosition(JointBody body) const { return Vector3::kZero; }
		Quaternion GetRotation(JointBody body) const { return Quaternion::kIdentity; }
		void SetTransform(JointBody body, const Vector3& position, const Quaternion& rotation) {}
		void SetBreakForce(float force) {}
		float GetBreakForce() const { return 0.0f; }
		void SetBreakTorque(float torque) {}
		float GetBreakTorque() const { return 0.0f; }
		void SetEnableCollision(bool value) {}
		bool GetEnableCollision() const { return false; }
	};

	/** Null implementation of a FixedJoint. */
	class NullPhysicsFixedJoint : public IFixedJointImplementation, public NullPhysicsJointBase
	{
	public:
		void SetBody(JointBody body, Rigidbody* value) override { NullPhysicsJointBase::SetBody(body, value); }
		Rigidbody* GetBody(JointBody body) const override { return NullPhysicsJointBase::GetBody(body); }
		Vector3 GetPosition(JointBody body) const override { return NullPhysicsJointBase::GetPosition(body); }
		Quaternion GetRotation(JointBody body) const override { return NullPhysicsJointBase::GetRotation(body); }
		void SetTransform(JointBody body, const Vector3& position, const Quaternion& rotation) override { NullPhysicsJointBase::SetTransform(body, position, rotation); }
		void SetBreakForce(float force) override { NullPhysicsJointBase::SetBreakForce(force); }
		float GetBreakForce() const override { return NullPhysicsJointBase::GetBreakForce(); }
		void SetBreakTorque(float torque) override { NullPhysicsJointBase::SetBreakTorque(torque); }
		float GetBreakTorque() const override { return NullPhysicsJointBase::GetBreakTorque(); }
		void SetEnableCollision(bool value) override { NullPhysicsJointBase::SetEnableCollision(value); }
		bool GetEnableCollision() const override { return NullPhysicsJointBase::GetEnableCollision(); }
	};

	/** Null implementation of a DistanceJoint. */
	class NullPhysicsDistanceJoint : public IDistanceJointImplementation, public NullPhysicsJointBase
	{
	public:
		void SetBody(JointBody body, Rigidbody* value) override { NullPhysicsJointBase::SetBody(body, value); }
		Rigidbody* GetBody(JointBody body) const override { return NullPhysicsJointBase::GetBody(body); }
		Vector3 GetPosition(JointBody body) const override { return NullPhysicsJointBase::GetPosition(body); }
		Quaternion GetRotation(JointBody body) const override { return NullPhysicsJointBase::GetRotation(body); }
		void SetTransform(JointBody body, const Vector3& position, const Quaternion& rotation) override { NullPhysicsJointBase::SetTransform(body, position, rotation); }
		void SetBreakForce(float force) override { NullPhysicsJointBase::SetBreakForce(force); }
		float GetBreakForce() const override { return NullPhysicsJointBase::GetBreakForce(); }
		void SetBreakTorque(float torque) override { NullPhysicsJointBase::SetBreakTorque(torque); }
		float GetBreakTorque() const override { return NullPhysicsJointBase::GetBreakTorque(); }
		void SetEnableCollision(bool value) override { NullPhysicsJointBase::SetEnableCollision(value); }
		bool GetEnableCollision() const override { return NullPhysicsJointBase::GetEnableCollision(); }

		float GetDistance() const override { return 0.0f; }
		void SetMinDistance(float value) override {}
		float GetMinDistance() const override { return 0.0f; }
		void SetMaxDistance(float value) override {}
		float GetMaxDistance() const override { return 0.0f; }
		float GetTolerance() const override { return 0.0f; }
		void SetTolerance(float value) override {}
		Spring GetSpring() const override { return {}; }
		void SetSpring(const Spring& value) override {}
		void SetFlag(DistanceJointFlag flag, bool enabled) override {}
		bool HasFlag(DistanceJointFlag flag) const override { return false; }
	};

	/** Null implementation of a HingeJoint. */
	class NullPhysicsHingeJoint : public IHingeJointImplementation, public NullPhysicsJointBase
	{
	public:
		void SetBody(JointBody body, Rigidbody* value) override { NullPhysicsJointBase::SetBody(body, value); }
		Rigidbody* GetBody(JointBody body) const override { return NullPhysicsJointBase::GetBody(body); }
		Vector3 GetPosition(JointBody body) const override { return NullPhysicsJointBase::GetPosition(body); }
		Quaternion GetRotation(JointBody body) const override { return NullPhysicsJointBase::GetRotation(body); }
		void SetTransform(JointBody body, const Vector3& position, const Quaternion& rotation) override { NullPhysicsJointBase::SetTransform(body, position, rotation); }
		void SetBreakForce(float force) override { NullPhysicsJointBase::SetBreakForce(force); }
		float GetBreakForce() const override { return NullPhysicsJointBase::GetBreakForce(); }
		void SetBreakTorque(float torque) override { NullPhysicsJointBase::SetBreakTorque(torque); }
		float GetBreakTorque() const override { return NullPhysicsJointBase::GetBreakTorque(); }
		void SetEnableCollision(bool value) override { NullPhysicsJointBase::SetEnableCollision(value); }
		bool GetEnableCollision() const override { return NullPhysicsJointBase::GetEnableCollision(); }

		Radian GetAngle() const override { return Radian(0.0f); }
		float GetSpeed() const override { return 0.0f; }
		LimitAngularRange GetLimit() const override { return {}; }
		void SetLimit(const LimitAngularRange& limit) override {}
		HingeJointDrive GetDrive() const override { return {}; }
		void SetDrive(const HingeJointDrive& drive) override {}
		void SetFlag(HingeJointFlag flag, bool enabled) override {}
		bool HasFlag(HingeJointFlag flag) const override { return false; }
	};

	/** Null implementation of a SphericalJoint. */
	class NullPhysicsSphericalJoint : public ISphericalJointImplementation, public NullPhysicsJointBase
	{
	public:
		void SetBody(JointBody body, Rigidbody* value) override { NullPhysicsJointBase::SetBody(body, value); }
		Rigidbody* GetBody(JointBody body) const override { return NullPhysicsJointBase::GetBody(body); }
		Vector3 GetPosition(JointBody body) const override { return NullPhysicsJointBase::GetPosition(body); }
		Quaternion GetRotation(JointBody body) const override { return NullPhysicsJointBase::GetRotation(body); }
		void SetTransform(JointBody body, const Vector3& position, const Quaternion& rotation) override { NullPhysicsJointBase::SetTransform(body, position, rotation); }
		void SetBreakForce(float force) override { NullPhysicsJointBase::SetBreakForce(force); }
		float GetBreakForce() const override { return NullPhysicsJointBase::GetBreakForce(); }
		void SetBreakTorque(float torque) override { NullPhysicsJointBase::SetBreakTorque(torque); }
		float GetBreakTorque() const override { return NullPhysicsJointBase::GetBreakTorque(); }
		void SetEnableCollision(bool value) override { NullPhysicsJointBase::SetEnableCollision(value); }
		bool GetEnableCollision() const override { return NullPhysicsJointBase::GetEnableCollision(); }

		LimitConeRange GetLimit() const override { return {}; }
		void SetLimit(const LimitConeRange& limit) override {}
		void SetFlag(SphericalJointFlag flag, bool isEnabled) override {}
		bool HasFlag(SphericalJointFlag flag) const override { return false; }
	};

	/** Null implementation of a SliderJoint. */
	class NullPhysicsSliderJoint : public ISliderJointImplementation, public NullPhysicsJointBase
	{
	public:
		void SetBody(JointBody body, Rigidbody* value) override { NullPhysicsJointBase::SetBody(body, value); }
		Rigidbody* GetBody(JointBody body) const override { return NullPhysicsJointBase::GetBody(body); }
		Vector3 GetPosition(JointBody body) const override { return NullPhysicsJointBase::GetPosition(body); }
		Quaternion GetRotation(JointBody body) const override { return NullPhysicsJointBase::GetRotation(body); }
		void SetTransform(JointBody body, const Vector3& position, const Quaternion& rotation) override { NullPhysicsJointBase::SetTransform(body, position, rotation); }
		void SetBreakForce(float force) override { NullPhysicsJointBase::SetBreakForce(force); }
		float GetBreakForce() const override { return NullPhysicsJointBase::GetBreakForce(); }
		void SetBreakTorque(float torque) override { NullPhysicsJointBase::SetBreakTorque(torque); }
		float GetBreakTorque() const override { return NullPhysicsJointBase::GetBreakTorque(); }
		void SetEnableCollision(bool value) override { NullPhysicsJointBase::SetEnableCollision(value); }
		bool GetEnableCollision() const override { return NullPhysicsJointBase::GetEnableCollision(); }

		float GetPosition() const override { return 0.0f; }
		float GetSpeed() const override { return 0.0f; }
		LimitLinearRange GetLimit() const override { return {}; }
		void SetLimit(const LimitLinearRange& limit) override {}
		void SetFlag(SliderJointFlag flag, bool enabled) override {}
		bool HasFlag(SliderJointFlag flag) const override { return false; }
	};

	/** Null implementation of a D6Joint. */
	class NullPhysicsD6Joint : public ID6JointImplementation, public NullPhysicsJointBase
	{
	public:
		void SetBody(JointBody body, Rigidbody* value) override { NullPhysicsJointBase::SetBody(body, value); }
		Rigidbody* GetBody(JointBody body) const override { return NullPhysicsJointBase::GetBody(body); }
		Vector3 GetPosition(JointBody body) const override { return NullPhysicsJointBase::GetPosition(body); }
		Quaternion GetRotation(JointBody body) const override { return NullPhysicsJointBase::GetRotation(body); }
		void SetTransform(JointBody body, const Vector3& position, const Quaternion& rotation) override { NullPhysicsJointBase::SetTransform(body, position, rotation); }
		void SetBreakForce(float force) override { NullPhysicsJointBase::SetBreakForce(force); }
		float GetBreakForce() const override { return NullPhysicsJointBase::GetBreakForce(); }
		void SetBreakTorque(float torque) override { NullPhysicsJointBase::SetBreakTorque(torque); }
		float GetBreakTorque() const override { return NullPhysicsJointBase::GetBreakTorque(); }
		void SetEnableCollision(bool value) override { NullPhysicsJointBase::SetEnableCollision(value); }
		bool GetEnableCollision() const override { return NullPhysicsJointBase::GetEnableCollision(); }

		void SetMotion(D6JointAxis axis, D6JointMotion motion) override {}
		D6JointMotion GetMotion(D6JointAxis axis) const override { return D6JointMotion::Free; }
		Radian GetTwist() const override { return Radian(0.0f); }
		Radian GetSwingY() const override { return Radian(0.0f); }
		Radian GetSwingZ() const override { return Radian(0.0f); }
		void SetLimitLinear(const LimitLinear& limit) override {}
		LimitLinear GetLimitLinear() const override { return {}; }
		void SetLimitTwist(const LimitAngularRange& limit) override {}
		LimitAngularRange GetLimitTwist() const override { return {}; }
		void SetLimitSwing(const LimitConeRange& limit) override {}
		LimitConeRange GetLimitSwing() const override { return {}; }
		void SetDrive(D6JointDriveType type, const D6JointDrive& drive) override {}
		D6JointDrive GetDrive(D6JointDriveType type) const override { return {}; }
		Vector3 GetDrivePosition() const override { return Vector3::kZero; }
		Quaternion GetDriveRotation() const override { return Quaternion::kIdentity; }
		void SetDriveTransform(const Vector3& position, const Quaternion& rotation) override {}
		Vector3 GetDriveLinearVelocity() const override { return Vector3::kZero; }
		Vector3 GetDriveAngularVelocity() const override { return Vector3::kZero; }
		void SetDriveVelocity(const Vector3& linear, const Vector3& angular) override {}
	};

	/** @} */
} // namespace b3d
