//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include <cfloat>

#include "B3DColliderShape.h"
#include "B3DPrerequisites.h"
#include "Physics/B3DPhysicsCommon.h"
#include "Utility/B3DModule.h"
#include "Math/B3DVector3.h"
#include "Math/B3DVector2.h"
#include "Math/B3DQuaternion.h"
#include "Script/B3DIScriptExportable.h"

namespace b3d
{
	class IPhysicsMeshImplementation;
	class ICharacterControllerImplementation;
	class IColliderImplementation;
	class ID6JointImplementation;
	class ISliderJointImplementation;
	class ISphericalJointImplementation;
	class IHingeJointImplementation;
	class IDistanceJointImplementation;
	class IFixedJointImplementation;
	class IRigidbodyImplementation;

	/** @addtogroup Physics
	 *  @{
	 */

	struct PhysicsCreateInformation;
	class PhysicsScene;

	/** Flags for controlling physics behaviour globally. */
	enum class PhysicsFlag
	{
		/**
		 * Automatically recovers character controllers that are interpenetrating geometry. This can happen if a controller
		 * is spawned or teleported into geometry, its size/rotation is changed so it penetrates geometry, or simply
		 * because of numerical imprecision.
		 */
		CCT_OverlapRecovery = 1 << 0,
		/**
		 * Performs more accurate sweeps when moving the character controller, making it less likely to interpenetrate
		 * geometry. When overlap recovery is turned on you can consider turning this off as it can compensate for the
		 * less precise sweeps.
		 */
		CCT_PreciseSweeps = 1 << 1,
		/**
		 * Large triangles can cause problems with character controller collision. If this option is enabled the triangles
		 * larger than a certain size will be automatically tesselated into smaller triangles, in order to help with
		 * precision.
		 *
		 * @see Physics::getMaxTesselationEdgeLength
		 */
		CCT_Tesselation = 1 << 2,
		/**
		 * Enables continous collision detection. This will prevent fast-moving objects from tunneling through each other.
		 * You must also enable CCD for individual Rigidbodies. This option can have a significant performance impact.
		 */
		CCD_Enable = 1 << 3
	};

	/** @copydoc CharacterCollisionFlag */
	typedef Flags<PhysicsFlag> PhysicsFlags;
	B3D_FLAGS_OPERATORS(PhysicsFlag)

	/** Provides global physics settings, factory methods for physics objects and scene queries. */
	class B3D_EXPORT B3D_SCRIPT_EXPORT(DocumentationGroup(Physics)) Physics : public Module<Physics>
	{
	public:
		Physics(const PhysicsCreateInformation& init);
		virtual ~Physics() = default;

		/******************************************************************************************************************/
		/************************************************* OPTIONS ********************************************************/
		/******************************************************************************************************************/

		/**
		 * Enables or disables collision between two layers. Each physics object can be assigned a specific layer, and here
		 * you can determine which layers can interact with each other.
		 */
		B3D_SCRIPT_EXPORT(ExportName(ToggleCollision))
		void ToggleCollision(u64 groupA, u64 groupB, bool enabled);

		/** Checks if two collision layers are allowed to interact. */
		B3D_SCRIPT_EXPORT(ExportName(IsCollisionEnabled))
		bool IsCollisionEnabled(u64 groupA, u64 groupB) const;

		/** @name Internal
		 *  @{
		 */

		/******************************************************************************************************************/
		/************************************************* CREATION *******************************************************/
		/******************************************************************************************************************/

		/** @copydoc PhysicsMaterial::Create */
		virtual TShared<PhysicsMaterial> CreateMaterial(float staticFriction, float dynamicFriction, float restitution) = 0;

		/** Creates a physics mesh implementation. See PhysicsMesh::Create. */
		virtual UPtr<IPhysicsMeshImplementation> CreateMesh(const TShared<MeshData>& meshData, PhysicsMeshType type) = 0;

		/** Creates an object representing the physics scene. Must be manually released via destroyPhysicsScene(). */
		virtual TShared<PhysicsScene> CreatePhysicsScene() = 0;

		/** Creates a new empty collider shape. Note you must set the shape information after creation. */
		virtual TShared<ColliderShape> CreateColliderShape() = 0;

		/** Creates an object that provides low-level functionality required for a Collider. */
		virtual UPtr<IColliderImplementation> CreateColliderImplementation() = 0;

		/** Creates an object that provides low-level functionality required for a Rigidbody. */
		virtual UPtr<IRigidbodyImplementation> CreateRigidbodyImplementation(Rigidbody& owner) = 0;

		/**
		 * Checks does the ray hit the provided collider shape.
		 *
		 * @param	origin				Origin of the ray to check.
		 * @param	unitDirection		Unit direction of the ray to check.
		 * @param	colliderShape		Collider shape to check for hit.
		 * @param	hit					Information about the hit. Valid only if the method returns true.
		 * @param	maximumDistance		Maximum distance from the ray origin to search for hits.
		 * @return						True if the ray has hit the collider.
		 */
		virtual bool RayCast(const Vector3& origin, const Vector3& unitDirection, const ColliderShape& colliderShape, PhysicsQueryHit& hit, float maximumDistance = FLT_MAX) const = 0;

		/**
		 * Checks does the ray hit any of the shapes on the provided collider.
		 *
		 * @param	origin				Origin of the ray to check.
		 * @param	unitDirection		Unit direction of the ray to check.
		 * @param	collider				Collider to check for hit.
		 * @param	hit					Information about the hit. Valid only if the method returns true.
		 * @param	maximumDistance		Maximum distance from the ray origin to search for hits.
		 * @return						True if the ray has hit the collider.
		 */
		virtual bool RayCast(const Vector3& origin, const Vector3& unitDirection, const Collider& collider, PhysicsQueryHit& hit, float maximumDistance = FLT_MAX) const = 0;

		/** @} */

		static const u64 kCollisionMapSize = 64;

	protected:
		friend class Rigidbody;

		mutable Mutex mMutex;
		bool mCollisionMap[kCollisionMapSize][kCollisionMapSize];
	};

	/** Provides easier access to Physics. */
	B3D_EXPORT Physics& GetPhysics();

	/** Contains parameters used for initializing the physics system. */
	struct PhysicsCreateInformation
	{
		float TypicalLength = 1.0f; /**< Typical length of an object in the scene. */
		float TypicalSpeed = 9.81f; /**< Typical speed of an object in the scene. */
		Vector3 Gravity = Vector3(0.0f, -9.81f, 0.0f); /**< Initial gravity. */
		bool InitCooking = true; /**< Determines should the cooking library be initialized. */
		/** Flags that control global physics option. */
		PhysicsFlags Flags = PhysicsFlag::CCT_OverlapRecovery | PhysicsFlag::CCT_PreciseSweeps | PhysicsFlag::CCD_Enable;
	};

	/**
	 * Physical representation of a scene, allowing creation of new physical objects in the scene and queries against
	 * those objects. Objects created in different scenes cannot physically interact with eachother.
	 */
	class B3D_EXPORT B3D_SCRIPT_EXPORT(DocumentationGroup(Physics)) PhysicsScene : public IScriptExportable
	{
	public:
		/**
		 * Updates the physics simulation. In order to maintain stability of the physics calculations this method should
		 * be called at fixed intervals (e.g. 60 times a second).
		 *
		 * @param	step	Time delta to advance the physics simulation by, in seconds.
		 */
		virtual void FixedUpdate(float step) = 0;

		/**
		 * Performs any physics operations that arent tied to the fixed update interval. Should be called once per frame.
		 */
		virtual void Update() {}

		/** Pauses or resumes the physics simulation. */
		virtual void SetPaused(bool paused) = 0;

		/** Checks is the physics simulation update currently in progress. */
		bool IsUpdateInProgress() const { return mUpdateInProgress; }

		/******************************************************************************************************************/
		/************************************************* QUERIES ********************************************************/
		/******************************************************************************************************************/

		/**
		 * Casts a ray into the scene and returns the closest found hit, if any.
		 *
		 * @param	ray		Ray to cast into the scene.
		 * @param	hit		Information recorded about a hit. Only valid if method returns true.
		 * @param	layer	Layers to consider for the query. This allows you to ignore certain groups of objects.
		 * @param	max		Maximum distance at which to perform the query. Hits past this distance will not be
		 *						detected.
		 * @return				True if something was hit, false otherwise.
		 */
		B3D_SCRIPT_EXPORT(ExportName(RayCast))
		virtual bool RayCast(const Ray& ray, PhysicsQueryHit& hit, u64 layer = BS_ALL_LAYERS, float max = FLT_MAX) const;

		/**
		 * Casts a ray into the scene and returns the closest found hit, if any.
		 *
		 * @param	origin		Origin of the ray to cast into the scene.
		 * @param	unitDir		Unit direction of the ray to cast into the scene.
		 * @param	hit			Information recorded about a hit. Only valid if method returns true.
		 * @param	layer		Layers to consider for the query. This allows you to ignore certain groups of objects.
		 * @param	max			Maximum distance at which to perform the query. Hits past this distance will not be
		 *							detected.
		 * @return					True if something was hit, false otherwise.
		 */
		B3D_SCRIPT_EXPORT(ExportName(RayCast))
		virtual bool RayCast(const Vector3& origin, const Vector3& unitDir, PhysicsQueryHit& hit, u64 layer = BS_ALL_LAYERS, float max = FLT_MAX) const = 0;

		/**
		 * Performs a sweep into the scene using a box and returns the closest found hit, if any.
		 *
		 * @param	box			Box to sweep through the scene.
		 * @param	rotation	Orientation of the box.
		 * @param	unitDir		Unit direction towards which to perform the sweep.
		 * @param	hit			Information recorded about a hit. Only valid if method returns true.
		 * @param	layer		Layers to consider for the query. This allows you to ignore certain groups of objects.
		 * @param	max			Maximum distance at which to perform the query. Hits past this distance will not be
		 *							detected.
		 * @return					True if something was hit, false otherwise.
		 */
		B3D_SCRIPT_EXPORT(ExportName(BoxCast))
		virtual bool BoxCast(const AABox& box, const Quaternion& rotation, const Vector3& unitDir, PhysicsQueryHit& hit, u64 layer = BS_ALL_LAYERS, float max = FLT_MAX) const = 0;

		/**
		 * Performs a sweep into the scene using a sphere and returns the closest found hit, if any.
		 *
		 * @param	sphere		Sphere to sweep through the scene.
		 * @param	unitDir		Unit direction towards which to perform the sweep.
		 * @param	hit			Information recorded about a hit. Only valid if method returns true.
		 * @param	layer		Layers to consider for the query. This allows you to ignore certain groups of objects.
		 * @param	max			Maximum distance at which to perform the query. Hits past this distance will not be
		 *							detected.
		 * @return					True if something was hit, false otherwise.
		 */
		B3D_SCRIPT_EXPORT(ExportName(SphereCast))
		virtual bool SphereCast(const Sphere& sphere, const Vector3& unitDir, PhysicsQueryHit& hit, u64 layer = BS_ALL_LAYERS, float max = FLT_MAX) const = 0;

		/**
		 * Performs a sweep into the scene using a capsule and returns the closest found hit, if any.
		 *
		 * @param	capsule		Capsule to sweep through the scene.
		 * @param	rotation	Orientation of the capsule.
		 * @param	unitDir		Unit direction towards which to perform the sweep.
		 * @param	hit			Information recorded about a hit. Only valid if method returns true.
		 * @param	layer		Layers to consider for the query. This allows you to ignore certain groups of objects.
		 * @param	max			Maximum distance at which to perform the query. Hits past this distance will not be
		 *							detected.
		 * @return					True if something was hit, false otherwise.
		 */
		B3D_SCRIPT_EXPORT(ExportName(CapsuleCast))
		virtual bool CapsuleCast(const Capsule& capsule, const Quaternion& rotation, const Vector3& unitDir, PhysicsQueryHit& hit, u64 layer = BS_ALL_LAYERS, float max = FLT_MAX) const = 0;

		/**
		 * Performs a sweep into the scene using a convex mesh and returns the closest found hit, if any.
		 *
		 * @param	mesh		Mesh to sweep through the scene. Must be convex.
		 * @param	position	Starting position of the mesh.
		 * @param	rotation	Orientation of the mesh.
		 * @param	unitDir		Unit direction towards which to perform the sweep.
		 * @param	hit			Information recorded about a hit. Only valid if method returns true.
		 * @param	layer		Layers to consider for the query. This allows you to ignore certain groups of objects.
		 * @param	max			Maximum distance at which to perform the query. Hits past this distance will not be
		 *							detected.
		 * @return					True if something was hit, false otherwise.
		 */
		B3D_SCRIPT_EXPORT(ExportName(ConvexCast))
		virtual bool ConvexCast(const HPhysicsMesh& mesh, const Vector3& position, const Quaternion& rotation, const Vector3& unitDir, PhysicsQueryHit& hit, u64 layer = BS_ALL_LAYERS, float max = FLT_MAX) const = 0;

		/**
		 * Casts a ray into the scene and returns all found hits.
		 *
		 * @param	ray		Ray to cast into the scene.
		 * @param	layer	Layers to consider for the query. This allows you to ignore certain groups of objects.
		 * @param	max		Maximum distance at which to perform the query. Hits past this distance will not be
		 *						detected.
		 * @return				List of all detected hits.
		 */
		B3D_SCRIPT_EXPORT(ExportName(RayCastAll))
		virtual Vector<PhysicsQueryHit> RayCastAll(const Ray& ray, u64 layer = BS_ALL_LAYERS, float max = FLT_MAX) const;

		/**
		 * Casts a ray into the scene and returns all found hits.
		 *
		 * @param	origin		Origin of the ray to cast into the scene.
		 * @param	unitDir		Unit direction of the ray to cast into the scene.
		 * @param	layer		Layers to consider for the query. This allows you to ignore certain groups of objects.
		 * @param	max			Maximum distance at which to perform the query. Hits past this distance will not be
		 *							detected.
		 * @return					List of all detected hits.
		 */
		B3D_SCRIPT_EXPORT(ExportName(RayCastAll))
		virtual Vector<PhysicsQueryHit> RayCastAll(const Vector3& origin, const Vector3& unitDir, u64 layer = BS_ALL_LAYERS, float max = FLT_MAX) const = 0;

		/**
		 * Performs a sweep into the scene using a box and returns all found hits.
		 *
		 * @param	box			Box to sweep through the scene.
		 * @param	rotation	Orientation of the box.
		 * @param	unitDir		Unit direction towards which to perform the sweep.
		 * @param	layer		Layers to consider for the query. This allows you to ignore certain groups of objects.
		 * @param	max			Maximum distance at which to perform the query. Hits past this distance will not be
		 *							detected.
		 * @return					List of all detected hits.
		 */
		B3D_SCRIPT_EXPORT(ExportName(BoxCastAll))
		virtual Vector<PhysicsQueryHit> BoxCastAll(const AABox& box, const Quaternion& rotation, const Vector3& unitDir, u64 layer = BS_ALL_LAYERS, float max = FLT_MAX) const = 0;

		/**
		 * Performs a sweep into the scene using a sphere and returns all found hits.
		 *
		 * @param	sphere		Sphere to sweep through the scene.
		 * @param	unitDir		Unit direction towards which to perform the sweep.
		 * @param	layer		Layers to consider for the query. This allows you to ignore certain groups of objects.
		 * @param	max			Maximum distance at which to perform the query. Hits past this distance will not be
		 *							detected.
		 * @return					List of all detected hits.
		 */
		B3D_SCRIPT_EXPORT(ExportName(SphereCastAll))
		virtual Vector<PhysicsQueryHit> SphereCastAll(const Sphere& sphere, const Vector3& unitDir, u64 layer = BS_ALL_LAYERS, float max = FLT_MAX) const = 0;

		/**
		 * Performs a sweep into the scene using a capsule and returns all found hits.
		 *
		 * @param	capsule		Capsule to sweep through the scene.
		 * @param	rotation	Orientation of the capsule.
		 * @param	unitDir		Unit direction towards which to perform the sweep.
		 * @param	layer		Layers to consider for the query. This allows you to ignore certain groups of objects.
		 * @param	max			Maximum distance at which to perform the query. Hits past this distance will not be
		 *							detected.
		 * @return					List of all detected hits.
		 */
		B3D_SCRIPT_EXPORT(ExportName(CapsuleCastAll))
		virtual Vector<PhysicsQueryHit> CapsuleCastAll(const Capsule& capsule, const Quaternion& rotation, const Vector3& unitDir, u64 layer = BS_ALL_LAYERS, float max = FLT_MAX) const = 0;

		/**
		 * Performs a sweep into the scene using a convex mesh and returns all found hits.
		 *
		 * @param	mesh		Mesh to sweep through the scene. Must be convex.
		 * @param	position	Starting position of the mesh.
		 * @param	rotation	Orientation of the mesh.
		 * @param	unitDir		Unit direction towards which to perform the sweep.
		 * @param	layer		Layers to consider for the query. This allows you to ignore certain groups of objects.
		 * @param	max			Maximum distance at which to perform the query. Hits past this distance will not be
		 *							detected.
		 * @return					List of all detected hits.
		 */
		B3D_SCRIPT_EXPORT(ExportName(ConvexCastAll))
		virtual Vector<PhysicsQueryHit> ConvexCastAll(const HPhysicsMesh& mesh, const Vector3& position, const Quaternion& rotation, const Vector3& unitDir, u64 layer = BS_ALL_LAYERS, float max = FLT_MAX) const = 0;

		/**
		 * Casts a ray into the scene and checks if it has hit anything. This can be significantly more efficient than other
		 * types of cast* calls.
		 *
		 * @param	ray		Ray to cast into the scene.
		 * @param	layer	Layers to consider for the query. This allows you to ignore certain groups of objects.
		 * @param	max		Maximum distance at which to perform the query. Hits past this distance will not be
		 *						detected.
		 * @return				True if something was hit, false otherwise.
		 */
		B3D_SCRIPT_EXPORT(ExportName(RayCastAny))
		virtual bool RayCastAny(const Ray& ray, u64 layer = BS_ALL_LAYERS, float max = FLT_MAX) const;

		/**
		 * Casts a ray into the scene and checks if it has hit anything. This can be significantly more efficient than other
		 * types of cast* calls.
		 *
		 * @param	origin		Origin of the ray to cast into the scene.
		 * @param	unitDir		Unit direction of the ray to cast into the scene.
		 * @param	layer		Layers to consider for the query. This allows you to ignore certain groups of objects.
		 * @param	max			Maximum distance at which to perform the query. Hits past this distance will not be
		 *							detected.
		 * @return					True if something was hit, false otherwise.
		 */
		B3D_SCRIPT_EXPORT(ExportName(RayCastAny))
		virtual bool RayCastAny(const Vector3& origin, const Vector3& unitDir, u64 layer = BS_ALL_LAYERS, float max = FLT_MAX) const = 0;

		/**
		 * Performs a sweep into the scene using a box and checks if it has hit anything. This can be significantly more
		 * efficient than other types of cast* calls.
		 *
		 * @param	box			Box to sweep through the scene.
		 * @param	rotation	Orientation of the box.
		 * @param	unitDir		Unit direction towards which to perform the sweep.
		 * @param	layer		Layers to consider for the query. This allows you to ignore certain groups of objects.
		 * @param	max			Maximum distance at which to perform the query. Hits past this distance will not be
		 *							detected.
		 * @return					True if something was hit, false otherwise.
		 */
		B3D_SCRIPT_EXPORT(ExportName(BoxCastAny))
		virtual bool BoxCastAny(const AABox& box, const Quaternion& rotation, const Vector3& unitDir, u64 layer = BS_ALL_LAYERS, float max = FLT_MAX) const = 0;

		/**
		 * Performs a sweep into the scene using a sphere and checks if it has hit anything. This can be significantly more
		 * efficient than other types of cast* calls.
		 *
		 * @param	sphere		Sphere to sweep through the scene.
		 * @param	unitDir		Unit direction towards which to perform the sweep.
		 * @param	layer		Layers to consider for the query. This allows you to ignore certain groups of objects.
		 * @param	max			Maximum distance at which to perform the query. Hits past this distance will not be
		 *							detected.
		 * @return					True if something was hit, false otherwise.
		 */
		B3D_SCRIPT_EXPORT(ExportName(SphereCastAny))
		virtual bool SphereCastAny(const Sphere& sphere, const Vector3& unitDir, u64 layer = BS_ALL_LAYERS, float max = FLT_MAX) const = 0;

		/**
		 * Performs a sweep into the scene using a capsule and checks if it has hit anything. This can be significantly more
		 * efficient than other types of cast* calls.
		 *
		 * @param	capsule		Capsule to sweep through the scene.
		 * @param	rotation	Orientation of the capsule.
		 * @param	unitDir		Unit direction towards which to perform the sweep.
		 * @param	layer		Layers to consider for the query. This allows you to ignore certain groups of objects.
		 * @param	max			Maximum distance at which to perform the query. Hits past this distance will not be
		 *							detected.
		 * @return					True if something was hit, false otherwise.
		 */
		B3D_SCRIPT_EXPORT(ExportName(CapsuleCastAny))
		virtual bool CapsuleCastAny(const Capsule& capsule, const Quaternion& rotation, const Vector3& unitDir, u64 layer = BS_ALL_LAYERS, float max = FLT_MAX) const = 0;

		/**
		 * Performs a sweep into the scene using a convex mesh and checks if it has hit anything. This can be significantly
		 * more efficient than other types of cast* calls.
		 *
		 * @param	mesh		Mesh to sweep through the scene. Must be convex.
		 * @param	position	Starting position of the mesh.
		 * @param	rotation	Orientation of the mesh.
		 * @param	unitDir		Unit direction towards which to perform the sweep.
		 * @param	layer		Layers to consider for the query. This allows you to ignore certain groups of objects.
		 * @param	max			Maximum distance at which to perform the query. Hits past this distance will not be
		 *							detected.
		 * @return					True if something was hit, false otherwise.
		 */
		B3D_SCRIPT_EXPORT(ExportName(ConvexCastAny))
		virtual bool ConvexCastAny(const HPhysicsMesh& mesh, const Vector3& position, const Quaternion& rotation, const Vector3& unitDir, u64 layer = BS_ALL_LAYERS, float max = FLT_MAX) const = 0;

		/**
		 * Returns a list of all colliders in the scene that overlap the provided box.
		 *
		 * @param	box			Box to check for overlap.
		 * @param	rotation	Orientation of the box.
		 * @param	layer		Layers to consider for the query. This allows you to ignore certain groups of objects.
		 * @return					List of all colliders that overlap the box.
		 */
		B3D_SCRIPT_EXPORT(ExportName(BoxOverlap))
		virtual Vector<HCollider> BoxOverlap(const AABox& box, const Quaternion& rotation, u64 layer = BS_ALL_LAYERS) const;

		/**
		 * Returns a list of all colliders in the scene that overlap the provided sphere.
		 *
		 * @param	sphere		Sphere to check for overlap.
		 * @param	layer		Layers to consider for the query. This allows you to ignore certain groups of objects.
		 * @return					List of all colliders that overlap the sphere.
		 */
		B3D_SCRIPT_EXPORT(ExportName(SphereOverlap))
		virtual Vector<HCollider> SphereOverlap(const Sphere& sphere, u64 layer = BS_ALL_LAYERS) const;

		/**
		 * Returns a list of all colliders in the scene that overlap the provided capsule.
		 *
		 * @param	capsule		Capsule to check for overlap.
		 * @param	rotation	Orientation of the capsule.
		 * @param	layer		Layers to consider for the query. This allows you to ignore certain groups of objects.
		 * @return					List of all colliders that overlap the capsule.
		 */
		B3D_SCRIPT_EXPORT(ExportName(CapsuleOverlap))
		virtual Vector<HCollider> CapsuleOverlap(const Capsule& capsule, const Quaternion& rotation, u64 layer = BS_ALL_LAYERS) const;

		/**
		 * Returns a list of all colliders in the scene that overlap the provided convex mesh.
		 *
		 * @param	mesh		Mesh to check for overlap. Must be convex.
		 * @param	position	Position of the mesh.
		 * @param	rotation	Orientation of the mesh.
		 * @param	layer		Layers to consider for the query. This allows you to ignore certain groups of objects.
		 * @return					List of all colliders that overlap the mesh.
		 */
		B3D_SCRIPT_EXPORT(ExportName(ConvexOverlap))
		virtual Vector<HCollider> ConvexOverlap(const HPhysicsMesh& mesh, const Vector3& position, const Quaternion& rotation, u64 layer = BS_ALL_LAYERS) const;

		/**
		 * Checks if the provided box overlaps any other collider in the scene.
		 *
		 * @param	box			Box to check for overlap.
		 * @param	rotation	Orientation of the box.
		 * @param	layer		Layers to consider for the query. This allows you to ignore certain groups of objects.
		 * @return					True if there is overlap with another object, false otherwise.
		 */
		B3D_SCRIPT_EXPORT(ExportName(BoxOverlapAny))
		virtual bool BoxOverlapAny(const AABox& box, const Quaternion& rotation, u64 layer = BS_ALL_LAYERS) const = 0;

		/**
		 * Checks if the provided sphere overlaps any other collider in the scene.
		 *
		 * @param	sphere		Sphere to check for overlap.
		 * @param	layer		Layers to consider for the query. This allows you to ignore certain groups of objects.
		 * @return					True if there is overlap with another object, false otherwise.
		 */
		B3D_SCRIPT_EXPORT(ExportName(SphereOverlapAny))
		virtual bool SphereOverlapAny(const Sphere& sphere, u64 layer = BS_ALL_LAYERS) const = 0;

		/**
		 * Checks if the provided capsule overlaps any other collider in the scene.
		 *
		 * @param	capsule		Capsule to check for overlap.
		 * @param	rotation	Orientation of the capsule.
		 * @param	layer		Layers to consider for the query. This allows you to ignore certain groups of objects.
		 * @return					True if there is overlap with another object, false otherwise.
		 */
		B3D_SCRIPT_EXPORT(ExportName(CapsuleOverlapAny))
		virtual bool CapsuleOverlapAny(const Capsule& capsule, const Quaternion& rotation, u64 layer = BS_ALL_LAYERS) const = 0;

		/**
		 * Checks if the provided convex mesh overlaps any other collider in the scene.
		 *
		 * @param	mesh		Mesh to check for overlap. Must be convex.
		 * @param	position	Position of the mesh.
		 * @param	rotation	Orientation of the mesh.
		 * @param	layer		Layers to consider for the query. This allows you to ignore certain groups of objects.
		 * @return					True if there is overlap with another object, false otherwise.
		 */
		B3D_SCRIPT_EXPORT(ExportName(ConvexOverlapAny))
		virtual bool ConvexOverlapAny(const HPhysicsMesh& mesh, const Vector3& position, const Quaternion& rotation, u64 layer = BS_ALL_LAYERS) const = 0;

		/******************************************************************************************************************/
		/************************************************* OPTIONS ********************************************************/
		/******************************************************************************************************************/

		/** Checks is a specific physics option enabled. */
		virtual bool HasFlag(PhysicsFlags flag) const { return mFlags & flag; }

		/** Enables or disabled a specific physics option. */
		virtual void SetFlag(PhysicsFlags flag, bool enabled)
		{
			if(enabled)
				mFlags |= flag;
			else
				mFlags &= ~flag;
		}

		/**
		 * Returns a maximum edge length before a triangle is tesselated.
		 *
		 * @see PhysicsFlags::CCT_Tesselation
		 */
		virtual float GetMaxTesselationEdgeLength() const = 0;

		/**
		 * Sets a maximum edge length before a triangle is tesselated.
		 *
		 * @see PhysicsFlags::CCT_Tesselation
		 */
		virtual void SetMaxTesselationEdgeLength(float length) = 0;

		/** @copydoc SetGravity() */
		B3D_SCRIPT_EXPORT(ExportName(Gravity), Property(Getter))
		virtual Vector3 GetGravity() const = 0;

		/** Determines the global gravity value for all objects in the scene. */
		B3D_SCRIPT_EXPORT(ExportName(Gravity), Property(Setter))
		virtual void SetGravity(const Vector3& gravity) = 0;

		/**
		 * Adds a new physics region. Certain physics options require you to set up regions in which physics objects are
		 * allowed to be in, and objects outside of these regions will not be handled by physics. You do not need to set
		 * up these regions by default.
		 */
		B3D_SCRIPT_EXPORT(ExportName(AddPhysicsRegion))
		virtual u32 AddBroadPhaseRegion(const AABox& region) = 0;

		/** Removes a physics region. */
		B3D_SCRIPT_EXPORT(ExportName(RemovePhysicsRegion))
		virtual void RemoveBroadPhaseRegion(u32 handle) = 0;

		/** Removes all physics regions. */
		B3D_SCRIPT_EXPORT(ExportName(ClearPhysicsRegions))
		virtual void ClearBroadPhaseRegions() = 0;

		/** @name Internal
		 *  @{
		 */

		/******************************************************************************************************************/
		/************************************************* CREATION *******************************************************/
		/******************************************************************************************************************/

		/** Creates a new fixed joint. */
		virtual UPtr<IFixedJointImplementation> CreateFixedJoint(Joint& owner, const FixedJointCreateInformation& createInformation) = 0;

		/** Creates a new distance joint. */
		virtual UPtr<IDistanceJointImplementation> CreateDistanceJoint(Joint& owner, const DistanceJointCreateInformation& createInformation) = 0;

		/** Creates a new hinge joint. */
		virtual UPtr<IHingeJointImplementation> CreateHingeJoint(Joint& owner, const HingeJointCreateInformation& createInformation) = 0;

		/** Creates a new spherical joint. */
		virtual UPtr<ISphericalJointImplementation> CreateSphericalJoint(Joint& owner, const SphericalJointCreateInformation& createInformation) = 0;

		/** Creates a new spherical joint. */
		virtual UPtr<ISliderJointImplementation> CreateSliderJoint(Joint& owner, const SliderJointCreateInformation& createInformation) = 0;

		/** Creates a new D6 joint. */
		virtual UPtr<ID6JointImplementation> CreateD6Joint(Joint& owner, const D6JointCreateInformation& createInformation) = 0;

		/** Creates a new character controller. */
		virtual UPtr<ICharacterControllerImplementation> CreateCharacterController(CharacterController& owner, const CharacterControllerCreateInformation& createInformation) = 0;

		/** @copydoc PhysicsScene::BoxOverlap() */
		virtual Vector<ColliderShape*> BoxOverlapInternal(const AABox& box, const Quaternion& rotation, u64 layer = BS_ALL_LAYERS) const = 0;

		/** @copydoc PhysicsScene::SphereOverlap() */
		virtual Vector<ColliderShape*> SphereOverlapInternal(const Sphere& sphere, u64 layer = BS_ALL_LAYERS) const = 0;

		/** @copydoc PhysicsScene::CapsuleOverlap() */
		virtual Vector<ColliderShape*> CapsuleOverlapInternal(const Capsule& capsule, const Quaternion& rotation, u64 layer = BS_ALL_LAYERS) const = 0;

		/** @copydoc PhysicsScene::ConvexOverlap() */
		virtual Vector<ColliderShape*> ConvexOverlapInternal(const HPhysicsMesh& mesh, const Vector3& position, const Quaternion& rotation, u64 layer = BS_ALL_LAYERS) const = 0;

		/** @} */
	protected:
		PhysicsScene() = default;
		virtual ~PhysicsScene() = default;

		bool mUpdateInProgress = false;
		PhysicsFlags mFlags;
	};

	/** @} */
} // namespace b3d
