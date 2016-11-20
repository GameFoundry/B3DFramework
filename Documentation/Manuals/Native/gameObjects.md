Game objects								{#gameObjects}
===============
[TOC]

@ref bs::GameObject "Game objects" represent the core of your game and where most of the gameplay code will be located. There are two types of game objects:
 - @ref bs::SceneObject "SceneObject" - Contains zero or multiple @ref bs::Component "components". Has a position/rotation/scale in space, and can be parented to other scene objects.
 - @ref bs::Component "Components" - Provides a set of functionality used for customizing a @ref bs::SceneObject "SceneObject". You can build your own components, and if fact this is where majority of your gameplay code will belong. A variety of built-in components are also provided.
 
# Scene objects {#gameObjects_a}
To create a @ref bs::SceneObject "SceneObject" call @ref bs::SceneObject::create "SceneObject::create" with a name of the object. The name doesn't have to be unique but is useful for identifying the object.

~~~~~~~~~~~~~{.cpp}
HSceneObject mySO = SceneObject::create("MySceneObject");
~~~~~~~~~~~~~

All @ref bs::SceneObject "SceneObjects" are organized in a hierarchy. Initially when a new @ref bs::SceneObject "SceneObject" is created it is parented to the scene root. However you can parent it to another scene object like so:

~~~~~~~~~~~~~{.cpp}
HSceneObject myParentSO = SceneObject::create("MyParentSceneObject");
HSceneObject mySO = SceneObject::create("MySceneObject");
mySO->setParent(myParentSO);
~~~~~~~~~~~~~

You can iterate over all children of a @ref bs::SceneObject "SceneObject" by calling @ref bs::SceneObject::getChild "SceneObjects::getChild"/@ref bs::SceneObject::getNumChildren "SceneObjects::getNumChildren". You can also search for a specific child by name with @ref bs::SceneObject::findChild "SceneObjects::findChild".

@ref bs::SceneObject "SceneObjects" can also be moved, rotated and scaled in the coordinate system. Use methods like @ref bs::SceneObject::setPosition "SceneObject::setPosition", @ref bs::SceneObject::setRotation "SceneObject::setRotation", @ref bs::SceneObject::setScale "SceneObject::setScale" to set these values relative to the parent object. You can also use @ref bs::SceneObject::setWorldPosition "SceneObject::setWorldPosition", @ref bs::SceneObject::setWorldRotation "SceneObject::setWorldRotation", @ref bs::SceneObject::setWorldScale "SceneObject::setWorldScale" to set them relative to the world space. A variety of helper methods like @ref bs::SceneObject::lookAt "SceneObject::lookAt" also exists. For example:

~~~~~~~~~~~~~{.cpp}
HSceneObject mySO = SceneObject::create("MySceneObject");

Vector3 position(0, 0, 50);
Vector3 lookAt(0, 0, 0);

// Object at (0, 0, 50) looking towards the origin
mySO->setPosition(position);
mySO->setLookAt(lookAt);
~~~~~~~~~~~~~

Once you are done with a scene object, call @ref bs::SceneObject::destroy "SceneObject::destroy". This will remove the scene object and all of its children out of the scene.
~~~~~~~~~~~~~{.cpp}
HSceneObject mySO = SceneObject::create("MySceneObject");
mySO->destroy()
~~~~~~~~~~~~~

Scene objects aren't very useful on their own however. Each scene object can have multiple @ref bs::Component "Components" attached.

# Components {#gameObjects_b}
Components provide logic that customizes a scene object. To attach a @ref bs::Component "Component" to a scene object use @ref bs::SceneObject::addComponent<T, Args...> "SceneObject::addComponent<T>". For example if we wanted to add the built-in @ref bs::CCamera "CCamera" component:
~~~~~~~~~~~~~{.cpp}
HSceneObject mySO = SceneObject::create("MySceneObject");
HCamera camera = mySO->addComponent<CCamera>();
~~~~~~~~~~~~~

Once a component is registered with a scene object its code will be executed as long as the scene object is active and not destroyed. Use @ref bs::SceneObject::setActive "SceneObject::setActive" to activate and deactivate a scene object. Deactive object will also deactivate all children and their components.

To search for a component on a scene object use @ref bs::SceneObject::getComponent<T> "SceneObject::getComponent<T>" like so:
~~~~~~~~~~~~~{.cpp}
HCamera camera = mySO->getComponent<CCamera>(); // Returns CCamera component if it exists on the scene object
~~~~~~~~~~~~~

If you wish to remove a component, call @ref bs::Component::destroy "Component::destroy".
~~~~~~~~~~~~~{.cpp}
HCamera camera = mySO->getComponent<CCamera>();
camera->destroy();
~~~~~~~~~~~~~

## Custom components {#gameObjects_b_a}
Using existing components is useful, but main purpose of components is for the user to implement their own gameplay code. To create your own component simply implement the @ref bs::Component "Component" interface.

The @ref bs::Component "Component" interface provides several methods which you can override with your own code:
 - @ref bs::Component::onInitialized "Component::onInitialized" - Called once when the component is first instantiated. You should use this instead of the constructor for initialization.
 - @ref bs::Component::update "Component::update" - Called every frame while the game is running and the component is enabled.
 - @ref bs::Component::onEnabled "Component::onEnabled" - Called whenever a component is enabled, or instantiated as enabled in which case it is called after @ref bs::Component::onInitialized "Component::onInitialized".
 - @ref bs::Component::onDisabled "Component::onDisabled" - Called whenever a component is disabled. This includes destruction where it is called before @ref bs::Component::onDestroyed "Component::onDestroyed". 
 - @ref bs::Component::onDestroyed "Component::onDestroyed" - Called before the component is destroyed. Destruction is usually delayed until the end of the current frame unless specified otherwise in a call to @ref bs::Component::destroy "Component::destroy". 
 - @ref bs::Component::onTransformChanged "Component::onTransformChanged" - Called when the transform of the owning scene object changes. You must subscribe to this event by calling @ref bs::Component::setNotifyFlags "Component::setNotifyFlags".
 
Since components are a part of the scene you must also make sure to implement their @ref bs::RTTITypeBase "RTTIType", so they can be saved and loaded. Read the [RTTI](@ref rtti) manual for more information. Their creation differs slightly to normal RTTI objects: when overloading @ref bs::RTTITypeBase::newRTTIObject "RTTIType::newRTTIObject" make sure to use `GameObjectRTTI::createGameObject<T>` to create the component, instead of manually creating the shared pointer. This ensures that the component handle is properly created.

# Game object handles {#gameObjects_c}
We have seen game object handles a lot in this manual, but haven't talked about them. All game objects are referenced via a @ref bs::GameObjectHandle<T> "GameObjectHandle<T>". They act similar to pointers where you can use the pointer member access "->" operator to access the underlying game object, while the normal member access "." operator can be used for operating directly on the handle data. 

You can use @ref bs::GameObjectHandle<T>::isDestroyed "GameObjectHandle<T>::isDestroyed" to check if a game object a handle is pointing to has been destroyed.

To create your own game object handle it is suggested you create a `typedef` and prefix your component name with an "H", like so:
~~~~~~~~~~~~~{.cpp}
class MyComponent : public Component
{
 ...
};

typedef GameObjectHandle<MyComponent> HMyComponent;
~~~~~~~~~~~~~

# Prefabs {#gameObjects_d}
@ref bs::Prefab "Prefabs" allow you to store groups of scene objects (and their components) and then re-use them. A prefab can be an entire scene or just a small subset of scene objects.

To create one call @ref bs::Prefab::create "Prefab::create" with a @ref bs::SceneObject "SceneObject" you want to create a prefab of. The object, all of its children and all of the components will be stored in the prefab. You can then save that prefab to disk like any other @ref bs::Resource "Resource" (see the [resources](@ref resources) manual), and load it later.

@ref bs::Prefab "Prefabs" can be instantiated back into @ref bs::SceneObject "SceneObjects" by calling @ref bs::Prefab::instantiate "Prefab::instantiate".

For example:
~~~~~~~~~~~~~{.cpp}
HSceneObject mySO = SceneObject::create("MySceneObject");
... create a more complex hierarchy with components here ...
HPrefab myPrefab = Prefab::create(mySO);
... potentially save the prefab for later use ...
// Or use it right away
HSceneObject clonedSO = myPrefab->instantiate();
~~~~~~~~~~~~~

Prefabs are used for more than just cloning. Any scene objects that are instantiated from a @ref bs::Prefab "Prefabs" (including the original scene object that the prefab was created from) become permanently linked to that scene object hierarchy. We call such scene objects "prefab instances". You can use @ref bs::SceneObject::getPrefabParent "SceneObject::getPrefabParent" to find a @ref bs::Prefab "Prefab" that a scene object might be an instance of.

This can be useful if you have many prefab instances of a particular prefab (e.g. buildings or trees in a level). You can quickly apply changes to all scene objects linked to a particular prefab. Simply make your modifications on an existing prefab instance, or create a new scene object hierarchy. Then call @ref bs::Prefab::update "Prefab::update" with the new data. This will replace contents of the prefab with the new hierarchy. After that you can use @ref bs::PrefabUtility::updateFromPrefab "PrefabUtility::updateFromPrefab" on all prefab instances and they will replace their contents with the new contents of the prefab.

You are also allowed to make changes to prefab instances. @ref bs::PrefabUtility::updateFromPrefab "PrefabUtility::updateFromPrefab" will try its best to keep the changes made, while also updating the underlying hierarchy. Obviously this is not possible if the hierarchy fully changes, but in most cases there will only be minor changes.

In case you made some changes to a prefab instance you can revert back to original prefab state by calling @ref bs::PrefabUtility::revertToPrefab "PrefabUtility::revertToPrefab". If you wish to break a link of a scene object with a prefab, call @ref bs::SceneObject::breakPrefabLink "PrefabUtility::breakPrefabLink".