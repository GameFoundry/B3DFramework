//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsCorePrerequisites.h"
#include "Physics/BsCollider.h"
#include "Scene/BsComponent.h"

namespace bs
{
	/** @addtogroup Components-Core
	 *  @{
	 */

	/**
	 * @copydoc	Collider
	 *
	 * Wraps Collider as a Component.
	 */
	class BS_CORE_EXPORT BS_SCRIPT_EXPORT(DocumentationGroup(Physics), ExportName(Collider)) CCollider : public Component
	{
	public:
		CCollider(const HSceneObject& parent);
		virtual ~CCollider() = default;

		/** @copydoc Collider::setIsTrigger */
		BS_SCRIPT_EXPORT(ExportName(Trigger), Property(Setter))
		void SetIsTrigger(bool value);

		/** @copydoc Collider::getIsTrigger */
		BS_SCRIPT_EXPORT(ExportName(Trigger), Property(Getter))

		bool GetIsTrigger() const { return mIsTrigger; }

		/** @copydoc Collider::setMass */
		BS_SCRIPT_EXPORT(ExportName(Mass), Property(Setter))
		void SetMass(float mass);

		/** @copydoc Collider::getMass */
		BS_SCRIPT_EXPORT(ExportName(Mass), Property(Getter))

		float GetMass() const { return mMass; }

		/** @copydoc Collider::setMaterial */
		BS_SCRIPT_EXPORT(ExportName(Material), Property(Setter))
		void SetMaterial(const HPhysicsMaterial& material);

		/** @copydoc Collider::getMaterial */
		BS_SCRIPT_EXPORT(ExportName(Material), Property(Getter))

		HPhysicsMaterial GetMaterial() const { return mMaterial; }

		/** @copydoc Collider::setContactOffset */
		BS_SCRIPT_EXPORT(ExportName(ContactOffset), Property(Setter))
		void SetContactOffset(float value);

		/** @copydoc Collider::getContactOffset */
		BS_SCRIPT_EXPORT(ExportName(ContactOffset), Property(Getter))

		float GetContactOffset() const { return mContactOffset; }

		/** @copydoc Collider::setRestOffset */
		BS_SCRIPT_EXPORT(ExportName(RestOffset), Property(Setter))
		void SetRestOffset(float value);

		/** @copydoc Collider::getRestOffset */
		BS_SCRIPT_EXPORT(ExportName(RestOffset), Property(Getter))

		float GetRestOffset() const { return mRestOffset; }

		/** @copydoc Collider::setLayer */
		BS_SCRIPT_EXPORT(ExportName(Layer), Property(Setter), UI(AsLayerMask))
		void SetLayer(u64 layer);

		/** @copydoc Collider::getLayer */
		BS_SCRIPT_EXPORT(ExportName(Layer), Property(Getter), UI(AsLayerMask))

		u64 GetLayer() const { return mLayer; }

		/** @copydoc Collider::setCollisionReportMode */
		BS_SCRIPT_EXPORT(ExportName(CollisionReportMode), Property(Setter))
		void SetCollisionReportMode(CollisionReportMode mode);

		/** @copydoc Collider::getCollisionReportMode */
		BS_SCRIPT_EXPORT(ExportName(CollisionReportMode), Property(Getter))

		CollisionReportMode GetCollisionReportMode() const { return mCollisionReportMode; }

		/** @copydoc Collider::getRigidbody */
		HRigidbody GetRigidbody() const { return mParent; }

		/** @copydoc Collider::rayCast(const Ray&, PhysicsQueryHit&, float) const */
		bool RayCast(const Ray& ray, PhysicsQueryHit& hit, float maxDist = FLT_MAX) const;

		/** @copydoc Collider::rayCast(const Vector3&, const Vector3&, PhysicsQueryHit&, float) const */
		bool RayCast(const Vector3& origin, const Vector3& unitDir, PhysicsQueryHit& hit, float maxDist = FLT_MAX) const;

		/** @copydoc Collider::onCollisionBegin */
		BS_SCRIPT_EXPORT(ExportName(OnCollisionBegin))
		Event<void(const CollisionData&)> OnCollisionBegin;

		/** @copydoc Collider::onCollisionStay */
		BS_SCRIPT_EXPORT(ExportName(OnCollisionStay))
		Event<void(const CollisionData&)> OnCollisionStay;

		/** @copydoc Collider::onCollisionEnd */
		BS_SCRIPT_EXPORT(ExportName(OnCollisionEnd))
		Event<void(const CollisionData&)> OnCollisionEnd;

		/** @name Internal
		 *  @{
		 */

		/** Returns the Collider implementation wrapped by this component. */
		Collider* GetInternalInternal() const { return mInternal.get(); }

		/** @} */

		/************************************************************************/
		/* 						COMPONENT OVERRIDES                      		*/
		/************************************************************************/
	protected:
		friend class SceneObject;
		friend class CRigidbody;

		/** @copydoc Component::onInitialized() */
		void OnInitialized() override;

		/** @copydoc Component::onDestroyed() */
		void OnDestroyed() override;

		/** @copydoc Component::onDisabled() */
		void OnDisabled() override;

		/** @copydoc Component::onEnabled() */
		void OnEnabled() override;

		/** @copydoc Component::onTransformChanged() */
		void OnTransformChanged(TransformChangedFlags flags) override;

	protected:
		using Component::DestroyInternal;

		/** Creates the internal representation of the Collider for use by the component. */
		virtual SPtr<Collider> CreateInternal() = 0;

		/** Creates the internal representation of the Collider and restores the values saved by the Component. */
		virtual void RestoreInternal();

		/** Destroys the internal collider representation. */
		void DestroyInternal();

		/**
		 * Checks is the provided rigidbody a valid parent for this collider.
		 *
		 * @note This is required because certain colliders are limited in how they can be used. */
		virtual bool IsValidParent(const HRigidbody& parent) const { return true; }

		/**
		 * Changes the rigidbody parent of the collider. Meant to be called from the Rigidbody itself.
		 *
		 * @param[in] rigidbody		New rigidbody to assign as the parent to the collider.
		 * @param[in] internal		If true the rigidbody will just be changed internally, but parent rigidbody will not be
		 *							notified.
		 */
		void SetRigidbody(const HRigidbody& rigidbody, bool internal = false);

		/**
		 * Updates the transform of the internal Collider representation from the transform of the component's scene object.
		 */
		void UpdateTransform();

		/** Applies the collision report mode to the internal collider depending on the current state. */
		void UpdateCollisionReportMode();

		/** Searches the parent scene object hierarchy to find a parent Rigidbody component. */
		void UpdateParentRigidbody();

		/** Triggered when the internal collider begins touching another object. */
		void TriggerOnCollisionBegin(const CollisionDataRaw& data);

		/** Triggered when the internal collider continues touching another object. */
		void TriggerOnCollisionStay(const CollisionDataRaw& data);

		/** Triggered when the internal collider ends touching another object. */
		void TriggerOnCollisionEnd(const CollisionDataRaw& data);

		SPtr<Collider> mInternal;

		u64 mLayer = 1;
		CollisionReportMode mCollisionReportMode = CollisionReportMode::None;
		float mRestOffset = 0.0f;
		float mContactOffset = 0.02f;
		HPhysicsMaterial mMaterial;
		float mMass = 1.0f;
		bool mIsTrigger = false;
		Vector3 mLocalPosition = Vector3::kZero;
		Quaternion mLocalRotation = Quaternion::kIdentity;

		HRigidbody mParent;

		/************************************************************************/
		/* 								RTTI		                     		*/
		/************************************************************************/
	public:
		friend class CColliderRTTI;
		static RTTITypeBase* GetRttiStatic();
		RTTITypeBase* GetRtti() const override;

	protected:
		CCollider(); // Serialization only
	};

	/** @} */
} // namespace bs
