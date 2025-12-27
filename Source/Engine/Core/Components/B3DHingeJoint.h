//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DPrerequisites.h"
#include "Components/B3DJoint.h"

namespace b3d
{
	/** @addtogroup Components
	 *  @{
	 */

	class IHingeJointImplementation;

	/** Flags that control hinge joint options. */
	enum class B3D_SCRIPT_EXPORT(DocumentationGroup(Physics)) HingeJointFlag
	{
		Limit = 0x1, /**< Joint limit is enabled. */
		Drive = 0x2 /**< Joint drive is enabled. */
	};

	using HingeJointFlags = Flags<HingeJointFlag>;
	B3D_FLAGS_OPERATORS(HingeJointFlag)

	/** Properties of a drive that drives the joint's angular velocity towards a paricular value. */
	struct B3D_SCRIPT_EXPORT(DocumentationGroup(Physics), ExportAsStruct(true)) HingeJointDrive
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

	/** Structure used for initializing a new HingeJoint. */
	struct HingeJointCreateInformation : JointCreateInformation
	{
		HingeJointDrive Drive;
		LimitAngularRange Limit;
		HingeJointFlags Flags;
	};

	/** Hinge joint removes all but a single rotation degree of freedom from its two attached bodies (for example a door hinge). */
	class B3D_EXPORT B3D_SCRIPT_EXPORT(DocumentationGroup(Physics)) HingeJoint : public Joint
	{
	public:
		HingeJoint(const HSceneObject& parent);

		/** Returns the current angle between the two attached bodes. */
		B3D_SCRIPT_EXPORT(ExportName(Angle), Property(Getter))
		Radian GetAngle() const;

		/** Returns the current angular speed of the joint. */
		B3D_SCRIPT_EXPORT(ExportName(Speed), Property(Getter))
		float GetSpeed() const;

		/**
		 * Determines the limit of the joint. Limit constrains the motion to the specified angle range. You must enable the
		 * limit flag on the joint in order for this to be recognized.
		 *
		 * @see LimitAngularRange
		 */
		B3D_SCRIPT_EXPORT(ExportName(Limit), Property(Setter))
		void SetLimit(const LimitAngularRange& limit);

		/** @copydoc SetLimit */
		B3D_SCRIPT_EXPORT(ExportName(Limit), Property(Getter))
		LimitAngularRange GetLimit() const { return mInformation.Limit; }

		/**
		 * Determines the drive properties of the joint. It drives the joint's angular velocity towards a particular value.
		 * You must enable the drive flag on the joint in order for the drive to be active.
		 *
		 * @see HingeJoint::Drive
		 */
		B3D_SCRIPT_EXPORT(ExportName(Drive), Property(Setter))
		void SetDrive(const HingeJointDrive& drive);

		/** @copydoc SetDrive */
		B3D_SCRIPT_EXPORT(ExportName(Drive), Property(Getter))
		HingeJointDrive GetDrive() const { return mInformation.Drive; }

		/** Enables or disables a flag that controls joint behaviour. */
		B3D_SCRIPT_EXPORT(ExportName(SetFlag))
		void SetFlag(HingeJointFlag flag, bool enabled);

		/** Checks is the specified option enabled. */
		B3D_SCRIPT_EXPORT(ExportName(HasFlag))
		bool HasFlag(HingeJointFlag flag) const { return mInformation.Flags.IsSet(flag); }

		/** @name Internal
		 *  @{
		 */

		/** Returns the low level joint implementation. */
		IHingeJointImplementation& GetImplementation() const;

		/** @} */

		/************************************************************************/
		/* 						COMPONENT OVERRIDES                      		*/
		/************************************************************************/
	protected:
		friend class SceneObject;

		UPtr<IJointImplementation> CreateImplementation() override;

		HingeJointCreateInformation mInformation;

		/************************************************************************/
		/* 								RTTI		                     		*/
		/************************************************************************/
	public:
		friend class HingeJointRTTI;
		static RTTIType* GetRttiStatic();
		RTTIType* GetRtti() const;

	protected:
		HingeJoint(); // Serialization only
	};

	/** Low-level interface for a joint used by the HingeJoint component. Should be implemented by the physics plugin to provide joint functionality. */
	class B3D_EXPORT IHingeJointImplementation : public IJointImplementation
	{
	public:
		/** @copydoc HingeJoint::GetAngle */
		virtual Radian GetAngle() const = 0;

		/** @copydoc HingeJoint::GetSpeed */
		virtual float GetSpeed() const = 0;

		/** @copydoc HingeJoint::SetLimit */
		virtual void SetLimit(const LimitAngularRange& limit) = 0;

		/** @copydoc HingeJoint::GetLimit */
		virtual LimitAngularRange GetLimit() const = 0;

		/** @copydoc HingeJoint::SetDrive */
		virtual void SetDrive(const HingeJointDrive& drive) = 0;

		/** @copydoc HingeJoint::GetDrive */
		virtual HingeJointDrive GetDrive() const = 0;

		/** @copydoc HingeJoint::SetFlag */
		virtual void SetFlag(HingeJointFlag flag, bool enabled) = 0;

		/** @copydoc HingeJoint::HasFlag */
		virtual bool HasFlag(HingeJointFlag flag) const = 0;
	};

	/** @} */
} // namespace b3d
