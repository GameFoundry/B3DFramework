Startup and main loop					{#startup}
===============
[TOC]

# Preparation {#startup_a}

Start by including *BsApplication.h* header into your project. It contains the @ref bs::Application "Application" class which is the primary entry point into bs::f.

Entirety of bs::f API is contained in the **bs** namespace, so you will also likely want to add a `using namespace bs;` directive.

~~~~~~~~~~~~~{.cpp}
#include "BsApplication.h"

using namespace bs;

int main()
{
	// ...

	return 0;
}
~~~~~~~~~~~~~

# Start up {#startup_b}

bs::f can then be started by calling @ref bs::Application::startUp "Application::startUp()". By default bs::f always creates a single window on start-up, and the method expects you to provide the initial resolution of the window, window title and an optional fullscreen flag.

~~~~~~~~~~~~~{.cpp}
// Start an application in windowed mode using 1280x720 resolution
Application::startUp(
	VideoMode(1280, 720), // Window resolution
	"My app", // Window title
	false); // True for fullscreen, false for windowed
~~~~~~~~~~~~~

# Scene setup  {#startup_c}

After the application has been started you can proceed to load necessary resources, create scene objects and set up their components.

By default bs::f uses an entity/component model for managing its scene. The scene is represented through scene objects which can be positioned and oriented in the scene, on which you attach components that execute some logic. Components can be built-in providing basic functionality like rendering an object, or they can be user-created and execute gameplay logic. You will also load resources like meshes and textures, which can then be provided to components.

We will go into much more detail about components in the next manual, but a quick example below shows how you would add a camera component in the scene.

~~~~~~~~~~~~~{.cpp}
// Add a scene object containing a camera component
SPtr<RenderWindow> window = gApplication().getPrimaryWindow();
HSceneObject sceneCameraSO = SceneObject::create("SceneCamera");
HCamera sceneCamera = sceneCameraSO->addComponent<CCamera>(window);

sceneCameraSO->setPosition(Vector3(40.0f, 30.0f, 230.0f));
sceneCameraSO->lookAt(Vector3(0, 0, 0));
~~~~~~~~~~~~~

# Running the main loop {#startup_d}

Once your scene has been set up, you need to start running the main loop by calling @ref bs::Application::runMainLoop "Application::runMainLoop()". The main loop will trigger updates on all the components you have set up, allowing you to execute game-logic.

# Stopping the main loop {#startup_e}

The main loop runs indefinitely until terminated by the user. You may call @ref bs::Application::stopMainLoop "Application::stopMainLoop()" to exit the loop.

# Shutting down {#startup_f}

Once the main loop has been stopped, you will want to clean up any allocated resources by calling @ref bs::Application::shutDown "Application::shutDown()".

# Complete example {#startup_g}

Here's a complete code of what we have so far. The code doesn't do much - it opens up a basic window and adds a camera to the scene. Since we haven't actually added any renderable objects to the scene the camera wont see anything. The code also doesn't respond to any input and therefore doesn't offer any way for the user to stop the main loop.

~~~~~~~~~~~~~{.cpp}
#include "BsApplication.h"

using namespace bs;

int main()
{
	Application::startUp(VideoMode(1280, 720),"My app", false);

	SPtr<RenderWindow> window = gApplication().getPrimaryWindow();
	HSceneObject sceneCameraSO = SceneObject::create("SceneCamera");
	HCamera sceneCamera = sceneCameraSO->addComponent<CCamera>(window);

	sceneCameraSO->setPosition(Vector3(40.0f, 30.0f, 230.0f));
	sceneCameraSO->lookAt(Vector3(0, 0, 0));
	
	Application::runMainLoop();
	Application::shutDown();
	return 0;
}
~~~~~~~~~~~~~