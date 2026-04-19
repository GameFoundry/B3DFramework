//************************************ B3D Framework - Copyright 2026 Marko Pintera **************************************//
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
		NullPhysics(const PhysicsCreateInformation& createInformation);
		~NullPhysics();

		SPtr<PhysicsMaterial> CreateMaterial(float staticFriction, float dynamicFriction, float restitution) override;
		UPtr<IPhysicsMeshImplementation> CreateMesh(const SPtr<MeshData>& meshData, PhysicsMeshType type) override;
		SPtr<PhysicsScene> CreatePhysicsScene() override;
		SPtr<ColliderShape> CreateColliderShape() override;
		UPtr<IColliderImplementation> CreateColliderImplementation() override;
		UPtr<IRigidbodyImplementation> CreateRigidbodyImplementation(Rigidbody& owner) override;

		bool RayCast(const Vector3& origin, const Vector3& unitDirection, const ColliderShape& colliderShape, PhysicsQueryHit& hit, float maximumDistance = FLT_MAX) const override { return false; }
		bool RayCast(const Vector3& origin, const Vector3& unitDirection, const Collider& collider, PhysicsQueryHit& hit, float maximumDistance = FLT_MAX) const override { return false; }

		/** Notifies the system that a physics scene is about to be destroyed. */
		void NotifySceneDestroyedInternal(NullPhysicsScene* scene);

	private:
		PhysicsCreateInformation mInitDesc;
		Vector<NullPhysicsScene*> mScenes;
	};

	/** Contains information about a single physics scene. */
	class NullPhysicsScene : public PhysicsScene
	{
	public:
		NullPhysicsScene(const PhysicsCreateInformation& createInformation);
		~NullPhysicsScene();

		void FixedUpdate(float step) override {}
		void SetPaused(bool paused) override {}

		UPtr<IFixedJointImplementation> CreateFixedJoint(Joint& owner, const FixedJointCreateInformation& createInformation) override;
		UPtr<IDistanceJointImplementation> CreateDistanceJoint(Joint& owner, const DistanceJointCreateInformation& createInformation) override;
		UPtr<IHingeJointImplementation> CreateHingeJoint(Joint& owner, const HingeJointCreateInformation& createInformation) override;
		UPtr<ISphericalJointImplementation> CreateSphericalJoint(Joint& owner, const SphericalJointCreateInformation& createInformation) override;
		UPtr<ISliderJointImplementation> CreateSliderJoint(Joint& owner, const SliderJointCreateInformation& createInformation) override;
		UPtr<ID6JointImplementation> CreateD6Joint(Joint& owner, const D6JointCreateInformation& createInformation) override;
		UPtr<ICharacterControllerImplementation> CreateCharacterController(CharacterController& owner, const CharacterControllerCreateInformation& createInformation) override;

		bool RayCast(const Vector3& origin, const Vector3& unitDir, PhysicsQueryHit& hit, u64 layer = BS_ALL_LAYERS, float max = FLT_MAX) const override { return false; }
		bool BoxCast(const AABox& box, const Quaternion& rotation, const Vector3& unitDir, PhysicsQueryHit& hit, u64 layer = BS_ALL_LAYERS, float max = FLT_MAX) const override { return false; }
		bool SphereCast(const Sphere& sphere, const Vector3& unitDir, PhysicsQueryHit& hit, u64 layer = BS_ALL_LAYERS, float max = FLT_MAX) const override { return false; }
		bool CapsuleCast(const Capsule& capsule, const Quaternion& rotation, const Vector3& unitDir, PhysicsQueryHit& hit, u64 layer = BS_ALL_LAYERS, float max = FLT_MAX) const override { return false; }
		bool ConvexCast(const HPhysicsMesh& mesh, const Vector3& position, const Quaternion& rotation, const Vector3& unitDir, PhysicsQueryHit& hit, u64 layer = BS_ALL_LAYERS, float max = FLT_MAX) const override { return false; }
		Vector<PhysicsQueryHit> RayCastAll(const Vector3& origin, const Vector3& unitDir, u64 layer = BS_ALL_LAYERS, float max = FLT_MAX) const override { return {}; }
		Vector<PhysicsQueryHit> BoxCastAll(const AABox& box, const Quaternion& rotation, const Vector3& unitDir, u64 layer = BS_ALL_LAYERS, float max = FLT_MAX) const override { return {}; }
		Vector<PhysicsQueryHit> SphereCastAll(const Sphere& sphere, const Vector3& unitDir, u64 layer = BS_ALL_LAYERS, float max = FLT_MAX) const override { return {}; }
		Vector<PhysicsQueryHit> CapsuleCastAll(const Capsule& capsule, const Quaternion& rotation, const Vector3& unitDir, u64 layer = BS_ALL_LAYERS, float max = FLT_MAX) const override { return {}; }
		Vector<PhysicsQueryHit> ConvexCastAll(const HPhysicsMesh& mesh, const Vector3& position, const Quaternion& rotation, const Vector3& unitDir, u64 layer = BS_ALL_LAYERS, float max = FLT_MAX) const override { return {}; }
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
		float GetMaxTesselationEdgeLength() const override { return mTesselationLength; }
		void SetMaxTesselationEdgeLength(float length) override { mTesselationLength = length; }

		Vector<ColliderShape*> BoxOverlapInternal(const AABox& box, const Quaternion& rotation, u64 layer = BS_ALL_LAYERS) const override { return {}; }
		Vector<ColliderShape*> SphereOverlapInternal(const Sphere& sphere, u64 layer = BS_ALL_LAYERS) const override { return {}; }
		Vector<ColliderShape*> CapsuleOverlapInternal(const Capsule& capsule, const Quaternion& rotation, u64 layer = BS_ALL_LAYERS) const override { return {}; }
		Vector<ColliderShape*> ConvexOverlapInternal(const HPhysicsMesh& mesh, const Vector3& position, const Quaternion& rotation, u64 layer = BS_ALL_LAYERS) const override { return {}; }

	private:
		friend class NullPhysics;

		float mTesselationLength = 3.0f;
		Vector3 mGravity = Vector3(0.0f, -9.81f, 0.0f);
	};

	/** Provides easier access to NullPhysics. */
	NullPhysics& GetNullPhysics();

	/** @} */
} // namespace b3d
