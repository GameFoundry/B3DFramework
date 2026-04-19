//************************************ B3D Framework - Copyright 2026 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Components/B3DPlaneCollider.h"
#include "Scene/B3DSceneObject.h"
#include "Components/B3DRigidbody.h"
#include "RTTI/B3DPlaneColliderRTTI.h"
#include "Scene/B3DSceneInstance.h"

using namespace b3d;

PlaneCollider::PlaneCollider(const HSceneObject& parent)
	: Collider(parent)
{
	SetName("PlaneCollider");

	mShapeLocalRotation = Quaternion::GetRotationFromTo(Vector3::kUnitX, mNormal);
}

PlaneCollider::PlaneCollider()
	: PlaneCollider(nullptr)
{ }

void PlaneCollider::OnCreated()
{
	SPtr<ColliderShape> colliderShape = ColliderShape::CreatePlane(PlaneColliderShapeInformation());
	colliderShape->SetPosition(mShapeLocalPosition);
	colliderShape->SetRotation(mShapeLocalRotation);

	mShapes = { colliderShape };

	Collider::OnCreated();
}

void PlaneCollider::SetNormal(const Vector3& normal)
{
	if(mNormal == normal)
		return;

	mNormal = normal;
	mNormal.Normalize();

	mShapeLocalRotation = Quaternion::GetRotationFromTo(Vector3::kUnitX, normal);
	mShapeLocalPosition = mNormal * mDistance;

	if(B3D_ENSURE(mShapes.Size() == 1))
	{
		mShapes[0]->SetPosition(mShapeLocalPosition);
		mShapes[0]->SetRotation(mShapeLocalRotation);
	}
}

void PlaneCollider::SetDistance(float distance)
{
	if(mDistance == distance)
		return;

	mDistance = distance;
	mShapeLocalPosition = mNormal * distance;

	if(B3D_ENSURE(mShapes.Size() == 1))
		mShapes[0]->SetPosition(mShapeLocalPosition);
}

bool PlaneCollider::IsValidParent(const HRigidbody& parent) const
{
	// Planes cannot be added to non-kinematic rigidbodies
	return parent->GetIsKinematic();
}

RTTIType* PlaneCollider::GetRttiStatic()
{
	return PlaneColliderRTTI::Instance();
}

RTTIType* PlaneCollider::GetRtti() const
{
	return PlaneCollider::GetRttiStatic();
}
