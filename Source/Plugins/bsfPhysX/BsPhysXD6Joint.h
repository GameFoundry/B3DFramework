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

		/** @copydoc D6Joint::getMotion */
		D6JointMotion GetMotion(D6JointAxis axis) const override;

		/** @copydoc D6Joint::setMotion */
		void SetMotion(D6JointAxis axis, D6JointMotion motion) override;

		/** @copydoc D6Joint::getTwist */
		Radian GetTwist() const override;

		/** @copydoc D6Joint::getSwingY */
		Radian GetSwingY() const override;

		/** @copydoc D6Joint::getSwingZ */
		Radian GetSwingZ() const override;

		/** @copydoc D6Joint::getLimitLinear */
		LimitLinear GetLimitLinear() const override;

		/** @copydoc D6Joint::setLimitLinear */
		void SetLimitLinear(const LimitLinear& limit) override;

		/** @copydoc D6Joint::getLimitTwist */
		LimitAngularRange GetLimitTwist() const override;

		/** @copydoc D6Joint::setLimitTwist */
		void SetLimitTwist(const LimitAngularRange& limit) override;

		/** @copydoc D6Joint::getLimitSwing */
		LimitConeRange GetLimitSwing() const override;

		/** @copydoc D6Joint::setLimitSwing */
		void SetLimitSwing(const LimitConeRange& limit) override;

		/** @copydoc D6Joint::getDrive */
		D6JointDrive GetDrive(D6JointDriveType type) const override;

		/** @copydoc D6Joint::setDrive */
		void SetDrive(D6JointDriveType type, const D6JointDrive& drive) override;

		/** @copydoc D6Joint::getDrivePosition */
		Vector3 GetDrivePosition() const override;

		/** @copydoc D6Joint::getDriveRotation */
		Quaternion GetDriveRotation() const override;

		/** @copydoc D6Joint::setDriveTransform */
		void SetDriveTransform(const Vector3& position, const Quaternion& rotation) override;

		/** @copydoc D6Joint::getDriveLinearVelocity */
		Vector3 GetDriveLinearVelocity() const override;

		/** @copydoc D6Joint::getDriveAngularVelocity */
		Vector3 GetDriveAngularVelocity() const override;

		/** @copydoc D6Joint::setDriveVelocity */
		void SetDriveVelocity(const Vector3& linear, const Vector3& angular) override;

	private:
		/** Returns the internal PhysX representation of the D6 joint. */
		inline physx::PxD6Joint* GetInternal() const;
	};

	/** @} */
} // namespace bs
