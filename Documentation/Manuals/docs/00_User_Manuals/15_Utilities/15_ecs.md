---
title: Entity component system
---

The framework provides a low-level entity component system (ECS) that allows efficient storage and iteration of entities and their associated components. This manual covers the core ECS types and their usage.

> **Note:** The framework's primary gameplay architecture uses @b3d::SceneObject and @b3d::Component classes, which provide a higher-level interface built on different principles. The ECS system documented here is a low-level utility primarily used for systems that require maximum performance and cache efficiency. ECS can be combined with the high-level scene object/component system to create a powerful gameplay architecture.

# What is ECS?

Entity Component System (ECS) is an architectural pattern that separates data (components) from entities. An **entity** is just a unique identifier, while **components** store the actual data. This separation allows for:

- **Cache-friendly iteration** - Components are stored contiguously in memory
- **Composition over inheritance** - Entities gain functionality by adding components
- **Fast queries** - Quickly find all entities with specific component combinations

# Entities

An entity is represented by the @b3d::ecs::Entity type, which contains both a unique identifier and a version number:

~~~~~~~~~~~~~{.cpp}
using namespace b3d::ecs;

// Entities are created by the registry
Registry registry;
Entity entity = registry.CreateEntity();

// Entity contains identifier and version
Entity::IdentifierType id = entity.GetIdentifier();
Entity::VersionType version = entity.GetVersion();
~~~~~~~~~~~~~

The version number allows the system to detect when an entity has been destroyed and its identifier reused. This prevents accessing stale entities.

## Special entity values

~~~~~~~~~~~~~{.cpp}
// Null entity (invalid identifier, any version)
Entity nullEntity = kNullEntity;
if (entity == kNullEntity)
    B3D_LOG(Info, LogGeneric, "Entity is null");

// Invalid entity (any identifier, invalid version)
Entity invalidEntity = kInvalidEntity;
if (entity == kInvalidEntity)
    B3D_LOG(Info, LogGeneric, "Entity was destroyed");
~~~~~~~~~~~~~

# Registry

The @b3d::ecs::Registry is the central hub of the ECS system. It stores all entities and components and provides methods for creating, destroying, and querying them:

~~~~~~~~~~~~~{.cpp}
Registry registry;

// Create entities
Entity player = registry.CreateEntity();
Entity enemy1 = registry.CreateEntity();
Entity enemy2 = registry.CreateEntity();

// Destroy an entity (also removes all its components)
registry.EraseEntity(enemy1);

// Check if entity is still valid
bool isValid = registry.IsEntityValid(player);
~~~~~~~~~~~~~

## Adding components

Components can be any C++ type. Add components to entities using @b3d::ecs::Registry::AddComponent:

~~~~~~~~~~~~~{.cpp}
struct Position
{
    float x, y, z;
};

struct Velocity
{
    float dx, dy, dz;
};

struct Health
{
    i32 current;
    i32 maximum;
};

// Add components to entity
Position& pos = registry.AddComponent<Position>(player, 0.0f, 0.0f, 0.0f);
Velocity& vel = registry.AddComponent<Velocity>(player, 1.0f, 0.0f, 0.0f);
Health& health = registry.AddComponent<Health>(player, 100, 100);
~~~~~~~~~~~~~

Components are constructed in-place, so you can pass constructor arguments directly.

## Retrieving components

Use @b3d::ecs::Registry::GetComponents to retrieve components from an entity:

~~~~~~~~~~~~~{.cpp}
// Get single component
Position& pos = registry.GetComponents<Position>(player);
pos.x += 10.0f;

// Get multiple components as tuple
auto [position, velocity] = registry.GetComponents<Position, Velocity>(player);
position.x += velocity.dx;

// Safely get components (returns nullptr if not present)
Position* pos = registry.TryGetComponents<Position>(player);
if (pos != nullptr)
    pos->x += 10.0f;
~~~~~~~~~~~~~

## Removing components

~~~~~~~~~~~~~{.cpp}
// Remove single component
registry.RemoveComponents<Velocity>(player);

// Remove multiple components
registry.RemoveComponents<Position, Velocity>(player);

// Check if entity has components
if (registry.HasAllOf<Position, Velocity>(player))
    B3D_LOG(Info, LogGeneric, "Entity has position and velocity");

if (registry.HasAnyOf<Health, Shield>(player))
    B3D_LOG(Info, LogGeneric, "Entity has health or shield");
~~~~~~~~~~~~~

# Views

Views allow you to iterate over all entities that have specific components. They are lightweight, compile-time filtered queries:

~~~~~~~~~~~~~{.cpp}
// Create a view for all entities with Position and Velocity
auto view = registry.CreateView<Position, Velocity>();

// Iterate using range-based for loop
for (auto [entity, position, velocity] : view.Each())
{
    position.x += velocity.dx;
    position.y += velocity.dy;
    position.z += velocity.dz;
}

// Iterate using DoForEach
view.DoForEach([](Entity entity, Position& pos, Velocity& vel)
{
    pos.x += vel.dx;
    pos.y += vel.dy;
    pos.z += vel.dz;
});

// Iterate over components only (faster)
view.DoForEach([](Position& pos, Velocity& vel)
{
    pos.x += vel.dx;
    pos.y += vel.dy;
    pos.z += vel.dz;
});
~~~~~~~~~~~~~

## Excluded types

You can exclude entities that have certain components:

~~~~~~~~~~~~~{.cpp}
// Find entities with Position but without Frozen component
auto view = registry.CreateView<Position>(TExcludedTypes<Frozen>{});

for (auto [entity, position] : view.Each())
{
    // Process all non-frozen positioned entities
    position.y -= 9.8f; // Apply gravity
}
~~~~~~~~~~~~~

## View operations

~~~~~~~~~~~~~{.cpp}
auto view = registry.CreateView<Position, Velocity>();

// Get size estimate
u64 estimatedSize = view.GetSizeEstimate();

// Check if entity is in view
if (view.Contains(entity))
    B3D_LOG(Info, LogGeneric, "Entity matches view filter");

// Get specific components for entity
auto [pos, vel] = view.Get<Position, Velocity>(entity);

// Access first/last entity
Entity first = view.Front();
Entity last = view.Back();

// Find entity in view
auto it = view.Find(entity);
if (it != view.End())
    B3D_LOG(Info, LogGeneric, "Found entity");
~~~~~~~~~~~~~

# Groups

Groups are similar to views but provide additional features:
- **Owned components** - Tightly packed for maximum iteration speed
- **Sorting** - Can sort entities based on components
- **Persistence** - Groups are stored in the registry and updated automatically

~~~~~~~~~~~~~{.cpp}
// Create a group that owns Position and Velocity components
auto group = registry.GetOrCreateGroup<TOwnedTypes<Position, Velocity>>();

// Iterate (very fast - components are tightly packed)
for (auto [entity, position, velocity] : group.Each())
{
    position.x += velocity.dx;
    position.y += velocity.dy;
    position.z += velocity.dz;
}
~~~~~~~~~~~~~

## Owning vs non-owning groups

**Owning groups** reorganize component storage so owned components are tightly packed. This makes iteration very fast, but means a component can only be owned by one group:

~~~~~~~~~~~~~{.cpp}
// Owning group - Position and Velocity are tightly packed
auto group1 = registry.GetOrCreateGroup<TOwnedTypes<Position, Velocity>>();

// This would fail - Position already owned by group1
// auto group2 = registry.GetOrCreateGroup<TOwnedTypes<Position, Health>>();
~~~~~~~~~~~~~

**Non-owning groups** don't reorganize storage but still maintain a list of matching entities:

~~~~~~~~~~~~~{.cpp}
// Non-owning group - just tracks entities with these components
auto group = registry.GetOrCreateGroup<TOwnedTypes<>>(
    TIncludedTypes<Position, Velocity>{});

// Iterate over matched entities
for (auto [entity, position, velocity] : group.Each())
{
    // Process entities
}
~~~~~~~~~~~~~

## Sorting groups

Groups can be sorted, which is useful for rendering order or other priority-based systems:

~~~~~~~~~~~~~{.cpp}
struct RenderOrder
{
    i32 layer;
    float depth;
};

auto group = registry.GetOrCreateGroup<TOwnedTypes<Position, RenderOrder>>();

// Sort by layer, then by depth
group.Sort<RenderOrder>([](const RenderOrder& lhs, const RenderOrder& rhs)
{
    if (lhs.layer != rhs.layer)
        return lhs.layer < rhs.layer;
    return lhs.depth < rhs.depth;
});

// Iterate in sorted order
for (auto [entity, position, renderOrder] : group.Each())
{
    // Entities are now in correct render order
}
~~~~~~~~~~~~~

You can sort by multiple components or by entity itself:

~~~~~~~~~~~~~{.cpp}
// Sort by Entity identifier
group.Sort<>([](Entity lhs, Entity rhs)
{
    return lhs.GetIdentifier() < rhs.GetIdentifier();
});

// Sort by multiple components
group.Sort<Position, RenderOrder>([](const auto& lhsTuple, const auto& rhsTuple)
{
    auto& [lhsPos, lhsOrder] = lhsTuple;
    auto& [rhsPos, rhsOrder] = rhsTuple;
    return lhsPos.x < rhsPos.x;
});
~~~~~~~~~~~~~

# Runtime views

Unlike regular views which require component types at compile-time, runtime views allow dynamic component filtering:

~~~~~~~~~~~~~{.cpp}
RuntimeView view;

// Dynamically add include filters
auto& posStorage = registry.GetOrCreateStorage<Position>();
auto& velStorage = registry.GetOrCreateStorage<Velocity>();
view.Include(posStorage);
view.Include(velStorage);

// Optionally add exclude filters
auto& frozenStorage = registry.GetOrCreateStorage<Frozen>();
view.Exclude(frozenStorage);

// Iterate over matching entities
for (Entity entity : view)
{
    // Process entity
    Position& pos = registry.GetComponents<Position>(entity);
    Velocity& vel = registry.GetComponents<Velocity>(entity);

    pos.x += vel.dx;
}

// Or use DoForEach
view.DoForEach([&registry](Entity entity)
{
    auto [pos, vel] = registry.GetComponents<Position, Velocity>(entity);
    pos.x += vel.dx;
});
~~~~~~~~~~~~~

Runtime views are less performant than compile-time views but are useful when:
- Component types are determined at runtime
- Building dynamic query systems
- Implementing scripting interfaces

# Tags

Tags are empty components used to mark entities with specific properties:

~~~~~~~~~~~~~{.cpp}
struct Player { };
struct Enemy { };
struct Dead { };

// Add tags to entities
registry.AddComponent<Player>(playerEntity);
registry.AddComponent<Enemy>(enemy1);
registry.AddComponent<Dead>(deadEntity);

// Query by tags
auto playerView = registry.CreateView<Player>();
auto livingEnemies = registry.CreateView<Enemy>(TExcludedTypes<Dead>{});

for (auto [entity] : playerView.Each())
{
    // Process player entities
}
~~~~~~~~~~~~~

Tags have zero memory overhead per entity - only the presence in storage is tracked.

# Component storage

The ECS uses **sparse sets** to store components efficiently. Understanding storage can help optimize performance.

## Storage types

Different component types use different storage implementations:

- **Empty types** (tags) - No payload storage, uses @b3d::ecs::TTagSparseSet
- **Movable types** - Uses @b3d::ecs::TComponentSparseSet with swap-and-erase deletion
- **Non-movable types** - Uses @b3d::ecs::TComponentSparseSet with in-place deletion

~~~~~~~~~~~~~{.cpp}
// Tag storage (no payload)
struct Frozen { };

// Movable component storage
struct Position { float x, y, z; };

// Non-movable component storage
struct NonMovable
{
    NonMovable() = default;
    NonMovable(const NonMovable&) = delete;
    NonMovable(NonMovable&&) = delete;
};
~~~~~~~~~~~~~

## Accessing storage directly

You can access component storage directly for advanced operations:

~~~~~~~~~~~~~{.cpp}
// Get storage for Position components
auto* storage = registry.TryGetStorage<Position>();
if (storage != nullptr)
{
    // Iterate over all positions directly (very fast)
    for (Position& pos : *storage)
    {
        pos.y -= 9.8f; // Apply gravity
    }

    // Get storage size
    u64 count = storage->Size();

    // Reserve capacity
    storage->Reserve(10000);
}
~~~~~~~~~~~~~

## Sorting storage

Component storage can be sorted independently:

~~~~~~~~~~~~~{.cpp}
// Sort Position components by X coordinate
registry.Sort<Position>([](const Entity& lhs, const Entity& rhs)
{
    // Note: This receives entities, you need to get components yourself
    return lhs.GetIdentifier() < rhs.GetIdentifier();
});

// Sort one storage based on another
registry.SortAs<Velocity, Position>();
~~~~~~~~~~~~~

# Common patterns

## Systems

Organize logic into systems that process components:

~~~~~~~~~~~~~{.cpp}
class MovementSystem
{
public:
    void Update(Registry& registry, float deltaTime)
    {
        auto view = registry.CreateView<Position, Velocity>();

        view.DoForEach([deltaTime](Position& pos, const Velocity& vel)
        {
            pos.x += vel.dx * deltaTime;
            pos.y += vel.dy * deltaTime;
            pos.z += vel.dz * deltaTime;
        });
    }
};

class GravitySystem
{
public:
    void Update(Registry& registry, float deltaTime)
    {
        auto view = registry.CreateView<Position, Velocity>(
            TExcludedTypes<Frozen>{});

        view.DoForEach([deltaTime](Velocity& vel)
        {
            vel.dy -= 9.8f * deltaTime;
        });
    }
};

// Usage
MovementSystem movementSystem;
GravitySystem gravitySystem;

void GameLoop(Registry& registry, float deltaTime)
{
    gravitySystem.Update(registry, deltaTime);
    movementSystem.Update(registry, deltaTime);
}
~~~~~~~~~~~~~

## Entity construction

Helper functions for creating entities with components:

~~~~~~~~~~~~~{.cpp}
Entity CreatePlayer(Registry& registry, float x, float y, float z)
{
    Entity entity = registry.CreateEntity();
    registry.AddComponent<Player>(entity);
    registry.AddComponent<Position>(entity, x, y, z);
    registry.AddComponent<Velocity>(entity, 0.0f, 0.0f, 0.0f);
    registry.AddComponent<Health>(entity, 100, 100);
    return entity;
}

Entity CreateProjectile(Registry& registry, const Position& pos, const Velocity& vel)
{
    Entity entity = registry.CreateEntity();
    registry.AddComponent<Projectile>(entity);
    registry.AddComponent<Position>(entity, pos);
    registry.AddComponent<Velocity>(entity, vel);
    return entity;
}
~~~~~~~~~~~~~

## Component relationships

Use entity references in components to create relationships:

~~~~~~~~~~~~~{.cpp}
struct Parent
{
    Entity parent;
};

struct Owner
{
    Entity owner;
};

// Create parent-child relationship
Entity parent = registry.CreateEntity();
Entity child = registry.CreateEntity();
registry.AddComponent<Parent>(child, parent);

// Query children of an entity
auto childView = registry.CreateView<Parent>();
for (auto [entity, parentComp] : childView.Each())
{
    if (parentComp.parent == parent)
    {
        // This entity is a child of parent
    }
}
~~~~~~~~~~~~~

# Performance considerations

## Iteration performance

Views and groups have different performance characteristics:

~~~~~~~~~~~~~{.cpp}
// Slower: Checks multiple storages per entity
auto view = registry.CreateView<Position, Velocity, Health>();

// Faster: Owns components for tight packing
auto group = registry.GetOrCreateGroup<TOwnedTypes<Position, Velocity, Health>>();

// Fastest: Direct storage iteration (single component)
auto& storage = registry.GetOrCreateStorage<Position>();
for (Position& pos : storage)
{
    // Very fast iteration
}
~~~~~~~~~~~~~

**Recommendations:**
- Use groups for frequently iterated component combinations
- Use views for occasional queries
- Use direct storage access when iterating single components
- Avoid creating temporary views in hot loops - cache them

## Memory management

~~~~~~~~~~~~~{.cpp}
// Reserve capacity upfront to avoid reallocations
auto& storage = registry.GetOrCreateStorage<Position>();
storage.Reserve(10000);

// Shrink storage to reduce memory usage
storage.Shrink();

// Clear all entities and components
registry.Clear();

// Clear specific component type
registry.ClearStorage<Position>();
~~~~~~~~~~~~~

## Entity recycling

The registry automatically recycles entity identifiers. Destroyed entities have their identifiers reused with incremented versions:

~~~~~~~~~~~~~{.cpp}
Entity entity1 = registry.CreateEntity();
registry.EraseEntity(entity1);

Entity entity2 = registry.CreateEntity();
// entity2 may have same identifier as entity1 but different version

// This is why version checking is important
if (registry.IsEntityValid(entity1))
{
    // Will be false - version mismatch
}
~~~~~~~~~~~~~

# Thread safety

The ECS registry is **not thread-safe**. If you need concurrent access:

- Use one registry per thread
- Use external synchronization (mutexes)
- Process different component types in parallel (they use separate storage)

~~~~~~~~~~~~~{.cpp}
// Safe: Different component types can be modified concurrently
ThreadPool::Execute([&registry]()
{
    auto view = registry.CreateView<Position>();
    view.DoForEach([](Position& pos) { pos.x += 1.0f; });
});

ThreadPool::Execute([&registry]()
{
    auto view = registry.CreateView<Velocity>();
    view.DoForEach([](Velocity& vel) { vel.dx *= 0.99f; });
});
~~~~~~~~~~~~~

