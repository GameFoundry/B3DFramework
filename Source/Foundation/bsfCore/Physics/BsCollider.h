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

		/** @copydoc FCollider::GetPosition */
		Vector3 GetPosition() const;

		/** @copydoc FCollider::GetRotation */
		Quaternion GetRotation() const;

		/** @copydoc FCollider::SetTransform */
		void SetTransform(const Vector3& pos, const Quaternion& rot);

		/** Sets the scale of the collider geometry. */
		virtual void SetScale(const Vector3& scale);

		/** Retrieves the scale of the collider geometry. */
		Vector3 GetScale() const;

		/** @copydoc FCollider::SetIsTrigger */
		void SetIsTrigger(bool value);

		/** @copydoc FCollider::GetIsTrigger */
		bool GetIsTrigger() const;

		/** Determines the Rigidbody that controls this collider (if any). */
		void SetRigidbody(Rigidbody* value);

		/** @copydoc Collider::SetRigidbody() */
		Rigidbody* GetRigidbody() const { return mRigidbody; }

		/** @copydoc FCollider::SetMass */
		void SetMass(float mass);

		/** @copydoc FCollider::GetMass */
		float GetMass() const;

		/** @copydoc FCollider::SetMaterial */
		void SetMaterial(const HPhysicsMaterial& material);

		/** @copydoc FCollider::GetMaterial */
		HPhysicsMaterial GetMaterial() const;

		/** @copydoc FCollider::SetContactOffset */
		void SetContactOffset(float value);

		/** @copydoc FCollider::GetContactOffset */
		float GetContactOffset();

		/** @copydoc FCollider::SetRestOffset */
		void SetRestOffset(float value);

		/** @copydoc FCollider::GetRestOffset */
		float GetRestOffset();

		/** @copydoc FCollider::SetLayer */
		void SetLayer(u64 layer);

		/** @copydoc FCollider::GetLayer */
		u64 GetLayer() const;

		/** @copydoc FCollider::SetCollisionReportMode */
		void SetCollisionReportMode(CollisionReportMode mode);

		/** @copydoc FCollider::GetCollisionReportMode */
		CollisionReportMode GetCollisionReportMode() const;

		/**
		 * Checks does the ray hit this collider.
		 *
		 * @param[in]	ray		Ray to check.
		 * @param[out]	hit		Information about the hit. Valid only if the method returns true.
		 * @param[in]	maxDist	Maximum distance from the ray origin to search for hits.
		 * @return				True if the ray has hit the collider.
		 */
		bool RayCast(const Ray& ray, PhysicsQueryHit& hit, float maxDist = FLT_MAX) const;

		/**
		 * Checks does the ray hit this collider.
		 *
		 * @param[in]	origin	Origin of the ray to check.
		 * @param[in]	unitDir	Unit direction of the ray to check.
		 * @param[out]	hit		Information about the hit. Valid only if the method returns true.
		 * @param[in]	maxDist	Maximum distance from the ray origin to search for hits.
		 * @return				True if the ray has hit the collider.
		 */
		bool RayCast(const Vector3& origin, const Vector3& unitDir, PhysicsQueryHit& hit, float maxDist = FLT_MAX) const;

		/**
		 * Triggered when some object starts interacting with the collider. Only triggered if proper collision report mode
		 * is turned on.
		 */
		Event<void(const CollisionDataRaw&)> OnCollisionBegin;
		/**
		 * Triggered for every frame that an object remains interacting with a collider. Only triggered if proper collision
		 * report mode is turned on.
		 */
		Event<void(const CollisionDataRaw&)> OnCollisionStay;
		/**
		 * Triggered when some object stops interacting with the collider. Only triggered if proper collision report mode
		 * is turned on.
		 */
		Event<void(const CollisionDataRaw&)> OnCollisionEnd;

		/** @name Internal
		 *  @{
		 */

		/** Returns the object containing common collider code. */
		FCollider* GetInternalInternal() const { return mInternal; }

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
		FCollider* mInternal = nullptr;
		PhysicsObjectOwner mOwner;
		Rigidbody* mRigidbody = nullptr;
		Vector3 mScale = Vector3::kOne;
	};

	/** @} */
} // namespace bs
