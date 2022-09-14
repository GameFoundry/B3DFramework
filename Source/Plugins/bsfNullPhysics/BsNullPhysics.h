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
		void fixedUpdate(float step) override { }

		/** @copydoc Physics::update */
		void update() override { }

		/** @copydoc Physics::createMaterial */
		SPtr<PhysicsMaterial> createMaterial(float staticFriction, float dynamicFriction, float restitution) override;

		/** @copydoc Physics::createMesh */
		SPtr<PhysicsMesh> createMesh(const SPtr<MeshData>& meshData, PhysicsMeshType type) override;

		/** @copydoc Physics::createPhysicsScene */
		SPtr<PhysicsScene> createPhysicsScene() override;

		/** @copydoc Physics::setPaused */
		void setPaused(bool paused) override { }

		bool RayCastInternal(const Vector3& origin, const Vector3& unitDir, const Collider& collider, PhysicsQueryHit& hit,
			float maxDist = FLT_MAX) const override { return false; }

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
			UINT64 layer = BS_ALL_LAYERS, float max = FLT_MAX) const override
		{
			return false;
		}

		/** @copydoc PhysicsScene::boxCast */
		bool boxCast(const AABox& box, const Quaternion& rotation, const Vector3& unitDir, PhysicsQueryHit& hit,
			UINT64 layer = BS_ALL_LAYERS, float max = FLT_MAX) const override
		{
			return false;
		}

		/** @copydoc PhysicsScene::sphereCast */
		bool sphereCast(const Sphere& sphere, const Vector3& unitDir, PhysicsQueryHit& hit,
			UINT64 layer = BS_ALL_LAYERS, float max = FLT_MAX) const override
		{
			return false;
		}

		/** @copydoc PhysicsScene::capsuleCast */
		bool capsuleCast(const Capsule& capsule, const Quaternion& rotation, const Vector3& unitDir,
			PhysicsQueryHit& hit, UINT64 layer = BS_ALL_LAYERS, float max = FLT_MAX) const override
		{
			return false;
		}

		/** @copydoc PhysicsScene::convexCast */
		bool convexCast(const HPhysicsMesh& mesh, const Vector3& position, const Quaternion& rotation,
			const Vector3& unitDir, PhysicsQueryHit& hit, UINT64 layer = BS_ALL_LAYERS, float max = FLT_MAX) const override
		{
			return false;
		}

		/** @copydoc PhysicsScene::rayCastAll(const Vector3&, const Vector3&, UINT64, float) const */
		Vector<PhysicsQueryHit> rayCastAll(const Vector3& origin, const Vector3& unitDir,
			UINT64 layer = BS_ALL_LAYERS, float max = FLT_MAX) const override
		{
			return {};
		}

		/** @copydoc PhysicsScene::boxCastAll */
		Vector<PhysicsQueryHit> boxCastAll(const AABox& box, const Quaternion& rotation,
			const Vector3& unitDir, UINT64 layer = BS_ALL_LAYERS, float max = FLT_MAX) const override
		{
			return {};
		}

		/** @copydoc PhysicsScene::sphereCastAll */
		Vector<PhysicsQueryHit> sphereCastAll(const Sphere& sphere, const Vector3& unitDir,
			UINT64 layer = BS_ALL_LAYERS, float max = FLT_MAX) const override
		{
			return {};
		}

		/** @copydoc PhysicsScene::capsuleCastAll */
		Vector<PhysicsQueryHit> capsuleCastAll(const Capsule& capsule, const Quaternion& rotation,
			const Vector3& unitDir, UINT64 layer = BS_ALL_LAYERS, float max = FLT_MAX) const override
		{
			return {};
		}

		/** @copydoc PhysicsScene::convexCastAll */
		Vector<PhysicsQueryHit> convexCastAll(const HPhysicsMesh& mesh, const Vector3& position,
			const Quaternion& rotation, const Vector3& unitDir, UINT64 layer = BS_ALL_LAYERS,
			float max = FLT_MAX) const override
		{
			return {};
		}

		/** @copydoc PhysicsScene::rayCastAny(const Vector3&, const Vector3&, UINT64, float) const */
		bool rayCastAny(const Vector3& origin, const Vector3& unitDir,
			UINT64 layer = BS_ALL_LAYERS, float max = FLT_MAX) const override
		{
			return false;
		}

		/** @copydoc PhysicsScene::boxCastAny */
		bool boxCastAny(const AABox& box, const Quaternion& rotation, const Vector3& unitDir,
			UINT64 layer = BS_ALL_LAYERS, float max = FLT_MAX) const override
		{
			return false;
		}

		/** @copydoc PhysicsScene::sphereCastAny */
		bool sphereCastAny(const Sphere& sphere, const Vector3& unitDir,
			UINT64 layer = BS_ALL_LAYERS, float max = FLT_MAX) const override
		{
			return false;
		}

		/** @copydoc PhysicsScene::capsuleCastAny */
		bool capsuleCastAny(const Capsule& capsule, const Quaternion& rotation, const Vector3& unitDir,
			UINT64 layer = BS_ALL_LAYERS, float max = FLT_MAX) const override
		{
			return false;
		}

		/** @copydoc PhysicsScene::convexCastAny */
		bool convexCastAny(const HPhysicsMesh& mesh, const Vector3& position, const Quaternion& rotation,
			const Vector3& unitDir, UINT64 layer = BS_ALL_LAYERS, float max = FLT_MAX) const override
		{
			return false;
		}

		/** @copydoc PhysicsScene::boxOverlapAny */
		bool boxOverlapAny(const AABox& box, const Quaternion& rotation, UINT64 layer = BS_ALL_LAYERS) const override
		{
			return false;
		}

		/** @copydoc PhysicsScene::sphereOverlapAny */
		bool sphereOverlapAny(const Sphere& sphere, UINT64 layer = BS_ALL_LAYERS) const override { return false; }

		/** @copydoc PhysicsScene::capsuleOverlapAny */
		bool capsuleOverlapAny(const Capsule& capsule, const Quaternion& rotation,
			UINT64 layer = BS_ALL_LAYERS) const override { return false; }

		/** @copydoc PhysicsScene::convexOverlapAny */
		bool convexOverlapAny(const HPhysicsMesh& mesh, const Vector3& position, const Quaternion& rotation,
			UINT64 layer = BS_ALL_LAYERS) const override { return false; }

		/** @copydoc PhysicsScene::getGravity */
		Vector3 getGravity() const override { return mGravity; }

		/** @copydoc PhysicsScene::setGravity */
		void setGravity(const Vector3& gravity) override { mGravity = gravity; }

		/** @copydoc PhysicsScene::addBroadPhaseRegion */
		UINT32 addBroadPhaseRegion(const AABox& region) override { return 0; }

		/** @copydoc PhysicsScene::removeBroadPhaseRegion */
		void removeBroadPhaseRegion(UINT32 regionId) override { }

		/** @copydoc PhysicsScene::clearBroadPhaseRegions */
		void clearBroadPhaseRegions() override { }

		/** @copydoc PhysicsScene::setFlag */
		void setFlag(PhysicsFlags flags, bool enabled) override { }

		/** @copydoc PhysicsScene::getMaxTesselationEdgeLength */
		float getMaxTesselationEdgeLength() const override { return mTesselationLength; }

		/** @copydoc PhysicsScene::setMaxTesselationEdgeLength */
		void setMaxTesselationEdgeLength(float length) override { mTesselationLength = length; }

		/** @copydoc PhysicsScene::BoxOverlapInternal */
		Vector<Collider*> BoxOverlapInternal(const AABox& box, const Quaternion& rotation,
			UINT64 layer = BS_ALL_LAYERS) const override { return {}; }

		/** @copydoc PhysicsScene::SphereOverlapInternal */
		Vector<Collider*> SphereOverlapInternal(const Sphere& sphere, UINT64 layer = BS_ALL_LAYERS) const override { return {}; }

		/** @copydoc PhysicsScene::CapsuleOverlapInternal */
		Vector<Collider*> CapsuleOverlapInternal(const Capsule& capsule, const Quaternion& rotation,
			UINT64 layer = BS_ALL_LAYERS) const override { return {}; }

		/** @copydoc PhysicsScene::ConvexOverlapInternal */
		Vector<Collider*> ConvexOverlapInternal(const HPhysicsMesh& mesh, const Vector3& position,
			const Quaternion& rotation, UINT64 layer = BS_ALL_LAYERS) const override { return {}; }

	private:
		friend class NullPhysics;

		float mTesselationLength = 3.0f;
		Vector3 mGravity = Vector3(0.0f, -9.81f, 0.0f);
	};

	/** Provides easier access to NullPhysics. */
	NullPhysics& gNullPhysics();

	/** @} */
}
