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

	struct HINGE_JOINT_DESC;

	/**
	 * Hinge joint removes all but a single rotation degree of freedom from its two attached bodies (for example a door 
	 * hinge). 
	 */
	class BS_CORE_EXPORT HingeJoint : public Joint
	{
	public:
		/** Flags that control hinge joint options. */
		enum class Flag
		{
			Limit = 0x1, /**< Joint limit is enabled. */
			Drive = 0x2 /**< Joint drive is enabled. */
		};

		/** Properties of a drive that drives the joint's angular velocity towards a paricular value. */
		struct Drive
		{
			/** Target speed of the joint. */
			float speed = 0.0f;

			/** Maximum torque the drive is allowed to apply .*/
			float forceLimit = FLT_MAX;

			/** Scales the velocity of the first body, and its response to drive torque is scaled down. */
			float gearRatio = 1.0f;

			/** 
			 * If the joint is moving faster than the drive's target speed, the drive will try to break. If you don't want
			 * the breaking to happen set this to true.
			 */
			bool freeSpin = false;

			bool operator==(const Drive& other) const
			{
				return speed == other.speed && forceLimit == other.forceLimit && gearRatio == other.gearRatio && 
					freeSpin && other.freeSpin;
			}
		};

	public:
		HingeJoint(const HINGE_JOINT_DESC& desc) { }
		virtual ~HingeJoint() { }

		/** Returns the current angle between the two attached bodes. */
		virtual Radian getAngle() const = 0;

		/** Returns the current angular speed of the joint. */
		virtual float getSpeed() const = 0;

		/** 
		 * Returns the limit of the joint. Limit constrains the motion to the specified angle range. You must enable the
		 * limit flag on the joint in order for this to be recognized. 
		 *
		 * @see LimitAngularRange
		 */
		virtual LimitAngularRange getLimit() const = 0;

		/**
		 * Sets the limit of the joint. Limit constrains the motion to the specified angle range. You must enable the limit
		 * flag on the joint in order for this to be recognized. 
		 *
		 * @see LimitAngularRange
		 */
		virtual void setLimit(const LimitAngularRange& limit) = 0;

		/** 
		 * Returns the drive properties of the joint. It drives the joint's angular velocity towards a particular value. You
		 * must enable the drive flag on the joint in order for the drive to be active.
		 *
		 * @see HingeJoint::Drive
		 */
		virtual Drive getDrive() const = 0;

		/** 
		 * Sets the drive properties of the joint. It drives the joint's angular velocity towards a particular value. You 
		 * must enable the drive flag on the joint in order for the drive to be active.
		 *
		 * @see HingeJoint::Drive
		 */
		virtual void setDrive(const Drive& drive) = 0;

		/** Enables or disables a flag that controls joint behaviour. */
		virtual void setFlag(Flag flag, bool enabled) = 0;

		/** Checks is the specified option enabled. */
		virtual bool hasFlag(Flag flag) const = 0;

		/** Creates a new hinge joint. */
		static SPtr<HingeJoint> create(const HINGE_JOINT_DESC& desc);
	};

	/** Structure used for initializing a new HingeJoint. */
	struct HINGE_JOINT_DESC : JOINT_DESC
	{
		HingeJoint::Drive drive;
		LimitAngularRange limit;
		HingeJoint::Flag flag = (HingeJoint::Flag)0;
	};

	/** @} */
}