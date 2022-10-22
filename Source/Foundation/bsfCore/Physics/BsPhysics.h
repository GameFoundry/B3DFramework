//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include <cfloat>

#include "BsCorePrerequisites.h"
#include "Physics/BsPhysicsCommon.h"
#include "Utility/BsModule.h"
#include "Math/BsVector3.h"
#include "Math/BsVector2.h"
#include "Math/BsQuaternion.h"

namespace bs
{
	/** @addtogroup Physics
	 *  @{
	 */

	struct PHYSICS_INIT_DESC;
	class PhysicsScene;

	/** Flags for controlling physics behaviour globally. */
	enum class PhysicsFlag
	{
		/**
		 * Automatically recovers character controllers that are interpenetrating geometry. This can happen if a controller
		 * is spawned or teleported into geometry, its size/rotation is changed so it penetrates geometry, or simply
		 * because of numerical imprecision.
		 */
		CCT_OverlapRecovery = 1<<0,
		/**
		 * Performs more accurate sweeps when moving the character controller, making it less likely to interpenetrate
		 * geometry. When overlap recovery is turned on you can consider turning this off as it can compensate for the
		 * less precise sweeps.
		 */
		CCT_PreciseSweeps = 1<<1,
		/**
		 * Large triangles can cause problems with character controller collision. If this option is enabled the triangles
		 * larger than a certain size will be automatically tesselated into smaller triangles, in order to help with
		 * precision.
		 *
		 * @see Physics::getMaxTesselationEdgeLength
		 */
		CCT_Tesselation = 1<<2,
		/**
		 * Enables continous collision detection. This will prevent fast-moving objects from tunneling through each other.
		 * You must also enable CCD for individual Rigidbodies. This option can have a significant performance impact.
		 */
		CCD_Enable = 1<<3
	};

	/** @copydoc CharacterCollisionFlag */
	typedef Flags<PhysicsFlag> PhysicsFlags;
	BS_FLAGS_OPERATORS(PhysicsFlag)

	/** Provides global physics settings, factory methods for physics objects and scene queries. */
	class BS_CORE_EXPORT BS_SCRIPT_EXPORT(DocumentationGroup(Physics)) Physics : public Module<Physics>
	{
	public:
		Physics(const PHYSICS_INIT_DESC& init);
		virtual ~Physics() = default;

		/******************************************************************************************************************/
		/************************************************* OPTIONS ********************************************************/
		/******************************************************************************************************************/

		/** Pauses or resumes the physics simulation. */
		virtual void SetPaused(bool paused) = 0;

		/**
		 * Enables or disables collision between two layers. Each physics object can be assigned a specific layer, and here
		 * you can determine which layers can interact with each other.
		 */
		BS_SCRIPT_EXPORT(n:ToggleCollision)
		void ToggleCollision(u64 groupA, u64 groupB, bool enabled);

		/** Checks if two collision layers are allowed to interact. */
		BS_SCRIPT_EXPORT(n:IsCollisionEnabled)
		bool IsCollisionEnabled(u64 groupA, u64 groupB) const;

		/** @name Internal
		 *  @{
		 */

		/******************************************************************************************************************/
		/************************************************* CREATION *******************************************************/
		/******************************************************************************************************************/

		/** @copydoc PhysicsMaterial::create */
		virtual SPtr<PhysicsMaterial> CreateMaterial(float staticFriction, float dynamicFriction, float restitution) = 0;

		/** @copydoc PhysicsMesh::create */
		virtual SPtr<PhysicsMesh> CreateMesh(const SPtr<MeshData>& meshData, PhysicsMeshType type) = 0;

		/** Creates an object representing the physics scene. Must be manually released via destroyPhysicsScene(). */
		virtual SPtr<PhysicsScene> CreatePhysicsScene() = 0;

		/**
		 * Updates the physics simulation. In order to maintain stability of the physics calculations this method should
		 * be called at fixed intervals (e.g. 60 times a second).
		 *
		 * @param[in]	step	Time delta to advance the physics simulation by, in seconds.
		 */
		virtual void FixedUpdate(float step) = 0;

		/**
		 * Performs any physics operations that arent tied to the fixed update interval. Should be called once per frame.
		 */
		virtual void Update() { }

		/** Checks is the physics simulation update currently in progress. */
		BS_SCRIPT_EXPORT(n:IsUpdateInProgress,pr:getter)
		bool IsUpdateInProgressInternal() const { return mUpdateInProgress; }

		/**
		 * Checks does the ray hit the provided collider.
		 *
		 * @param[in]	origin		Origin of the ray to check.
		 * @param[in]	unitDir		Unit direction of the ray to check.
		 * @param[in]	collider	Collider to check for hit.
		 * @param[out]	hit			Information about the hit. Valid only if the method returns true.
		 * @param[in]	maxDist		Maximum distance from the ray origin to search for hits.
		 * @return					True if the ray has hit the collider.
		 */
		virtual bool RayCastInternal(const Vector3& origin, const Vector3& unitDir, const Collider& collider, PhysicsQueryHit& hit,
			float maxDist = FLT_MAX) const = 0;

		/** @} */

		static const u64 CollisionMapSize = 64;
	protected:
		friend class Rigidbody;

		mutable Mutex mMutex;
		bool mCollisionMap[CollisionMapSize][CollisionMapSize];

		bool mUpdateInProgress = false;
	};

	/** Provides easier access to Physics. */
	BS_CORE_EXPORT Physics& gPhysics();

	/** Contains parameters used for initializing the physics system. */
	struct PHYSICS_INIT_DESC
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
	class BS_CORE_EXPORT BS_SCRIPT_EXPORT(DocumentationGroup(Physics)) PhysicsScene
	{
	public:
		/******************************************************************************************************************/
		/************************************************* QUERIES ********************************************************/
		/******************************************************************************************************************/

		/**
		 * Casts a ray into the scene and returns the closest found hit, if any.
		 *
		 * @param[in]	ray		Ray to cast into the scene.
		 * @param[out]	hit		Information recorded about a hit. Only valid if method returns true.
		 * @param[in]	layer	Layers to consider for the query. This allows you to ignore certain groups of objects.
		 * @param[in]	max		Maximum distance at which to perform the query. Hits past this distance will not be
		 *						detected.
		 * @return				True if something was hit, false otherwise.
		 */
		BS_SCRIPT_EXPORT(n:RayCast)
		virtual bool RayCast(const Ray& ray, PhysicsQueryHit& hit, u64 layer = BS_ALL_LAYERS, float max = FLT_MAX) const;

		/**
		 * Casts a ray into the scene and returns the closest found hit, if any.
		 *
		 * @param[in]	origin		Origin of the ray to cast into the scene.
		 * @param[in]	unitDir		Unit direction of the ray to cast into the scene.
		 * @param[out]	hit			Information recorded about a hit. Only valid if method returns true.
		 * @param[in]	layer		Layers to consider for the query. This allows you to ignore certain groups of objects.
		 * @param[in]	max			Maximum distance at which to perform the query. Hits past this distance will not be
		 *							detected.
		 * @return					True if something was hit, false otherwise.
		 */
		BS_SCRIPT_EXPORT(n:RayCast)
		virtual bool RayCast(const Vector3& origin, const Vector3& unitDir, PhysicsQueryHit& hit,
			u64 layer = BS_ALL_LAYERS, float max = FLT_MAX) const = 0;

		/**
		 * Performs a sweep into the scene using a box and returns the closest found hit, if any.
		 *
		 * @param[in]	box			Box to sweep through the scene.
		 * @param[in]	rotation	Orientation of the box.
		 * @param[in]	unitDir		Unit direction towards which to perform the sweep.
		 * @param[out]	hit			Information recorded about a hit. Only valid if method returns true.
		 * @param[in]	layer		Layers to consider for the query. This allows you to ignore certain groups of objects.
		 * @param[in]	max			Maximum distance at which to perform the query. Hits past this distance will not be
		 *							detected.
		 * @return					True if something was hit, false otherwise.
		 */
		BS_SCRIPT_EXPORT(n:BoxCast)
		virtual bool BoxCast(const AABox& box, const Quaternion& rotation, const Vector3& unitDir, PhysicsQueryHit& hit,
			u64 layer = BS_ALL_LAYERS, float max = FLT_MAX) const = 0;

		/**
		 * Performs a sweep into the scene using a sphere and returns the closest found hit, if any.
		 *
		 * @param[in]	sphere		Sphere to sweep through the scene.
		 * @param[in]	unitDir		Unit direction towards which to perform the sweep.
		 * @param[out]	hit			Information recorded about a hit. Only valid if method returns true.
		 * @param[in]	layer		Layers to consider for the query. This allows you to ignore certain groups of objects.
		 * @param[in]	max			Maximum distance at which to perform the query. Hits past this distance will not be
		 *							detected.
		 * @return					True if something was hit, false otherwise.
		 */
		BS_SCRIPT_EXPORT(n:SphereCast)
		virtual bool SphereCast(const Sphere& sphere, const Vector3& unitDir, PhysicsQueryHit& hit,
			u64 layer = BS_ALL_LAYERS, float max = FLT_MAX) const = 0;

		/**
		 * Performs a sweep into the scene using a capsule and returns the closest found hit, if any.
		 *
		 * @param[in]	capsule		Capsule to sweep through the scene.
		 * @param[in]	rotation	Orientation of the capsule.
		 * @param[in]	unitDir		Unit direction towards which to perform the sweep.
		 * @param[out]	hit			Information recorded about a hit. Only valid if method returns true.
		 * @param[in]	layer		Layers to consider for the query. This allows you to ignore certain groups of objects.
		 * @param[in]	max			Maximum distance at which to perform the query. Hits past this distance will not be
		 *							detected.
		 * @return					True if something was hit, false otherwise.
		 */
		BS_SCRIPT_EXPORT(n:CapsuleCast)
		virtual bool CapsuleCast(const Capsule& capsule, const Quaternion& rotation, const Vector3& unitDir,
			PhysicsQueryHit& hit, u64 layer = BS_ALL_LAYERS, float max = FLT_MAX) const = 0;

		/**
		 * Performs a sweep into the scene using a convex mesh and returns the closest found hit, if any.
		 *
		 * @param[in]	mesh		Mesh to sweep through the scene. Must be convex.
		 * @param[in]	position	Starting position of the mesh.
		 * @param[in]	rotation	Orientation of the mesh.
		 * @param[in]	unitDir		Unit direction towards which to perform the sweep.
		 * @param[out]	hit			Information recorded about a hit. Only valid if method returns true.
		 * @param[in]	layer		Layers to consider for the query. This allows you to ignore certain groups of objects.
		 * @param[in]	max			Maximum distance at which to perform the query. Hits past this distance will not be
		 *							detected.
		 * @return					True if something was hit, false otherwise.
		 */
		BS_SCRIPT_EXPORT(n:ConvexCast)
		virtual bool ConvexCast(const HPhysicsMesh& mesh, const Vector3& position, const Quaternion& rotation,
			const Vector3& unitDir, PhysicsQueryHit& hit, u64 layer = BS_ALL_LAYERS, float max = FLT_MAX) const = 0;

		/**
		 * Casts a ray into the scene and returns all found hits.
		 *
		 * @param[in]	ray		Ray to cast into the scene.
		 * @param[in]	layer	Layers to consider for the query. This allows you to ignore certain groups of objects.
		 * @param[in]	max		Maximum distance at which to perform the query. Hits past this distance will not be
		 *						detected.
		 * @return				List of all detected hits.
		 */
		BS_SCRIPT_EXPORT(n:RayCastAll)
		virtual Vector<PhysicsQueryHit> RayCastAll(const Ray& ray, u64 layer = BS_ALL_LAYERS, float max = FLT_MAX) const;

		/**
		 * Casts a ray into the scene and returns all found hits.
		 *
		 * @param[in]	origin		Origin of the ray to cast into the scene.
		 * @param[in]	unitDir		Unit direction of the ray to cast into the scene.
		 * @param[in]	layer		Layers to consider for the query. This allows you to ignore certain groups of objects.
		 * @param[in]	max			Maximum distance at which to perform the query. Hits past this distance will not be
		 *							detected.
		 * @return					List of all detected hits.
		 */
		BS_SCRIPT_EXPORT(n:RayCastAll)
		virtual Vector<PhysicsQueryHit> RayCastAll(const Vector3& origin, const Vector3& unitDir,
			u64 layer = BS_ALL_LAYERS, float max = FLT_MAX) const = 0;

		/**
		 * Performs a sweep into the scene using a box and returns all found hits.
		 *
		 * @param[in]	box			Box to sweep through the scene.
		 * @param[in]	rotation	Orientation of the box.
		 * @param[in]	unitDir		Unit direction towards which to perform the sweep.
		 * @param[in]	layer		Layers to consider for the query. This allows you to ignore certain groups of objects.
		 * @param[in]	max			Maximum distance at which to perform the query. Hits past this distance will not be
		 *							detected.
		 * @return					List of all detected hits.
		 */
		BS_SCRIPT_EXPORT(n:BoxCastAll)
		virtual Vector<PhysicsQueryHit> BoxCastAll(const AABox& box, const Quaternion& rotation,
			const Vector3& unitDir, u64 layer = BS_ALL_LAYERS, float max = FLT_MAX) const = 0;

		/**
		 * Performs a sweep into the scene using a sphere and returns all found hits.
		 *
		 * @param[in]	sphere		Sphere to sweep through the scene.
		 * @param[in]	unitDir		Unit direction towards which to perform the sweep.
		 * @param[in]	layer		Layers to consider for the query. This allows you to ignore certain groups of objects.
		 * @param[in]	max			Maximum distance at which to perform the query. Hits past this distance will not be
		 *							detected.
		 * @return					List of all detected hits.
		 */
		BS_SCRIPT_EXPORT(n:SphereCastAll)
		virtual Vector<PhysicsQueryHit> SphereCastAll(const Sphere& sphere, const Vector3& unitDir,
			u64 layer = BS_ALL_LAYERS, float max = FLT_MAX) const = 0;

		/**
		 * Performs a sweep into the scene using a capsule and returns all found hits.
		 *
		 * @param[in]	capsule		Capsule to sweep through the scene.
		 * @param[in]	rotation	Orientation of the capsule.
		 * @param[in]	unitDir		Unit direction towards which to perform the sweep.
		 * @param[in]	layer		Layers to consider for the query. This allows you to ignore certain groups of objects.
		 * @param[in]	max			Maximum distance at which to perform the query. Hits past this distance will not be
		 *							detected.
		 * @return					List of all detected hits.
		 */
		BS_SCRIPT_EXPORT(n:CapsuleCastAll)
		virtual Vector<PhysicsQueryHit> CapsuleCastAll(const Capsule& capsule, const Quaternion& rotation,
			const Vector3& unitDir, u64 layer = BS_ALL_LAYERS, float max = FLT_MAX) const = 0;

		/**
		 * Performs a sweep into the scene using a convex mesh and returns all found hits.
		 *
		 * @param[in]	mesh		Mesh to sweep through the scene. Must be convex.
		 * @param[in]	position	Starting position of the mesh.
		 * @param[in]	rotation	Orientation of the mesh.
		 * @param[in]	unitDir		Unit direction towards which to perform the sweep.
		 * @param[in]	layer		Layers to consider for the query. This allows you to ignore certain groups of objects.
		 * @param[in]	max			Maximum distance at which to perform the query. Hits past this distance will not be
		 *							detected.
		 * @return					List of all detected hits.
		 */
		BS_SCRIPT_EXPORT(n:ConvexCastAll)
		virtual Vector<PhysicsQueryHit> ConvexCastAll(const HPhysicsMesh& mesh, const Vector3& position,
			const Quaternion& rotation, const Vector3& unitDir, u64 layer = BS_ALL_LAYERS, float max = FLT_MAX) const = 0;

		/**
		 * Casts a ray into the scene and checks if it has hit anything. This can be significantly more efficient than other
		 * types of cast* calls.
		 *
		 * @param[in]	ray		Ray to cast into the scene.
		 * @param[in]	layer	Layers to consider for the query. This allows you to ignore certain groups of objects.
		 * @param[in]	max		Maximum distance at which to perform the query. Hits past this distance will not be
		 *						detected.
		 * @return				True if something was hit, false otherwise.
		 */
		BS_SCRIPT_EXPORT(n:RayCastAny)
		virtual bool RayCastAny(const Ray& ray, u64 layer = BS_ALL_LAYERS, float max = FLT_MAX) const;

		/**
		 * Casts a ray into the scene and checks if it has hit anything. This can be significantly more efficient than other
		 * types of cast* calls.
		 *
		 * @param[in]	origin		Origin of the ray to cast into the scene.
		 * @param[in]	unitDir		Unit direction of the ray to cast into the scene.
		 * @param[in]	layer		Layers to consider for the query. This allows you to ignore certain groups of objects.
		 * @param[in]	max			Maximum distance at which to perform the query. Hits past this distance will not be
		 *							detected.
		 * @return					True if something was hit, false otherwise.
		 */
		BS_SCRIPT_EXPORT(n:RayCastAny)
		virtual bool RayCastAny(const Vector3& origin, const Vector3& unitDir,
			u64 layer = BS_ALL_LAYERS, float max = FLT_MAX) const = 0;

		/**
		 * Performs a sweep into the scene using a box and checks if it has hit anything. This can be significantly more
		 * efficient than other types of cast* calls.
		 *
		 * @param[in]	box			Box to sweep through the scene.
		 * @param[in]	rotation	Orientation of the box.
		 * @param[in]	unitDir		Unit direction towards which to perform the sweep.
		 * @param[in]	layer		Layers to consider for the query. This allows you to ignore certain groups of objects.
		 * @param[in]	max			Maximum distance at which to perform the query. Hits past this distance will not be
		 *							detected.
		 * @return					True if something was hit, false otherwise.
		 */
		BS_SCRIPT_EXPORT(n:BoxCastAny)
		virtual bool BoxCastAny(const AABox& box, const Quaternion& rotation, const Vector3& unitDir,
			u64 layer = BS_ALL_LAYERS, float max = FLT_MAX) const = 0;

		/**
		 * Performs a sweep into the scene using a sphere and checks if it has hit anything. This can be significantly more
		 * efficient than other types of cast* calls.
		 *
		 * @param[in]	sphere		Sphere to sweep through the scene.
		 * @param[in]	unitDir		Unit direction towards which to perform the sweep.
		 * @param[in]	layer		Layers to consider for the query. This allows you to ignore certain groups of objects.
		 * @param[in]	max			Maximum distance at which to perform the query. Hits past this distance will not be
		 *							detected.
		 * @return					True if something was hit, false otherwise.
		 */
		BS_SCRIPT_EXPORT(n:SphereCastAny)
		virtual bool SphereCastAny(const Sphere& sphere, const Vector3& unitDir,
			u64 layer = BS_ALL_LAYERS, float max = FLT_MAX) const = 0;

		/**
		 * Performs a sweep into the scene using a capsule and checks if it has hit anything. This can be significantly more
		 * efficient than other types of cast* calls.
		 *
		 * @param[in]	capsule		Capsule to sweep through the scene.
		 * @param[in]	rotation	Orientation of the capsule.
		 * @param[in]	unitDir		Unit direction towards which to perform the sweep.
		 * @param[in]	layer		Layers to consider for the query. This allows you to ignore certain groups of objects.
		 * @param[in]	max			Maximum distance at which to perform the query. Hits past this distance will not be
		 *							detected.
		 * @return					True if something was hit, false otherwise.
		 */
		BS_SCRIPT_EXPORT(n:CapsuleCastAny)
		virtual bool CapsuleCastAny(const Capsule& capsule, const Quaternion& rotation, const Vector3& unitDir,
			u64 layer = BS_ALL_LAYERS, float max = FLT_MAX) const = 0;

		/**
		 * Performs a sweep into the scene using a convex mesh and checks if it has hit anything. This can be significantly
		 * more efficient than other types of cast* calls.
		 *
		 * @param[in]	mesh		Mesh to sweep through the scene. Must be convex.
		 * @param[in]	position	Starting position of the mesh.
		 * @param[in]	rotation	Orientation of the mesh.
		 * @param[in]	unitDir		Unit direction towards which to perform the sweep.
		 * @param[in]	layer		Layers to consider for the query. This allows you to ignore certain groups of objects.
		 * @param[in]	max			Maximum distance at which to perform the query. Hits past this distance will not be
		 *							detected.
		 * @return					True if something was hit, false otherwise.
		 */
		BS_SCRIPT_EXPORT(n:ConvexCastAny)
		virtual bool ConvexCastAny(const HPhysicsMesh& mesh, const Vector3& position, const Quaternion& rotation,
			const Vector3& unitDir, u64 layer = BS_ALL_LAYERS, float max = FLT_MAX) const = 0;

		/**
		 * Returns a list of all colliders in the scene that overlap the provided box.
		 *
		 * @param[in]	box			Box to check for overlap.
		 * @param[in]	rotation	Orientation of the box.
		 * @param[in]	layer		Layers to consider for the query. This allows you to ignore certain groups of objects.
		 * @return					List of all colliders that overlap the box.
		 */
		BS_SCRIPT_EXPORT(n:BoxOverlap)
		virtual Vector<HCollider> BoxOverlap(const AABox& box, const Quaternion& rotation,
			u64 layer = BS_ALL_LAYERS) const;

		/**
		 * Returns a list of all colliders in the scene that overlap the provided sphere.
		 *
		 * @param[in]	sphere		Sphere to check for overlap.
		 * @param[in]	layer		Layers to consider for the query. This allows you to ignore certain groups of objects.
		 * @return					List of all colliders that overlap the sphere.
		 */
		BS_SCRIPT_EXPORT(n:SphereOverlap)
		virtual Vector<HCollider> SphereOverlap(const Sphere& sphere, u64 layer = BS_ALL_LAYERS) const;

		/**
		 * Returns a list of all colliders in the scene that overlap the provided capsule.
		 *
		 * @param[in]	capsule		Capsule to check for overlap.
		 * @param[in]	rotation	Orientation of the capsule.
		 * @param[in]	layer		Layers to consider for the query. This allows you to ignore certain groups of objects.
		 * @return					List of all colliders that overlap the capsule.
		 */
		BS_SCRIPT_EXPORT(n:CapsuleOverlap)
		virtual Vector<HCollider> CapsuleOverlap(const Capsule& capsule, const Quaternion& rotation,
			u64 layer = BS_ALL_LAYERS) const;

		/**
		 * Returns a list of all colliders in the scene that overlap the provided convex mesh.
		 *
		 * @param[in]	mesh		Mesh to check for overlap. Must be convex.
		 * @param[in]	position	Position of the mesh.
		 * @param[in]	rotation	Orientation of the mesh.
		 * @param[in]	layer		Layers to consider for the query. This allows you to ignore certain groups of objects.
		 * @return					List of all colliders that overlap the mesh.
		 */
		BS_SCRIPT_EXPORT(n:ConvexOverlap)
		virtual Vector<HCollider> ConvexOverlap(const HPhysicsMesh& mesh, const Vector3& position,
			const Quaternion& rotation, u64 layer = BS_ALL_LAYERS) const;

		/**
		 * Checks if the provided box overlaps any other collider in the scene.
		 *
		 * @param[in]	box			Box to check for overlap.
		 * @param[in]	rotation	Orientation of the box.
		 * @param[in]	layer		Layers to consider for the query. This allows you to ignore certain groups of objects.
		 * @return					True if there is overlap with another object, false otherwise.
		 */
		BS_SCRIPT_EXPORT(n:BoxOverlapAny)
		virtual bool BoxOverlapAny(const AABox& box, const Quaternion& rotation, u64 layer = BS_ALL_LAYERS) const = 0;

		/**
		 * Checks if the provided sphere overlaps any other collider in the scene.
		 *
		 * @param[in]	sphere		Sphere to check for overlap.
		 * @param[in]	layer		Layers to consider for the query. This allows you to ignore certain groups of objects.
		 * @return					True if there is overlap with another object, false otherwise.
		 */
		BS_SCRIPT_EXPORT(n:SphereOverlapAny)
		virtual bool SphereOverlapAny(const Sphere& sphere, u64 layer = BS_ALL_LAYERS) const = 0;

		/**
		 * Checks if the provided capsule overlaps any other collider in the scene.
		 *
		 * @param[in]	capsule		Capsule to check for overlap.
		 * @param[in]	rotation	Orientation of the capsule.
		 * @param[in]	layer		Layers to consider for the query. This allows you to ignore certain groups of objects.
		 * @return					True if there is overlap with another object, false otherwise.
		 */
		BS_SCRIPT_EXPORT(n:CapsuleOverlapAny)
		virtual bool CapsuleOverlapAny(const Capsule& capsule, const Quaternion& rotation,
			u64 layer = BS_ALL_LAYERS) const = 0;

		/**
		 * Checks if the provided convex mesh overlaps any other collider in the scene.
		 *
		 * @param[in]	mesh		Mesh to check for overlap. Must be convex.
		 * @param[in]	position	Position of the mesh.
		 * @param[in]	rotation	Orientation of the mesh.
		 * @param[in]	layer		Layers to consider for the query. This allows you to ignore certain groups of objects.
		 * @return					True if there is overlap with another object, false otherwise.
		 */
		BS_SCRIPT_EXPORT(n:ConvexOverlapAny)
		virtual bool ConvexOverlapAny(const HPhysicsMesh& mesh, const Vector3& position, const Quaternion& rotation,
			u64 layer = BS_ALL_LAYERS) const = 0;

		/******************************************************************************************************************/
		/************************************************* OPTIONS ********************************************************/
		/******************************************************************************************************************/

		/** Checks is a specific physics option enabled. */
		virtual bool HasFlag(PhysicsFlags flag) const { return mFlags & flag; }

		/** Enables or disabled a specific physics option. */
		virtual void SetFlag(PhysicsFlags flag, bool enabled) { if (enabled) mFlags |= flag; else mFlags &= ~flag; }

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

		/** @copydoc setGravity() */
		BS_SCRIPT_EXPORT(n:Gravity,pr:getter)
		virtual Vector3 GetGravity() const = 0;

		/** Determines the global gravity value for all objects in the scene. */
		BS_SCRIPT_EXPORT(n:Gravity,pr:setter)
		virtual void SetGravity(const Vector3& gravity) = 0;

		/**
		 * Adds a new physics region. Certain physics options require you to set up regions in which physics objects are
		 * allowed to be in, and objects outside of these regions will not be handled by physics. You do not need to set
		 * up these regions by default.
		 */
		BS_SCRIPT_EXPORT(n:AddPhysicsRegion)
		virtual u32 AddBroadPhaseRegion(const AABox& region) = 0;

		/** Removes a physics region. */
		BS_SCRIPT_EXPORT(n:RemovePhysicsRegion)
		virtual void RemoveBroadPhaseRegion(u32 handle) = 0;

		/** Removes all physics regions. */
		BS_SCRIPT_EXPORT(n:ClearPhysicsRegions)
		virtual void ClearBroadPhaseRegions() = 0;

		/** @name Internal
		 *  @{
		 */

		/******************************************************************************************************************/
		/************************************************* CREATION *******************************************************/
		/******************************************************************************************************************/

		/** @copydoc Rigidbody::create */
		virtual SPtr<Rigidbody> CreateRigidbody(const HSceneObject& linkedSO) = 0;

		/**
		 * Creates a new box collider.
		 *
		 * @param[in]	extents		Extents (half size) of the box.
		 * @param[in]	position	Center of the box.
		 * @param[in]	rotation	Rotation of the box.
		 */
		virtual SPtr<BoxCollider> CreateBoxCollider(const Vector3& extents, const Vector3& position,
			const Quaternion& rotation) = 0;

		/**
		 * Creates a new sphere collider.
		 *
		 * @param[in]	radius		Radius of the sphere geometry.
		 * @param[in]	position	Position of the collider.
		 * @param[in]	rotation	Rotation of the collider.
		 */
		virtual SPtr<SphereCollider> CreateSphereCollider(float radius,
			const Vector3& position, const Quaternion& rotation) = 0;

		/**
		 * Creates a new plane collider.
		 *
		 * @param[in]	position	Position of the collider.
		 * @param[in]	rotation	Rotation of the collider.
		 */
		virtual SPtr<PlaneCollider> CreatePlaneCollider(const Vector3& position, const Quaternion& rotation) = 0;

		/**
		 * Creates a new capsule collider.
		 *
		 * @param[in]	radius		Radius of the capsule.
		 * @param[in]	halfHeight	Half height of the capsule, from the origin to one of the hemispherical centers, along
		 *							the normal vector.
		 * @param[in]	position	Center of the box.
		 * @param[in]	rotation	Rotation of the box.
		 */
		virtual SPtr<CapsuleCollider> CreateCapsuleCollider(float radius, float halfHeight,
			const Vector3& position, const Quaternion& rotation) = 0;

		/**
		 * Creates a new mesh collider.
		 *
		 * @param[in]	position	Position of the collider.
		 * @param[in]	rotation	Rotation of the collider.
		 */
		virtual SPtr<MeshCollider> CreateMeshCollider(const Vector3& position, const Quaternion& rotation) = 0;

		/**
		 * Creates a new fixed joint.
		 *
		 * @param[in]	desc		Settings describing the joint.
		 */
		virtual SPtr<FixedJoint> CreateFixedJoint(const FIXED_JOINT_DESC& desc) = 0;

		/**
		 * Creates a new distance joint.
		 *
		 * @param[in]	desc		Settings describing the joint.
		 */
		virtual SPtr<DistanceJoint> CreateDistanceJoint(const DISTANCE_JOINT_DESC& desc) = 0;

		/**
		 * Creates a new hinge joint.
		 *
		 * @param[in]	desc		Settings describing the joint.
		 */
		virtual SPtr<HingeJoint> CreateHingeJoint(const HINGE_JOINT_DESC& desc) = 0;

		/**
		 * Creates a new spherical joint.
		 *
		 * @param[in]	desc		Settings describing the joint.
		 */
		virtual SPtr<SphericalJoint> CreateSphericalJoint(const SPHERICAL_JOINT_DESC& desc) = 0;

		/**
		 * Creates a new spherical joint.
		 *
		 * @param[in]	desc		Settings describing the joint.
		 */
		virtual SPtr<SliderJoint> CreateSliderJoint(const SLIDER_JOINT_DESC& desc) = 0;

		/**
		 * Creates a new D6 joint.
		 *
		 * @param[in]	desc		Settings describing the joint.
		 */
		virtual SPtr<D6Joint> CreateD6Joint(const D6_JOINT_DESC& desc) = 0;

		/**
		 * Creates a new character controller.
		 *
		 * @param[in]	desc		Describes controller geometry and movement.
		 */
		virtual SPtr<CharacterController> CreateCharacterController(const CHAR_CONTROLLER_DESC& desc) = 0;

		/** @copydoc PhysicsScene::boxOverlap() */
		virtual Vector<Collider*> BoxOverlapInternal(const AABox& box, const Quaternion& rotation,
			u64 layer = BS_ALL_LAYERS) const = 0;

		/** @copydoc PhysicsScene::sphereOverlap() */
		virtual Vector<Collider*> SphereOverlapInternal(const Sphere& sphere, u64 layer = BS_ALL_LAYERS) const = 0;

		/** @copydoc PhysicsScene::capsuleOverlap() */
		virtual Vector<Collider*> CapsuleOverlapInternal(const Capsule& capsule, const Quaternion& rotation,
			u64 layer = BS_ALL_LAYERS) const = 0;

		/** @copydoc PhysicsScene::convexOverlap() */
		virtual Vector<Collider*> ConvexOverlapInternal(const HPhysicsMesh& mesh, const Vector3& position,
			const Quaternion& rotation, u64 layer = BS_ALL_LAYERS) const = 0;

		/** @} */
	protected:
		PhysicsScene() = default;
		virtual ~PhysicsScene() = default;

		PhysicsFlags mFlags;
	};

	/** @} */
}
