//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsCorePrerequisites.h"
#include "Physics/BsJoint.h"

namespace bs
{
	class PhysicsScene;
	/** @addtogroup Physics
	 *  @{
	 */

	struct SLIDER_JOINT_DESC;

	/** Flag that controls slider joint's behaviour. */
	enum class BS_SCRIPT_EXPORT(m:Physics) SliderJointFlag
	{
		Limit = 0x1 /**< Enables the linear range limit. */
	};

	/** 
	 * Joint that removes all but a single translational degree of freedom. Bodies are allowed to move along a single axis. 
	 */
	class BS_CORE_EXPORT SliderJoint : public Joint
	{
	public:
		SliderJoint(const SLIDER_JOINT_DESC& desc) { }
		virtual ~SliderJoint() = default;

		/** Returns the current position of the slider. */
		virtual float getPosition() const = 0;

		/** Returns the current speed of the slider. */
		virtual float getSpeed() const = 0;

		/** @copydoc setLimit() */
		virtual LimitLinearRange getLimit() const = 0;

		/** 
		 * Determines a limit that constrains the movement of the joint to a specific minimum and maximum distance. You must
		 * enable the limit flag on the joint in order for this to be recognized. 
		 *
		 * @see LimitLinearRange
		 */
		virtual void setLimit(const LimitLinearRange& limit) = 0;

		/** Enables or disables a flag that controls the joint's behaviour. */
		virtual void setFlag(SliderJointFlag flag, bool enabled) = 0;

		/** Checks is the specified flag enabled. */
		virtual bool hasFlag(SliderJointFlag flag) const = 0;

		/** 
		 * Creates a new spherical joint. 
		 * 
		 * @param[in]	scene		Scene to which to add the joint.
		 * @param[in]	desc		Settings describing the joint.
		 */
		static SPtr<SliderJoint> create(PhysicsScene& scene, const SLIDER_JOINT_DESC& desc);
	};

	/** Structure used for initializing a new SliderJoint. */
	struct SLIDER_JOINT_DESC : JOINT_DESC
	{
		LimitLinearRange limit;
		SliderJointFlag flag = (SliderJointFlag)0;
	};

	/** @} */
}