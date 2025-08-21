//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsCorePrerequisites.h"
#include "Physics/BsDistanceJoint.h"
#include "Components/BsCJoint.h"

namespace b3d
{
	/** @addtogroup Components-Core
	 *  @{
	 */

	class IDistanceJointImplementation;

	/** Controls distance joint options. */
	enum class B3D_SCRIPT_EXPORT(DocumentationGroup(Physics)) DistanceJointFlag
	{
		MinDistance = 0x1, /**< Enables minimum distance limit. */
		MaxDistance = 0x2, /**< Enables maximum distance limit. */
		Spring = 0x4 /**< Enables spring when maintaining limits. */
	};

	/** Structure used for initializing a new DistanceJoint. */
	struct DistanceJointCreateInformation : JointCreateInformation
	{
		float MinDistance = 0.0f;
		float MaxDistance = 0.0f;
		float Tolerance = 0.25f;
		Spring Spring;
		DistanceJointFlag Flag = (DistanceJointFlag)0;
	};

	/** A joint that maintains an upper or lower (or both) bound on the distance between two bodies. */
	class B3D_CORE_EXPORT B3D_SCRIPT_EXPORT(DocumentationGroup(Physics), ExportName(DistanceJoint)) CDistanceJoint : public CJoint
	{
	public:
		CDistanceJoint(const HSceneObject& parent);

		/** Returns the current distance between the two joint bodies. */
		B3D_SCRIPT_EXPORT(ExportName(Distance), Property(Getter))
		float GetDistance() const;

		/**
		 * Determines the minimum distance the bodies are allowed to be at, they will get no closer. You must enable min
		 * distance flag in order for this limit to be applied.
		 */
		B3D_SCRIPT_EXPORT(ExportName(MinDistance), Property(Setter))
		void SetMinDistance(float value);

		/** @copydoc SetMinDistance */
		B3D_SCRIPT_EXPORT(ExportName(MinDistance), Property(Getter))
		float GetMinDistance() const;

		/**
		 * Determines the maximum distance the bodies are allowed to be at, they will get no further. You must enable max
		 * distance flag in order for this limit to be applied.
		 */
		B3D_SCRIPT_EXPORT(ExportName(MaxDistance), Property(Setter))
		void SetMaxDistance(float value);

		/** @copydoc SetMaxDistance */
		B3D_SCRIPT_EXPORT(ExportName(MaxDistance), Property(Getter))
		float GetMaxDistance() const;

		/**
		 * Determines the error tolerance of the joint at which the joint becomes active. This value slightly extends the
		 * lower and upper limit.
		 */
		B3D_SCRIPT_EXPORT(ExportName(Tolerance), Property(Setter))
		void SetTolerance(float value);

		/** @copydoc SetTolerance */
		B3D_SCRIPT_EXPORT(ExportName(Tolerance), Property(Getter))
		float GetTolerance() const;

		/**
		 * Determines a spring that controls how the joint responds when a limit is reached. You must enable the spring
		 * flag on the joint in order for this to be recognized.
		 *
		 * @see	Spring
		 */
		B3D_SCRIPT_EXPORT(ExportName(Spring), Property(Setter))
		void SetSpring(const Spring& value);

		/** @copydoc SetSpring */
		B3D_SCRIPT_EXPORT(ExportName(Spring), Property(Getter))
		Spring GetSpring() const;

		/** Enables or disables a flag that controls joint behaviour. */
		B3D_SCRIPT_EXPORT(ExportName(SetFlag))
		void SetFlag(DistanceJointFlag flag, bool enabled);

		/** Checks whether a certain joint flag is enabled. */
		B3D_SCRIPT_EXPORT(ExportName(HasFlag))
		bool HasFlag(DistanceJointFlag flag) const;

		/** @name Internal
		 *  @{
		 */

		/** Returns the low level joint implementation. */
		IDistanceJointImplementation& GetImplementation() const { return static_cast<IDistanceJointImplementation&>(*mImplementation); }

		/** @} */

		/************************************************************************/
		/* 						COMPONENT OVERRIDES                      		*/
		/************************************************************************/
	protected:
		friend class SceneObject;

		SPtr<Joint> CreateInternal() override;

		DistanceJointCreateInformation mInformation;

		/************************************************************************/
		/* 								RTTI		                     		*/
		/************************************************************************/
	public:
		friend class CDistanceJointRTTI;
		static RTTIType* GetRttiStatic();
		RTTIType* GetRtti() const;

	protected:
		CDistanceJoint(); // Serialization only
	};

	/** Low-level interface for a joint used by the DistanceJoint component. Should be implemented by the physics plugin to provide joint functionality. */
	class B3D_CORE_EXPORT IDistanceJointImplementation : public IJointImplementation
	{
	public:
		/** @copydoc DistanceJoint::GetDistance */
		virtual float GetDistance() const = 0;

		/** @copydoc DistanceJoint::SetMinDistance */
		virtual void SetMinDistance(float value) = 0;

		/** @copydoc DistanceJoint::GetMinDistance */
		virtual float GetMinDistance() const = 0;

		/** @copydoc DistanceJoint::SetMaxDistance */
		virtual void SetMaxDistance(float value) = 0;

		/** @copydoc DistanceJoint::GetMaxDistance */
		virtual float GetMaxDistance() const = 0;

		/** @copydoc DistanceJoint::GetTolerance */
		virtual float GetTolerance() const = 0;

		/** @copydoc DistanceJoint::SetTolerance */
		virtual void SetTolerance(float value) = 0;

		/** @copydoc DistanceJoint::GetSpring */
		virtual Spring GetSpring() const = 0;

		/** @copydoc DistanceJoint::SetSpring */
		virtual void SetSpring(const Spring& value) = 0;

		/** @copydoc DistanceJoint::SetFlag */
		virtual void SetFlag(DistanceJointFlag flag, bool enabled) = 0;

		/** @copydoc DistanceJoint::HasFlag */
		virtual bool HasFlag(DistanceJointFlag flag) const = 0;
	};

	/** @} */
} // namespace b3d
