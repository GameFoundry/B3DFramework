//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Physics/BsCollider.h"
#include "Physics/BsCollider.h"
#include "Physics/BsFCollider.h"
#include "Math/BsRay.h"
#include "Physics/BsPhysics.h"

using namespace bs;

Vector3 Collider::GetPosition() const
{
	return mInternal->GetPosition();
}

Quaternion Collider::GetRotation() const
{
	return mInternal->GetRotation();
}

void Collider::SetTransform(const Vector3& pos, const Quaternion& rot)
{
	mInternal->SetTransform(pos, rot);
}

void Collider::SetScale(const Vector3& scale)
{
	mScale = scale;
}

Vector3 Collider::GetScale() const
{
	return mScale;
}

void Collider::SetIsTrigger(bool value)
{
	mInternal->SetIsTrigger(value);
}

bool Collider::GetIsTrigger() const
{
	return mInternal->GetIsTrigger();
}

void Collider::SetRigidbody(Rigidbody* value)
{
	mInternal->SetIsStatic(value == nullptr);

	mRigidbody = value;
}

void Collider::SetMass(float mass)
{
	mInternal->SetMass(mass);
}

float Collider::GetMass() const
{
	return mInternal->GetMass();
}

void Collider::SetMaterial(const HPhysicsMaterial& material)
{
	mInternal->SetMaterial(material);
}

HPhysicsMaterial Collider::GetMaterial() const
{
	return mInternal->GetMaterial();
}

void Collider::SetContactOffset(float value)
{
	mInternal->SetContactOffset(value);
}

float Collider::GetContactOffset()
{
	return mInternal->GetContactOffset();
}

void Collider::SetRestOffset(float value)
{
	mInternal->SetRestOffset(value);
}

float Collider::GetRestOffset()
{
	return mInternal->GetRestOffset();
}

void Collider::SetLayer(u64 layer)
{
	mInternal->SetLayer(layer);
}

u64 Collider::GetLayer() const
{
	return mInternal->GetLayer();
}

void Collider::SetCollisionReportMode(CollisionReportMode mode)
{
	mInternal->SetCollisionReportMode(mode);
}

CollisionReportMode Collider::GetCollisionReportMode() const
{
	return mInternal->GetCollisionReportMode();
}

bool Collider::RayCast(const Ray& ray, PhysicsQueryHit& hit, float maxDist) const
{
	return GetPhysics().RayCastInternal(ray.GetOrigin(), ray.GetDirection(), *this, hit, maxDist);
}

bool Collider::RayCast(const Vector3& origin, const Vector3& unitDir, PhysicsQueryHit& hit, float maxDist) const
{
	return GetPhysics().RayCastInternal(origin, unitDir, *this, hit, maxDist);
}
