//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Components/BsCMeshCollider.h"
#include "Scene/BsSceneObject.h"
#include "Components/BsCRigidbody.h"
#include "Physics/BsPhysicsMesh.h"
#include "Private/RTTI/BsCMeshColliderRTTI.h"
#include "Scene/BsSceneInstance.h"

using namespace b3d;

CMeshCollider::CMeshCollider()
{
	SetName("MeshCollider");
}

CMeshCollider::CMeshCollider(const HSceneObject& parent)
	: CCollider(parent)
{
	SetName("MeshCollider");
}

void CMeshCollider::SetMesh(const HPhysicsMesh& mesh)
{
	if(mMesh == mesh)
		return;

	if(GetIsTrigger() && mesh->GetType() == PhysicsMeshType::Triangle)
	{
		B3D_LOG(Warning, Physics, "Triangle meshes are not supported on Trigger colliders.");
		return;
	}

	mMesh = mesh;

	if(mInternal != nullptr)
	{
		TInlineArray<SPtr<ColliderShape>, 1> shapes = mInternal->GetShapes();
		if(B3D_ENSURE(shapes.Size() == 1))
			shapes[0]->SetShape(MeshColliderShapeInformation(mesh));

		if(mRigidbody != nullptr)
		{
			// If triangle mesh its possible the parent can no longer use this collider (they're not supported for
			// non-kinematic rigidbodies)
			if(mMesh.IsLoaded() && mMesh->GetType() == PhysicsMeshType::Triangle)
				UpdateParentRigidbody();
			else
				mRigidbody->UpdateMassDistribution();
		}
	}
}

SPtr<Collider> CMeshCollider::CreateInternal()
{
	const SPtr<SceneInstance>& scene = SO()->GetScene();
	const Transform& transform = SO()->GetTransform();

	SPtr<ColliderShape> colliderShape = ColliderShape::CreateMesh(mMesh);
	colliderShape->SetPosition(mShapeLocalPosition);
	colliderShape->SetRotation(mShapeLocalRotation);

	SPtr<Collider> collider = Collider::Create(*scene->GetPhysicsScene(), transform.GetPosition(), transform.GetRotation(), transform.GetScale());
	collider->SetOwner(PhysicsOwnerType::Component, this);
	collider->SetShapes(TArray{ colliderShape });

	return collider;
}

bool CMeshCollider::IsValidParent(const HRigidbody& parent) const
{
	// Triangle mesh colliders cannot be used for non-kinematic rigidbodies
	return !mMesh.IsLoaded() || mMesh->GetType() == PhysicsMeshType::Convex || parent->GetIsKinematic();
}

RTTIType* CMeshCollider::GetRttiStatic()
{
	return CMeshColliderRTTI::Instance();
}

RTTIType* CMeshCollider::GetRtti() const
{
	return CMeshCollider::GetRttiStatic();
}
