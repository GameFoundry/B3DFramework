Prefabs				{#prefabs}
===============

We talked about prefabs when discussing on how to save a game scene, but they can actually be used for saving of any group of scene objects. Contents of such prefabs can then be easily instantiated throughout the scene, allowing you to build complex scenes more easily. Prefabs can be nested within other prefabs with no limits (where the top level prefab is the "scene") and provide other useful functionality we'll talk about.

For example you might create a scene object that contains a **Renderable** displaying a mesh of a house a material referencing the relevant textures. The scene object could also contain physics objects like a **Collider** and even child scene objects. You could then group all the scene objects and their components into a single prefab, which you can then easily re-use all over your scene.

# Creation
To create such a prefab call @ref bs::Prefab::create "Prefab::create()" with the first parameter being a **SceneObject** from which to create the prefab from. All child scene objects will be included in the prefab as well. Second parameter of **Prefab::create()** should equal to false, indicating this prefab doesn't represent a scene.

~~~~~~~~~~~~~{.cpp}
// Create a renderable object as normal
HSceneObject renderableSO = SceneObject::create("3D object");
HRenderable renderable = renderableSO->addComponent<CRenderable>();
// Set up renderable mesh & material

HPrefab renderablePrefab = Prefab::create(renderableSO, false);
~~~~~~~~~~~~~

# Instantiation
Once created you can now instantiate the prefab as many times as you wish by calling @ref bs::Prefab::instantiate "Prefab::instantiate()". This will return a scene object, which will by default be parented to scene root.

~~~~~~~~~~~~~{.cpp}
HSceneObject instance1 = renderablePrefab->instantiate();
HSceneObject instance2 = renderablePrefab->instantiate();
~~~~~~~~~~~~~

# Updates
Often you might want to modify the contents of a prefab. To do that call @ref bs::Prefab::update "Prefab::update()", preferably with the same scene object it was created with.

~~~~~~~~~~~~~{.cpp}
// Update mesh on our renderable prefab
renderable->setMesh(differentMesh);

// Save the updated data into the prefab
renderablePrefab->update(renderableSO);
~~~~~~~~~~~~~

Once a prefab has been updated, you might also want to update any instances of that prefab. A prefab instance is any scene object resulting from **Prefab::instantiate()**, and the scene object used for originally creating the prefab.

You can update the instances by calling @ref bs::PrefabUtility::updateFromPrefab() "PrefabUtility::updateFromPrefab()".

~~~~~~~~~~~~~{.cpp}
// Update all the instances of our prefab with new prefab data (renderableSO is also an instance, but we don't update it since it was the source of the changes, so there's no need)

PrefabUtility::updateFromPrefab(instance1);
PrefabUtility::updateFromPrefab(instance2);
~~~~~~~~~~~~~

Note that this process is recursive, so you can call **PrefabUtility::updateFromPrefab()** on the entire scene, in order to update all prefabs.

~~~~~~~~~~~~~{.cpp}
PrefabUtility::updateFromPrefab(gSceneManager().getRootNode());
~~~~~~~~~~~~~

## Links
You'll notice we didn't need to provide a reference to **Prefab** when performing prefab updates. This is because every prefab instance is linked to its prefab. You can break the link by calling @ref bs::SceneObject::breakPrefabLink "SceneObject::breakPrefabLink()". This will make it into a regular scene object and it will no longer be updated from the prefab.

~~~~~~~~~~~~~{.cpp}
instance1->breakPrefabLink();
~~~~~~~~~~~~~

## Saving
If a scene using prefabs is saved to disk, and if you plan on updating the prefab later, you must save the prefabs same as we save scenes. Otherwise the next time you call **PrefabUtility::updateFromPrefab()** the system will be unable to find the prefab.

~~~~~~~~~~~~~{.cpp}
gResources().save(renderablePrefab, "myPrefab.asset");
~~~~~~~~~~~~~

> Prefabs generally won't be updated during normal application runs, and therefore non-scene prefabs don't need to be distributed with your application or used outside of development.

# Instance modifications
All prefab instances are by default identical, and if you make any changes to the instances any calls to **PrefabUtility::updateFromPrefab()** will discard those changes and update the instance with latest data from the prefab. You can choose to break the prefab link by calling **SceneObject::breakPrefabLink()** but in that case you can no longer update the scene object if the source prefab changes.

The solution for this problem are instance modifications. Using this system you can modify prefab instance same as a normal scene object, as long as you call @ref bs::PrefabUtility::recordPrefabDiff "PrefabUtility::recordPrefabDiff()" after.

~~~~~~~~~~~~~{.cpp}
// Change the material of the prefab instance's renderable
HRenderable instanceRenderable = instance2->getComponent<CRenderable>();
instanceRenderable->setMaterial(0, differentMaterial);

PrefabUtility::recordPrefabDiff(instanceRenderable);
// Now it's safe to call PrefabUtility::updateFromPrefab() and our material change will remain even after update
~~~~~~~~~~~~~

Note the system will automatically call **PrefabUtility::recordPrefabDiff()** when a new prefab is created or updated. Meaning this will also be done automatically when you're saving scenes, and therefore there should be rare need to call it manually. 

## Reverting
Sometimes you might want to discard all instance modifications, and revert back to original data from the prefab. In that case you can call @ref bs::PrefabUtility::revertToPrefab "PrefabUtility::revertToPrefab()".

~~~~~~~~~~~~~{.cpp}
// Discard any instance specific changes and update from the latest data from the prefab
PrefabUtility::revertToPrefab(instance2);
~~~~~~~~~~~~~