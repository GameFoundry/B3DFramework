//************************************ B3D Framework - Copyright 2026 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DPhysXJoint.h"
#include "B3DPhysXPrerequisites.h"
#include "PxPhysics.h"
#include "Components/B3DHingeJoint.h"
#include "extensions/PxRevoluteJoint.h"

namespace b3d
{
	/** @addtogroup PhysX
	 *  @{
	 */

	/** Converts an engine Hinge joint flag into a PhysX variant. */
	inline physx::PxRevoluteJointFlag::Enum ToPxFlag(HingeJointFlag flag)
	{
		switch(flag)
		{
		case HingeJointFlag::Limit:
			return physx::PxRevoluteJointFlag::eLIMIT_ENABLED;
		default:
		case HingeJointFlag::Drive:
			return physx::PxRevoluteJointFlag::eDRIVE_ENABLED;
		}
	}

	/** PhysX implementation of a HingeJoint. */
	class PhysXHingeJoint : public IHingeJointImplementation
	{
	public:
		PhysXHingeJoint(physx::PxPhysics* physx, Joint& owner, const HingeJointCreateInformation& createInformation);

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

		// Begin IHingeJointImplementation
		Radian GetAngle() const override { return Radian(GetPxRevoluteJoint().getAngle()); }
		float GetSpeed() const override { return GetPxRevoluteJoint().getVelocity(); }
		LimitAngularRange GetLimit() const override;
		void SetLimit(const LimitAngularRange& limit) override;
		HingeJointDrive GetDrive() const override;
		void SetDrive(const HingeJointDrive& drive) override;
		void SetFlag(HingeJointFlag flag, bool enabled) override { GetPxRevoluteJoint().setRevoluteJointFlag(ToPxFlag(flag), enabled); }
		bool HasFlag(HingeJointFlag flag) const override { return GetPxRevoluteJoint().getRevoluteJointFlags() & ToPxFlag(flag); }
		// End IHingeJointImplementation

	private:
		/** Returns the internal PhysX representation of the hinge (revolute) joint. */
		physx::PxRevoluteJoint& GetPxRevoluteJoint() const { return static_cast<physx::PxRevoluteJoint&>(mInternal.GetPxJoint()); }

		PhysXJoint mInternal;
	};

	/** @} */
} // namespace b3d
