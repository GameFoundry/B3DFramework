---
title: Startup and main loop
---

# Preparation

Start by including the *B3DApplication.h* header into your project. It contains the @b3d::Application class which is used for starting up and running the framework.

The entirety of the framework API is contained in the **b3d** namespace, so you will also likely want to add a `using namespace b3d;` directive.

~~~~~~~~~~~~~{.cpp}
#include "B3DApplication.h"
#include "B3DEntry.h"

using namespace b3d;
~~~~~~~~~~~~~

# Entry point

The framework provides a unified entry point through the `B3DMain()` function. This function handles the platform-specific differences automatically:

~~~~~~~~~~~~~{.cpp}
int B3DMain()
{
	using namespace b3d;

	// ... application code ...

	return 0;
}
~~~~~~~~~~~~~

# Start up

The framework can then be started by calling @b3d::Application::StartUp. By default the framework always creates a single window on start-up, and the method expects you to provide the initial resolution of the window, window title and an optional fullscreen flag.

~~~~~~~~~~~~~{.cpp}
// Start an application in windowed mode using 1280x720 resolution
VideoMode videoMode(1280, 720);
Application::StartUp(videoMode, "My Application", false);
~~~~~~~~~~~~~

The first parameter is a @b3d::VideoMode structure that specifies the window resolution. The second parameter is the window title string. The third parameter is a boolean that specifies whether to start in fullscreen mode (true) or windowed mode (false).

# Scene setup

After the application has been started you can proceed to load necessary resources, create scene objects and set up their components.

By default the framework uses an scene object/component model for managing its scene. The scene is represented through scene objects which can be positioned and oriented in the scene, on which you attach components that execute some logic. Components can be built-in providing basic functionality like rendering an object, or they can be user-created and execute gameplay logic. You will also load resources like meshes and textures, which can then be provided to components.

We will go into much more detail about components in the next manual, but a quick example below shows how you would add a camera component in the scene.

~~~~~~~~~~~~~{.cpp}
// Add a scene object containing a camera component
HSceneObject cameraSceneObject = SceneObject::Create("SceneCamera");
HCamera sceneCamera = cameraSceneObject->AddComponent<Camera>();

// Get the primary window for rendering
TShared<RenderWindow> window = GetApplication().GetPrimaryWindow();

// Set the camera to render to the primary window
sceneCamera->GetViewport()->SetTarget(window);

// Position the camera
cameraSceneObject->SetPosition(Vector3(0.0f, 1.0f, 5.0f));
cameraSceneObject->LookAt(Vector3(0.0f, 0.0f, 0.0f));
~~~~~~~~~~~~~

Scene objects are created using @b3d::SceneObject::Create which returns a handle to the scene object. You can then add components to the scene object using @b3d::SceneObject::AddComponent. The scene object's transform can be modified using methods like @b3d::SceneObject::SetPosition, @b3d::SceneObject::SetRotation, and @b3d::SceneObject::LookAt.

# Running the main loop

Once your scene has been set up, you need to start running the main loop by calling @b3d::Application::RunMainLoop. The main loop will trigger updates on all the components you have set up, allowing you to execute game logic.

~~~~~~~~~~~~~{.cpp}
Application::Instance().RunMainLoop();
~~~~~~~~~~~~~

# Stopping the main loop

The main loop runs indefinitely until terminated by the user. You may call @b3d::Application::StopMainLoop to exit the loop programmatically.

~~~~~~~~~~~~~{.cpp}
Application::Instance().StopMainLoop();
~~~~~~~~~~~~~

# Shutting down

Once the main loop has been stopped, you will want to clean up any allocated resources by calling @b3d::Application::ShutDown.

~~~~~~~~~~~~~{.cpp}
Application::ShutDown();
~~~~~~~~~~~~~

# Complete example

Here's a complete code example. The code opens up a basic window and adds a camera to the scene. Since we haven't actually added any renderable objects to the scene, the camera won't see anything. The code also doesn't respond to any input and therefore doesn't offer any way for the user to stop the main loop.

~~~~~~~~~~~~~{.cpp}
#include "B3DApplication.h"
#include "B3DEntry.h"
#include "Scene/B3DSceneObject.h"
#include "Components/B3DCamera.h"
#include "GpuBackend/B3DRenderWindow.h"

int B3DMain()
{
	using namespace b3d;

	// Initialize the application
	VideoMode videoMode(1280, 720);
	Application::StartUp(videoMode, "My Application", false);

	// Create a camera
	HSceneObject cameraSceneObject = SceneObject::Create("SceneCamera");
	HCamera sceneCamera = cameraSceneObject->AddComponent<Camera>();

	// Get the primary window and set the camera to render to it
	TShared<RenderWindow> window = GetApplication().GetPrimaryWindow();
	sceneCamera->GetViewport()->SetTarget(window);

	// Position the camera
	cameraSceneObject->SetPosition(Vector3(0.0f, 1.0f, 5.0f));
	cameraSceneObject->LookAt(Vector3(0.0f, 0.0f, 0.0f));

	// Run the main loop
	Application::Instance().RunMainLoop();

	// Clean up
	Application::ShutDown();

	return 0;
}
~~~~~~~~~~~~~

# Advanced application setup

For more advanced use cases, you can use @b3d::ApplicationCreateInformation to specify detailed startup parameters:

~~~~~~~~~~~~~{.cpp}
ApplicationCreateInformation createInfo = Application::BuildCreateInformation(
	VideoMode(1280, 720),
	"My Application",
	false);

// Customize settings
createInfo.GpuBackend = "Vulkan"; // Specify render API
createInfo.PhysicsCooking = false; // Disable physics cooking

// Create application with custom settings
Application::StartUp(createInfo);
~~~~~~~~~~~~~

The @b3d::ApplicationCreateInformation structure allows you to specify:
- Render API plugin (e.g., "Vulkan", "D3D12", "Null")
- Renderer plugin
- Physics plugin
- Audio plugin
- Input plugin
- Physics cooking settings
- Async animation settings
- Custom importers
- Log callbacks
