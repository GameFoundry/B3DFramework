//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsCorePrerequisites.h"
#include "Physics/BsD6Joint.h"
#include "Components/BsCJoint.h"

namespace bs 
{
	/** @addtogroup Components-Core
	 *  @{
	 */

	/**
	 * @copydoc	D6Joint
	 *
	 * @note	Wraps D6Joint as a Component.
	 */
	class BS_CORE_EXPORT BS_SCRIPT_EXPORT(m:Physics,n:D6Joint) CD6Joint : public CJoint
	{
	public:
		CD6Joint(const HSceneObject& parent);

		/** @copydoc D6Joint::getMotion */
		BS_SCRIPT_EXPORT(n:GetMotion)
		D6JointMotion getMotion(D6JointAxis axis) const;

		/** @copydoc D6Joint::setMotion */
		BS_SCRIPT_EXPORT(n:SetMotion)
		void setMotion(D6JointAxis axis, D6JointMotion motion);

		/** @copydoc D6Joint::getTwist */
		BS_SCRIPT_EXPORT(n:Twist,pr:getter)
		Radian getTwist() const;

		/** @copydoc D6Joint::getSwingY */
		BS_SCRIPT_EXPORT(n:SwingY,pr:getter)
		Radian getSwingY() const;

		/** @copydoc D6Joint::getSwingZ */
		BS_SCRIPT_EXPORT(n:SwingZ,pr:getter)
		Radian getSwingZ() const;

		/** @copydoc D6Joint::getLimitLinear */
		BS_SCRIPT_EXPORT(n:LimitLinear,pr:getter)
		LimitLinear getLimitLinear() const;

		/** @copydoc D6Joint::setLimitLinear */
		BS_SCRIPT_EXPORT(n:LimitLinear,pr:setter)
		void setLimitLinear(const LimitLinear& limit);

		/** @copydoc D6Joint::getLimitTwist */
		BS_SCRIPT_EXPORT(n:LimitTwist,pr:getter)
		LimitAngularRange getLimitTwist() const;

		/** @copydoc D6Joint::setLimitTwist */
		BS_SCRIPT_EXPORT(n:LimitTwist,pr:setter)
		void setLimitTwist(const LimitAngularRange& limit);

		/** @copydoc D6Joint::getLimitSwing */
		BS_SCRIPT_EXPORT(n:LimitSwing,pr:getter)
		LimitConeRange getLimitSwing() const;

		/** @copydoc D6Joint::setLimitSwing */
		BS_SCRIPT_EXPORT(n:LimitSwing,pr:setter)
		void setLimitSwing(const LimitConeRange& limit);

		/** @copydoc D6Joint::getDrive */
		BS_SCRIPT_EXPORT(n:GetDrive)
		D6JointDrive getDrive(D6JointDriveType type) const;

		/** @copydoc D6Joint::setDrive */
		BS_SCRIPT_EXPORT(n:SetDrive)
		void setDrive(D6JointDriveType type, const D6JointDrive& drive);

		/** @copydoc D6Joint::getDrivePosition */
		BS_SCRIPT_EXPORT(n:DrivePosition,pr:getter)
		Vector3 getDrivePosition() const;

		/** @copydoc D6Joint::getDriveRotation */
		BS_SCRIPT_EXPORT(n:DriveRotation,pr:getter)
		Quaternion getDriveRotation() const;

		/** @copydoc D6Joint::setDriveTransform */
		BS_SCRIPT_EXPORT(n:SetDriveTransform)
		void setDriveTransform(const Vector3& position, const Quaternion& rotation);

		/** @copydoc D6Joint::getDriveLinearVelocity */
		BS_SCRIPT_EXPORT(n:DriveLinearVelocity,pr:getter)
		Vector3 getDriveLinearVelocity() const;

		/** @copydoc D6Joint::getDriveAngularVelocity */
		BS_SCRIPT_EXPORT(n:DriveAngularVelocity,pr:getter)
		Vector3 getDriveAngularVelocity() const;

		/** @copydoc D6Joint::setDriveVelocity */
		BS_SCRIPT_EXPORT(n:SetDriveVelocity)
		void setDriveVelocity(const Vector3& linear, const Vector3& angular);

		/** @name Internal
		 *  @{
		 */

		/**	Returns the D6 joint that this component wraps. */
		D6Joint* _getInternal() const { return static_cast<D6Joint*>(mInternal.get()); }

		/** @} */

		/************************************************************************/
		/* 						COMPONENT OVERRIDES                      		*/
		/************************************************************************/
	protected:
		friend class SceneObject;

		/** @copydoc CJoint::createInternal */
		SPtr<Joint> createInternal() override;

		D6_JOINT_DESC mDesc;

		/************************************************************************/
		/* 								RTTI		                     		*/
		/************************************************************************/
	public:
		friend class CD6JointRTTI;
		static RTTITypeBase* getRTTIStatic();
		RTTITypeBase* getRTTI() const override;

	protected:
		CD6Joint(); // Serialization only
	 };

	 /** @} */
}