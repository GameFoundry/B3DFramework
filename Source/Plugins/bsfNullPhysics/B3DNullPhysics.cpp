//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DNullPhysics.h"
#include "B3DNullPhysicsMaterial.h"
#include "B3DNullPhysicsMesh.h"
#include "B3DNullPhysicsRigidbody.h"
#include "B3DNullPhysicsColliders.h"
#include "B3DNullPhysicsJoints.h"
#include "B3DNullPhysicsCharacterController.h"
#include "Threading/B3DTaskScheduler.h"
#include "Components/B3DCCollider.h"
#include "Utility/B3DTime.h"
#include "Math/B3DVector3.h"
#include "Math/B3DAABox.h"
#include "Math/B3DCapsule.h"

using namespace b3d;

NullPhysics::NullPhysics(const PHYSICS_INIT_DESC& input)
	: Physics(input), mInitDesc(input)
{}

NullPhysics::~NullPhysics()
{
	B3D_ASSERT(mScenes.empty() && "All scenes must be freed before physics system shutdown");
}

SPtr<PhysicsMaterial> NullPhysics::CreateMaterial(float staticFriction, float dynamicFriction, float restitution)
{
	return B3DMakeCoreShared<NullPhysicsMaterial>(staticFriction, dynamicFriction, restitution);
}

SPtr<PhysicsMesh> NullPhysics::CreateMesh(const SPtr<MeshData>& meshData, PhysicsMeshType type)
{
	return B3DMakeCoreShared<NullPhysicsMesh>(meshData, type);
}

SPtr<PhysicsScene> NullPhysics::CreatePhysicsScene()
{
	SPtr<NullPhysicsScene> scene = B3DMakeShared<NullPhysicsScene>(mInitDesc);
	mScenes.push_back(scene.get());

	return scene;
}

void NullPhysics::NotifySceneDestroyedInternal(NullPhysicsScene* scene)
{
	auto iterFind = std::find(mScenes.begin(), mScenes.end(), scene);
	B3D_ASSERT(iterFind != mScenes.end());

	mScenes.erase(iterFind);
}

NullPhysicsScene::NullPhysicsScene(const PHYSICS_INIT_DESC& input)
{}

NullPhysicsScene::~NullPhysicsScene()
{
	GetNullPhysics().NotifySceneDestroyedInternal(this);
}

SPtr<Rigidbody> NullPhysicsScene::CreateRigidbody(const HSceneObject& linkedSO)
{
	return B3DMakeShared<NullPhysicsRigidbody>(linkedSO);
}

SPtr<BoxCollider> NullPhysicsScene::CreateBoxCollider(const Vector3& extents, const Vector3& position, const Quaternion& rotation)
{
	return B3DMakeShared<NullPhysicsBoxCollider>(position, rotation, extents);
}

SPtr<SphereCollider> NullPhysicsScene::CreateSphereCollider(float radius, const Vector3& position, const Quaternion& rotation)
{
	return B3DMakeShared<NullPhysicsSphereCollider>(position, rotation, radius);
}

SPtr<PlaneCollider> NullPhysicsScene::CreatePlaneCollider(const Vector3& position, const Quaternion& rotation)
{
	return B3DMakeShared<NullPhysicsPlaneCollider>(position, rotation);
}

SPtr<CapsuleCollider> NullPhysicsScene::CreateCapsuleCollider(float radius, float halfHeight, const Vector3& position, const Quaternion& rotation)
{
	return B3DMakeShared<NullPhysicsCapsuleCollider>(position, rotation, radius, halfHeight);
}

SPtr<MeshCollider> NullPhysicsScene::CreateMeshCollider(const Vector3& position, const Quaternion& rotation)
{
	return B3DMakeShared<NullPhysicsMeshCollider>(position, rotation);
}

SPtr<FixedJoint> NullPhysicsScene::CreateFixedJoint(const FIXED_JOINT_DESC& desc)
{
	return B3DMakeShared<NullPhysicsFixedJoint>(desc);
}

SPtr<DistanceJoint> NullPhysicsScene::CreateDistanceJoint(const DISTANCE_JOINT_DESC& desc)
{
	return B3DMakeShared<NullPhysicsDistanceJoint>(desc);
}

SPtr<HingeJoint> NullPhysicsScene::CreateHingeJoint(const HINGE_JOINT_DESC& desc)
{
	return B3DMakeShared<NullPhysicsHingeJoint>(desc);
}

SPtr<SphericalJoint> NullPhysicsScene::CreateSphericalJoint(const SPHERICAL_JOINT_DESC& desc)
{
	return B3DMakeShared<NullPhysicsSphericalJoint>(desc);
}

SPtr<SliderJoint> NullPhysicsScene::CreateSliderJoint(const SLIDER_JOINT_DESC& desc)
{
	return B3DMakeShared<NullPhysicsSliderJoint>(desc);
}

SPtr<D6Joint> NullPhysicsScene::CreateD6Joint(const D6_JOINT_DESC& desc)
{
	return B3DMakeShared<NullPhysicsD6Joint>(desc);
}

SPtr<CharacterController> NullPhysicsScene::CreateCharacterController(const CHAR_CONTROLLER_DESC& desc)
{
	return B3DMakeShared<NullPhysicsCharacterController>(desc);
}

namespace b3d {
NullPhysics& GetNullPhysics()
{
	return static_cast<NullPhysics&>(NullPhysics::Instance());
}
} // namespace b3d
