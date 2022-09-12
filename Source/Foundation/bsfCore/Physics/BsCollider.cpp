//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Physics/BsCollider.h"
#include "Physics/BsCollider.h"
#include "Physics/BsFCollider.h"
#include "Math/BsRay.h"
#include "Physics/BsPhysics.h"

namespace bs
{
	Vector3 Collider::GetPosition() const
	{
		return mInternal->getPosition();
	}

	Quaternion Collider::GetRotation() const
	{
		return mInternal->getRotation();
	}

	void Collider::SetTransform(const Vector3& pos, const Quaternion& rot)
	{
		mInternal->setTransform(pos, rot);
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
		mInternal->setIsTrigger(value);
	}

	bool Collider::GetIsTrigger() const
	{
		return mInternal->getIsTrigger();
	}

	void Collider::SetRigidbody(Rigidbody* value)
	{
		mInternal->setIsStatic(value == nullptr);

		mRigidbody = value;
	}

	void Collider::SetMass(float mass)
	{
		mInternal->setMass(mass);
	}

	float Collider::GetMass() const
	{
		return mInternal->getMass();
	}

	void Collider::SetMaterial(const HPhysicsMaterial& material)
	{
		mInternal->setMaterial(material);
	}

	HPhysicsMaterial Collider::GetMaterial() const
	{
		return mInternal->getMaterial();
	}

	void Collider::SetContactOffset(float value)
	{
		mInternal->setContactOffset(value);
	}

	float Collider::GetContactOffset()
	{
		return mInternal->getContactOffset();
	}

	void Collider::SetRestOffset(float value)
	{
		mInternal->setRestOffset(value);
	}

	float Collider::GetRestOffset()
	{
		return mInternal->getRestOffset();
	}

	void Collider::SetLayer(UINT64 layer)
	{
		mInternal->setLayer(layer);
	}

	UINT64 Collider::GetLayer() const
	{
		return mInternal->getLayer();
	}

	void Collider::SetCollisionReportMode(CollisionReportMode mode)
	{
		mInternal->setCollisionReportMode(mode);
	}

	CollisionReportMode Collider::GetCollisionReportMode() const
	{
		return mInternal->getCollisionReportMode();
	}

	bool Collider::RayCast(const Ray& ray, PhysicsQueryHit& hit, float maxDist) const
	{
		return GPhysics()._rayCast(ray.getOrigin(), ray.getDirection(), *this, hit, maxDist);
	}

	bool Collider::RayCast(const Vector3& origin, const Vector3& unitDir, PhysicsQueryHit& hit, float maxDist) const
	{
		return GPhysics()._rayCast(origin, unitDir, *this, hit, maxDist);
	}
}
