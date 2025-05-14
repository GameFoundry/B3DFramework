//************************************ bs::framework - Copyright 2025 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsECSTestSuite.h"
#include "ECS/BsEntitySparseSet.h"

using namespace bs;
using namespace bs::ecs;

void ECSTestSuite::StartUp() { }
void ECSTestSuite::ShutDown() { }

ECSTestSuite::ECSTestSuite()
{
	B3D_ADD_TEST(ECSTestSuite::TestEntitySparseSet)
}

void ECSTestSuite::TestEntitySparseSet()
{
	const TArray<Entity> kEntities = {
		Entity(0, 0),
		Entity(1, 0),
		Entity(2, 0),
		Entity(3, 0),
		Entity(5000, 0),
		Entity(50000, 0),
		Entity(50001, 0) };

	auto fnTestSparseSet = [this, kEntities](auto&& entitySparseSet)
	{
		for(const auto& entity : kEntities)
			entitySparseSet.Add(entity);

		for(const auto& entity : kEntities)
		{
			B3D_TEST_ASSERT(entitySparseSet.Contains(entity))
		}

		u32 foundEntityCount = 0;
		for(const auto entity : entitySparseSet)
		{
			auto found = std::find(kEntities.begin(), kEntities.end(), entity);
			B3D_TEST_ASSERT(found != kEntities.end())
			if(found != kEntities.end())
				foundEntityCount++;
		}

		B3D_TEST_ASSERT(foundEntityCount == (u32)kEntities.Size())

		auto foundEntry3 = entitySparseSet.Find(kEntities[3]);
		B3D_TEST_ASSERT(foundEntry3 != entitySparseSet.End())

		if(foundEntry3 != entitySparseSet.End())
		{
			entitySparseSet.Erase(*foundEntry3);
			B3D_TEST_ASSERT(entitySparseSet.Find(kEntities[3]) == entitySparseSet.End())
		}

		auto foundEntry6 = entitySparseSet.Find(kEntities[6]);
		B3D_TEST_ASSERT(foundEntry6 != entitySparseSet.End())

		if(foundEntry6 != entitySparseSet.End())
		{
			entitySparseSet.Erase(*foundEntry6);
			B3D_TEST_ASSERT(entitySparseSet.Find(kEntities[6]) == entitySparseSet.End())
		}

		foundEntityCount = 0;
		for(const auto entity : entitySparseSet)
		{
			auto found = std::find(kEntities.begin(), kEntities.end(), entity);
			if(found != kEntities.end())
				foundEntityCount++;
		}

		B3D_TEST_ASSERT(foundEntityCount == (u32)(kEntities.Size() - 2u))

		entitySparseSet.ClearInvalid();

		const u32 expectedEntityCount = entitySparseSet.GetDeletePolicy() == SparseSetDeletePolicy::SwapOnly
			? (u32)kEntities.Size()
			: (u32)(kEntities.Size() - 2u);
		B3D_TEST_ASSERT(entitySparseSet.Size() == expectedEntityCount)
	};

	TSparseSet<SparseSetDeletePolicy::SwapAndErase> swapAndEraseSparseSet;
	fnTestSparseSet(swapAndEraseSparseSet);

	TSparseSet<SparseSetDeletePolicy::SwapOnly> swapOnlySparseSet;
	fnTestSparseSet(swapOnlySparseSet);

	TSparseSet<SparseSetDeletePolicy::InPlace> inPlaceDeleteSparseSet;
	fnTestSparseSet(inPlaceDeleteSparseSet);

}
