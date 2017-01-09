//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsVulkanRenderWindowManager.h"
#include "Win32/BsWin32RenderWindow.h"

namespace bs 
{
	VulkanRenderWindowManager::VulkanRenderWindowManager()
	{ }

	SPtr<RenderWindow> VulkanRenderWindowManager::createImpl(RENDER_WINDOW_DESC& desc, UINT32 windowId, const SPtr<RenderWindow>& parentWindow)
	{
		if(parentWindow != nullptr)
		{
			UINT64 hWnd;
			parentWindow->getCustomAttribute("WINDOW", &hWnd);
			desc.platformSpecific["parentWindowHandle"] = toString(hWnd);
		}

		// Create the window
		Win32RenderWindow* renderWindow = new (bs_alloc<Win32RenderWindow>()) Win32RenderWindow(desc, windowId);
		return bs_core_ptr<Win32RenderWindow>(renderWindow);
	}

	namespace ct
	{
		VulkanRenderWindowManager::VulkanRenderWindowManager(VulkanRenderAPI& renderAPI)
		:mRenderAPI(renderAPI)
	{ }

	SPtr<RenderWindowCore> VulkanRenderWindowManager::createInternal(RENDER_WINDOW_DESC& desc, UINT32 windowId)
	{
		// Create the window
		Win32RenderWindow* renderWindow =
			new (bs_alloc<Win32RenderWindow>()) Win32RenderWindow(desc, windowId, mRenderAPI);

		SPtr<Win32RenderWindow> renderWindowPtr = bs_shared_ptr<Win32RenderWindow>(renderWindow);
		renderWindowPtr->_setThisPtr(renderWindowPtr);

		windowCreated(renderWindow);

		return renderWindowPtr;
	}
	}
}