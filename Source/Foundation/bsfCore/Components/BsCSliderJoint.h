//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsCorePrerequisites.h"
#include "Physics/BsSliderJoint.h"
#include "Components/BsCJoint.h"

namespace bs
{
	/** @addtogroup Components-Core
	 *  @{
	 */

	/**
	 * @copydoc	SliderJoint
	 *
	 * @note	Wraps SliderJoint as a Component.
	 */
	class BS_CORE_EXPORT BS_SCRIPT_EXPORT(m:Physics,n:SliderJoint) CSliderJoint : public CJoint
	{
	public:
		CSliderJoint(const HSceneObject& parent);

		/** @copydoc SliderJoint::getPosition */
		BS_SCRIPT_EXPORT(n:Position,pr:getter)
		float getPosition() const;

		/** @copydoc SliderJoint::getSpeed */
		BS_SCRIPT_EXPORT(n:Speed,pr:getter)
		float getSpeed() const;

		/** @copydoc SliderJoint::getLimit */
		BS_SCRIPT_EXPORT(n:Limit,pr:getter)
		LimitLinearRange getLimit() const;

		/** @copydoc SliderJoint::setLimit */
		BS_SCRIPT_EXPORT(n:Limit,pr:setter)
		void setLimit(const LimitLinearRange& limit);

		/** @copydoc SliderJoint::setFlag */
		BS_SCRIPT_EXPORT(n:SetFlag)
		void setFlag(SliderJointFlag flag, bool enabled);

		/** @copydoc SliderJoint::hasFlag */
		BS_SCRIPT_EXPORT(n:HasFlag)
		bool hasFlag(SliderJointFlag flag) const;

		/** @name Internal
		 *  @{
		 */

		/**	Returns the slider joint that this component wraps. */
		SliderJoint* GetInternalInternal() const { return static_cast<SliderJoint*>(mInternal.get()); }

		/** @} */

		/************************************************************************/
		/* 						COMPONENT OVERRIDES                      		*/
		/************************************************************************/
	protected:
		friend class SceneObject;

		/** @copydoc CJoint::createInternal */
		SPtr<Joint> createInternal() override;

		/** @copydoc CJoint::getLocalTransform */
		void getLocalTransform(JointBody body, Vector3& position, Quaternion& rotation) override;

		SLIDER_JOINT_DESC mDesc;

		/************************************************************************/
		/* 								RTTI		                     		*/
		/************************************************************************/
	public:
		friend class CSliderJointRTTI;
		static RTTITypeBase* getRTTIStatic();
		RTTITypeBase* getRTTI() const override;

	protected:
		CSliderJoint(); // Serialization only
	};

	 /** @} */
}
