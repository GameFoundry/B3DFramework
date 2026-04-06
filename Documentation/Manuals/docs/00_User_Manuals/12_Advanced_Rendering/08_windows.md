---
title: Windows
---

A window represents the final destination where the application's rendered output gets displayed to the user. It has a title, size and a position. Window can cover the entirety of the user's screen (fullscreen mode) or just part of it (windowed mode). In the framework a window is represented using the @b3d::RenderWindow class. We have already shown how the application creates a primary window when it is first started up, and in this chapter we'll show how to create more windows manually as well as manipulate them.

![Render window](../../Images/RenderWindow.png)

# Creating windows
You can also create your own windows by filling out the @b3d::RenderWindowCreateInformation structure and calling @b3d::RenderWindow::Create.

~~~~~~~~~~~~~{.cpp}
RenderWindowCreateInformation createInformation;
createInformation.VideoMode = VideoMode(1280, 720);
createInformation.Fullscreen = false;
createInformation.Title = "Helper window";

// Creates a new non-fullscreen window with size 1280x720, at the center of the screen
SPtr<RenderWindow> helperWindow = RenderWindow::Create(createInformation);
~~~~~~~~~~~~~

# Destroying windows
You can destroy a window by calling @b3d::RenderWindow::Destroy.

~~~~~~~~~~~~~{.cpp}
helperWindow->Destroy();
~~~~~~~~~~~~~

> Do not destroy the primary window, as it will result in undefined behaviour.

# Manipulating windows
Window size can be changed by calling @b3d::RenderWindow::Resize.

~~~~~~~~~~~~~{.cpp}
helperWindow->Resize(1920, 1080);
~~~~~~~~~~~~~

And they can be moved by calling @b3d::RenderWindow::Move. Movement is not relevant for windows in fullscreen mode.

~~~~~~~~~~~~~{.cpp}
helperWindow->Move(0, 0); // Move to top left of the screen
~~~~~~~~~~~~~

If you wish to switch from windowed to fullscreen mode call @b3d::RenderWindow::SetFullscreen.

~~~~~~~~~~~~~{.cpp}
helperWindow->SetFullscreen(VideoMode(1920, 1080));
~~~~~~~~~~~~~

And if you wish to switch from fullscreen to windowed call @b3d::RenderWindow::SetWindowed.

~~~~~~~~~~~~~{.cpp}
helperWindow->SetWindowed(1280, 720);
~~~~~~~~~~~~~

# Window properties
You can access current properties of the window, like its size and position, by calling @b3d::RenderWindow::GetRenderWindowProperties, which returns a @b3d::RenderWindowProperties object. For example let's print out current window's size:

~~~~~~~~~~~~~{.cpp}
const RenderWindowProperties& windowProperties = helperWindow->GetRenderWindowProperties();

B3D_LOG(Info, LogRenderer, "Window size: {0} x {1}", windowProperties.Width, windowProperties.Height);
~~~~~~~~~~~~~

You can also query the window's position and focus state:

~~~~~~~~~~~~~{.cpp}
const RenderWindowProperties& windowProperties = helperWindow->GetRenderWindowProperties();

B3D_LOG(Info, LogRenderer, "Window position: ({0}, {1})", windowProperties.Left, windowProperties.Top);
B3D_LOG(Info, LogRenderer, "Window has focus: {0}", windowProperties.HasFocus);
B3D_LOG(Info, LogRenderer, "Window is fullscreen: {0}", windowProperties.IsFullScreen);
~~~~~~~~~~~~~

# Window events
Sometimes you might want to be notified if the user resizes the window externally, in which case use the @b3d::RenderWindow::OnResized event.

~~~~~~~~~~~~~{.cpp}
void NotifyResized()
{
	B3D_LOG(Info, LogRenderer, "Window was resized.");
}

helperWindow->OnResized.Connect(&NotifyResized);
~~~~~~~~~~~~~

# Video modes
During window creation and calls to **RenderWindow::SetFullscreen()** we have seen the use of the @b3d::VideoMode class. This class allows you to specify the resolution of the window, along with an optional refresh rate and output monitor (in case of multi-monitor setups, to choose on which monitor to show the window).

You can create your own **VideoMode** with custom parameters (as we did so far), or you can query for all video modes supported by the user's GPU by calling @b3d::GpuDevice::GetVideoModeInfo(). This will return a @b3d::VideoModeInfo object that contains information about all available monitors, their supported resolutions and refresh rates.

An example on how to use the video mode enumeration to set a window to fullscreen mode using the user's desktop resolution of the primary monitor:
~~~~~~~~~~~~~{.cpp}
const SPtr<GpuDevice>& gpuDevice = GetApplication().GetPrimaryGpuDevice();
const VideoModeInfo& videoModeInfo = gpuDevice->GetVideoModeInfo();
const VideoOutputInfo& primaryMonitorInfo = videoModeInfo.GetOutputInfo(0); // 0th monitor is always primary

helperWindow->SetFullscreen(primaryMonitorInfo.GetDesktopVideoMode());
~~~~~~~~~~~~~

An example to make a window fullscreen on a secondary monitor if one is available:
~~~~~~~~~~~~~{.cpp}
const SPtr<GpuDevice>& gpuDevice = GetApplication().GetPrimaryGpuDevice();
const VideoModeInfo& videoModeInfo = gpuDevice->GetVideoModeInfo();

u32 outputCount = videoModeInfo.GetOutputCount();
if(outputCount > 1)
{
	const VideoOutputInfo& secondaryMonitorInfo = videoModeInfo.GetOutputInfo(1);
	helperWindow->SetFullscreen(secondaryMonitorInfo.GetDesktopVideoMode());
}
~~~~~~~~~~~~~

And an example how to enumerate and print all available resolutions on the primary monitor:
~~~~~~~~~~~~~{.cpp}
const SPtr<GpuDevice>& gpuDevice = GetApplication().GetPrimaryGpuDevice();
const VideoModeInfo& videoModeInfo = gpuDevice->GetVideoModeInfo();
const VideoOutputInfo& primaryMonitorInfo = videoModeInfo.GetOutputInfo(0);

u32 videoModeCount = primaryMonitorInfo.GetVideoModeCount();
for (u32 i = 0; i < videoModeCount; i++)
{
	const VideoMode& currentVideoMode = primaryMonitorInfo.GetVideoMode(i);

	B3D_LOG(Info, LogRenderer, "Video mode: {0} x {1} at {2}Hz", currentVideoMode.Width, currentVideoMode.Height, currentVideoMode.RefreshRate);
}
~~~~~~~~~~~~~
