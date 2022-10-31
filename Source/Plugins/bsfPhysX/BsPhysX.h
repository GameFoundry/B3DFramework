//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsPhysXPrerequisites.h"
#include "Physics/BsPhysics.h"
#include "Physics/BsPhysicsCommon.h"
#include "PxPhysics.h"
#include "foundation/Px.h"
#include "characterkinematic/PxControllerManager.h"
#include "cooking/PxCooking.h"

namespace bs
{
	/** @addtogroup PhysX
	 *  @{
	 */

	class PhysXScene;

	/** NVIDIA PhysX implementation of Physics. */
	class PhysX : public Physics
	{
		/** Type of contacts reported by PhysX simulation. */
		enum class ContactEventType
		{
			ContactBegin,
			ContactStay,
			ContactEnd
		};

		/** Event reported when a physics object interacts with a collider. */
		struct TriggerEvent
		{
			Collider* Trigger; /** Trigger that was interacted with. */
			Collider* Other; /** Collider that was interacted with. */
			ContactEventType Type; /** Exact type of the event. */
		};

		/** Event reported when two colliders interact. */
		struct ContactEvent
		{
			Collider* ColliderA; /** First collider. */
			Collider* ColliderB; /** Second collider. */
			ContactEventType Type; /** Exact type of the event. */
			// Note: Not too happy this is heap allocated, use static allocator?
			Vector<ContactPoint> Points; /** Information about all contact points between the colliders. */
		};

		/** Event reported when a joint breaks. */
		struct JointBreakEvent
		{
			Joint* Joint; /** Broken joint. */
		};

	public:
		PhysX(const PHYSICS_INIT_DESC& input);
		~PhysX();

		void FixedUpdate(float step) override;
		void Update() override;
		SPtr<PhysicsMaterial> CreateMaterial(float staticFriction, float dynamicFriction, float restitution) override;
		SPtr<PhysicsMesh> CreateMesh(const SPtr<MeshData>& meshData, PhysicsMeshType type) override;
		SPtr<PhysicsScene> CreatePhysicsScene() override;
		void SetPaused(bool paused) override;

		/** Triggered by the PhysX simulation when an interaction between two colliders is found. */
		void ReportContactEventInternal(const ContactEvent& event);

		/** Triggered by the PhysX simulation when an interaction between two trigger and a collider is found. */
		void ReportTriggerEventInternal(const TriggerEvent& event);

		/** Triggered by the PhysX simulation when a joint breaks. */
		void ReportJointBreakEventInternal(const JointBreakEvent& event);

		bool RayCastInternal(const Vector3& origin, const Vector3& unitDir, const Collider& collider, PhysicsQueryHit& hit, float maxDist = FLT_MAX) const override;

		/** Notifies the system that at physics scene is about to be destroyed. */
		void NotifySceneDestroyedInternal(PhysXScene* scene);

		/** Returns the default PhysX material. */
		physx::PxMaterial* GetDefaultMaterial() const { return mDefaultMaterial; }

		/** Returns the main PhysX object. */
		physx::PxPhysics* GetPhysX() const { return mPhysics; }

		/** Returns the PhysX object used for mesh cooking. */
		physx::PxCooking* GetCooking() const { return mCooking; }

		/** Returns default scale used in the PhysX scene. */
		physx::PxTolerancesScale GetScale() const { return mScale; }

	private:
		friend class PhysXEventCallback;

		/** Sends out all events recorded during simulation to the necessary physics objects. */
		void TriggerEvents();

		PHYSICS_INIT_DESC mInitDesc;
		bool mPaused = false;

		Vector<TriggerEvent> mTriggerEvents;
		Vector<ContactEvent> mContactEvents;
		Vector<JointBreakEvent> mJointBreakEvents;
		Vector<PhysXScene*> mScenes;
		UnorderedMap<u32, u32> mBroadPhaseRegionHandles;

		physx::PxFoundation* mFoundation = nullptr;
		physx::PxPhysics* mPhysics = nullptr;
		physx::PxCooking* mCooking = nullptr;
		physx::PxMaterial* mDefaultMaterial = nullptr;
		physx::PxTolerancesScale mScale;

		static const u32 kScratchBufferSize;
	};

	/** Contains information about a single PhysX scene. */
	class PhysXScene : public PhysicsScene
	{
	public:
		PhysXScene(physx::PxPhysics* physics, const PHYSICS_INIT_DESC& input, const physx::PxTolerancesScale& scale);
		~PhysXScene();

		SPtr<Rigidbody> CreateRigidbody(const HSceneObject& linkedSO) override;
		SPtr<BoxCollider> CreateBoxCollider(const Vector3& extents, const Vector3& position, const Quaternion& rotation) override;
		SPtr<SphereCollider> CreateSphereCollider(float radius, const Vector3& position, const Quaternion& rotation) override;
		SPtr<PlaneCollider> CreatePlaneCollider(const Vector3& position, const Quaternion& rotation) override;
		SPtr<CapsuleCollider> CreateCapsuleCollider(float radius, float halfHeight, const Vector3& position, const Quaternion& rotation) override;
		SPtr<MeshCollider> CreateMeshCollider(const Vector3& position, const Quaternion& rotation) override;
		SPtr<FixedJoint> CreateFixedJoint(const FIXED_JOINT_DESC& desc) override;
		SPtr<DistanceJoint> CreateDistanceJoint(const DISTANCE_JOINT_DESC& desc) override;
		SPtr<HingeJoint> CreateHingeJoint(const HINGE_JOINT_DESC& desc) override;
		SPtr<SphericalJoint> CreateSphericalJoint(const SPHERICAL_JOINT_DESC& desc) override;
		SPtr<SliderJoint> CreateSliderJoint(const SLIDER_JOINT_DESC& desc) override;
		SPtr<D6Joint> CreateD6Joint(const D6_JOINT_DESC& desc) override;
		SPtr<CharacterController> CreateCharacterController(const CHAR_CONTROLLER_DESC& desc) override;

		bool RayCast(const Vector3& origin, const Vector3& unitDir, PhysicsQueryHit& hit, u64 layer = BS_ALL_LAYERS, float max = FLT_MAX) const override;
		bool BoxCast(const AABox& box, const Quaternion& rotation, const Vector3& unitDir, PhysicsQueryHit& hit, u64 layer = BS_ALL_LAYERS, float max = FLT_MAX) const override;
		bool SphereCast(const Sphere& sphere, const Vector3& unitDir, PhysicsQueryHit& hit, u64 layer = BS_ALL_LAYERS, float max = FLT_MAX) const override;
		bool CapsuleCast(const Capsule& capsule, const Quaternion& rotation, const Vector3& unitDir, PhysicsQueryHit& hit, u64 layer = BS_ALL_LAYERS, float max = FLT_MAX) const override;
		bool ConvexCast(const HPhysicsMesh& mesh, const Vector3& position, const Quaternion& rotation, const Vector3& unitDir, PhysicsQueryHit& hit, u64 layer = BS_ALL_LAYERS, float max = FLT_MAX) const override;

		Vector<PhysicsQueryHit> RayCastAll(const Vector3& origin, const Vector3& unitDir, u64 layer = BS_ALL_LAYERS, float max = FLT_MAX) const override;
		Vector<PhysicsQueryHit> BoxCastAll(const AABox& box, const Quaternion& rotation, const Vector3& unitDir, u64 layer = BS_ALL_LAYERS, float max = FLT_MAX) const override;
		Vector<PhysicsQueryHit> SphereCastAll(const Sphere& sphere, const Vector3& unitDir, u64 layer = BS_ALL_LAYERS, float max = FLT_MAX) const override;
		Vector<PhysicsQueryHit> CapsuleCastAll(const Capsule& capsule, const Quaternion& rotation, const Vector3& unitDir, u64 layer = BS_ALL_LAYERS, float max = FLT_MAX) const override;
		Vector<PhysicsQueryHit> ConvexCastAll(const HPhysicsMesh& mesh, const Vector3& position, const Quaternion& rotation, const Vector3& unitDir, u64 layer = BS_ALL_LAYERS, float max = FLT_MAX) const override;

		bool RayCastAny(const Vector3& origin, const Vector3& unitDir, u64 layer = BS_ALL_LAYERS, float max = FLT_MAX) const override;
		bool BoxCastAny(const AABox& box, const Quaternion& rotation, const Vector3& unitDir, u64 layer = BS_ALL_LAYERS, float max = FLT_MAX) const override;
		bool SphereCastAny(const Sphere& sphere, const Vector3& unitDir, u64 layer = BS_ALL_LAYERS, float max = FLT_MAX) const override;
		bool CapsuleCastAny(const Capsule& capsule, const Quaternion& rotation, const Vector3& unitDir, u64 layer = BS_ALL_LAYERS, float max = FLT_MAX) const override;
		bool ConvexCastAny(const HPhysicsMesh& mesh, const Vector3& position, const Quaternion& rotation, const Vector3& unitDir, u64 layer = BS_ALL_LAYERS, float max = FLT_MAX) const override;

		bool BoxOverlapAny(const AABox& box, const Quaternion& rotation, u64 layer = BS_ALL_LAYERS) const override;
		bool SphereOverlapAny(const Sphere& sphere, u64 layer = BS_ALL_LAYERS) const override;
		bool CapsuleOverlapAny(const Capsule& capsule, const Quaternion& rotation, u64 layer = BS_ALL_LAYERS) const override;
		bool ConvexOverlapAny(const HPhysicsMesh& mesh, const Vector3& position, const Quaternion& rotation, u64 layer = BS_ALL_LAYERS) const override;

		Vector3 GetGravity() const override;
		void SetGravity(const Vector3& gravity) override;
		u32 AddBroadPhaseRegion(const AABox& region) override;
		void RemoveBroadPhaseRegion(u32 regionId) override;
		void ClearBroadPhaseRegions() override;
		void SetFlag(PhysicsFlags flags, bool enabled) override;
		float GetMaxTesselationEdgeLength() const override { return mTesselationLength; }
		void SetMaxTesselationEdgeLength(float length) override;

		Vector<Collider*> BoxOverlapInternal(const AABox& box, const Quaternion& rotation, u64 layer = BS_ALL_LAYERS) const override;
		Vector<Collider*> SphereOverlapInternal(const Sphere& sphere, u64 layer = BS_ALL_LAYERS) const override;
		Vector<Collider*> CapsuleOverlapInternal(const Capsule& capsule, const Quaternion& rotation, u64 layer = BS_ALL_LAYERS) const override;
		Vector<Collider*> ConvexOverlapInternal(const HPhysicsMesh& mesh, const Vector3& position, const Quaternion& rotation, u64 layer = BS_ALL_LAYERS) const override;

	private:
		/**
		 * Helper method that performs a sweep query by checking if the provided geometry hits any physics objects
		 * when moved along the specified direction. Returns information about the first hit.
		 */
		inline bool Sweep(const physx::PxGeometry& geometry, const physx::PxTransform& tfrm, const Vector3& unitDir, PhysicsQueryHit& hit, u64 layer, float maxDist) const;

		/**
		 * Helper method that performs a sweep query by checking if the provided geometry hits any physics objects
		 * when moved along the specified direction. Returns information about all hit.
		 */
		inline Vector<PhysicsQueryHit> SweepAll(const physx::PxGeometry& geometry, const physx::PxTransform& tfrm, const Vector3& unitDir, u64 layer, float maxDist) const;

		/**
		 * Helper method that performs a sweep query by checking if the provided geometry hits any physics objects
		 * when moved along the specified direction. Returns no information about the hit, but rather just if it happened or
		 * not.
		 */
		inline bool SweepAny(const physx::PxGeometry& geometry, const physx::PxTransform& tfrm, const Vector3& unitDir, u64 layer, float maxDist) const;

		/** Helper method that returns all colliders that are overlapping the provided geometry. */
		inline Vector<Collider*> Overlap(const physx::PxGeometry& geometry, const physx::PxTransform& tfrm, u64 layer) const;

		/** Helper method that checks if the provided geometry overlaps any physics object. */
		inline bool OverlapAny(const physx::PxGeometry& geometry, const physx::PxTransform& tfrm, u64 layer) const;

	private:
		friend class PhysX;

		float mTesselationLength = 3.0f;

		UnorderedMap<u32, u32> mBroadPhaseRegionHandles;
		u32 mNextRegionIdx = 1;

		physx::PxPhysics* mPhysics = nullptr;
		physx::PxScene* mScene = nullptr;
		physx::PxControllerManager* mCharManager = nullptr;
	};

	/** Provides easier access to PhysX. */
	PhysX& GetPhysX();

	/** @} */
} // namespace bs
