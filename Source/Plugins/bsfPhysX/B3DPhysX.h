//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once


#include "B3DPhysXPrerequisites.h"
#include "Physics/B3DPhysics.h"
#include "Physics/B3DPhysicsCommon.h"
#include "PxPhysics.h"
#include "foundation/Px.h"
#include "characterkinematic/PxControllerManager.h"
#include "cooking/PxCooking.h"
#include "PxSimulationEventCallback.h"

namespace b3d
{
	class IColliderImplementation;
	/** @addtogroup PhysX
	 *  @{
	 */

	class PhysXScene;

	/** NVIDIA PhysX implementation of Physics. */
	class PhysX : public Physics
	{
	public:
		PhysX(const PhysicsCreateInformation& input);
		~PhysX();

		TShared<PhysicsMaterial> CreateMaterial(float staticFriction, float dynamicFriction, float restitution) override;
		UPtr<IPhysicsMeshImplementation> CreateMesh(const TShared<MeshData>& meshData, PhysicsMeshType type) override;
		TShared<PhysicsScene> CreatePhysicsScene() override;
		TShared<ColliderShape> CreateColliderShape() override;
		UPtr<IColliderImplementation> CreateColliderImplementation() override;
		UPtr<IRigidbodyImplementation> CreateRigidbodyImplementation(Rigidbody& owner) override;

		bool RayCast(const Vector3& origin, const Vector3& unitDirection, const ColliderShape& colliderShape, PhysicsQueryHit& hit, float maxDistance = FLT_MAX) const override;
		bool RayCast(const Vector3& origin, const Vector3& unitDirection, const Collider& collider, PhysicsQueryHit& hit, float maxDistance = FLT_MAX) const override;

		/** Notifies the system that at physics scene is about to be destroyed. */
		void NotifySceneDestroyed(PhysXScene* scene);

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

		PhysicsCreateInformation mInitDesc;

		Vector<PhysXScene*> mScenes;
		UnorderedMap<u32, u32> mBroadPhaseRegionHandles;

		physx::PxFoundation* mFoundation = nullptr;
		physx::PxPhysics* mPhysics = nullptr;
		physx::PxCooking* mCooking = nullptr;
		physx::PxMaterial* mDefaultMaterial = nullptr;
		physx::PxTolerancesScale mScale;
	};

	class PhysXEventCallback : public physx::PxSimulationEventCallback
	{
	public:
		PhysXEventCallback(PhysXScene& scene)
			:mScene(scene)
		{ }

	protected:
		void onWake(physx::PxActor** actors, physx::PxU32 count) override { /* Do nothing. */ }
		void onSleep(physx::PxActor** actors, physx::PxU32 count) override { /* Do nothing. */ }
		void onTrigger(physx::PxTriggerPair* pairs, physx::PxU32 count) override;
		void onContact(const physx::PxContactPairHeader& pairHeader, const physx::PxContactPair* pairs, physx::PxU32 count) override;
		void onConstraintBreak(physx::PxConstraintInfo* constraints, physx::PxU32 count) override;
		void onAdvance(const physx::PxRigidBody*const* bodyBuffer, const physx::PxTransform* poseBuffer, const physx::PxU32 count) override { /* Do nothing. */ }

	private:
		PhysXScene& mScene;
	};

	/** Contains information about a single PhysX scene. */
	class PhysXScene : public PhysicsScene
	{
	public:
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
			ColliderShape* Trigger; /** Trigger shape that was interacted with. */
			ColliderShape* Other; /** Collider shape that was interacted with. */
			ContactEventType Type; /** Exact type of the event. */
		};

		/** Event reported when two colliders interact. */
		struct ContactEvent
		{
			ColliderShape* ColliderShapeA; /** First collider shape. */
			ColliderShape* ColliderShapeB; /** Second collider shape. */
			ContactEventType Type; /** Exact type of the event. */
			// Note: Not too happy this is heap allocated, use static allocator?
			Vector<ContactPoint> Points; /** Information about all contact points between the colliders. */
		};

		/** Event reported when a joint breaks. */
		struct JointBreakEvent
		{
			Joint* Joint; /** Broken joint. */
		};

		PhysXScene(physx::PxPhysics* physics, const PhysicsCreateInformation& input, const physx::PxTolerancesScale& scale);
		~PhysXScene();

		void FixedUpdate(float step) override;
		void Update() override;
		void SetPaused(bool paused) override;

		/** Returns the underlying PhysX scene. */
		physx::PxScene& GetPxScene() const { return *mScene; }

		UPtr<IFixedJointImplementation> CreateFixedJoint(Joint& owner, const FixedJointCreateInformation& createInformation) override;
		UPtr<IDistanceJointImplementation> CreateDistanceJoint(Joint& owner, const DistanceJointCreateInformation& createInformation) override;
		UPtr<IHingeJointImplementation> CreateHingeJoint(Joint& owner, const HingeJointCreateInformation& createInformation) override;
		UPtr<ISphericalJointImplementation> CreateSphericalJoint(Joint& owner, const SphericalJointCreateInformation& createInformation) override;
		UPtr<ISliderJointImplementation> CreateSliderJoint(Joint& owner, const SliderJointCreateInformation& createInformation) override;
		UPtr<ID6JointImplementation> CreateD6Joint(Joint& owner, const D6JointCreateInformation& createInformation) override;
		UPtr<ICharacterControllerImplementation> CreateCharacterController(CharacterController& owner, const CharacterControllerCreateInformation& createInformation) override;

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

		Vector<ColliderShape*> BoxOverlapInternal(const AABox& box, const Quaternion& rotation, u64 layer = BS_ALL_LAYERS) const override;
		Vector<ColliderShape*> SphereOverlapInternal(const Sphere& sphere, u64 layer = BS_ALL_LAYERS) const override;
		Vector<ColliderShape*> CapsuleOverlapInternal(const Capsule& capsule, const Quaternion& rotation, u64 layer = BS_ALL_LAYERS) const override;
		Vector<ColliderShape*> ConvexOverlapInternal(const HPhysicsMesh& mesh, const Vector3& position, const Quaternion& rotation, u64 layer = BS_ALL_LAYERS) const override;

		/** Triggered by the PhysX simulation when an interaction between two colliders is found. */
		void ReportContactEvent(const ContactEvent& event);

		/** Triggered by the PhysX simulation when an interaction between two trigger and a collider is found. */
		void ReportTriggerEvent(const TriggerEvent& event);

		/** Triggered by the PhysX simulation when a joint breaks. */
		void ReportJointBreakEvent(const JointBreakEvent& event);

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
		inline Vector<ColliderShape*> Overlap(const physx::PxGeometry& geometry, const physx::PxTransform& tfrm, u64 layer) const;

		/** Helper method that checks if the provided geometry overlaps any physics object. */
		inline bool OverlapAny(const physx::PxGeometry& geometry, const physx::PxTransform& tfrm, u64 layer) const;

		/** Sends out all events recorded during simulation to the necessary physics objects. */
		void TriggerEvents();
	private:
		friend class PhysX;

		float mTesselationLength = 3.0f;

		UnorderedMap<u32, u32> mBroadPhaseRegionHandles;
		u32 mNextRegionIdx = 1;
		bool mPaused = false;

		physx::PxPhysics* mPhysics = nullptr;
		physx::PxScene* mScene = nullptr;
		physx::PxControllerManager* mCharManager = nullptr;
		PhysXEventCallback mEventCallback;

		Vector<TriggerEvent> mTriggerEvents;
		Vector<ContactEvent> mContactEvents;
		Vector<JointBreakEvent> mJointBreakEvents;
	};

	/** Provides easier access to PhysX. */
	PhysX& GetPhysX();

	/** @} */
} // namespace b3d
