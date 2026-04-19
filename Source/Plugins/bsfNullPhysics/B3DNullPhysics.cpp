//************************************ B3D Framework - Copyright 2026 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DNullPhysics.h"
#include "B3DNullPhysicsMaterial.h"
#include "B3DNullPhysicsMesh.h"
#include "B3DNullPhysicsRigidbody.h"
#include "B3DNullPhysicsColliders.h"
#include "B3DNullPhysicsJoints.h"
#include "B3DNullPhysicsCharacterController.h"
#include "Physics/B3DColliderShape.h"

using namespace b3d;

NullPhysics::NullPhysics(const PhysicsCreateInformation& createInformation)
	: Physics(createInformation), mInitDesc(createInformation)
{}

NullPhysics::~NullPhysics()
{
	B3D_ASSERT(mScenes.empty() && "All scenes must be freed before physics system shutdown");
}

SPtr<PhysicsMaterial> NullPhysics::CreateMaterial(float staticFriction, float dynamicFriction, float restitution)
{
	return B3DMakeShared<NullPhysicsMaterial>(staticFriction, dynamicFriction, restitution);
}

UPtr<IPhysicsMeshImplementation> NullPhysics::CreateMesh(const SPtr<MeshData>& meshData, PhysicsMeshType type)
{
	return B3DMakeUnique<NullPhysicsMeshImplementation>(meshData, type);
}

SPtr<PhysicsScene> NullPhysics::CreatePhysicsScene()
{
	SPtr<NullPhysicsScene> scene = B3DMakeShared<NullPhysicsScene>(mInitDesc);
	mScenes.push_back(scene.get());

	return scene;
}

SPtr<ColliderShape> NullPhysics::CreateColliderShape()
{
	return B3DMakeShared<NullPhysicsColliderShape>();
}

UPtr<IColliderImplementation> NullPhysics::CreateColliderImplementation()
{
	return B3DMakeUnique<NullPhysicsCollider>();
}

UPtr<IRigidbodyImplementation> NullPhysics::CreateRigidbodyImplementation(Rigidbody& owner)
{
	return B3DMakeUnique<NullPhysicsRigidbody>(owner);
}

void NullPhysics::NotifySceneDestroyedInternal(NullPhysicsScene* scene)
{
	auto iterFind = std::find(mScenes.begin(), mScenes.end(), scene);
	B3D_ASSERT(iterFind != mScenes.end());

	mScenes.erase(iterFind);
}

NullPhysicsScene::NullPhysicsScene(const PhysicsCreateInformation& createInformation)
{}

NullPhysicsScene::~NullPhysicsScene()
{
	GetNullPhysics().NotifySceneDestroyedInternal(this);
}

UPtr<IFixedJointImplementation> NullPhysicsScene::CreateFixedJoint(Joint& owner, const FixedJointCreateInformation& createInformation)
{
	return B3DMakeUnique<NullPhysicsFixedJoint>();
}

UPtr<IDistanceJointImplementation> NullPhysicsScene::CreateDistanceJoint(Joint& owner, const DistanceJointCreateInformation& createInformation)
{
	return B3DMakeUnique<NullPhysicsDistanceJoint>();
}

UPtr<IHingeJointImplementation> NullPhysicsScene::CreateHingeJoint(Joint& owner, const HingeJointCreateInformation& createInformation)
{
	return B3DMakeUnique<NullPhysicsHingeJoint>();
}

UPtr<ISphericalJointImplementation> NullPhysicsScene::CreateSphericalJoint(Joint& owner, const SphericalJointCreateInformation& createInformation)
{
	return B3DMakeUnique<NullPhysicsSphericalJoint>();
}

UPtr<ISliderJointImplementation> NullPhysicsScene::CreateSliderJoint(Joint& owner, const SliderJointCreateInformation& createInformation)
{
	return B3DMakeUnique<NullPhysicsSliderJoint>();
}

UPtr<ID6JointImplementation> NullPhysicsScene::CreateD6Joint(Joint& owner, const D6JointCreateInformation& createInformation)
{
	return B3DMakeUnique<NullPhysicsD6Joint>();
}

UPtr<ICharacterControllerImplementation> NullPhysicsScene::CreateCharacterController(CharacterController& owner, const CharacterControllerCreateInformation& createInformation)
{
	return B3DMakeUnique<NullPhysicsCharacterController>(owner, createInformation);
}

namespace b3d {
NullPhysics& GetNullPhysics()
{
	return static_cast<NullPhysics&>(NullPhysics::Instance());
}
} // namespace b3d
