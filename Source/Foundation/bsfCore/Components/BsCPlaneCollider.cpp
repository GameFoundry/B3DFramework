//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Components/BsCPlaneCollider.h"
#include "Scene/BsSceneObject.h"
#include "Components/BsCRigidbody.h"
#include "Private/RTTI/BsCPlaneColliderRTTI.h"
#include "Scene/BsSceneInstance.h"

using namespace b3d;

CPlaneCollider::CPlaneCollider()
{
	SetName("PlaneCollider");

	mShapeLocalRotation = Quaternion::GetRotationFromTo(Vector3::kUnitX, mNormal);
}

CPlaneCollider::CPlaneCollider(const HSceneObject& parent)
	: CCollider(parent)
{
	SetName("PlaneCollider");

	mShapeLocalRotation = Quaternion::GetRotationFromTo(Vector3::kUnitX, mNormal);
}

void CPlaneCollider::SetNormal(const Vector3& normal)
{
	if(mNormal == normal)
		return;

	mNormal = normal;
	mNormal.Normalize();

	mShapeLocalRotation = Quaternion::GetRotationFromTo(Vector3::kUnitX, normal);
	mShapeLocalPosition = mNormal * mDistance;

	if(mInternal != nullptr)
	{
		TInlineArray<SPtr<ColliderShape>, 1> shapes = mInternal->GetShapes();
		if(B3D_ENSURE(shapes.Size() == 1))
		{
			shapes[0]->SetPosition(mShapeLocalPosition);
			shapes[0]->SetRotation(mShapeLocalRotation);
		}
	}
}

void CPlaneCollider::SetDistance(float distance)
{
	if(mDistance == distance)
		return;

	mDistance = distance;
	mShapeLocalPosition = mNormal * distance;

	if(mInternal != nullptr)
	{
		TInlineArray<SPtr<ColliderShape>, 1> shapes = mInternal->GetShapes();
		if(B3D_ENSURE(shapes.Size() == 1))
			shapes[0]->SetPosition(mShapeLocalPosition);
	}
}

SPtr<Collider> CPlaneCollider::CreateInternal()
{
	const SPtr<SceneInstance>& scene = SO()->GetScene();
	const Transform& transform = SO()->GetTransform();

	SPtr<ColliderShape> colliderShape = ColliderShape::CreatePlane(PlaneColliderShapeInformation());
	colliderShape->SetPosition(mShapeLocalPosition);
	colliderShape->SetRotation(mShapeLocalRotation);

	SPtr<Collider> collider = Collider::Create(*scene->GetPhysicsScene(), transform.GetPosition(), transform.GetRotation(), transform.GetScale());
	collider->SetOwner(PhysicsOwnerType::Component, this);
	collider->SetShapes(TArray{ colliderShape });

	return collider;
}

bool CPlaneCollider::IsValidParent(const HRigidbody& parent) const
{
	// Planes cannot be added to non-kinematic rigidbodies
	return parent->GetIsKinematic();
}

RTTIType* CPlaneCollider::GetRttiStatic()
{
	return CPlaneColliderRTTI::Instance();
}

RTTIType* CPlaneCollider::GetRtti() const
{
	return CPlaneCollider::GetRttiStatic();
}
