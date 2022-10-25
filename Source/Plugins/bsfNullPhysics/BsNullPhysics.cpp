//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsNullPhysics.h"
#include "BsNullPhysicsMaterial.h"
#include "BsNullPhysicsMesh.h"
#include "BsNullPhysicsRigidbody.h"
#include "BsNullPhysicsColliders.h"
#include "BsNullPhysicsJoints.h"
#include "BsNullPhysicsCharacterController.h"
#include "Threading/BsTaskScheduler.h"
#include "Components/BsCCollider.h"
#include "Utility/BsTime.h"
#include "Math/BsVector3.h"
#include "Math/BsAABox.h"
#include "Math/BsCapsule.h"

namespace bs
{
	NullPhysics::NullPhysics(const PHYSICS_INIT_DESC& input)
		: Physics(input), mInitDesc(input)
	{}

	NullPhysics::~NullPhysics()
	{
		assert(mScenes.empty() && "All scenes must be freed before physics system shutdown");
	}

	SPtr<PhysicsMaterial> NullPhysics::CreateMaterial(float staticFriction, float dynamicFriction, float restitution)
	{
		return bs_core_ptr_new<NullPhysicsMaterial>(staticFriction, dynamicFriction, restitution);
	}

	SPtr<PhysicsMesh> NullPhysics::CreateMesh(const SPtr<MeshData>& meshData, PhysicsMeshType type)
	{
		return bs_core_ptr_new<NullPhysicsMesh>(meshData, type);
	}

	SPtr<PhysicsScene> NullPhysics::CreatePhysicsScene()
	{
		SPtr<NullPhysicsScene> scene = bs_shared_ptr_new<NullPhysicsScene>(mInitDesc);
		mScenes.push_back(scene.get());

		return scene;
	}

	void NullPhysics::NotifySceneDestroyedInternal(NullPhysicsScene* scene)
	{
		auto iterFind = std::find(mScenes.begin(), mScenes.end(), scene);
		assert(iterFind != mScenes.end());

		mScenes.erase(iterFind);
	}

	NullPhysicsScene::NullPhysicsScene(const PHYSICS_INIT_DESC& input)
	{}

	NullPhysicsScene::~NullPhysicsScene()
	{
		gNullPhysics().NotifySceneDestroyedInternal(this);
	}

	SPtr<Rigidbody> NullPhysicsScene::CreateRigidbody(const HSceneObject& linkedSO)
	{
		return bs_shared_ptr_new<NullPhysicsRigidbody>(linkedSO);
	}

	SPtr<BoxCollider> NullPhysicsScene::CreateBoxCollider(const Vector3& extents, const Vector3& position, const Quaternion& rotation)
	{
		return bs_shared_ptr_new<NullPhysicsBoxCollider>(position, rotation, extents);
	}

	SPtr<SphereCollider> NullPhysicsScene::CreateSphereCollider(float radius, const Vector3& position, const Quaternion& rotation)
	{
		return bs_shared_ptr_new<NullPhysicsSphereCollider>(position, rotation, radius);
	}

	SPtr<PlaneCollider> NullPhysicsScene::CreatePlaneCollider(const Vector3& position, const Quaternion& rotation)
	{
		return bs_shared_ptr_new<NullPhysicsPlaneCollider>(position, rotation);
	}

	SPtr<CapsuleCollider> NullPhysicsScene::CreateCapsuleCollider(float radius, float halfHeight, const Vector3& position, const Quaternion& rotation)
	{
		return bs_shared_ptr_new<NullPhysicsCapsuleCollider>(position, rotation, radius, halfHeight);
	}

	SPtr<MeshCollider> NullPhysicsScene::CreateMeshCollider(const Vector3& position, const Quaternion& rotation)
	{
		return bs_shared_ptr_new<NullPhysicsMeshCollider>(position, rotation);
	}

	SPtr<FixedJoint> NullPhysicsScene::CreateFixedJoint(const FIXED_JOINT_DESC& desc)
	{
		return bs_shared_ptr_new<NullPhysicsFixedJoint>(desc);
	}

	SPtr<DistanceJoint> NullPhysicsScene::CreateDistanceJoint(const DISTANCE_JOINT_DESC& desc)
	{
		return bs_shared_ptr_new<NullPhysicsDistanceJoint>(desc);
	}

	SPtr<HingeJoint> NullPhysicsScene::CreateHingeJoint(const HINGE_JOINT_DESC& desc)
	{
		return bs_shared_ptr_new<NullPhysicsHingeJoint>(desc);
	}

	SPtr<SphericalJoint> NullPhysicsScene::CreateSphericalJoint(const SPHERICAL_JOINT_DESC& desc)
	{
		return bs_shared_ptr_new<NullPhysicsSphericalJoint>(desc);
	}

	SPtr<SliderJoint> NullPhysicsScene::CreateSliderJoint(const SLIDER_JOINT_DESC& desc)
	{
		return bs_shared_ptr_new<NullPhysicsSliderJoint>(desc);
	}

	SPtr<D6Joint> NullPhysicsScene::CreateD6Joint(const D6_JOINT_DESC& desc)
	{
		return bs_shared_ptr_new<NullPhysicsD6Joint>(desc);
	}

	SPtr<CharacterController> NullPhysicsScene::CreateCharacterController(const CHAR_CONTROLLER_DESC& desc)
	{
		return bs_shared_ptr_new<NullPhysicsCharacterController>(desc);
	}

	NullPhysics& gNullPhysics()
	{
		return static_cast<NullPhysics&>(NullPhysics::Instance());
	}
} // namespace bs
