Startup and main loop					{#startup}
===============

Banshee is started through the @ref bs::Application "Application" interface. 

~~~~~~~~~~~~~{.cpp}
// Start an application in windowed mode using 1280x720 resolution
Application::startUp(
	VideoMode(1280, 720), // Window resolution
	"My app", // Window title
	false); // True for fullscreen, false for windowed

// Set up your scene here

Application::instance().runMainLoop();
Application::shutDown();
~~~~~~~~~~~~~

# Start up
@ref bs::Application::startUp "Application::startUp" expects you to provide the resolution for the primary application window, its title and a couple of optional parameters.

> Advanced: You can perform a more customized startup by filling out the @ref bs::START_UP_DESC "START_UP_DESC" structure and passing it to **Application::startUp**.

# Main loop
Following start-up we run the main game loop by calling @ref bs::Application::runMainLoop "Application::runMainLoop". This is where your game runs, where every frame is updated and rendered.

# Shut down
Finally, when the user decides to close the application and the main loop exits, we wish to clean up the application by calling @ref bs::Application::shutDown "Application::shutDown".

# Scene setup
You are expected to perform initial scene setup in-between the **Application::startUp** and **Application::runMainLoop**, as we will demonstrate later.