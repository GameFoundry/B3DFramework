//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsCorePrerequisites.h"
#include "Physics/BsDistanceJoint.h"
#include "Components/BsCJoint.h"

namespace bs
{
	/** @addtogroup Components-Core
	 *  @{
	 */

	/**
	 * @copydoc	DistanceJoint
	 *
	 * @note	Wraps DistanceJoint as a Component.
	 */
	class BS_CORE_EXPORT BS_SCRIPT_EXPORT(m:Physics,n:DistanceJoint) CDistanceJoint : public CJoint
	{
	public:
		CDistanceJoint(const HSceneObject& parent);

		/** @copydoc DistanceJoint::getDistance */
		BS_SCRIPT_EXPORT(n:Distance,pr:getter)
		float GetDistance() const;

		/** @copydoc DistanceJoint::getMinDistance */
		BS_SCRIPT_EXPORT(n:MinDistance,pr:getter)
		float GetMinDistance() const;

		/** @copydoc DistanceJoint::setMinDistance */
		BS_SCRIPT_EXPORT(n:MinDistance,pr:setter)
		void SetMinDistance(float value);

		/** @copydoc DistanceJoint::getMaxDistance */
		BS_SCRIPT_EXPORT(n:MaxDistance,pr:getter)
		float GetMaxDistance() const;

		/** @copydoc DistanceJoint::setMaxDistance */
		BS_SCRIPT_EXPORT(n:MaxDistance,pr:setter)
		void SetMaxDistance(float value);

		/** @copydoc DistanceJoint::getTolerance */
		BS_SCRIPT_EXPORT(n:Tolerance,pr:getter)
		float GetTolerance() const;

		/** @copydoc DistanceJoint::setTolerance */
		BS_SCRIPT_EXPORT(n:Tolerance,pr:setter)
		void SetTolerance(float value);

		/** @copydoc DistanceJoint::getSpring */
		BS_SCRIPT_EXPORT(n:Spring,pr:getter)
		Spring GetSpring() const;

		/** @copydoc DistanceJoint::setSpring */
		BS_SCRIPT_EXPORT(n:Spring,pr:setter)
		void SetSpring(const Spring& value);

		/** @copydoc DistanceJoint::setFlag */
		BS_SCRIPT_EXPORT(n:SetFlag)
		void SetFlag(DistanceJointFlag flag, bool enabled);

		/** @copydoc DistanceJoint::hasFlag */
		BS_SCRIPT_EXPORT(n:HasFlag)
		bool HasFlag(DistanceJointFlag flag) const;

		/** @name Internal
		 *  @{
		 */

		/**	Returns the distance joint that this component wraps. */
		DistanceJoint* _getInternal() const { return static_cast<DistanceJoint*>(mInternal.Get()); }

		/** @} */

		/************************************************************************/
		/* 						COMPONENT OVERRIDES                      		*/
		/************************************************************************/
	protected:
		friend class SceneObject;

		/** @copydoc CJoint::createInternal */
		SPtr<Joint> CreateInternal() override;

		DISTANCE_JOINT_DESC mDesc;

		/************************************************************************/
		/* 								RTTI		                     		*/
		/************************************************************************/
	public:
		friend class CDistanceJointRTTI;
		static RTTITypeBase* getRTTIStatic();
		RTTITypeBase* getRTTI() const override;

	protected:
		CDistanceJoint(); // Serialization only
	};

	 /** @} */
}
