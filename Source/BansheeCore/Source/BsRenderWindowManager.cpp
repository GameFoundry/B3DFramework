//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsRenderWindowManager.h"
#include "BsPlatform.h"

using namespace std::placeholders;

namespace bs
{
	RenderWindowManager::RenderWindowManager()
		:mWindowInFocus(nullptr), mNewWindowInFocus(nullptr)
	{
		Platform::onMouseLeftWindow.connect(std::bind(&RenderWindowManager::windowMouseLeft, this, _1));
	}

	RenderWindowManager::~RenderWindowManager()
	{

	}

	SPtr<RenderWindow> RenderWindowManager::create(RENDER_WINDOW_DESC& desc, SPtr<RenderWindow> parentWindow)
	{
		UINT32 id = ct::RenderWindowManager::instance().mNextWindowId.fetch_add(1, std::memory_order_relaxed);

		SPtr<RenderWindow> renderWindow = createImpl(desc, id, parentWindow);
		renderWindow->_setThisPtr(renderWindow);
		
		{
			Lock lock(mWindowMutex);

			mWindows[renderWindow->mWindowId] = renderWindow.get();
		}

		renderWindow->initialize();
		
		return renderWindow;
	}

	void RenderWindowManager::notifyWindowDestroyed(RenderWindow* window)
	{
		{
			Lock lock(mWindowMutex);

			auto iterFind = std::find_if(begin(mMovedOrResizedWindows), end(mMovedOrResizedWindows), 
				[&](const MoveOrResizeData& x) { return x.window == window; });

			if(iterFind != mMovedOrResizedWindows.end())
				mMovedOrResizedWindows.erase(iterFind);

			if (mNewWindowInFocus == window)
				mNewWindowInFocus = nullptr;

			mWindows.erase(window->mWindowId);
			mDirtyProperties.erase(window);
		}
	}

	void RenderWindowManager::notifyFocusReceived(ct::RenderWindowCore* coreWindow)
	{
		Lock lock(mWindowMutex);

		RenderWindow* window = getNonCore(coreWindow);
		mNewWindowInFocus = window;
	}

	void RenderWindowManager::notifyFocusLost(ct::RenderWindowCore* coreWindow)
	{
		Lock lock(mWindowMutex);

		mNewWindowInFocus = nullptr;
	}

	void RenderWindowManager::notifyMovedOrResized(ct::RenderWindowCore* coreWindow)
	{
		Lock lock(mWindowMutex);

		RenderWindow* window = getNonCore(coreWindow);
		if (window == nullptr)
			return;

		auto iterFind = std::find_if(begin(mMovedOrResizedWindows), end(mMovedOrResizedWindows), 
			[&](const MoveOrResizeData& x) { return x.window == window; });

		const RenderWindowProperties& props = coreWindow->getProperties();
		MoveOrResizeData* moveResizeData = nullptr;

		if (iterFind != end(mMovedOrResizedWindows))
		{
			moveResizeData = &*iterFind;
		}
		else
		{
			MoveOrResizeData newEntry;
			newEntry.window = window;

			mMovedOrResizedWindows.push_back(newEntry);
			moveResizeData = &mMovedOrResizedWindows.back();
		}
		
		moveResizeData->x = props.getLeft();
		moveResizeData->y = props.getTop();
		moveResizeData->width = props.getWidth();
		moveResizeData->height = props.getHeight();
	}

	void RenderWindowManager::notifySyncDataDirty(ct::RenderWindowCore* coreWindow)
	{
		Lock lock(mWindowMutex);

		RenderWindow* window = getNonCore(coreWindow);

		if (window != nullptr)
			mDirtyProperties.insert(window);
	}

	void RenderWindowManager::windowMouseLeft(ct::RenderWindowCore* coreWindow)
	{
		Lock lock(mWindowMutex);

		RenderWindow* window = getNonCore(coreWindow);
		auto iterFind = std::find(begin(mMouseLeftWindows), end(mMouseLeftWindows), window);

		if (iterFind == end(mMouseLeftWindows))
			mMouseLeftWindows.push_back(window);
	}

	void RenderWindowManager::_update()
	{
		RenderWindow* newWinInFocus = nullptr;
		Vector<MoveOrResizeData> movedOrResizedWindows;
		Vector<RenderWindow*> mouseLeftWindows;

		{
			Lock lock(mWindowMutex);
			newWinInFocus = mNewWindowInFocus;

			for (auto& moveResizeData : mMovedOrResizedWindows)
			{
				RenderWindow* window = moveResizeData.window;
				const RenderWindowProperties& props = window->getProperties();

				// Need to eliminate non-dirty ones because it's possible we already triggered the resize event
				// if the resize call originated from the sim thread, so we don't trigger it twice.

				bool isDirty = moveResizeData.x != props.getLeft() || moveResizeData.y != props.getTop()
					|| moveResizeData.width != props.getWidth() || moveResizeData.height != props.getHeight();

				if (isDirty)
					movedOrResizedWindows.push_back(moveResizeData);
			}

			mMovedOrResizedWindows.clear();

			mouseLeftWindows = mMouseLeftWindows;
			mMouseLeftWindows.clear();

			for (auto& dirtyPropertyWindow : mDirtyProperties)
				dirtyPropertyWindow->syncProperties();

			mDirtyProperties.clear();
		}

		if(mWindowInFocus != newWinInFocus)
		{
			if(mWindowInFocus != nullptr)
				onFocusLost(*mWindowInFocus);

			if(newWinInFocus != nullptr)
				onFocusGained(*newWinInFocus);

			mWindowInFocus = newWinInFocus;
		}

		for (auto& moveResizeData : movedOrResizedWindows)
		{
			moveResizeData.window->onResized();
		}

		if (!onMouseLeftWindow.empty())
		{
			for (auto& window : mouseLeftWindows)
				onMouseLeftWindow(*window);
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

	RenderWindow* RenderWindowManager::getNonCore(const ct::RenderWindowCore* window) const
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

	SPtr<RenderWindowCore> RenderWindowManager::create(RENDER_WINDOW_DESC& desc)
	{
		UINT32 id = mNextWindowId.fetch_add(1, std::memory_order_relaxed);

		SPtr<RenderWindowCore> renderWindow = createInternal(desc, id);
		renderWindow->initialize();

		return renderWindow;
	}

	void RenderWindowManager::_update()
	{
		Lock lock(mWindowMutex);

		for (auto& dirtyPropertyWindow : mDirtyProperties)
			dirtyPropertyWindow->syncProperties();

		mDirtyProperties.clear();
	}

	void RenderWindowManager::windowCreated(RenderWindowCore* window)
	{
		Lock lock(mWindowMutex);

		mCreatedWindows.push_back(window);
	}

	void RenderWindowManager::windowDestroyed(RenderWindowCore* window)
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

	Vector<RenderWindowCore*> RenderWindowManager::getRenderWindows() const
	{
		Lock lock(mWindowMutex);

		return mCreatedWindows;
	}

	void RenderWindowManager::notifySyncDataDirty(RenderWindowCore* window)
	{
		Lock lock(mWindowMutex);

		mDirtyProperties.insert(window);
	}
	}
}