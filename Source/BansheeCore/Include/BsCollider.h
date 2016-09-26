//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include <cfloat>

#include "BsCorePrerequisites.h"
#include "BsPhysicsCommon.h"
#include "BsVector3.h"
#include "BsQuaternion.h"

namespace BansheeEngine
{
	/** @addtogroup Physics
	 *  @{
	 */

	/** 
	 * Collider represents physics geometry that can be in multiple states:
	 *  - Default: Static geometry that physics objects can collide with.
	 *  - Trigger: Static geometry that can't be collided with but will report touch events.
	 *  - Dynamic: Dynamic geometry that is a part of a Rigidbody. A set of colliders defines the shape of the parent 
	 *		       rigidbody.
	 */
	class BS_CORE_EXPORT Collider
	{
	public:
		Collider() { }
		virtual ~Collider() { }

		/** @copydoc FCollider::getPosition */
		inline Vector3 getPosition() const;

		/** @copydoc FCollider::getRotation */
		inline Quaternion getRotation() const;

		/** @copydoc FCollider::setTransform */
		inline void setTransform(const Vector3& pos, const Quaternion& rot);

		/** Sets the scale of the collider geometry. */
		virtual void setScale(const Vector3& scale);

		/** Retrieves the scale of the collider geometry. */
		inline Vector3 getScale() const;

		/** @copydoc FCollider::setIsTrigger */
		inline void setIsTrigger(bool value);

		/** @copydoc FCollider::getIsTrigger */
		inline bool getIsTrigger() const;

		/** Sets the Rigidbody that controls this collider (if any). */
		inline void setRigidbody(Rigidbody* value);

		/** Retrieves the Rigidbody that controls this collider (if any). */
		Rigidbody* getRigidbody() const { return mRigidbody; }

		/** @copydoc FCollider::setMass */
		inline void setMass(float mass);

		/** @copydoc FCollider::getMass */
		inline float getMass() const;

		/** @copydoc FCollider::setMaterial */
		inline void setMaterial(const HPhysicsMaterial& material);

		/** @copydoc FCollider::getMaterial */
		inline HPhysicsMaterial getMaterial() const;

		/** @copydoc FCollider::setContactOffset */
		inline void setContactOffset(float value);

		/** @copydoc FCollider::getContactOffset */
		inline float getContactOffset();

		/** @copydoc FCollider::setRestOffset */
		inline void setRestOffset(float value);

		/** @copydoc FCollider::getRestOffset */
		inline float getRestOffset();

		/** @copydoc FCollider::setLayer */
		inline void setLayer(UINT64 layer);

		/** @copydoc FCollider::getLayer */
		inline UINT64 getLayer() const;

		/** Sets a value that determines which (if any) collision events are reported. */
		inline void setCollisionReportMode(CollisionReportMode mode);

		/** Gets a value that determines which (if any) collision events are reported. */
		inline CollisionReportMode getCollisionReportMode() const;

		/** 
		 * Checks does the ray hit this collider. 
		 *
		 * @param[in]	ray		Ray to check.
		 * @param[out]	hit		Information about the hit. Valid only if the method returns true.
		 * @param[in]	maxDist	Maximum distance from the ray origin to search for hits.
		 * @return				True if the ray has hit the collider.
		 */
		inline bool rayCast(const Ray& ray, PhysicsQueryHit& hit, float maxDist = FLT_MAX) const;

		/** 
		 * Checks does the ray hit this collider. 
		 *
		 * @param[in]	origin	Origin of the ray to check.
		 * @param[in]	unitDir	Unit direction of the ray to check.
		 * @param[out]	hit		Information about the hit. Valid only if the method returns true.
		 * @param[in]	maxDist	Maximum distance from the ray origin to search for hits.
		 * @return				True if the ray has hit the collider.
		 */
		inline bool rayCast(const Vector3& origin, const Vector3& unitDir, PhysicsQueryHit& hit,
			float maxDist = FLT_MAX) const;

		/** 
		 * Triggered when some object starts interacting with the collider. Only triggered if proper collision report mode
		 * is turned on. 
		 */
		Event<void(const CollisionData&)> onCollisionBegin;
		/** 
		 * Triggered for every frame that an object remains interacting with a collider. Only triggered if proper collision
		 * report mode is turned on. 
		 */
		Event<void(const CollisionData&)> onCollisionStay;
		/**
		 * Triggered when some object stops interacting with the collider. Only triggered if proper collision report mode
		 * is turned on. 
		 */
		Event<void(const CollisionData&)> onCollisionEnd;

		/** @name Internal
		 *  @{
		 */

		/** Returns the object containing common collider code. */
		FCollider* _getInternal() const { return mInternal; }

		/** 
		 * Sets the object that owns this physics object, if any. Used for high level systems so they can easily map their
		 * high level physics objects from the low level ones returned by various queries and events.
		 */
		void _setOwner(PhysicsOwnerType type, void* owner) { mOwner.type = type; mOwner.ownerData = owner; }

		/** 
		 * Gets the object that owns this physics object, if any. Used for high level systems so they can easily map their
		 * high level physics objects from the low level ones returned by various queries and events.
		 */
		void* _getOwner(PhysicsOwnerType type) const { return mOwner.type == type ? mOwner.ownerData : nullptr; }

		/** @} */
	protected:
		FCollider* mInternal = nullptr;
		PhysicsObjectOwner mOwner;
		Rigidbody* mRigidbody = nullptr;
		Vector3 mScale = Vector3::ONE;
	};

	/** @} */
}
