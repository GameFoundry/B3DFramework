Windows					{#window}
===============

A window represents the final destination where the applications rendered output gets displayed to the user. It has a title, size and a position. Window can cover the entirety of the user's screen (fullscreen mode) or just part of it (windowed mode). In Banshee a window is represented using the @ref bs::RenderWindow "RenderWindow" class.

![Render window](RenderWindow.png)  

# Primary window
When we initialized the application by calling **Application::startUp** in a previous chapter a primary window was created for us by default. You can access this window through @ref bs::Application::getPrimaryWindow "Application::getPrimaryWindow".

~~~~~~~~~~~~~{.cpp}
SPtr<RenderWindow> primaryWindow = gApplication().getPrimaryWindow();
~~~~~~~~~~~~~

> **gApplication()** is just a shortcut for **Application::instance()** we used earlier.

# Creating windows
You can also create your own windows by filling out the @ref bs::RENDER_WINDOW_DESC "RENDER_WINDOW_DESC" structure and calling @ref bs::RenderWindow::create "RenderWindow::create".

~~~~~~~~~~~~~{.cpp}
RENDER_WINDOW_DESC desc;
desc.videoMode = VideoMode(1280, 720);
desc.fullscreen = false;
desc.title = "Helper window".

SPtr<RenderWindow> newWindow = RenderWindow::create(desc);
~~~~~~~~~~~~~

# Destroying windows
You can destroy a window by calling @ref RenderWindow::destroy "RenderWindow::destroy". 

~~~~~~~~~~~~~{.cpp}
newWindow->destroy();
~~~~~~~~~~~~~

> Do not destroy the primary window, as it will result in undefined behaviour.

# Manipulating windows

