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

		/** @copydoc Physics::fixedUpdate */
		void FixedUpdate(float step);

		/** @copydoc Physics::update */
		void Update();

		/** @copydoc Physics::createMaterial */
		SPtr<PhysicsMaterial> CreateMaterial(float staticFriction, float dynamicFriction, float restitution);

		/** @copydoc Physics::createMesh */
		SPtr<PhysicsMesh> CreateMesh(const SPtr<MeshData>& meshData, PhysicsMeshType type);

		/** @copydoc Physics::createPhysicsScene */
		SPtr<PhysicsScene> CreatePhysicsScene();

		/** @copydoc Physics::setPaused */
		void SetPaused(bool paused);

		/** Triggered by the PhysX simulation when an interaction between two colliders is found. */
		void ReportContactEventInternal(const ContactEvent& event);

		/** Triggered by the PhysX simulation when an interaction between two trigger and a collider is found. */
		void ReportTriggerEventInternal(const TriggerEvent& event);

		/** Triggered by the PhysX simulation when a joint breaks. */
		void ReportJointBreakEventInternal(const JointBreakEvent& event);

		/** @copydoc Physics::_rayCast */
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

		/** @copydoc PhysicsScene::createRigidbody */
		SPtr<Rigidbody> CreateRigidbody(const HSceneObject& linkedSO);

		/** @copydoc PhysicsScene::createBoxCollider */
		SPtr<BoxCollider> CreateBoxCollider(const Vector3& extents, const Vector3& position, const Quaternion& rotation);

		/** @copydoc PhysicsScene::createSphereCollider */
		SPtr<SphereCollider> CreateSphereCollider(float radius, const Vector3& position, const Quaternion& rotation);

		/** @copydoc PhysicsScene::createPlaneCollider */
		SPtr<PlaneCollider> CreatePlaneCollider(const Vector3& position, const Quaternion& rotation);

		/** @copydoc PhysicsScene::createCapsuleCollider */
		SPtr<CapsuleCollider> CreateCapsuleCollider(float radius, float halfHeight, const Vector3& position, const Quaternion& rotation);

		/** @copydoc PhysicsScene::createMeshCollider */
		SPtr<MeshCollider> CreateMeshCollider(const Vector3& position, const Quaternion& rotation);

		/** @copydoc PhysicsScene::createFixedJoint */
		SPtr<FixedJoint> CreateFixedJoint(const FIXED_JOINT_DESC& desc);

		/** @copydoc PhysicsScene::createDistanceJoint */
		SPtr<DistanceJoint> CreateDistanceJoint(const DISTANCE_JOINT_DESC& desc);

		/** @copydoc PhysicsScene::createHingeJoint */
		SPtr<HingeJoint> CreateHingeJoint(const HINGE_JOINT_DESC& desc);

		/** @copydoc PhysicsScene::createSphericalJoint */
		SPtr<SphericalJoint> CreateSphericalJoint(const SPHERICAL_JOINT_DESC& desc);

		/** @copydoc PhysicsScene::createSliderJoint */
		SPtr<SliderJoint> CreateSliderJoint(const SLIDER_JOINT_DESC& desc);

		/** @copydoc PhysicsScene::createD6Joint */
		SPtr<D6Joint> CreateD6Joint(const D6_JOINT_DESC& desc) override;

		/** @copydoc PhysicsScene::createCharacterController*/
		SPtr<CharacterController> CreateCharacterController(const CHAR_CONTROLLER_DESC& desc);

		/** @copydoc PhysicsScene::rayCast(const Vector3&, const Vector3&, PhysicsQueryHit&, u64, float) const */
		bool RayCast(const Vector3& origin, const Vector3& unitDir, PhysicsQueryHit& hit, u64 layer = BS_ALL_LAYERS, float max = FLT_MAX) const override;

		/** @copydoc PhysicsScene::boxCast */
		bool BoxCast(const AABox& box, const Quaternion& rotation, const Vector3& unitDir, PhysicsQueryHit& hit, u64 layer = BS_ALL_LAYERS, float max = FLT_MAX) const override;

		/** @copydoc PhysicsScene::sphereCast */
		bool SphereCast(const Sphere& sphere, const Vector3& unitDir, PhysicsQueryHit& hit, u64 layer = BS_ALL_LAYERS, float max = FLT_MAX) const override;

		/** @copydoc PhysicsScene::capsuleCast */
		bool CapsuleCast(const Capsule& capsule, const Quaternion& rotation, const Vector3& unitDir, PhysicsQueryHit& hit, u64 layer = BS_ALL_LAYERS, float max = FLT_MAX) const override;

		/** @copydoc PhysicsScene::convexCast */
		bool ConvexCast(const HPhysicsMesh& mesh, const Vector3& position, const Quaternion& rotation, const Vector3& unitDir, PhysicsQueryHit& hit, u64 layer = BS_ALL_LAYERS, float max = FLT_MAX) const override;

		/** @copydoc PhysicsScene::rayCastAll(const Vector3&, const Vector3&, u64, float) const */
		Vector<PhysicsQueryHit> RayCastAll(const Vector3& origin, const Vector3& unitDir, u64 layer = BS_ALL_LAYERS, float max = FLT_MAX) const;

		/** @copydoc PhysicsScene::boxCastAll */
		Vector<PhysicsQueryHit> BoxCastAll(const AABox& box, const Quaternion& rotation, const Vector3& unitDir, u64 layer = BS_ALL_LAYERS, float max = FLT_MAX) const;

		/** @copydoc PhysicsScene::sphereCastAll */
		Vector<PhysicsQueryHit> SphereCastAll(const Sphere& sphere, const Vector3& unitDir, u64 layer = BS_ALL_LAYERS, float max = FLT_MAX) const;

		/** @copydoc PhysicsScene::capsuleCastAll */
		Vector<PhysicsQueryHit> CapsuleCastAll(const Capsule& capsule, const Quaternion& rotation, const Vector3& unitDir, u64 layer = BS_ALL_LAYERS, float max = FLT_MAX) const;

		/** @copydoc PhysicsScene::convexCastAll */
		Vector<PhysicsQueryHit> ConvexCastAll(const HPhysicsMesh& mesh, const Vector3& position, const Quaternion& rotation, const Vector3& unitDir, u64 layer = BS_ALL_LAYERS, float max = FLT_MAX) const;

		/** @copydoc PhysicsScene::rayCastAny(const Vector3&, const Vector3&, u64, float) const */
		bool RayCastAny(const Vector3& origin, const Vector3& unitDir, u64 layer = BS_ALL_LAYERS, float max = FLT_MAX) const override;

		/** @copydoc PhysicsScene::boxCastAny */
		bool BoxCastAny(const AABox& box, const Quaternion& rotation, const Vector3& unitDir, u64 layer = BS_ALL_LAYERS, float max = FLT_MAX) const override;

		/** @copydoc PhysicsScene::sphereCastAny */
		bool SphereCastAny(const Sphere& sphere, const Vector3& unitDir, u64 layer = BS_ALL_LAYERS, float max = FLT_MAX) const override;

		/** @copydoc PhysicsScene::capsuleCastAny */
		bool CapsuleCastAny(const Capsule& capsule, const Quaternion& rotation, const Vector3& unitDir, u64 layer = BS_ALL_LAYERS, float max = FLT_MAX) const override;

		/** @copydoc PhysicsScene::convexCastAny */
		bool ConvexCastAny(const HPhysicsMesh& mesh, const Vector3& position, const Quaternion& rotation, const Vector3& unitDir, u64 layer = BS_ALL_LAYERS, float max = FLT_MAX) const override;

		/** @copydoc PhysicsScene::boxOverlapAny */
		bool BoxOverlapAny(const AABox& box, const Quaternion& rotation, u64 layer = BS_ALL_LAYERS) const override;

		/** @copydoc PhysicsScene::sphereOverlapAny */
		bool SphereOverlapAny(const Sphere& sphere, u64 layer = BS_ALL_LAYERS) const override;

		/** @copydoc PhysicsScene::capsuleOverlapAny */
		bool CapsuleOverlapAny(const Capsule& capsule, const Quaternion& rotation, u64 layer = BS_ALL_LAYERS) const override;

		/** @copydoc PhysicsScene::convexOverlapAny */
		bool ConvexOverlapAny(const HPhysicsMesh& mesh, const Vector3& position, const Quaternion& rotation, u64 layer = BS_ALL_LAYERS) const override;

		/** @copydoc PhysicsScene::getGravity */
		Vector3 GetGravity() const override;

		/** @copydoc PhysicsScene::setGravity */
		void SetGravity(const Vector3& gravity) override;

		/** @copydoc PhysicsScene::addBroadPhaseRegion */
		u32 AddBroadPhaseRegion(const AABox& region) override;

		/** @copydoc PhysicsScene::removeBroadPhaseRegion */
		void RemoveBroadPhaseRegion(u32 regionId) override;

		/** @copydoc PhysicsScene::clearBroadPhaseRegions */
		void ClearBroadPhaseRegions() override;

		/** @copydoc PhysicsScene::setFlag */
		void SetFlag(PhysicsFlags flags, bool enabled) override;

		/** @copydoc PhysicsScene::getMaxTesselationEdgeLength */
		float GetMaxTesselationEdgeLength() const override { return mTesselationLength; }

		/** @copydoc PhysicsScene::setMaxTesselationEdgeLength */
		void SetMaxTesselationEdgeLength(float length) override;

		/** @copydoc PhysicsScene::_boxOverlap */
		Vector<Collider*> BoxOverlapInternal(const AABox& box, const Quaternion& rotation, u64 layer = BS_ALL_LAYERS) const override;

		/** @copydoc PhysicsScene::_sphereOverlap */
		Vector<Collider*> SphereOverlapInternal(const Sphere& sphere, u64 layer = BS_ALL_LAYERS) const override;

		/** @copydoc PhysicsScene::_capsuleOverlap */
		Vector<Collider*> CapsuleOverlapInternal(const Capsule& capsule, const Quaternion& rotation, u64 layer = BS_ALL_LAYERS) const override;

		/** @copydoc PhysicsScene::_convexOverlap */
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
	PhysX& gPhysX();

	/** @} */
} // namespace bs
