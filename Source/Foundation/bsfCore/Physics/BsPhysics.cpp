//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Physics/BsPhysics.h"
#include "Physics/BsRigidbody.h"
#include "Math/BsRay.h"
#include "Components/BsCCollider.h"

namespace bs
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

	bool PhysicsScene::rayCast(const Ray& ray, PhysicsQueryHit& hit, UINT64 layer, float max) const
	{
		return rayCast(ray.getOrigin(), ray.getDirection(), hit, layer, max);
	}

	Vector<PhysicsQueryHit> PhysicsScene::rayCastAll(const Ray& ray, UINT64 layer, float max) const
	{
		return rayCastAll(ray.getOrigin(), ray.getDirection(), layer, max);
	}

	bool PhysicsScene::rayCastAny(const Ray& ray, UINT64 layer, float max) const
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

			output.push_back(static_object_cast<CCollider>(component->getHandle()));
		}

		return output;
	}

	Vector<HCollider> PhysicsScene::boxOverlap(const AABox& box, const Quaternion& rotation, UINT64 layer) const
	{
		return rawToComponent(_boxOverlap(box, rotation, layer));
	}

	Vector<HCollider> PhysicsScene::sphereOverlap(const Sphere& sphere, UINT64 layer) const
	{
		return rawToComponent(_sphereOverlap(sphere, layer));
	}

	Vector<HCollider> PhysicsScene::capsuleOverlap(const Capsule& capsule, const Quaternion& rotation, UINT64 layer) const
	{
		return rawToComponent(_capsuleOverlap(capsule, rotation, layer));
	}

	Vector<HCollider> PhysicsScene::convexOverlap(const HPhysicsMesh& mesh, const Vector3& position,
		const Quaternion& rotation, UINT64 layer) const
	{
		return rawToComponent(_convexOverlap(mesh, position, rotation, layer));
	}

	Physics& gPhysics()
	{
		return Physics::instance();
	}
}