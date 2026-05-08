---
title: Saving a scene
---

Once you have populated your scene with scene objects and components you will want to save it so you can easily load it later.

Scenes can be saved as @b3d::Scene resources. The framework provides simple methods for quickly saving and loading complete scene hierarchies.

# Saving a scene

To save a scene, create a @b3d::Scene resource from your scene object hierarchy using @b3d::Scene::Create:

~~~~~~~~~~~~~{.cpp}
// Assuming you have built a scene hierarchy
HSceneObject root = SceneObject::Create("SceneRoot");
// ... add various scene objects and components ...

// Create a scene from the hierarchy
HScene scene = Scene::Create(root);
~~~~~~~~~~~~~

A **Scene** is a **Resource**, and as such can be saved like any other resource:

~~~~~~~~~~~~~{.cpp}
// Save the scene to a package
GetResources().SaveAsSinglePackage(scene, "D:/MyGame/Scenes/", "MainLevel");
// This creates D:/MyGame/Scenes/MainLevel.b3d

// Or save to an existing package or file location
GetResources().Save(scene, "D:/MyGame/Scenes/MainLevel.asset");
~~~~~~~~~~~~~

# Loading a scene

When you wish to restore a scene you've saved, load the scene resource and instantiate it:

~~~~~~~~~~~~~{.cpp}
// Load the scene resource
HScene scene = GetResources().Load<Scene>("D:/MyGame/Scenes/MainLevel.b3d/MainLevel");

// Instantiate the scene into the world
TShared<SceneInstance> sceneInstance = scene->Instantiate();

// The instantiated hierarchy is now active and will start updating
~~~~~~~~~~~~~

The @b3d::SceneInstance manages the lifetime of all scene objects created from the scene.

# Resource references
If your scene contains components that reference resources (e.g. a **Renderable** referencing a mesh or material), those resources must be saved separately and be loadable when the scene is loaded. The framework automatically handles resource references through the resource system.

~~~~~~~~~~~~~{.cpp}
// Save your resources first
GetResources().Save(mesh, "PlayerMesh.asset");
GetResources().Save(material, "PlayerMaterial.asset");

// Then save your scene - it will automatically reference the saved resources
HScene scene = Scene::Create(sceneRoot);
GetResources().Save(scene, "GameLevel.asset");

// When loading, the scene will automatically load the referenced resources
HScene loadedScene = GetResources().Load<Scene>("GameLevel.asset");
~~~~~~~~~~~~~

For more detailed information about scenes, scene instances, and prefabs, see the [Scenes and prefabs](../06_Gameplay/02_scenesAndPrefabs.md) manual.
