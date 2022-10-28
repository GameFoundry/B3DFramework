//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsCorePrerequisites.h"
#include "Physics/BsPhysicsCommon.h"
#include "Math/BsVector3.h"
#include "Math/BsQuaternion.h"

namespace bs
{
	/** @addtogroup Physics-Internal
	 *  @{
	 */

	/** Provides common functionality used by all Collider types. */
	class BS_CORE_EXPORT FCollider
	{
	public:
		virtual ~FCollider() = default;

		/** Returns the position of the collider. */
		virtual Vector3 GetPosition() const = 0;

		/** Returns the rotation of the collider. */
		virtual Quaternion GetRotation() const = 0;

		/** Sets the position and rotation of the collider. */
		virtual void SetTransform(const Vector3& pos, const Quaternion& rotation) = 0;

		/**
		 * Enables/disables a collider as a trigger. A trigger will not be used for collisions (objects will pass
		 * through it), but collision events will still be reported.
		 */
		virtual void SetIsTrigger(bool value) = 0;

		/** @copydoc SetIsTrigger() */
		virtual bool GetIsTrigger() const = 0;

		/**
		 * Determines whether the collider is a part of a rigidbody (non-static), or is on its own (static). You should
		 * change this whenever you are attaching or detaching a collider from a rigidbody.
		 */
		virtual void SetIsStatic(bool value) = 0;

		/** @copydoc SetIsStatic() */
		virtual bool GetIsStatic() const = 0;

		/**
		 * Determines the mass of the collider. Only relevant if the collider is part of a rigidbody. Ultimately this will
		 * determine the total mass, center of mass and inertia tensors of the parent rigidbody (if they're being calculated
		 * automatically).
		 */
		virtual void SetMass(float mass) { mMass = mass; }

		/** @copydoc SetMass() */
		virtual float GetMass() const { return mMass; }

		/**
		 * Determines the physical material of the collider. The material determines how objects hitting the collider
		 * behave.
		 */
		virtual void SetMaterial(const HPhysicsMaterial& material);

		/** @copydoc SetMaterial() */
		virtual HPhysicsMaterial GetMaterial() const { return mMaterial; }

		/**
		 * Determines how far apart do two shapes need to be away from each other before the physics runtime starts
		 * generating repelling impulse for them. This distance will be the sum of contact offsets of the two interacting
		 * objects. If objects are moving fast you can increase this value to start generating the impulse earlier and
		 * potentially prevent the objects from interpenetrating. This value is in meters. Must be positive and greater
		 * than rest offset.
		 *
		 * Also see SetRestOffset().
		 */
		virtual void SetContactOffset(float value) = 0;

		/** @copydoc SetContactOffset() */
		virtual float GetContactOffset() const = 0;

		/**
		 * Determines at what distance should two objects resting on one another come to an equilibrium. The value used in
		 * the runtime will be the sum of rest offsets for both interacting objects. This value is in meters. Cannot be
		 * larger than contact offset.
		 *
		 * Also see SetContactOffset().
		 */
		virtual void SetRestOffset(float value) = 0;

		/** @copydoc SetRestOffset() */
		virtual float GetRestOffset() const = 0;

		/** Determines the layer of the collider. Layer controls with which objects will the collider collide. */
		virtual void SetLayer(u64 layer) = 0;

		/** @copydoc SetLayer() */
		virtual u64 GetLayer() const = 0;

		/** Determines which (if any) collision events are reported. */
		virtual void SetCollisionReportMode(CollisionReportMode mode) = 0;

		/** @copydoc SetCollisionReportMode() */
		virtual CollisionReportMode GetCollisionReportMode() const = 0;

		/** Enables continous collision detect for this collider. Only valid if the collider is a part of a rigidbody. */
		virtual void SetCCDInternal(bool enabled) = 0;

	protected:
		float mMass = 1.0f;

		HPhysicsMaterial mMaterial;
	};

	/** @} */
} // namespace bs
