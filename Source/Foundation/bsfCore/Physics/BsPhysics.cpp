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

	void Physics::ToggleCollision(UINT64 groupA, UINT64 groupB, bool enabled)
	{
		assert(groupA < CollisionMapSize && groupB < CollisionMapSize);

		Lock lock(mMutex);
		mCollisionMap[groupA][groupB] = enabled;
	}

	bool Physics::IsCollisionEnabled(UINT64 groupA, UINT64 groupB) const
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

	bool PhysicsScene::RayCast(const Ray& ray, PhysicsQueryHit& hit, UINT64 layer, float max) const
	{
		return RayCast(ray.GetOrigin(), ray.getDirection(), hit, layer, max);
	}

	Vector<PhysicsQueryHit> PhysicsScene::RayCastAll(const Ray& ray, UINT64 layer, float max) const
	{
		return RayCastAll(ray.GetOrigin(), ray.getDirection(), layer, max);
	}

	bool PhysicsScene::RayCastAny(const Ray& ray, UINT64 layer, float max) const
	{
		return RayCastAny(ray.GetOrigin(), ray.getDirection(), layer, max);
	}

	Vector<HCollider> RawToComponent(const Vector<Collider*>& raw)
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

			output.push_back(static_object_cast<CCollider>(component->GetHandle()));
		}

		return output;
	}

	Vector<HCollider> PhysicsScene::BoxOverlap(const AABox& box, const Quaternion& rotation, UINT64 layer) const
	{
		return RawToComponent(_boxOverlap(box, rotation, layer));
	}

	Vector<HCollider> PhysicsScene::SphereOverlap(const Sphere& sphere, UINT64 layer) const
	{
		return RawToComponent(_sphereOverlap(sphere, layer));
	}

	Vector<HCollider> PhysicsScene::CapsuleOverlap(const Capsule& capsule, const Quaternion& rotation, UINT64 layer) const
	{
		return RawToComponent(_capsuleOverlap(capsule, rotation, layer));
	}

	Vector<HCollider> PhysicsScene::convexOverlap(const HPhysicsMesh& mesh, const Vector3& position,
		const Quaternion& rotation, UINT64 layer) const
	{
		return RawToComponent(_convexOverlap(mesh, position, rotation, layer));
	}

	Physics& GPhysics()
	{
		return Physics::Instance();
	}
}
