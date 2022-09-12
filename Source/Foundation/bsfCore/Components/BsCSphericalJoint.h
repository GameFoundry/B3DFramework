//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsCorePrerequisites.h"
#include "Physics/BsSphericalJoint.h"
#include "Components/BsCJoint.h"

namespace bs
{
	/** @addtogroup Components-Core
	 *  @{
	 */

	/**
	 * @copydoc	SphericalJoint
	 *
	 * @note	Wraps SphericalJoint as a Component.
	 */
	class BS_CORE_EXPORT BS_SCRIPT_EXPORT(m:Physics,n:SphericalJoint) CSphericalJoint : public CJoint
	{
	public:
		CSphericalJoint(const HSceneObject& parent);

		/** @copydoc SphericalJoint::getLimit */
		BS_SCRIPT_EXPORT(n:Limit,pr:getter)
		LimitConeRange GetLimit() const;

		/** @copydoc SphericalJoint::setLimit */
		BS_SCRIPT_EXPORT(n:Limit,pr:setter)
		void SetLimit(const LimitConeRange& limit);

		/** @copydoc SphericalJoint::setFlag */
		BS_SCRIPT_EXPORT(n:SetFlag)
		void SetFlag(SphericalJointFlag flag, bool enabled);

		/** @copydoc SphericalJoint::hasFlag */
		BS_SCRIPT_EXPORT(n:HasFlag)
		bool HasFlag(SphericalJointFlag flag) const;

		/** @name Internal
		 *  @{
		 */

		/**	Returns the spherical joint that this component wraps. */
		SphericalJoint* _getInternal() const { return static_cast<SphericalJoint*>(mInternal.Get()); }

		/** @} */

		/************************************************************************/
		/* 						COMPONENT OVERRIDES                      		*/
		/************************************************************************/
	protected:
		friend class SceneObject;

		/** @copydoc CJoint::createInternal */
		SPtr<Joint> CreateInternal() override;

		SPHERICAL_JOINT_DESC mDesc;

		/************************************************************************/
		/* 								RTTI		                     		*/
		/************************************************************************/
	public:
		friend class CSphericalJointRTTI;
		static RTTITypeBase* getRTTIStatic();
		RTTITypeBase* getRTTI() const override;

	protected:
		CSphericalJoint(); // Serialization only
	};

	 /** @} */
}
