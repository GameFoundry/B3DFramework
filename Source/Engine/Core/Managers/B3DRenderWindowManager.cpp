//************************************ B3D Framework - Copyright 2026 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Managers/B3DRenderWindowManager.h"

#include "B3DApplication.h"
#include "Platform/B3DPlatform.h"
#include "RenderAPI/B3DHeadlessRenderWindow.h"

#if B3D_PLATFORM_WIN32
#	include "Private/Win32/B3DWin32RenderWindow.h"
#elif B3D_PLATFORM_LINUX
#	include "Private/Linux/B3DLinuxRenderWindow.h"
#elif B3D_PLATFORM_MACOS
#	include "Private/MacOS/B3DMacOSRenderWindow.h"
#endif

using namespace b3d;

SPtr<RenderWindow> RenderWindowManager::CreateRenderWindow(const RenderWindowCreateInformation& createInformation, const SPtr<RenderWindow>& parentWindow)
{
	const u32 id = mNextWindowId++;

	SPtr<RenderWindow> renderWindow;

	// Check if headless mode is requested
	if(createInformation.Headless)
	{
		renderWindow = B3DMakeShared<HeadlessRenderWindow>(createInformation, id, parentWindow);
	}
	else
	{
#if B3D_PLATFORM_WIN32
		renderWindow = B3DMakeShared<Win32RenderWindow>(createInformation, id, parentWindow);
#elif B3D_PLATFORM_LINUX
		renderWindow = B3DMakeShared<LinuxRenderWindow>(createInformation, id, parentWindow);
#elif B3D_PLATFORM_MACOS
		renderWindow = B3DMakeShared<MacOSRenderWindow>(createInformation, id, parentWindow);
#endif
	}

	renderWindow->SetShared(renderWindow);
	mWindows[renderWindow->mWindowId] = renderWindow.get();

	if(renderWindow->GetRenderWindowProperties().IsModal)
		mModalWindowStack.push_back(renderWindow.get());

	renderWindow->Initialize();
	return renderWindow;
}

void RenderWindowManager::NotifyWindowDestroyed(RenderWindow& window)
{
	if(mWindowInFocus == &window)
		mWindowInFocus = nullptr;

	mWindows.erase(window.mWindowId);

	auto found = std::find(begin(mModalWindowStack), end(mModalWindowStack), &window);
	if(found != mModalWindowStack.end())
		mModalWindowStack.erase(found);
}

void RenderWindowManager::NotifyFocusReceived(RenderWindow& window)
{
	if(mWindowInFocus != &window)
	{
		if(mWindowInFocus != nullptr)
			OnFocusLost(*mWindowInFocus);

		OnFocusGained(window);
		mWindowInFocus = &window;
	}
}

void RenderWindowManager::NotifyFocusLost(RenderWindow& window)
{
	if(mWindowInFocus != nullptr)
	{
		OnFocusLost(*mWindowInFocus);
		mWindowInFocus = nullptr;
	}
}

void RenderWindowManager::NotifyMouseLeft(RenderWindow& window)
{
	OnMouseLeftWindow(window);
}

void RenderWindowManager::RequestShowWindow(u32 windowId, bool show)
{
	SignalEvent event;
	auto fnShowWindow = [windowId, show, &event, this]
	{
		auto found = mWindows.find(windowId);
		if(found == mWindows.end())
			return;

		if(show)
			found->second->Show();
		else
			found->second->Hide();

		event.Signal();
	};

	if(GetApplication().GetMainThreadId() == B3D_CURRENT_THREAD_ID)
		fnShowWindow();
	else
	{
		GetApplication().GetMainThreadScheduler().Post(SchedulerTask(std::move(fnShowWindow), "Show render window"));

		// Make sure to wait for the message to be processed by the main thread, because if we present an image onto a hidden window it will get lost (at least on Win32).
		event.Wait();
	}
}

void RenderWindowManager::Update()
{
	// TODO - Can be removed, but keeping it for now in case there are ordering issues with render thread sync
}

Vector<RenderWindow*> RenderWindowManager::GetRenderWindows() const
{
	Vector<RenderWindow*> windows;
	for(auto& windowPair : mWindows)
		windows.push_back(windowPair.second);

	return windows;
}

RenderWindow* RenderWindowManager::GetTopMostModal() const
{
	if(mModalWindowStack.empty())
		return nullptr;

	return mModalWindowStack.back();
}
