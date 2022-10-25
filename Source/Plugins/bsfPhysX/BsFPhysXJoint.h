//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsPhysXPrerequisites.h"
#include "Physics/BsFJoint.h"
#include "extensions/PxJoint.h"

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
		Rigidbody* GetBody(JointBody body) const;

		/** @copydoc FJoint::setBody */
		void SetBody(JointBody body, Rigidbody* value);

		/** @copydoc FJoint::getPosition */
		Vector3 GetPosition(JointBody body) const;

		/** @copydoc FJoint::getRotation */
		Quaternion GetRotation(JointBody body) const;

		/** @copydoc FJoint::setTransform */
		void SetTransform(JointBody body, const Vector3& position, const Quaternion& rotation);

		/** @copydoc FJoint::getBreakForce */
		float GetBreakForce() const;

		/** @copydoc FJoint::setBreakForce */
		void SetBreakForce(float force);

		/** @copydoc FJoint::getBreakTorque */
		float GetBreakTorque() const;

		/** @copydoc FJoint::setBreakTorque */
		void SetBreakTorque(float torque);

		/** @copydoc FJoint::getEnableCollision */
		bool GetEnableCollision() const;

		/** @copydoc FJoint::setEnableCollision */
		void SetEnableCollision(bool value);

		/** Gets the internal PhysX joint object. */
		physx::PxJoint* GetInternalInternal() const { return mJoint; }

	protected:
		physx::PxJoint* mJoint;
	};

	/** @} */
} // namespace bs
