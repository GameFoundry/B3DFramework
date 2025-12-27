//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
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
	SPtr<ColliderShape> colliderShape = ColliderShape::CreateMesh(mMesh);

	mShapes = { colliderShape };

	Collider::OnCreated();
}

void MeshCollider::SetMesh(const HPhysicsMesh& mesh)
{
	if(mMesh == mesh)
		return;

	if(GetIsTrigger() && mesh->GetType() == PhysicsMeshType::Triangle)
	{
		B3D_LOG(Warning, Physics, "Triangle meshes are not supported on Trigger colliders.");
		return;
	}

	mMesh = mesh;

	if(B3D_ENSURE(mShapes.Size() == 1))
		mShapes[0]->SetShape(MeshColliderShapeInformation(mesh));

	if(mParentRigidbody != nullptr)
	{
		// If triangle mesh its possible the parent can no longer use this collider (they're not supported for
		// non-kinematic rigidbodies)
		if(mMesh.IsLoaded() && mMesh->GetType() == PhysicsMeshType::Triangle)
			RefreshParentRigidbody();
		else
			mParentRigidbody->UpdateMassDistribution();
	}
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
