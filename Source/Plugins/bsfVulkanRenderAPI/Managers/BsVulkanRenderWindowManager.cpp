//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Managers/BsVulkanRenderWindowManager.h"

#if B3D_PLATFORM == B3D_PLATFORM_ID_WIN32
#	include "Win32/BsWin32RenderWindow.h"
#elif B3D_PLATFORM == B3D_PLATFORM_ID_LINUX
#	include "Linux/BsLinuxRenderWindow.h"
#elif B3D_PLATFORM == B3D_PLATFORM_ID_MACOS
#	include "MacOS/BsMacOSRenderWindow.h"
#endif

using namespace bs;

SPtr<RenderWindow> VulkanRenderWindowManager::CreateImpl(RENDER_WINDOW_DESC& desc, u32 windowId, const SPtr<RenderWindow>& parentWindow)
{
	if(parentWindow != nullptr)
	{
		u64 hWnd;
		parentWindow->GetCustomAttribute("WINDOW", &hWnd);
		desc.PlatformSpecific["parentWindowHandle"] = ToString(hWnd);
	}

	// Create the window
#if B3D_PLATFORM == B3D_PLATFORM_ID_WIN32
	Win32RenderWindow* renderWindow = new(B3DAllocate<Win32RenderWindow>()) Win32RenderWindow(desc, windowId);
	return B3DMakeSharedFromExisting<Win32RenderWindow>(renderWindow);
#elif B3D_PLATFORM == B3D_PLATFORM_ID_LINUX
	LinuxRenderWindow* renderWindow = new(B3DAllocate<LinuxRenderWindow>()) LinuxRenderWindow(desc, windowId);
	return B3DMakeSharedFromExisting<LinuxRenderWindow>(renderWindow);
#elif B3D_PLATFORM == B3D_PLATFORM_ID_MACOS
	MacOSRenderWindow* renderWindow = new(B3DAllocate<MacOSRenderWindow>()) MacOSRenderWindow(desc, windowId);
	return B3DMakeSharedFromExisting<MacOSRenderWindow>(renderWindow);
#endif
}
