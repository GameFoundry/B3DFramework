//************************************ B3D Framework - Copyright 2026 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DPrerequisites.h"
#include "Components/B3DJoint.h"

namespace b3d
{
	/** @addtogroup Physics
	 *  @{
	 */

	class ID6JointImplementation;

	/** Specifies axes that the D6 joint can constrain motion on. */
	enum class B3D_SCRIPT_EXPORT(DocumentationGroup(Physics)) D6JointAxis
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
	enum class B3D_SCRIPT_EXPORT(DocumentationGroup(Physics)) D6JointMotion
	{
		Locked, /**< Axis is immovable. */
		Limited, /**< Axis will be constrained by the specified limits. */
		Free, /**< Axis will not be constrained. */
		Count
	};

	/** Type of drives that can be used for moving or rotating bodies attached to the joint. */
	enum class B3D_SCRIPT_EXPORT(DocumentationGroup(Physics)) D6JointDriveType
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
	struct B3D_EXPORT B3D_SCRIPT_EXPORT(DocumentationGroup(Physics), ExportAsStruct(true)) D6JointDrive : public IReflectable
	{
		bool operator==(const D6JointDrive& other) const
		{
			return Stiffness == other.Stiffness && Damping == other.Damping && ForceLimit == other.ForceLimit &&
				Acceleration == other.Acceleration;
		}

		/** Spring strength. Force proportional to the position error. */
		float Stiffness = 0.0f;

		/** Damping strength. Force propertional to the velocity error. */
		float Damping = 0.0f;

		/** Maximum force the drive can apply. */
		float ForceLimit = FLT_MAX;

		/**
		 * If true the drive will generate acceleration instead of forces. Acceleration drives are easier to tune as
		 * they account for the masses of the actors to which the joint is attached.
		 */
		bool Acceleration = false;

		/************************************************************************/
		/* 								RTTI		                     		*/
		/************************************************************************/
	public:
		friend class D6JointDriveRTTI;
		static RTTIType* GetRttiStatic();
		RTTIType* GetRtti() const override;
	};

	/** Structure used for initializing a new D6Joint. */
	struct D6JointCreateInformation : JointCreateInformation
	{
		D6JointCreateInformation()
		{
			Motion.Resize((u32)D6JointAxis::Count);
			Drive.Resize((u32)D6JointDriveType::Count);
		}

		TInlineArray<D6JointMotion, (u32)D6JointAxis::Count> Motion;
		TInlineArray<D6JointDrive, (u32)D6JointDriveType::Count> Drive;
		LimitLinear LimitLinear;
		LimitAngularRange LimitTwist;
		LimitConeRange LimitSwing;
		Vector3 DrivePosition = Vector3::kZero;
		Quaternion DriveRotation = Quaternion::kIdentity;
		Vector3 DriveLinearVelocity = Vector3::kZero;
		Vector3 DriveAngularVelocity = Vector3::kZero;
	};

	/**
	 * Represents the most customizable type of joint. This joint type can be used to create all other built-in joint
	 * types, and to design your own custom ones, but is less intuitive to use. Allows a specification of a linear
	 * constraint (for example for slider), twist constraint (rotating around X) and swing constraint (rotating around Y and
	 * Z). It also allows you to constrain limits to only specific axes or completely lock specific axes.
	 */
	class B3D_EXPORT B3D_SCRIPT_EXPORT(DocumentationGroup(Physics)) D6Joint : public Joint
	{
	public:
		D6Joint(const HSceneObject& parent);

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
		B3D_SCRIPT_EXPORT(ExportName(SetMotion))
		void SetMotion(D6JointAxis axis, D6JointMotion motion);

		/**
		 * Returns motion constraint for the specified axis.
		 *
		 * @see	SetMotion
		 */
		B3D_SCRIPT_EXPORT(ExportName(GetMotion))
		D6JointMotion GetMotion(D6JointAxis axis) const { return mInformation.Motion[(int)axis]; }

		/** Returns the current rotation of the joint around the X axis. */
		B3D_SCRIPT_EXPORT(ExportName(Twist), Property(Getter))
		Radian GetTwist() const;

		/** Returns the current rotation of the joint around the Y axis. */
		B3D_SCRIPT_EXPORT(ExportName(SwingY), Property(Getter))
		Radian GetSwingY() const;

		/** Returns the current rotation of the joint around the Z axis. */
		B3D_SCRIPT_EXPORT(ExportName(SwingZ), Property(Getter))
		Radian GetSwingZ() const;

		/** Determines the linear limit used for constraining translation degrees of freedom. */
		B3D_SCRIPT_EXPORT(ExportName(LimitLinear), Property(Setter))
		void SetLimitLinear(const LimitLinear& limit);

		/** @copydoc SetLimitLinear */
		B3D_SCRIPT_EXPORT(ExportName(LimitLinear), Property(Getter))
		LimitLinear GetLimitLinear() const { return mInformation.LimitLinear; }

		/** Determines the angular limit used for constraining the twist (rotation around X) degree of freedom. */
		B3D_SCRIPT_EXPORT(ExportName(LimitTwist), Property(Setter))
		void SetLimitTwist(const LimitAngularRange& limit);

		/** @copydoc SetLimitTwist */
		B3D_SCRIPT_EXPORT(ExportName(LimitTwist), Property(Getter))
		LimitAngularRange GetLimitTwist() const { return mInformation.LimitTwist; }

		/** Determines the cone limit used for constraining the swing (rotation around Y and Z) degree of freedom. */
		B3D_SCRIPT_EXPORT(ExportName(LimitSwing), Property(Setter))
		void SetLimitSwing(const LimitConeRange& limit);

		/** @copydoc SetLimitSwing */
		B3D_SCRIPT_EXPORT(ExportName(LimitSwing), Property(Getter))
		LimitConeRange GetLimitSwing() const { return mInformation.LimitSwing; }

		/**
		 * Determines a drive that will attempt to move the specified degree(s) of freedom to the wanted position and
		 * velocity.
		 */
		B3D_SCRIPT_EXPORT(ExportName(SetDrive))
		void SetDrive(D6JointDriveType type, const D6JointDrive& drive);

		/** @copydoc SetDrive */
		B3D_SCRIPT_EXPORT(ExportName(GetDrive))
		D6JointDrive GetDrive(D6JointDriveType type) const;

		/** Returns the drive's target position relative to the joint's first body. */
		B3D_SCRIPT_EXPORT(ExportName(DrivePosition), Property(Getter))
		Vector3 GetDrivePosition() const { return mInformation.DrivePosition; }

		/** Returns the drive's target rotation relative to the joint's first body. */
		B3D_SCRIPT_EXPORT(ExportName(DriveRotation), Property(Getter))
		Quaternion GetDriveRotation() const { return mInformation.DriveRotation; }

		/** Sets the drive's target position and rotation relative to the joint's first body. */
		B3D_SCRIPT_EXPORT(ExportName(SetDriveTransform))
		void SetDriveTransform(const Vector3& position, const Quaternion& rotation);

		/** Returns the drive's target linear velocity. */
		B3D_SCRIPT_EXPORT(ExportName(DriveLinearVelocity), Property(Getter))
		Vector3 GetDriveLinearVelocity() const { return mInformation.DriveLinearVelocity; }

		/** Returns the drive's target angular velocity. */
		B3D_SCRIPT_EXPORT(ExportName(DriveAngularVelocity), Property(Getter))
		Vector3 GetDriveAngularVelocity() const { return mInformation.DriveAngularVelocity; }

		/** Sets the drive's target linear and angular velocities. */
		B3D_SCRIPT_EXPORT(ExportName(SetDriveVelocity))
		void SetDriveVelocity(const Vector3& linear, const Vector3& angular);

		/** @name Internal
		 *  @{
		 */

		/** Returns the low level joint implementation. */
		ID6JointImplementation& GetImplementation() const;

		/** @} */

		/************************************************************************/
		/* 						COMPONENT OVERRIDES                      		*/
		/************************************************************************/
	protected:
		friend class SceneObject;

		UPtr<IJointImplementation> CreateImplementation() override;

		D6JointCreateInformation mInformation;

		/************************************************************************/
		/* 								RTTI		                     		*/
		/************************************************************************/
	public:
		friend class D6JointRTTI;
		static RTTIType* GetRttiStatic();
		RTTIType* GetRtti() const;

	protected:
		D6Joint(); // Serialization only
	};

	/** @} */

	/**
	 * @addtogroup Physics-Internal
	 */

	/** Low-level interface for a joint used by the D6Joint component. Should be implemented by the physics plugin to provide joint functionality. */
	class B3D_EXPORT ID6JointImplementation : public IJointImplementation
	{
	public:
		/** @copydoc D6Joint::SetMotion */
		virtual void SetMotion(D6JointAxis axis, D6JointMotion motion) = 0;

		/** @copydoc D6Joint::GetMotion */
		virtual D6JointMotion GetMotion(D6JointAxis axis) const = 0;

		/** @copydoc D6Joint::GetTwist */
		virtual Radian GetTwist() const = 0;

		/** @copydoc D6Joint::GetSwingY */
		virtual Radian GetSwingY() const = 0;

		/** @copydoc D6Joint::GetSwingZ */
		virtual Radian GetSwingZ() const = 0;

		/** @copydoc D6Joint::SetLimitLinear */
		virtual void SetLimitLinear(const LimitLinear& limit) = 0;

		/** @copydoc D6Joint::GetLimitLinear */
		virtual LimitLinear GetLimitLinear() const = 0;

		/** @copydoc D6Joint::SetLimitTwist */
		virtual void SetLimitTwist(const LimitAngularRange& limit) = 0;

		/** @copydoc D6Joint::GetLimitTwist */
		virtual LimitAngularRange GetLimitTwist() const = 0;

		/** @copydoc D6Joint::GetLimitSwing */
		virtual LimitConeRange GetLimitSwing() const = 0;

		/** @copydoc D6Joint::SetLimitSwing */
		virtual void SetLimitSwing(const LimitConeRange& limit) = 0;

		/** @copydoc D6Joint::GetDrive */
		virtual D6JointDrive GetDrive(D6JointDriveType type) const = 0;

		/** @copydoc D6Joint::SetDrive */
		virtual void SetDrive(D6JointDriveType type, const D6JointDrive& drive) = 0;

		/** @copydoc D6Joint::GetDrivePosition */
		virtual Vector3 GetDrivePosition() const = 0;

		/** @copydoc D6Joint::GetDriveRotation */
		virtual Quaternion GetDriveRotation() const = 0;

		/** @copydoc D6Joint::SetDriveTransform */
		virtual void SetDriveTransform(const Vector3& position, const Quaternion& rotation) = 0;

		/** @copydoc D6Joint::GetDriveLinearVelocity */
		virtual Vector3 GetDriveLinearVelocity() const = 0;

		/** @copydoc D6Joint::GetDriveAngularVelocity */
		virtual Vector3 GetDriveAngularVelocity() const = 0;

		/** @copydoc D6Joint::SetDriveVelocity */
		virtual void SetDriveVelocity(const Vector3& linear, const Vector3& angular) = 0;
	};

	/** @} */
} // namespace b3d
