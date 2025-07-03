//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Components/BsCSphereCollider.h"
#include "Scene/BsSceneObject.h"
#include "Components/BsCRigidbody.h"
#include "Private/RTTI/BsCSphereColliderRTTI.h"
#include "Scene/BsSceneInstance.h"

using namespace b3d;

CSphereCollider::CSphereCollider()
{
	SetName("SphereCollider");
}

CSphereCollider::CSphereCollider(const HSceneObject& parent, float radius)
	: CCollider(parent), mRadius(radius)
{
	SetName("SphereCollider");
}

void CSphereCollider::SetRadius(float radius)
{
	float clampedRadius = std::max(radius, 0.01f);
	if(mRadius == clampedRadius)
		return;

	mRadius = clampedRadius;

	if(mInternal != nullptr)
	{
		TInlineArray<SPtr<ColliderShape>, 1> shapes = mInternal->GetShapes();
		if(B3D_ENSURE(shapes.Size() == 1))
			shapes[0]->SetShape(SphereColliderShapeInformation(clampedRadius));

		if(mParent != nullptr)
			mParent->UpdateMassDistribution();
	}
}

void CSphereCollider::SetCenter(const Vector3& center)
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

SPtr<Collider> CSphereCollider::CreateInternal()
{
	const SPtr<SceneInstance>& scene = SO()->GetScene();
	const Transform& transform = SO()->GetTransform();

	SPtr<ColliderShape> colliderShape = ColliderShape::CreateSphere(mRadius);
	colliderShape->SetPosition(mLocalPosition);
	colliderShape->SetRotation(mLocalRotation);

	SPtr<Collider> collider = Collider::Create(*scene->GetPhysicsScene(), transform.GetPosition(), transform.GetRotation(), transform.GetScale());
	collider->SetOwner(PhysicsOwnerType::Component, this);
	collider->SetShapes(TArray{ colliderShape });

	return collider;
}

RTTIType* CSphereCollider::GetRttiStatic()
{
	return CSphereColliderRTTI::Instance();
}

RTTIType* CSphereCollider::GetRtti() const
{
	return CSphereCollider::GetRttiStatic();
}
