//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
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
		Radian GetAngle() const;

		/** @copydoc HingeJoint::getSpeed */
		BS_SCRIPT_EXPORT(n:Speed,pr:getter)
		float GetSpeed() const;

		/** @copydoc HingeJoint::getLimit */
		BS_SCRIPT_EXPORT(n:Limit,pr:getter)
		LimitAngularRange GetLimit() const;

		/** @copydoc HingeJoint::setLimit */
		BS_SCRIPT_EXPORT(n:Limit,pr:setter)
		void SetLimit(const LimitAngularRange& limit);

		/** @copydoc HingeJoint::getDrive */
		BS_SCRIPT_EXPORT(n:Drive,pr:getter)
		HingeJointDrive GetDrive() const;

		/** @copydoc HingeJoint::setDrive */
		BS_SCRIPT_EXPORT(n:Drive,pr:setter)
		void SetDrive(const HingeJointDrive& drive);

		/** @copydoc HingeJoint::setFlag */
		BS_SCRIPT_EXPORT(n:SetFlag)
		void SetFlag(HingeJointFlag flag, bool enabled);

		/** @copydoc HingeJoint::hasFlag */
		BS_SCRIPT_EXPORT(n:HasFlag)
		bool HasFlag(HingeJointFlag flag) const;

		/** @name Internal
		 *  @{
		 */

		/**	Returns the hinge joint that this component wraps. */
		HingeJoint* _getInternal() const { return static_cast<HingeJoint*>(mInternal.Get()); }

		/** @} */

		/************************************************************************/
		/* 						COMPONENT OVERRIDES                      		*/
		/************************************************************************/
	protected:
		friend class SceneObject;

		/** @copydoc CJoint::createInternal */
		SPtr<Joint> CreateInternal() override;

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
