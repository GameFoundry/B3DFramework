Getting started								{#gettingStarted}
===============
[TOC]

This manual offers a quick overview of commonly used Banshee functionality, in order to give you a better idea of how Banshee works. For a fully working example check out the `ExampleProject` project available with the source code.

# Starting an application
Banshee is started through the @ref bs::Application "Application" interface. To start the engine you need to provide it with a description of the primary render window and choose which modules to load. Since Banshee is a plugin based engine you have a selection between multiple modules for each system like render API or physics, and a set of completely optional plugins (like importers for various file formats).

After the application is started by calling @ref bs::Application::startUp "Application::startUp", you can set up your custom code in the form of @ref bs::Component "components" (see later). After that you can run the main loop with @ref bs::Application::runMainLoop "Application::runMainLoop" which will execute your code and actually get everything in motion.

Once the main loop terminates use @ref bs::Application::shutDown "Application::shutDown" to clean everything up.

~~~~~~~~~~~~~{.cpp}
// Descriptor used for initializing the engine
START_UP_DESC startUpDesc;

// Choose which plugins to load. In this case use the default values as specified by the build system.
startUpDesc.renderAPI = BS_RENDER_API_MODULE;
startUpDesc.renderer = BS_RENDERER_MODULE;
startUpDesc.audio = BS_AUDIO_MODULE;
startUpDesc.physics = BS_PHYSICS_MODULE;
startUpDesc.input = BS_INPUT_MODULE;

// List of optional importer plugins we plan on using for importing various resources.
startUpDesc.importers.push_back("BansheeFreeImgImporter"); // For importing textures
startUpDesc.importers.push_back("BansheeFBXImporter"); // For importing meshes
startUpDesc.importers.push_back("BansheeFontImporter"); // For importing fonts
startUpDesc.importers.push_back("BansheeSL"); // For importing shaders

// Descriptor used for initializing the primary application window.
startUpDesc.primaryWindowDesc.videoMode = VideoMode(windowResWidth, windowResHeight);
startUpDesc.primaryWindowDesc.title = "Banshee Example App";
startUpDesc.primaryWindowDesc.fullscreen = false;
startUpDesc.primaryWindowDesc.depthBuffer = false;

Application::startUp(startUpDesc);
... set up game code ...
Application::instance().runMainLoop();
Application::shutDown();
~~~~~~~~~~~~~

Read the [render targets](@ref renderTargets) manual for more information about creating render windows, and the [game objects](@ref gameObjects) manual on how to create your own components.

# Importing resources
To import a resource from a third party format into an engine-readable format use the @ref bs::Importer "Importer" module. A variety of formats like PNG, JPG, PSD, FBX, etc. are supported. Read the [importer](@ref customImporters) manual for more information about importers. 

In the example below we'll import a @ref bs::Mesh "Mesh" and a @ref bs::Texture "Texture".
~~~~~~~~~~~~~{.cpp}
HMesh dragonModel = gImporter().import<Mesh>("Dragon.fbx");
HTexture dragonTexture = gImporter().import<Texture>("Dragon.psd");
~~~~~~~~~~~~~

# Setting up a material
Once we have a mesh and a texture we need some way to apply that texture to the mesh. For that reason we first import a @ref bs::Shader "Shader" that describes how is an object rendered, which we then use to create a @ref bs::Material "Material" which allows us to apply our previously loaded @ref bs::Texture "Texture".

Read the [material](@ref materials) manual for more information about shaders and materials.

~~~~~~~~~~~~~{.cpp}
HShader diffuse = gImporter().import<Shader>("Diffuse.bsl");
HMaterial dragonMaterial = Material::create(diffuse);

dragonMaterial->setTexture("albedo", dragonTexture);
~~~~~~~~~~~~~

# Adding an object for rendering
To actually make our mesh render with our material, we need to add a @ref bs::SceneObject "SceneObject" onto which we attach a @ref bs::CRenderable "CRenderable" component. This component allows us to set up a mesh and a material, after which they will be automatically rendered to the active camera.

To learn more about @ref bs::SceneObject "scene objects" and @ref bs::Component "components" read the [game object](@ref gameObjects) manual. You can use the same approach we followed here to add your own custom components, containing custom code.

You can also read up on the [renderer](@ref renderer) manual to learn more about how the @ref bs::CRenderable "CRenderable" components works.

~~~~~~~~~~~~~{.cpp}
HSceneObject dragonSO = SceneObject::create("Dragon");

HRenderable renderable = dragonSO->addComponent<CRenderable>();
renderable->setMesh(dragonModel);
renderable->setMaterial(dragonMaterial);
~~~~~~~~~~~~~

# Adding a scene camera
In order to actually see our object we need to set up a camera. First create a new @ref bs::SceneObject "SceneObject" onto which you can then attach a @ref bs::CCamera "CCamera" component. After that we position the @ref bs::SceneObject "SceneObject" so it is looking at the object we set up in previous steps.

~~~~~~~~~~~~~{.cpp}
HSceneObject sceneCameraSO = SceneObject::create("SceneCamera");
HCamera sceneCamera = sceneCameraSO->addComponent<CCamera>(window);

sceneCameraSO->setPosition(Vector3(40.0f, 30.0f, 230.0f));
sceneCameraSO->lookAt(Vector3(0, 0, 0));
~~~~~~~~~~~~~

# Adding a GUI element
Finally you might want to add a GUI to your application. Similar to above, create a new @ref bs::SceneObject "SceneObject" onto which we add a new @ref bs::CCamera "CCamera" used only for rendering GUI. Since we don't want it to render normal scene objects, we let it's filter `layers` to 0. 

We also add a @ref bs::CGUIWidget "CGUIWidget" component onto the same @ref bs::SceneObject "SceneObject", which is used as a container for all GUI elements. After that we add a couple of @ref bs::GUIButton "GUIButtons" to the GUI.

Read the [GUI](@ref customGUI) manual for more information about GUI.

~~~~~~~~~~~~~{.cpp}
HSceneObject guiSO = SceneObject::create("GUI");

HCamera guiCamera = guiSO->addComponent<CCamera>(window);
guiCamera->setLayers(0); // Force camera to ignore normal scene geometry

HGUIWidget gui = guiSO->addComponent<CGUIWidget>(guiCamera);
GUIPanel* guiPanel = gui->getPanel();

GUILayout* guiLayout = guiPanel->addNewElement<GUILayoutY>();
guiLayout->addNewElement<GUIButton>(HString(L"Click me!"));
guiLayout->addNewElement<GUIButton>(HString(L"Click me too!"));
~~~~~~~~~~~~~

There is a lot more to Banshee, but hopefully this gave you a quick taste of how it works. Continue reading other manuals and the API reference for more information.