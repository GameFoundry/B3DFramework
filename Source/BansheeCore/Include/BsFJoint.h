//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsCorePrerequisites.h"
#include "BsVector3.h"
#include "BsQuaternion.h"

namespace bs
{
	/** @addtogroup Physics
	 *  @{
	 */

	/** Specifies first or second body referenced by a Joint. */
	enum class JointBody
	{
		Target, /**< Body the joint is influencing. */
		Anchor /**< Body the joint is attached to (if any). */
	};

	/** @} */
	/** @addtogroup Physics-Internal
	 *  @{
	 */

	/** Provides common functionality used by all Joint types. */
	class BS_CORE_EXPORT FJoint
	{
	public:
		FJoint(const JOINT_DESC& desc) { }
		virtual ~FJoint() { }

		/** Returns one of the bodies managed by the joint. */
		virtual Rigidbody* getBody(JointBody body) const = 0;

		/** Sets a body managed by the joint. One of the bodies must be movable (non-kinematic). */
		virtual void setBody(JointBody body, Rigidbody* value) = 0;

		/** Returns the position relative to the body, at which the body is anchored to the joint. */
		virtual Vector3 getPosition(JointBody body) const = 0;

		/** Returns the rotation relative to the body, at which the body is anchored to the joint. */
		virtual Quaternion getRotation(JointBody body) const = 0;

		/** Sets the position and rotation relative to the body, at which the body is anchored to the joint.  */
		virtual void setTransform(JointBody body, const Vector3& position, const Quaternion& rotation) = 0;

		/** 
		 * Returns the maximum force the joint can apply before breaking. Broken joints no longer participate in physics 
		 * simulation. 
		 */
		virtual float getBreakForce() const = 0;

		/** 
		 * Sets the maximum force the joint can apply before breaking. Broken joints no longer participate in physics 
		 * simulation. 
		 */
		virtual void setBreakForce(float force) = 0;

		/** 
		 * Returns the maximum torque the joint can apply before breaking. Broken joints no longer participate in physics 
		 * simulation. 
		 */
		virtual float getBreakTorque() const = 0;

		/** 
		 * Sets the maximum torque the joint can apply before breaking. Broken joints no longer participate in physics 
		 * simulation. 
		 */
		virtual void setBreakTorque(float torque) = 0;

		/** Checks whether collisions between the two bodies managed by the joint are enabled. */
		virtual bool getEnableCollision() const = 0;

		/** Sets whether collision between the two bodies managed by the joint are enabled. */
		virtual void setEnableCollision(bool value) = 0;
	};

	/** @} */
}