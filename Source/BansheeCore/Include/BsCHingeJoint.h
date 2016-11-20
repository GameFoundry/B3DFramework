//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsCorePrerequisites.h"
#include "BsHingeJoint.h"
#include "BsCJoint.h"

namespace bs 
{
	/** @addtogroup Components-Core
	 *  @{
	 */

	/**
	 * @copydoc	HingeJoint
	 *
	 * Wraps HingeJoint as a Component.
	 */
    class BS_CORE_EXPORT CHingeJoint : public CJoint
    {
    public:
		CHingeJoint(const HSceneObject& parent);

		/** @copydoc HingeJoint::getAngle */
		inline Radian getAngle() const;

		/** @copydoc HingeJoint::getSpeed */
		inline float getSpeed() const;

		/** @copydoc HingeJoint::getLimit */
		inline LimitAngularRange getLimit() const;

		/** @copydoc HingeJoint::setLimit */
		inline void setLimit(const LimitAngularRange& limit);

		/** @copydoc HingeJoint::getDrive */
		inline HingeJoint::Drive getDrive() const;

		/** @copydoc HingeJoint::setDrive */
		inline void setDrive(const HingeJoint::Drive& drive);

		/** @copydoc HingeJoint::setFlag */
		inline void setFlag(HingeJoint::Flag flag, bool enabled);

		/** @copydoc HingeJoint::hasFlag */
		inline bool hasFlag(HingeJoint::Flag flag) const;

		/** @name Internal
		 *  @{
		 */

	    /**	Returns the hinge joint that this component wraps. */
		HingeJoint* _getInternal() const { return static_cast<HingeJoint*>(mInternal.get()); }

		/** @} */

		/************************************************************************/
		/* 						COMPONENT OVERRIDES                      		*/
		/************************************************************************/
	protected:
		friend class SceneObject;

		/** @copydoc CJoint::createInternal */
		SPtr<Joint> createInternal() override;

		HINGE_JOINT_DESC mDesc;

		/************************************************************************/
		/* 								RTTI		                     		*/
		/************************************************************************/
	public:
		friend class CHingeJointRTTI;
		static RTTITypeBase* getRTTIStatic();
		RTTITypeBase* getRTTI() const override;

	protected:
		CHingeJoint(); // Serialization only
     };

	 /** @} */
}