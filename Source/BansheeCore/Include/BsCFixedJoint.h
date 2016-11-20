//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsCorePrerequisites.h"
#include "BsFixedJoint.h"
#include "BsCJoint.h"
#include "BsJoint.h"

namespace bs 
{
	/** @addtogroup Components-Core
	 *  @{
	 */

	/**
	 * @copydoc	FixedJoint
	 *
	 * Wraps FixedJoint as a Component.
	 */
    class BS_CORE_EXPORT CFixedJoint : public CJoint
    {
    public:
		CFixedJoint(const HSceneObject& parent);

		/** @name Internal
		 *  @{
		 */

	    /**	Returns the fixed joint that this component wraps. */
		FixedJoint* _getInternal() const { return static_cast<FixedJoint*>(mInternal.get()); }

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