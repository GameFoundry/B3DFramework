Non-component approach				{#nonComponentApproach}
===============
[TOC]

bs::f provides an alternate API to its scene-object/component model. It provides a more traditional way of dealing with gameplay logic at the cost of more complexity, but with potentially more freedom. Note that this is a fully optional approach and just another way of doing what we have described so far.

# Scene actors {#nonComponentApproach_a}
Each **Component** class has a corresponding @ref bs::SceneActor "SceneActor", which shares the same name as the component without the "C" prefix (e.g. @ref bs::Renderable "Renderable" scene actor vs. **CRenderable** component)). 

> Note that throughout this manual we have often referred to components without using the "C" prefix, but in this manual we are referring to the scene actor.

To create a scene actor you should call its static **create()** method. A scene actor is not attached to a **SceneObject** and it instead stands on its own.

~~~~~~~~~~~~~{.cpp}
// Creating a renderable actor
SPtr<Renderable> renderable = Renderable::create();
~~~~~~~~~~~~~

Scene actors and components share most of the same functionality and methods, although scene actors sometimes require some additional setup (for example an **CAnimation** component is able to find its child **CBone** components automatically, but with scene actors you must connect them manually). Since the majority of the interface is the same we won't talk about individual actor types, and you can instead refer to the API reference.

~~~~~~~~~~~~~{.cpp}
HMesh mesh = ...;
HMaterial material = ...;

// Set up Renderable actor's mesh and material identically as we would a CRenderable component
renderable->setMesh(mesh);
renderable->setMaterial(material);
~~~~~~~~~~~~~

## Transform {#nonComponentApproach_a_a}
Each scene actor has a **Transform** object you can use to position and orient it in the scene. The transform can be accessed through @ref bs::SceneActor::getTransform() "SceneActor::getTransform()" and @ref bs::SceneActor::setTransform() "SceneActor::setTransform()". It can be manipulated the same as you would a **Transform** on a **SceneObject**. 

~~~~~~~~~~~~~{.cpp}
Transform tfrm = renderable->getTransform();
tfrm.setPosition(Vector3(0.0f, 50.0f, 0.0f));
renderable->setTransform(tfrm);
~~~~~~~~~~~~~

# Running custom logic {#nonComponentApproach_b}
When you use scene objects and components to set up your scene, you do so before calling the main loop. During the main loop the system will call various callbacks in which you can implement your game logic. But without components we need a different way of executing gameplay logic.

To do this we need to change how we start the application. You will need to create your own version of the **Application** class by deriving from it. Once derived you can override any of the following methods:
 - @ref bs::Application::onStartUp "Application::onStartUp()" - Called when the application is first starting up.
 - @ref bs::Application::preUpdate "Application::preUpdate()" - Called every frame, just before component and plugin updates are triggered.
 - @ref bs::Application::postUpdate "Application::postUpdate()" - Called every frame, after component and plugin updates are triggered.
 - @ref bs::Application::onShutDown "Application::onShutDown()" - Called just before the application is about to shut down.
 
~~~~~~~~~~~~~{.cpp}
class MyApplication : public Application
{
public:
	// Pass along the start-up structure to the parent
	MyApplication(const START_UP_DESC& desc)
		:Application(desc)
	{ }

private:
	void onStartUp() override
	{
		// Ensure all parent systems are initialized first
		Application::onStartUp();

		// Do your own initialization
	}

	void onShutDown() override
	{
		// Do your own shut-down

		// Shut-down engine components
		Application::onShutDown();
	}

	void preUpdate() override
	{
		// Execute per-frame logic (optionally do it in postUpdate)
	}
};
~~~~~~~~~~~~~

Once you have your own application override, you can now start the application by calling @ref bs::Application::startUp<T> "Application::startUp<T>()" with the template parameter being your application override class. Everything else regarding start-up remains the same.

~~~~~~~~~~~~~{.cpp}
Application::startUp<MyApplication>(VideoMode(1280, 720), "My app", false);

// Set up your scene here

Application::instance().runMainLoop();
Application::shutDown();
~~~~~~~~~~~~~

Having access to start-up, shut-down and update methods directly allows you to write your game logic as you see fit, without having to follow the **Component** interface.
 