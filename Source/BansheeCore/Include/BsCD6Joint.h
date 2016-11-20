//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsCorePrerequisites.h"
#include "BsD6Joint.h"
#include "BsCJoint.h"

namespace bs 
{
	/** @addtogroup Components-Core
	 *  @{
	 */

	/**
	 * @copydoc	D6Joint
	 *
	 * Wraps D6Joint as a Component.
	 */
    class BS_CORE_EXPORT CD6Joint : public CJoint
    {
    public:
		CD6Joint(const HSceneObject& parent);

		/** @copydoc D6Joint::getMotion */
		inline D6Joint::Motion getMotion(D6Joint::Axis axis) const;

		/** @copydoc D6Joint::setMotion */
		inline void setMotion(D6Joint::Axis axis, D6Joint::Motion motion);

		/** @copydoc D6Joint::getTwist */
		inline Radian getTwist() const;

		/** @copydoc D6Joint::getSwingY */
		inline Radian getSwingY() const;

		/** @copydoc D6Joint::getSwingZ */
		inline Radian getSwingZ() const;

		/** @copydoc D6Joint::getLimitLinear */
		inline LimitLinear getLimitLinear() const;

		/** @copydoc D6Joint::setLimitLinear */
		inline void setLimitLinear(const LimitLinear& limit);

		/** @copydoc D6Joint::getLimitTwist */
		inline LimitAngularRange getLimitTwist() const;

		/** @copydoc D6Joint::setLimitTwist */
		inline void setLimitTwist(const LimitAngularRange& limit);

		/** @copydoc D6Joint::getLimitSwing */
		inline LimitConeRange getLimitSwing() const;

		/** @copydoc D6Joint::setLimitSwing */
		inline void setLimitSwing(const LimitConeRange& limit);

		/** @copydoc D6Joint::getDrive */
		inline D6Joint::Drive getDrive(D6Joint::DriveType type) const;

		/** @copydoc D6Joint::setDrive */
		inline void setDrive(D6Joint::DriveType type, const D6Joint::Drive& drive);

		/** @copydoc D6Joint::getDrivePosition */
		inline Vector3 getDrivePosition() const;

		/** @copydoc D6Joint::getDriveRotation */
		inline Quaternion getDriveRotation() const;

		/** @copydoc D6Joint::setDriveTransform */
		inline void setDriveTransform(const Vector3& position, const Quaternion& rotation);

		/** @copydoc D6Joint::getDriveLinearVelocity */
		inline Vector3 getDriveLinearVelocity() const;

		/** @copydoc D6Joint::getDriveAngularVelocity */
		inline Vector3 getDriveAngularVelocity() const;

		/** @copydoc D6Joint::setDriveVelocity */
		inline void setDriveVelocity(const Vector3& linear, const Vector3& angular);

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