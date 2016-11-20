//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsCorePrerequisites.h"
#include "BsSphericalJoint.h"
#include "BsCJoint.h"

namespace bs 
{
	/** @addtogroup Components-Core
	 *  @{
	 */

	/**
	 * @copydoc	SphericalJoint
	 *
	 * Wraps SphericalJoint as a Component.
	 */
    class BS_CORE_EXPORT CSphericalJoint : public CJoint
    {
    public:
		CSphericalJoint(const HSceneObject& parent);

		/** @copydoc SphericalJoint::getLimit */
		inline LimitConeRange getLimit() const;

		/** @copydoc SphericalJoint::setLimit */
		inline void setLimit(const LimitConeRange& limit);

		/** @copydoc SphericalJoint::setFlag */
		inline void setFlag(SphericalJoint::Flag flag, bool enabled);

		/** @copydoc SphericalJoint::hasFlag */
		inline bool hasFlag(SphericalJoint::Flag flag) const;

		/** @name Internal
		 *  @{
		 */

	    /**	Returns the spherical joint that this component wraps. */
		SphericalJoint* _getInternal() const { return static_cast<SphericalJoint*>(mInternal.get()); }

		/** @} */

		/************************************************************************/
		/* 						COMPONENT OVERRIDES                      		*/
		/************************************************************************/
	protected:
		friend class SceneObject;

		/** @copydoc CJoint::createInternal */
		SPtr<Joint> createInternal() override;

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