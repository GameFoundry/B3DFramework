---
title: Scenes and prefabs
---

Scenes and prefabs are the primary way to organize and reuse your game content. Scenes allow you to save and load complete hierarchies of scene objects, while prefabs enable you to create reusable templates that can be instantiated multiple times.

# Scenes
A @b3d::Scene is a saveable hierarchy of scene objects that can be instantiated into a scene instance. Scenes are resources, which means they can be saved to disk and loaded like any other resource.

## Creating scenes
You can create a scene from any scene object hierarchy using @b3d::Scene::Create:

~~~~~~~~~~~~~{.cpp}
// Create a scene object hierarchy
HSceneObject root = SceneObject::Create("LevelRoot");

HSceneObject player = SceneObject::Create("Player");
player->SetParent(root);
HRenderable playerRenderable = player->AddComponent<Renderable>();
// ... set up player

HSceneObject enemy = SceneObject::Create("Enemy");
enemy->SetParent(root);
// ... set up enemy

// Create a scene from the hierarchy
HScene scene = Scene::Create(root);
~~~~~~~~~~~~~

## Saving and loading scenes
Once you've created a scene, you can save it as a resource:

~~~~~~~~~~~~~{.cpp}
// Save the scene to a package
GetResources().SaveAsSinglePackage(scene, "D:/MyGame/Levels/", "Level01");
// This creates D:/MyGame/Levels/Level01.b3d
~~~~~~~~~~~~~

To load and instantiate a scene:

~~~~~~~~~~~~~{.cpp}
// Load the scene resource
HScene scene = GetResources().Load<Scene>("D:/MyGame/Levels/Level01.b3d/Level01");

// Instantiate the scene into the world
TShared<SceneInstance> sceneInstance = scene->Instantiate();

// The instantiated hierarchy is now active in the scene
~~~~~~~~~~~~~

## Scene instances
When you instantiate a scene, it creates a @b3d::SceneInstance that contains the instantiated scene object hierarchy. The scene instance manages the lifetime of all scene objects created from the scene.

~~~~~~~~~~~~~{.cpp}
// Instantiate a scene
TShared<SceneInstance> sceneInstance = scene->Instantiate();

// You can destroy the entire scene instance when done
sceneInstance->Destroy();
~~~~~~~~~~~~~

# Prefabs
A @b3d::Prefab is a saveable hierarchy of scene objects that can be instantiated multiple times, with instances maintaining a link to the original prefab. This allows you to update all instances when the prefab changes, while preserving per-instance modifications.

## Creating prefabs
You create prefabs similarly to scenes, using @b3d::Prefab::Create:

~~~~~~~~~~~~~{.cpp}
// Create a reusable enemy template
HSceneObject enemyTemplate = SceneObject::Create("Enemy");

HRenderable renderable = enemyTemplate->AddComponent<Renderable>();
renderable->SetMesh(enemyMesh);
renderable->SetMaterial(enemyMaterial);

// Add AI component
HEnemyAI ai = enemyTemplate->AddComponent<EnemyAI>();
ai->SetSpeed(5.0f);

// Create a prefab from the template
HPrefab enemyPrefab = Prefab::Create(enemyTemplate);
~~~~~~~~~~~~~

## Saving and loading prefabs
Prefabs are resources and can be saved/loaded like any other resource:

~~~~~~~~~~~~~{.cpp}
// Save the prefab
GetResources().SaveAsSinglePackage(enemyPrefab, "D:/MyGame/Prefabs/", "Enemy");

// Load the prefab
HPrefab enemyPrefab = GetResources().Load<Prefab>("D:/MyGame/Prefabs/Enemy.b3d/Enemy");
~~~~~~~~~~~~~

## Instantiating prefabs
When you instantiate a prefab, the created scene objects maintain a link to the prefab:

~~~~~~~~~~~~~{.cpp}
// Get the main scene instance (assuming you have one)
TShared<SceneInstance> sceneInstance = GetSceneManager().GetMainSceneInstance();

// Instantiate the prefab
HSceneObject enemy1 = enemyPrefab->Instantiate(sceneInstance);
enemy1->SetPosition(Vector3(10, 0, 0));

HSceneObject enemy2 = enemyPrefab->Instantiate(sceneInstance);
enemy2->SetPosition(Vector3(-10, 0, 0));

// Both enemy1 and enemy2 are linked to enemyPrefab
~~~~~~~~~~~~~

## Prefab instances and modifications
Prefab instances can have per-instance modifications that are preserved even when the prefab is updated:

~~~~~~~~~~~~~{.cpp}
// Instantiate a prefab
HSceneObject enemy = enemyPrefab->Instantiate(sceneInstance);

// Make instance-specific modifications
enemy->SetPosition(Vector3(5, 0, 0));
enemy->SetScale(Vector3(1.5f, 1.5f, 1.5f));

HEnemyAI ai = enemy->GetComponent<EnemyAI>();
ai->SetSpeed(8.0f);  // This instance is faster

// These modifications are recorded as a "prefab delta"
// If the prefab is updated, these modifications will be preserved
~~~~~~~~~~~~~

## Updating prefab instances
When you modify a prefab and save it, you can update all instances to the new version:

~~~~~~~~~~~~~{.cpp}
// Get the prefab instance root
HSceneObject enemyInstance = ...; // Some prefab instance

// Check if this is a prefab instance
if (enemyInstance->GetPrefabResourceId() != UUID::kEmpty)
{
	// Get the prefab resource
	HPrefab prefab = GetResources().Load<Prefab>(enemyInstance->GetPrefabResourceId());

	// Update this instance from the prefab
	// This will apply prefab changes while preserving instance modifications
	PrefabUtility::UpdateFromPrefab(enemyInstance);
}
~~~~~~~~~~~~~

## Breaking prefab links
Sometimes you want to break the connection between a prefab instance and its prefab:

~~~~~~~~~~~~~{.cpp}
// Break the prefab link
enemy->BreakPrefabLink();

// enemy is now a regular scene object, no longer connected to the prefab
~~~~~~~~~~~~~

## Nested prefabs
Prefabs can contain other prefabs, allowing you to build complex hierarchies:

~~~~~~~~~~~~~{.cpp}
// Create a weapon prefab
HSceneObject weaponTemplate = SceneObject::Create("Weapon");
// ... set up weapon
HPrefab weaponPrefab = Prefab::Create(weaponTemplate);
GetResources().SaveAsSinglePackage(weaponPrefab, "D:/MyGame/Prefabs/", "Weapon");

// Create an enemy prefab that uses the weapon prefab
HSceneObject enemyTemplate = SceneObject::Create("Enemy");
// ... set up enemy

// Instantiate weapon prefab as child of enemy
HPrefab loadedWeaponPrefab = GetResources().Load<Prefab>("D:/MyGame/Prefabs/Weapon.b3d/Weapon");
HSceneObject weapon = loadedWeaponPrefab->Instantiate(sceneInstance);
weapon->SetParent(enemyTemplate);

// Create enemy prefab (now contains nested weapon prefab)
HPrefab enemyPrefab = Prefab::Create(enemyTemplate);
~~~~~~~~~~~~~
