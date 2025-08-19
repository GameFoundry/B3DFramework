//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include <cfloat>

#include "BsCorePrerequisites.h"
#include "Physics/BsPhysicsCommon.h"
#include "Physics/BsFJoint.h"

namespace b3d
{
	/** @addtogroup Physics
	 *  @{
	 */

	struct JOINT_DESC;

	/**
	 * Base class for all Joint types. Joints constrain how two rigidbodies move relative to one another (for example a door
	 * hinge). One of the bodies in the joint must always be movable (non-kinematic).
	 */
	class B3D_CORE_EXPORT Joint
	{
	public:
		Joint() = default;
		virtual ~Joint() = default;

		/** @copydoc FJoint::GetBody */
		CRigidbody* GetBody(JointBody body) const;

		/** @copydoc FJoint::SetBody */
		void SetBody(JointBody body, CRigidbody* value);

		/** @copydoc FJoint::GetPosition */
		Vector3 GetPosition(JointBody body) const;

		/** @copydoc FJoint::GetRotation */
		Quaternion GetRotation(JointBody body) const;

		/** @copydoc FJoint::SetTransform */
		void SetTransform(JointBody body, const Vector3& position, const Quaternion& rotation);

		/** @copydoc FJoint::GetBreakForce */
		float GetBreakForce() const;

		/** @copydoc FJoint::SetBreakForce */
		void SetBreakForce(float force);

		/** @copydoc FJoint::GetBreakTorque */
		float GetBreakTorque() const;

		/** @copydoc FJoint::SetBreakTorque */
		void SetBreakTorque(float torque);

		/** @copydoc FJoint::GetEnableCollision */
		bool GetEnableCollision() const;

		/** @copydoc FJoint::SetEnableCollision */
		void SetEnableCollision(bool value);

		/** Triggered when the joint's break force or torque is exceeded. */
		Event<void()> OnJointBreak;

		/** @name Internal
		 *  @{
		 */

		/**
		 * Sets the object that owns this physics object, if any. Used for high level systems so they can easily map their
		 * high level physics objects from the low level ones returned by various queries and events.
		 */
		void SetOwnerInternal(PhysicsOwnerType type, void* owner)
		{
			mOwner.Type = type;
			mOwner.OwnerData = owner;
		}

		/**
		 * Gets the object that owns this physics object, if any. Used for high level systems so they can easily map their
		 * high level physics objects from the low level ones returned by various queries and events.
		 */
		void* GetOwnerInternal(PhysicsOwnerType type) const { return mOwner.Type == type ? mOwner.OwnerData : nullptr; }

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
			CRigidbody* Body = nullptr;
			Vector3 Position = Vector3::kZero;
			Quaternion Rotation = Quaternion::kIdentity;
		};

		BodyInfo Bodies[2];
		float BreakForce = FLT_MAX;
		float BreakTorque = FLT_MAX;
		bool EnableCollision = false;
	};

	/**
	 * Controls spring parameters for a physics joint limits. If a limit is soft (body bounces back due to restition when
	 * the limit is reached) the spring will pull the body back towards the limit using the specified parameters.
	 */
	struct B3D_SCRIPT_EXPORT(DocumentationGroup(Physics), ExportAsStruct(true)) Spring
	{
		/** Constructs a spring with no force. */
		Spring() {}

		/**
		 * Constructs a spring.
		 *
		 * @param	stiffness	Spring strength. Force proportional to the position error.
		 * @param	damping		Damping strength. Force propertional to the velocity error.
		 */
		Spring(float stiffness, float damping)
			: Stiffness(stiffness), Damping(damping)
		{}

		bool operator==(const Spring& other) const
		{
			return Stiffness == other.Stiffness && Damping == other.Damping;
		}

		/** Spring strength. Force proportional to the position error. */
		float Stiffness = 0.0f;

		/** Damping strength. Force propertional to the velocity error. */
		float Damping = 0.0f;
	};

	/** Contains common values used by all Joint limit types. */
	struct B3D_SCRIPT_EXPORT(DocumentationGroup(Physics), ExportAsStruct(true)) LimitCommon
	{
		LimitCommon(float contactDist = -1.0f)
			: ContactDist(contactDist)
		{}

		LimitCommon(const Spring& spring, float restitution = 0.0f)
			: Restitution(restitution), Spring(spring)
		{}

		/**
		 * Distance from the limit at which it becomes active. Allows the solver to activate earlier than the limit is
		 * reached to avoid breaking the limit.
		 */
		float ContactDist = -1.0f;

		/**
		 * Controls how do objects react when the limit is reached, values closer to zero specify non-ellastic collision,
		 * while those closer to one specify more ellastic (i.e bouncy) collision. Must be in [0, 1] range.
		 */
		float Restitution = 0.0f;

		/** Spring that controls how are the bodies pulled back towards the limit when they breach it. */
		Spring Spring;
	};

	/** Represents a joint limit between two distance values. Lower value must be less than the upper value. */
	struct B3D_SCRIPT_EXPORT(DocumentationGroup(Physics), ExportAsStruct(true)) LimitLinearRange : LimitCommon
	{
		/** Constructs an empty limit. */
		LimitLinearRange()
		{}

		/**
		 * Constructs a hard limit. Once the limit is reached the movement of the attached bodies will come to a stop.
		 *
		 * @param	lower		Lower distance of the limit. Must be less than @p upper.
		 * @param	upper		Upper distance of the limit. Must be more than @p lower.
		 * @param	contactDist	Distance from the limit at which it becomes active. Allows the solver to activate earlier
		 *						than the limit is reached to avoid breaking the limit. Specify -1 for the default.
		 */
		LimitLinearRange(float lower, float upper, float contactDist = -1.0f)
			: LimitCommon(contactDist), Lower(lower), Upper(upper)
		{}

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
		LimitLinearRange(float lower, float upper, const b3d::Spring& spring, float restitution = 0.0f)
			: LimitCommon(spring, restitution), Lower(lower), Upper(upper)
		{}

		bool operator==(const LimitLinearRange& other) const
		{
			return Lower == other.Lower && Upper == other.Upper && ContactDist == other.ContactDist &&
				Restitution == other.Restitution && Spring == other.Spring;
		}

		/** Lower distance of the limit. Must be less than #upper. */
		float Lower = 0.0f;

		/** Upper distance of the limit. Must be more than #lower. */
		float Upper = 0.0f;
	};

	/** Represents a joint limit between zero a single distance value. */
	struct B3D_SCRIPT_EXPORT(DocumentationGroup(Physics), ExportAsStruct(true)) LimitLinear : LimitCommon
	{
		/** Constructs an empty limit. */
		LimitLinear()
		{}

		/**
		 * Constructs a hard limit. Once the limit is reached the movement of the attached bodies will come to a stop.
		 *
		 * @param	extent		Distance at which the limit becomes active.
		 * @param	contactDist	Distance from the limit at which it becomes active. Allows the solver to activate earlier
		 *						than the limit is reached to avoid breaking the limit. Specify -1 for the default.
		 */
		LimitLinear(float extent, float contactDist = -1.0f)
			: LimitCommon(contactDist), Extent(extent)
		{}

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
		LimitLinear(float extent, const b3d::Spring& spring, float restitution = 0.0f)
			: LimitCommon(spring, restitution), Extent(extent)
		{}

		bool operator==(const LimitLinear& other) const
		{
			return Extent == other.Extent && ContactDist == other.ContactDist && Restitution == other.Restitution &&
				Spring == other.Spring;
		}

		/** Distance at which the limit becomes active. */
		float Extent = 0.0f;
	};

	/** Represents a joint limit between two angles. */
	struct B3D_SCRIPT_EXPORT(DocumentationGroup(Physics), ExportAsStruct(true)) LimitAngularRange : LimitCommon
	{
		/** Constructs an empty limit. */
		LimitAngularRange()
		{}

		/**
		 * Constructs a hard limit. Once the limit is reached the movement of the attached bodies will come to a stop.
		 *
		 * @param	lower		Lower angle of the limit. Must be less than @p upper.
		 * @param	upper		Upper angle of the limit. Must be more than @p lower.
		 * @param	contactDist	Distance from the limit at which it becomes active. Allows the solver to activate earlier
		 *						than the limit is reached to avoid breaking the limit. Specify -1 for the default.
		 */
		LimitAngularRange(Radian lower, Radian upper, float contactDist = -1.0f)
			: LimitCommon(contactDist), Lower(lower), Upper(upper)
		{}

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
		LimitAngularRange(Radian lower, Radian upper, const b3d::Spring& spring, float restitution = 0.0f)
			: LimitCommon(spring, restitution), Lower(lower), Upper(upper)
		{}

		bool operator==(const LimitAngularRange& other) const
		{
			return Lower == other.Lower && Upper == other.Upper && ContactDist == other.ContactDist &&
				Restitution == other.Restitution && Spring == other.Spring;
		}

		/** Lower angle of the limit. Must be less than #upper. */
		B3D_SCRIPT_EXPORT(UIValueRange([ 0, 359 ]))
		Radian Lower = Radian(0.0f);

		/** Upper angle of the limit. Must be less than #lower. */
		B3D_SCRIPT_EXPORT(UIValueRange([ 0, 359 ]))
		Radian Upper = Radian(0.0f);
	};

	/** Represents a joint limit that contraints movement to within an elliptical cone. */
	struct B3D_SCRIPT_EXPORT(DocumentationGroup(Physics), ExportAsStruct(true)) LimitConeRange : LimitCommon
	{
		/** Constructs a limit with a 45 degree cone. */
		LimitConeRange()
		{}

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
			: LimitCommon(contactDist), YLimitAngle(yLimitAngle), ZLimitAngle(zLimitAngle)
		{}

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
		LimitConeRange(Radian yLimitAngle, Radian zLimitAngle, const b3d::Spring& spring, float restitution = 0.0f)
			: LimitCommon(spring, restitution), YLimitAngle(yLimitAngle), ZLimitAngle(zLimitAngle)
		{}

		bool operator==(const LimitConeRange& other) const
		{
			return YLimitAngle == other.YLimitAngle && ZLimitAngle == other.ZLimitAngle &&
				ContactDist == other.ContactDist && Restitution == other.Restitution && Spring == other.Spring;
		}

		/** Y angle of the cone. Movement is constrainted between 0 and this angle on the Y axis. */
		B3D_SCRIPT_EXPORT(UIValueRange([ 0, 180 ]))
		Radian YLimitAngle = Radian(Math::kHalfPi);

		/** Z angle of the cone. Movement is constrainted between 0 and this angle on the Z axis. */
		B3D_SCRIPT_EXPORT(UIValueRange([ 0, 180 ]))
		Radian ZLimitAngle = Radian(Math::kHalfPi);
	};

	/** @} */
} // namespace b3d
