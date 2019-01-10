//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Managers/BsRenderWindowManager.h"
#include "Platform/BsPlatform.h"
#include "BsCoreApplication.h"

using namespace std::placeholders;

namespace bs
{
	SPtr<RenderWindow> RenderWindowManager::create(RENDER_WINDOW_DESC& desc, SPtr<RenderWindow> parentWindow)
	{
		UINT32 id = ct::RenderWindowManager::instance().mNextWindowId.fetch_add(1, std::memory_order_relaxed);

		SPtr<RenderWindow> renderWindow = createImpl(desc, id, parentWindow);
		renderWindow->_setThisPtr(renderWindow);
		
		{
			Lock lock(mWindowMutex);

			mWindows[renderWindow->mWindowId] = renderWindow.get();
		}

		if (renderWindow->getProperties().isModal)
			mModalWindowStack.push_back(renderWindow.get());

		renderWindow->initialize();
		
		return renderWindow;
	}

	void RenderWindowManager::notifyWindowDestroyed(RenderWindow* window)
	{
		{
			Lock lock(mWindowMutex);

			auto iterFind = std::find(begin(mMovedOrResizedWindows), end(mMovedOrResizedWindows), window);

			if(iterFind != mMovedOrResizedWindows.end())
				mMovedOrResizedWindows.erase(iterFind);

			if (mNewWindowInFocus == window)
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

	void RenderWindowManager::notifyFocusReceived(ct::RenderWindow* coreWindow)
	{
		Lock lock(mWindowMutex);

		RenderWindow* window = getNonCore(coreWindow);
		mNewWindowInFocus = window;
	}

	void RenderWindowManager::notifyFocusLost(ct::RenderWindow* coreWindow)
	{
		Lock lock(mWindowMutex);

		mNewWindowInFocus = nullptr;
	}

	void RenderWindowManager::notifyMovedOrResized(ct::RenderWindow* coreWindow)
	{
		Lock lock(mWindowMutex);

		RenderWindow* window = getNonCore(coreWindow);
		if (window == nullptr)
			return;

		auto iterFind = std::find(begin(mMovedOrResizedWindows), end(mMovedOrResizedWindows), window);
		if (iterFind == end(mMovedOrResizedWindows))
			mMovedOrResizedWindows.push_back(window);
	}

	void RenderWindowManager::notifyMouseLeft(ct::RenderWindow* coreWindow)
	{
		Lock lock(mWindowMutex);

		RenderWindow* window = getNonCore(coreWindow);
		auto iterFind = std::find(begin(mMouseLeftWindows), end(mMouseLeftWindows), window);

		if (iterFind == end(mMouseLeftWindows))
			mMouseLeftWindows.push_back(window);
	}

	void RenderWindowManager::notifyCloseRequested(ct::RenderWindow* coreWindow)
	{
		Lock lock(mWindowMutex);

		RenderWindow* window = getNonCore(coreWindow);
		auto iterFind = std::find(begin(mCloseRequestedWindows), end(mCloseRequestedWindows), window);

		if (iterFind == end(mCloseRequestedWindows))
			mCloseRequestedWindows.push_back(window);
	}

	void RenderWindowManager::notifySyncDataDirty(ct::RenderWindow* coreWindow)
	{
		Lock lock(mWindowMutex);

		RenderWindow* window = getNonCore(coreWindow);

		if (window != nullptr)
			mDirtyProperties.insert(window);
	}

	void RenderWindowManager::_update()
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

			for (auto& dirtyPropertyWindow : mDirtyProperties)
				dirtyPropertyWindow->syncProperties();

			mDirtyProperties.clear();

			std::swap(mCloseRequestedWindows, closeRequestedWindows);
		}

		if(mWindowInFocus != newWinInFocus)
		{
			if(mWindowInFocus != nullptr)
				onFocusLost(*mWindowInFocus);

			if(newWinInFocus != nullptr)
				onFocusGained(*newWinInFocus);

			mWindowInFocus = newWinInFocus;
		}

		for (auto& window : movedOrResizedWindows)
			window->onResized();

		if (!onMouseLeftWindow.empty())
		{
			for (auto& window : mouseLeftWindows)
				onMouseLeftWindow(*window);
		}

		SPtr<RenderWindow> primaryWindow = gCoreApplication().getPrimaryWindow();
		for(auto& entry : closeRequestedWindows)
		{
			// Default behaviour for primary window is to quit the app on close
			if(entry == primaryWindow.get() && entry->onCloseRequested.empty())
			{
				gCoreApplication().quitRequested();
				continue;
			}

			entry->onCloseRequested();
		}
	}

	Vector<RenderWindow*> RenderWindowManager::getRenderWindows() const
	{
		Lock lock(mWindowMutex);

		Vector<RenderWindow*> windows;
		for (auto& windowPair : mWindows)
			windows.push_back(windowPair.second);

		return windows;
	}

	RenderWindow* RenderWindowManager::getTopMostModal() const
	{
		if (mModalWindowStack.empty())
			return nullptr;
		
		return mModalWindowStack.back();
	}

	RenderWindow* RenderWindowManager::getNonCore(const ct::RenderWindow* window) const
	{
		auto iterFind = mWindows.find(window->mWindowId);

		if (iterFind != mWindows.end())
			return iterFind->second;

		return nullptr;
	}

	namespace ct
	{

	RenderWindowManager::RenderWindowManager()
	{
		mNextWindowId = 0;
	}

	void RenderWindowManager::_update()
	{
		Lock lock(mWindowMutex);

		for (auto& dirtyPropertyWindow : mDirtyProperties)
			dirtyPropertyWindow->syncProperties();

		mDirtyProperties.clear();
	}

	void RenderWindowManager::windowCreated(RenderWindow* window)
	{
		Lock lock(mWindowMutex);

		mCreatedWindows.push_back(window);
	}

	void RenderWindowManager::windowDestroyed(RenderWindow* window)
	{
		{
			Lock lock(mWindowMutex);

			auto iterFind = std::find(begin(mCreatedWindows), end(mCreatedWindows), window);

			if (iterFind == mCreatedWindows.end())
				BS_EXCEPT(InternalErrorException, "Trying to destroy a window that is not in the created windows list.");

			mCreatedWindows.erase(iterFind);
			mDirtyProperties.erase(window);
		}
	}

	Vector<RenderWindow*> RenderWindowManager::getRenderWindows() const
	{
		Lock lock(mWindowMutex);

		return mCreatedWindows;
	}

	void RenderWindowManager::notifySyncDataDirty(RenderWindow* window)
	{
		Lock lock(mWindowMutex);

		mDirtyProperties.insert(window);
	}
	}
}