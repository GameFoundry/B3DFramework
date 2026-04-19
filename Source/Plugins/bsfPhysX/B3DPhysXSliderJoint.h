//************************************ B3D Framework - Copyright 2026 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DPhysXPrerequisites.h"
#include "PxPhysics.h"
#include "Components/B3DSliderJoint.h"
#include "extensions/PxPrismaticJoint.h"
#include "B3DPhysXJoint.h"

namespace b3d
{
	/** @addtogroup PhysX
	 *  @{
	 */

	/** Converts an engine Slider joint flag into a PhysX variant. */
	inline physx::PxPrismaticJointFlag::Enum ToPxFlag(SliderJointFlag flag)
	{
		switch(flag)
		{
		default:
		case SliderJointFlag::Limit:
			return physx::PxPrismaticJointFlag::eLIMIT_ENABLED;
		}
	}

	/** PhysX implementation of a SliderJoint. */
	class PhysXSliderJoint : public ISliderJointImplementation
	{
	public:
		PhysXSliderJoint(physx::PxPhysics* physx, Joint& owner, const SliderJointCreateInformation& createInformation);

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

		// Begin ISliderJointImplementation
		float GetPosition() const override { return GetPxPrismaticJoint().getPosition(); }
		float GetSpeed() const override { return GetPxPrismaticJoint().getVelocity(); }
		LimitLinearRange GetLimit() const override;
		void SetLimit(const LimitLinearRange& limit) override;
		void SetFlag(SliderJointFlag flag, bool enabled) override { GetPxPrismaticJoint().setPrismaticJointFlag(ToPxFlag(flag), enabled); }
		bool HasFlag(SliderJointFlag flag) const override { return GetPxPrismaticJoint().getPrismaticJointFlags() & ToPxFlag(flag); }
		// End ISliderJointImplementation

	private:
		/** Returns the internal PhysX representation of the slider (prismatic) joint. */
		physx::PxPrismaticJoint& GetPxPrismaticJoint() const { return static_cast<physx::PxPrismaticJoint&>(mInternal.GetPxJoint()); }

		PhysXJoint mInternal;
	};

	/** @} */
} // namespace b3d
