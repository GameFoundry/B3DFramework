//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsPhysXPrerequisites.h"
#include "Physics/BsDistanceJoint.h"
#include "PxPhysics.h"
#include "extensions/PxDistanceJoint.h"

namespace bs
{
	/** @addtogroup PhysX
	 *  @{
	 */

	/** PhysX implementation of a DistanceJoint */
	class PhysXDistanceJoint : public DistanceJoint
	{
	public:
		PhysXDistanceJoint(physx::PxPhysics* physx, const DISTANCE_JOINT_DESC& desc);
		~PhysXDistanceJoint();

		/** @copydoc DistanceJoint::getDistance */
		float GetDistance() const;

		/** @copydoc DistanceJoint::getMinDistance */
		float GetMinDistance() const;

		/** @copydoc DistanceJoint::setMinDistance */
		void SetMinDistance(float value);

		/** @copydoc DistanceJoint::getMaxDistance */
		float GetMaxDistance() const;

		/** @copydoc DistanceJoint::setMaxDistance */
		void SetMaxDistance(float value);

		/** @copydoc DistanceJoint::getTolerance */
		float GetTolerance() const;

		/** @copydoc DistanceJoint::setTolerance */
		void SetTolerance(float value);

		/** @copydoc DistanceJoint::getSpring */
		Spring GetSpring() const;

		/** @copydoc DistanceJoint::setSpring */
		void SetSpring(const Spring& value);

		/** @copydoc DistanceJoint::setFlag */
		void SetFlag(DistanceJointFlag flag, bool enabled);

		/** @copydoc DistanceJoint::hasFlag */
		bool HasFlag(DistanceJointFlag flag) const;

	private:
		/** Returns the internal PhysX representation of the distance joint. */
		inline physx::PxDistanceJoint* GetInternal() const;
	};

	/** @} */
} // namespace bs
