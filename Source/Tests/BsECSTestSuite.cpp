//************************************ B3D Framework - Copyright 2025 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsECSTestSuite.h"
#include "ECS/BsRegistry.h"
#include "Scene/BsComponent.h"
#include "Utility/BsShared.h"

using namespace b3d;
using namespace b3d::ecs;

static const TArray<Entity> kEntities = {
	Entity(0, 0),
	Entity(1, 0),
	Entity(2, 0),
	Entity(3, 0),
	Entity(5000, 0),
	Entity(50000, 0),
	Entity(50001, 0) };

namespace test
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

	struct Velocity 
	{
		Velocity() = default;
		Velocity(float x, float y, float z)
			:X(x), Y(y), Z(z)
		{ }

		bool operator==(const Velocity& other) const
		{
			return X == other.X && Y == other.Y && Z == other.Z;
		}

		bool operator<(const Velocity& other) const
		{
			return X < other.X;
		}

		float X = 0.0f;
		float Y = 0.0f;
		float Z = 0.0f;
	};

	struct IsEnemyTag { };
}

void ECSTestSuite::StartUp() { }
void ECSTestSuite::ShutDown() { }

ECSTestSuite::ECSTestSuite()
{
	B3D_ADD_TEST(ECSTestSuite::TestSparseSet)
	B3D_ADD_TEST(ECSTestSuite::TestRegistry)
	B3D_ADD_TEST(ECSTestSuite::TestComponentSparseSet)
	B3D_ADD_TEST(ECSTestSuite::TestViews)
	B3D_ADD_TEST(ECSTestSuite::TestGroups)
}

void ECSTestSuite::TestSparseSet()
{
	TShared<int> a = TShared<int>(new int);

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
}

template<typename ComponentType>
static void RunComponentTests(ECSTestSuite& testSuite)
{
	static constexpr bool kIsTypeMovable = std::is_move_constructible_v<ComponentType> && std::is_move_assignable_v<ComponentType>;
	static constexpr bool kIsTypeEmpty = std::is_empty_v<ComponentType>;

	TStorageType<ComponentType> componentSparseSet;
	componentSparseSet.Reserve(10);

	u32 index = 0;
	for(const auto& entity : kEntities)
	{
		if constexpr(!kIsTypeEmpty)
			componentSparseSet.Add(entity, (float)index + 1.0f, (float)index + 2.0f, (float)index + 3.0f);
		else
			componentSparseSet.Add(entity);

		index++;
	}

	index = 0;
	for(const auto& entity : kEntities)
	{
		B3D_TEST_ASSERT_EXTERNAL(testSuite, componentSparseSet.Contains(entity))
		if constexpr(!kIsTypeEmpty)
		{
			B3D_TEST_ASSERT_EXTERNAL(testSuite, componentSparseSet.Get(entity) == ComponentType((float)index + 1.0f, (float)index + 2.0f, (float)index + 3.0f))
		}

		index++;
	}

	if constexpr(!kIsTypeEmpty)
	{
		index = 0;
		for(const auto& component : componentSparseSet)
		{
			B3D_TEST_ASSERT_EXTERNAL(testSuite, component == ComponentType((float)index + 1.0f, (float)index + 2.0f, (float)index + 3.0f))
			index++;
		}
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

	if constexpr(!kIsTypeEmpty)
	{
		count = 0;
		for(const auto& [entity, component] : componentSparseSet.Each())
		{
			if(kIsTypeMovable || entity != kInvalidEntity)
			{
				B3D_TEST_ASSERT_EXTERNAL(testSuite, componentSparseSet.Get(entity) == component)
				count++;
			}
		}

		B3D_TEST_ASSERT_EXTERNAL(testSuite, count == (kEntities.Size() - 3))
	}

	if constexpr(!kIsTypeEmpty)
	{
		componentSparseSet.Add(kEntities[1], 2.0f, 3.0f, 4.0f);
		componentSparseSet.Add(kEntities[3], 4.0f, 5.0f, 6.0f);
		componentSparseSet.Add(kEntities[5], 6.0f, 7.0f, 8.0f);
	}
	else
	{
		componentSparseSet.Add(kEntities[1]);
		componentSparseSet.Add(kEntities[3]);
		componentSparseSet.Add(kEntities[5]);
	}

	if constexpr(kIsTypeMovable)
	{
		componentSparseSet.Sort();

		index = 0;
		for(const auto& entity : kEntities)
		{
			B3D_TEST_ASSERT_EXTERNAL(testSuite, componentSparseSet.Contains(entity))

			if constexpr(!kIsTypeEmpty)
			{
				B3D_TEST_ASSERT_EXTERNAL(testSuite, componentSparseSet.Get(entity) == ComponentType((float)index + 1.0f, (float)index + 2.0f, (float)index + 3.0f))
			}

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
	using namespace test;

	static_assert(std::is_move_constructible_v<Position> && std::is_move_assignable_v<Position>);
	static_assert(std::is_same_v<TStorageType<Position>, TComponentSparseSet<Position>>, "Invalid storage type");
	static_assert(std::is_same_v<TStorageType<NonMovablePosition>, TComponentSparseSet<NonMovablePosition, true>>, "Invalid storage type");
	static_assert(std::is_same_v<TStorageType<IsEnemyTag>, TTagSparseSet<IsEnemyTag>>, "Invalid storage type");
	static_assert(std::is_same_v<TStorageType<Entity>, EntitySparseSet>, "Invalid storage type");

	RunComponentTests<Position>(*this);
	RunComponentTests<NonMovablePosition>(*this);
	RunComponentTests<IsEnemyTag>(*this);

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

void ECSTestSuite::TestRegistry()
{
#define COMMA ,
	Registry registry;

	static constexpr u32 kEntityCount = 30;
	static constexpr u32 kEntityWithPositionCount = 20;
	static constexpr u32 kEntityWithVelocityCount = 10;
	std::array<Entity, kEntityCount> entities;

	for(u32 i = 0; i < kEntityCount; ++i)
	{
		entities[i] = registry.CreateEntity();
	}

	for(u32 i = 0; i < kEntityWithPositionCount; ++i)
	{
		registry.AddComponent<test::Position>(entities[i], (float)i + 1.0f, (float)i + 2.0f, (float)i + 3.0f);
	}

	registry.AddComponents(entities.begin(), entities.begin() + kEntityWithVelocityCount, test::Velocity(5.0f, 5.0f, 5.0f));

	for(u32 i = 0; i < kEntityCount; ++i)
	{
		B3D_TEST_ASSERT(registry.IsEntityValid(entities[i]))
		B3D_TEST_ASSERT(registry.GetEntityVersion(entities[i]) == 0)

		if(i < kEntityWithPositionCount)
		{
			B3D_TEST_ASSERT(registry.HasEntityAnyComponents(entities[i]));
			B3D_TEST_ASSERT(registry.HasAnyOf<test::Position COMMA test::Velocity>(entities[i]));

			const test::Position& position = registry.GetComponents<const test::Position>(entities[i]);
			B3D_TEST_ASSERT(position == test::Position((float)i + 1.0f, (float)i + 2.0f, (float)i + 3.0f));
		}

		if(i < kEntityWithVelocityCount)
		{
			B3D_TEST_ASSERT(registry.HasAllOf<test::Position COMMA test::Velocity>(entities[i]))
		}
	}

	registry.RemoveComponents<test::Velocity>(&entities[5], &entities[10]);

	for(u32 i = 0; i < kEntityWithVelocityCount; ++i)
	{
		if(i < 5)
		{
			B3D_TEST_ASSERT(registry.HasAllOf<test::Position COMMA test::Velocity>(entities[i]))
		}
		else
		{
			B3D_TEST_ASSERT(registry.HasAllOf<test::Position>(entities[i]))
			B3D_TEST_ASSERT(!registry.HasAllOf<test::Velocity>(entities[i]))
		}
	}

	registry.DestroyEntities(&entities[8], &entities[12]);

	for(u32 i = 0; i < kEntityCount; ++i)
	{
		if(i < 8 || i >= 12)
		{
			B3D_TEST_ASSERT(registry.IsEntityValid(entities[i]))
		}
		else
		{
			B3D_TEST_ASSERT(!registry.IsEntityValid(entities[i]))
		}
	}

	const Entity oldEntity10 = entities[10];
	const Entity oldEntity11 = entities[11];

	entities[10] = registry.CreateEntity(oldEntity10);
	registry.Shrink();

	entities[11] = registry.CreateEntity(oldEntity11);

	B3D_TEST_ASSERT(!registry.IsEntityValid(oldEntity10))
	B3D_TEST_ASSERT(!registry.IsEntityValid(oldEntity11))

	B3D_TEST_ASSERT(registry.IsEntityValid(entities[10]))
	B3D_TEST_ASSERT(registry.IsEntityValid(entities[11]))

	B3D_TEST_ASSERT(registry.GetEntityVersion(entities[10]) == 1)
	B3D_TEST_ASSERT(registry.GetEntityVersion(entities[11]) == 1)

	B3D_TEST_ASSERT(!registry.HasEntityAnyComponents(entities[10]))
	B3D_TEST_ASSERT(!registry.HasEntityAnyComponents(entities[11]))

	registry.ClearStorage<test::Velocity>();

	TComponentSparseSet<test::Velocity>* velocityStorage = registry.TryGetStorage<test::Velocity>();
	B3D_TEST_ASSERT(velocityStorage != nullptr)
	B3D_TEST_ASSERT(velocityStorage->Size() == 0)

	TComponentSparseSet<test::Position>* positionStorage = registry.TryGetStorage<test::Position>();
	B3D_TEST_ASSERT(positionStorage != nullptr)
	B3D_TEST_ASSERT(positionStorage->Size() == 16)

	for(u32 i = 0; i < kEntityWithPositionCount; ++i)
	{
		if(i < 8 || i >= 12)
		{
			B3D_TEST_ASSERT(!registry.HasAnyOf<test::Velocity>(entities[i]))
			B3D_TEST_ASSERT(registry.HasAnyOf<test::Position>(entities[i]))
		}
	}

	registry.RemoveStorage<test::Velocity>();
	B3D_TEST_ASSERT(registry.TryGetStorage<test::Velocity>() == nullptr)

	registry.RemoveStorage<test::Position>();
	for(u32 i = 0; i < kEntityCount; ++i)
	{
		if(i < 8 || i >= 10)
		{
			B3D_TEST_ASSERT(!registry.HasAnyOf<test::Velocity COMMA test::Position>(entities[i]))
		}
	}

	const EntitySparseSet* entityStorage = registry.TryGetStorage<Entity>();
	B3D_TEST_ASSERT(entityStorage != nullptr)
	B3D_TEST_ASSERT(entityStorage->Size() == 30)

	registry.Clear();
	registry.Shrink<Entity>();
	B3D_TEST_ASSERT(entityStorage->Size() == 0)
}

void ECSTestSuite::TestViews()
{
	using TestTypeList = TTypeList<test::Position, test::Velocity>;
	static_assert(TTypeListIndexOf<test::Position, TestTypeList> == 0);
	static_assert(TTypeListIndexOf<test::Velocity, TestTypeList> == 1);
	static_assert(std::is_same_v<TTypeListElementAt<0, TestTypeList>, test::Position>);
	static_assert(std::is_same_v<TTypeListElementAt<1, TestTypeList>, test::Velocity>);

	Registry registry;

	static constexpr u32 kEntityCount = 30;
	static constexpr u32 kEntityWithPositionCount = 20;
	static constexpr u32 kEntityWithVelocityCount = 10;
	static constexpr u32 kEntityWithEnemyTagCount = 5;
	std::array<Entity, kEntityCount> entities;

	for(u32 i = 0; i < kEntityCount; ++i)
	{
		entities[i] = registry.CreateEntity();
	}

	for(u32 i = 0; i < kEntityWithPositionCount; ++i)
	{
		registry.AddComponent<test::Position>(entities[i], (float)i + 1.0f, (float)i + 2.0f, (float)i + 3.0f);
	}

	registry.AddComponents(entities.begin(), entities.begin() + kEntityWithVelocityCount, test::Velocity(5.0f, 5.0f, 5.0f));
	registry.AddComponents(entities.begin(), entities.begin() + kEntityWithEnemyTagCount, test::IsEnemyTag());

	const Registry& constRegistry = registry;

	// Multi-view
	auto positionVelocityView = constRegistry.CreateView<test::Position, test::Velocity>();
	static_assert(std::is_same_v<decltype(std::get<0>(positionVelocityView.Get({}))), const test::Position&>, "Unexpected type");
	static_assert(std::is_same_v<decltype(std::get<1>(positionVelocityView.Get({}))), const test::Velocity&>, "Unexpected type");
	static_assert(!std::is_same_v<decltype(std::get<0>(positionVelocityView.Get({}))), test::Position&>, "Unexpected type");
	static_assert(!std::is_same_v<decltype(std::get<1>(positionVelocityView.Get({}))), test::Velocity&>, "Unexpected type");

	u32 index = 0;
	for(const auto& entity : positionVelocityView)
	{
		auto tuple = positionVelocityView.Get<test::Position, test::Velocity>(entity);
		auto tuple2 = positionVelocityView.Get(entity);

		const test::Position& position3 = positionVelocityView.GetStorage<test::Position>()->Get(entity);
		const test::Position& position2 = positionVelocityView.GetStorage<0>()->Get(entity);
		const test::Position& position1 = positionVelocityView.Get<test::Position>(entity);
		const test::Position& position = std::get<0>(tuple);
		const test::Position& position4 = std::get<0>(tuple2);
		const test::Velocity& velocity = std::get<1>(tuple);

		B3D_TEST_ASSERT(position == position1)
		B3D_TEST_ASSERT(position == position2)
		B3D_TEST_ASSERT(position == position3)
		B3D_TEST_ASSERT(position == position4)
		B3D_TEST_ASSERT(position == test::Position((float)index + 1.0f, (float)index + 2.0f, (float)index + 3.0f))
		B3D_TEST_ASSERT(velocity == test::Velocity(5.0f, 5.0f, 5.0f))

		index++;
	}

	B3D_TEST_ASSERT(index == Math::Min(kEntityWithVelocityCount, kEntityWithPositionCount))

	index = 0;
	for(auto [entity, position, velocity] : positionVelocityView.Each())
	{
		B3D_TEST_ASSERT(entity == entities[index])
		B3D_TEST_ASSERT(position == test::Position((float)index + 1.0f, (float)index + 2.0f, (float)index + 3.0f))
		B3D_TEST_ASSERT(velocity == test::Velocity(5.0f, 5.0f, 5.0f))

		index++;
	}

	B3D_TEST_ASSERT(index == Math::Min(kEntityWithVelocityCount, kEntityWithPositionCount))

	index = 0;
	positionVelocityView.DoForEach([&index, &entities, this](Entity entity, const test::Position& position, const test::Velocity& velocity)
	{
		B3D_TEST_ASSERT(entity == entities[index])
		B3D_TEST_ASSERT(position == test::Position((float)index + 1.0f, (float)index + 2.0f, (float)index + 3.0f))
		B3D_TEST_ASSERT(velocity == test::Velocity(5.0f, 5.0f, 5.0f))
		index++;
	});

	B3D_TEST_ASSERT(index == Math::Min(kEntityWithVelocityCount, kEntityWithPositionCount))

	index = 0;
	positionVelocityView.DoForEach([&index, &entities, this](const test::Position& position, const test::Velocity& velocity)
	{
		B3D_TEST_ASSERT(position == test::Position((float)index + 1.0f, (float)index + 2.0f, (float)index + 3.0f))
		B3D_TEST_ASSERT(velocity == test::Velocity(5.0f, 5.0f, 5.0f))
		index++;
	});

	B3D_TEST_ASSERT(index == Math::Min(kEntityWithVelocityCount, kEntityWithPositionCount))

	// Multi-view with tags
	auto positionVelocityEnemyView = constRegistry.CreateView<test::Position, test::Velocity, test::IsEnemyTag>();

	index = 0;
	for(const auto& entity : positionVelocityEnemyView)
	{
		auto tuple = positionVelocityEnemyView.Get<test::Position, test::Velocity>(entity);

		const test::Position& position = std::get<0>(tuple);
		const test::Velocity& velocity = std::get<1>(tuple);

		index++;
	}

	B3D_TEST_ASSERT(index == kEntityWithEnemyTagCount)

	index = 0;
	for(auto [entity, position, velocity] : positionVelocityEnemyView.Each())
	{
		B3D_TEST_ASSERT(entity == entities[index])
		B3D_TEST_ASSERT(position == test::Position((float)index + 1.0f, (float)index + 2.0f, (float)index + 3.0f))
		B3D_TEST_ASSERT(velocity == test::Velocity(5.0f, 5.0f, 5.0f))

		index++;
	}

	B3D_TEST_ASSERT(index == kEntityWithEnemyTagCount)

	index = 0;
	positionVelocityEnemyView.DoForEach([&index, &entities, this](Entity entity, const test::Position& position, const test::Velocity& velocity)
	{
		B3D_TEST_ASSERT(entity == entities[index])
		B3D_TEST_ASSERT(position == test::Position((float)index + 1.0f, (float)index + 2.0f, (float)index + 3.0f))
		B3D_TEST_ASSERT(velocity == test::Velocity(5.0f, 5.0f, 5.0f))
		index++;
	});

	B3D_TEST_ASSERT(index == kEntityWithEnemyTagCount)

	index = 0;
	positionVelocityEnemyView.DoForEach([&index, &entities, this](const test::Position& position, const test::Velocity& velocity)
	{
		B3D_TEST_ASSERT(position == test::Position((float)index + 1.0f, (float)index + 2.0f, (float)index + 3.0f))
		B3D_TEST_ASSERT(velocity == test::Velocity(5.0f, 5.0f, 5.0f))
		index++;
	});

	B3D_TEST_ASSERT(index == kEntityWithEnemyTagCount)

	// Test single storage
	auto positionView = registry.CreateView<const test::Position>();
	static_assert(std::is_same_v<decltype(std::get<0>(positionView.Get({}))), const test::Position&>, "Unexpected type");
	static_assert(!std::is_same_v<decltype(std::get<0>(positionView.Get({}))), test::Position&>, "Unexpected type");

	auto nonConstPositionView = registry.CreateView<test::Position>();
	static_assert(std::is_same_v<decltype(std::get<0>(nonConstPositionView.Get({}))), test::Position&>, "Unexpected type");
	static_assert(!std::is_same_v<decltype(std::get<0>(nonConstPositionView.Get({}))), const test::Position&>, "Unexpected type");

	index = 0;
	for(auto [entity, position] : positionView.Each())
	{
		B3D_TEST_ASSERT(entity == entities[index])
		B3D_TEST_ASSERT(position == test::Position((float)index + 1.0f, (float)index + 2.0f, (float)index + 3.0f))

		index++;
	}

	B3D_TEST_ASSERT(index == kEntityWithPositionCount)

	index = 0;
	nonConstPositionView.DoForEach([&index, &entities, this](test::Position& position)
	{
		B3D_TEST_ASSERT(position == test::Position((float)index + 1.0f, (float)index + 2.0f, (float)index + 3.0f))
		index++;
	});

	B3D_TEST_ASSERT(index == kEntityWithPositionCount)

	//auto isEnemyView = registry.CreateView<test::IsEnemyTag>();

	// TODO - Add empty type to view, try iterating it in single storage mode

	// TODO - Test view with exclude


}

void ECSTestSuite::TestGroups()
{
	Registry registry;

	static constexpr u32 kEntityCount = 30;
	static constexpr u32 kEntityWithPositionCount = kEntityCount;
	static constexpr u32 kEntityWithVelocityCount = kEntityCount / 2;
	static constexpr u32 kEntityWithEnemyTagCount = kEntityCount / 3;
	std::array<Entity, kEntityCount> entities;

	for(u32 i = 0; i < kEntityCount; ++i)
	{
		entities[i] = registry.CreateEntity();
		registry.AddComponent<test::Position>(entities[i], (float)i + 1.0f, (float)i + 2.0f, (float)i + 3.0f);
	}

	// Add velocity to every second entity
	for(u32 i = 0; i < kEntityWithVelocityCount; ++i)
		registry.AddComponent<test::Velocity>(entities[i * 2 + 0], 5.0f, 5.0f, 5.0f);

	// Add tags to last five entities
	for(u32 i = 0; i < kEntityWithEnemyTagCount; ++i)
		registry.AddComponents(entities.begin() + kEntityWithPositionCount - kEntityWithEnemyTagCount, entities.end(), test::IsEnemyTag());

	// Owning group
	auto positionVelocityOwningGroup = registry.GetOrCreateGroup<test::Position, test::Velocity>();

	static_assert(std::is_same_v<decltype(std::get<0>(positionVelocityOwningGroup.Get({}))), test::Position&>, "Unexpected type");
	static_assert(std::is_same_v<decltype(std::get<1>(positionVelocityOwningGroup.Get({}))), test::Velocity&>, "Unexpected type");

	//u32 index = 0;
	//for(const auto& entity : positionVelocityView)
	//{
	//	auto tuple = positionVelocityView.Get<test::Position, test::Velocity>(entity);
	//	auto tuple2 = positionVelocityView.Get(entity);

	//	const test::Position& position3 = positionVelocityView.GetStorage<test::Position>()->Get(entity);
	//	const test::Position& position2 = positionVelocityView.GetStorage<0>()->Get(entity);
	//	const test::Position& position1 = positionVelocityView.Get<test::Position>(entity);
	//	const test::Position& position = std::get<0>(tuple);
	//	const test::Position& position4 = std::get<0>(tuple2);
	//	const test::Velocity& velocity = std::get<1>(tuple);

	//	B3D_TEST_ASSERT(position == position1)
	//	B3D_TEST_ASSERT(position == position2)
	//	B3D_TEST_ASSERT(position == position3)
	//	B3D_TEST_ASSERT(position == position4)
	//	B3D_TEST_ASSERT(position == test::Position((float)index + 1.0f, (float)index + 2.0f, (float)index + 3.0f))
	//	B3D_TEST_ASSERT(velocity == test::Velocity(5.0f, 5.0f, 5.0f))

	//	index++;
	//}

	//B3D_TEST_ASSERT(index == Math::Min(kEntityWithVelocityCount, kEntityWithPositionCount))

	//index = 0;
	//for(auto [entity, position, velocity] : positionVelocityView.Each())
	//{
	//	B3D_TEST_ASSERT(entity == entities[index])
	//	B3D_TEST_ASSERT(position == test::Position((float)index + 1.0f, (float)index + 2.0f, (float)index + 3.0f))
	//	B3D_TEST_ASSERT(velocity == test::Velocity(5.0f, 5.0f, 5.0f))

	//	index++;
	//}

	//B3D_TEST_ASSERT(index == Math::Min(kEntityWithVelocityCount, kEntityWithPositionCount))

	//index = 0;
	//positionVelocityView.DoForEach([&index, &entities, this](Entity entity, const test::Position& position, const test::Velocity& velocity)
	//{
	//	B3D_TEST_ASSERT(entity == entities[index])
	//	B3D_TEST_ASSERT(position == test::Position((float)index + 1.0f, (float)index + 2.0f, (float)index + 3.0f))
	//	B3D_TEST_ASSERT(velocity == test::Velocity(5.0f, 5.0f, 5.0f))
	//	index++;
	//});

	//B3D_TEST_ASSERT(index == Math::Min(kEntityWithVelocityCount, kEntityWithPositionCount))

	//index = 0;
	//positionVelocityView.DoForEach([&index, &entities, this](const test::Position& position, const test::Velocity& velocity)
	//{
	//	B3D_TEST_ASSERT(position == test::Position((float)index + 1.0f, (float)index + 2.0f, (float)index + 3.0f))
	//	B3D_TEST_ASSERT(velocity == test::Velocity(5.0f, 5.0f, 5.0f))
	//	index++;
	//});

	//B3D_TEST_ASSERT(index == Math::Min(kEntityWithVelocityCount, kEntityWithPositionCount))

	//// Multi-view with tags
	//auto positionVelocityEnemyView = constRegistry.CreateView<test::Position, test::Velocity, test::IsEnemyTag>();

	//index = 0;
	//for(const auto& entity : positionVelocityEnemyView)
	//{
	//	auto tuple = positionVelocityEnemyView.Get<test::Position, test::Velocity>(entity);

	//	const test::Position& position = std::get<0>(tuple);
	//	const test::Velocity& velocity = std::get<1>(tuple);

	//	index++;
	//}

	//B3D_TEST_ASSERT(index == kEntityWithEnemyTagCount)

	//index = 0;
	//for(auto [entity, position, velocity] : positionVelocityEnemyView.Each())
	//{
	//	B3D_TEST_ASSERT(entity == entities[index])
	//	B3D_TEST_ASSERT(position == test::Position((float)index + 1.0f, (float)index + 2.0f, (float)index + 3.0f))
	//	B3D_TEST_ASSERT(velocity == test::Velocity(5.0f, 5.0f, 5.0f))

	//	index++;
	//}

	//B3D_TEST_ASSERT(index == kEntityWithEnemyTagCount)

	//index = 0;
	//positionVelocityEnemyView.DoForEach([&index, &entities, this](Entity entity, const test::Position& position, const test::Velocity& velocity)
	//{
	//	B3D_TEST_ASSERT(entity == entities[index])
	//	B3D_TEST_ASSERT(position == test::Position((float)index + 1.0f, (float)index + 2.0f, (float)index + 3.0f))
	//	B3D_TEST_ASSERT(velocity == test::Velocity(5.0f, 5.0f, 5.0f))
	//	index++;
	//});

	//B3D_TEST_ASSERT(index == kEntityWithEnemyTagCount)

	//index = 0;
	//positionVelocityEnemyView.DoForEach([&index, &entities, this](const test::Position& position, const test::Velocity& velocity)
	//{
	//	B3D_TEST_ASSERT(position == test::Position((float)index + 1.0f, (float)index + 2.0f, (float)index + 3.0f))
	//	B3D_TEST_ASSERT(velocity == test::Velocity(5.0f, 5.0f, 5.0f))
	//	index++;
	//});

	//B3D_TEST_ASSERT(index == kEntityWithEnemyTagCount)

	//// Test single storage
	//auto positionView = registry.CreateView<const test::Position>();
	//static_assert(std::is_same_v<decltype(std::get<0>(positionView.Get({}))), const test::Position&>, "Unexpected type");
	//static_assert(!std::is_same_v<decltype(std::get<0>(positionView.Get({}))), test::Position&>, "Unexpected type");

	//auto nonConstPositionView = registry.CreateView<test::Position>();
	//static_assert(std::is_same_v<decltype(std::get<0>(nonConstPositionView.Get({}))), test::Position&>, "Unexpected type");
	//static_assert(!std::is_same_v<decltype(std::get<0>(nonConstPositionView.Get({}))), const test::Position&>, "Unexpected type");

	//index = 0;
	//for(auto [entity, position] : positionView.Each())
	//{
	//	B3D_TEST_ASSERT(entity == entities[index])
	//	B3D_TEST_ASSERT(position == test::Position((float)index + 1.0f, (float)index + 2.0f, (float)index + 3.0f))

	//	index++;
	//}

	//B3D_TEST_ASSERT(index == kEntityWithPositionCount)

	//index = 0;
	//nonConstPositionView.DoForEach([&index, &entities, this](test::Position& position)
	//{
	//	B3D_TEST_ASSERT(position == test::Position((float)index + 1.0f, (float)index + 2.0f, (float)index + 3.0f))
	//	index++;
	//});

	//B3D_TEST_ASSERT(index == kEntityWithPositionCount)

	//auto isEnemyView = registry.CreateView<test::IsEnemyTag>();

	// TODO - Add empty type to view, try iterating it in single storage mode

	// TODO - Test view with exclude


}
