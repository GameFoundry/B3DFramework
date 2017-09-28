//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include <X11/Xutil.h>
#include "Platform/BsSplashScreen.h"
#include "Resources/BsBuiltinResources.h"
#include "Utility/BsTimer.h"
#include "CoreThread/BsCoreThread.h"

#if BS_PLATFORM == BS_PLATFORM_WIN32
#include "Win32/BsWin32Platform.h"
#include "Win32/BsWin32Window.h"

namespace bs
{
	struct SplashScreen::Pimpl
	{
		Win32Window* window = nullptr;
		Timer timer;
	};

	// Note: Never freed, but that's fine
	SplashScreen::Pimpl* SplashScreen::m = bs_new<Pimpl>();
	const UINT32 SplashScreen::SPLASH_SCREEN_DURATION_MS = 1000;

	void SplashScreen::show()
	{
		gCoreThread().queueCommand(&SplashScreen::create, CTQF_InternalQueue);
	}

	void SplashScreen::hide()
	{
		gCoreThread().queueCommand(&SplashScreen::destroy, CTQF_InternalQueue);
	}

	void SplashScreen::create()
	{
		if (m->window != nullptr)
			return;

		WINDOW_DESC windowDesc;
		windowDesc.border = WindowBorder::None;
		windowDesc.width = 543;
		windowDesc.height = 680;
		windowDesc.left = -1;
		windowDesc.top = -1;
		windowDesc.title = "Banshee Splash";
		windowDesc.toolWindow = true;
		windowDesc.alphaBlending = true;
		windowDesc.wndProc = Win32Platform::_win32WndProc;

		SPtr<PixelData> splashPixelData = BuiltinResources::getSplashScreen();
		if (splashPixelData == nullptr)
			return;

		Vector<Color> pixels = splashPixelData->getColors();
		windowDesc.backgroundPixels = (Color*)pixels.data();
		windowDesc.backgroundWidth = splashPixelData->getWidth();
		windowDesc.backgroundHeight = splashPixelData->getHeight();

		m->window = bs_new<Win32Window>(windowDesc);
		m->timer.reset();
	}

	void SplashScreen::destroy()
	{
		if (m->window == nullptr)
			return;

		UINT64 currentTime = m->timer.getMilliseconds();
		if (currentTime < SPLASH_SCREEN_DURATION_MS)
			BS_THREAD_SLEEP(SPLASH_SCREEN_DURATION_MS - currentTime);

		bs_delete(m->window);
		m->window = nullptr;
	}
}

#elif BS_PLATFORM == BS_PLATFORM_LINUX
#include "Linux/BsLinuxPlatform.h"
#include "Linux/BsLinuxWindow.h"

namespace bs
{
	struct SplashScreen::Pimpl
	{
		LinuxWindow* window = nullptr;
		Timer timer;
	};

	// Note: Never freed, but that's fine
	SplashScreen::Pimpl* SplashScreen::m = bs_new<Pimpl>();
	const UINT32 SplashScreen::SPLASH_SCREEN_DURATION_MS = 1000;

	void SplashScreen::show()
	{
		gCoreThread().queueCommand(&SplashScreen::create, CTQF_InternalQueue);
	}

	void SplashScreen::hide()
	{
		gCoreThread().queueCommand(&SplashScreen::destroy, CTQF_InternalQueue);
	}

	void SplashScreen::create()
	{
		if (m->window != nullptr)
			return;

		WINDOW_DESC windowDesc;
		windowDesc.width = 543;
		windowDesc.height = 680;
		windowDesc.x = -1;
		windowDesc.y = -1;
		windowDesc.title = "Banshee Splash";
		windowDesc.showDecorations = false;
		windowDesc.allowResize = false;

		SPtr<PixelData> splashPixelData = BuiltinResources::getSplashScreen();
		if (splashPixelData == nullptr)
			return;

		windowDesc.background = splashPixelData;

		LinuxPlatform::lockX();
		windowDesc.screen = XDefaultScreen(LinuxPlatform::getXDisplay());
		windowDesc.visualInfo.depth = XDefaultDepth(LinuxPlatform::getXDisplay(), windowDesc.screen);
		windowDesc.visualInfo.visual = XDefaultVisual(LinuxPlatform::getXDisplay(), windowDesc.screen);

		m->window = bs_new<LinuxWindow>(windowDesc);
		LinuxPlatform::unlockX();

		m->timer.reset();
	}

	void SplashScreen::destroy()
	{
		if (m->window == nullptr)
			return;

		UINT64 currentTime = m->timer.getMilliseconds();
		if (currentTime < SPLASH_SCREEN_DURATION_MS)
			BS_THREAD_SLEEP(SPLASH_SCREEN_DURATION_MS - currentTime);

		LinuxPlatform::lockX();
		bs_delete(m->window);
		LinuxPlatform::unlockX();

		m->window = nullptr;
	}
}

#else

static_assert("Missing SplashScreen implementation.");

#endif