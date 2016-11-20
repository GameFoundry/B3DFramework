//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsCorePrerequisites.h"
#include "BsDistanceJoint.h"
#include "BsCJoint.h"

namespace bs 
{
	/** @addtogroup Components-Core
	 *  @{
	 */

	/**
	 * @copydoc	DistanceJoint
	 *
	 * Wraps DistanceJoint as a Component.
	 */
    class BS_CORE_EXPORT CDistanceJoint : public CJoint
    {
    public:
		CDistanceJoint(const HSceneObject& parent);

		/** @copydoc DistanceJoint::getDistance */
		inline float getDistance() const;

		/** @copydoc DistanceJoint::getMinDistance */
		inline float getMinDistance() const;

		/** @copydoc DistanceJoint::setMinDistance */
		inline void setMinDistance(float value);

		/** @copydoc DistanceJoint::getMaxDistance */
		inline float getMaxDistance() const;

		/** @copydoc DistanceJoint::setMaxDistance */
		inline void setMaxDistance(float value);

		/** @copydoc DistanceJoint::getTolerance */
		inline float getTolerance() const;

		/** @copydoc DistanceJoint::setTolerance */
		inline void setTolerance(float value);

		/** @copydoc DistanceJoint::getSpring */
		inline Spring getSpring() const;

		/** @copydoc DistanceJoint::setSpring */
		inline void setSpring(const Spring& value);

		/** @copydoc DistanceJoint::setFlag */
		inline void setFlag(DistanceJoint::Flag flag, bool enabled);

		/** @copydoc DistanceJoint::hasFlag */
		inline bool hasFlag(DistanceJoint::Flag flag) const;

		/** @name Internal
		 *  @{
		 */

	    /**	Returns the distance joint that this component wraps. */
		DistanceJoint* _getInternal() const { return static_cast<DistanceJoint*>(mInternal.get()); }

		/** @} */

		/************************************************************************/
		/* 						COMPONENT OVERRIDES                      		*/
		/************************************************************************/
	protected:
		friend class SceneObject;

		/** @copydoc CJoint::createInternal */
		SPtr<Joint> createInternal() override;

		DISTANCE_JOINT_DESC mDesc;

		/************************************************************************/
		/* 								RTTI		                     		*/
		/************************************************************************/
	public:
		friend class CDistanceJointRTTI;
		static RTTITypeBase* getRTTIStatic();
		RTTITypeBase* getRTTI() const override;

	protected:
		CDistanceJoint();// Serialization only
     };

	 /** @} */
}