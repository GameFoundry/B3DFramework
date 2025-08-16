//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsCorePrerequisites.h"
#include "Physics/BsCollider.h"
#include "Physics/BsColliderShape.h"
#include "Scene/BsComponent.h"

namespace b3d
{
	/** @addtogroup Components-Core
	 *  @{
	 */

	/**
	 * Physics object that can be interacted with in the physics world. It can either block other physics objects, or act as a trigger. Trigger will report
	 * collision events, but won't actually prevent other physical objects from going through them.
	 *
	 * This object is intended to remain static in the world. You /can/ move it, but it will not interact with the physics world correctly when moved. For that case
	 * use Rigidbody instead.
	 */
	class B3D_CORE_EXPORT B3D_SCRIPT_EXPORT(DocumentationGroup(Physics)) Collider : public Component
	{
	public:
		Collider(const HSceneObject& parent);
		virtual ~Collider() = default;

		/** Determines if the collider is a trigger. Trigger collider will not prevent objects from going through its shapes but it will still report collision events. */
		B3D_SCRIPT_EXPORT(ExportName(Trigger), Property(Setter))
		void SetIsTrigger(bool value);

		/** @copydoc SetIsTrigger */
		B3D_SCRIPT_EXPORT(ExportName(Trigger), Property(Getter))
		bool GetIsTrigger() const { return mIsTrigger; }

		/** @copydoc ColliderShape::SetMass */
		B3D_SCRIPT_EXPORT(ExportName(Mass), Property(Setter))
		void SetMass(float mass);

		/** @copydoc ColliderShape::GetMass */
		B3D_SCRIPT_EXPORT(ExportName(Mass), Property(Getter))
		float GetMass() const { return mMass; }

		/** @copydoc ColliderShape::SetMaterial */
		B3D_SCRIPT_EXPORT(ExportName(Material), Property(Setter))
		void SetMaterial(const HPhysicsMaterial& material);

		/** @copydoc ColliderShape::GetMaterial */
		B3D_SCRIPT_EXPORT(ExportName(Material), Property(Getter))
		HPhysicsMaterial GetMaterial() const { return mMaterial; }

		/** @copydoc ColliderShape::SetContactOffset */
		B3D_SCRIPT_EXPORT(ExportName(ContactOffset), Property(Setter))
		void SetContactOffset(float value);

		/** @copydoc ColliderShape::GetContactOffset */
		B3D_SCRIPT_EXPORT(ExportName(ContactOffset), Property(Getter))
		float GetContactOffset() const { return mContactOffset; }

		/** @copydoc ColliderShape::SetRestOffset */
		B3D_SCRIPT_EXPORT(ExportName(RestOffset), Property(Setter))
		void SetRestOffset(float value);

		/** @copydoc ColliderShape::GetRestOffset */
		B3D_SCRIPT_EXPORT(ExportName(RestOffset), Property(Getter))
		float GetRestOffset() const { return mRestOffset; }

		/** @copydoc ColliderShape::SetLayer */
		B3D_SCRIPT_EXPORT(ExportName(Layer), Property(Setter), UI(AsLayerMask))
		void SetLayer(u64 layer);

		/** @copydoc ColliderShape::GetLayer */
		B3D_SCRIPT_EXPORT(ExportName(Layer), Property(Getter), UI(AsLayerMask))
		u64 GetLayer() const { return mLayer; }

		/** @copydoc ColliderShape::SetCollisionReportMode */
		B3D_SCRIPT_EXPORT(ExportName(CollisionReportMode), Property(Setter))
		void SetCollisionReportMode(CollisionReportMode mode);

		/** @copydoc ColliderShape::GetCollisionReportMode */
		B3D_SCRIPT_EXPORT(ExportName(CollisionReportMode), Property(Getter))
		CollisionReportMode GetCollisionReportMode() const { return mCollisionReportMode; }

		/** Determines the Rigidbody that controls this collider (if any). */
		HRigidbody GetRigidbody() const { return mParentDynamicRigidbody; }

		/** Returns all the shapes associated with this collider. */
		TInlineArray<SPtr<ColliderShape>, 1> GetShapes() const { return mShapes; }

		/**
		 * Checks does the ray hit this collider.
		 *
		 * @param	ray				Ray to check.
		 * @param	outHit			Information about the hit. Valid only if the method returns true.
		 * @param	maximumDistance	Maximum distance from the ray origin to search for hits.
		 * @return			True if the ray has hit the collider.
		 */
		bool RayCast(const Ray& ray, PhysicsQueryHit& outHit, float maximumDistance = FLT_MAX) const;

		/**
		 * Checks does the ray hit this collider.
		 *
		 * @param	origin			Origin of the ray to check.
		 * @param	direction		Unit direction of the ray to check.
		 * @param	outHit			Information about the hit. Valid only if the method returns true.
		 * @param	maximumDistance	Maximum distance from the ray origin to search for hits.
		 * @return					True if the ray has hit the collider.
		 */
		bool RayCast(const Vector3& origin, const Vector3& direction, PhysicsQueryHit& outHit, float maximumDistance = FLT_MAX) const;

		/** Triggered when some object starts interacting with the collider. Only triggered if proper collision report mode is turned on. */
		B3D_SCRIPT_EXPORT(ExportName(OnCollisionBegin))
		Event<void(const CollisionData&)> OnCollisionBegin;

		/** Triggered for every frame that an object remains interacting with a collider. Only triggered if proper collision report mode is turned on. */
		B3D_SCRIPT_EXPORT(ExportName(OnCollisionStay))
		Event<void(const CollisionData&)> OnCollisionStay;

		/** Triggered when some object stops interacting with the collider. Only triggered if proper collision report mode is turned on. */
		B3D_SCRIPT_EXPORT(ExportName(OnCollisionEnd))
		Event<void(const CollisionData&)> OnCollisionEnd;

		/**
		 * @name Internal
		 * @{
		 */

		// TODO - Remove. Just directly use SO position/rotation and remove special handling for rigibodies in CCollider & ColliderShape UpdateTransform
		const Vector3& GetAdjustedPosition() const { return mAdjustedPosition; }
		const Quaternion& GetAdjustedRotation() const { return mAdjustedRotation; }

		/** Populates CollisionData structure for a collision event involving this component. */
		CollisionData PopulateCollisionData(const CollisionDataRaw& data); // TODO - Shouldn't be necessary, pass CollisionData directly from Physics subsystem

		/** @} */

		/************************************************************************/
		/* 						COMPONENT OVERRIDES                      		*/
		/************************************************************************/
	protected:
		friend class SceneObject;
		friend class CRigidbody;

		void OnCreated() override;
		void OnDestroyed() override;
		void OnDisabled() override;
		void OnEnabled() override;
		void OnTransformChanged(TransformChangedFlags flags) override;

	protected:
		/**
		 * Checks is the provided rigidbody a valid parent for this collider.
		 *
		 * @note This is required because certain colliders are limited in how they can be used. */
		virtual bool IsValidParent(const HRigidbody& parent) const { return true; }

		/**
		 * Changes the dynamic rigidbody parent of the collider. When set all shapes are parented
		 * this this rigidbody instead of the internal static rigidbody. Returns true if the provided rigidbody
		 * is different from existing one.
		 */
		bool SetDynamicRigidbody(const HRigidbody& rigidbody);

		/**
		 * Updates the transform of the internal Collider representation from the transform of the component's scene object.
		 */
		void UpdateTransform(bool updateShapeTransforms = true);

		/** Applies the collision report mode to the internal collider depending on the current state. */
		void UpdateCollisionReportMode();

		/**
		 * Searches the parent scene object hierarchy to find a parent dynamic Rigidbody component, and determines if shapes need to be
		 * detached/attached if the parent rigidbody changed, was added or removed. Returns true if parent changed.
		 */
		bool RefreshParentRigidbody();

		TInlineArray<SPtr<ColliderShape>, 1> mShapes;
		SPtr<StaticRigidbody> mStaticRigidbody; // TODO - Can be unique ptr

		u64 mLayer = 1;
		CollisionReportMode mCollisionReportMode = CollisionReportMode::None;
		float mRestOffset = 0.0f;
		float mContactOffset = 0.02f;
		HPhysicsMaterial mMaterial;
		float mMass = 1.0f;
		bool mIsTrigger = false;

		HRigidbody mParentDynamicRigidbody;

		// TODO - We should remove this, I'm not sure why it's needed
		Vector3 mAdjustedPosition{BsZero};
		Quaternion mAdjustedRotation{BsIdentity};

		/************************************************************************/
		/* 								RTTI		                     		*/
		/************************************************************************/
	public:
		friend class ColliderRTTI;
		static RTTIType* GetRttiStatic();
		RTTIType* GetRtti() const override;

	protected:
		Collider(); // Serialization only
	};

	/** @} */
} // namespace b3d
