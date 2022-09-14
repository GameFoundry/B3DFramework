//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Physics/BsCollider.h"
#include "Physics/BsCollider.h"
#include "Physics/BsFCollider.h"
#include "Math/BsRay.h"
#include "Physics/BsPhysics.h"

namespace bs
{
	Vector3 Collider::getPosition() const
	{
		return mInternal->getPosition();
	}

	Quaternion Collider::getRotation() const
	{
		return mInternal->getRotation();
	}

	void Collider::setTransform(const Vector3& pos, const Quaternion& rot)
	{
		mInternal->setTransform(pos, rot);
	}

	void Collider::setScale(const Vector3& scale)
	{
		mScale = scale;
	}

	Vector3 Collider::getScale() const
	{
		return mScale;
	}

	void Collider::setIsTrigger(bool value)
	{
		mInternal->setIsTrigger(value);
	}

	bool Collider::getIsTrigger() const
	{
		return mInternal->getIsTrigger();
	}

	void Collider::setRigidbody(Rigidbody* value)
	{
		mInternal->setIsStatic(value == nullptr);

		mRigidbody = value;
	}

	void Collider::setMass(float mass)
	{
		mInternal->setMass(mass);
	}

	float Collider::getMass() const
	{
		return mInternal->getMass();
	}

	void Collider::setMaterial(const HPhysicsMaterial& material)
	{
		mInternal->setMaterial(material);
	}

	HPhysicsMaterial Collider::getMaterial() const
	{
		return mInternal->getMaterial();
	}

	void Collider::setContactOffset(float value)
	{
		mInternal->setContactOffset(value);
	}

	float Collider::getContactOffset()
	{
		return mInternal->getContactOffset();
	}

	void Collider::setRestOffset(float value)
	{
		mInternal->setRestOffset(value);
	}

	float Collider::getRestOffset()
	{
		return mInternal->getRestOffset();
	}

	void Collider::setLayer(UINT64 layer)
	{
		mInternal->setLayer(layer);
	}

	UINT64 Collider::getLayer() const
	{
		return mInternal->getLayer();
	}

	void Collider::setCollisionReportMode(CollisionReportMode mode)
	{
		mInternal->setCollisionReportMode(mode);
	}

	CollisionReportMode Collider::getCollisionReportMode() const
	{
		return mInternal->getCollisionReportMode();
	}

	bool Collider::rayCast(const Ray& ray, PhysicsQueryHit& hit, float maxDist) const
	{
		return gPhysics().RayCastInternal(ray.getOrigin(), ray.getDirection(), *this, hit, maxDist);
	}

	bool Collider::rayCast(const Vector3& origin, const Vector3& unitDir, PhysicsQueryHit& hit, float maxDist) const
	{
		return gPhysics().RayCastInternal(origin, unitDir, *this, hit, maxDist);
	}
}
