//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsPhysics.h"
#include "BsRigidbody.h"
#include "BsRay.h"
#include "BsCCollider.h"

namespace BansheeEngine
{
	Physics::Physics(const PHYSICS_INIT_DESC& init)
	{
		memset(mCollisionMap, 1, CollisionMapSize * CollisionMapSize * sizeof(bool));
	}

	void Physics::toggleCollision(UINT64 groupA, UINT64 groupB, bool enabled)
	{
		assert(groupA < CollisionMapSize && groupB < CollisionMapSize);

		Lock lock(mMutex);
		mCollisionMap[groupA][groupB] = enabled;
	}

	bool Physics::isCollisionEnabled(UINT64 groupA, UINT64 groupB) const
	{
		assert(groupA < CollisionMapSize && groupB < CollisionMapSize);

		enum class MyFlag
		{
			Flag1 = 1 << 0,
			Flag2 = 1 << 1,
			Flag3 = 1 << 2
		};

		Lock lock(mMutex);
		return mCollisionMap[groupA][groupB];
	}

	bool Physics::rayCast(const Ray& ray, PhysicsQueryHit& hit, UINT64 layer, float max) const
	{
		return rayCast(ray.getOrigin(), ray.getDirection(), hit, layer, max);
	}

	Vector<PhysicsQueryHit> Physics::rayCastAll(const Ray& ray, UINT64 layer, float max) const
	{
		return rayCastAll(ray.getOrigin(), ray.getDirection(), layer, max);
	}

	bool Physics::rayCastAny(const Ray& ray, UINT64 layer, float max) const
	{
		return rayCastAny(ray.getOrigin(), ray.getDirection(), layer, max);
	}

	Vector<HCollider> rawToComponent(const Vector<Collider*>& raw)
	{
		if (raw.empty())
			return Vector<HCollider>(0);

		Vector<HCollider> output;
		for (auto& entry : raw)
		{
			if (entry == nullptr)
				continue;

			CCollider* component = (CCollider*)entry->_getOwner(PhysicsOwnerType::Component);
			if (component == nullptr)
				continue;

			output.push_back(component->getHandle());
		}

		return output;
	}

	Vector<HCollider> Physics::boxOverlap(const AABox& box, const Quaternion& rotation, UINT64 layer) const
	{
		return rawToComponent(_boxOverlap(box, rotation, layer));
	}

	Vector<HCollider> Physics::sphereOverlap(const Sphere& sphere, UINT64 layer) const
	{
		return rawToComponent(_sphereOverlap(sphere, layer));
	}

	Vector<HCollider> Physics::capsuleOverlap(const Capsule& capsule, const Quaternion& rotation, UINT64 layer) const
	{
		return rawToComponent(_capsuleOverlap(capsule, rotation, layer));
	}

	Vector<HCollider> Physics::convexOverlap(const HPhysicsMesh& mesh, const Vector3& position,
		const Quaternion& rotation, UINT64 layer) const
	{
		return rawToComponent(_convexOverlap(mesh, position, rotation, layer));
	}

	Physics& gPhysics()
	{
		return Physics::instance();
	}
}