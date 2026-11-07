//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Components/B3DMeshCollider.h"
#include "Scene/B3DSceneObject.h"
#include "Components/B3DRigidbody.h"
#include "Physics/B3DPhysicsMesh.h"
#include "RTTI/B3DMeshColliderRTTI.h"
#include "Scene/B3DSceneInstance.h"

using namespace b3d;

MeshCollider::MeshCollider(const HSceneObject& parent)
	: Collider(parent)
{
	SetName("MeshCollider");
}

MeshCollider::MeshCollider()
	: MeshCollider(nullptr)
{ }

void MeshCollider::OnCreated()
{
	TShared<ColliderShape> colliderShape = ColliderShape::CreateMesh(mMesh);

	mShapes = { colliderShape };

	Collider::OnCreated();
}

void MeshCollider::SetMesh(const HPhysicsMesh& mesh)
{
	if(mMesh == mesh)
		return;

	const bool isTriangleMesh = mesh.IsValid() && mesh->GetType() == PhysicsMeshType::Triangle;
	if(GetIsTrigger() && isTriangleMesh)
	{
		B3D_LOG(Warning, LogPhysics, "Triangle meshes are not supported on Trigger colliders.");
		return;
	}

	mMesh = mesh;

	// Triangle meshes aren't supported on non-kinematic rigidbodies. The parent must be re-evaluated while the shape holds
	// geometry that the parent can accept: before switching to a triangle mesh, and after switching away from one.
	if(isTriangleMesh)
		RefreshParentRigidbody();

	if(B3D_ENSURE(mShapes.Size() == 1))
		mShapes[0]->SetShape(MeshColliderShapeInformation(mesh));

	if(!isTriangleMesh)
		RefreshParentRigidbody();

	if(mParentRigidbody != nullptr)
		mParentRigidbody->UpdateMassDistribution();
}

bool MeshCollider::IsValidParent(const HRigidbody& parent) const
{
	// Triangle mesh colliders cannot be used for non-kinematic rigidbodies
	return !mMesh.IsLoaded() || mMesh->GetType() == PhysicsMeshType::Convex || parent->GetIsKinematic();
}

RTTIType* MeshCollider::GetRttiStatic()
{
	return MeshColliderRTTI::Instance();
}

RTTIType* MeshCollider::GetRtti() const
{
	return MeshCollider::GetRttiStatic();
}
