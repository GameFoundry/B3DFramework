---
title: Scene objects and components
---

All scenes in bs::f are constructed from scene objects. Each scene object is a part of the scene hierarchy, and can have a parent scene object and zero or multiple child scene objects. Each scene object can also be positioned, oriented and scaled within the scene.

Components can be attached to scene objects - each scene object can have zero or multiple components attached to it. Components provide various functionality and contain the logic for your game. For example there is a **Camera** component that lets the user see into the scene, or a **Renderable** component that represents a single 3D mesh in the scene. 

Once your scene has been set up the engine takes care of everything else, like rendering your meshes. You may also create your own components to put custom game logic in, receive events from the game, process input and manipulate the scene after initial set up. We will go into detail on how to create custom components later.

Let see an example where we add a single scene object to the scene, position it, and attach a **Renderable** component to it. For now you do not need to know exactly how **Renderable** component works, as we will explain that later. Focus rather on how a scene object is set up and and how components are added to it.

~~~~~~~~~~~~~{.cpp}
// Create a brand new scene object named My Object. It is placed at the root 
// of the scene hierarchy, at position (0, 0, 0)
HSceneObject so = SceneObject::create("My object");

// Change position of the object
so->setPosition(Vector3(0, 30, 0));

// Add a Renderable component to the scene object
HRenderable renderable = so->addComponent<CRenderable>();
~~~~~~~~~~~~~

> As a convention, almost all complex classes in bs::f use the static **create** method as a way to create new objects. More simple classes and structures, like **Vector3**, use the traditional constructors instead.

# Handles
Whenever you wish to keep a reference to a scene object or a component you do so via a handle. They are represented with classes prefixed with an "H", as you might have noticed in the example above. 

Scene objects are always referenced using the **HSceneObject** handle, while components have handles named with an "H" prefix, followed by the component name (e.g. **HRenderable** for the **Renderable** component).

You may treat the handles as pointers, using "->" to access their members, comparing them for equality or with *nullptr* to check their validity. 

# Scene object creation and destruction
We have already shown how to use @bs::SceneObject::create to create a new scene object. 

If you wish to destroy a scene object call @bs::SceneObject::destroy. Note that destroying a scene object will destroy all of the components attached to it, as well as any child scene objects.

~~~~~~~~~~~~~{.cpp}
// Create a scene object
HSceneObject so = SceneObject::create("My object");

// Destroy the scene object
so->destroy();
~~~~~~~~~~~~~

# Transforming scene objects
You can change scene object position, orientation and scale using @bs::SceneObject::setPosition, @bs::SceneObject::setRotation and @bs::SceneObject::setScale.

Components attached to scene objects will reflect the scene object transform. For example, moving a scene object with a **Renderable** component will make the 3D mesh referenced by **Renderable** display in a different location in the scene.

~~~~~~~~~~~~~{.cpp}
HSceneObject so = SceneObject::create("My object");

// Move the object to 30 units on the X axis
so->setPosition(Vector3(30, 0, 0));

// Rotate 90 degrees around the Y axis
so->setRotation(Quaternion(Degree(0), Degree(90), Degree(0)));

// Double its size
so->setScale(Vector3(2.0f, 2.0f, 2.0f));
~~~~~~~~~~~~~

Internally these methods manipulate a @bs::Transform object. You can also retrieve the transform from a scene object and manipulate it directly for greater control. To retrieve the world-space transform call @bs::SceneObject::getTransform().

There also other useful methods when it comes to dealing with scene object positions and orientations, like @bs::SceneObject::move, @bs::SceneObject::lookAt. See the @bs::SceneObject API reference for a full overview.

# Scene object hierarchy
As mentioned, scene objects can be arranged in a hierarchy. Hierarchies allow you to transform multiple scene objects at once, since any transforms applied to a parent will also be applied to a child.

All newly created scene objects are parented to the scene root by default. Use @bs::SceneObject::setParent to change their parents.

~~~~~~~~~~~~~{.cpp}
HSceneObject parent = SceneObject::create("Parent");
HSceneObject childA = SceneObject::create("Child");
HSceneObject childB = SceneObject::create("Child");

// Scene hierarchy:
// Scene root
//  - Parent
//  - Child
//  - Child 

childA->setParent(parent);
childB->setParent(parent);

// Scene hierarchy:
// Scene root
//  - Parent
//   - Child
//   - Child 


// Transforming an object will move all its children
// This operation moves the parent and both children to 30 units on the X axis
parent->setPosition(Vector3(30, 0, 0));
~~~~~~~~~~~~~

You may query for parent and children of a scene object using methods like @bs::SceneObject::getParent, @bs::SceneObject::getNumChildren, @bs::SceneObject::getChild or @bs::SceneObject::findChild. See the @bs::SceneObject API reference for a full overview.

# Components
You may add components to a scene object using the @ref bs::SceneObject::addComponent<T, Args...> "SceneObject::addComponent<T>()" method. 

You may retrieve existing components by calling @bs::SceneObject::getComponent<T>.

Components can be removed by calling the @bs::Component::destroy method on the component.

~~~~~~~~~~~~~{.cpp}
HSceneObject so = SceneObject::create("My object");

// Add a Renderable component to the scene object
so->addComponent<CRenderable>();

// Find an existing component
HRenderable renderable = so->getComponent<CRenderable>();

// Destroy the component
renderable->destroy();
~~~~~~~~~~~~~

> As a convention, all component class names are prefixed with a "C".