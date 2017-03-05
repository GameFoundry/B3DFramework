Advanced startup				{#advancedStartup}
===============

We have already shown how to perform basic start-up of a Banshee application.

~~~~~~~~~~~~~{.cpp}
Application::startUp(VideoMode(1280, 720), "My app", false);

// Set up your scene here

Application::instance().runMainLoop();
Application::shutDown();
~~~~~~~~~~~~~

This form of start-up is adequate for applications using only the scene object/component system for implementing game logic. Applications wishing to implement systems that aren't components must use a more advanced form of start up. Implementing systems in such a way is necessary when extending the engine with new features, using low level rendering API, or if its just a preference.

To perform advanced start-up you must create your own version of the **Application** class by deriving from it. Once derived you can override any of the following methods:
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

		// Do your own initialization (i.e. start your custom systems)
	}

	void onShutDown() override
	{
		// Do your own shut-down (i.e. shut down your custom systems)

		// Shut-down engine components
		Application::onShutDown();
	}

	void preUpdate() override
	{
		// Execute per-frame logic of your custom systems (optionally do it in postUpdate)
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

Take a look at the *ExampleLowLevelRendering* for a working example of how to use advanced start-up in order to perform low-level rendering.