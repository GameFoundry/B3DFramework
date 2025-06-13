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
	class PhysicsScene;
}

namespace bs
{
	class ColliderShape;
	/** @addtogroup Physics
	 *  @{
	 */

	/**
	 * Can contain one or multiple collider shapes that can be interacted with in the physics world. Each collider can be in multiple states:
	 *  - Static: Static geometry that physics objects can collide with.
	 *  - Trigger: Static geometry that can't be collided with but will report touch events.
	 *  - Dynamic: Dynamic geometry that is a part of a Rigidbody. 
	 */
	class B3D_CORE_EXPORT Collider
	{
	public:
		Collider(PhysicsScene& physicsScene, const Vector3& position = Vector3::kZero, const Quaternion& rotation = Quaternion::kIdentity, const Vector3& scale = Vector3::kOne);
		virtual ~Collider() = default;

		/** Returns the position of the collider. */
		Vector3 GetPosition() const { return mPosition; }

		/** Returns the rotation of the collider. */
		Quaternion GetRotation() const { return mRotation; }

		/** Changes the position and rotation of the collider. All child shapes will maintain relative position and rotation to the collider. */
		void SetTransform(const Vector3& position, const Quaternion& rotation);

		/** Determines the scale of the collider geometry. Note changing scale may require underlying shapes to be re-created. */
		void SetScale(const Vector3& scale);

		/** @copydoc SetScale */
		Vector3 GetScale() const { return mScale; }

		/** Determines if the collider is a trigger. Trigger collider will not prevent objects from going through its shapes but it will still report collision events. */
		void SetIsTrigger(bool value);

		/** @copydoc SetIsTrigger */
		bool GetIsTrigger() const { return mIsTrigger; }

		/** Determines the Rigidbody that controls this collider (if any). */
		virtual void SetRigidbody(Rigidbody* value);

		/** @copydoc SetRigidbody() */
		Rigidbody* GetRigidbody() const { return mRigidbody; }

		/** Returns all the shapes associated with this collider. */
		TInlineArray<SPtr<ColliderShape>, 1> GetShapes() const { return mShapes; }

		/** Sets shapes to associate with the collider. Existing shapes are cleared. */
		virtual void SetShapes(const TArrayView<SPtr<ColliderShape>>& shapes);

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
		 * Creates a collider with zero shapes. Make sure to assign at least one shape after construction.
		 *
		 * @param	scene		Scene into which to add the collider to.
		 * @param	position	Position of the collider. If collider is attached to a rigidbody, this represents position relative to the rigidbody.
		 *						If collider is not attached to a rigidbody, this represents world space position.
		 * @param	position	Rotation of the collider. If collider is attached to a rigidbody, this represents rotation relative to the rigidbody.
		 *						If collider is not attached to a rigidbody, this represents world space rotation.
		 * @param	scale		Scale of the collider. If collider is attached to a rigidbody, this represents scale relative to the rigidbody.
		 *						If collider is not attached to a rigidbody, this represents world space scale.
		 */
		static SPtr<Collider> Create(PhysicsScene& scene, const Vector3& position = Vector3::kZero, const Quaternion& rotation = Quaternion::kIdentity, const Vector3& scale = Vector3::kOne);

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

		/**
		 * Sets the object that owns this physics object, if any. Used for high level systems so they can easily map their
		 * high level physics objects from the low level ones returned by various queries and events.
		 */
		void SetOwner(PhysicsOwnerType type, void* owner)
		{
			mOwner.Type = type;
			mOwner.OwnerData = owner;
		}

		/**
		 * Gets the object that owns this physics object, if any. Used for high level systems so they can easily map their
		 * high level physics objects from the low level ones returned by various queries and events.
		 */
		void* GetOwner(PhysicsOwnerType type) const { return mOwner.Type == type ? mOwner.OwnerData : nullptr; }

		/** @} */
	protected:
		/** Updates the transforms of all the currently attached shapes and the underlying collider object. Should be called after the collider transform changes. */
		virtual void UpdateTransform();

		Vector3 mPosition = Vector3::kZero;
		Quaternion mRotation = Quaternion::kIdentity;
		Vector3 mScale = Vector3::kOne;

		PhysicsScene& mPhysicsScene;
		TInlineArray<SPtr<ColliderShape>, 1> mShapes;
		bool mIsTrigger = false;

		PhysicsObjectOwner mOwner;
		Rigidbody* mRigidbody = nullptr;

	};

	/** @} */
} // namespace bs
