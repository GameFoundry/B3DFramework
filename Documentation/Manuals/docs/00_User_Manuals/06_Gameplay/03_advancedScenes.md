---
title: Advanced scene management
---

This manual covers advanced topics related to scene and component management, including component states, scene instances, and scene object flags.

# Component States

Scene instances control the state of all components within them. Components can be in one of three states:

- **Running** - All component callbacks (Update, FixedUpdate, etc.) are triggered
- **Paused** - All callbacks except Update/FixedUpdate are triggered
- **Stopped** - No callbacks are triggered (except OnCreated/OnDestroyed)

~~~~~~~~~~~~~{.cpp}
TShared<SceneInstance> sceneInstance = SceneInstance::Create("MyScene");

// Change the component state for all components in the scene
sceneInstance->SetComponentState(ComponentState::Paused);

// Check if components are running
bool isRunning = sceneInstance->IsRunning();
~~~~~~~~~~~~~

Components can override the global state using the **ComponentFlag::AlwaysRun** flag, which ensures they always run regardless of the scene state. This flag must be set in the component's constructor:

~~~~~~~~~~~~~{.cpp}
class MyAlwaysRunningComponent : public Component
{
public:
    MyAlwaysRunningComponent(const HSceneObject& parent)
        : Component(parent)
    {
        SetFlags(ComponentFlag::AlwaysRun);
    }

    void Update() override
    {
        // This will be called even when the scene is paused or stopped
    }
};
~~~~~~~~~~~~~

Note that if a component's parent SceneObject is inactive (via SetActive(false)), the component will be in Stopped state regardless of the AlwaysRun flag or scene state.

# Finding components in a scene

You can find all components of a specific type in a scene instance using @b3d::SceneInstance::FindComponents:

~~~~~~~~~~~~~{.cpp}
TShared<SceneInstance> sceneInstance = SceneInstance::Create("MyScene");

// Find all Camera components in the scene
Vector<HCamera> cameras = sceneInstance->FindComponents<Camera>();

// Find only active components
Vector<HRenderable> activeRenderables = sceneInstance->FindComponents<Renderable>(true);

// Find all components (including inactive ones)
Vector<HRenderable> allRenderables = sceneInstance->FindComponents<Renderable>(false);
~~~~~~~~~~~~~

This is useful for:
- Finding all lights to adjust lighting settings
- Implementing gameplay systems that need to interact with all instances of a component type

# Clearing scenes

You can remove all objects from a scene instance using @b3d::SceneInstance::Clear:

~~~~~~~~~~~~~{.cpp}
TShared<SceneInstance> sceneInstance = SceneInstance::Create("MyScene");

// Clear all non-persistent objects
sceneInstance->Clear();

// Clear all objects (including persistent ones)
sceneInstance->Clear(true);
~~~~~~~~~~~~~

Note that objects marked with the **SceneObjectFlag::RuntimePersistent** flag will not be removed unless you pass `true` to Clear().

This is useful for:
- Resetting a level while keeping persistent UI or system objects
- Implementing a "restart level" feature
- Cleaning up before loading a new scene

Example:

~~~~~~~~~~~~~{.cpp}
// Create a persistent UI object that survives scene clears
HSceneObject uiRoot = SceneObject::Create("UI",
    (u32)SceneObjectFlag::RuntimePersistent);

// Create regular game objects
HSceneObject player = SceneObject::Create("Player");
HSceneObject enemy = SceneObject::Create("Enemy");

// Clear non-persistent objects (UI remains)
sceneInstance->Clear();

// UI object is still present, player and enemy are destroyed
~~~~~~~~~~~~~

# Scene instance time

Each scene instance has its own time management through the @b3d::SceneTime object. This allows you to control time independently for different scenes, enabling features like pause, slow motion, and time scaling.

For detailed information about using scene time, including time scaling, pausing, fixed timesteps, and practical examples, see the [Time utilities](../15_Utilities/08_time.md#scene-time) manual.

~~~~~~~~~~~~~{.cpp}
TShared<SceneInstance> sceneInstance = SceneInstance::Create("MyScene");
SceneTime& time = sceneInstance->GetTime();

// Get simulation time since scene started (affected by time scale)
float timeSinceStart = time.GetTimeInSeconds();

// Control time scale
time.SetScale(0.5f); // Slow motion
time.SetPaused(true); // Pause simulation
~~~~~~~~~~~~~

# Main camera

Scene instances track the main camera, which is used for rendering the primary viewport. You can access the main camera using @b3d::SceneInstance::GetMainCamera:

~~~~~~~~~~~~~{.cpp}
TShared<SceneInstance> sceneInstance = SceneInstance::Create("MyScene");

// Get the main camera in the scene
HCamera mainCamera = sceneInstance->GetMainCamera();

if (mainCamera != nullptr)
{
    // Configure the main camera
    mainCamera->SetNearClipDistance(0.1f);
    mainCamera->SetFarClipDistance(1000.0f);
}

// Get all cameras in the scene
const UnorderedMap<UUID, HCamera>& allCameras = sceneInstance->GetAllCameras();

// Iterate through all cameras
for (const auto& cameraEntry : allCameras)
{
    const HCamera& camera = cameraEntry.second;
    // Do something with each camera
}
~~~~~~~~~~~~~

The main camera is determined by the camera's viewport target. Typically, the camera that renders to the primary window or game viewport is considered the main camera.

You can set a camera's render target using:

~~~~~~~~~~~~~{.cpp}
HSceneObject cameraObject = SceneObject::Create("MainCamera");
HCamera camera = cameraObject->AddComponent<Camera>();

// Get the primary window and set it as the camera's target
TShared<RenderWindow> window = GetApplication().GetPrimaryWindow();
camera->GetViewport()->SetTarget(window);
~~~~~~~~~~~~~

# Scene object flags

Scene objects can have flags that control their behavior. These flags are set when creating the scene object and affect how it's handled by the scene system.

~~~~~~~~~~~~~{.cpp}
// Create an object that won't be saved
HSceneObject temporaryObject = SceneObject::Create("TempObject",
    (u32)SceneObjectFlag::DontSave);

// Create a persistent object that survives scene clear
HSceneObject persistentObject = SceneObject::Create("Persistent",
    (u32)SceneObjectFlag::RuntimePersistent);

// Create an internal object (used by engine systems)
HSceneObject internalObject = SceneObject::Create("Internal",
    (u32)SceneObjectFlag::Internal);

// Check if an object has a specific flag
bool isDontSave = temporaryObject->HasFlag(SceneObjectFlag::DontSave);
~~~~~~~~~~~~~

Available flags:

## DontSave

Objects marked with this flag will not be saved when saving the scene or prefab. This is useful for:
- Temporary debug visualization objects
- Runtime-generated objects that shouldn't persist
- Editor-only helpers

~~~~~~~~~~~~~{.cpp}
// Create a debug visualization that won't be saved
HSceneObject debugVisualization = SceneObject::Create("DebugViz",
    (u32)SceneObjectFlag::DontSave);
~~~~~~~~~~~~~

## RuntimePersistent

Objects marked with this flag survive calls to SceneInstance::Clear() (unless `forceAll` is true). This is useful for:
- UI elements that should persist across level reloads
- Game manager objects
- Audio systems
- Player controllers in certain game architectures

~~~~~~~~~~~~~{.cpp}
// Create a persistent game manager
HSceneObject gameManager = SceneObject::Create("GameManager",
    (u32)SceneObjectFlag::RuntimePersistent);

// This object will survive scene clears
sceneInstance->Clear(); // gameManager remains
~~~~~~~~~~~~~

Important notes:
- RuntimePersistent only works with top-level objects (objects directly parented to scene root)
- RuntimePersistent objects cannot be saved to scenes or prefabs
- Combining RuntimePersistent with DontSave is recommended for clarity

## Internal

This flag marks objects as used by engine internals. External systems (like editors) might use this to:
- Hide these objects from scene hierarchies
- Prevent user modification
- Apply special handling

~~~~~~~~~~~~~{.cpp}
// Create an internal rendering helper
HSceneObject renderHelper = SceneObject::Create("RenderHelper",
    (u32)SceneObjectFlag::Internal);
~~~~~~~~~~~~~

## Combining flags

You can combine multiple flags using the bitwise OR operator:

~~~~~~~~~~~~~{.cpp}
// Create a persistent object that won't be saved
HSceneObject persistentUI = SceneObject::Create("PersistentUI",
    (u32)(SceneObjectFlag::RuntimePersistent | SceneObjectFlag::DontSave));

// Create an internal object that won't be saved
HSceneObject internalHelper = SceneObject::Create("Helper",
    (u32)(SceneObjectFlag::Internal | SceneObjectFlag::DontSave));
~~~~~~~~~~~~~

## Setting flags after creation

Flags can also be set or unset after object creation:

~~~~~~~~~~~~~{.cpp}
HSceneObject sceneObject = SceneObject::Create("MyObject");

// Set flags (recursively applies to all children)
sceneObject->SetFlags(SceneObjectFlag::DontSave);

// Unset flags (recursively removes from all children)
sceneObject->UnsetFlags(SceneObjectFlag::DontSave);
~~~~~~~~~~~~~

Note that SetFlags and UnsetFlags affect the object and all its children recursively.
