//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsPhysXPrerequisites.h"
#include "BsSliderJoint.h"
#include "PxPhysics.h"
#include "extensions\PxPrismaticJoint.h"

namespace bs
{
	/** @addtogroup PhysX
	 *  @{
	 */

	/** PhysX implementation of a SliderJoint. */
	class PhysXSliderJoint : public SliderJoint
	{
	public:
		PhysXSliderJoint(physx::PxPhysics* physx, const SLIDER_JOINT_DESC& desc);
		~PhysXSliderJoint();

		/** @copydoc SliderJoint::getPosition */
		float getPosition() const override;

		/** @copydoc SliderJoint::getSpeed */
		float getSpeed() const override;

		/** @copydoc SliderJoint::getLimit */
		LimitLinearRange getLimit() const override;

		/** @copydoc SliderJoint::setLimit */
		void setLimit(const LimitLinearRange& limit) override;

		/** @copydoc SliderJoint::setFlag */
		void setFlag(Flag flag, bool enabled) override;

		/** @copydoc SliderJoint::hasFlag */
		bool hasFlag(Flag flag) const override;

	private:
		/** Returns the internal PhysX representation of the slider (prismatic) joint. */
		inline physx::PxPrismaticJoint* getInternal() const;
	};

	/** @} */
}