//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsCorePrerequisites.h"
#include "Math/BsVector3.h"
#include "Math/BsQuaternion.h"

namespace b3d
{
	/** @addtogroup Physics
	 *  @{
	 */

	/** Specifies first or second body referenced by a Joint. */
	enum class B3D_SCRIPT_EXPORT(DocumentationGroup(Physics)) JointBody
	{
		Target, /**< Body the joint is influencing. */
		Anchor /**< Body the joint is attached to (if any). */
	};

	/** @} */
	/** @addtogroup Physics-Internal
	 *  @{
	 */

	/** Provides common functionality used by all Joint types. */
	class B3D_CORE_EXPORT FJoint
	{
	public:
		FJoint(const JOINT_DESC& desc) {}

		virtual ~FJoint() = default;

		/** @copydoc SetBody() */
		virtual CRigidbody* GetBody(JointBody body) const = 0;

		/** Determines a body managed by the joint. One of the bodies must be movable (non-kinematic). */
		virtual void SetBody(JointBody body, CRigidbody* value) = 0;

		/** Returns the position relative to the body, at which the body is anchored to the joint. */
		virtual Vector3 GetPosition(JointBody body) const = 0;

		/** Returns the rotation relative to the body, at which the body is anchored to the joint. */
		virtual Quaternion GetRotation(JointBody body) const = 0;

		/** Sets the position and rotation relative to the body, at which the body is anchored to the joint.  */
		virtual void SetTransform(JointBody body, const Vector3& position, const Quaternion& rotation) = 0;

		/** @copydoc SetBreakForce() */
		virtual float GetBreakForce() const = 0;

		/**
		 * Determines the maximum force the joint can apply before breaking. Broken joints no longer participate in physics
		 * simulation.
		 */
		virtual void SetBreakForce(float force) = 0;

		/** @copydoc SetBreakTorque() */
		virtual float GetBreakTorque() const = 0;

		/**
		 * Determines the maximum torque the joint can apply before breaking. Broken joints no longer participate in physics
		 * simulation.
		 */
		virtual void SetBreakTorque(float torque) = 0;

		/** @copydoc SetEnableCollision() */
		virtual bool GetEnableCollision() const = 0;

		/** Determines whether collision between the two bodies managed by the joint are enabled. */
		virtual void SetEnableCollision(bool value) = 0;
	};

	/** @} */
} // namespace b3d
