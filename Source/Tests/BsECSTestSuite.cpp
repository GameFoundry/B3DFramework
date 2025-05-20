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

template<typename ComponentType>
static void RunComponentTests(ECSTestSuite& testSuite)
{
	static constexpr bool kIsTypeMovable = std::is_move_constructible_v<ComponentType> && std::is_move_assignable_v<ComponentType>;

	StorageType<ComponentType> componentSparseSet;
	componentSparseSet.Reserve(10);

	u32 index = 0;
	for(const auto& entity : kEntities)
	{
		componentSparseSet.Add(entity, (float)index + 1.0f, (float)index + 2.0f, (float)index + 3.0f);
		index++;
	}

	index = 0;
	for(const auto& entity : kEntities)
	{
		B3D_TEST_ASSERT_EXTERNAL(testSuite, componentSparseSet.Contains(entity))
		B3D_TEST_ASSERT_EXTERNAL(testSuite, componentSparseSet.Get(entity) == ComponentType((float)index + 1.0f, (float)index + 2.0f, (float)index + 3.0f))

		index++;
	}

	index = 0;
	for(const auto& component : componentSparseSet)
	{
		B3D_TEST_ASSERT_EXTERNAL(testSuite, component == ComponentType((float)index + 1.0f, (float)index + 2.0f, (float)index + 3.0f))
		index++;
	}

	componentSparseSet.Erase(kEntities[1]);
	componentSparseSet.Erase(kEntities[3]);
	componentSparseSet.Erase(kEntities[5]);

	componentSparseSet.Shrink();

	B3D_TEST_ASSERT_EXTERNAL(testSuite, !componentSparseSet.Contains(kEntities[1]))
	B3D_TEST_ASSERT_EXTERNAL(testSuite, !componentSparseSet.Contains(kEntities[3]))
	B3D_TEST_ASSERT_EXTERNAL(testSuite, !componentSparseSet.Contains(kEntities[5]))

	u32 count = 0;
	for(const auto& component : componentSparseSet)
		count++;

	if constexpr(kIsTypeMovable)
		B3D_TEST_ASSERT_EXTERNAL(testSuite, count == (kEntities.Size() - 3))
	else
		B3D_TEST_ASSERT_EXTERNAL(testSuite, count == kEntities.Size())

	componentSparseSet.Add(kEntities[1], 2.0f, 3.0f, 4.0f);
	componentSparseSet.Add(kEntities[3], 4.0f, 5.0f, 6.0f);
	componentSparseSet.Add(kEntities[5], 6.0f, 7.0f, 8.0f);

	if constexpr(kIsTypeMovable)
	{
		componentSparseSet.Sort();

		index = 0;
		for(const auto& entity : kEntities)
		{
			B3D_TEST_ASSERT_EXTERNAL(testSuite, componentSparseSet.Contains(entity))
			B3D_TEST_ASSERT_EXTERNAL(testSuite, componentSparseSet.Get(entity) == ComponentType((float)index + 1.0f, (float)index + 2.0f, (float)index + 3.0f))

			index++;
		}
	}

	componentSparseSet.Clear();

	count = 0;
	for(const auto& component : componentSparseSet)
		count++;

	B3D_TEST_ASSERT_EXTERNAL(testSuite, count == 0)

	componentSparseSet.Shrink();
	B3D_TEST_ASSERT_EXTERNAL(testSuite, componentSparseSet.Capacity() == 0)
}

void ECSTestSuite::TestComponentSparseSet()
{
	struct Position
	{
		Position() = default;
		Position(float x, float y, float z)
			:X(x), Y(y), Z(z)
		{ }

		bool operator==(const Position& other) const
		{
			return X == other.X && Y == other.Y && Z == other.Z;
		}

		bool operator<(const Position& other) const
		{
			return X < other.X;
		}

		float X = 0.0f;
		float Y = 0.0f;
		float Z = 0.0f;
	};

	struct NonMovablePosition 
	{
		NonMovablePosition() = default;
		NonMovablePosition(float x, float y, float z)
			:X(x), Y(y), Z(z)
		{ }

		NonMovablePosition(NonMovablePosition&& other) = delete;

		bool operator==(const NonMovablePosition& other) const
		{
			return X == other.X && Y == other.Y && Z == other.Z;
		}

		bool operator<(const NonMovablePosition& other) const
		{
			return X < other.X;
		}

		float X = 0.0f;
		float Y = 0.0f;
		float Z = 0.0f;
	};

	struct IsEnemyTag { };

	static_assert(std::is_move_constructible_v<Position> && std::is_move_assignable_v<Position>);
	static_assert(std::is_same_v<StorageType<Position>, TComponentSparseSet<Position>>, "Invalid storage type");
	static_assert(std::is_same_v<StorageType<NonMovablePosition>, TComponentSparseSet<NonMovablePosition, true>>, "Invalid storage type");
	static_assert(std::is_same_v<StorageType<IsEnemyTag>, TTagSparseSet<IsEnemyTag>>, "Invalid storage type");
	static_assert(std::is_same_v<StorageType<Entity>, EntitySparseSet>, "Invalid storage type");

	RunComponentTests<Position>(*this);
	RunComponentTests<NonMovablePosition>(*this);

	static constexpr u32 kEntityCount = 10;

	std::array<Entity, kEntityCount> createdEntities;

	EntitySparseSet entitySparseSet;
	for(u32 i = 0; i < kEntityCount; ++i)
		createdEntities[i] = entitySparseSet.Create();

	for(const auto& entity : createdEntities)
		B3D_TEST_ASSERT(entitySparseSet.Contains(entity))

	u32 index = 0;
	for(const auto& entity : entitySparseSet)
	{
		B3D_TEST_ASSERT(entity == createdEntities[index])
		index++;
	}

	B3D_TEST_ASSERT(index == kEntityCount)

	entitySparseSet.Erase(createdEntities[1]);
	entitySparseSet.Erase(createdEntities[3]);
	entitySparseSet.Erase(createdEntities[5]);

	entitySparseSet.Shrink();

	B3D_TEST_ASSERT(!entitySparseSet.Contains(createdEntities[1]))
	B3D_TEST_ASSERT(!entitySparseSet.Contains(createdEntities[3]))
	B3D_TEST_ASSERT(!entitySparseSet.Contains(createdEntities[5]))

	B3D_TEST_ASSERT(entitySparseSet.Size() == kEntityCount)

	createdEntities[1] = entitySparseSet.Create(createdEntities[1]);
	createdEntities[3] = entitySparseSet.Create(createdEntities[3]);
	createdEntities[5] = entitySparseSet.Create(createdEntities[5]);

	B3D_TEST_ASSERT(entitySparseSet.Size() == kEntityCount)

	entitySparseSet.Clear();

	B3D_TEST_ASSERT(entitySparseSet.Size() == 0)

	entitySparseSet.Shrink();
	B3D_TEST_ASSERT(entitySparseSet.Capacity() == 0)

}
