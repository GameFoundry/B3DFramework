//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include <cfloat>

#include "BsCorePrerequisites.h"
#include "Physics/BsPhysicsCommon.h"
#include "Physics/BsFJoint.h"

namespace bs
{
	/** @addtogroup Physics
	 *  @{
	 */

	struct JOINT_DESC;

	/**
	 * Base class for all Joint types. Joints constrain how two rigidbodies move relative to one another (for example a door
	 * hinge). One of the bodies in the joint must always be movable (non-kinematic).
	 */
	class BS_CORE_EXPORT Joint
	{
	public:
		Joint() = default;
		virtual ~Joint() = default;

		/** @copydoc FJoint::getBody */
		Rigidbody* getBody(JointBody body) const;

		/** @copydoc FJoint::setBody */
		void setBody(JointBody body, Rigidbody* value);

		/** @copydoc FJoint::getPosition */
		Vector3 getPosition(JointBody body) const;

		/** @copydoc FJoint::getRotation */
		Quaternion getRotation(JointBody body) const;

		/** @copydoc FJoint::setTransform */
		void setTransform(JointBody body, const Vector3& position, const Quaternion& rotation);

		/** @copydoc FJoint::getBreakForce */
		float getBreakForce() const;

		/** @copydoc FJoint::setBreakForce */
		void setBreakForce(float force);

		/** @copydoc FJoint::getBreakTorque */
		float getBreakTorque() const;

		/** @copydoc FJoint::setBreakTorque */
		void setBreakTorque(float torque);

		/** @copydoc FJoint::getEnableCollision */
		bool getEnableCollision() const;

		/** @copydoc FJoint::setEnableCollision */
		void setEnableCollision(bool value);

		/** Triggered when the joint's break force or torque is exceeded. */
		Event<void()> onJointBreak;

		/** @name Internal
		 *  @{
		 */

		/**
		 * Sets the object that owns this physics object, if any. Used for high level systems so they can easily map their
		 * high level physics objects from the low level ones returned by various queries and events.
		 */
		void SetOwnerInternal(PhysicsOwnerType type, void* owner) { mOwner.type = type; mOwner.ownerData = owner; }

		/**
		 * Gets the object that owns this physics object, if any. Used for high level systems so they can easily map their
		 * high level physics objects from the low level ones returned by various queries and events.
		 */
		void* GetOwnerInternal(PhysicsOwnerType type) const { return mOwner.type == type ? mOwner.ownerData : nullptr; }

		/** @} */

	protected:
		PhysicsObjectOwner mOwner;
		FJoint* mInternal = nullptr;
	};

	/** Structure used for initializing a new Joint. */
	struct JOINT_DESC
	{
		struct BodyInfo
		{
			Rigidbody* body = nullptr;
			Vector3 position = Vector3::ZERO;
			Quaternion rotation = Quaternion::IDENTITY;
		};

		BodyInfo bodies[2];
		float breakForce = FLT_MAX;
		float breakTorque = FLT_MAX;
		bool enableCollision = false;
	};

	/**
	 * Controls spring parameters for a physics joint limits. If a limit is soft (body bounces back due to restition when
	 * the limit is reached) the spring will pull the body back towards the limit using the specified parameters.
	 */
	struct BS_SCRIPT_EXPORT(m:Physics,pl:true) Spring
	{
		/** Constructs a spring with no force. */
		Spring() { }

		/**
		 * Constructs a spring.
		 *
		 * @param	stiffness	Spring strength. Force proportional to the position error.
		 * @param	damping		Damping strength. Force propertional to the velocity error.
		 */
		Spring(float stiffness, float damping)
			:stiffness(stiffness), damping(damping)
		{ }

		bool operator==(const Spring& other) const
		{
			return stiffness == other.stiffness && damping == other.damping;
		}

		/** Spring strength. Force proportional to the position error. */
		float stiffness = 0.0f;

		/** Damping strength. Force propertional to the velocity error. */
		float damping = 0.0f;
	};

	/** Contains common values used by all Joint limit types. */
	struct BS_SCRIPT_EXPORT(m:Physics,pl:true) LimitCommon
	{
		LimitCommon(float contactDist = -1.0f)
			:contactDist(contactDist)
		{ }

		LimitCommon(const Spring& spring, float restitution = 0.0f)
			: restitution(restitution), spring(spring)
		{ }

		/**
		 * Distance from the limit at which it becomes active. Allows the solver to activate earlier than the limit is
		 * reached to avoid breaking the limit.
		 */
		float contactDist = -1.0f;

		/**
		 * Controls how do objects react when the limit is reached, values closer to zero specify non-ellastic collision,
		 * while those closer to one specify more ellastic (i.e bouncy) collision. Must be in [0, 1] range.
		 */
		float restitution = 0.0f;

		/** Spring that controls how are the bodies pulled back towards the limit when they breach it. */
		Spring spring;
	};

	/** Represents a joint limit between two distance values. Lower value must be less than the upper value. */
	struct BS_SCRIPT_EXPORT(m:Physics,pl:true) LimitLinearRange : LimitCommon
	{
		/** Constructs an empty limit. */
		LimitLinearRange()
		{ }

		/**
		 * Constructs a hard limit. Once the limit is reached the movement of the attached bodies will come to a stop.
		 *
		 * @param	lower		Lower distance of the limit. Must be less than @p upper.
		 * @param	upper		Upper distance of the limit. Must be more than @p lower.
		 * @param	contactDist	Distance from the limit at which it becomes active. Allows the solver to activate earlier
		 *						than the limit is reached to avoid breaking the limit. Specify -1 for the default.
		 */
		LimitLinearRange(float lower, float upper, float contactDist = -1.0f)
			:LimitCommon(contactDist), lower(lower), upper(upper)
		{ }

		/**
		 * Constructs a soft limit. Once the limit is reached the bodies will bounce back according to the resitution
		 * parameter and will be pulled back towards the limit by the provided spring.
		 *
		 * @param	lower		Lower distance of the limit. Must be less than @p upper.
		 * @param	upper		Upper distance of the limit. Must be more than @p lower.
		 * @param	spring		Spring that controls how are the bodies pulled back towards the limit when they breach it.
		 * @param	restitution	Controls how do objects react when the limit is reached, values closer to zero specify
		 *						non-ellastic collision, while those closer to one specify more ellastic (i.e bouncy)
		 *						collision. Must be in [0, 1] range.
		 */
		LimitLinearRange(float lower, float upper, const Spring& spring, float restitution = 0.0f)
			:LimitCommon(spring, restitution), lower(lower), upper(upper)
		{ }

		bool operator==(const LimitLinearRange& other) const
		{
			return lower == other.lower && upper == other.upper && contactDist == other.contactDist &&
				restitution == other.restitution && spring == other.spring;
		}

		/** Lower distance of the limit. Must be less than #upper. */
		float lower = 0.0f;

		/** Upper distance of the limit. Must be more than #lower. */
		float upper = 0.0f;
	};

	/** Represents a joint limit between zero a single distance value. */
	struct BS_SCRIPT_EXPORT(m:Physics,pl:true) LimitLinear : LimitCommon
	{
		/** Constructs an empty limit. */
		LimitLinear()
		{ }

		/**
		 * Constructs a hard limit. Once the limit is reached the movement of the attached bodies will come to a stop.
		 *
		 * @param	extent		Distance at which the limit becomes active.
		 * @param	contactDist	Distance from the limit at which it becomes active. Allows the solver to activate earlier
		 *						than the limit is reached to avoid breaking the limit. Specify -1 for the default.
		 */
		LimitLinear(float extent, float contactDist = -1.0f)
			:LimitCommon(contactDist), extent(extent)
		{ }

		/**
		 * Constructs a soft limit. Once the limit is reached the bodies will bounce back according to the resitution
		 * parameter and will be pulled back towards the limit by the provided spring.
		 *
		 * @param	extent		Distance at which the limit becomes active.
		 * @param	spring		Spring that controls how are the bodies pulled back towards the limit when they breach it.
		 * @param	restitution	Controls how do objects react when the limit is reached, values closer to zero specify
		 *						non-ellastic collision, while those closer to one specify more ellastic (i.e bouncy)
		 *						collision. Must be in [0, 1] range.
		 */
		LimitLinear(float extent, const Spring& spring, float restitution = 0.0f)
			:LimitCommon(spring, restitution), extent(extent)
		{ }

		bool operator==(const LimitLinear& other) const
		{
			return extent == other.extent && contactDist == other.contactDist && restitution == other.restitution &&
				spring == other.spring;
		}

		/** Distance at which the limit becomes active. */
		float extent = 0.0f;
	};

	/** Represents a joint limit between two angles. */
	struct BS_SCRIPT_EXPORT(m:Physics,pl:true) LimitAngularRange : LimitCommon
	{
		/** Constructs an empty limit. */
		LimitAngularRange()
		{ }

		/**
		 * Constructs a hard limit. Once the limit is reached the movement of the attached bodies will come to a stop.
		 *
		 * @param	lower		Lower angle of the limit. Must be less than @p upper.
		 * @param	upper		Upper angle of the limit. Must be more than @p lower.
		 * @param	contactDist	Distance from the limit at which it becomes active. Allows the solver to activate earlier
		 *						than the limit is reached to avoid breaking the limit. Specify -1 for the default.
		 */
		LimitAngularRange(Radian lower, Radian upper, float contactDist = -1.0f)
			:LimitCommon(contactDist), lower(lower), upper(upper)
		{ }

		/**
		 * Constructs a soft limit. Once the limit is reached the bodies will bounce back according to the resitution
		 * parameter and will be pulled back towards the limit by the provided spring.
		 *
		 * @param	lower		Lower angle of the limit. Must be less than @p upper.
		 * @param	upper		Upper angle of the limit. Must be more than @p lower.
		 * @param	spring		Spring that controls how are the bodies pulled back towards the limit when they breach it.
		 * @param	restitution	Controls how do objects react when the limit is reached, values closer to zero specify
		 *						non-ellastic collision, while those closer to one specify more ellastic (i.e bouncy)
		 *						collision. Must be in [0, 1] range.
		 */
		LimitAngularRange(Radian lower, Radian upper, const Spring& spring, float restitution = 0.0f)
			:LimitCommon(spring, restitution), lower(lower), upper(upper)
		{ }

		bool operator==(const LimitAngularRange& other) const
		{
			return lower == other.lower && upper == other.upper && contactDist == other.contactDist &&
				restitution == other.restitution && spring == other.spring;
		}

		/** Lower angle of the limit. Must be less than #upper. */
		BS_SCRIPT_EXPORT(range:[0,359])
		Radian lower = Radian(0.0f);

		/** Upper angle of the limit. Must be less than #lower. */
		BS_SCRIPT_EXPORT(range:[0,359])
		Radian upper = Radian(0.0f);
	};

	/** Represents a joint limit that contraints movement to within an elliptical cone. */
	struct BS_SCRIPT_EXPORT(m:Physics,pl:true) LimitConeRange : LimitCommon
	{
		/** Constructs a limit with a 45 degree cone. */
		LimitConeRange()
		{ }

		/**
		 * Constructs a hard limit. Once the limit is reached the movement of the attached bodies will come to a stop.
		 *
		 * @param	yLimitAngle		Y angle of the cone. Movement is constrainted between 0 and this angle on the Y axis.
		 * @param	zLimitAngle		Z angle of the cone. Movement is constrainted between 0 and this angle on the Z axis.
		 * @param	contactDist		Distance from the limit at which it becomes active. Allows the solver to activate
		 *							earlier than the limit is reached to avoid breaking the limit. Specify -1 for the
		 *							default.
		 */
		LimitConeRange(Radian yLimitAngle, Radian zLimitAngle, float contactDist = -1.0f)
			:LimitCommon(contactDist), yLimitAngle(yLimitAngle), zLimitAngle(zLimitAngle)
		{ }

		/**
		 * Constructs a soft limit. Once the limit is reached the bodies will bounce back according to the resitution
		 * parameter and will be pulled back towards the limit by the provided spring.
		 *
		 * @param	yLimitAngle	Y angle of the cone. Movement is constrainted between 0 and this angle on the Y axis.
		 * @param	zLimitAngle	Z angle of the cone. Movement is constrainted between 0 and this angle on the Z axis.
		 * @param	spring		Spring that controls how are the bodies pulled back towards the limit when they breach it.
		 * @param	restitution	Controls how do objects react when the limit is reached, values closer to zero specify
		 *						non-ellastic collision, while those closer to one specify more ellastic (i.e bouncy)
		 *						collision. Must be in [0, 1] range.
		 */
		LimitConeRange(Radian yLimitAngle, Radian zLimitAngle, const Spring& spring, float restitution = 0.0f)
			:LimitCommon(spring, restitution), yLimitAngle(yLimitAngle), zLimitAngle(zLimitAngle)
		{ }

		bool operator==(const LimitConeRange& other) const
		{
			return yLimitAngle == other.yLimitAngle && zLimitAngle == other.zLimitAngle &&
				contactDist == other.contactDist && restitution == other.restitution && spring == other.spring;
		}

		/** Y angle of the cone. Movement is constrainted between 0 and this angle on the Y axis. */
		BS_SCRIPT_EXPORT(range:[0,180])
		Radian yLimitAngle = Radian(Math::HALF_PI);

		/** Z angle of the cone. Movement is constrainted between 0 and this angle on the Z axis. */
		BS_SCRIPT_EXPORT(range:[0,180])
		Radian zLimitAngle = Radian(Math::HALF_PI);
	};

	/** @} */
}
