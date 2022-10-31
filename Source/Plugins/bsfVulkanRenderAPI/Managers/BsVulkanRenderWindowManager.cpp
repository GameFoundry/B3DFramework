//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Managers/BsVulkanRenderWindowManager.h"

#if BS_PLATFORM == BS_PLATFORM_WIN32
#	include "Win32/BsWin32RenderWindow.h"
#elif BS_PLATFORM == BS_PLATFORM_LINUX
#	include "Linux/BsLinuxRenderWindow.h"
#elif BS_PLATFORM == BS_PLATFORM_OSX
#	include "MacOS/BsMacOSRenderWindow.h"
#endif

using namespace bs;

SPtr<RenderWindow> VulkanRenderWindowManager::CreateImpl(RENDER_WINDOW_DESC& desc, u32 windowId, const SPtr<RenderWindow>& parentWindow)
{
	if(parentWindow != nullptr)
	{
		u64 hWnd;
		parentWindow->GetCustomAttribute("WINDOW", &hWnd);
		desc.PlatformSpecific["parentWindowHandle"] = toString(hWnd);
	}

	// Create the window
#if BS_PLATFORM == BS_PLATFORM_WIN32
	Win32RenderWindow* renderWindow = new(B3DAllocate<Win32RenderWindow>()) Win32RenderWindow(desc, windowId);
	return B3DMakeCoreFromExisting<Win32RenderWindow>(renderWindow);
#elif BS_PLATFORM == BS_PLATFORM_LINUX
	LinuxRenderWindow* renderWindow = new(B3DAllocate<LinuxRenderWindow>()) LinuxRenderWindow(desc, windowId);
	return B3DMakeCoreFromExisting<LinuxRenderWindow>(renderWindow);
#elif BS_PLATFORM == BS_PLATFORM_OSX
	MacOSRenderWindow* renderWindow = new(B3DAllocate<MacOSRenderWindow>()) MacOSRenderWindow(desc, windowId);
	return B3DMakeCoreFromExisting<MacOSRenderWindow>(renderWindow);
#endif
}
