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

	/**
	 * Base implementation of a window that when open doesn't allow you to interact with other windows. Modal windows are
	 * similar to editor windows but cannot be docked, and are meant to be used for temporary operations like dialog boxes
	 * and progress bars.
	 */
	class BS_ED_EXPORT ModalWindow : public EditorWindowBase
	{
	public:
		virtual ~ModalWindow();

		/** @copydoc EditorWindowBase::update */
		virtual void update() override;

		/** @copydoc EditorWindowBase::close */
		virtual void close() override;

		/**	Changes the text in the modal window title bar. */
		void setTitle(const HString& title);

		/** Converts screen pointer coordinates into coordinates relative to the window content's GUI panel. */
		Vector2I screenToWindowPos(const Vector2I& screenPos) const;

		/** Converts pointer coordinates relative to the window content's GUI panel into screen coordinates. */
		Vector2I windowToScreenPos(const Vector2I& windowPos) const;

	protected:
		friend class EditorWindowManager;

		ModalWindow(const HString& title, bool hasCloseButton = false);

		/**
		 * Returns the area in which the GUI contents are displayed (not including title bar and other default 
		 * elements). Area is relative to window.
		 */
		Rect2I getContentArea() const;

		/** @copydoc EditorWindowBase::resized */
		virtual void resized() override;

	private:
		/**
		 * Updates the placement of child GUI elements and their non-client areas (used for OS move/resize operations). 
		 * Should be called after window size changes.
		 */
		void updateSize();

		/**	Returns the height in pixels taken up by the title bar. */
		UINT32 getTitleBarHeight() const;

		GUIPanel* mTitleBarPanel;
		GUIPanel* mTitleBarBgPanel;

		GUILabel* mTitle;
		GUIButton* mCloseButton;
		GUITexture* mTitleBarBg;

	protected:
		GUIPanel* mContents;
	};

	/** @} */
}