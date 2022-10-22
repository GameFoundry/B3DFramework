//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsCorePrerequisites.h"
#include "Physics/BsPhysicsCommon.h"

namespace bs
{
	/** @addtogroup Physics
	 *  @{
	 */

	/** Type of force or torque that can be applied to a rigidbody. */
	enum class BS_SCRIPT_EXPORT(DocumentationGroup(Physics)) ForceMode
	{
		Force, /**< Value applied is a force. */
		Impulse, /**< Value applied is an impulse (a direct change in its linear or angular momentum). */
		Velocity, /**< Value applied is velocity. */
		Acceleration /**< Value applied is accelearation. */
	};

	/** Type of force that can be applied to a rigidbody at an arbitrary point. */
	enum class BS_SCRIPT_EXPORT(DocumentationGroup(Physics)) PointForceMode
	{
		Force, /**< Value applied is a force. */
		Impulse, /**< Value applied is an impulse (a direct change in its linear or angular momentum). */
	};

	/** Flags that control options of a Rigidbody object. */
	enum class BS_SCRIPT_EXPORT(DocumentationGroup(Physics)) RigidbodyFlag
	{
		/** No options. */
		None = 0x00,
		/** Automatically calculate center of mass transform and inertia tensors from child shapes (colliders). */
		AutoTensors = 0x01,
		/** Calculate mass distribution from child shapes (colliders). Only relevant when auto-tensors is on. */
		AutoMass = 0x02,
		/**
		 * Enables continous collision detection. This can prevent fast moving bodies from tunneling through each other.
		 * This must also be enabled globally in Physics otherwise the flag will be ignored.
		 */
		CCD = 0x04
	};

	/**
	 * Rigidbody is a dynamic physics object that can be moved using forces (or directly). It will interact with other
	 * static and dynamic physics objects in the scene accordingly (it will push other non-kinematic rigidbodies,
	 * and collide with static objects).
	 *
	 * The shape and mass of a rigidbody is governed by its colliders. You must attach at least one collider for the
	 * rigidbody to be valid.
	 */
	class BS_CORE_EXPORT Rigidbody
	{
	public:
		/**
		 * Constructs a new rigidbody.
		 *
		 * @param[in]	linkedSO	Scene object that owns this rigidbody. All physics updates applied to this object
		 *							will be transfered to this scene object (the movement/rotation resulting from
		 *							those updates).
		 */
		Rigidbody(const HSceneObject& linkedSO);
		virtual ~Rigidbody() = default;

		/**
		 * Moves the rigidbody to a specific position. This method will ensure physically correct movement, meaning the body
		 * will collide with other objects along the way.
		 */
		virtual void Move(const Vector3& position) = 0;

		/**
		 * Rotates the rigidbody. This method will ensure physically correct rotation, meaning the body will collide with
		 * other objects along the way.
		 */
		virtual void Rotate(const Quaternion& rotation) = 0;

		/** Returns the current position of the rigidbody. */
		virtual Vector3 GetPosition() const = 0;

		/** Returns the current rotation of the rigidbody. */
		virtual Quaternion GetRotation() const = 0;

		/**
		 * Moves and rotates the rigidbody to a specific position. Unlike move() and rotate() this will not transform the
		 * body in a physically correct manner, but will instead "teleport" it immediately to the specified position and
		 * rotation.
		 */
		virtual void SetTransform(const Vector3& pos, const Quaternion& rot) = 0;

		/**
		 * Determines the mass of the object and all of its collider shapes. Only relevant if RigidbodyFlag::AutoMass or
		 * RigidbodyFlag::AutoTensors is turned off. Value of zero means the object is immovable (but can be rotated).
		 */
		virtual void SetMass(float mass) = 0;

		/** @copydoc setMass() */
		virtual float GetMass() const = 0;

		/**
		 * Determines if the body is kinematic. Kinematic body will not move in response to external forces (for example
		 * gravity, or another object pushing it), essentially behaving like collider. Unlike a collider though, you can
		 * still move the object and have other dynamic objects respond correctly (meaning it will push other objects).
		 */
		virtual void SetIsKinematic(bool kinematic) = 0;

		/** @copydoc setIsKinematic() */
		virtual bool GetIsKinematic() const = 0;

		/**
		 * Checks if the body is sleeping. Objects that aren't moved/rotated for a while are put to sleep to reduce load
		 * on the physics system.
		 */
		virtual bool IsSleeping() const = 0;

		/** Forces the object to sleep. Useful if you know the object will not move in any significant way for a while. */
		virtual void Sleep() = 0;

		/**
		 * Wakes an object up. Useful if you modified properties of this object, and potentially surrounding objects which
		 * might result in the object being moved by physics (although the physics system will automatically wake the
		 * object up for majority of such cases).
		 */
		virtual void WakeUp() = 0;

		/** Determines a threshold of force and torque under which the object will be considered to be put to sleep. */
		virtual void SetSleepThreshold(float threshold) = 0;

		/** @copydoc setSleepThreshold() */
		virtual float GetSleepThreshold() const = 0;

		/** Determines whether or not the rigidbody will have the global gravity force applied to it. */
		virtual void SetUseGravity(bool gravity) = 0;

		/** @copydoc setUseGravity() */
		virtual bool GetUseGravity() const = 0;

		/** Determines the linear velocity of the body. */
		virtual void SetVelocity(const Vector3& velocity) = 0;

		/** @copydoc setVelocity() */
		virtual Vector3 GetVelocity() const = 0;

		/** Determines the angular velocity of the body. */
		virtual void SetAngularVelocity(const Vector3& velocity) = 0;

		/** @copydoc setAngularVelocity() */
		virtual Vector3 GetAngularVelocity() const = 0;

		/** Determines the linear drag of the body. Higher drag values means the object resists linear movement more. */
		virtual void SetDrag(float drag) = 0;

		/** @copydoc setDrag() */
		virtual float GetDrag() const = 0;

		/** Determines the angular drag of the body. Higher drag values means the object resists angular movement more. */
		virtual void SetAngularDrag(float drag) = 0;

		/** @copydoc setAngularDrag() */
		virtual float GetAngularDrag() const = 0;

		/**
		 * Determines the inertia tensor in local mass space. Inertia tensor determines how difficult is to rotate the
		 * object. Values of zero in the inertia tensor mean the object will be unable to rotate around a specific axis.
		 * Only relevant if RigidbodyFlag::AutoTensors is turned off.
		 */
		virtual void SetInertiaTensor(const Vector3& tensor) = 0;

		/** @copydoc setInertiaTensor() */
		virtual Vector3 GetInertiaTensor() const = 0;

		/** Determines the maximum angular velocity of the rigidbody. Velocity will be clamped to this value. */
		virtual void SetMaxAngularVelocity(float maxVelocity) = 0;

		/** @copydoc setMaxAngularVelocity() */
		virtual float GetMaxAngularVelocity() const = 0;

		/**
		 * Sets the rigidbody's center of mass transform. Only relevant if RigibodyFlag::AutoTensors is turned off.
		 *
		 * @param[in]	position	Position of the center of mass.
		 * @param[in]	rotation	Rotation that determines orientation of the inertia tensor (rotation of the center of
		 *							mass frame).
		 */
		virtual void SetCenterOfMass(const Vector3& position, const Quaternion& rotation) = 0;

		/** Returns the position of the center of mass. */
		virtual Vector3 GetCenterOfMassPosition() const = 0;

		/** Returns the rotation of the inertia tensor. */
		virtual Quaternion GetCenterOfMassRotation() const = 0;

		/**
		 * Determines the number of iterations to use when solving for position. Higher values can improve precision and
		 * numerical stability of the simulation.
		 */
		virtual void SetPositionSolverCount(u32 count) = 0;

		/** @copydoc setPositionSolverCount() */
		virtual u32 GetPositionSolverCount() const = 0;

		/**
		 * Determines the number of iterations to use when solving for velocity. Higher values can improve precision and
		 * numerical stability of the simulation.
		 */
		virtual void SetVelocitySolverCount(u32 count) = 0;

		/** @copydoc setVelocitySolverCount() */
		virtual u32 GetVelocitySolverCount() const = 0;

		/** Flags that control the behaviour of the rigidbody. */
		virtual void SetFlags(RigidbodyFlag flags) { mFlags = flags; }

		/** @copydoc setFlags() */
		virtual RigidbodyFlag GetFlags() const { return mFlags; }

		/**
		 * Applies a force to the center of the mass of the rigidbody. This will produce linear momentum.
		 *
		 * @param[in]	force		Force to apply.
		 * @param[in]	mode		Determines what is the type of @p force.
		 */
		virtual void AddForce(const Vector3& force, ForceMode mode = ForceMode::Force) = 0;

		/**
		 * Applies a torque to the rigidbody. This will produce angular momentum.
		 *
		 * @param[in]	torque		Torque to apply.
		 * @param[in]	mode		Determines what is the type of @p torque.
		 */
		virtual void AddTorque(const Vector3& torque, ForceMode mode = ForceMode::Force) = 0;

		/**
		 * Applies a force to a specific point on the rigidbody. This will in most cases produce both linear and angular
		 * momentum.
		 *
		 * @param[in]	force		Force to apply.
		 * @param[in]	position	World position to apply the force at.
		 * @param[in]	mode		Determines what is the type of @p force.
		 */
		virtual void AddForceAtPoint(const Vector3& force, const Vector3& position,
			PointForceMode mode = PointForceMode::Force) = 0;

		/**
		 * Returns the total (linear + angular) velocity at a specific point.
		 *
		 * @param[in]	point	Point in world space.
		 * @return				Total velocity of the point.
		 */
		virtual Vector3 GetVelocityAtPoint(const Vector3& point) const = 0;

		/** Registers a new collider as a child of this rigidbody. */
		virtual void AddCollider(Collider* collider) = 0;

		/** Removes a collider from the child list of this rigidbody. */
		virtual void RemoveCollider(Collider* collider) = 0;

		/** Removes all colliders from the child list of this rigidbody. */
		virtual void RemoveColliders() = 0;

		/**
		 * Recalculates rigidbody's mass, inertia tensors and center of mass depending on the currently set child colliders.
		 * This should be called whenever relevant child collider properties change (like mass or shape).
		 *
		 * If automatic tensor calculation is turned off then this will do nothing. If automatic mass calculation is turned
		 * off then this will use the mass set directly on the body using setMass().
		 */
		virtual void UpdateMassDistribution() { }

		/**
		 * Creates a new rigidbody.
		 *
		 * @param[in]	linkedSO	Scene object that owns this rigidbody. All physics updates applied to this object
		 *							will be transfered to this scene object (the movement/rotation resulting from
		 *							those updates).
		 */
		static SPtr<Rigidbody> Create(const HSceneObject& linkedSO);

		/** Triggered when one of the colliders owned by the rigidbody starts colliding with another object. */
		Event<void(const CollisionDataRaw&)> OnCollisionBegin;

		/** Triggered when a previously colliding collider stays in collision. Triggered once per frame. */
		Event<void(const CollisionDataRaw&)> OnCollisionStay;

		/** Triggered when one of the colliders owned by the rigidbody stops colliding with another object. */
		Event<void(const CollisionDataRaw&)> OnCollisionEnd;

		/** @name Internal
		 *  @{
		 */

		/**
		 * Sets the priority of the physics update. Bodies with a higher priority will be updated before the bodies with
		 * lower priority. This allows you to control the order of updated in case rigidbodies are in some way dependant.
		 */
		void SetPriorityInternal(u32 priority);

		/** Sets a unique ID of the rigidbody, so it can be recognized by the physics system. */
		void SetPhysicsIdInternal(u32 id) { mPhysicsId = id; }

		/**
		 * Applies new transform values retrieved from the most recent physics update (values resulting from physics
		 * simulation).
		 */
		void SetTransformInternal(const Vector3& position, const Quaternion& rotation);

		/**
		 * Sets the object that owns this physics object, if any. Used for high level systems so they can easily map their
		 * high level physics objects from the low level ones returned by various queries and events.
		 */
		void SetOwnerInternal(PhysicsOwnerType type, void* owner) { mOwner.Type = type; mOwner.OwnerData = owner; }

		/**
		 * Gets the object that owns this physics object, if any. Used for high level systems so they can easily map their
		 * high level physics objects from the low level ones returned by various queries and events.
		 */
		void* GetOwnerInternal(PhysicsOwnerType type) const { return mOwner.Type == type ? mOwner.OwnerData : nullptr; }

		/** @} */

	protected:
		RigidbodyFlag mFlags = RigidbodyFlag::None;
		PhysicsObjectOwner mOwner;
		u32 mPhysicsId = 0;
		HSceneObject mLinkedSO;
	};

	/** @} */
}
