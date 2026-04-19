//************************************ B3D Framework - Copyright 2026 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DPhysXCollider.h"
#include "B3DPhysX.h"
#include "B3DPhysXColliderShape.h"
#include "B3DPhysXRigidbody.h"
#include "B3DPhysXMaterial.h"
#include "PxScene.h"

using namespace physx;
using namespace b3d;

PhysXCollider::PhysXCollider()
{
	mPxRigidStatic = GetPhysX().GetPhysX()->createRigidStatic(PxTransform(PxIdentity));
}

PhysXCollider::~PhysXCollider()
{
	if(mPxScene != nullptr)
		PhysXCollider::RemoveFromScene();

	mPxRigidStatic->release();
}

void PhysXCollider::AddToScene(PhysicsScene& scene)
{
	if(!B3D_ENSURE(mPxScene == nullptr))
		return;

	const PhysXScene& physxScene = static_cast<PhysXScene&>(scene);
	mPxScene = &physxScene.GetPxScene();

	mPxScene->addActor(*mPxRigidStatic);
}

void PhysXCollider::RemoveFromScene()
{
	if(!B3D_ENSURE(mPxScene != nullptr))
		return;

	mPxScene->removeActor(*mPxRigidStatic);
	mPxScene = nullptr;
}

void PhysXCollider::AttachShape(const SPtr<ColliderShape>& shape)
{
	if(!B3D_ENSURE(shape != nullptr))
		return;

	const PhysXColliderShape& physxShape = static_cast<const PhysXColliderShape&>(*shape);
	mPxRigidStatic->attachShape(*physxShape.GetPxShape());
}

void PhysXCollider::DetachShape(const SPtr<ColliderShape>& shape)
{
	if(!B3D_ENSURE(shape != nullptr))
		return;

	const PhysXColliderShape& physxShape = static_cast<const PhysXColliderShape&>(*shape);
	mPxRigidStatic->detachShape(*physxShape.GetPxShape());
}

void PhysXCollider::SetTransform(const Vector3& position, const Quaternion& rotation)
{
	mPxRigidStatic->setGlobalPose(ToPxTransform(position, rotation));
}
