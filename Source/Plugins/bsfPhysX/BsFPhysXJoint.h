//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsPhysXPrerequisites.h"
#include "Physics/BsFJoint.h"
#include "extensions/PxJoint.h"

namespace b3d
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

		CRigidbody* GetBody(JointBody body) const override;
		void SetBody(JointBody body, CRigidbody* value) override;
		Vector3 GetPosition(JointBody body) const override;
		Quaternion GetRotation(JointBody body) const override;
		void SetTransform(JointBody body, const Vector3& position, const Quaternion& rotation) override;
		float GetBreakForce() const override;
		void SetBreakForce(float force) override;
		float GetBreakTorque() const override;
		void SetBreakTorque(float torque) override;
		bool GetEnableCollision() const override;
		void SetEnableCollision(bool value) override;

		/** Gets the internal PhysX joint object. */
		physx::PxJoint* GetInternalInternal() const { return mJoint; }

	protected:
		physx::PxJoint* mJoint;
	};

	/** @} */
} // namespace b3d
