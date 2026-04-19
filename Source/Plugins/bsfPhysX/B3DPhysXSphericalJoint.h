//************************************ B3D Framework - Copyright 2026 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DPhysXPrerequisites.h"
#include "PxPhysics.h"
#include "Components/B3DSphericalJoint.h"
#include "extensions/PxSphericalJoint.h"
#include "B3DPhysXJoint.h"

namespace b3d
{
	/** @addtogroup PhysX
	 *  @{
	 */

	/** Converts an engine Spherical joint flag into a PhysX variant. */
	inline physx::PxSphericalJointFlag::Enum ToPxFlag(SphericalJointFlag flag)
	{
		switch(flag)
		{
		default:
		case SphericalJointFlag::Limit:
			return physx::PxSphericalJointFlag::eLIMIT_ENABLED;
		}
	}

	/** PhysX implementation of a SphericalJoint. */
	class PhysXSphericalJoint : public ISphericalJointImplementation
	{
	public:
		PhysXSphericalJoint(physx::PxPhysics* physx, Joint& owner, const SphericalJointCreateInformation& createInformation);

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

		// Begin ISphericalJointImplementation
		LimitConeRange GetLimit() const override;
		void SetLimit(const LimitConeRange& limit) override;
		void SetFlag(SphericalJointFlag flag, bool enabled) override { GetPxSphericalJoint().setSphericalJointFlag(ToPxFlag(flag), enabled); }
		bool HasFlag(SphericalJointFlag flag) const override { return GetPxSphericalJoint().getSphericalJointFlags() & ToPxFlag(flag); }
		// End ISphericalJointImplementation

	private:
		/** Returns the internal PhysX representation of the spherical joint. */
		physx::PxSphericalJoint& GetPxSphericalJoint() const { return static_cast<physx::PxSphericalJoint&>(mInternal.GetPxJoint()); }

		PhysXJoint mInternal;
	};

	/** @} */
} // namespace b3d
