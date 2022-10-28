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
	class BS_CORE_EXPORT BS_SCRIPT_EXPORT(DocumentationGroup(Physics), ExportName(FixedJoint)) CFixedJoint : public CJoint
	{
	public:
		CFixedJoint(const HSceneObject& parent);

		/** @name Internal
		 *  @{
		 */

		/**	Returns the fixed joint that this component wraps. */
		FixedJoint* GetInternalInternal() const { return static_cast<FixedJoint*>(mInternal.get()); }

		/** @} */

		/************************************************************************/
		/* 						COMPONENT OVERRIDES                      		*/
		/************************************************************************/
	protected:
		friend class SceneObject;

		SPtr<Joint> CreateInternal() override;
		void GetLocalTransform(JointBody body, Vector3& position, Quaternion& rotation) override;

		FIXED_JOINT_DESC mDesc;

		/************************************************************************/
		/* 								RTTI		                     		*/
		/************************************************************************/
	public:
		friend class CFixedJointRTTI;
		static RTTITypeBase* GetRttiStatic();
		RTTITypeBase* GetRtti() const;

	protected:
		CFixedJoint(); // Serialization only
	};

	/** @} */
} // namespace bs
