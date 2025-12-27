//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Components/B3DCapsuleCollider.h"
#include "Scene/B3DSceneObject.h"
#include "Components/B3DRigidbody.h"
#include "RTTI/B3DCapsuleColliderRTTI.h"
#include "Scene/B3DSceneInstance.h"

using namespace b3d;

CapsuleCollider::CapsuleCollider(const HSceneObject& parent, float radius, float halfHeight)
	: Collider(parent), mRadius(radius), mHalfHeight(halfHeight)
{
	SetName("CapsuleCollider");

	mShapeLocalRotation = Quaternion::GetRotationFromTo(Vector3::kUnitX, mNormal);
}

CapsuleCollider::CapsuleCollider()
	: CapsuleCollider(nullptr)
{ }

void CapsuleCollider::OnCreated()
{
	SPtr<ColliderShape> colliderShape = ColliderShape::CreateCapsule(CapsuleColliderShapeInformation(mRadius, mHalfHeight));
	colliderShape->SetPosition(mShapeLocalPosition);
	colliderShape->SetRotation(mShapeLocalRotation);

	mShapes = { colliderShape };

	Collider::OnCreated();
}

void CapsuleCollider::SetNormal(const Vector3& normal)
{
	if(mNormal == normal)
		return;

	mNormal = b3d::Vector3::Normalize(normal);
	mShapeLocalRotation = Quaternion::GetRotationFromTo(Vector3::kUnitX, mNormal);

	if(B3D_ENSURE(mShapes.Size() == 1))
		mShapes[0]->SetRotation(mShapeLocalRotation);
		
}

void CapsuleCollider::SetCenter(const Vector3& center)
{
	if(mShapeLocalPosition == center)
		return;

	mShapeLocalPosition = center;

	if(B3D_ENSURE(mShapes.Size() == 1))
		mShapes[0]->SetPosition(mShapeLocalPosition);
}

void CapsuleCollider::SetHalfHeight(float halfHeight)
{
	float clampedHalfHeight = std::max(halfHeight, 0.01f);
	if(mHalfHeight == clampedHalfHeight)
		return;

	mHalfHeight = clampedHalfHeight;

	if(B3D_ENSURE(mShapes.Size() == 1))
		mShapes[0]->SetShape(CapsuleColliderShapeInformation(mRadius, clampedHalfHeight));

	if(mParentRigidbody != nullptr)
		mParentRigidbody->UpdateMassDistribution();
}

void CapsuleCollider::SetRadius(float radius)
{
	float clampedRadius = std::max(radius, 0.01f);
	if(mRadius == clampedRadius)
		return;

	mRadius = clampedRadius;

	if(B3D_ENSURE(mShapes.Size() == 1))
		mShapes[0]->SetShape(CapsuleColliderShapeInformation(clampedRadius, mHalfHeight));

	if(mParentRigidbody != nullptr)
		mParentRigidbody->UpdateMassDistribution();
}

RTTIType* CapsuleCollider::GetRttiStatic()
{
	return CapsuleColliderRTTI::Instance();
}

RTTIType* CapsuleCollider::GetRtti() const
{
	return CapsuleCollider::GetRttiStatic();
}
