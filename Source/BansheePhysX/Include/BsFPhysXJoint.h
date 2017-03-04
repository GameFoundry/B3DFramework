//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsPhysXPrerequisites.h"
#include "BsFJoint.h"
#include "extensions\PxJoint.h"

namespace bs
{
	/** @addtogroup PhysX
	 *  @{
	 */

	/** PhysX implementation of an FJoint. */
	class FPhysXJoint : public FJoint
	{
	public:
		FPhysXJoint(physx::PxJoint* joint, const JOINT_DESC& desc);
		~FPhysXJoint();

		/** @copydoc FJoint::getBody */
		Rigidbody* getBody(JointBody body) const override;

		/** @copydoc FJoint::setBody */
		void setBody(JointBody body, Rigidbody* value) override;

		/** @copydoc FJoint::getPosition */
		Vector3 getPosition(JointBody body) const override;

		/** @copydoc FJoint::getRotation */
		Quaternion getRotation(JointBody body) const override;

		/** @copydoc FJoint::setTransform */
		void setTransform(JointBody body, const Vector3& position, const Quaternion& rotation) override;

		/** @copydoc FJoint::getBreakForce */
		float getBreakForce() const override;

		/** @copydoc FJoint::setBreakForce */
		void setBreakForce(float force) override;

		/** @copydoc FJoint::getBreakTorque */
		float getBreakTorque() const override;

		/** @copydoc FJoint::setBreakTorque */
		void setBreakTorque(float torque) override;

		/** @copydoc FJoint::getEnableCollision */
		bool getEnableCollision() const override;

		/** @copydoc FJoint::setEnableCollision */
		void setEnableCollision(bool value) override;

		/** Gets the internal PhysX joint object. */
		physx::PxJoint* _getInternal() const { return mJoint; }
	protected:
		physx::PxJoint* mJoint;
	};

	/** @} */
}