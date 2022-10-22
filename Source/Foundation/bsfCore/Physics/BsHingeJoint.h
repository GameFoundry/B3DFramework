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

	struct HINGE_JOINT_DESC;

	/** Flags that control hinge joint options. */
	enum class BS_SCRIPT_EXPORT(DocumentationGroup(Physics)) HingeJointFlag
	{
		Limit = 0x1, /**< Joint limit is enabled. */
		Drive = 0x2 /**< Joint drive is enabled. */
	};

	/** Properties of a drive that drives the joint's angular velocity towards a paricular value. */
	struct BS_SCRIPT_EXPORT(DocumentationGroup(Physics),ExportAsStruct(true)) HingeJointDrive
	{
		/** Target speed of the joint. */
		float Speed = 0.0f;

		/** Maximum torque the drive is allowed to apply .*/
		float ForceLimit = FLT_MAX;

		/** Scales the velocity of the first body, and its response to drive torque is scaled down. */
		float GearRatio = 1.0f;

		/**
		 * If the joint is moving faster than the drive's target speed, the drive will try to break. If you don't want
		 * the breaking to happen set this to true.
		 */
		bool FreeSpin = false;

		bool operator==(const HingeJointDrive& other) const
		{
			return Speed == other.Speed && ForceLimit == other.ForceLimit && GearRatio == other.GearRatio &&
				FreeSpin && other.FreeSpin;
		}
	};

	/**
	 * Hinge joint removes all but a single rotation degree of freedom from its two attached bodies (for example a door
	 * hinge).
	 */
	class BS_CORE_EXPORT HingeJoint : public Joint
	{
	public:
		HingeJoint(const HINGE_JOINT_DESC& desc) { }
		virtual ~HingeJoint() = default;

		/** Returns the current angle between the two attached bodes. */
		virtual Radian GetAngle() const = 0;

		/** Returns the current angular speed of the joint. */
		virtual float GetSpeed() const = 0;

		/** @copydoc setLimit() */
		virtual LimitAngularRange GetLimit() const = 0;

		/**
		 * Determines the limit of the joint. Limit constrains the motion to the specified angle range. You must enable the
		 * limit flag on the joint in order for this to be recognized.
		 *
		 * @see LimitAngularRange
		 */
		virtual void SetLimit(const LimitAngularRange& limit) = 0;

		/** @copydoc setDrive() */
		virtual HingeJointDrive GetDrive() const = 0;

		/**
		 * Determines the drive properties of the joint. It drives the joint's angular velocity towards a particular value.
		 * You must enable the drive flag on the joint in order for the drive to be active.
		 *
		 * @see HingeJoint::Drive
		 */
		virtual void SetDrive(const HingeJointDrive& drive) = 0;

		/** Enables or disables a flag that controls joint behaviour. */
		virtual void SetFlag(HingeJointFlag flag, bool enabled) = 0;

		/** Checks is the specified option enabled. */
		virtual bool HasFlag(HingeJointFlag flag) const = 0;

		/**
		 * Creates a new hinge joint.
		 *
		 * @param[in]	scene		Scene to which to add the joint.
		 * @param[in]	desc		Settings describing the joint.
		 */
		static SPtr<HingeJoint> Create(PhysicsScene& scene, const HINGE_JOINT_DESC& desc);
	};

	/** Structure used for initializing a new HingeJoint. */
	struct HINGE_JOINT_DESC : JOINT_DESC
	{
		HingeJointDrive Drive;
		LimitAngularRange Limit;
		HingeJointFlag Flag = (HingeJointFlag)0;
	};

	/** @} */
}
