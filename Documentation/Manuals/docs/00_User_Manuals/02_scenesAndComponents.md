---
title: Scene objects and components
---

All scenes in the framework are constructed from scene objects. Each scene object is a part of the scene hierarchy, and can have a parent scene object and zero or multiple child scene objects. Each scene object can also be positioned, oriented and scaled within the scene.

Components can be attached to scene objects - each scene object can have zero or multiple components attached to it. Components provide various functionality and contain the logic for your game. For example there is a **Camera** component that lets the user see into the scene, or a **Renderable** component that represents a single 3D mesh in the scene.

Once your scene has been set up the engine takes care of everything else, like rendering your meshes. You may also create your own components to put custom game logic in, receive events from the game, process input and manipulate the scene after initial set up. We will go into detail on how to create custom components later.

Let's see an example where we add a single scene object to the scene, position it, and attach a **Renderable** component to it. For now you do not need to know exactly how **Renderable** component works, as we will explain that later. Focus rather on how a scene object is set up and how components are added to it.

~~~~~~~~~~~~~{.cpp}
// Create a brand new scene object named "My Object". It is placed at the root
// of the scene hierarchy, at position (0, 0, 0)
HSceneObject sceneObject = SceneObject::Create("My Object");

// Change position of the object
sceneObject->SetPosition(Vector3(0, 30, 0));

// Add a Renderable component to the scene object
HRenderable renderable = sceneObject->AddComponent<Renderable>();
~~~~~~~~~~~~~

> As a convention, almost all complex classes in the framework use the static **Create** method as a way to create new objects. More simple classes and structures, like **Vector3**, use the traditional constructors instead.

# Handles

Whenever you wish to keep a reference to a scene object or a component you do so via a handle. They are represented with classes prefixed with an "H", as you might have noticed in the example above.

Scene objects are always referenced using the **HSceneObject** handle, while components have handles named with an "H" prefix, followed by the component name (e.g. **HRenderable** for the **Renderable** component, **HCamera** for the **Camera** component).

You may treat the handles as pointers, using "->" to access their members, comparing them for equality or with *nullptr* to check their validity.

~~~~~~~~~~~~~{.cpp}
HSceneObject sceneObject = SceneObject::Create("My Object");

// Check if handle is valid
if (sceneObject.IsValid())
{
    // Access members using ->
    sceneObject->SetPosition(Vector3(0, 30, 0));
}

// Compare handles
HSceneObject anotherObject = SceneObject::Create("Another Object");
if (sceneObject == anotherObject)
{
    // Same object
}
~~~~~~~~~~~~~

# Scene object creation and destruction

We have already shown how to use @b3d::SceneObject::Create to create a new scene object.

If you wish to destroy a scene object call @b3d::SceneObject::Destroy. Note that destroying a scene object will destroy all of the components attached to it, as well as any child scene objects.

~~~~~~~~~~~~~{.cpp}
// Create a scene object
HSceneObject sceneObject = SceneObject::Create("My Object");

// Destroy the scene object
sceneObject->Destroy();
~~~~~~~~~~~~~

By default, object destruction is delayed until the end of the current frame. If you need immediate destruction, you can pass `true` as a parameter:

~~~~~~~~~~~~~{.cpp}
// Destroy immediately
sceneObject->Destroy(true);
~~~~~~~~~~~~~

Handles provide safety when referencing game objects. If the referenced object is destroyed, the handle automatically becomes invalid and you can check this using the @b3d::GameObjectHandle::IsDestroyed method or by comparing with nullptr.

~~~~~~~~~~~~~{.cpp}
HSceneObject sceneObject = SceneObject::Create("My Object");
sceneObject->Destroy();

// Check if the object has been destroyed
if (!sceneObject.IsValid())
{
    // Object is no longer valid
}
~~~~~~~~~~~~~

# Transforming scene objects

You can change scene object position, orientation and scale using @b3d::SceneObject::SetPosition, @b3d::SceneObject::SetRotation and @b3d::SceneObject::SetScale.

Components attached to scene objects will reflect the scene object transform. For example, moving a scene object with a **Renderable** component will make the 3D mesh referenced by **Renderable** display in a different location in the scene.

~~~~~~~~~~~~~{.cpp}
HSceneObject sceneObject = SceneObject::Create("My Object");

// Move the object to 30 units on the X axis
sceneObject->SetPosition(Vector3(30, 0, 0));

// Rotate 90 degrees around the Y axis
sceneObject->SetRotation(Quaternion(Degree(0), Degree(90), Degree(0)));

// Double its size
sceneObject->SetScale(Vector3(2.0f, 2.0f, 2.0f));
~~~~~~~~~~~~~

Internally these methods manipulate a @b3d::Transform object. You can also retrieve the transform from a scene object and manipulate it directly for greater control. To retrieve the world-space transform call @b3d::SceneObject::GetTransform.

There are also other useful methods when it comes to dealing with scene object positions and orientations, like @b3d::SceneObject::Move, @b3d::SceneObject::LookAt, @b3d::SceneObject::Rotate. See the @b3d::SceneObject API reference for a full overview.

~~~~~~~~~~~~~{.cpp}
// Move the object relative to its current position
sceneObject->Move(Vector3(10, 0, 0));

// Make the object look at a specific point
sceneObject->LookAt(Vector3(100, 0, 100));

// Rotate the object relative to its current rotation
sceneObject->Rotate(Vector3(0, Degree(45), 0));
~~~~~~~~~~~~~

# Scene object hierarchy

As mentioned, scene objects can be arranged in a hierarchy. Hierarchies allow you to transform multiple scene objects at once, since any transforms applied to a parent will also be applied to a child.

All newly created scene objects are parented to the scene root by default. Use @b3d::SceneObject::SetParent to change their parents.

~~~~~~~~~~~~~{.cpp}
HSceneObject parent = SceneObject::Create("Parent");
HSceneObject childA = SceneObject::Create("Child A");
HSceneObject childB = SceneObject::Create("Child B");

// Scene hierarchy:
// Scene root
//  - Parent
//  - Child A
//  - Child B

childA->SetParent(parent);
childB->SetParent(parent);

// Scene hierarchy:
// Scene root
//  - Parent
//    - Child A
//    - Child B

// Transforming an object will move all its children
// This operation moves the parent and both children to 30 units on the X axis
parent->SetPosition(Vector3(30, 0, 0));
~~~~~~~~~~~~~

You may query for parent and children of a scene object using methods like @b3d::SceneObject::GetParent, @b3d::SceneObject::GetChildCount, @b3d::SceneObject::GetChild or @b3d::SceneObject::FindChild. See the @b3d::SceneObject API reference for a full overview.

~~~~~~~~~~~~~{.cpp}
// Get the parent of a scene object
HSceneObject parent = childA->GetParent();

// Get number of children
u32 childCount = parent->GetChildCount();

// Get a child by index
HSceneObject firstChild = parent->GetChild(0);

// Find a child by name
HSceneObject namedChild = parent->FindChild("Child A");
~~~~~~~~~~~~~

# Components

You may add components to a scene object using the @b3d::SceneObject::AddComponent<T> method.

You may retrieve existing components by calling @b3d::SceneObject::GetComponent<T>.

Components can be removed by calling the @b3d::Component::Destroy method on the component.

~~~~~~~~~~~~~{.cpp}
HSceneObject sceneObject = SceneObject::Create("My Object");

// Add a Renderable component to the scene object
HRenderable renderable = sceneObject->AddComponent<Renderable>();

// Find an existing component
HRenderable existingRenderable = sceneObject->GetComponent<Renderable>();

// Destroy the component
renderable->Destroy();
~~~~~~~~~~~~~

## Component lifecycle

Components have several lifecycle methods that are called at different points:
- **OnCreated()** - Called when the component is first created
- **OnBeginPlay()** - Called when the scene starts playing (scene enters Running state)
- **Update()** - Called every frame while the component is active
- **FixedUpdate()** - Called at fixed time intervals (useful for physics)
- **OnEnabled()** - Called when the component is enabled
- **OnDisabled()** - Called when the component is disabled
- **OnDestroyed()** - Called just before the component is destroyed

~~~~~~~~~~~~~{.cpp}
class MyComponent : public Component
{
public:
    void OnCreated() override
    {
        // Component has just been created
    }

    void OnBeginPlay() override
    {
        // Scene has started running
    }

    void Update() override
    {
        // Called every frame
    }
};
~~~~~~~~~~~~~

# Enabling and disabling components

Components can be enabled or disabled using @b3d::Component::SetEnabled. Disabled components do not receive Update() or FixedUpdate() calls.

~~~~~~~~~~~~~{.cpp}
HSceneObject sceneObject = SceneObject::Create("My Object");
HRenderable renderable = sceneObject->AddComponent<Renderable>();

// Disable the component
renderable->SetEnabled(false);

// Check if component is enabled
bool isEnabled = renderable->GetEnabled();

// Re-enable the component
renderable->SetEnabled(true);
~~~~~~~~~~~~~

When a component is disabled:
- Update() and FixedUpdate() methods are not called
- OnDisabled() is called when the component becomes disabled
- OnEnabled() is called when the component is re-enabled

You can also check if a component is enabled including its parent hierarchy:

~~~~~~~~~~~~~{.cpp}
// Check if component itself is enabled (ignoring parent state)
bool selfEnabled = renderable->GetEnabled(true);

// Check if component is enabled and all parents are active (default)
bool fullyEnabled = renderable->GetEnabled();
~~~~~~~~~~~~~

# Scenes and Scene Instances

The framework supports organizing your scene objects into reusable **Scene** resources and **SceneInstance** objects.

## Scenes

A **Scene** is a resource that stores a hierarchy of scene objects that can be saved to disk and instantiated multiple times. Think of it as a template for creating scene instances.

~~~~~~~~~~~~~{.cpp}
// Create a scene from an existing scene object hierarchy
HSceneObject root = SceneObject::Create("SceneRoot");
// ... set up the hierarchy ...

HScene scene = Scene::Create(root);

// Save the scene to disk
Resources::Save(scene, "MyScene.asset");

// Load a scene from disk
HScene loadedScene = Resources::Load<Scene>("MyScene.asset");

// Instantiate the scene (creates a live copy in the world)
SPtr<SceneInstance> instance = loadedScene->Instantiate();
~~~~~~~~~~~~~

## Scene Instances

A **SceneInstance** represents an active, running scene in your game. It manages all the scene objects, components, and their updates.

~~~~~~~~~~~~~{.cpp}
// Create a new empty scene instance
SPtr<SceneInstance> sceneInstance = SceneInstance::Create("MyScene");

// Create scene objects within the scene instance
HSceneObject sceneObject = sceneInstance->CreateSceneObject("MyObject");

// Get the root object of the scene instance
HSceneObject root = sceneInstance->GetRoot();

// Access scene instance properties
String name = sceneInstance->GetName();
bool isActive = sceneInstance->IsActive();
~~~~~~~~~~~~~

For more advanced scene management topics including component states, finding components, scene clearing, time management, and scene object flags, see the [Advanced Scene Management](06_Gameplay/03_advancedScenes.md) manual.
