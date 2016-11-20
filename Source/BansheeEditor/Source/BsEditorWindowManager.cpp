//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsEditorWindowManager.h"
#include "BsEditorWindow.h"
#include "BsMainEditorWindow.h"

namespace bs
{
	EditorWindowManager::EditorWindowManager()
		:mMainWindow(nullptr)
	{

	}

	EditorWindowManager::~EditorWindowManager()
	{
		while(mEditorWindows.size() > 0)
			destroy(mEditorWindows[0]);

		for (auto& windowToDestroy : mScheduledForDestruction)
			bs_delete(windowToDestroy);

		mScheduledForDestruction.clear();

		if(mMainWindow != nullptr)
			bs_delete(mMainWindow);
	}

	MainEditorWindow* EditorWindowManager::createMain(const SPtr<RenderWindow>& parentRenderWindow)
	{
		if(mMainWindow == nullptr)
			mMainWindow = new (bs_alloc<MainEditorWindow>()) MainEditorWindow(parentRenderWindow);

		return mMainWindow;
	}

	EditorWindow* EditorWindowManager::create()
	{
		EditorWindow* newWindow = new (bs_alloc<EditorWindow>()) EditorWindow();
		mEditorWindows.push_back(newWindow);

		newWindow->initialize();
		return newWindow;
	}

	void EditorWindowManager::registerWindow(EditorWindowBase* window)
	{
		mEditorWindows.push_back(window);
	}

	void EditorWindowManager::destroy(EditorWindowBase* window)
	{
		auto iterFind = std::find(begin(mEditorWindows), end(mEditorWindows), window);

		if(iterFind == end(mEditorWindows))
			return;

		auto iterFind2 = std::find(begin(mScheduledForDestruction), end(mScheduledForDestruction), window);
		
		if(iterFind2 == end(mScheduledForDestruction))
			mScheduledForDestruction.push_back(window);

		mEditorWindows.erase(iterFind);
	}

	void EditorWindowManager::update()
	{
		// Editor window destroy is deferred to this point, otherwise we risk
		// destroying a window while it's still being used (situation that was happening with GUIManager)
		
		for(auto& windowToDestroy : mScheduledForDestruction)
		{
			bs_delete(windowToDestroy);
		}

		mScheduledForDestruction.clear();

		// Make a copy since other editors might be opened/closed from editor update() methods
		mEditorWindowsSnapshot = mEditorWindows; 
		mMainWindow->update();

		for (auto& window : mEditorWindowsSnapshot)
		{
			window->update();
		}
	}

	bool EditorWindowManager::hasFocus() const
	{
		if (mMainWindow->hasFocus())
			return true;

		for(auto& window : mEditorWindows)
		{
			if (window->hasFocus())
				return true;
		}

		return false;
	}
}