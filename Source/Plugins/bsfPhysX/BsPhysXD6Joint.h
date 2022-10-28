//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsPhysXPrerequisites.h"
#include "Physics/BsD6Joint.h"
#include "PxPhysics.h"
#include "extensions/PxD6Joint.h"

namespace bs
{
	/** @addtogroup PhysX
	 *  @{
	 */

	/** PhysX implementation of a D6 joint. */
	class PhysXD6Joint : public D6Joint
	{
	public:
		PhysXD6Joint(physx::PxPhysics* physx, const D6_JOINT_DESC& desc);
		~PhysXD6Joint();

		D6JointMotion GetMotion(D6JointAxis axis) const override;
		void SetMotion(D6JointAxis axis, D6JointMotion motion) override;
		Radian GetTwist() const override;
		Radian GetSwingY() const override;
		Radian GetSwingZ() const override;
		LimitLinear GetLimitLinear() const override;
		void SetLimitLinear(const LimitLinear& limit) override;
		LimitAngularRange GetLimitTwist() const override;
		void SetLimitTwist(const LimitAngularRange& limit) override;
		LimitConeRange GetLimitSwing() const override;
		void SetLimitSwing(const LimitConeRange& limit) override;
		D6JointDrive GetDrive(D6JointDriveType type) const override;
		void SetDrive(D6JointDriveType type, const D6JointDrive& drive) override;
		Vector3 GetDrivePosition() const override;
		Quaternion GetDriveRotation() const override;
		void SetDriveTransform(const Vector3& position, const Quaternion& rotation) override;
		Vector3 GetDriveLinearVelocity() const override;
		Vector3 GetDriveAngularVelocity() const override;
		void SetDriveVelocity(const Vector3& linear, const Vector3& angular) override;

	private:
		/** Returns the internal PhysX representation of the D6 joint. */
		inline physx::PxD6Joint* GetInternal() const;
	};

	/** @} */
} // namespace bs
