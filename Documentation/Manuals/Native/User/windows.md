Windows					{#windows}
===============

A window represents the final destination where the application's rendered output gets displayed to the user. It has a title, size and a position. Window can cover the entirety of the user's screen (fullscreen mode) or just part of it (windowed mode). In Banshee a window is represented using the @ref bs::RenderWindow "RenderWindow" class.

![Render window](RenderWindow.png)  

# Primary window
When we initialized the application by calling **Application::startUp** in a previous chapter a primary window was created for us by default. You can access this window through @ref bs::Application::getPrimaryWindow "Application::getPrimaryWindow()".

~~~~~~~~~~~~~{.cpp}
SPtr<RenderWindow> primaryWindow = gApplication().getPrimaryWindow();
~~~~~~~~~~~~~

> **gApplication()** is just a shortcut for **Application::instance()** we used earlier.

# Creating windows
You can also create your own windows by filling out the @ref bs::RENDER_WINDOW_DESC "RENDER_WINDOW_DESC" structure and calling @ref bs::RenderWindow::create "RenderWindow::create()".

~~~~~~~~~~~~~{.cpp}
RENDER_WINDOW_DESC desc;
desc.videoMode = VideoMode(1280, 720);
desc.fullscreen = false;
desc.title = "Helper window".

// Creates a new non-fullscreen window with size 1280x720, at the center of the screen
SPtr<RenderWindow> newWindow = RenderWindow::create(desc);
~~~~~~~~~~~~~

# Destroying windows
You can destroy a window by calling @ref bs::RenderWindow::destroy "RenderWindow::destroy()". 

~~~~~~~~~~~~~{.cpp}
newWindow->destroy();
~~~~~~~~~~~~~

> Do not destroy the primary window, as it will result in undefined behaviour.

# Manipulating windows
Window size can be changed by calling @ref bs::RenderWindow::resize "RenderWindow::resize()".

~~~~~~~~~~~~~{.cpp}
newWindow->resize(1920, 1080);
~~~~~~~~~~~~~

And they can be moved by calling @ref bs::RenderWindow::move "RenderWindow::move()". Movement is not relevant for windows in fullscreen mode.

~~~~~~~~~~~~~{.cpp}
newWindow->move(0, 0); // Move to top right of the screen
~~~~~~~~~~~~~

If you wish to switch from windowed to fullscreen mode call @ref bs::RenderWindow::setFullscreen "RenderWindow::setFullscreen()".

~~~~~~~~~~~~~{.cpp}
newWindow->setFullscreen(VideoMode(1920, 1080));
~~~~~~~~~~~~~

And if you wish to switch from fullscreen to windowed call @ref bs::RenderWindow::setWindowed "RenderWindow::setWindowed()".

~~~~~~~~~~~~~{.cpp}
newWindow->setWindowed(1280, 720);
~~~~~~~~~~~~~

# Window properties
You can access current properties of the window, like its size and position, by calling @ref bs::RenderWindow::getProperties "RenderWindow::getProperties", which returns a @ref bs::RenderWindowProperties "RenderWindowProperties" object. For example let's print out current window's size:

~~~~~~~~~~~~~{.cpp}
auto& props = newWindow->getProperties();

gDebug().logDebug(toString(props.getWidth()) + " x " + toString(props.getHeight()));
~~~~~~~~~~~~~

> gDebug() is a shortcut to the logging system, as explained in the [logging manual](@ref logging), covered later.

# Window events
Sometimes you might want to be notified if the user resizes the window externally, in which case use the @ref bs::RenderWindow::onResized "RenderWindow::onResized" event.

~~~~~~~~~~~~~{.cpp}
void notifyResized()
{
	gDebug().logDebug("Window was resized.");
}

newWindow->onResized.connect(&notifyResized);
~~~~~~~~~~~~~

> **RenderWindow::onResized** is an example of an event. They are explained later in the [event manual](@ref events).

# Video modes
During window creation and calls to **RenderWindow::setFullscreen** we have seen the use of the @ref bs::VideoMode "VideoMode" class. This class allows you to specify the resolution of the window, along with an optional refresh rate and output monitor (in case of multi-monitor setups, to choose on which monitor to show the window). 

You can create your own **VideoMode** with custom parameters (as we did so far), or you can query for all video modes supported by the user's GPU by calling @ref bs::RenderAPI::getVideoModeInfo() "RenderAPI::getVideoModeInfo()". This will return a @ref bs::VideoModeInfo "VideoModeInfo" object that contains information about all available monitors, their supported resolutions and refresh rates.

An example on how to use the video mode enumeration to set a window to fullscreen mode using the user's desktop resolution of the primary monitor:
~~~~~~~~~~~~~{.cpp}
VideoModeInfo videoModeInfo = RenderAPI::getVideoModeInfo();
VideoOutputInfo primaryMonitorInfo = videoModeInfo.getOutputInfo(0); // 0th monitor is always primary

newWindow->setFullscreen(primaryMonitorInfo.getDesktopVideoMode());
~~~~~~~~~~~~~

An example to make a window fullscreen on a secondary monitor if one is available:
~~~~~~~~~~~~~{.cpp}
VideoModeInfo videoModeInfo = RenderAPI::getVideoModeInfo();

UINT32 numOutputs = videoModeInfo.getNumOutputs();
if(numOutputs > 1)
{
	VideoOutputInfo secondaryMonitorInfo = videoModeInfo.getOutputInfo(1);
	newWindow->setFullscreen(secondaryMonitorInfo.getDesktopVideoMode());
}
~~~~~~~~~~~~~

And an example how to enumerate and print all available resolutions on the primary monitor:
~~~~~~~~~~~~~{.cpp}
VideoModeInfo videoModeInfo = RenderAPI::getVideoModeInfo();
VideoOutputInfo primaryMonitorInfo = videoModeInfo.getOutputInfo(0);

UINT32 numVideoModes = primaryMonitorInfo.getNumVideoModes();
for (UINT32 i = 0; i < numVideoModes; i++)
{
	const VideoMode& curVideoMode = primaryMonitorInfo.getVideoMode(i);

	String videoModeString = toString(curVideoMode.getWidth()) + " x " + toString(curVideoMode.getHeight()) + " at " + toString(curVideoMode.getRefreshRate()) + "Hz";
	
	gDebug().logDebug(videoModeString);
}
~~~~~~~~~~~~~