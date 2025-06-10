//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Physics/BsCollider.h"
#include "Physics/BsFCollider.h"
#include "Math/BsRay.h"
#include "Physics/BsPhysics.h"

using namespace bs;

Collider::Collider(PhysicsScene& physicsScene, const Vector3& position, const Quaternion& rotation, const Vector3& scale)
	:mPosition(position), mRotation(rotation), mScale(scale), mPhysicsScene(physicsScene)
{ }

void Collider::SetShapes(const TArrayView<SPtr<ColliderShape>>& shapes)
{
	u32 newShapeIndexInParent = 0;
	for (const auto& newShape : shapes)
	{
		if (newShape == nullptr)
		{
			newShapeIndexInParent++;
			continue;
		}

		bool isFound = false;
		for (auto& existingShape : mShapes)
		{
			if (existingShape == newShape)
			{
				existingShape->SetShapeIndexInParent(newShapeIndexInParent);
				existingShape = nullptr;
				isFound = true;
				break;
			}
		}

		if (!isFound)
		{
			newShape->AttachToCollider(*this);
			newShape->SetShapeIndexInParent(newShapeIndexInParent);
			newShape->SetIsTrigger(mIsTrigger);
			newShape->SetScale(mScale);
		}
	}

	// Detach all shapes we didn't clear above
	for (const auto& existingShape : mShapes)
	{
		if (existingShape == nullptr)
			continue;

		existingShape->DetachFromCollider();
		existingShape->SetShapeIndexInParent(~0u);
	}

	mShapes.Clear();
	mShapes.Insert(mShapes.Begin(),shapes.Begin(), shapes.End());
}

void Collider::SetTransform(const Vector3& position, const Quaternion& rotation)
{
	if(mPosition == position && mRotation == rotation)
		return;

	mPosition = position;
	mRotation = rotation;

	UpdateShapeTransforms();
}

void Collider::SetScale(const Vector3& scale)
{
	mScale = scale;

	UpdateShapeTransforms();
}

void Collider::SetIsTrigger(bool value)
{
	if(mIsTrigger == value)
		return;

	for(auto& shape : mShapes)
		shape->SetIsTrigger(value);

	mIsTrigger = value;
}

void Collider::SetRigidbody(Rigidbody* value)
{
	for(auto& entry : mShapes)
		entry->DetachFromCollider();

	mRigidbody = value;

	for(auto& entry : mShapes)
		entry->AttachToCollider(*this);
}

bool Collider::RayCast(const Ray& ray, PhysicsQueryHit& hit, float maxDist) const
{
	return GetPhysics().RayCast(ray.Origin, ray.Direction, *this, hit, maxDist);
}

bool Collider::RayCast(const Vector3& origin, const Vector3& unitDir, PhysicsQueryHit& hit, float maxDist) const
{
	return GetPhysics().RayCast(origin, unitDir, *this, hit, maxDist);
}

void Collider::UpdateShapeTransforms()
{
	for(auto& entry : mShapes)
		entry->UpdateTransform();
}

SPtr<Collider> Collider::Create(PhysicsScene& scene, const Vector3& position, const Quaternion& rotation, const Vector3& scale)
{
	return scene.CreateCollider(position, rotation, scale);
}

