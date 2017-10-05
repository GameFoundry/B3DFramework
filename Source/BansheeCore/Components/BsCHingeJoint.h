//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsCorePrerequisites.h"
#include "Physics/BsHingeJoint.h"
#include "Components/BsCJoint.h"

namespace bs 
{
	/** @addtogroup Components-Core
	 *  @{
	 */

	/**
	 * @copydoc	HingeJoint
	 *
	 * @note	Wraps HingeJoint as a Component.
	 */
	class BS_CORE_EXPORT BS_SCRIPT_EXPORT(m:Physics,n:HingeJoint) CHingeJoint : public CJoint
	{
	public:
		CHingeJoint(const HSceneObject& parent);

		/** @copydoc HingeJoint::getAngle */
		BS_SCRIPT_EXPORT(n:Angle,pr:getter)
		Radian getAngle() const;

		/** @copydoc HingeJoint::getSpeed */
		BS_SCRIPT_EXPORT(n:Speed,pr:getter)
		float getSpeed() const;

		/** @copydoc HingeJoint::getLimit */
		BS_SCRIPT_EXPORT(n:Limit,pr:getter)
		LimitAngularRange getLimit() const;

		/** @copydoc HingeJoint::setLimit */
		BS_SCRIPT_EXPORT(n:Limit,pr:setter)
		void setLimit(const LimitAngularRange& limit);

		/** @copydoc HingeJoint::getDrive */
		BS_SCRIPT_EXPORT(n:Drive,pr:getter)
		HingeJointDrive getDrive() const;

		/** @copydoc HingeJoint::setDrive */
		BS_SCRIPT_EXPORT(n:Drive,pr:setter)
		void setDrive(const HingeJointDrive& drive);

		/** @copydoc HingeJoint::setFlag */
		BS_SCRIPT_EXPORT(n:SetFlag)
		void setFlag(HingeJointFlag flag, bool enabled);

		/** @copydoc HingeJoint::hasFlag */
		BS_SCRIPT_EXPORT(n:HasFlag)
		bool hasFlag(HingeJointFlag flag) const;

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