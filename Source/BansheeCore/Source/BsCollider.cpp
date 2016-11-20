//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsCollider.h"
#include "BsCollider.h"
#include "BsFCollider.h"
#include "BsRay.h"
#include "BsPhysics.h"

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
		return gPhysics()._rayCast(ray.getOrigin(), ray.getDirection(), *this, hit, maxDist);
	}

	bool Collider::rayCast(const Vector3& origin, const Vector3& unitDir, PhysicsQueryHit& hit, float maxDist) const
	{
		return gPhysics()._rayCast(origin, unitDir, *this, hit, maxDist);
	}
}