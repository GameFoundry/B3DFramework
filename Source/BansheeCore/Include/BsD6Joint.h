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

	struct D6_JOINT_DESC;

	/** 
	 * Represents the most customizable type of joint. This joint type can be used to create all other built-in joint 
	 * types, and to design your own custom ones, but is less intuitive to use. Allows a specification of a linear 
	 * constraint (for example for slider), twist constraint (rotating around X) and swing constraint (rotating around Y and
	 * Z). It also allows you to constrain limits to only specific axes or completely lock specific axes.
	 */
	class BS_CORE_EXPORT D6Joint : public Joint
	{
	public:
		/** Specifies axes that the D6 joint can constrain motion on. */
		enum class Axis
		{
			X, /**< Movement on the X axis. */
			Y, /**< Movement on the Y axis. */
			Z, /**< Movement on the Z axis. */
			Twist, /**< Rotation around the X axis. */
			SwingY, /**< Rotation around the Y axis. */
			SwingZ, /**< Rotation around the Z axis. */
			Count
		};

		/** Specifies type of constraint placed on a specific axis. */
		enum class Motion
		{
			Locked, /**< Axis is immovable. */
			Limited, /**< Axis will be constrained by the specified limits. */
			Free, /**< Axis will not be constrained. */
			Count
		};

		/** Type of drives that can be used for moving or rotating bodies attached to the joint. */
		enum class DriveType
		{
			X, /**< Linear movement on the X axis using the linear drive model. */
			Y, /**< Linear movement on the Y axis using the linear drive model. */
			Z, /**< Linear movement on the Z axis using the linear drive model. */
			/**
			 * Rotation around the Y axis using the twist/swing angular drive model. Should not be used together with 
			 * SLERP mode. 
			 */
			Swing, 
			/** 
			 * Rotation around the Z axis using the twist/swing angular drive model. Should not be used together with 
			 * SLERP mode. 
			 */
			Twist,
			/** 
			 * Rotation using spherical linear interpolation. Uses the SLERP angular drive mode which performs rotation
			 * by interpolating the quaternion values directly over the shortest path (applies to all three axes, which
			 * they all must be unlocked).
			 */
			SLERP, 
			Count
		};

		/** 
		 * Specifies parameters for a drive that will attempt to move the joint bodies to the specified drive position and
		 * velocity.
		 */
		struct Drive
		{
			bool operator==(const Drive& other) const
			{
				return stiffness == other.stiffness && damping == other.damping && forceLimit == other.forceLimit &&
					acceleration == other.acceleration;
			}

			/** Spring strength. Force proportional to the position error. */
			float stiffness = 0.0f;

			/** Damping strength. Force propertional to the velocity error. */
			float damping = 0.0f;

			/** Maximum force the drive can apply. */
			float forceLimit = FLT_MAX;

			/** 
			 * If true the drive will generate acceleration instead of forces. Acceleration drives are easier to tune as
			 * they account for the masses of the actors to which the joint is attached.
			 */
			bool acceleration = false;
		};

	public:
		D6Joint(const D6_JOINT_DESC& desc) { }
		virtual ~D6Joint() { }

		/** 
		 * Returns motion constraint for the specified axis. 
		 *
		 * @see	setMotion
		 */
		virtual Motion getMotion(Axis axis) const = 0;

		/** 
		 * Allows you to constrain motion of the specified axis. Be aware that when setting drives for a specific axis
		 * you must also take care not to constrain its motion in a conflicting way (for example you cannot add a drive that
		 * moves the joint on X axis, and then lock the X axis).
		 *
		 * Unlocking translations degrees of freedom allows the bodies to move along the subset of the unlocked axes.
		 * (for example unlocking just one translational axis is the equivalent of a slider joint.)
		 *
		 * Angular degrees of freedom are partitioned as twist (around X axis) and swing (around Y and Z axes). Different
		 * effects can be achieves by unlocking their various combinations: 
		 *  - If a single degree of angular freedom is unlocked it should be the twist degree as it has extra options for
		 *    that case (for example for a hinge joint).
		 *  - If both swing degrees are unlocked but twist is locked the result is a zero-twist joint. 
		 *  - If one swing and one twist degree of freedom are unlocked the result is a zero-swing joint (for example an arm
		 *    attached at the elbow)
		 *  - If all angular degrees of freedom are unlocked the result is the same as the spherical joint.
		 */
		virtual void setMotion(Axis axis, Motion motion) = 0;

		/** Returns the current rotation of the joint around the X axis. */
		virtual Radian getTwist() const = 0;

		/** Returns the current rotation of the joint around the Y axis. */
		virtual Radian getSwingY() const = 0;

		/** Returns the current rotation of the joint around the Z axis. */
		virtual Radian getSwingZ() const = 0;

		/** Returns the linear limit used for constraining translation degrees of freedom. */
		virtual LimitLinear getLimitLinear() const = 0;

		/** Sets the linear limit used for constraining translation degrees of freedom. */
		virtual void setLimitLinear(const LimitLinear& limit) = 0;
		
		/** Returns the angular limit used for constraining the twist (rotation around X) degree of freedom. */
		virtual LimitAngularRange getLimitTwist() const = 0;

		/** Sets the angular limit used for constraining the twist (rotation around X) degree of freedom. */
		virtual void setLimitTwist(const LimitAngularRange& limit) = 0;

		/** Returns the cone limit used for constraining the swing (rotation around Y and Z) degree of freedom. */
		virtual LimitConeRange getLimitSwing() const = 0;

		/** Sets the cone limit used for constraining the swing (rotation around Y and Z) degree of freedom. */
		virtual void setLimitSwing(const LimitConeRange& limit) = 0;
	
		/**
		 * Returns a drive that will attempt to move the specified degree(s) of freedom to the wanted position and velocity. 
		 */
		virtual Drive getDrive(DriveType type) const = 0;

		/**
		 * Sets a drive that will attempt to move the specified degree(s) of freedom to the wanted position and velocity. 
		 */
		virtual void setDrive(DriveType type, const Drive& drive) = 0;

		/** Returns the drive's target position relative to the joint's first body. */
		virtual Vector3 getDrivePosition() const = 0;

		/** Returns the drive's target rotation relative to the joint's first body. */
		virtual Quaternion getDriveRotation() const = 0;

		/** Sets the drive's target position and rotation relative to the joint's first body. */
		virtual void setDriveTransform(const Vector3& position, const Quaternion& rotation) = 0;

		/** Returns the drive's target linear velocity. */
		virtual Vector3 getDriveLinearVelocity() const = 0;

		/** Returns the drive's target angular velocity. */
		virtual Vector3 getDriveAngularVelocity() const = 0;

		/** Sets the drive's target linear and angular velocities. */
		virtual void setDriveVelocity(const Vector3& linear, const Vector3& angular) = 0;

		/** Creates a new D6 joint. */
		static SPtr<D6Joint> create(const D6_JOINT_DESC& desc);
	};

	/** Structure used for initializing a new D6Joint. */
	struct D6_JOINT_DESC : JOINT_DESC
	{
		D6_JOINT_DESC() { }

		D6Joint::Motion motion[(UINT32)D6Joint::Axis::Count];
		D6Joint::Drive drive[(UINT32)D6Joint::DriveType::Count];
		LimitLinear limitLinear;
		LimitAngularRange limitTwist;
		LimitConeRange limitSwing;
		Vector3 drivePosition = Vector3::ZERO;
		Quaternion driveRotation = Quaternion::IDENTITY;
		Vector3 driveLinearVelocity = Vector3::ZERO;
		Vector3 driveAngularVelocity = Vector3::ZERO;
	};

	/** @} */
}