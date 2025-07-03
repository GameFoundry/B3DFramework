//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Components/BsCCapsuleCollider.h"
#include "Scene/BsSceneObject.h"
#include "Components/BsCRigidbody.h"
#include "Private/RTTI/BsCCapsuleColliderRTTI.h"
#include "Scene/BsSceneInstance.h"

using namespace b3d;

CCapsuleCollider::CCapsuleCollider()
{
	SetName("CapsuleCollider");

	mLocalRotation = Quaternion::GetRotationFromTo(Vector3::kUnitX, mNormal);
}

CCapsuleCollider::CCapsuleCollider(const HSceneObject& parent, float radius, float halfHeight)
	: CCollider(parent), mRadius(radius), mHalfHeight(halfHeight)
{
	SetName("CapsuleCollider");

	mLocalRotation = Quaternion::GetRotationFromTo(Vector3::kUnitX, mNormal);
}

void CCapsuleCollider::SetNormal(const Vector3& normal)
{
	if(mNormal == normal)
		return;

	mNormal = b3d::Vector3::Normalize(normal);
	mLocalRotation = Quaternion::GetRotationFromTo(Vector3::kUnitX, mNormal);

	if(mInternal != nullptr)
	{
		TInlineArray<SPtr<ColliderShape>, 1> shapes = mInternal->GetShapes();
		if(B3D_ENSURE(shapes.Size() == 1))
			shapes[0]->SetRotation(mLocalRotation);
		
	}
}

void CCapsuleCollider::SetCenter(const Vector3& center)
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

void CCapsuleCollider::SetHalfHeight(float halfHeight)
{
	float clampedHalfHeight = std::max(halfHeight, 0.01f);
	if(mHalfHeight == clampedHalfHeight)
		return;

	mHalfHeight = clampedHalfHeight;

	if(mInternal != nullptr)
	{
		TInlineArray<SPtr<ColliderShape>, 1> shapes = mInternal->GetShapes();
		if(B3D_ENSURE(shapes.Size() == 1))
			shapes[0]->SetShape(CapsuleColliderShapeInformation(mRadius, clampedHalfHeight));

		if(mParent != nullptr)
			mParent->UpdateMassDistribution();
	}
}

void CCapsuleCollider::SetRadius(float radius)
{
	float clampedRadius = std::max(radius, 0.01f);
	if(mRadius == clampedRadius)
		return;

	mRadius = clampedRadius;

	if(mInternal != nullptr)
	{
		TInlineArray<SPtr<ColliderShape>, 1> shapes = mInternal->GetShapes();
		if(B3D_ENSURE(shapes.Size() == 1))
			shapes[0]->SetShape(CapsuleColliderShapeInformation(clampedRadius, mHalfHeight));

		if(mParent != nullptr)
			mParent->UpdateMassDistribution();
	}
}

SPtr<Collider> CCapsuleCollider::CreateInternal()
{
	const SPtr<SceneInstance>& scene = SO()->GetScene();
	const Transform& transform = SO()->GetTransform();

	SPtr<ColliderShape> colliderShape = ColliderShape::CreateCapsule(CapsuleColliderShapeInformation(mRadius, mHalfHeight));
	colliderShape->SetPosition(mLocalPosition);
	colliderShape->SetRotation(mLocalRotation);

	SPtr<Collider> collider = Collider::Create(*scene->GetPhysicsScene(), transform.GetPosition(), transform.GetRotation(), transform.GetScale());
	collider->SetOwner(PhysicsOwnerType::Component, this);
	collider->SetShapes(TArray{ colliderShape });

	return collider;
}

RTTIType* CCapsuleCollider::GetRttiStatic()
{
	return CCapsuleColliderRTTI::Instance();
}

RTTIType* CCapsuleCollider::GetRtti() const
{
	return CCapsuleCollider::GetRttiStatic();
}
