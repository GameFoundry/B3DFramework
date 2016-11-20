//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsPhysXPrerequisites.h"
#include "BsHingeJoint.h"
#include "PxPhysics.h"
#include "extensions\PxRevoluteJoint.h"

namespace bs
{
	/** @addtogroup PhysX
	 *  @{
	 */

	/** PhysX implementation of a HingeJoint. */
	class BS_PHYSX_EXPORT PhysXHingeJoint : public HingeJoint
	{
	public:
		PhysXHingeJoint(physx::PxPhysics* physx, const HINGE_JOINT_DESC& desc);
		~PhysXHingeJoint();

		/** @copydoc HingeJoint::getAngle */
		Radian getAngle() const override;

		/** @copydoc HingeJoint::getSpeed */
		float getSpeed() const override;

		/** @copydoc HingeJoint::getLimit */
		LimitAngularRange getLimit() const override;

		/** @copydoc HingeJoint::setLimit */
		void setLimit(const LimitAngularRange& limit) override;

		/** @copydoc HingeJoint::getDrive */
		Drive getDrive() const override;

		/** @copydoc HingeJoint::setDrive */
		void setDrive(const Drive& drive) override;

		/** @copydoc HingeJoint::setFlag */
		void setFlag(Flag flag, bool enabled) override;

		/** @copydoc HingeJoint::hasFlag */
		bool hasFlag(Flag flag) const override;

	private:
		/** Returns the internal PhysX representation of the hinge (revolute) joint. */
		inline physx::PxRevoluteJoint* getInternal() const;
	};

	/** @} */
}