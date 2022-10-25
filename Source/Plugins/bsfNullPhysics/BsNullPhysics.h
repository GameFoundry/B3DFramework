//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsNullPhysicsPrerequisites.h"
#include "Physics/BsPhysics.h"
#include "Physics/BsPhysicsCommon.h"

namespace bs
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

		/** @copydoc Physics::fixedUpdate */
		void FixedUpdate(float step) override {}

		/** @copydoc Physics::update */
		void Update() override {}

		/** @copydoc Physics::createMaterial */
		SPtr<PhysicsMaterial> CreateMaterial(float staticFriction, float dynamicFriction, float restitution);

		/** @copydoc Physics::createMesh */
		SPtr<PhysicsMesh> CreateMesh(const SPtr<MeshData>& meshData, PhysicsMeshType type);

		/** @copydoc Physics::createPhysicsScene */
		SPtr<PhysicsScene> CreatePhysicsScene();

		/** @copydoc Physics::setPaused */
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
		SPtr<D6Joint> CreateD6Joint(const D6_JOINT_DESC& desc);

		/** @copydoc PhysicsScene::createCharacterController*/
		SPtr<CharacterController> CreateCharacterController(const CHAR_CONTROLLER_DESC& desc);

		/** @copydoc PhysicsScene::rayCast(const Vector3&, const Vector3&, PhysicsQueryHit&, u64, float) const */
		bool RayCast(const Vector3& origin, const Vector3& unitDir, PhysicsQueryHit& hit, u64 layer = BS_ALL_LAYERS, float max = FLT_MAX) const override
		{
			return false;
		}

		/** @copydoc PhysicsScene::boxCast */
		bool BoxCast(const AABox& box, const Quaternion& rotation, const Vector3& unitDir, PhysicsQueryHit& hit, u64 layer = BS_ALL_LAYERS, float max = FLT_MAX) const override
		{
			return false;
		}

		/** @copydoc PhysicsScene::sphereCast */
		bool SphereCast(const Sphere& sphere, const Vector3& unitDir, PhysicsQueryHit& hit, u64 layer = BS_ALL_LAYERS, float max = FLT_MAX) const override
		{
			return false;
		}

		/** @copydoc PhysicsScene::capsuleCast */
		bool CapsuleCast(const Capsule& capsule, const Quaternion& rotation, const Vector3& unitDir, PhysicsQueryHit& hit, u64 layer = BS_ALL_LAYERS, float max = FLT_MAX) const override
		{
			return false;
		}

		/** @copydoc PhysicsScene::convexCast */
		bool ConvexCast(const HPhysicsMesh& mesh, const Vector3& position, const Quaternion& rotation, const Vector3& unitDir, PhysicsQueryHit& hit, u64 layer = BS_ALL_LAYERS, float max = FLT_MAX) const override
		{
			return false;
		}

		/** @copydoc PhysicsScene::rayCastAll(const Vector3&, const Vector3&, u64, float) const */
		Vector<PhysicsQueryHit> RayCastAll(const Vector3& origin, const Vector3& unitDir, u64 layer = BS_ALL_LAYERS, float max = FLT_MAX) const
		{
			return {};
		}

		/** @copydoc PhysicsScene::boxCastAll */
		Vector<PhysicsQueryHit> BoxCastAll(const AABox& box, const Quaternion& rotation, const Vector3& unitDir, u64 layer = BS_ALL_LAYERS, float max = FLT_MAX) const
		{
			return {};
		}

		/** @copydoc PhysicsScene::sphereCastAll */
		Vector<PhysicsQueryHit> SphereCastAll(const Sphere& sphere, const Vector3& unitDir, u64 layer = BS_ALL_LAYERS, float max = FLT_MAX) const
		{
			return {};
		}

		/** @copydoc PhysicsScene::capsuleCastAll */
		Vector<PhysicsQueryHit> CapsuleCastAll(const Capsule& capsule, const Quaternion& rotation, const Vector3& unitDir, u64 layer = BS_ALL_LAYERS, float max = FLT_MAX) const override
		{
			return {};
		}

		/** @copydoc PhysicsScene::convexCastAll */
		Vector<PhysicsQueryHit> ConvexCastAll(const HPhysicsMesh& mesh, const Vector3& position, const Quaternion& rotation, const Vector3& unitDir, u64 layer = BS_ALL_LAYERS, float max = FLT_MAX) const
		{
			return {};
		}

		/** @copydoc PhysicsScene::rayCastAny(const Vector3&, const Vector3&, u64, float) const */
		bool RayCastAny(const Vector3& origin, const Vector3& unitDir, u64 layer = BS_ALL_LAYERS, float max = FLT_MAX) const override
		{
			return false;
		}

		/** @copydoc PhysicsScene::boxCastAny */
		bool BoxCastAny(const AABox& box, const Quaternion& rotation, const Vector3& unitDir, u64 layer = BS_ALL_LAYERS, float max = FLT_MAX) const override
		{
			return false;
		}

		/** @copydoc PhysicsScene::sphereCastAny */
		bool SphereCastAny(const Sphere& sphere, const Vector3& unitDir, u64 layer = BS_ALL_LAYERS, float max = FLT_MAX) const override
		{
			return false;
		}

		/** @copydoc PhysicsScene::capsuleCastAny */
		bool CapsuleCastAny(const Capsule& capsule, const Quaternion& rotation, const Vector3& unitDir, u64 layer = BS_ALL_LAYERS, float max = FLT_MAX) const override
		{
			return false;
		}

		/** @copydoc PhysicsScene::convexCastAny */
		bool ConvexCastAny(const HPhysicsMesh& mesh, const Vector3& position, const Quaternion& rotation, const Vector3& unitDir, u64 layer = BS_ALL_LAYERS, float max = FLT_MAX) const override
		{
			return false;
		}

		/** @copydoc PhysicsScene::boxOverlapAny */
		bool BoxOverlapAny(const AABox& box, const Quaternion& rotation, u64 layer = BS_ALL_LAYERS) const override
		{
			return false;
		}

		/** @copydoc PhysicsScene::sphereOverlapAny */
		bool SphereOverlapAny(const Sphere& sphere, u64 layer = BS_ALL_LAYERS) const override { return false; }

		/** @copydoc PhysicsScene::capsuleOverlapAny */
		bool CapsuleOverlapAny(const Capsule& capsule, const Quaternion& rotation, u64 layer = BS_ALL_LAYERS) const override { return false; }

		/** @copydoc PhysicsScene::convexOverlapAny */
		bool ConvexOverlapAny(const HPhysicsMesh& mesh, const Vector3& position, const Quaternion& rotation, u64 layer = BS_ALL_LAYERS) const override { return false; }

		/** @copydoc PhysicsScene::getGravity */
		Vector3 GetGravity() const override { return mGravity; }

		/** @copydoc PhysicsScene::setGravity */
		void SetGravity(const Vector3& gravity) override { mGravity = gravity; }

		/** @copydoc PhysicsScene::addBroadPhaseRegion */
		u32 AddBroadPhaseRegion(const AABox& region) override { return 0; }

		/** @copydoc PhysicsScene::removeBroadPhaseRegion */
		void RemoveBroadPhaseRegion(u32 regionId) override {}

		/** @copydoc PhysicsScene::clearBroadPhaseRegions */
		void ClearBroadPhaseRegions() override {}

		/** @copydoc PhysicsScene::setFlag */
		void SetFlag(PhysicsFlags flags, bool enabled) override {}

		/** @copydoc PhysicsScene::getMaxTesselationEdgeLength */
		float GetMaxTesselationEdgeLength() const override { return mTesselationLength; }

		/** @copydoc PhysicsScene::setMaxTesselationEdgeLength */
		void SetMaxTesselationEdgeLength(float length) override { mTesselationLength = length; }

		/** @copydoc PhysicsScene::BoxOverlapInternal */
		Vector<Collider*> BoxOverlapInternal(const AABox& box, const Quaternion& rotation, u64 layer = BS_ALL_LAYERS) const override { return {}; }

		/** @copydoc PhysicsScene::SphereOverlapInternal */
		Vector<Collider*> SphereOverlapInternal(const Sphere& sphere, u64 layer = BS_ALL_LAYERS) const override { return {}; }

		/** @copydoc PhysicsScene::CapsuleOverlapInternal */
		Vector<Collider*> CapsuleOverlapInternal(const Capsule& capsule, const Quaternion& rotation, u64 layer = BS_ALL_LAYERS) const override { return {}; }

		/** @copydoc PhysicsScene::ConvexOverlapInternal */
		Vector<Collider*> ConvexOverlapInternal(const HPhysicsMesh& mesh, const Vector3& position, const Quaternion& rotation, u64 layer = BS_ALL_LAYERS) const override { return {}; }

	private:
		friend class NullPhysics;

		float mTesselationLength = 3.0f;
		Vector3 mGravity = Vector3(0.0f, -9.81f, 0.0f);
	};

	/** Provides easier access to NullPhysics. */
	NullPhysics& gNullPhysics();

	/** @} */
} // namespace bs
