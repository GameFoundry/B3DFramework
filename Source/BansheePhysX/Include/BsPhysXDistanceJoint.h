//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsPhysXPrerequisites.h"
#include "BsDistanceJoint.h"
#include "PxPhysics.h"
#include "extensions\PxDistanceJoint.h"

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
		float getDistance() const override;

		/** @copydoc DistanceJoint::getMinDistance */
		float getMinDistance() const override;

		/** @copydoc DistanceJoint::setMinDistance */
		void setMinDistance(float value) override;

		/** @copydoc DistanceJoint::getMaxDistance */
		float getMaxDistance() const override;

		/** @copydoc DistanceJoint::setMaxDistance */
		void setMaxDistance(float value) override;

		/** @copydoc DistanceJoint::getTolerance */
		float getTolerance() const override;

		/** @copydoc DistanceJoint::setTolerance */
		void setTolerance(float value) override;

		/** @copydoc DistanceJoint::getSpring */
		Spring getSpring() const override;

		/** @copydoc DistanceJoint::setSpring */
		void setSpring(const Spring& value) override;

		/** @copydoc DistanceJoint::setFlag */
		void setFlag(Flag flag, bool enabled) override;

		/** @copydoc DistanceJoint::hasFlag */
		bool hasFlag(Flag flag) const override;

	private:
		/** Returns the internal PhysX representation of the distance joint. */
		inline physx::PxDistanceJoint* getInternal() const;
	};

	/** @} */
}