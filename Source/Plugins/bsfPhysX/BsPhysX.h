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
			Collider* trigger; /** Trigger that was interacted with. */
			Collider* other; /** Collider that was interacted with. */
			ContactEventType type; /** Exact type of the event. */
		};

		/** Event reported when two colliders interact. */
		struct ContactEvent
		{
			Collider* colliderA; /** First collider. */
			Collider* colliderB; /** Second collider. */
			ContactEventType type; /** Exact type of the event. */
			// Note: Not too happy this is heap allocated, use static allocator?
			Vector<ContactPoint> points; /** Information about all contact points between the colliders. */
		};

		/** Event reported when a joint breaks. */
		struct JointBreakEvent
		{
			Joint* joint; /** Broken joint. */
		};

	public:
		PhysX(const PHYSICS_INIT_DESC& input);
		~PhysX();

		/** @copydoc Physics::fixedUpdate */
		void fixedUpdate(float step) override;

		/** @copydoc Physics::update */
		void update() override;

		/** @copydoc Physics::createMaterial */
		SPtr<PhysicsMaterial> createMaterial(float staticFriction, float dynamicFriction, float restitution) override;

		/** @copydoc Physics::createMesh */
		SPtr<PhysicsMesh> createMesh(const SPtr<MeshData>& meshData, PhysicsMeshType type) override;

		/** @copydoc Physics::createPhysicsScene */
		SPtr<PhysicsScene> createPhysicsScene() override;

		/** @copydoc Physics::setPaused */
		void setPaused(bool paused) override;

		/** Triggered by the PhysX simulation when an interaction between two colliders is found. */
		void _reportContactEvent(const ContactEvent& event);

		/** Triggered by the PhysX simulation when an interaction between two trigger and a collider is found. */
		void _reportTriggerEvent(const TriggerEvent& event);

		/** Triggered by the PhysX simulation when a joint breaks. */
		void _reportJointBreakEvent(const JointBreakEvent& event);

		/** @copydoc Physics::_rayCast */
		bool _rayCast(const Vector3& origin, const Vector3& unitDir, const Collider& collider, PhysicsQueryHit& hit, 
			float maxDist = FLT_MAX) const override;

		/** Notifies the system that at physics scene is about to be destroyed. */
		void _notifySceneDestroyed(PhysXScene* scene);

		/** Returns the default PhysX material. */
		physx::PxMaterial* getDefaultMaterial() const { return mDefaultMaterial; }

		/** Returns the main PhysX object. */
		physx::PxPhysics* getPhysX() const { return mPhysics; }

		/** Returns the PhysX object used for mesh cooking. */
		physx::PxCooking* getCooking() const { return mCooking; }

		/** Returns default scale used in the PhysX scene. */
		physx::PxTolerancesScale getScale() const { return mScale; }

	private:
		friend class PhysXEventCallback;

		/** Sends out all events recorded during simulation to the necessary physics objects. */
		void triggerEvents();

		PHYSICS_INIT_DESC mInitDesc;
		bool mPaused = false;

		Vector<TriggerEvent> mTriggerEvents;
		Vector<ContactEvent> mContactEvents;
		Vector<JointBreakEvent> mJointBreakEvents;
		Vector<PhysXScene*> mScenes;
		UnorderedMap<UINT32, UINT32> mBroadPhaseRegionHandles;

		physx::PxFoundation* mFoundation = nullptr;
		physx::PxPhysics* mPhysics = nullptr;
		physx::PxCooking* mCooking = nullptr;
		physx::PxMaterial* mDefaultMaterial = nullptr;
		physx::PxTolerancesScale mScale;

		static const UINT32 SCRATCH_BUFFER_SIZE;
	};

	/** Contains information about a single PhysX scene. */
	class PhysXScene : public PhysicsScene
	{
	public:
		PhysXScene(physx::PxPhysics* physics, const PHYSICS_INIT_DESC& input, const physx::PxTolerancesScale& scale);
		~PhysXScene();

		/** @copydoc PhysicsScene::createRigidbody */
		SPtr<Rigidbody> createRigidbody(const HSceneObject& linkedSO) override;

		/** @copydoc PhysicsScene::createBoxCollider */
		SPtr<BoxCollider> createBoxCollider(const Vector3& extents, const Vector3& position,
			const Quaternion& rotation) override;

		/** @copydoc PhysicsScene::createSphereCollider */
		SPtr<SphereCollider> createSphereCollider(float radius, const Vector3& position, const Quaternion& rotation) override;

		/** @copydoc PhysicsScene::createPlaneCollider */
		SPtr<PlaneCollider> createPlaneCollider(const Vector3& position, const Quaternion& rotation) override;

		/** @copydoc PhysicsScene::createCapsuleCollider */
		SPtr<CapsuleCollider> createCapsuleCollider(float radius, float halfHeight, const Vector3& position, 
			const Quaternion& rotation) override;

		/** @copydoc PhysicsScene::createMeshCollider */
		SPtr<MeshCollider> createMeshCollider(const Vector3& position, const Quaternion& rotation) override;

		/** @copydoc PhysicsScene::createFixedJoint */
		SPtr<FixedJoint> createFixedJoint(const FIXED_JOINT_DESC& desc) override;

		/** @copydoc PhysicsScene::createDistanceJoint */
		SPtr<DistanceJoint> createDistanceJoint(const DISTANCE_JOINT_DESC& desc) override;

		/** @copydoc PhysicsScene::createHingeJoint */
		SPtr<HingeJoint> createHingeJoint(const HINGE_JOINT_DESC& desc) override;

		/** @copydoc PhysicsScene::createSphericalJoint */
		SPtr<SphericalJoint> createSphericalJoint(const SPHERICAL_JOINT_DESC& desc) override;

		/** @copydoc PhysicsScene::createSliderJoint */
		SPtr<SliderJoint> createSliderJoint(const SLIDER_JOINT_DESC& desc) override;

		/** @copydoc PhysicsScene::createD6Joint */
		SPtr<D6Joint> createD6Joint(const D6_JOINT_DESC& desc) override;

		/** @copydoc PhysicsScene::createCharacterController*/
		SPtr<CharacterController> createCharacterController(const CHAR_CONTROLLER_DESC& desc) override;

		/** @copydoc PhysicsScene::rayCast(const Vector3&, const Vector3&, PhysicsQueryHit&, UINT64, float) const */
		bool rayCast(const Vector3& origin, const Vector3& unitDir, PhysicsQueryHit& hit,
			UINT64 layer = BS_ALL_LAYERS, float max = FLT_MAX) const override;

		/** @copydoc PhysicsScene::boxCast */
		bool boxCast(const AABox& box, const Quaternion& rotation, const Vector3& unitDir, PhysicsQueryHit& hit,
			UINT64 layer = BS_ALL_LAYERS, float max = FLT_MAX) const override;

		/** @copydoc PhysicsScene::sphereCast */
		bool sphereCast(const Sphere& sphere, const Vector3& unitDir, PhysicsQueryHit& hit,
			UINT64 layer = BS_ALL_LAYERS, float max = FLT_MAX) const override;

		/** @copydoc PhysicsScene::capsuleCast */
		bool capsuleCast(const Capsule& capsule, const Quaternion& rotation, const Vector3& unitDir,
			PhysicsQueryHit& hit, UINT64 layer = BS_ALL_LAYERS, float max = FLT_MAX) const override;

		/** @copydoc PhysicsScene::convexCast */
		bool convexCast(const HPhysicsMesh& mesh, const Vector3& position, const Quaternion& rotation,
			const Vector3& unitDir, PhysicsQueryHit& hit, UINT64 layer = BS_ALL_LAYERS, float max = FLT_MAX) const override;

		/** @copydoc PhysicsScene::rayCastAll(const Vector3&, const Vector3&, UINT64, float) const */
		Vector<PhysicsQueryHit> rayCastAll(const Vector3& origin, const Vector3& unitDir,
			UINT64 layer = BS_ALL_LAYERS, float max = FLT_MAX) const override;

		/** @copydoc PhysicsScene::boxCastAll */
		Vector<PhysicsQueryHit> boxCastAll(const AABox& box, const Quaternion& rotation,
			const Vector3& unitDir, UINT64 layer = BS_ALL_LAYERS, float max = FLT_MAX) const override;

		/** @copydoc PhysicsScene::sphereCastAll */
		Vector<PhysicsQueryHit> sphereCastAll(const Sphere& sphere, const Vector3& unitDir,
			UINT64 layer = BS_ALL_LAYERS, float max = FLT_MAX) const override;

		/** @copydoc PhysicsScene::capsuleCastAll */
		Vector<PhysicsQueryHit> capsuleCastAll(const Capsule& capsule, const Quaternion& rotation,
			const Vector3& unitDir, UINT64 layer = BS_ALL_LAYERS, float max = FLT_MAX) const override;

		/** @copydoc PhysicsScene::convexCastAll */
		Vector<PhysicsQueryHit> convexCastAll(const HPhysicsMesh& mesh, const Vector3& position,
			const Quaternion& rotation, const Vector3& unitDir, UINT64 layer = BS_ALL_LAYERS, 
			float max = FLT_MAX) const override;

		/** @copydoc PhysicsScene::rayCastAny(const Vector3&, const Vector3&, UINT64, float) const */
		bool rayCastAny(const Vector3& origin, const Vector3& unitDir,
			UINT64 layer = BS_ALL_LAYERS, float max = FLT_MAX) const override;

		/** @copydoc PhysicsScene::boxCastAny */
		bool boxCastAny(const AABox& box, const Quaternion& rotation, const Vector3& unitDir,
			UINT64 layer = BS_ALL_LAYERS, float max = FLT_MAX) const override;

		/** @copydoc PhysicsScene::sphereCastAny */
		bool sphereCastAny(const Sphere& sphere, const Vector3& unitDir,
			UINT64 layer = BS_ALL_LAYERS, float max = FLT_MAX) const override;

		/** @copydoc PhysicsScene::capsuleCastAny */
		bool capsuleCastAny(const Capsule& capsule, const Quaternion& rotation, const Vector3& unitDir,
			UINT64 layer = BS_ALL_LAYERS, float max = FLT_MAX) const override;

		/** @copydoc PhysicsScene::convexCastAny */
		bool convexCastAny(const HPhysicsMesh& mesh, const Vector3& position, const Quaternion& rotation,
			const Vector3& unitDir, UINT64 layer = BS_ALL_LAYERS, float max = FLT_MAX) const override;

		/** @copydoc PhysicsScene::boxOverlapAny */
		bool boxOverlapAny(const AABox& box, const Quaternion& rotation, UINT64 layer = BS_ALL_LAYERS) const override;

		/** @copydoc PhysicsScene::sphereOverlapAny */
		bool sphereOverlapAny(const Sphere& sphere, UINT64 layer = BS_ALL_LAYERS) const override;

		/** @copydoc PhysicsScene::capsuleOverlapAny */
		bool capsuleOverlapAny(const Capsule& capsule, const Quaternion& rotation,
			UINT64 layer = BS_ALL_LAYERS) const override;

		/** @copydoc PhysicsScene::convexOverlapAny */
		bool convexOverlapAny(const HPhysicsMesh& mesh, const Vector3& position, const Quaternion& rotation,
			UINT64 layer = BS_ALL_LAYERS) const override;

		/** @copydoc PhysicsScene::getGravity */
		Vector3 getGravity() const override;

		/** @copydoc PhysicsScene::setGravity */
		void setGravity(const Vector3& gravity) override;

		/** @copydoc PhysicsScene::addBroadPhaseRegion */
		UINT32 addBroadPhaseRegion(const AABox& region) override;

		/** @copydoc PhysicsScene::removeBroadPhaseRegion */
		void removeBroadPhaseRegion(UINT32 regionId) override;

		/** @copydoc PhysicsScene::clearBroadPhaseRegions */
		void clearBroadPhaseRegions() override;

		/** @copydoc PhysicsScene::setFlag */
		void setFlag(PhysicsFlags flags, bool enabled) override;

		/** @copydoc PhysicsScene::getMaxTesselationEdgeLength */
		float getMaxTesselationEdgeLength() const override { return mTesselationLength; }

		/** @copydoc PhysicsScene::setMaxTesselationEdgeLength */
		void setMaxTesselationEdgeLength(float length) override;

		/** @copydoc PhysicsScene::_boxOverlap */
		Vector<Collider*> _boxOverlap(const AABox& box, const Quaternion& rotation,
			UINT64 layer = BS_ALL_LAYERS) const override;

		/** @copydoc PhysicsScene::_sphereOverlap */
		Vector<Collider*> _sphereOverlap(const Sphere& sphere, UINT64 layer = BS_ALL_LAYERS) const override;

		/** @copydoc PhysicsScene::_capsuleOverlap */
		Vector<Collider*> _capsuleOverlap(const Capsule& capsule, const Quaternion& rotation,
			UINT64 layer = BS_ALL_LAYERS) const override;

		/** @copydoc PhysicsScene::_convexOverlap */
		Vector<Collider*> _convexOverlap(const HPhysicsMesh& mesh, const Vector3& position,
			const Quaternion& rotation, UINT64 layer = BS_ALL_LAYERS) const override;

	private:
		/**
		 * Helper method that performs a sweep query by checking if the provided geometry hits any physics objects
		 * when moved along the specified direction. Returns information about the first hit.
		 */
		inline bool sweep(const physx::PxGeometry& geometry, const physx::PxTransform& tfrm, const Vector3& unitDir,
			PhysicsQueryHit& hit, UINT64 layer, float maxDist) const;

		/**
		 * Helper method that performs a sweep query by checking if the provided geometry hits any physics objects
		 * when moved along the specified direction. Returns information about all hit.
		 */
		inline Vector<PhysicsQueryHit> sweepAll(const physx::PxGeometry& geometry, const physx::PxTransform& tfrm, 
			const Vector3& unitDir, UINT64 layer, float maxDist) const;

		/**
		 * Helper method that performs a sweep query by checking if the provided geometry hits any physics objects
		 * when moved along the specified direction. Returns no information about the hit, but rather just if it happened or
		 * not.
		 */
		inline bool sweepAny(const physx::PxGeometry& geometry, const physx::PxTransform& tfrm, const Vector3& unitDir,
			UINT64 layer, float maxDist) const;

		/** Helper method that returns all colliders that are overlapping the provided geometry. */
		inline Vector<Collider*> overlap(const physx::PxGeometry& geometry, const physx::PxTransform& tfrm, 
			UINT64 layer) const;

		/** Helper method that checks if the provided geometry overlaps any physics object. */
		inline bool overlapAny(const physx::PxGeometry& geometry, const physx::PxTransform& tfrm, UINT64 layer) const;

	private:
		friend class PhysX;

		float mTesselationLength = 3.0f;

		UnorderedMap<UINT32, UINT32> mBroadPhaseRegionHandles;
		UINT32 mNextRegionIdx = 1;

		physx::PxPhysics* mPhysics = nullptr;
		physx::PxScene* mScene = nullptr;
		physx::PxControllerManager* mCharManager = nullptr;
	};

	/** Provides easier access to PhysX. */
	PhysX& gPhysX();

	/** @} */
}