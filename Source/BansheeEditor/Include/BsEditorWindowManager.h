//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsEditorPrerequisites.h"
#include "BsModule.h"
#include "BsEvent.h"

namespace bs
{
	/** @addtogroup EditorWindow-Internal
	 *  @{
	 */

	/** Manages that handles creation, destruction and updates of editor windows. */
	class BS_ED_EXPORT EditorWindowManager : public Module<EditorWindowManager>
	{
	public:
		EditorWindowManager();
		~EditorWindowManager();

		/**
		 * Creates the main editor window using a previously created render window. If a main window already exists, this
		 * will return the existing instance.
		 */
		MainEditorWindow* createMain(const SPtr<RenderWindow>& parentRenderWindow);

		/**	Creates a new editor window. There is no limit on the number of editor windows. */
		EditorWindow* create();

		/**	Notifies the manager that a new editor window was created. */
		void registerWindow(EditorWindowBase* window);

		/**	Schedules the window for destruction. Actual destruction will happen on next update. */
		void destroy(EditorWindowBase* window);

		/**	Returns the main editor window, or null if one doesn't exist. */
		MainEditorWindow* getMainWindow() const { return mMainWindow; }

		/**	Update to be called once per frame. Calls update on all active editor windows. */
		void update();

		/**	Checks if any editor window has keyboard focus. */
		bool hasFocus() const;
	protected:
		MainEditorWindow* mMainWindow;

		Vector<EditorWindowBase*> mEditorWindows;
		Vector<EditorWindowBase*> mScheduledForDestruction;

		Vector<EditorWindowBase*> mEditorWindowsSnapshot;
	};

	/** @} */
}