//************************************ B3D Framework - Copyright 2026 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DPhysXPrerequisites.h"
#include "B3DPhysXJoint.h"
#include "Components/B3DDistanceJoint.h"
#include "extensions/PxDistanceJoint.h"

namespace b3d
{
	/** @addtogroup PhysX
	 *  @{
	 */

	/** Converts an engine Distance joint flag into a PhysX variant. */
	inline physx::PxDistanceJointFlag::Enum ToPxFlag(DistanceJointFlag flag)
	{
		switch(flag)
		{
		case DistanceJointFlag::MaxDistance:
			return physx::PxDistanceJointFlag::eMAX_DISTANCE_ENABLED;
		case DistanceJointFlag::MinDistance:
			return physx::PxDistanceJointFlag::eMIN_DISTANCE_ENABLED;
		default:
		case DistanceJointFlag::Spring:
			return physx::PxDistanceJointFlag::eSPRING_ENABLED;
		}
	}

	/** PhysX implementation of a DistanceJoint */
	class PhysXDistanceJoint : public IDistanceJointImplementation
	{
	public:
		PhysXDistanceJoint(physx::PxPhysics* physx, Joint& owner, const DistanceJointCreateInformation& createInformation);

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

		// Begin IDistanceJointImplementation
		float GetDistance() const override { return GetPxDistanceJoint().getDistance(); }
		float GetMinDistance() const override { return GetPxDistanceJoint().getMinDistance(); }
		void SetMinDistance(float value) override { GetPxDistanceJoint().setMinDistance(value); }
		float GetMaxDistance() const override { return GetPxDistanceJoint().getMaxDistance(); }
		void SetMaxDistance(float value) override { GetPxDistanceJoint().setMaxDistance(value); }
		float GetTolerance() const override { return GetPxDistanceJoint().getTolerance(); }
		void SetTolerance(float value) override { GetPxDistanceJoint().setTolerance(value); }
		Spring GetSpring() const override;
		void SetSpring(const Spring& value) override;
		void SetFlag(DistanceJointFlag flag, bool enabled) override { GetPxDistanceJoint().setDistanceJointFlag(ToPxFlag(flag), enabled); }
		bool HasFlag(DistanceJointFlag flag) const override { return GetPxDistanceJoint().getDistanceJointFlags() & ToPxFlag(flag); }
		// End IDistanceJointImplementation

	private:
		/** Returns the internal PhysX representation of the distance joint. */
		physx::PxDistanceJoint& GetPxDistanceJoint() const { return static_cast<physx::PxDistanceJoint&>(mInternal.GetPxJoint()); }

		PhysXJoint mInternal;
	};

	/** @} */
} // namespace b3d
