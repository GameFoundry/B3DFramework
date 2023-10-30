//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Managers/BsRenderWindowManager.h"
#include "Platform/BsPlatform.h"
#include "BsCoreApplication.h"

using namespace std::placeholders;

using namespace bs;

SPtr<RenderWindow> RenderWindowManager::Create(RENDER_WINDOW_DESC& desc, SPtr<RenderWindow> parentWindow)
{
	u32 id = ct::RenderWindowManager::Instance().mNextWindowId.fetch_add(1, std::memory_order_relaxed);

	SPtr<RenderWindow> renderWindow = CreateImpl(desc, id, parentWindow);
	renderWindow->SetShared(renderWindow);

	{
		Lock lock(mWindowMutex);

		mWindows[renderWindow->mWindowId] = renderWindow.get();
	}

	if(renderWindow->GetProperties().IsModal)
		mModalWindowStack.push_back(renderWindow.get());

	renderWindow->Initialize();

	return renderWindow;
}

void RenderWindowManager::NotifyWindowDestroyed(RenderWindow* window)
{
	{
		Lock lock(mWindowMutex);

		auto iterFind = std::find(begin(mMovedOrResizedWindows), end(mMovedOrResizedWindows), window);

		if(iterFind != mMovedOrResizedWindows.end())
			mMovedOrResizedWindows.erase(iterFind);

		if(mNewWindowInFocus == window)
			mNewWindowInFocus = nullptr;

		mWindows.erase(window->mWindowId);
		mDirtyProperties.erase(window);
	}

	{
		auto iterFind = std::find(begin(mModalWindowStack), end(mModalWindowStack), window);
		if(iterFind != mModalWindowStack.end())
			mModalWindowStack.erase(iterFind);
	}
}

void RenderWindowManager::NotifyFocusReceived(ct::RenderWindow* renderProxy)
{
	Lock lock(mWindowMutex);

	RenderWindow* window = GetCoreObject(renderProxy);
	mNewWindowInFocus = window;
}

void RenderWindowManager::NotifyFocusLost(ct::RenderWindow* renderProxy)
{
	Lock lock(mWindowMutex);

	mNewWindowInFocus = nullptr;
}

void RenderWindowManager::NotifyMovedOrResized(ct::RenderWindow* renderProxy)
{
	Lock lock(mWindowMutex);

	RenderWindow* window = GetCoreObject(renderProxy);
	if(window == nullptr)
		return;

	auto iterFind = std::find(begin(mMovedOrResizedWindows), end(mMovedOrResizedWindows), window);
	if(iterFind == end(mMovedOrResizedWindows))
		mMovedOrResizedWindows.push_back(window);
}

void RenderWindowManager::NotifyMouseLeft(ct::RenderWindow* renderProxy)
{
	Lock lock(mWindowMutex);

	RenderWindow* window = GetCoreObject(renderProxy);
	auto iterFind = std::find(begin(mMouseLeftWindows), end(mMouseLeftWindows), window);

	if(iterFind == end(mMouseLeftWindows))
		mMouseLeftWindows.push_back(window);
}

void RenderWindowManager::NotifyCloseRequested(ct::RenderWindow* renderProxy)
{
	Lock lock(mWindowMutex);

	RenderWindow* window = GetCoreObject(renderProxy);
	auto iterFind = std::find(begin(mCloseRequestedWindows), end(mCloseRequestedWindows), window);

	if(iterFind == end(mCloseRequestedWindows))
		mCloseRequestedWindows.push_back(window);
}

void RenderWindowManager::NotifySyncDataDirty(ct::RenderWindow* renderProxy)
{
	Lock lock(mWindowMutex);

	RenderWindow* window = GetCoreObject(renderProxy);

	if(window != nullptr)
		mDirtyProperties.insert(window);
}

void RenderWindowManager::UpdateInternal()
{
	RenderWindow* newWinInFocus = nullptr;
	Vector<RenderWindow*> movedOrResizedWindows;
	Vector<RenderWindow*> mouseLeftWindows;
	Vector<RenderWindow*> closeRequestedWindows;

	{
		Lock lock(mWindowMutex);
		newWinInFocus = mNewWindowInFocus;

		std::swap(mMovedOrResizedWindows, movedOrResizedWindows);
		std::swap(mMouseLeftWindows, mouseLeftWindows);

		for(auto& dirtyPropertyWindow : mDirtyProperties)
			dirtyPropertyWindow->SyncProperties();

		mDirtyProperties.clear();

		std::swap(mCloseRequestedWindows, closeRequestedWindows);
	}

	if(mWindowInFocus != newWinInFocus)
	{
		if(mWindowInFocus != nullptr)
			OnFocusLost(*mWindowInFocus);

		if(newWinInFocus != nullptr)
			OnFocusGained(*newWinInFocus);

		mWindowInFocus = newWinInFocus;
	}

	for(auto& window : movedOrResizedWindows)
		window->OnResized();

	if(!OnMouseLeftWindow.Empty())
	{
		for(auto& window : mouseLeftWindows)
			OnMouseLeftWindow(*window);
	}

	SPtr<RenderWindow> primaryWindow = GetCoreApplication().GetPrimaryWindow();
	for(auto& entry : closeRequestedWindows)
	{
		// Default behaviour for primary window is to quit the app on close
		if(entry == primaryWindow.get() && entry->OnCloseRequested.Empty())
		{
			GetCoreApplication().QuitRequested();
			continue;
		}

		entry->OnCloseRequested();
	}
}

Vector<RenderWindow*> RenderWindowManager::GetRenderWindows() const
{
	Lock lock(mWindowMutex);

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

RenderWindow* RenderWindowManager::GetCoreObject(const ct::RenderWindow* window) const
{
	auto iterFind = mWindows.find(window->mWindowId);

	if(iterFind != mWindows.end())
		return iterFind->second;

	return nullptr;
}

namespace bs { namespace ct
{

RenderWindowManager::RenderWindowManager()
{
	mNextWindowId = 0;
}

void RenderWindowManager::UpdateInternal()
{
	Lock lock(mWindowMutex);

	for(auto& dirtyPropertyWindow : mDirtyProperties)
		dirtyPropertyWindow->SyncProperties();

	mDirtyProperties.clear();
}

void RenderWindowManager::WindowCreated(RenderWindow* window)
{
	Lock lock(mWindowMutex);

	mCreatedWindows.push_back(window);
}

void RenderWindowManager::WindowDestroyed(RenderWindow* window)
{
	{
		Lock lock(mWindowMutex);

		auto iterFind = std::find(begin(mCreatedWindows), end(mCreatedWindows), window);

		if(iterFind == mCreatedWindows.end())
			B3D_EXCEPT(InternalErrorException, "Trying to destroy a window that is not in the created windows list.");

		mCreatedWindows.erase(iterFind);
		mDirtyProperties.erase(window);
	}
}

Vector<RenderWindow*> RenderWindowManager::GetRenderWindows() const
{
	Lock lock(mWindowMutex);

	return mCreatedWindows;
}

void RenderWindowManager::NotifySyncDataDirty(RenderWindow* window)
{
	Lock lock(mWindowMutex);

	mDirtyProperties.insert(window);
}
}}
