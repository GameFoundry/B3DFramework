//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Components/BsCBoxCollider.h"
#include "Scene/BsSceneObject.h"
#include "Components/BsCRigidbody.h"
#include "Private/RTTI/BsCBoxColliderRTTI.h"
#include "Scene/BsSceneInstance.h"

using namespace b3d;

CBoxCollider::CBoxCollider()
{
	SetName("BoxCollider");
}

CBoxCollider::CBoxCollider(const HSceneObject& parent, const Vector3& extents)
	: CCollider(parent), mExtents(extents)
{
	SetName("BoxCollider");
}

void CBoxCollider::SetExtents(const Vector3& extents)
{
	Vector3 clampedExtents = Vector3::Max(extents, Vector3(0.01f, 0.01f, 0.01f));

	if(mExtents == clampedExtents)
		return;

	mExtents = clampedExtents;

	if(mInternal != nullptr)
	{
		TInlineArray<SPtr<ColliderShape>, 1> shapes = mInternal->GetShapes();
		if(B3D_ENSURE(shapes.Size() == 1))
			shapes[0]->SetShape(BoxColliderShapeInformation(clampedExtents));

		if(mParent != nullptr)
			mParent->UpdateMassDistribution();
	}
}

void CBoxCollider::SetCenter(const Vector3& center)
{
	if(mLocalPosition == center)
		return;

	mLocalPosition = center;

	if(mInternal != nullptr)
	{
		TInlineArray<SPtr<ColliderShape>, 1> shapes = mInternal->GetShapes();
		if(B3D_ENSURE(shapes.Size() == 1))
			shapes[0]->SetPosition(mLocalPosition);
	}
}

SPtr<Collider> CBoxCollider::CreateInternal()
{
	const SPtr<SceneInstance>& scene = SO()->GetScene();
	const Transform& transform = SO()->GetTransform();

	SPtr<ColliderShape> colliderShape = ColliderShape::CreateBox(mExtents);
	colliderShape->SetPosition(mLocalPosition);
	colliderShape->SetRotation(mLocalRotation);

	SPtr<Collider> collider = Collider::Create(*scene->GetPhysicsScene(), transform.GetPosition(), transform.GetRotation(), transform.GetScale());
	collider->SetOwner(PhysicsOwnerType::Component, this);
	collider->SetShapes(TArray{ colliderShape });

	return collider;
}

RTTIType* CBoxCollider::GetRttiStatic()
{
	return CBoxColliderRTTI::Instance();
}

RTTIType* CBoxCollider::GetRtti() const
{
	return CBoxCollider::GetRttiStatic();
}
