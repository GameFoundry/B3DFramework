//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsPhysXPrerequisites.h"
#include "Physics/BsSliderJoint.h"
#include "PxPhysics.h"
#include "extensions/PxPrismaticJoint.h"

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

		float GetPosition() const override;
		float GetSpeed() const override;
		LimitLinearRange GetLimit() const override;
		void SetLimit(const LimitLinearRange& limit) override;
		void SetFlag(SliderJointFlag flag, bool enabled) override;
		bool HasFlag(SliderJointFlag flag) const override;

	private:
		/** Returns the internal PhysX representation of the slider (prismatic) joint. */
		inline physx::PxPrismaticJoint* GetInternal() const;
	};

	/** @} */
} // namespace bs
