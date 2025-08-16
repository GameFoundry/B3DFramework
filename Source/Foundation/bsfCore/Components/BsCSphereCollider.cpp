//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Components/BsCSphereCollider.h"
#include "Scene/BsSceneObject.h"
#include "Components/BsCRigidbody.h"
#include "Private/RTTI/BsCSphereColliderRTTI.h"
#include "Scene/BsSceneInstance.h"

using namespace b3d;

SphereCollider::SphereCollider(const HSceneObject& parent, float radius)
	: Collider(parent), mRadius(radius)
{
	SetName("SphereCollider");
}

SphereCollider::SphereCollider()
	: SphereCollider(nullptr)
{ }

void SphereCollider::OnCreated()
{
	SPtr<ColliderShape> colliderShape = ColliderShape::CreateSphere(mRadius);
	colliderShape->SetPosition(mShapeLocalPosition);

	mShapes = { colliderShape };

	Collider::OnCreated();
}

void SphereCollider::SetRadius(float radius)
{
	float clampedRadius = std::max(radius, 0.01f);
	if(mRadius == clampedRadius)
		return;

	mRadius = clampedRadius;

	if(B3D_ENSURE(mShapes.Size() == 1))
		mShapes[0]->SetShape(SphereColliderShapeInformation(clampedRadius));

	if(mParentDynamicRigidbody != nullptr)
		mParentDynamicRigidbody->UpdateMassDistribution();
}

void SphereCollider::SetCenter(const Vector3& center)
{
	if(mShapeLocalPosition == center)
		return;

	mShapeLocalPosition = center;

	if(B3D_ENSURE(mShapes.Size() == 1))
		mShapes[0]->SetPosition(mShapeLocalPosition);
}

RTTIType* SphereCollider::GetRttiStatic()
{
	return CSphereColliderRTTI::Instance();
}

RTTIType* SphereCollider::GetRtti() const
{
	return SphereCollider::GetRttiStatic();
}
