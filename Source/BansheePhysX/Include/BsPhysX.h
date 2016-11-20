//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsPhysXPrerequisites.h"
#include "BsPhysics.h"
#include "BsPhysicsCommon.h"
#include "PxPhysics.h"
#include "foundation/Px.h"
#include "characterkinematic\PxControllerManager.h"
#include "cooking/PxCooking.h"

namespace bs
{
	/** @addtogroup PhysX
	 *  @{
	 */

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

		/** @copydoc Physics::update */
		void update() override;

		/** @copydoc Physics::createMaterial */
		SPtr<PhysicsMaterial> createMaterial(float staticFriction, float dynamicFriction, float restitution) override;

		/** @copydoc Physics::createMesh */
		SPtr<PhysicsMesh> createMesh(const SPtr<MeshData>& meshData, PhysicsMeshType type) override;

		/** @copydoc Physics::createRigidbody */
		SPtr<Rigidbody> createRigidbody(const HSceneObject& linkedSO) override;

		/** @copydoc Physics::createBoxCollider */
		SPtr<BoxCollider> createBoxCollider(const Vector3& extents, const Vector3& position,
			const Quaternion& rotation) override;

		/** @copydoc Physics::createSphereCollider */
		SPtr<SphereCollider> createSphereCollider(float radius, const Vector3& position, const Quaternion& rotation) override;

		/** @copydoc Physics::createPlaneCollider */
		SPtr<PlaneCollider> createPlaneCollider(const Vector3& position, const Quaternion& rotation) override;

		/** @copydoc Physics::createCapsuleCollider */
		SPtr<CapsuleCollider> createCapsuleCollider(float radius, float halfHeight, const Vector3& position, 
			const Quaternion& rotation) override;

		/** @copydoc Physics::createMeshCollider */
		SPtr<MeshCollider> createMeshCollider(const Vector3& position, const Quaternion& rotation) override;

		/** @copydoc Physics::createFixedJoint */
		SPtr<FixedJoint> createFixedJoint(const FIXED_JOINT_DESC& desc) override;

		/** @copydoc Physics::createDistanceJoint */
		SPtr<DistanceJoint> createDistanceJoint(const DISTANCE_JOINT_DESC& desc) override;

		/** @copydoc Physics::createHingeJoint */
		SPtr<HingeJoint> createHingeJoint(const HINGE_JOINT_DESC& desc) override;

		/** @copydoc Physics::createSphericalJoint */
		SPtr<SphericalJoint> createSphericalJoint(const SPHERICAL_JOINT_DESC& desc) override;

		/** @copydoc Physics::createSliderJoint */
		SPtr<SliderJoint> createSliderJoint(const SLIDER_JOINT_DESC& desc) override;

		/** @copydoc Physics::createD6Joint */
		SPtr<D6Joint> createD6Joint(const D6_JOINT_DESC& desc) override;

		/** @copydoc Physics::createCharacterController*/
		SPtr<CharacterController> createCharacterController(const CHAR_CONTROLLER_DESC& desc) override;

		/** @copydoc Physics::rayCast(const Vector3&, const Vector3&, PhysicsQueryHit&, UINT64, float) const */
		bool rayCast(const Vector3& origin, const Vector3& unitDir, PhysicsQueryHit& hit,
			UINT64 layer = BS_ALL_LAYERS, float max = FLT_MAX) const override;

		/** @copydoc Physics::boxCast */
		bool boxCast(const AABox& box, const Quaternion& rotation, const Vector3& unitDir, PhysicsQueryHit& hit,
			UINT64 layer = BS_ALL_LAYERS, float max = FLT_MAX) const override;

		/** @copydoc Physics::sphereCast */
		bool sphereCast(const Sphere& sphere, const Vector3& unitDir, PhysicsQueryHit& hit,
			UINT64 layer = BS_ALL_LAYERS, float max = FLT_MAX) const override;

		/** @copydoc Physics::capsuleCast */
		bool capsuleCast(const Capsule& capsule, const Quaternion& rotation, const Vector3& unitDir,
			PhysicsQueryHit& hit, UINT64 layer = BS_ALL_LAYERS, float max = FLT_MAX) const override;

		/** @copydoc Physics::convexCast */
		bool convexCast(const HPhysicsMesh& mesh, const Vector3& position, const Quaternion& rotation,
			const Vector3& unitDir, PhysicsQueryHit& hit, UINT64 layer = BS_ALL_LAYERS, float max = FLT_MAX) const override;

		/** @copydoc Physics::rayCastAll(const Vector3&, const Vector3&, UINT64, float) const */
		Vector<PhysicsQueryHit> rayCastAll(const Vector3& origin, const Vector3& unitDir,
			UINT64 layer = BS_ALL_LAYERS, float max = FLT_MAX) const override;

		/** @copydoc Physics::boxCastAll */
		Vector<PhysicsQueryHit> boxCastAll(const AABox& box, const Quaternion& rotation,
			const Vector3& unitDir, UINT64 layer = BS_ALL_LAYERS, float max = FLT_MAX) const override;

		/** @copydoc Physics::sphereCastAll */
		Vector<PhysicsQueryHit> sphereCastAll(const Sphere& sphere, const Vector3& unitDir,
			UINT64 layer = BS_ALL_LAYERS, float max = FLT_MAX) const override;

		/** @copydoc Physics::capsuleCastAll */
		Vector<PhysicsQueryHit> capsuleCastAll(const Capsule& capsule, const Quaternion& rotation,
			const Vector3& unitDir, UINT64 layer = BS_ALL_LAYERS, float max = FLT_MAX) const override;

		/** @copydoc Physics::convexCastAll */
		Vector<PhysicsQueryHit> convexCastAll(const HPhysicsMesh& mesh, const Vector3& position,
			const Quaternion& rotation, const Vector3& unitDir, UINT64 layer = BS_ALL_LAYERS, 
			float max = FLT_MAX) const override;

		/** @copydoc Physics::rayCastAny(const Vector3&, const Vector3&, UINT64, float) const */
		bool rayCastAny(const Vector3& origin, const Vector3& unitDir,
			UINT64 layer = BS_ALL_LAYERS, float max = FLT_MAX) const override;

		/** @copydoc Physics::boxCastAny */
		bool boxCastAny(const AABox& box, const Quaternion& rotation, const Vector3& unitDir,
			UINT64 layer = BS_ALL_LAYERS, float max = FLT_MAX) const override;

		/** @copydoc Physics::sphereCastAny */
		bool sphereCastAny(const Sphere& sphere, const Vector3& unitDir,
			UINT64 layer = BS_ALL_LAYERS, float max = FLT_MAX) const override;

		/** @copydoc Physics::capsuleCastAny */
		bool capsuleCastAny(const Capsule& capsule, const Quaternion& rotation, const Vector3& unitDir,
			UINT64 layer = BS_ALL_LAYERS, float max = FLT_MAX) const override;

		/** @copydoc Physics::convexCastAny */
		bool convexCastAny(const HPhysicsMesh& mesh, const Vector3& position, const Quaternion& rotation,
			const Vector3& unitDir, UINT64 layer = BS_ALL_LAYERS, float max = FLT_MAX) const override;

		/** @copydoc Physics::boxOverlapAny */
		bool boxOverlapAny(const AABox& box, const Quaternion& rotation, UINT64 layer = BS_ALL_LAYERS) const override;

		/** @copydoc Physics::sphereOverlapAny */
		bool sphereOverlapAny(const Sphere& sphere, UINT64 layer = BS_ALL_LAYERS) const override;

		/** @copydoc Physics::capsuleOverlapAny */
		bool capsuleOverlapAny(const Capsule& capsule, const Quaternion& rotation,
			UINT64 layer = BS_ALL_LAYERS) const override;

		/** @copydoc Physics::convexOverlapAny */
		bool convexOverlapAny(const HPhysicsMesh& mesh, const Vector3& position, const Quaternion& rotation,
			UINT64 layer = BS_ALL_LAYERS) const override;

		/** @copydoc Physics::setFlag */
		void setFlag(PhysicsFlags flags, bool enabled) override;

		/** @copydoc Physics::setPaused */
		void setPaused(bool paused) override;

		/** @copydoc Physics::getGravity */
		Vector3 getGravity() const override;

		/** @copydoc Physics::setGravity */
		void setGravity(const Vector3& gravity) override;

		/** @copydoc Physics::getMaxTesselationEdgeLength */
		float getMaxTesselationEdgeLength() const override { return mTesselationLength; }

		/** @copydoc Physics::setMaxTesselationEdgeLength */
		void setMaxTesselationEdgeLength(float length) override;

		/** @copydoc Physics::addBroadPhaseRegion */
		UINT32 addBroadPhaseRegion(const AABox& region) override;

		/** @copydoc Physics::removeBroadPhaseRegion */
		void removeBroadPhaseRegion(UINT32 regionId) override;

		/** @copydoc Physics::clearBroadPhaseRegions */
		void clearBroadPhaseRegions() override;

		/** @copydoc Physics::_boxOverlap */
		Vector<Collider*> _boxOverlap(const AABox& box, const Quaternion& rotation,
			UINT64 layer = BS_ALL_LAYERS) const override;

		/** @copydoc Physics::_sphereOverlap */
		Vector<Collider*> _sphereOverlap(const Sphere& sphere, UINT64 layer = BS_ALL_LAYERS) const override;

		/** @copydoc Physics::_capsuleOverlap */
		Vector<Collider*> _capsuleOverlap(const Capsule& capsule, const Quaternion& rotation,
			UINT64 layer = BS_ALL_LAYERS) const override;

		/** @copydoc Physics::_convexOverlap */
		Vector<Collider*> _convexOverlap(const HPhysicsMesh& mesh, const Vector3& position,
			const Quaternion& rotation, UINT64 layer = BS_ALL_LAYERS) const override;

		/** @copydoc Physics::_rayCast */
		bool _rayCast(const Vector3& origin, const Vector3& unitDir, const Collider& collider, PhysicsQueryHit& hit, 
			float maxDist = FLT_MAX) const override;

		/** Triggered by the PhysX simulation when an interaction between two colliders is found. */
		void _reportContactEvent(const ContactEvent& event);

		/** Triggered by the PhysX simulation when an interaction between two trigger and a collider is found. */
		void _reportTriggerEvent(const TriggerEvent& event);

		/** Triggered by the PhysX simulation when a joint breaks. */
		void _reportJointBreakEvent(const JointBreakEvent& event);

		/** Returns the default PhysX material. */
		physx::PxMaterial* getDefaultMaterial() const { return mDefaultMaterial; }

		/** Returns the main PhysX object. */
		physx::PxPhysics* getPhysX() const { return mPhysics; }

		/** Returns the main PhysX scene. */
		physx::PxScene* getScene() const { return mScene; }

		/** Returns the PhysX object used for mesh cooking. */
		physx::PxCooking* getCooking() const { return mCooking; }

		/** Returns default scale used in the PhysX scene. */
		physx::PxTolerancesScale getScale() const { return mScale; }

	private:
		friend class PhysXEventCallback;

		/** Sends out all events recorded during simulation to the necessary physics objects. */
		void triggerEvents();

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

		float mSimulationStep = 1.0f/60.0f;
		float mSimulationTime = 0.0f;
		float mFrameTime = 0.0f;
		float mTesselationLength = 3.0f;
		UINT32 mNextRegionIdx = 1;
		bool mPaused = false;

		Vector<TriggerEvent> mTriggerEvents;
		Vector<ContactEvent> mContactEvents;
		Vector<JointBreakEvent> mJointBreakEvents;
		UnorderedMap<UINT32, UINT32> mBroadPhaseRegionHandles;

		physx::PxFoundation* mFoundation = nullptr;
		physx::PxPhysics* mPhysics = nullptr;
		physx::PxCooking* mCooking = nullptr;
		physx::PxScene* mScene = nullptr;
		physx::PxControllerManager* mCharManager = nullptr;

		physx::PxMaterial* mDefaultMaterial = nullptr;
		physx::PxTolerancesScale mScale;

		static const UINT32 SCRATCH_BUFFER_SIZE;
		/** Determines how many physics updates per frame are allowed. Only relevant when framerate is low. */
		static const UINT32 MAX_ITERATIONS_PER_FRAME;
	};

	/** Provides easier access to PhysX. */
	PhysX& gPhysX();

	/** @} */
}