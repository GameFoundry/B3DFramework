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
		virtual Vector3 getPosition() const = 0;

		/** Returns the rotation of the collider. */
		virtual Quaternion getRotation() const = 0;

		/** Sets the position and rotation of the collider. */
		virtual void setTransform(const Vector3& pos, const Quaternion& rotation) = 0;

		/** 
		 * Enables/disables a collider as a trigger. A trigger will not be used for collisions (objects will pass 
		 * through it), but collision events will still be reported. 
		 */
		virtual void setIsTrigger(bool value) = 0;

		/** @copydoc setIsTrigger() */
		virtual bool getIsTrigger() const = 0;

		/** 
		 * Determines whether the collider is a part of a rigidbody (non-static), or is on its own (static). You should 
		 * change this whenever you are attaching or detaching a collider from a rigidbody.
		 */
		virtual void setIsStatic(bool value) = 0;

		/** @copydoc setIsStatic() */
		virtual bool getIsStatic() const = 0;

		/** 
		 * Determines the mass of the collider. Only relevant if the collider is part of a rigidbody. Ultimately this will
		 * determine the total mass, center of mass and inertia tensors of the parent rigidbody (if they're being calculated
		 * automatically).
		 */
		virtual void setMass(float mass) { mMass = mass; }

		/** @copydoc setMass() */
		virtual float getMass() const { return mMass; }

		/** 
		 * Determines the physical material of the collider. The material determines how objects hitting the collider 
		 * behave. 
		 */
		virtual void setMaterial(const HPhysicsMaterial& material);

		/** @copydoc setMaterial() */
		virtual HPhysicsMaterial getMaterial() const { return mMaterial; }

		/**
		 * Determines how far apart do two shapes need to be away from each other before the physics runtime starts 
		 * generating repelling impulse for them. This distance will be the sum of contact offsets of the two interacting
		 * objects. If objects are moving fast you can increase this value to start generating the impulse earlier and 
		 * potentially prevent the objects from interpenetrating. This value is in meters. Must be positive and greater
		 * than rest offset.
		 *
		 * Also see setRestOffset().
		 */
		virtual void setContactOffset(float value) = 0;

		/** @copydoc setContactOffset() */
		virtual float getContactOffset() const = 0;

		/**
		 * Determines at what distance should two objects resting on one another come to an equilibrium. The value used in
		 * the runtime will be the sum of rest offsets for both interacting objects. This value is in meters. Cannot be 
		 * larger than contact offset.
		 *
		 * Also see setContactOffset().
		 */
		virtual void setRestOffset(float value) = 0;

		/** @copydoc setRestOffset() */
		virtual float getRestOffset() const = 0;

		/** Determines the layer of the collider. Layer controls with which objects will the collider collide. */
		virtual void setLayer(UINT64 layer) = 0;

		/** @copydoc setLayer() */
		virtual UINT64 getLayer() const = 0;

		/** Determines which (if any) collision events are reported. */
		virtual void setCollisionReportMode(CollisionReportMode mode) = 0;

		/** @copydoc setCollisionReportMode() */
		virtual CollisionReportMode getCollisionReportMode() const = 0;

		/** Enables continous collision detect for this collider. Only valid if the collider is a part of a rigidbody. */
		virtual void _setCCD(bool enabled) = 0;
	protected:
		float mMass = 1.0f;
		
		HPhysicsMaterial mMaterial;
	};

	/** @} */
}