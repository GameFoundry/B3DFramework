//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DNullPhysicsPrerequisites.h"
#include "Physics/B3DPhysics.h"
#include "Physics/B3DPhysicsCommon.h"

namespace b3d
{
	/** @addtogroup NullPhysics
	 *  @{
	 */

	class NullPhysicsScene;

	/** Null implementation of Physics. */
	class NullPhysics : public Physics
	{
	public:
		NullPhysics(const PHYSICS_INIT_DESC& input);
		~NullPhysics();

		void FixedUpdate(float step) override {}
		void Update() override {}

		SPtr<PhysicsMaterial> CreateMaterial(float staticFriction, float dynamicFriction, float restitution) override;
		SPtr<PhysicsMesh> CreateMesh(const SPtr<MeshData>& meshData, PhysicsMeshType type) override;
		SPtr<PhysicsScene> CreatePhysicsScene() override;
		void SetPaused(bool paused) override {}

		bool RayCastInternal(const Vector3& origin, const Vector3& unitDir, const Collider& collider, PhysicsQueryHit& hit, float maxDist = FLT_MAX) const override { return false; }

		/** Notifies the system that at physics scene is about to be destroyed. */
		void NotifySceneDestroyedInternal(NullPhysicsScene* scene);

	private:
		friend class NullPhysicsEventCallback;

		PHYSICS_INIT_DESC mInitDesc;
		Vector<NullPhysicsScene*> mScenes;
	};

	/** Contains information about a single physics scene. */
	class NullPhysicsScene : public PhysicsScene
	{
	public:
		NullPhysicsScene(const PHYSICS_INIT_DESC& input);
		~NullPhysicsScene();

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

		bool RayCast(const Vector3& origin, const Vector3& unitDir, PhysicsQueryHit& hit, u64 layer = BS_ALL_LAYERS, float max = FLT_MAX) const override { return false; }
		bool BoxCast(const AABox& box, const Quaternion& rotation, const Vector3& unitDir, PhysicsQueryHit& hit, u64 layer = BS_ALL_LAYERS, float max = FLT_MAX) const override { return false; }
		bool SphereCast(const Sphere& sphere, const Vector3& unitDir, PhysicsQueryHit& hit, u64 layer = BS_ALL_LAYERS, float max = FLT_MAX) const override { return false; }
		bool CapsuleCast(const Capsule& capsule, const Quaternion& rotation, const Vector3& unitDir, PhysicsQueryHit& hit, u64 layer = BS_ALL_LAYERS, float max = FLT_MAX) const override { return false; }
		bool ConvexCast(const HPhysicsMesh& mesh, const Vector3& position, const Quaternion& rotation, const Vector3& unitDir, PhysicsQueryHit& hit, u64 layer = BS_ALL_LAYERS, float max = FLT_MAX) const override { return false; }
		Vector<PhysicsQueryHit> RayCastAll(const Vector3& origin, const Vector3& unitDir, u64 layer = BS_ALL_LAYERS, float max = FLT_MAX) const { return {}; }
		Vector<PhysicsQueryHit> BoxCastAll(const AABox& box, const Quaternion& rotation, const Vector3& unitDir, u64 layer = BS_ALL_LAYERS, float max = FLT_MAX) const { return {}; }
		Vector<PhysicsQueryHit> SphereCastAll(const Sphere& sphere, const Vector3& unitDir, u64 layer = BS_ALL_LAYERS, float max = FLT_MAX) const { return {}; }
		Vector<PhysicsQueryHit> CapsuleCastAll(const Capsule& capsule, const Quaternion& rotation, const Vector3& unitDir, u64 layer = BS_ALL_LAYERS, float max = FLT_MAX) const override { return {}; }
		Vector<PhysicsQueryHit> ConvexCastAll(const HPhysicsMesh& mesh, const Vector3& position, const Quaternion& rotation, const Vector3& unitDir, u64 layer = BS_ALL_LAYERS, float max = FLT_MAX) const { return {}; }
		bool RayCastAny(const Vector3& origin, const Vector3& unitDir, u64 layer = BS_ALL_LAYERS, float max = FLT_MAX) const override { return false; }
		bool BoxCastAny(const AABox& box, const Quaternion& rotation, const Vector3& unitDir, u64 layer = BS_ALL_LAYERS, float max = FLT_MAX) const override { return false; }
		bool SphereCastAny(const Sphere& sphere, const Vector3& unitDir, u64 layer = BS_ALL_LAYERS, float max = FLT_MAX) const override { return false; }
		bool CapsuleCastAny(const Capsule& capsule, const Quaternion& rotation, const Vector3& unitDir, u64 layer = BS_ALL_LAYERS, float max = FLT_MAX) const override { return false; }
		bool ConvexCastAny(const HPhysicsMesh& mesh, const Vector3& position, const Quaternion& rotation, const Vector3& unitDir, u64 layer = BS_ALL_LAYERS, float max = FLT_MAX) const override { return false; }

		bool BoxOverlapAny(const AABox& box, const Quaternion& rotation, u64 layer = BS_ALL_LAYERS) const override { return false; }
		bool SphereOverlapAny(const Sphere& sphere, u64 layer = BS_ALL_LAYERS) const override { return false; } 
		bool CapsuleOverlapAny(const Capsule& capsule, const Quaternion& rotation, u64 layer = BS_ALL_LAYERS) const override { return false; }
		bool ConvexOverlapAny(const HPhysicsMesh& mesh, const Vector3& position, const Quaternion& rotation, u64 layer = BS_ALL_LAYERS) const override { return false; }

		Vector3 GetGravity() const override { return mGravity; }
		void SetGravity(const Vector3& gravity) override { mGravity = gravity; }
		u32 AddBroadPhaseRegion(const AABox& region) override { return 0; }
		void RemoveBroadPhaseRegion(u32 regionId) override {}
		void ClearBroadPhaseRegions() override {}
		void SetFlag(PhysicsFlags flags, bool enabled) override {}
		float GetMaxTesselationEdgeLength() const override { return mTesselationLength; }
		void SetMaxTesselationEdgeLength(float length) override { mTesselationLength = length; }

		Vector<Collider*> BoxOverlapInternal(const AABox& box, const Quaternion& rotation, u64 layer = BS_ALL_LAYERS) const override { return {}; }
		Vector<Collider*> SphereOverlapInternal(const Sphere& sphere, u64 layer = BS_ALL_LAYERS) const override { return {}; }
		Vector<Collider*> CapsuleOverlapInternal(const Capsule& capsule, const Quaternion& rotation, u64 layer = BS_ALL_LAYERS) const override { return {}; }
		Vector<Collider*> ConvexOverlapInternal(const HPhysicsMesh& mesh, const Vector3& position, const Quaternion& rotation, u64 layer = BS_ALL_LAYERS) const override { return {}; }

	private:
		friend class NullPhysics;

		float mTesselationLength = 3.0f;
		Vector3 mGravity = Vector3(0.0f, -9.81f, 0.0f);
	};

	/** Provides easier access to NullPhysics. */
	NullPhysics& GetNullPhysics();

	/** @} */
} // namespace b3d
