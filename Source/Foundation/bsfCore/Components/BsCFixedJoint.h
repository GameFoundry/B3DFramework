//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsCorePrerequisites.h"
#include "Physics/BsFixedJoint.h"
#include "Components/BsCJoint.h"
#include "Physics/BsJoint.h"

namespace bs
{
	/** @addtogroup Components-Core
	 *  @{
	 */

	/**
	 * @copydoc	FixedJoint
	 *
	 * @note	Wraps FixedJoint as a Component.
	 */
	class BS_CORE_EXPORT BS_SCRIPT_EXPORT(m:Physics,n:FixedJoint) CFixedJoint : public CJoint
	{
	public:
		CFixedJoint(const HSceneObject& parent);

		/** @name Internal
		 *  @{
		 */

		/**	Returns the fixed joint that this component wraps. */
		FixedJoint* _getInternal() const { return static_cast<FixedJoint*>(mInternal.Get()); }

		/** @} */

		/************************************************************************/
		/* 						COMPONENT OVERRIDES                      		*/
		/************************************************************************/
	protected:
		friend class SceneObject;

		/** @copydoc CJoint::createInternal */
		SPtr<Joint> CreateInternal() override;

		/** @copydoc CJoint::getLocalTransform */
		void GetLocalTransform(JointBody body, Vector3& position, Quaternion& rotation) override;

		FIXED_JOINT_DESC mDesc;

		/************************************************************************/
		/* 								RTTI		                     		*/
		/************************************************************************/
	public:
		friend class CFixedJointRTTI;
		static RTTITypeBase* getRTTIStatic();
		RTTITypeBase* getRTTI() const override;

	protected:
		CFixedJoint(); // Serialization only
	};

	 /** @} */
}
