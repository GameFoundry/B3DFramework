//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include <cfloat>

#include "BsCorePrerequisites.h"
#include "Physics/BsPhysicsCommon.h"
#include "Math/BsVector3.h"
#include "Math/BsQuaternion.h"

namespace bs
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
		Collider() = default;
		virtual ~Collider() = default;

		/** @copydoc FCollider::getPosition */
		Vector3 getPosition() const;

		/** @copydoc FCollider::getRotation */
		Quaternion getRotation() const;

		/** @copydoc FCollider::setTransform */
		void setTransform(const Vector3& pos, const Quaternion& rot);

		/** Sets the scale of the collider geometry. */
		virtual void setScale(const Vector3& scale);

		/** Retrieves the scale of the collider geometry. */
		Vector3 getScale() const;

		/** @copydoc FCollider::setIsTrigger */
		void setIsTrigger(bool value);

		/** @copydoc FCollider::getIsTrigger */
		bool getIsTrigger() const;

		/** Determines the Rigidbody that controls this collider (if any). */
		void setRigidbody(Rigidbody* value);

		/** @copydoc Collider::setRigidbody() */
		Rigidbody* getRigidbody() const { return mRigidbody; }

		/** @copydoc FCollider::setMass */
		void setMass(float mass);

		/** @copydoc FCollider::getMass */
		float getMass() const;

		/** @copydoc FCollider::setMaterial */
		void setMaterial(const HPhysicsMaterial& material);

		/** @copydoc FCollider::getMaterial */
		HPhysicsMaterial getMaterial() const;

		/** @copydoc FCollider::setContactOffset */
		void setContactOffset(float value);

		/** @copydoc FCollider::getContactOffset */
		float getContactOffset();

		/** @copydoc FCollider::setRestOffset */
		void setRestOffset(float value);

		/** @copydoc FCollider::getRestOffset */
		float getRestOffset();

		/** @copydoc FCollider::setLayer */
		void setLayer(UINT64 layer);

		/** @copydoc FCollider::getLayer */
		UINT64 getLayer() const;

		/** @copydoc FCollider::setCollisionReportMode */
		void setCollisionReportMode(CollisionReportMode mode);

		/** @copydoc FCollider::getCollisionReportMode */
		CollisionReportMode getCollisionReportMode() const;

		/**
		 * Checks does the ray hit this collider.
		 *
		 * @param[in]	ray		Ray to check.
		 * @param[out]	hit		Information about the hit. Valid only if the method returns true.
		 * @param[in]	maxDist	Maximum distance from the ray origin to search for hits.
		 * @return				True if the ray has hit the collider.
		 */
		bool rayCast(const Ray& ray, PhysicsQueryHit& hit, float maxDist = FLT_MAX) const;

		/**
		 * Checks does the ray hit this collider.
		 *
		 * @param[in]	origin	Origin of the ray to check.
		 * @param[in]	unitDir	Unit direction of the ray to check.
		 * @param[out]	hit		Information about the hit. Valid only if the method returns true.
		 * @param[in]	maxDist	Maximum distance from the ray origin to search for hits.
		 * @return				True if the ray has hit the collider.
		 */
		bool rayCast(const Vector3& origin, const Vector3& unitDir, PhysicsQueryHit& hit,
			float maxDist = FLT_MAX) const;

		/**
		 * Triggered when some object starts interacting with the collider. Only triggered if proper collision report mode
		 * is turned on.
		 */
		Event<void(const CollisionDataRaw&)> onCollisionBegin;
		/**
		 * Triggered for every frame that an object remains interacting with a collider. Only triggered if proper collision
		 * report mode is turned on.
		 */
		Event<void(const CollisionDataRaw&)> onCollisionStay;
		/**
		 * Triggered when some object stops interacting with the collider. Only triggered if proper collision report mode
		 * is turned on.
		 */
		Event<void(const CollisionDataRaw&)> onCollisionEnd;

		/** @name Internal
		 *  @{
		 */

		/** Returns the object containing common collider code. */
		FCollider* GetInternalInternal() const { return mInternal; }

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
		FCollider* mInternal = nullptr;
		PhysicsObjectOwner mOwner;
		Rigidbody* mRigidbody = nullptr;
		Vector3 mScale = Vector3::ONE;
	};

	/** @} */
}
