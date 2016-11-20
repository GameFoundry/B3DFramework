//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsCorePrerequisites.h"
#include "BsJoint.h"

namespace bs
{
	/** @addtogroup Physics
	 *  @{
	 */

	struct SLIDER_JOINT_DESC;

	/** 
	 * Joint that removes all but a single translational degree of freedom. Bodies are allowed to move along a single axis. 
	 */
	class BS_CORE_EXPORT SliderJoint : public Joint
	{
	public:
		/** Flag that controls slider joint's behaviour. */
		enum class Flag
		{
			Limit = 0x1,
		};

	public:
		SliderJoint(const SLIDER_JOINT_DESC& desc) { }
		virtual ~SliderJoint() { }

		/** Returns the current position of the slider. */
		virtual float getPosition() const = 0;

		/** Returns the current speed of the slider. */
		virtual float getSpeed() const = 0;

		/** 
		 * Returns a limit that constrains the movement of the joint to a specific minimum and maximum distance. You must
		 * enable the limit flag on the joint in order for this to be recognized. 
		 *
		 * @see LimitLinearRange
		 */
		virtual LimitLinearRange getLimit() const = 0;

		/** 
		 * Sets a limit that constrains the movement of the joint to a specific minimum and maximum distance. You must
		 * enable the limit flag on the joint in order for this to be recognized. 
		 *
		 * @see LimitLinearRange
		 */
		virtual void setLimit(const LimitLinearRange& limit) = 0;

		/** Enables or disables a flag that controls the joint's behaviour. */
		virtual void setFlag(Flag flag, bool enabled) = 0;

		/** Checks is the specified flag enabled. */
		virtual bool hasFlag(Flag flag) const = 0;

		/** Creates a new spherical joint. */
		static SPtr<SliderJoint> create(const SLIDER_JOINT_DESC& desc);
	};

	/** Structure used for initializing a new SliderJoint. */
	struct SLIDER_JOINT_DESC : JOINT_DESC
	{
		LimitLinearRange limit;
		SliderJoint::Flag flag = (SliderJoint::Flag)0;
	};

	/** @} */
}