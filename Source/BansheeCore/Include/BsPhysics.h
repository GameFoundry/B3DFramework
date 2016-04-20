//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsCorePrerequisites.h"
#include "BsPhysicsCommon.h"
#include "BsModule.h"
#include "BsVector3.h"
#include "BsVector2.h"
#include "BsQuaternion.h"

namespace BansheeEngine
{
	/** @addtogroup Physics
	 *  @{
	 */

	struct PHYSICS_INIT_DESC;

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
	class BS_CORE_EXPORT Physics : public Module<Physics>
	{
	public:
		Physics(const PHYSICS_INIT_DESC& init);
		virtual ~Physics() { }

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
		virtual bool rayCast(const Ray& ray, PhysicsQueryHit& hit, UINT64 layer = BS_ALL_LAYERS, float max = FLT_MAX) const;

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
		virtual bool rayCast(const Vector3& origin, const Vector3& unitDir, PhysicsQueryHit& hit,
			UINT64 layer = BS_ALL_LAYERS, float max = FLT_MAX) const = 0;

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
		virtual bool boxCast(const AABox& box, const Quaternion& rotation, const Vector3& unitDir, PhysicsQueryHit& hit,
			UINT64 layer = BS_ALL_LAYERS, float max = FLT_MAX) const = 0;

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
		virtual bool sphereCast(const Sphere& sphere, const Vector3& unitDir, PhysicsQueryHit& hit,
			UINT64 layer = BS_ALL_LAYERS, float max = FLT_MAX) const = 0;

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
		virtual bool capsuleCast(const Capsule& capsule, const Quaternion& rotation, const Vector3& unitDir,
			PhysicsQueryHit& hit, UINT64 layer = BS_ALL_LAYERS, float max = FLT_MAX) const = 0;

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
		virtual bool convexCast(const HPhysicsMesh& mesh, const Vector3& position, const Quaternion& rotation,
			const Vector3& unitDir, PhysicsQueryHit& hit, UINT64 layer = BS_ALL_LAYERS, float max = FLT_MAX) const = 0;

		/**
		 * Casts a ray into the scene and returns all found hits.
		 * 
		 * @param[in]	ray		Ray to cast into the scene.
		 * @param[in]	layer	Layers to consider for the query. This allows you to ignore certain groups of objects.
		 * @param[in]	max		Maximum distance at which to perform the query. Hits past this distance will not be
		 *						detected.
		 * @return				List of all detected hits.
		 */
		virtual Vector<PhysicsQueryHit> rayCastAll(const Ray& ray, UINT64 layer = BS_ALL_LAYERS, float max = FLT_MAX) const;

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
		virtual Vector<PhysicsQueryHit> rayCastAll(const Vector3& origin, const Vector3& unitDir,
			UINT64 layer = BS_ALL_LAYERS, float max = FLT_MAX) const = 0;

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
		virtual Vector<PhysicsQueryHit> boxCastAll(const AABox& box, const Quaternion& rotation, 
			const Vector3& unitDir, UINT64 layer = BS_ALL_LAYERS, float max = FLT_MAX) const = 0;

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
		virtual Vector<PhysicsQueryHit> sphereCastAll(const Sphere& sphere, const Vector3& unitDir,
			UINT64 layer = BS_ALL_LAYERS, float max = FLT_MAX) const = 0;

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
		virtual Vector<PhysicsQueryHit> capsuleCastAll(const Capsule& capsule, const Quaternion& rotation, 
			const Vector3& unitDir, UINT64 layer = BS_ALL_LAYERS, float max = FLT_MAX) const = 0;

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
		virtual Vector<PhysicsQueryHit> convexCastAll(const HPhysicsMesh& mesh, const Vector3& position, 
			const Quaternion& rotation, const Vector3& unitDir, UINT64 layer = BS_ALL_LAYERS, float max = FLT_MAX) const = 0;

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
		virtual bool rayCastAny(const Ray& ray, UINT64 layer = BS_ALL_LAYERS, float max = FLT_MAX) const;

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
		virtual bool rayCastAny(const Vector3& origin, const Vector3& unitDir,
			UINT64 layer = BS_ALL_LAYERS, float max = FLT_MAX) const = 0;

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
		virtual bool boxCastAny(const AABox& box, const Quaternion& rotation, const Vector3& unitDir,
			UINT64 layer = BS_ALL_LAYERS, float max = FLT_MAX) const = 0;

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
		virtual bool sphereCastAny(const Sphere& sphere, const Vector3& unitDir,
			UINT64 layer = BS_ALL_LAYERS, float max = FLT_MAX) const = 0;

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
		virtual bool capsuleCastAny(const Capsule& capsule, const Quaternion& rotation, const Vector3& unitDir,
			UINT64 layer = BS_ALL_LAYERS, float max = FLT_MAX) const = 0;

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
		virtual bool convexCastAny(const HPhysicsMesh& mesh, const Vector3& position, const Quaternion& rotation,
			const Vector3& unitDir, UINT64 layer = BS_ALL_LAYERS, float max = FLT_MAX) const = 0;

		/**
		 * Returns a list of all colliders in the scene that overlap the provided box.
		 * 
		 * @param[in]	box			Box to check for overlap.
		 * @param[in]	rotation	Orientation of the box.
		 * @param[in]	layer		Layers to consider for the query. This allows you to ignore certain groups of objects.
		 * @return					List of all colliders that overlap the box.
		 */
		virtual Vector<HCollider> boxOverlap(const AABox& box, const Quaternion& rotation, 
			UINT64 layer = BS_ALL_LAYERS) const;

		/**
		 * Returns a list of all colliders in the scene that overlap the provided sphere.
		 * 
		 * @param[in]	sphere		Sphere to check for overlap.
		 * @param[in]	layer		Layers to consider for the query. This allows you to ignore certain groups of objects.
		 * @return					List of all colliders that overlap the sphere.
		 */
		virtual Vector<HCollider> sphereOverlap(const Sphere& sphere, UINT64 layer = BS_ALL_LAYERS) const;

		/**
		 * Returns a list of all colliders in the scene that overlap the provided capsule.
		 * 
		 * @param[in]	capsule		Capsule to check for overlap.
		 * @param[in]	rotation	Orientation of the capsule.
		 * @param[in]	layer		Layers to consider for the query. This allows you to ignore certain groups of objects.
		 * @return					List of all colliders that overlap the capsule.
		 */
		virtual Vector<HCollider> capsuleOverlap(const Capsule& capsule, const Quaternion& rotation,
			UINT64 layer = BS_ALL_LAYERS) const;

		/**
		 * Returns a list of all colliders in the scene that overlap the provided convex mesh.
		 * 
		 * @param[in]	mesh		Mesh to check for overlap. Must be convex.
		 * @param[in]	position	Position of the mesh.
		 * @param[in]	rotation	Orientation of the mesh.
		 * @param[in]	layer		Layers to consider for the query. This allows you to ignore certain groups of objects.
		 * @return					List of all colliders that overlap the mesh.
		 */
		virtual Vector<HCollider> convexOverlap(const HPhysicsMesh& mesh, const Vector3& position, 
			const Quaternion& rotation, UINT64 layer = BS_ALL_LAYERS) const;

		/**
		 * Checks if the provided box overlaps any other collider in the scene.
		 * 
		 * @param[in]	box			Box to check for overlap.
		 * @param[in]	rotation	Orientation of the box.
		 * @param[in]	layer		Layers to consider for the query. This allows you to ignore certain groups of objects.
		 * @return					True if there is overlap with another object, false otherwise.
		 */
		virtual bool boxOverlapAny(const AABox& box, const Quaternion& rotation, UINT64 layer = BS_ALL_LAYERS) const = 0;

		/**
		 * Checks if the provided sphere overlaps any other collider in the scene.
		 * 
		 * @param[in]	sphere		Sphere to check for overlap.
		 * @param[in]	layer		Layers to consider for the query. This allows you to ignore certain groups of objects.
		 * @return					True if there is overlap with another object, false otherwise.
		 */
		virtual bool sphereOverlapAny(const Sphere& sphere, UINT64 layer = BS_ALL_LAYERS) const = 0;

		/**
		 * Checks if the provided capsule overlaps any other collider in the scene.
		 * 
		 * @param[in]	capsule		Capsule to check for overlap.
		 * @param[in]	rotation	Orientation of the capsule.
		 * @param[in]	layer		Layers to consider for the query. This allows you to ignore certain groups of objects.
		 * @return					True if there is overlap with another object, false otherwise.
		 */
		virtual bool capsuleOverlapAny(const Capsule& capsule, const Quaternion& rotation, 
			UINT64 layer = BS_ALL_LAYERS) const = 0;

		/**
		 * Checks if the provided convex mesh overlaps any other collider in the scene.
		 * 
		 * @param[in]	mesh		Mesh to check for overlap. Must be convex.
		 * @param[in]	position	Position of the mesh.
		 * @param[in]	rotation	Orientation of the mesh.
		 * @param[in]	layer		Layers to consider for the query. This allows you to ignore certain groups of objects.
		 * @return					True if there is overlap with another object, false otherwise.
		 */
		virtual bool convexOverlapAny(const HPhysicsMesh& mesh, const Vector3& position, const Quaternion& rotation,
			UINT64 layer = BS_ALL_LAYERS) const = 0;

		/******************************************************************************************************************/
		/************************************************* OPTIONS ********************************************************/
		/******************************************************************************************************************/

		/** Checks is a specific physics option enabled. */
		virtual bool hasFlag(PhysicsFlags flag) const { return mFlags & flag; }

		/** Enables or disabled a specific physics option. */
		virtual void setFlag(PhysicsFlags flag, bool enabled) { if (enabled) mFlags |= flag; else mFlags &= ~flag; }

		/** Pauses or resumes the physics simulation. */
		virtual void setPaused(bool paused) = 0;

		/** Gets the global gravity value for all objects in the scene. */
		virtual Vector3 getGravity() const = 0;

		/** Sets the global gravity value for all objects in the scene. */
		virtual void setGravity(const Vector3& gravity) = 0;

		/** 
		 * Adds a new physics region. Certain physics options require you to set up regions in which physics objects are
		 * allowed to be in, and objects outside of these regions will not be handled by physics. You do not need to set
		 * up these regions by default.
		 */
		virtual UINT32 addBroadPhaseRegion(const AABox& region) = 0;

		/** Removes a physics region. */
		virtual void removeBroadPhaseRegion(UINT32 handle) = 0;

		/** Removes all physics regions. */
		virtual void clearBroadPhaseRegions() = 0;

		/** 
		 * Returns a maximum edge length before a triangle is tesselated. 
		 *
		 * @see PhysicsFlags::CCT_Tesselation
		 */
		virtual float getMaxTesselationEdgeLength() const = 0;

		/** 
		 * Sets a maximum edge length before a triangle is tesselated. 
		 *
		 * @see PhysicsFlags::CCT_Tesselation
		 */
		virtual void setMaxTesselationEdgeLength(float length) = 0;

		/** 
		 * Enables or disables collision between two layers. Each physics object can be assigned a specific layer, and here
		 * you can determine which layers can interact with each other.
		 */
		void toggleCollision(UINT64 groupA, UINT64 groupB, bool enabled);

		/** Checks if two collision layers are allowed to interact. */
		bool isCollisionEnabled(UINT64 groupA, UINT64 groupB) const;

		/** @name Internal
		 *  @{
		 */

		/******************************************************************************************************************/
		/************************************************* CREATION *******************************************************/
		/******************************************************************************************************************/

		/** @copydoc PhysicsMaterial::create */
		virtual SPtr<PhysicsMaterial> createMaterial(float staticFriction, float dynamicFriction, float restitution) = 0;

		/** @copydoc PhysicsMesh::create */
		virtual SPtr<PhysicsMesh> createMesh(const SPtr<MeshData>& meshData, PhysicsMeshType type) = 0;

		/** @copydoc Rigidbody::create */
		virtual SPtr<Rigidbody> createRigidbody(const HSceneObject& linkedSO) = 0;

		/** @copydoc BoxCollider::create */
		virtual SPtr<BoxCollider> createBoxCollider(const Vector3& extents, const Vector3& position,
			const Quaternion& rotation) = 0;

		/** @copydoc SphereCollider::create */
		virtual SPtr<SphereCollider> createSphereCollider(float radius,
			const Vector3& position, const Quaternion& rotation) = 0;

		/** @copydoc PlaneCollider::create */
		virtual SPtr<PlaneCollider> createPlaneCollider(const Vector3& position, const Quaternion& rotation) = 0;

		/** @copydoc CapsuleCollider::create */
		virtual SPtr<CapsuleCollider> createCapsuleCollider(float radius, float halfHeight,
			const Vector3& position, const Quaternion& rotation) = 0;

		/** @copydoc MeshCollider::create */
		virtual SPtr<MeshCollider> createMeshCollider(const Vector3& position, const Quaternion& rotation) = 0;

		/** @copydoc FixedJoint::create */
		virtual SPtr<FixedJoint> createFixedJoint(const FIXED_JOINT_DESC& desc) = 0;

		/** @copydoc DistanceJoint::create */
		virtual SPtr<DistanceJoint> createDistanceJoint(const DISTANCE_JOINT_DESC& desc) = 0;

		/** @copydoc HingeJoint::create */
		virtual SPtr<HingeJoint> createHingeJoint(const HINGE_JOINT_DESC& desc) = 0;

		/** @copydoc SphericalJoint::create */
		virtual SPtr<SphericalJoint> createSphericalJoint(const SPHERICAL_JOINT_DESC& desc) = 0;

		/** @copydoc SliderJoint::create */
		virtual SPtr<SliderJoint> createSliderJoint(const SLIDER_JOINT_DESC& desc) = 0;

		/** @copydoc D6Joint::create */
		virtual SPtr<D6Joint> createD6Joint(const D6_JOINT_DESC& desc) = 0;

		/** @copydoc CharacterController::create */
		virtual SPtr<CharacterController> createCharacterController(const CHAR_CONTROLLER_DESC& desc) = 0;

		/** Triggers physics simulation update as needed. Should be called once per frame. */
		virtual void update() = 0;

		/** @copydoc Physics::boxOverlap() */
		virtual Vector<Collider*> _boxOverlap(const AABox& box, const Quaternion& rotation,
			UINT64 layer = BS_ALL_LAYERS) const = 0;

		/** @copydoc Physics::sphereOverlap() */
		virtual Vector<Collider*> _sphereOverlap(const Sphere& sphere, UINT64 layer = BS_ALL_LAYERS) const = 0;

		/** @copydoc Physics::capsuleOverlap() */
		virtual Vector<Collider*> _capsuleOverlap(const Capsule& capsule, const Quaternion& rotation,
			UINT64 layer = BS_ALL_LAYERS) const = 0;

		/** @copydoc Physics::convexOverlap() */
		virtual Vector<Collider*> _convexOverlap(const HPhysicsMesh& mesh, const Vector3& position,
			const Quaternion& rotation, UINT64 layer = BS_ALL_LAYERS) const = 0;

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
		virtual bool _rayCast(const Vector3& origin, const Vector3& unitDir, const Collider& collider, PhysicsQueryHit& hit, 
			float maxDist = FLT_MAX) const = 0;

		/** Checks is the physics simulation update currently in progress. */
		bool _isUpdateInProgress() const { return mUpdateInProgress; }

		/** @} */

		static const UINT64 CollisionMapSize = 64;
	protected:
		friend class Rigidbody;

		mutable Mutex mMutex;
		bool mCollisionMap[CollisionMapSize][CollisionMapSize];

		bool mUpdateInProgress = false;
		PhysicsFlags mFlags;

		const static UINT32 MAX_PRIORITY = 128;
	};

	/** Provides easier access to Physics. */
	BS_CORE_EXPORT Physics& gPhysics();

	/** Contains parameters used for initializing the physics system. */
	struct PHYSICS_INIT_DESC
	{
		float typicalLength = 1.0f; /**< Typical length of an object in the scene. */
		float typicalSpeed = 9.81f; /**< Typical speed of an object in the scene. */
		Vector3 gravity = Vector3(0.0f, -9.81f, 0.0f); /**< Initial gravity. */
		bool initCooking = true; /**< Determines should the cooking library be initialized. */
		float timeStep = 1.0f / 60.0f; /**< Determines using what interval should the physics update happen. */
		/** Flags that control global physics option. */
		PhysicsFlags flags = PhysicsFlag::CCT_OverlapRecovery | PhysicsFlag::CCT_PreciseSweeps | PhysicsFlag::CCD_Enable;
	};

	/** @} */
}