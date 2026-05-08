//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DSceneObjectTransformTestSuite.h"

#include "Math/B3DMath.h"
#include "Scene/B3DSceneObject.h"
#include "Scene/B3DSceneInstance.h"
#include "Scene/B3DSceneObjectFragments.h"
#include "Scene/B3DTransformSystem.h"
#include "Scene/B3DGameObjectCollection.h"
#include "ECS/B3DRegistry.h"
#include "Serialization/B3DBinarySerializer.h"
#include "FileSystem/B3DDataStream.h"
#include "Utility/B3DUtility.h"

using namespace b3d;

static constexpr float kEpsilon = 0.001f;

SceneObjectTransformTestSuite::SceneObjectTransformTestSuite()
	: TestSuite("SceneObjectTransformTestSuite")
{
	// Phase 1: ECS Fragment Tests
	B3D_ADD_TEST(SceneObjectTransformTestSuite::TestLocalTransformFragment)
	B3D_ADD_TEST(SceneObjectTransformTestSuite::TestWorldTransformFragment)
	B3D_ADD_TEST(SceneObjectTransformTestSuite::TestParentChildFragments)
	B3D_ADD_TEST(SceneObjectTransformTestSuite::TestHierarchyDepthFragment)
	B3D_ADD_TEST(SceneObjectTransformTestSuite::TestTransformDirtyTag)
	B3D_ADD_TEST(SceneObjectTransformTestSuite::TestMobilityTags)
	B3D_ADD_TEST(SceneObjectTransformTestSuite::TestMobilityAffectsTransformDirty)

	// Phase 2: TransformSystem Tests
	B3D_ADD_TEST(SceneObjectTransformTestSuite::TestTransformSystemRootOnly)
	B3D_ADD_TEST(SceneObjectTransformTestSuite::TestTransformSystemHierarchy)
	B3D_ADD_TEST(SceneObjectTransformTestSuite::TestTransformSystemDirtyPropagation)
	B3D_ADD_TEST(SceneObjectTransformTestSuite::TestTransformSystemPartialDirty)
	B3D_ADD_TEST(SceneObjectTransformTestSuite::TestTransformSystemMultipleUpdates)

	// Phase 3: Serialization & Cloning Tests
	B3D_ADD_TEST(SceneObjectTransformTestSuite::TestSceneObjectSerialization)
	B3D_ADD_TEST(SceneObjectTransformTestSuite::TestSceneObjectClone)
	B3D_ADD_TEST(SceneObjectTransformTestSuite::TestSceneObjectCloneTransformIndependence)
}

/************************************************************************/
/* 					Phase 1: ECS Fragment Tests							*/
/************************************************************************/

void SceneObjectTransformTestSuite::TestLocalTransformFragment()
{
	TShared<SceneInstance> scene = SceneInstance::Create("TestLocalTransform");

	HSceneObject sceneObject = scene->CreateSceneObject("TestObject");
	ecs::Registry& registry = scene->GetGameObjectCollection()->GetECSRegistry();
	ecs::Entity entity = sceneObject->GetECSEntity();

	// Verify default local transform is identity
	const Transform& localTransform = sceneObject->GetLocalTransform();
	B3D_TEST_ASSERT(Math::ApproxEquals(localTransform.GetPosition(), Vector3::kZero, kEpsilon))
	B3D_TEST_ASSERT(Math::ApproxEquals(localTransform.GetRotation(), Quaternion::kIdentity, kEpsilon))
	B3D_TEST_ASSERT(Math::ApproxEquals(localTransform.GetScale(), Vector3::kOne, kEpsilon))

	// SetPosition
	sceneObject->SetPosition(Vector3(1.0f, 2.0f, 3.0f));
	const ecs::LocalTransform& ecsLocal = registry.GetComponents<ecs::LocalTransform>(entity);
	B3D_TEST_ASSERT(Math::ApproxEquals(ecsLocal.GetPosition(), Vector3(1.0f, 2.0f, 3.0f), kEpsilon))

	// SetRotation
	Quaternion rotation(Degree(0), Degree(90), Degree(0));
	sceneObject->SetRotation(rotation);
	B3D_TEST_ASSERT(Math::ApproxEquals(ecsLocal.GetRotation(), rotation, kEpsilon))

	// SetScale
	sceneObject->SetScale(Vector3(2.0f, 3.0f, 4.0f));
	B3D_TEST_ASSERT(Math::ApproxEquals(ecsLocal.GetScale(), Vector3(2.0f, 3.0f, 4.0f), kEpsilon))

	// SetLocalTransform (all at once)
	Transform newTransform(Vector3(5.0f, 6.0f, 7.0f), Quaternion::kIdentity, Vector3(0.5f, 0.5f, 0.5f));
	sceneObject->SetLocalTransform(newTransform);
	B3D_TEST_ASSERT(Math::ApproxEquals(sceneObject->GetLocalTransform().GetPosition(), Vector3(5.0f, 6.0f, 7.0f), kEpsilon))
	B3D_TEST_ASSERT(Math::ApproxEquals(sceneObject->GetLocalTransform().GetScale(), Vector3(0.5f, 0.5f, 0.5f), kEpsilon))

	scene->Destroy();
}

void SceneObjectTransformTestSuite::TestWorldTransformFragment()
{
	TShared<SceneInstance> scene = SceneInstance::Create("TestWorldTransform");

	HSceneObject parent = scene->CreateSceneObject("Parent");
	HSceneObject child = scene->CreateSceneObject("Child");
	child->SetParent(parent);

	// Set parent position, child local position
	parent->SetPosition(Vector3(10.0f, 0.0f, 0.0f));
	child->SetPosition(Vector3(0.0f, 5.0f, 0.0f));

	// Child world position should be parent + child local
	const Transform& childWorld = child->GetTransform();
	B3D_TEST_ASSERT(Math::ApproxEquals(childWorld.GetPosition(), Vector3(10.0f, 5.0f, 0.0f), kEpsilon))

	// Change parent position and verify child updates
	parent->SetPosition(Vector3(20.0f, 0.0f, 0.0f));
	const Transform& childWorldUpdated = child->GetTransform();
	B3D_TEST_ASSERT(Math::ApproxEquals(childWorldUpdated.GetPosition(), Vector3(20.0f, 5.0f, 0.0f), kEpsilon))

	// Test SetWorldPosition decomposes into local space correctly
	child->SetWorldPosition(Vector3(25.0f, 10.0f, 0.0f));
	B3D_TEST_ASSERT(Math::ApproxEquals(child->GetLocalTransform().GetPosition(), Vector3(5.0f, 10.0f, 0.0f), kEpsilon))
	B3D_TEST_ASSERT(Math::ApproxEquals(child->GetTransform().GetPosition(), Vector3(25.0f, 10.0f, 0.0f), kEpsilon))

	// Test with rotation: parent rotated 90 degrees around Y
	parent->SetPosition(Vector3::kZero);
	parent->SetRotation(Quaternion(Degree(0), Degree(90), Degree(0)));
	child->SetPosition(Vector3(1.0f, 0.0f, 0.0f));

	const Transform& childRotatedWorld = child->GetTransform();
	// Rotating (1,0,0) by 90 degrees around Y gives approximately (0,0,-1)
	B3D_TEST_ASSERT(Math::ApproxEquals(childRotatedWorld.GetPosition(), Vector3(0.0f, 0.0f, -1.0f), kEpsilon))

	scene->Destroy();
}

void SceneObjectTransformTestSuite::TestParentChildFragments()
{
	TShared<SceneInstance> scene = SceneInstance::Create("TestParentChild");
	ecs::Registry& registry = scene->GetGameObjectCollection()->GetECSRegistry();

	HSceneObject root = scene->CreateSceneObject("Root");
	HSceneObject childA = scene->CreateSceneObject("ChildA");
	HSceneObject childB = scene->CreateSceneObject("ChildB");
	HSceneObject childC = scene->CreateSceneObject("ChildC");

	childA->SetParent(root);
	childB->SetParent(childA);
	childC->SetParent(root);

	// Verify Parent component on childA points to root's entity
	const ecs::Parent& childAParent = registry.GetComponents<ecs::Parent>(childA->GetECSEntity());
	B3D_TEST_ASSERT(childAParent.Entity == root->GetECSEntity())

	// Verify Children component on root contains childA and childC entities
	const ecs::Children& rootChildren = registry.GetComponents<ecs::Children>(root->GetECSEntity());
	B3D_TEST_ASSERT(rootChildren.Entities.Size() == 2)

	bool foundChildA = false;
	bool foundChildC = false;
	for(ecs::Entity childEntity : rootChildren.Entities)
	{
		if(childEntity == childA->GetECSEntity())
			foundChildA = true;
		if(childEntity == childC->GetECSEntity())
			foundChildC = true;
	}
	B3D_TEST_ASSERT(foundChildA)
	B3D_TEST_ASSERT(foundChildC)

	// Reparent childB from childA to childC
	childB->SetParent(childC);
	const ecs::Parent& childBParentAfter = registry.GetComponents<ecs::Parent>(childB->GetECSEntity());
	B3D_TEST_ASSERT(childBParentAfter.Entity == childC->GetECSEntity())

	// Verify childA no longer has children
	B3D_TEST_ASSERT(!registry.HasAllOf<ecs::Children>(childA->GetECSEntity()))

	// Verify childC now has childB as child
	const ecs::Children& childCChildren = registry.GetComponents<ecs::Children>(childC->GetECSEntity());
	B3D_TEST_ASSERT(childCChildren.Entities.Size() == 1)
	B3D_TEST_ASSERT(childCChildren.Entities[0] == childB->GetECSEntity())

	scene->Destroy();
}

void SceneObjectTransformTestSuite::TestHierarchyDepthFragment()
{
	TShared<SceneInstance> scene = SceneInstance::Create("TestHierarchyDepth");
	ecs::Registry& registry = scene->GetGameObjectCollection()->GetECSRegistry();

	HSceneObject root = scene->CreateSceneObject("Root");
	HSceneObject childA = scene->CreateSceneObject("ChildA");
	HSceneObject childB = scene->CreateSceneObject("ChildB");
	HSceneObject childC = scene->CreateSceneObject("ChildC");

	childA->SetParent(root);
	childB->SetParent(childA);
	childC->SetParent(childB);

	// Root is parented to scene root, so root depth = 1, A = 2, B = 3, C = 4
	u16 rootDepth = registry.GetComponents<ecs::HierarchyDepth>(root->GetECSEntity()).Depth;
	u16 depthA = registry.GetComponents<ecs::HierarchyDepth>(childA->GetECSEntity()).Depth;
	u16 depthB = registry.GetComponents<ecs::HierarchyDepth>(childB->GetECSEntity()).Depth;
	u16 depthC = registry.GetComponents<ecs::HierarchyDepth>(childC->GetECSEntity()).Depth;

	B3D_TEST_ASSERT(depthA == rootDepth + 1)
	B3D_TEST_ASSERT(depthB == rootDepth + 2)
	B3D_TEST_ASSERT(depthC == rootDepth + 3)

	// Reparent C to root — depth should decrease
	childC->SetParent(root);
	u16 newDepthC = registry.GetComponents<ecs::HierarchyDepth>(childC->GetECSEntity()).Depth;
	B3D_TEST_ASSERT(newDepthC == rootDepth + 1)

	// Reparent A (with child B) under C
	childA->SetParent(childC);
	u16 newDepthA = registry.GetComponents<ecs::HierarchyDepth>(childA->GetECSEntity()).Depth;
	u16 newDepthB = registry.GetComponents<ecs::HierarchyDepth>(childB->GetECSEntity()).Depth;
	B3D_TEST_ASSERT(newDepthA == newDepthC + 1)
	B3D_TEST_ASSERT(newDepthB == newDepthA + 1)

	scene->Destroy();
}

void SceneObjectTransformTestSuite::TestTransformDirtyTag()
{
	TShared<SceneInstance> scene = SceneInstance::Create("TestTransformDirty");
	ecs::Registry& registry = scene->GetGameObjectCollection()->GetECSRegistry();

	HSceneObject parent = scene->CreateSceneObject("Parent");
	HSceneObject child = scene->CreateSceneObject("Child");
	child->SetParent(parent);

	// Resolve transforms (clears dirty flags)
	parent->UpdateWorldTransformIfDirty();
	child->UpdateWorldTransformIfDirty();

	// Verify dirty tags are cleared
	B3D_TEST_ASSERT(!registry.HasAllOf<ecs::TransformDirty>(parent->GetECSEntity()))
	B3D_TEST_ASSERT(!registry.HasAllOf<ecs::TransformDirty>(child->GetECSEntity()))

	// Modify parent's local position
	parent->SetPosition(Vector3(1.0f, 0.0f, 0.0f));

	// Verify both parent and child are tagged dirty (NotifyTransformChanged recurses to children)
	B3D_TEST_ASSERT(registry.HasAllOf<ecs::TransformDirty>(parent->GetECSEntity()))
	B3D_TEST_ASSERT(registry.HasAllOf<ecs::TransformDirty>(child->GetECSEntity()))

	// Resolve world transform — should clear dirty
	child->UpdateWorldTransformIfDirty();
	B3D_TEST_ASSERT(!registry.HasAllOf<ecs::TransformDirty>(child->GetECSEntity()))

	scene->Destroy();
}

void SceneObjectTransformTestSuite::TestMobilityTags()
{
	TShared<SceneInstance> scene = SceneInstance::Create("TestMobilityTags");
	ecs::Registry& registry = scene->GetGameObjectCollection()->GetECSRegistry();

	HSceneObject sceneObject = scene->CreateSceneObject("TestObject");
	ecs::Entity entity = sceneObject->GetECSEntity();

	// Default mobility should be Movable
	B3D_TEST_ASSERT(sceneObject->GetMobility() == ObjectMobility::Movable)
	B3D_TEST_ASSERT(registry.HasAllOf<ecs::Movable>(entity))
	B3D_TEST_ASSERT(!registry.HasAllOf<ecs::Immovable>(entity))
	B3D_TEST_ASSERT(!registry.HasAllOf<ecs::Static>(entity))

	// Set to Static
	sceneObject->SetMobility(ObjectMobility::Static);
	B3D_TEST_ASSERT(sceneObject->GetMobility() == ObjectMobility::Static)
	B3D_TEST_ASSERT(!registry.HasAllOf<ecs::Movable>(entity))
	B3D_TEST_ASSERT(!registry.HasAllOf<ecs::Immovable>(entity))
	B3D_TEST_ASSERT(registry.HasAllOf<ecs::Static>(entity))

	// Set to Immovable
	sceneObject->SetMobility(ObjectMobility::Immovable);
	B3D_TEST_ASSERT(sceneObject->GetMobility() == ObjectMobility::Immovable)
	B3D_TEST_ASSERT(!registry.HasAllOf<ecs::Movable>(entity))
	B3D_TEST_ASSERT(registry.HasAllOf<ecs::Immovable>(entity))
	B3D_TEST_ASSERT(!registry.HasAllOf<ecs::Static>(entity))

	// Set back to Movable
	sceneObject->SetMobility(ObjectMobility::Movable);
	B3D_TEST_ASSERT(sceneObject->GetMobility() == ObjectMobility::Movable)
	B3D_TEST_ASSERT(registry.HasAllOf<ecs::Movable>(entity))

	scene->Destroy();
}

void SceneObjectTransformTestSuite::TestMobilityAffectsTransformDirty()
{
	TShared<SceneInstance> scene = SceneInstance::Create("TestMobilityDirty");
	ecs::Registry& registry = scene->GetGameObjectCollection()->GetECSRegistry();

	HSceneObject sceneObject = scene->CreateSceneObject("TestObject");
	ecs::Entity entity = sceneObject->GetECSEntity();

	// Set to Static
	sceneObject->SetMobility(ObjectMobility::Static);

	// Resolve transform
	sceneObject->UpdateWorldTransformIfDirty();
	B3D_TEST_ASSERT(!registry.HasAllOf<ecs::TransformDirty>(entity))

	// Try to modify position — static objects skip TCF_Transform so dirty should NOT be set
	sceneObject->SetPosition(Vector3(1.0f, 0.0f, 0.0f));
	B3D_TEST_ASSERT(!registry.HasAllOf<ecs::TransformDirty>(entity))

	// Set to Movable and modify — dirty SHOULD be set
	sceneObject->SetMobility(ObjectMobility::Movable);

	// Resolve after mobility change
	sceneObject->UpdateWorldTransformIfDirty();

	sceneObject->SetPosition(Vector3(2.0f, 0.0f, 0.0f));
	B3D_TEST_ASSERT(registry.HasAllOf<ecs::TransformDirty>(entity))

	scene->Destroy();
}

/************************************************************************/
/* 				Phase 2: TransformSystem Tests							*/
/************************************************************************/

void SceneObjectTransformTestSuite::TestTransformSystemRootOnly()
{
	TShared<SceneInstance> scene = SceneInstance::Create("TestSystemRootOnly");
	ecs::Registry& registry = scene->GetGameObjectCollection()->GetECSRegistry();

	HSceneObject objectA = scene->CreateSceneObject("A");
	HSceneObject objectB = scene->CreateSceneObject("B");
	HSceneObject objectC = scene->CreateSceneObject("C");

	objectA->SetPosition(Vector3(1.0f, 2.0f, 3.0f));
	objectB->SetPosition(Vector3(4.0f, 5.0f, 6.0f));
	objectC->SetPosition(Vector3(7.0f, 8.0f, 9.0f));

	// Run TransformSystem
	ecs::TransformSystem::Update(registry);

	// For root-level objects (parented to scene root), world position should match
	// the composed transform of scene root + local. Since scene root is at identity,
	// world should equal local.
	const ecs::WorldTransform& worldA = registry.GetComponents<ecs::WorldTransform>(objectA->GetECSEntity());
	const ecs::WorldTransform& worldB = registry.GetComponents<ecs::WorldTransform>(objectB->GetECSEntity());
	const ecs::WorldTransform& worldC = registry.GetComponents<ecs::WorldTransform>(objectC->GetECSEntity());

	// These objects are children of the scene root. After TransformSystem, world = composed with parent.
	// Since scene root is at identity, world should match local.
	const ecs::LocalTransform& localA = registry.GetComponents<ecs::LocalTransform>(objectA->GetECSEntity());
	const ecs::LocalTransform& localB = registry.GetComponents<ecs::LocalTransform>(objectB->GetECSEntity());
	const ecs::LocalTransform& localC = registry.GetComponents<ecs::LocalTransform>(objectC->GetECSEntity());

	B3D_TEST_ASSERT(Math::ApproxEquals(worldA.GetPosition(), localA.GetPosition(), kEpsilon))
	B3D_TEST_ASSERT(Math::ApproxEquals(worldB.GetPosition(), localB.GetPosition(), kEpsilon))
	B3D_TEST_ASSERT(Math::ApproxEquals(worldC.GetPosition(), localC.GetPosition(), kEpsilon))

	// Verify dirty tags are cleared
	B3D_TEST_ASSERT(!registry.HasAllOf<ecs::TransformDirty>(objectA->GetECSEntity()))
	B3D_TEST_ASSERT(!registry.HasAllOf<ecs::TransformDirty>(objectB->GetECSEntity()))
	B3D_TEST_ASSERT(!registry.HasAllOf<ecs::TransformDirty>(objectC->GetECSEntity()))

	scene->Destroy();
}

void SceneObjectTransformTestSuite::TestTransformSystemHierarchy()
{
	TShared<SceneInstance> scene = SceneInstance::Create("TestSystemHierarchy");
	ecs::Registry& registry = scene->GetGameObjectCollection()->GetECSRegistry();

	HSceneObject root = scene->CreateSceneObject("Root");
	HSceneObject parentObj = scene->CreateSceneObject("Parent");
	HSceneObject child = scene->CreateSceneObject("Child");
	HSceneObject grandChild = scene->CreateSceneObject("GrandChild");

	parentObj->SetParent(root);
	child->SetParent(parentObj);
	grandChild->SetParent(child);

	root->SetPosition(Vector3(1.0f, 0.0f, 0.0f));
	parentObj->SetPosition(Vector3(0.0f, 2.0f, 0.0f));
	child->SetPosition(Vector3(0.0f, 0.0f, 3.0f));
	grandChild->SetPosition(Vector3(0.5f, 0.5f, 0.5f));

	// Run TransformSystem
	ecs::TransformSystem::Update(registry);

	// Compute expected world positions (no rotation/scale, so positions just add)
	// root's parent is the scene root (identity), so root world = (1, 0, 0)
	// parent world = root world + parent local = (1, 2, 0)
	// child world = parent world + child local = (1, 2, 3)
	// grandChild world = child world + grandChild local = (1.5, 2.5, 3.5)
	const ecs::WorldTransform& rootWorld = registry.GetComponents<ecs::WorldTransform>(root->GetECSEntity());
	const ecs::WorldTransform& parentWorld = registry.GetComponents<ecs::WorldTransform>(parentObj->GetECSEntity());
	const ecs::WorldTransform& childWorld = registry.GetComponents<ecs::WorldTransform>(child->GetECSEntity());
	const ecs::WorldTransform& grandChildWorld = registry.GetComponents<ecs::WorldTransform>(grandChild->GetECSEntity());

	B3D_TEST_ASSERT(Math::ApproxEquals(rootWorld.GetPosition(), Vector3(1.0f, 0.0f, 0.0f), kEpsilon))
	B3D_TEST_ASSERT(Math::ApproxEquals(parentWorld.GetPosition(), Vector3(1.0f, 2.0f, 0.0f), kEpsilon))
	B3D_TEST_ASSERT(Math::ApproxEquals(childWorld.GetPosition(), Vector3(1.0f, 2.0f, 3.0f), kEpsilon))
	B3D_TEST_ASSERT(Math::ApproxEquals(grandChildWorld.GetPosition(), Vector3(1.5f, 2.5f, 3.5f), kEpsilon))

	scene->Destroy();
}

void SceneObjectTransformTestSuite::TestTransformSystemDirtyPropagation()
{
	TShared<SceneInstance> scene = SceneInstance::Create("TestSystemDirtyPropagation");
	ecs::Registry& registry = scene->GetGameObjectCollection()->GetECSRegistry();

	HSceneObject objectA = scene->CreateSceneObject("A");
	HSceneObject objectB = scene->CreateSceneObject("B");
	HSceneObject objectC = scene->CreateSceneObject("C");
	HSceneObject objectD = scene->CreateSceneObject("D");

	objectB->SetParent(objectA);
	objectC->SetParent(objectB);
	objectD->SetParent(objectA);

	// First, resolve all transforms
	ecs::TransformSystem::Update(registry);

	// Now modify only A — NotifyTransformChanged will already tag children, but
	// TransformSystem::PropagateDirtyFlags should ensure all descendants are dirty
	objectA->SetPosition(Vector3(5.0f, 0.0f, 0.0f));
	objectB->SetPosition(Vector3(0.0f, 1.0f, 0.0f));
	objectC->SetPosition(Vector3(0.0f, 0.0f, 1.0f));
	objectD->SetPosition(Vector3(0.0f, 0.0f, 2.0f));

	ecs::TransformSystem::Update(registry);

	// A world = (5, 0, 0) relative to scene root
	// B world = A + B local = (5, 1, 0)
	// C world = B + C local = (5, 1, 1)
	// D world = A + D local = (5, 0, 2)
	const ecs::WorldTransform& worldA = registry.GetComponents<ecs::WorldTransform>(objectA->GetECSEntity());
	const ecs::WorldTransform& worldB = registry.GetComponents<ecs::WorldTransform>(objectB->GetECSEntity());
	const ecs::WorldTransform& worldC = registry.GetComponents<ecs::WorldTransform>(objectC->GetECSEntity());
	const ecs::WorldTransform& worldD = registry.GetComponents<ecs::WorldTransform>(objectD->GetECSEntity());

	B3D_TEST_ASSERT(Math::ApproxEquals(worldA.GetPosition(), Vector3(5.0f, 0.0f, 0.0f), kEpsilon))
	B3D_TEST_ASSERT(Math::ApproxEquals(worldB.GetPosition(), Vector3(5.0f, 1.0f, 0.0f), kEpsilon))
	B3D_TEST_ASSERT(Math::ApproxEquals(worldC.GetPosition(), Vector3(5.0f, 1.0f, 1.0f), kEpsilon))
	B3D_TEST_ASSERT(Math::ApproxEquals(worldD.GetPosition(), Vector3(5.0f, 0.0f, 2.0f), kEpsilon))

	// All dirty tags should be cleared
	B3D_TEST_ASSERT(!registry.HasAllOf<ecs::TransformDirty>(objectA->GetECSEntity()))
	B3D_TEST_ASSERT(!registry.HasAllOf<ecs::TransformDirty>(objectB->GetECSEntity()))
	B3D_TEST_ASSERT(!registry.HasAllOf<ecs::TransformDirty>(objectC->GetECSEntity()))
	B3D_TEST_ASSERT(!registry.HasAllOf<ecs::TransformDirty>(objectD->GetECSEntity()))

	scene->Destroy();
}

void SceneObjectTransformTestSuite::TestTransformSystemPartialDirty()
{
	TShared<SceneInstance> scene = SceneInstance::Create("TestSystemPartialDirty");
	ecs::Registry& registry = scene->GetGameObjectCollection()->GetECSRegistry();

	HSceneObject root = scene->CreateSceneObject("Root");
	HSceneObject childA = scene->CreateSceneObject("A");
	HSceneObject childB = scene->CreateSceneObject("B");
	HSceneObject childC = scene->CreateSceneObject("C");
	HSceneObject childD = scene->CreateSceneObject("D");

	childA->SetParent(root);
	childB->SetParent(childA);
	childC->SetParent(root);
	childD->SetParent(childC);

	root->SetPosition(Vector3(1.0f, 0.0f, 0.0f));
	childA->SetPosition(Vector3(0.0f, 1.0f, 0.0f));
	childB->SetPosition(Vector3(0.0f, 0.0f, 1.0f));
	childC->SetPosition(Vector3(2.0f, 0.0f, 0.0f));
	childD->SetPosition(Vector3(0.0f, 0.0f, 3.0f));

	// Resolve all transforms first
	ecs::TransformSystem::Update(registry);

	// Record A and B's world positions before partial change
	Vector3 worldPosA = registry.GetComponents<ecs::WorldTransform>(childA->GetECSEntity()).GetPosition();
	Vector3 worldPosB = registry.GetComponents<ecs::WorldTransform>(childB->GetECSEntity()).GetPosition();

	// Only modify C (not root or A)
	childC->SetPosition(Vector3(10.0f, 0.0f, 0.0f));

	ecs::TransformSystem::Update(registry);

	// A and B should be unchanged
	Vector3 newWorldPosA = registry.GetComponents<ecs::WorldTransform>(childA->GetECSEntity()).GetPosition();
	Vector3 newWorldPosB = registry.GetComponents<ecs::WorldTransform>(childB->GetECSEntity()).GetPosition();
	B3D_TEST_ASSERT(Math::ApproxEquals(newWorldPosA, worldPosA, kEpsilon))
	B3D_TEST_ASSERT(Math::ApproxEquals(newWorldPosB, worldPosB, kEpsilon))

	// C and D should be updated
	// C world = root world + C local = (1, 0, 0) + (10, 0, 0) = (11, 0, 0)
	// D world = C world + D local = (11, 0, 0) + (0, 0, 3) = (11, 0, 3)
	Vector3 worldPosC = registry.GetComponents<ecs::WorldTransform>(childC->GetECSEntity()).GetPosition();
	Vector3 worldPosD = registry.GetComponents<ecs::WorldTransform>(childD->GetECSEntity()).GetPosition();
	B3D_TEST_ASSERT(Math::ApproxEquals(worldPosC, Vector3(11.0f, 0.0f, 0.0f), kEpsilon))
	B3D_TEST_ASSERT(Math::ApproxEquals(worldPosD, Vector3(11.0f, 0.0f, 3.0f), kEpsilon))

	scene->Destroy();
}

void SceneObjectTransformTestSuite::TestTransformSystemMultipleUpdates()
{
	TShared<SceneInstance> scene = SceneInstance::Create("TestSystemMultipleUpdates");
	ecs::Registry& registry = scene->GetGameObjectCollection()->GetECSRegistry();

	HSceneObject root = scene->CreateSceneObject("Root");
	HSceneObject child = scene->CreateSceneObject("Child");
	HSceneObject grandChild = scene->CreateSceneObject("GrandChild");

	child->SetParent(root);
	grandChild->SetParent(child);

	root->SetPosition(Vector3(1.0f, 0.0f, 0.0f));
	child->SetPosition(Vector3(0.0f, 1.0f, 0.0f));
	grandChild->SetPosition(Vector3(0.0f, 0.0f, 1.0f));

	// First update
	ecs::TransformSystem::Update(registry);

	Vector3 grandChildWorldPos1 = registry.GetComponents<ecs::WorldTransform>(grandChild->GetECSEntity()).GetPosition();
	B3D_TEST_ASSERT(Math::ApproxEquals(grandChildWorldPos1, Vector3(1.0f, 1.0f, 1.0f), kEpsilon))

	// Modify mid-level node
	child->SetPosition(Vector3(0.0f, 5.0f, 0.0f));

	// Second update
	ecs::TransformSystem::Update(registry);

	// GrandChild should now be at (1, 5, 1)
	Vector3 grandChildWorldPos2 = registry.GetComponents<ecs::WorldTransform>(grandChild->GetECSEntity()).GetPosition();
	B3D_TEST_ASSERT(Math::ApproxEquals(grandChildWorldPos2, Vector3(1.0f, 5.0f, 1.0f), kEpsilon))

	// Root should be unchanged
	Vector3 rootWorldPos = registry.GetComponents<ecs::WorldTransform>(root->GetECSEntity()).GetPosition();
	B3D_TEST_ASSERT(Math::ApproxEquals(rootWorldPos, Vector3(1.0f, 0.0f, 0.0f), kEpsilon))

	scene->Destroy();
}

/************************************************************************/
/* 			Phase 3: Serialization & Cloning Tests						*/
/************************************************************************/

void SceneObjectTransformTestSuite::TestSceneObjectSerialization()
{
	TShared<SceneInstance> scene = SceneInstance::Create("TestSerialization");

	HSceneObject root = scene->CreateSceneObject("SerRoot");
	HSceneObject child = scene->CreateSceneObject("SerChild");
	child->SetParent(root);

	root->SetPosition(Vector3(10.0f, 20.0f, 30.0f));
	root->SetRotation(Quaternion(Degree(0), Degree(45), Degree(0)));
	root->SetScale(Vector3(2.0f, 2.0f, 2.0f));
	root->SetMobility(ObjectMobility::Immovable);

	child->SetPosition(Vector3(1.0f, 2.0f, 3.0f));

	// Resolve transforms before serializing
	root->UpdateWorldTransformIfDirty();
	child->UpdateWorldTransformIfDirty();

	// Serialize
	TShared<MemoryDataStream> stream = B3DMakeShared<MemoryDataStream>();
	BinarySerializer serializer;
	serializer.Encode(root.Get(), stream);

	// Create a new collection for deserialization
	TShared<GameObjectCollection> destCollection = GameObjectCollection::Create();

	RTTIOperationEngineContext rttiContext;
	rttiContext.PreserveGameObjectIds = false;
	rttiContext.GameObjectCollection = destCollection;

	stream->Seek(0);
	TShared<SceneObject> deserialized = std::static_pointer_cast<SceneObject>(
		serializer.Decode(stream, (u32)stream->Size(), rttiContext));

	B3D_TEST_ASSERT(deserialized != nullptr)

	// Verify local transform
	const Transform& deserializedLocal = deserialized->GetLocalTransform();
	B3D_TEST_ASSERT(Math::ApproxEquals(deserializedLocal.GetPosition(), Vector3(10.0f, 20.0f, 30.0f), kEpsilon))
	B3D_TEST_ASSERT(Math::ApproxEquals(deserializedLocal.GetRotation(), Quaternion(Degree(0), Degree(45), Degree(0)), kEpsilon))
	B3D_TEST_ASSERT(Math::ApproxEquals(deserializedLocal.GetScale(), Vector3(2.0f, 2.0f, 2.0f), kEpsilon))

	// Verify mobility
	B3D_TEST_ASSERT(deserialized->GetMobility() == ObjectMobility::Immovable)

	// Verify hierarchy is preserved (child was serialized as part of root)
	B3D_TEST_ASSERT(deserialized->GetChildCount() == 1)

	HSceneObject deserializedChild = deserialized->GetChild(0);
	const Transform& childLocal = deserializedChild->GetLocalTransform();
	B3D_TEST_ASSERT(Math::ApproxEquals(childLocal.GetPosition(), Vector3(1.0f, 2.0f, 3.0f), kEpsilon))

	// Clean up deserialized hierarchy
	deserialized->Destroy(true);

	scene->Destroy();
}

void SceneObjectTransformTestSuite::TestSceneObjectClone()
{
	TShared<SceneInstance> scene = SceneInstance::Create("TestClone");

	HSceneObject root = scene->CreateSceneObject("CloneRoot");
	HSceneObject childA = scene->CreateSceneObject("CloneChildA");
	HSceneObject childB = scene->CreateSceneObject("CloneChildB");

	childA->SetParent(root);
	childB->SetParent(childA);

	root->SetPosition(Vector3(1.0f, 2.0f, 3.0f));
	childA->SetPosition(Vector3(4.0f, 5.0f, 6.0f));
	childA->SetMobility(ObjectMobility::Static);
	childB->SetPosition(Vector3(7.0f, 8.0f, 9.0f));

	// Clone into the same scene
	HSceneObject clone = root->Clone();

	B3D_TEST_ASSERT(clone != nullptr)

	// Verify structure
	B3D_TEST_ASSERT(clone->GetChildCount() == 1)
	HSceneObject clonedChildA = clone->GetChild(0);
	B3D_TEST_ASSERT(clonedChildA->GetChildCount() == 1)
	HSceneObject clonedChildB = clonedChildA->GetChild(0);

	// Verify transforms match
	B3D_TEST_ASSERT(Math::ApproxEquals(clone->GetLocalTransform().GetPosition(), Vector3(1.0f, 2.0f, 3.0f), kEpsilon))
	B3D_TEST_ASSERT(Math::ApproxEquals(clonedChildA->GetLocalTransform().GetPosition(), Vector3(4.0f, 5.0f, 6.0f), kEpsilon))
	B3D_TEST_ASSERT(Math::ApproxEquals(clonedChildB->GetLocalTransform().GetPosition(), Vector3(7.0f, 8.0f, 9.0f), kEpsilon))

	// Verify cloned objects have different instance IDs
	B3D_TEST_ASSERT(clone->GetId() != root->GetId())
	B3D_TEST_ASSERT(clonedChildA->GetId() != childA->GetId())
	B3D_TEST_ASSERT(clonedChildB->GetId() != childB->GetId())

	// Verify mobility matches
	B3D_TEST_ASSERT(clonedChildA->GetMobility() == ObjectMobility::Static)

	scene->Destroy();
}

void SceneObjectTransformTestSuite::TestSceneObjectCloneTransformIndependence()
{
	TShared<SceneInstance> scene = SceneInstance::Create("TestCloneIndependence");

	HSceneObject root = scene->CreateSceneObject("IndepRoot");
	HSceneObject child = scene->CreateSceneObject("IndepChild");
	child->SetParent(root);

	root->SetPosition(Vector3(1.0f, 2.0f, 3.0f));
	child->SetPosition(Vector3(4.0f, 5.0f, 6.0f));

	// Clone the hierarchy
	HSceneObject clone = root->Clone();
	HSceneObject clonedChild = clone->GetChild(0);

	// Modify original
	root->SetPosition(Vector3(100.0f, 200.0f, 300.0f));
	child->SetPosition(Vector3(400.0f, 500.0f, 600.0f));

	// Clone should be unaffected
	B3D_TEST_ASSERT(Math::ApproxEquals(clone->GetLocalTransform().GetPosition(), Vector3(1.0f, 2.0f, 3.0f), kEpsilon))
	B3D_TEST_ASSERT(Math::ApproxEquals(clonedChild->GetLocalTransform().GetPosition(), Vector3(4.0f, 5.0f, 6.0f), kEpsilon))

	// Modify clone
	clone->SetPosition(Vector3(999.0f, 999.0f, 999.0f));

	// Original should be unaffected
	B3D_TEST_ASSERT(Math::ApproxEquals(root->GetLocalTransform().GetPosition(), Vector3(100.0f, 200.0f, 300.0f), kEpsilon))

	scene->Destroy();
}
