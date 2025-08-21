//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsCorePrerequisites.h"
#include "Components/BsCJoint.h"

namespace b3d
{
	/** @addtogroup Components-Core
	 *  @{
	 */

	class ISphericalJointImplementation;

	/** Flags that control options for the spherical joint */
	enum class B3D_SCRIPT_EXPORT(DocumentationGroup(Physics)) SphericalJointFlag
	{
		Limit = 0x1 /**< Enables the cone range limit. */
	};

	/** Structure used for initializing a new SphericalJoint. */
	struct SphericalJointCreateInformation : JointCreateInformation
	{
		LimitConeRange Limit;
		SphericalJointFlag Flag = (SphericalJointFlag)0;
	};

	/**
	 * A spherical joint removes all translational degrees of freedom but allows all rotational degrees of freedom.
	 * Essentially this ensures that the anchor points of the two bodies are always coincident. Bodies are allowed to
	 * rotate around the anchor points, and their rotation can be limited by an elliptical cone.
	 */
	class B3D_CORE_EXPORT B3D_SCRIPT_EXPORT(DocumentationGroup(Physics), ExportName(SphericalJoint)) CSphericalJoint : public CJoint
	{
	public:
		CSphericalJoint(const HSceneObject& parent);

		/**
		 * Determines the limit of the joint. This clamps the rotation inside an eliptical angular cone. You must enable
		 * limit flag on the joint in order for this to be recognized.
		 */
		B3D_SCRIPT_EXPORT(ExportName(Limit), Property(Setter))
		void SetLimit(const LimitConeRange& limit);

		/** @copydoc SetLimit */
		B3D_SCRIPT_EXPORT(ExportName(Limit), Property(Getter))
		LimitConeRange GetLimit() const;

		/** Enables or disables a flag that controls the joint's behaviour. */
		B3D_SCRIPT_EXPORT(ExportName(SetFlag))
		void SetFlag(SphericalJointFlag flag, bool enabled);

		/** Checks is the specified flag enabled. */
		B3D_SCRIPT_EXPORT(ExportName(HasFlag))
		bool HasFlag(SphericalJointFlag flag) const;

		/** @name Internal
		 *  @{
		 */

		/** Returns the low level joint implementation. */
		ISphericalJointImplementation& GetImplementation() const { return static_cast<ISphericalJointImplementation&>(*mImplementation); }

		/** @} */

		/************************************************************************/
		/* 						COMPONENT OVERRIDES                      		*/
		/************************************************************************/
	protected:
		friend class SceneObject;

		SPtr<Joint> CreateInternal() override;

		SphericalJointCreateInformation mInformation;

		/************************************************************************/
		/* 								RTTI		                     		*/
		/************************************************************************/
	public:
		friend class CSphericalJointRTTI;
		static RTTIType* GetRttiStatic();
		RTTIType* GetRtti() const;

	protected:
		CSphericalJoint(); // Serialization only
	};

	/** Low-level interface for a joint used by the SphericalJoint component. Should be implemented by the physics plugin to provide joint functionality. */
	class B3D_CORE_EXPORT ISphericalJointImplementation : public IJointImplementation
	{
	public:
		/** @copydoc SphericalJoint::SetLimit */
		virtual void SetLimit(const LimitConeRange& limit) = 0;

		/** @copydoc SphericalJoint::GetLimit */
		virtual LimitConeRange GetLimit() const = 0;

		/** @copydoc SphericalJoint::SetFlag */
		virtual void SetFlag(SphericalJointFlag flag, bool enabled) = 0;

		/** @copydoc SphericalJoint::HasFlag */
		virtual bool HasFlag(SphericalJointFlag flag) const = 0;
	};

	/** @} */
} // namespace b3d
