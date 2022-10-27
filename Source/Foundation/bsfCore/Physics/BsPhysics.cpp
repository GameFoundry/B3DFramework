//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Physics/BsPhysics.h"
#include "Physics/BsRigidbody.h"
#include "Math/BsRay.h"
#include "Components/BsCCollider.h"

using namespace bs;

Physics::Physics(const PHYSICS_INIT_DESC& init)
{
	memset(mCollisionMap, 1, kCollisionMapSize * kCollisionMapSize * sizeof(bool));
}

void Physics::ToggleCollision(u64 groupA, u64 groupB, bool enabled)
{
	assert(groupA < kCollisionMapSize && groupB < kCollisionMapSize);

	Lock lock(mMutex);
	mCollisionMap[groupA][groupB] = enabled;
}

bool Physics::IsCollisionEnabled(u64 groupA, u64 groupB) const
{
	assert(groupA < kCollisionMapSize && groupB < kCollisionMapSize);

	enum class MyFlag
	{
		Flag1 = 1 << 0,
		Flag2 = 1 << 1,
		Flag3 = 1 << 2
	};

	Lock lock(mMutex);
	return mCollisionMap[groupA][groupB];
}

bool PhysicsScene::RayCast(const Ray& ray, PhysicsQueryHit& hit, u64 layer, float max) const
{
	return RayCast(ray.GetOrigin(), ray.GetDirection(), hit, layer, max);
}

Vector<PhysicsQueryHit> PhysicsScene::RayCastAll(const Ray& ray, u64 layer, float max) const
{
	return RayCastAll(ray.GetOrigin(), ray.GetDirection(), layer, max);
}

bool PhysicsScene::RayCastAny(const Ray& ray, u64 layer, float max) const
{
	return RayCastAny(ray.GetOrigin(), ray.GetDirection(), layer, max);
}

Vector<HCollider> rawToComponent(const Vector<Collider*>& raw)
{
	if(raw.empty())
		return Vector<HCollider>(0);

	Vector<HCollider> output;
	for(auto& entry : raw)
	{
		if(entry == nullptr)
			continue;

		CCollider* component = (CCollider*)entry->GetOwnerInternal(PhysicsOwnerType::Component);
		if(component == nullptr)
			continue;

		output.push_back(static_object_cast<CCollider>(component->GetHandle()));
	}

	return output;
}

Vector<HCollider> PhysicsScene::BoxOverlap(const AABox& box, const Quaternion& rotation, u64 layer) const
{
	return rawToComponent(BoxOverlapInternal(box, rotation, layer));
}

Vector<HCollider> PhysicsScene::SphereOverlap(const Sphere& sphere, u64 layer) const
{
	return rawToComponent(SphereOverlapInternal(sphere, layer));
}

Vector<HCollider> PhysicsScene::CapsuleOverlap(const Capsule& capsule, const Quaternion& rotation, u64 layer) const
{
	return rawToComponent(CapsuleOverlapInternal(capsule, rotation, layer));
}

Vector<HCollider> PhysicsScene::ConvexOverlap(const HPhysicsMesh& mesh, const Vector3& position, const Quaternion& rotation, u64 layer) const
{
	return rawToComponent(ConvexOverlapInternal(mesh, position, rotation, layer));
}

namespace bs
{
Physics& gPhysics()
{
	return Physics::Instance();
}
} // namespace bs
