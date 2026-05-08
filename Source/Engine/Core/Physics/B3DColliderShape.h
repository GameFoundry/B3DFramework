//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DPrerequisites.h"
#include "Physics/B3DPhysicsCommon.h"
#include "Math/B3DVector3.h"
#include "Math/B3DQuaternion.h"
#include <variant>

namespace b3d
{
	/** @addtogroup Physics
	 *  @{
	 */

	/** Supported collider shapes. */
	enum class B3D_SCRIPT_EXPORT(DocumentationGroup(Physics)) ColliderShapeType
	{
		Plane,
		Box,
		Sphere,
		Capsule,
		Mesh
	};

	/** Information describing a plane collider shape that extends infinitely in the X/Z axes. */
	struct B3D_SCRIPT_EXPORT(ExportAsStruct(true), DocumentationGroup(Physics)) PlaneColliderShapeInformation
	{
		PlaneColliderShapeInformation() = default;
	};

	/** Information describing box collider shape defined by its extents (half-size). */
	struct B3D_SCRIPT_EXPORT(ExportAsStruct(true), DocumentationGroup(Physics)) BoxColliderShapeInformation
	{
		BoxColliderShapeInformation() = default;
		BoxColliderShapeInformation(const Vector3& extents)
			: Extents(extents)
		{ }

		Vector3 Extents = Vector3(0.5f, 0.5f, 0.5f); /**< Half-size of the box (Distance from center to one side of the box). */
	};

	/** Information describing a sphere collider shape defined by is radius. */
	struct B3D_SCRIPT_EXPORT(ExportAsStruct(true), DocumentationGroup(Physics)) SphereColliderShapeInformation
	{
		SphereColliderShapeInformation(float radius = 1.0f)
			: Radius(radius)
		{ }

		float Radius;
	};

	/** Information describing a capsule collider shape defined by its radius and half-height. */
	struct B3D_SCRIPT_EXPORT(ExportAsStruct(true), DocumentationGroup(Physics)) CapsuleColliderShapeInformation
	{
		CapsuleColliderShapeInformation(float radius = 0.5f, float halfHeight = 0.5f)
			: Radius(radius), HalfHeight(halfHeight)
		{ }

		float Radius;
		float HalfHeight;
	};

	/** Information describing a mesh collider shape. */
	struct B3D_SCRIPT_EXPORT(ExportAsStruct(true) DocumentationGroup(Physics)) MeshColliderShapeInformation
	{
		MeshColliderShapeInformation(const HPhysicsMesh& mesh = nullptr)
			: Mesh(mesh)
		{ }

		HPhysicsMesh Mesh;
	};

	/** Variant type that is able to contain information about all collider shape types. */
	struct ColliderShapeInformation : std::variant<PlaneColliderShapeInformation, BoxColliderShapeInformation, SphereColliderShapeInformation, CapsuleColliderShapeInformation, MeshColliderShapeInformation>
	{
		using Super = std::variant<PlaneColliderShapeInformation, BoxColliderShapeInformation, SphereColliderShapeInformation, CapsuleColliderShapeInformation, MeshColliderShapeInformation>;
		using Super::Super;
	};

	/** Represents a single collider shape that can be assigned to a collider. */
	class B3D_EXPORT B3D_SCRIPT_EXPORT() ColliderShape : public IReflectable, public IScriptExportable
	{
	public:
		ColliderShape();
		~ColliderShape() override = default;

		/** Returns the type of the collider shape. */
		B3D_SCRIPT_EXPORT(Property(Getter), ExportName(Type))
		virtual ColliderShapeType GetType() const = 0;

		/** Position of the collider shape, relative to the parent collider. */
		B3D_SCRIPT_EXPORT(Property(Setter), ExportName(Position))
		virtual void SetPosition(const Vector3& position) { mPosition = position; }

		/** @copydoc SetPosition */
		B3D_SCRIPT_EXPORT(Property(Getter), ExportName(Position))
		Vector3 GetPosition() const { return mPosition; }

		/** Rotation of the collider shape, relative to the parent collider. */
		B3D_SCRIPT_EXPORT(Property(Setter), ExportName(Rotation))
		virtual void SetRotation(const Quaternion& rotation) { mRotation = rotation; }

		B3D_SCRIPT_EXPORT(Property(Getter), ExportName(Rotation))
		Quaternion GetRotation() const { return mRotation; }

		/** Scale of the collider shape, relative to the parent collider. */
		B3D_SCRIPT_EXPORT(Property(Setter), ExportName(Scale))
		virtual void SetScale(const Vector3& scale) { mScale = scale; }

		B3D_SCRIPT_EXPORT(Property(Getter), ExportName(Scale))
		Vector3 GetScale() const { return mScale; }

		/**
		 * Enables/disables a collider as a trigger. A trigger will not be used for collisions (objects will pass
		 * through it), but collision events will still be reported.
		 */
		B3D_SCRIPT_EXPORT(Property(Setter), ExportName(IsTrigger))
		virtual void SetIsTrigger(bool value) { mIsTrigger = value; }

		/** @copydoc SetIsTrigger() */
		B3D_SCRIPT_EXPORT(Property(Getter), ExportName(IsTrigger))
		bool GetIsTrigger() const { return mIsTrigger; }

		/**
		 * Determines the mass of the collider shape. Only relevant if the parent collider is part of a rigidbody. Ultimately this will
		 * determine the total mass, center of mass and inertia tensors of the parent rigidbody (if they're being calculated
		 * automatically).
		 */
		B3D_SCRIPT_EXPORT(Property(Setter), ExportName(Mass))
		virtual void SetMass(float mass) { mMass = mass; }

		/** @copydoc SetMass() */
		B3D_SCRIPT_EXPORT(Property(Getter), ExportName(Mass))
		float GetMass() const { return mMass; }

		/** Determines the physical material of the collider shape. The material determines how objects hitting the collider shape. */
		B3D_SCRIPT_EXPORT(Property(Setter), ExportName(Material))
		virtual void SetMaterial(const HPhysicsMaterial& material) { mMaterial = material; }

		/** @copydoc SetMaterial() */
		B3D_SCRIPT_EXPORT(Property(Getter), ExportName(Material))
		HPhysicsMaterial GetMaterial() const { return mMaterial; }

		/**
		 * Determines how far apart do two shapes need to be away from each other before the physics runtime starts
		 * generating repelling impulse for them. This distance will be the sum of contact offsets of the two interacting
		 * objects. If objects are moving fast you can increase this value to start generating the impulse earlier and
		 * potentially prevent the objects from interpenetrating. This value is in meters. Must be positive and greater
		 * than rest offset.
		 *
		 * Also see SetRestOffset().
		 */
		B3D_SCRIPT_EXPORT(Property(Setter), ExportName(ContactOffset))
		virtual void SetContactOffset(float value) { mContactOffset = value; }

		/** @copydoc SetContactOffset() */
		B3D_SCRIPT_EXPORT(Property(Getter), ExportName(ContactOffset))
		float GetContactOffset() const { return mContactOffset; }

		/**
		 * Determines at what distance should two objects resting on one another come to an equilibrium. The value used in
		 * the runtime will be the sum of rest offsets for both interacting objects. This value is in meters. Cannot be
		 * larger than contact offset.
		 *
		 * Also see SetContactOffset().
		 */
		B3D_SCRIPT_EXPORT(Property(Setter), ExportName(RestOffset))
		virtual void SetRestOffset(float value) { mRestOffset = value; }

		/** @copydoc SetRestOffset() */
		B3D_SCRIPT_EXPORT(Property(Getter), ExportName(RestOffset))
		float GetRestOffset() const { return mRestOffset; }

		/** Determines the layer of the collider shape. Layer controls with which shapes will this shape collide. */
		B3D_SCRIPT_EXPORT(Property(Setter), ExportName(Layer))
		virtual void SetLayer(u64 layer) { mLayer = layer; }

		/** @copydoc SetLayer() */
		B3D_SCRIPT_EXPORT(Property(Getter), ExportName(Layer))
		u64 GetLayer() const { return mLayer; }

		/** Determines which (if any) collision events are reported. */
		B3D_SCRIPT_EXPORT(Property(Setter), ExportName(CollisionReportMode))
		virtual void SetCollisionReportMode(CollisionReportMode mode) { mCollisionReportMode = mode; }

		/** @copydoc SetCollisionReportMode() */
		B3D_SCRIPT_EXPORT(Property(Getter), ExportName(CollisionReportMode))
		CollisionReportMode GetCollisionReportMode() const { return mCollisionReportMode; }

		/** Changes or sets the collider shape to a plane. */
		B3D_SCRIPT_EXPORT()
		virtual void SetShape(const PlaneColliderShapeInformation& information) = 0;

		/** Changes or sets the collider shape to a box. */
		B3D_SCRIPT_EXPORT()
		virtual void SetShape(const BoxColliderShapeInformation& information) = 0;

		/** Changes or sets the collider shape to a sphere. */
		B3D_SCRIPT_EXPORT()
		virtual void SetShape(const SphereColliderShapeInformation& information) = 0;

		/** Changes or sets the collider shape to a capsule. */
		B3D_SCRIPT_EXPORT()
		virtual void SetShape(const CapsuleColliderShapeInformation& information) = 0;

		/** Changes or sets the collider shape to a mesh. */
		B3D_SCRIPT_EXPORT()
		virtual void SetShape(const MeshColliderShapeInformation& information) = 0;

		/** Returns information about the plane shape. Will return default shape information if the current shape is not a plane. */
		B3D_SCRIPT_EXPORT(Property(Getter), ExportName(PlaneShapeInformation))
		PlaneColliderShapeInformation GetPlaneShapeInformation() const;

		/** Returns information about the box shape. Will return default shape information if the current shape is not a box. */
		B3D_SCRIPT_EXPORT(Property(Getter), ExportName(BoxShapeInformation))
		BoxColliderShapeInformation GetBoxShapeInformation() const;

		/** Returns information about the sphere shape. Will return default shape information if the current shape is not a sphere. */
		B3D_SCRIPT_EXPORT(Property(Getter), ExportName(SphereShapeInformation))
		SphereColliderShapeInformation GetSphereShapeInformation() const;

		/** Returns information about the capsule shape. Will return default shape information if the current shape is not a capsule. */
		B3D_SCRIPT_EXPORT(Property(Getter), ExportName(CapsuleShapeInformation))
		CapsuleColliderShapeInformation GetCapsuleShapeInformation() const;

		/** Returns information about the mesh shape. Will return default shape information if the current shape is not a mesh. */
		B3D_SCRIPT_EXPORT(Property(Getter), ExportName(MeshShapeInformation))
		MeshColliderShapeInformation GetMeshShapeInformation() const;

		/** Creates a new plane collider shape. */
		B3D_SCRIPT_EXPORT(ExtensionConstructorForType(ColliderShape))
		static TShared<ColliderShape> CreatePlane(const PlaneColliderShapeInformation& information);

		/** Creates a new box collider shape. */
		B3D_SCRIPT_EXPORT(ExtensionConstructorForType(ColliderShape))
		static TShared<ColliderShape> CreateBox(const BoxColliderShapeInformation& information);

		/** Creates a new sphere collider shape. */
		B3D_SCRIPT_EXPORT(ExtensionConstructorForType(ColliderShape))
		static TShared<ColliderShape> CreateSphere(const SphereColliderShapeInformation& information);

		/** Creates a new capsule collider shape. */
		B3D_SCRIPT_EXPORT(ExtensionConstructorForType(ColliderShape))
		static TShared<ColliderShape> CreateCapsule(const CapsuleColliderShapeInformation& information);

		/** Creates a new mesh collider shape. */
		B3D_SCRIPT_EXPORT(ExtensionConstructorForType(ColliderShape))
		static TShared<ColliderShape> CreateMesh(const MeshColliderShapeInformation& information);

		/** Creates a new collider shape without any shape initialized. */
		static TShared<ColliderShape> CreateEmpty();

		/**
		 * @name Internal
		 * @{
		 */

		/** Notifies the shape that the provided colliders owns it. */
		void SetParentCollider(Collider* collider) { mParentCollider = collider; }

		/** Returns the collider the shape is currently attached to, if any. */
		Collider* GetParentCollider() const { return mParentCollider; }

		/** Index of the shape in the parent collider. */
		void SetShapeIndexInParent(u32 index) { mShapeIndexInParent = index; }

		/** @copydoc SetShapeIndexInParent */
		u32 GetShapeIndexInParent() const { return mShapeIndexInParent; }

		/**
		 * Determines if continuous collision detection is enabled or disabled. When disabled it prevents fast moving objects from
		 * passing through obstacles, at the cost of extra performance.
		 */
		virtual void SetContinuousCollisionDetection(bool value) { mContinuousCollisionDetectionEnabled = value; }

		/** @copydoc SetContinuousCollisionDetection() */
		bool GetContinuousCollisionDetection() const { return mContinuousCollisionDetectionEnabled; }

		/** @} */

	protected:
		friend class Collider;

		/** Updates the local shape transform based on the requested local transform values, and the object the shape is currently attached to. */
		virtual void UpdateTransform() = 0;

		Vector3 mPosition = Vector3::kZero;
		Quaternion mRotation = Quaternion::kIdentity;
		Vector3 mScale = Vector3::kOne;

		u32 mShapeIndexInParent = ~0u;
		Collider* mParentCollider = nullptr;

		HPhysicsMaterial mMaterial;

		ColliderShapeInformation mShapeInformation;

		u64 mLayer = 1;
		float mMass = 1.0f;
		float mRestOffset = 0.0f;
		float mContactOffset = 0.02f;
		CollisionReportMode mCollisionReportMode = CollisionReportMode::None;
		bool mContinuousCollisionDetectionEnabled;
		bool mIsTrigger;

		/************************************************************************/
		/* 								RTTI		                     		*/
		/************************************************************************/
	public:
		friend class ColliderShapeRTTI;
		static RTTIType* GetRttiStatic();
		RTTIType* GetRtti() const override;
	};

	/** @} */
} // namespace b3d
