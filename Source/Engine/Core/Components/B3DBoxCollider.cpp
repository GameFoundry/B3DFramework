//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Components/B3DBoxCollider.h"
#include "Scene/B3DSceneObject.h"
#include "Components/B3DRigidbody.h"
#include "RTTI/B3DBoxColliderRTTI.h"
#include "Scene/B3DSceneInstance.h"

using namespace b3d;

BoxCollider::BoxCollider(const HSceneObject& parent, const Vector3& extents)
	: Collider(parent), mExtents(extents)
{
	SetName("BoxCollider");
}

BoxCollider::BoxCollider()
	: BoxCollider(nullptr)
{ }

void BoxCollider::OnCreated()
{
	TShared<ColliderShape> colliderShape = ColliderShape::CreateBox(mExtents);
	colliderShape->SetPosition(mShapeLocalPosition);

	mShapes = { colliderShape };

	Collider::OnCreated();
}

void BoxCollider::SetExtents(const Vector3& extents)
{
	Vector3 clampedExtents = Vector3::Max(extents, Vector3(0.01f, 0.01f, 0.01f));

	if(mExtents == clampedExtents)
		return;

	mExtents = clampedExtents;

	if(B3D_ENSURE(mShapes.Size() == 1))
		mShapes[0]->SetShape(BoxColliderShapeInformation(clampedExtents));

	if(mParentRigidbody != nullptr)
		mParentRigidbody->UpdateMassDistribution();
}

void BoxCollider::SetCenter(const Vector3& center)
{
	if(mShapeLocalPosition == center)
		return;

	mShapeLocalPosition = center;

	if(B3D_ENSURE(mShapes.Size() == 1))
		mShapes[0]->SetPosition(mShapeLocalPosition);
}

RTTIType* BoxCollider::GetRttiStatic()
{
	return BoxColliderRTTI::Instance();
}

RTTIType* BoxCollider::GetRtti() const
{
	return BoxCollider::GetRttiStatic();
}
