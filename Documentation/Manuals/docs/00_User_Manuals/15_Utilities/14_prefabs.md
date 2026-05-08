---
title: Prefabs
---
We talked about prefabs when discussing on how to save a game scene, but they can actually be used for saving of any group of scene objects. Contents of such prefabs can then be easily instantiated throughout the scene, allowing you to build complex scenes more easily. 

For example you might create a scene object that contains a **Renderable** displaying a mesh of a house a material referencing the relevant textures. The scene object could also contain physics objects like a **Collider** and even child scene objects. You could then group all the scene objects and their components into a single prefab, which you can then easily re-use all over your scene.

# Creation
To create such a prefab call @b3d::Prefab::Create with a **SceneObject** from which to create the prefab from. All child scene objects will be included in the prefab as well.

~~~~~~~~~~~~~{.cpp}
// Create a renderable object as normal
HSceneObject renderableSO = SceneObject::Create("3D object");
HRenderable renderable = renderableSO->AddComponent<Renderable>();
// Set up renderable mesh & material

HPrefab renderablePrefab = Prefab::Create(renderableSO);
~~~~~~~~~~~~~

# Instantiation
Once created you can now instantiate the prefab as many times as you wish by calling @b3d::Prefab::Instantiate. This will return a scene object, which will be parented to the provided scene instance.

~~~~~~~~~~~~~{.cpp}
TShared<SceneInstance> sceneInstance = GetSceneManager().GetMainScene();
HSceneObject instance1 = renderablePrefab->Instantiate(sceneInstance);
HSceneObject instance2 = renderablePrefab->Instantiate(sceneInstance);
~~~~~~~~~~~~~

# Updates
Often you might want to modify the contents of a prefab. To do that call @b3d::PrefabUtility::UpdatePrefab with the prefab to update and the scene object it was created with.

~~~~~~~~~~~~~{.cpp}
// Update mesh on our renderable prefab
renderable->SetMesh(differentMesh);

// Save the updated data into the prefab
PrefabUtility::UpdatePrefab(renderablePrefab, renderableSO);
~~~~~~~~~~~~~

Once a prefab has been updated, any instances of that prefab will be automatically updated when the prefab system detects version changes. This happens automatically when nested prefab instances are scanned.

## Links
Every prefab instance is linked to its prefab through internal IDs. You can break the link by calling @b3d::SceneObject::BreakPrefabLink. This will make it into a regular scene object and it will no longer be updated from the prefab.

~~~~~~~~~~~~~{.cpp}
instance1->BreakPrefabLink();
~~~~~~~~~~~~~

## Saving
If a scene using prefabs is saved to disk, and if you plan on updating the prefab later, you must save the prefabs same as we save scenes. Otherwise the next time the system tries to update instances the system will be unable to find the prefab.

~~~~~~~~~~~~~{.cpp}
GetResources().Save(renderablePrefab, "myPrefab.asset");
~~~~~~~~~~~~~

> Prefabs generally won't be updated during normal application runs, and therefore non-scene prefabs don't need to be distributed with your application or used outside of development.

# Instance modifications
All prefab instances are by default identical, and if you make any changes to the instances, the prefab system preserves these changes as deltas. The system automatically handles instance-specific modifications through its delta system.

~~~~~~~~~~~~~{.cpp}
// Change the material of the prefab instance's renderable
HRenderable instanceRenderable = instance2->GetComponent<Renderable>();
instanceRenderable->SetMaterial(0, differentMaterial);

// The system automatically tracks this change as a delta
// No manual delta recording is needed in the current API
~~~~~~~~~~~~~

## Reverting
Sometimes you might want to discard all instance modifications, and revert back to original data from the prefab. In that case you can call @b3d::PrefabUtility::RevertToPrefab.

~~~~~~~~~~~~~{.cpp}
// Discard any instance specific changes and update from the latest data from the prefab
PrefabUtility::RevertToPrefab(instance2);
~~~~~~~~~~~~~
