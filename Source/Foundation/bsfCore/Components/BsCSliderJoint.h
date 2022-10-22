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
	class BS_CORE_EXPORT BS_SCRIPT_EXPORT(DocumentationGroup(Physics),n:SliderJoint) CSliderJoint : public CJoint
	{
	public:
		CSliderJoint(const HSceneObject& parent);

		/** @copydoc SliderJoint::getPosition */
		BS_SCRIPT_EXPORT(n:Position,pr:getter)
		float GetPosition() const;

		/** @copydoc SliderJoint::getSpeed */
		BS_SCRIPT_EXPORT(n:Speed,pr:getter)
		float GetSpeed() const;

		/** @copydoc SliderJoint::getLimit */
		BS_SCRIPT_EXPORT(n:Limit,pr:getter)
		LimitLinearRange GetLimit() const;

		/** @copydoc SliderJoint::setLimit */
		BS_SCRIPT_EXPORT(n:Limit,pr:setter)
		void SetLimit(const LimitLinearRange& limit);

		/** @copydoc SliderJoint::setFlag */
		BS_SCRIPT_EXPORT(n:SetFlag)
		void SetFlag(SliderJointFlag flag, bool enabled);

		/** @copydoc SliderJoint::hasFlag */
		BS_SCRIPT_EXPORT(n:HasFlag)
		bool HasFlag(SliderJointFlag flag) const;

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
		SPtr<Joint> CreateInternal() ;

		/** @copydoc CJoint::getLocalTransform */
		void GetLocalTransform(JointBody body, Vector3& position, Quaternion& rotation) ;

		SLIDER_JOINT_DESC mDesc;

		/************************************************************************/
		/* 								RTTI		                     		*/
		/************************************************************************/
	public:
		friend class CSliderJointRTTI;
		static RTTITypeBase* GetRttiStatic();
		RTTITypeBase* GetRtti() const ;

	protected:
		CSliderJoint(); // Serialization only
	};

	 /** @} */
}
