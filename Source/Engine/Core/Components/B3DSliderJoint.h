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

	class ISliderJointImplementation;

	/** Flag that controls slider joint's behaviour. */
	enum class B3D_SCRIPT_EXPORT(DocumentationGroup(Physics)) SliderJointFlag
	{
		Limit = 0x1 /**< Enables the linear range limit. */
	};

	using SliderJointFlags = Flags<SliderJointFlag>;
	B3D_FLAGS_OPERATORS(SliderJointFlag);

	/** Structure used for initializing a new SliderJoint. */
	struct SliderJointCreateInformation : JointCreateInformation
	{
		LimitLinearRange Limit;
		SliderJointFlags Flags;
	};

	/** Joint that removes all but a single translational degree of freedom. Bodies are allowed to move along a single axis. */
	class B3D_EXPORT B3D_SCRIPT_EXPORT(DocumentationGroup(Physics)) SliderJoint : public Joint
	{
	public:
		SliderJoint(const HSceneObject& parent);

		/** Returns the current position of the slider. */
		B3D_SCRIPT_EXPORT(ExportName(Position), Property(Getter))
		float GetPosition() const;

		/** Returns the current speed of the slider. */
		B3D_SCRIPT_EXPORT(ExportName(Speed), Property(Getter))
		float GetSpeed() const;

		/**
		 * Determines a limit that constrains the movement of the joint to a specific minimum and maximum distance. You must
		 * enable the limit flag on the joint in order for this to be recognized.
		 *
		 * @see LimitLinearRange
		 */
		B3D_SCRIPT_EXPORT(ExportName(Limit), Property(Setter))
		void SetLimit(const LimitLinearRange& limit);

		/** @copydoc SetLimit */
		B3D_SCRIPT_EXPORT(ExportName(Limit), Property(Getter))
		LimitLinearRange GetLimit() const { return mInformation.Limit; }

		/** Enables or disables a flag that controls the joint's behaviour. */
		B3D_SCRIPT_EXPORT(ExportName(SetFlag))
		void SetFlag(SliderJointFlag flag, bool enabled);

		/** Checks is the specified flag enabled. */
		B3D_SCRIPT_EXPORT(ExportName(HasFlag))
		bool HasFlag(SliderJointFlag flag) const { return mInformation.Flags.IsSet(flag); }

		/** @name Internal
		 *  @{
		 */

		/** Returns the low level joint implementation. */
		ISliderJointImplementation& GetImplementation() const;

		/** @} */

		/************************************************************************/
		/* 						COMPONENT OVERRIDES                      		*/
		/************************************************************************/
	protected:
		friend class SceneObject;

		UPtr<IJointImplementation> CreateImplementation() override;
		void CalculateLocalBodyTransform(JointBody body, Vector3& position, Quaternion& rotation) override;

		SliderJointCreateInformation mInformation;

		/************************************************************************/
		/* 								RTTI		                     		*/
		/************************************************************************/
	public:
		friend class SliderJointRTTI;
		static RTTIType* GetRttiStatic();
		RTTIType* GetRtti() const;

	protected:
		SliderJoint(); // Serialization only
	};

	/** @} */

	/**
	 * @addtogroup Physics-Internal
	 * @{
	 */

	/** Low-level interface for a joint used by the SliderJoint component. Should be implemented by the physics plugin to provide joint functionality. */
	class B3D_EXPORT ISliderJointImplementation : public IJointImplementation
	{
	public:
		using IJointImplementation::GetPosition;

		/** @copydoc SliderJoint::GetPosition */
		virtual float GetPosition() const = 0;

		/** @copydoc SliderJoint::GetSpeed */
		virtual float GetSpeed() const = 0;

		/** @copydoc SliderJoint::SetLimit */
		virtual void SetLimit(const LimitLinearRange& limit) = 0;

		/** @copydoc SliderJoint::GetLimit */
		virtual LimitLinearRange GetLimit() const = 0;

		/** @copydoc SliderJoint::SetFlag */
		virtual void SetFlag(SliderJointFlag flag, bool enabled) = 0;

		/** @copydoc SliderJoint::HasFlag */
		virtual bool HasFlag(SliderJointFlag flag) const = 0;
	};
	/** @} */
} // namespace b3d
