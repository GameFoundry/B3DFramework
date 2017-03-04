//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsPhysXPrerequisites.h"
#include "BsSphericalJoint.h"
#include "PxPhysics.h"
#include "extensions\PxSphericalJoint.h"

namespace bs
{
	/** @addtogroup PhysX
	 *  @{
	 */

	/** PhysX implementation of a SphericalJoint. */
	class PhysXSphericalJoint : public SphericalJoint
	{
	public:
		PhysXSphericalJoint(physx::PxPhysics* physx, const SPHERICAL_JOINT_DESC& desc);
		~PhysXSphericalJoint();

		/** @copydoc SphericalJoint::getLimit */
		LimitConeRange getLimit() const override;

		/** @copydoc SphericalJoint::setLimit */
		void setLimit(const LimitConeRange& limit) override;

		/** @copydoc SphericalJoint::setFlag */
		void setFlag(Flag flag, bool enabled) override;

		/** @copydoc SphericalJoint::hasFlag */
		bool hasFlag(Flag flag) const override;

	private:
		/** Returns the internal PhysX representation of the spherical joint. */
		inline physx::PxSphericalJoint* getInternal() const;
	};

	/** @} */
}