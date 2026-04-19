//************************************ B3D Framework - Copyright 2026 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DPhysXPrerequisites.h"
#include "PxPhysics.h"
#include "Components/B3DD6Joint.h"
#include "extensions/PxD6Joint.h"
#include "B3DPhysXJoint.h"

namespace b3d
{
	/** @addtogroup PhysX
	 *  @{
	 */

	/** Converts an engine D6 axis flag into a PhysX variant. */
	inline physx::PxD6Axis::Enum ToPxAxis(D6JointAxis axis)
	{
		switch(axis)
		{
		default:
		case D6JointAxis::X:
			return physx::PxD6Axis::eX;
		case D6JointAxis::Y:
			return physx::PxD6Axis::eY;
		case D6JointAxis::Z:
			return physx::PxD6Axis::eZ;
		case D6JointAxis::Twist:
			return physx::PxD6Axis::eTWIST;
		case D6JointAxis::SwingY:
			return physx::PxD6Axis::eSWING1;
		case D6JointAxis::SwingZ:
			return physx::PxD6Axis::eSWING2;
		}
	}

	/** Converts an engine D6 motion flag into a PhysX variant. */
	inline physx::PxD6Motion::Enum ToPxMotion(D6JointMotion motion)
	{
		switch(motion)
		{
		default:
		case D6JointMotion::Free:
			return physx::PxD6Motion::eFREE;
		case D6JointMotion::Limited:
			return physx::PxD6Motion::eLIMITED;
		case D6JointMotion::Locked:
			return physx::PxD6Motion::eLOCKED;
		}
	}

	/** Converts a PhysX D6 motion flag into an engine variant. */
	inline D6JointMotion FromPxMotion(physx::PxD6Motion::Enum motion)
	{
		switch(motion)
		{
		default:
		case physx::PxD6Motion::eFREE:
			return D6JointMotion::Free;
		case physx::PxD6Motion::eLIMITED:
			return D6JointMotion::Limited;
		case physx::PxD6Motion::eLOCKED:
			return D6JointMotion::Locked;
		}
	}

	/** PhysX implementation of a D6 joint. */
	class PhysXD6Joint : public ID6JointImplementation
	{
	public:
		PhysXD6Joint(physx::PxPhysics* physx, Joint& owner, const D6JointCreateInformation& createInformation);

		// Begin IJointImplementation
		void SetBody(JointBody body, Rigidbody* value) override { mInternal.SetBody(body, value); }
		Rigidbody* GetBody(JointBody body) const override { return mInternal.GetBody(body); }
		Vector3 GetPosition(JointBody body) const override { return mInternal.GetPosition(body); }
		Quaternion GetRotation(JointBody body) const override { return mInternal.GetRotation(body); }
		void SetTransform(JointBody body, const Vector3& position, const Quaternion& rotation) override { mInternal.SetTransform(body, position, rotation); }
		void SetBreakForce(float force) override { mInternal.SetBreakForce(force); }
		float GetBreakForce() const override { return mInternal.GetBreakForce(); }
		void SetBreakTorque(float torque) override { mInternal.SetBreakTorque(torque); }
		float GetBreakTorque() const override { return mInternal.GetBreakTorque(); }
		void SetEnableCollision(bool value) override { mInternal.SetEnableCollision(value); } 
		bool GetEnableCollision() const override { return mInternal.GetEnableCollision(); }
		// End IJointImplementation

		// Begin ID6JointImplementation
		D6JointMotion GetMotion(D6JointAxis axis) const override { return FromPxMotion(GetPxD6Joint().getMotion(ToPxAxis(axis))); }
		void SetMotion(D6JointAxis axis, D6JointMotion motion) override { GetPxD6Joint().setMotion(ToPxAxis(axis), ToPxMotion(motion)); }
		Radian GetTwist() const override { return Radian(GetPxD6Joint().getTwist()); }
		Radian GetSwingY() const override { return Radian(GetPxD6Joint().getSwingYAngle()); }
		Radian GetSwingZ() const override { return Radian(GetPxD6Joint().getSwingZAngle()); }
		LimitLinear GetLimitLinear() const override;
		void SetLimitLinear(const LimitLinear& limit) override;
		LimitAngularRange GetLimitTwist() const override;
		void SetLimitTwist(const LimitAngularRange& limit) override;
		LimitConeRange GetLimitSwing() const override;
		void SetLimitSwing(const LimitConeRange& limit) override;
		D6JointDrive GetDrive(D6JointDriveType type) const override;
		void SetDrive(D6JointDriveType type, const D6JointDrive& drive) override;
		Vector3 GetDrivePosition() const override { return FromPxVector(GetPxD6Joint().getDrivePosition().p); }
		Quaternion GetDriveRotation() const override { return FromPxQuaternion(GetPxD6Joint().getDrivePosition().q); }
		void SetDriveTransform(const Vector3& position, const Quaternion& rotation) override { GetPxD6Joint().setDrivePosition(ToPxTransform(position, rotation)); }
		Vector3 GetDriveLinearVelocity() const override;
		Vector3 GetDriveAngularVelocity() const override;
		void SetDriveVelocity(const Vector3& linear, const Vector3& angular) override { GetPxD6Joint().setDriveVelocity(ToPxVector(linear), ToPxVector(angular)); }
		// End ID6JointImplementation

	private:
		/** Returns the internal PhysX representation of the D6 joint. */
		physx::PxD6Joint& GetPxD6Joint() const { return static_cast<physx::PxD6Joint&>(mInternal.GetPxJoint()); }

		PhysXJoint mInternal;
	};

	/** @} */
} // namespace b3d
