//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsEditorPrerequisites.h"
#include "BsEditorWindowBase.h"

namespace bs
{
	/** @addtogroup EditorWindow
	 *  @{
	 */

	/**	Primary editor window, containing the menu bar, status bar and the dock area. */
	class BS_ED_EXPORT MainEditorWindow : public EditorWindowBase
	{
	public:
		~MainEditorWindow();

		/** @copydoc EditorWindowBase::update */
		void update() override;

		/** @copydoc EditorWindowBase::isMain */
		bool isMain() const override { return true; }

		/**	Gets the DockManager that is responsible for docking and placement of EditorWidget%s on the main window. */
		DockManager& getDockManager() const { return *mDockManager; }

		/**	Gets the primary menu bar element. */
		GUIMenuBar& getMenuBar() const { return *mMenuBar; }

		/**	Gets status bar GUI element. */
		GUIStatusBar& getStatusBar() const { return *mStatusBar; }

		/**
		 * Creates a new main editor window. If one is already open this method will return the existing one.
		 *
		 * @param[in]	renderWindow	Previously created render window to initialize the main editor window in.
		 * @return						Instance of the main editor window.
		 */
		static MainEditorWindow* create(const SPtr<RenderWindow>& renderWindow);
	protected:
		friend class EditorWindowManager;
		MainEditorWindow(const SPtr<RenderWindow>& renderWindow);

	protected:
		/** @copydoc EditorWindowBase::resized */
		void resized() override;

		/**
		 * Updates the placement of child GUI elements and their non-client areas (used for OS move/resize operations). 
		 * Should be called after window size changes.
		 */
		void updateAreas();

		static const UINT32 MENU_BAR_HEIGHT;
		static const UINT32 STATUS_BAR_HEIGHT;

		GUIMenuBar* mMenuBar;
		DockManager* mDockManager;
		GUIStatusBar* mStatusBar;
		HProfilerOverlay mProfilerOverlay;
	};

	/** @} */
}