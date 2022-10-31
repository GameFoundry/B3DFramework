---
title: Saving a scene
---

Once you have populated your scene with scene objects and components you will want to save it so you can easily load it later. 

Saved scenes are represented as a @bs::Prefab object. Scene saving and loading is handled through @bs::SceneManager, which can be accessed globally through @bs::GetSceneManager().

Scene saving in particular is done by calling @bs::SceneManager::saveScene, which will return a **Prefab** representation of the current scene.

~~~~~~~~~~~~~{.cpp}
HPrefab scenePrefab = GetSceneManager().saveScene();
~~~~~~~~~~~~~

A **Prefab** is a **Resource**, and as such can then be saved or loaded as any other **Resource**.

~~~~~~~~~~~~~{.cpp}
// Save the prefab we created previously
GetResources().save(scenePrefab, "scenePrefab.asset");

// Then when ready, restore it
HPrefab loadedScenePrefab = GetResources().load<Prefab>("scenePrefab.asset");
~~~~~~~~~~~~~

Finally, when you wish to restore a scene you've loaded, you may call @bs::SceneManager::loadScene, and pass it the **Prefab** of the scene to load. The contents of the prefab will replace the currently loaded scene (if any).

~~~~~~~~~~~~~{.cpp}
GetSceneManager().loadScene(loadedScenePrefab);
~~~~~~~~~~~~~

# Resource manifest

If your scene contains components that reference resources (e.g. a **Renderable** referencing a mesh or a material) you will also need to save a resource manifest along with your scene. This is an important step as every scene will almost certainly reference some resources. The resource manifest allows the system to automatically find the referenced resources when loading the scene, even after application has been shutdown and started again. Without the manifest your scene will lose all references to any resources after attempting to load it in a new application session.

A manifest can be retrieved from @bs::Resources::getResourceManifest(). The method expect a manifest name, which will be "Default" for the default manifest (which will be the only available manifest unless you have set up your own). Resources will be registered in this manifest whenever you call **Resources::save()**. 

> You can also create your own manifests and manage them manually but that is outside the scope of this topic. See the API reference for @bs::ResourceManifest.

The manifest can then be saved by calling @bs::ResourceManifest::save(). The method expects a file path in which to save the manifest in, as well as an optional path to which to make all the resources relative to. You will want to make the manifest relative to some folder so that you can relocate both the manifest and the resources and the system is still able to find them.

~~~~~~~~~~~~~{.cpp}
SPtr<ResourceManifest> manifest = GetResources().getResourceManifest("Default");

// Save the manifest as "myManifest.asset", with the assumption that all the resources
// it references have been saved to the "C:/Data" folder
ResourceManifest::save(manifest, "C:/myManifest.asset", "C:/Data");
~~~~~~~~~~~~~

> **SPtr** is a shared pointer, used in the framework for most object instances that aren't components, scene objects or resources. You can find more about it in the [smart pointers manual](../Utilities/smartPointers).

When starting a new application session and loading a **Prefab** you will first need to restore the manifest by calling @bs::ResourceManifest::load. Note that you only need to restore the manifest once when your application starts up (usually before any scene loads).

Loaded manifest should then be registered with **Resources** by calling @bs::Resources::registerResourceManifest.

~~~~~~~~~~~~~{.cpp}
// Load the manifest. Assume that the application has moved to "C:/Program Files (x86)/MyApp".
SPtr<ResourceManifest> manifest = ResourceManifest::load("C:/Program Files (x86)/MyApp/myManifest.asset", "C:/Program Files (x86)/MyApp/Data");

// Register the manifest
GetResources().registerResourceManifest(manifest);
~~~~~~~~~~~~~



