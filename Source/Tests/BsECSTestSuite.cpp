//************************************ bs::framework - Copyright 2025 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsECSTestSuite.h"
#include "ECS/BsEntitySparseSet.h"
#include "Scene/BsComponent.h"

using namespace bs;
using namespace bs::ecs;

static const TArray<Entity> kEntities = {
	Entity(0, 0),
	Entity(1, 0),
	Entity(2, 0),
	Entity(3, 0),
	Entity(5000, 0),
	Entity(50000, 0),
	Entity(50001, 0) };

void ECSTestSuite::StartUp() { }
void ECSTestSuite::ShutDown() { }

ECSTestSuite::ECSTestSuite()
{
	B3D_ADD_TEST(ECSTestSuite::TestSparseSet)
	B3D_ADD_TEST(ECSTestSuite::TestComponentSparseSet)
}

void ECSTestSuite::TestSparseSet()
{
	auto fnTestSparseSet = [this](auto&& entitySparseSet)
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
		entitySparseSet.Shrink();

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

	//EntitySparseSet entitySparseSet;
	//fnTestSparseSet(entitySparseSet); // TODO - Need separate test of this

	// TODO - Add tests for SortN

}

void ECSTestSuite::TestComponentSparseSet()
{
	struct Position
	{
		Position() = default;
		Position(float x, float y, float z)
			:X(x), Y(y), Z(z)
		{ }

		bool operator==(const Position& other)
		{
			return X == other.X && Y == other.Y && Z == other.Z;
		}

		float X = 0.0f;
		float Y = 0.0f;
		float Z = 0.0f;
	};

	struct NonMovableComponent
	{
		NonMovableComponent() = default;
		NonMovableComponent(NonMovableComponent&& other) = delete;

		float Value = 0.0f;
	};

	struct IsEnemyTag { };

	static_assert(std::is_move_constructible_v<Position> && std::is_move_assignable_v<Position>);
	static_assert(std::is_same_v<StorageType<Position>, TComponentSparseSet<Position>>, "Invalid storage type");
	static_assert(std::is_same_v<StorageType<NonMovableComponent>, TComponentSparseSet<NonMovableComponent, true>>, "Invalid storage type");
	static_assert(std::is_same_v<StorageType<IsEnemyTag>, TTagSparseSet<IsEnemyTag>>, "Invalid storage type");
	static_assert(std::is_same_v<StorageType<Entity>, EntitySparseSet>, "Invalid storage type");

	TComponentSparseSet<Position> positionComponentSparseSet;
	positionComponentSparseSet.Reserve(10);

	for(const auto& entity : kEntities)
		positionComponentSparseSet.Add(entity, 1.0f, 2.0f, 3.0f);

	for(const auto& entity : kEntities)
	{
		B3D_TEST_ASSERT(positionComponentSparseSet.Contains(entity));
		B3D_TEST_ASSERT(positionComponentSparseSet.Get(entity) == Position(1.0f, 2.0f, 3.0f));
	}

	// TODO - Add tests for:
	//  - Remove entry
	//  - Iterate components
	//  - Sort
	//  - ClearInvalid
	//  - Shrink
	//  - Clear


	// TODO - Add set of tests for tags and non-movable components


	//auto fnTestSparseSet = [this](auto&& entitySparseSet)
	//{
	//	for(const auto& entity : kEntities)
	//		entitySparseSet.Add(entity);

	//	for(const auto& entity : kEntities)
	//	{
	//		B3D_TEST_ASSERT(entitySparseSet.Contains(entity))
	//	}

	//	u32 foundEntityCount = 0;
	//	for(const auto entity : entitySparseSet)
	//	{
	//		auto found = std::find(kEntities.begin(), kEntities.end(), entity);
	//		B3D_TEST_ASSERT(found != kEntities.end())
	//		if(found != kEntities.end())
	//			foundEntityCount++;
	//	}

	//	B3D_TEST_ASSERT(foundEntityCount == (u32)kEntities.Size())

	//	auto foundEntry3 = entitySparseSet.Find(kEntities[3]);
	//	B3D_TEST_ASSERT(foundEntry3 != entitySparseSet.End())

	//	if(foundEntry3 != entitySparseSet.End())
	//	{
	//		entitySparseSet.Erase(*foundEntry3);
	//		B3D_TEST_ASSERT(entitySparseSet.Find(kEntities[3]) == entitySparseSet.End())
	//	}

	//	auto foundEntry6 = entitySparseSet.Find(kEntities[6]);
	//	B3D_TEST_ASSERT(foundEntry6 != entitySparseSet.End())

	//	if(foundEntry6 != entitySparseSet.End())
	//	{
	//		entitySparseSet.Erase(*foundEntry6);
	//		B3D_TEST_ASSERT(entitySparseSet.Find(kEntities[6]) == entitySparseSet.End())
	//	}

	//	foundEntityCount = 0;
	//	for(const auto entity : entitySparseSet)
	//	{
	//		auto found = std::find(kEntities.begin(), kEntities.end(), entity);
	//		if(found != kEntities.end())
	//			foundEntityCount++;
	//	}

	//	B3D_TEST_ASSERT(foundEntityCount == (u32)(kEntities.Size() - 2u))

	//	entitySparseSet.ClearInvalid();

	//	const u32 expectedEntityCount = entitySparseSet.GetDeletePolicy() == SparseSetDeletePolicy::SwapOnly
	//		? (u32)kEntities.Size()
	//		: (u32)(kEntities.Size() - 2u);
	//	B3D_TEST_ASSERT(entitySparseSet.Size() == expectedEntityCount)
	//};

	//fnTestSparseSet(inPlaceDeleteSparseSet);

	//// TODO - Add tests for SortN

}
