Saving a scene						{#savingScene}
===============
[TOC]

Once you have populated your scene with scene objects and components you will want to save it so you can easily load it later. The entire scene, as well as parts of the scene can be saved by creating a @ref bs::Prefab "Prefab".

# Creating prefabs {#savingScene_a}

A prefab can be created by calling @ref bs::Prefab::create "Prefab::create()" method and providing the relevant **SceneObject**. You can retrieve the root **SceneObject** of the current scene by calling @ref bs::SceneManager::getRootNode() "SceneManager::getRootNode()", accessible through @ref bs::gSceneManager() "gSceneManager()". The second parameter of **Prefab::create()** controls whether the prefab represents an entire scene, or just a subset of scene objects.

~~~~~~~~~~~~~{.cpp}
// Get scene root
HSceneObject sceneRoot = gSceneManager().getRootNode();

// Create a prefab of some sub-object
HSceneObject subObject = sceneRoot->findPath("Path/To/Some/Object");
HPrefab partialPrefab = Prefab::create(subObject, false);

// Create a prefab of the entire scene
HPrefab scenePrefab = Prefab::create(sceneRoot, true);
~~~~~~~~~~~~~

# Saving & loading prefabs {#savingScene_b}

Once a prefab has been created it can be saved and loaded as any other **Resource**.

~~~~~~~~~~~~~{.cpp}
// Save the prefabs we created previously
gResources().save(partialPrefab, "partialPrefab.asset");
gResources().save(scenePrefab, "scenePrefab.asset");

// Then when ready, restore them
HPrefab loadedPartialPrefab = gResources().load<Prefab>("partialPrefab.asset");
HPrefab loadedScenePrefab = gResources().load<Prefab>("scenePrefab.asset");
~~~~~~~~~~~~~

# Instantiating prefabs {#savingScene_c}

After loading the prefab must be instantiated, creating a representation of the **SceneObject** hierarchy it contains. This is done by calling @ref bs::Prefab::instantiate() "Prefab::instantiate()" which returns a **SceneObject**. By default this scene object will be parented to the current scene root, but can then be manipulated as any other scene object. You can replace the current scene with a new **SceneObject** root by calling @ref bs::SceneManager::setRootNode() "SceneManager::setRootNode()".

~~~~~~~~~~~~~{.cpp}
// Instatiate the scene prefab and replace the current scene
HSceneObject newSceneHierarchy = loadedScenePrefab->instantiate();
gSceneManager().setRootNode(newSceneHierarchy);
~~~~~~~~~~~~~

Since prefabs can also be created from arbitrary sub-hierarcies, you can use them to create groups of scene objects and components that are commonly used together, and then re-use them throughout the scene.

~~~~~~~~~~~~~{.cpp}
// Make a couple of copies of the prefab and place them in different parts of the scene
HSceneObject subObject1 = loadedPartialPrefab->instantiate();
HSceneObject subObject2 = loadedPartialPrefab->instantiate();

subObject1->setPosition(Vector3(10.0f, 0.0f, 0.0f));
subObject1->setPosition(Vector3(50.0f, 0.0f, 0.0f));
~~~~~~~~~~~~~

# Resource manifest {#savingScene_d}

If your scene contains components that reference resources (e.g. a **Renderable** referencing a mesh or a material) you will also need to save a resource manifest along your scene. This is an important step as every scene will almost certainly reference some resources. The resource manifest allows the system to automatically find the referenced resources when loading the scene, even after application has been shutdown and started again. Without the manifest your scene will lose all references to any resources after attempting to load it in a new application session.

A manifest can be retrieved from @ref bs::Resources::getResourceManifest() "Resources::getResourceManifest()". The method expect a manifest name, which will be "Default" for the default manifest. Resources will be registered in this manifest whenever you call **Resources::save()**. 

> You can also create your own manifests and manage them manually but that is outside the scope of this topic. See the API reference for @ref bs::ResourceManifest "ResourceManifest".

The manifest can then be saved by calling @ref bs::ResourceManifest::save() "ResourceManifest::save()". The method expects a file path in which to save the manifest in, as well as an optional path to which to make all the resources relative to. You will want to make the manifest relative to some folder so that you can relocate both the manifest and the resources and the system is still able to find them.

~~~~~~~~~~~~~{.cpp}
SPtr<ResourceManifest> manifest = gResources().getResourceManifest("Default");

// Save the manifest as "myManifest.asset", with the assumption that all the resources
// it references have been saved to the "C:/Data" folder
ResourceManifest::save(manifest, "C:/myManifest.asset", "C:/Data");
~~~~~~~~~~~~~

> **SPtr** is a shared pointer, used in bs::f for most object instances that aren't components, scene objects or resources. It is covered later in the [smart pointers manual](@ref smartPointers).

Before loading a **Prefab** you will need to restore the manifest by calling @ref bs::ResourceManifest::load "ResourceManifest::load()". Note that you only need to restore the manifest once when your application starts up (usually before any other resource loads).

Loaded manifest should then be registered with **Resources** by calling @ref bs::Resources::registerResourceManifest "Resources::registerResourceManifest()".

~~~~~~~~~~~~~{.cpp}
// Load the manifest. Assume that the application has moved to "C:/Program Files (x86)/MyApp".
SPtr<ResourceManifest> manifest = ResourceManifest::load("C:/Program Files (x86)/MyApp/myManifest.asset", "C:/Program Files (x86)/MyApp/Data");

// Register the manifest
gResources().registerResourceManifest(manifest);
~~~~~~~~~~~~~



