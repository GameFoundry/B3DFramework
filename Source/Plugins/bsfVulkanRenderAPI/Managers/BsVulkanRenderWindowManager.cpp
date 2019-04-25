//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Managers/BsVulkanRenderWindowManager.h"

#if BS_PLATFORM == BS_PLATFORM_WIN32
#include "Win32/BsWin32RenderWindow.h"
#elif BS_PLATFORM == BS_PLATFORM_LINUX
#include "Linux/BsLinuxRenderWindow.h"
#endif

namespace bs 
{
	SPtr<RenderWindow> VulkanRenderWindowManager::createImpl(RENDER_WINDOW_DESC& desc, UINT32 windowId, const SPtr<RenderWindow>& parentWindow)
	{
		if(parentWindow != nullptr)
		{
			UINT64 hWnd;
			parentWindow->getCustomAttribute("WINDOW", &hWnd);
			desc.platformSpecific["parentWindowHandle"] = toString(hWnd);
		}

		// Create the window
#if BS_PLATFORM == BS_PLATFORM_WIN32
		Win32RenderWindow* renderWindow = new (bs_alloc<Win32RenderWindow>()) Win32RenderWindow(desc, windowId);
		return bs_core_ptr<Win32RenderWindow>(renderWindow);
#elif BS_PLATFORM == BS_PLATFORM_LINUX
		LinuxRenderWindow* renderWindow = new (bs_alloc<LinuxRenderWindow>()) LinuxRenderWindow(desc, windowId);
		return bs_core_ptr<LinuxRenderWindow>(renderWindow);
#endif
	}
}