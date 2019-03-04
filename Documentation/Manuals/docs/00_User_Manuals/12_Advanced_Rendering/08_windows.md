---
title: Windows
---

A window represents the final destination where the application's rendered output gets displayed to the user. It has a title, size and a position. Window can cover the entirety of the user's screen (fullscreen mode) or just part of it (windowed mode). In bs::f a window is represented using the @bs::RenderWindow class. We have already shown how the application creates a primary window when it is first started up, and in this chapter we'll show how to create more windows manually as well as manipulate them.

![Render window](RenderWindow.png)  

# Creating windows
You can also create your own windows by filling out the @bs::RENDER_WINDOW_DESC structure and calling @bs::RenderWindow::create.

~~~~~~~~~~~~~{.cpp}
RENDER_WINDOW_DESC desc;
desc.videoMode = VideoMode(1280, 720);
desc.fullscreen = false;
desc.title = "Helper window".

// Creates a new non-fullscreen window with size 1280x720, at the center of the screen
SPtr<RenderWindow> newWindow = RenderWindow::create(desc);
~~~~~~~~~~~~~

# Destroying windows
You can destroy a window by calling @bs::RenderWindow::destroy. 

~~~~~~~~~~~~~{.cpp}
newWindow->destroy();
~~~~~~~~~~~~~

> Do not destroy the primary window, as it will result in undefined behaviour.

# Manipulating windows
Window size can be changed by calling @bs::RenderWindow::resize.

~~~~~~~~~~~~~{.cpp}
newWindow->resize(1920, 1080);
~~~~~~~~~~~~~

And they can be moved by calling @bs::RenderWindow::move. Movement is not relevant for windows in fullscreen mode.

~~~~~~~~~~~~~{.cpp}
newWindow->move(0, 0); // Move to top right of the screen
~~~~~~~~~~~~~

If you wish to switch from windowed to fullscreen mode call @bs::RenderWindow::setFullscreen.

~~~~~~~~~~~~~{.cpp}
newWindow->setFullscreen(VideoMode(1920, 1080));
~~~~~~~~~~~~~

And if you wish to switch from fullscreen to windowed call @bs::RenderWindow::setWindowed.

~~~~~~~~~~~~~{.cpp}
newWindow->setWindowed(1280, 720);
~~~~~~~~~~~~~

# Window properties
You can access current properties of the window, like its size and position, by calling @bs::RenderWindow::getProperties, which returns a @bs::RenderWindowProperties object. For example let's print out current window's size:

~~~~~~~~~~~~~{.cpp}
auto& props = newWindow->getProperties();

gDebug().logDebug(toString(props.width) + " x " + toString(props.height));
~~~~~~~~~~~~~

# Window events
Sometimes you might want to be notified if the user resizes the window externally, in which case use the @bs::RenderWindow::onResized event.

~~~~~~~~~~~~~{.cpp}
void notifyResized()
{
	gDebug().logDebug("Window was resized.");
}

newWindow->onResized.connect(&notifyResized);
~~~~~~~~~~~~~

# Video modes
During window creation and calls to **RenderWindow::setFullscreen()** we have seen the use of the @bs::VideoMode class. This class allows you to specify the resolution of the window, along with an optional refresh rate and output monitor (in case of multi-monitor setups, to choose on which monitor to show the window). 

You can create your own **VideoMode** with custom parameters (as we did so far), or you can query for all video modes supported by the user's GPU by calling @bs::RenderAPI::getVideoModeInfo(). This will return a @bs::VideoModeInfo object that contains information about all available monitors, their supported resolutions and refresh rates.

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

	String videoModeString = toString(curVideoMode.width) + " x " + toString(curVideoMode.height) + " at " + toString(curVideoMode.refreshRate) + "Hz";
	
	gDebug().logDebug(videoModeString);
}
~~~~~~~~~~~~~