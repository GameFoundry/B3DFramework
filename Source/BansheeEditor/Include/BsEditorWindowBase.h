//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsEditorPrerequisites.h"
#include "BsEvent.h"

namespace bs
{
	/** @addtogroup Implementation
	 *  @{
	 */

	/**
	 * A base class for all editor window. Each editor window is backed by a render window (for example the OS "window"), 
	 * and also provides a GUI widget for child elements to use. On top of that it also provides basic GUI like window
	 * background and frame, as well as setting up the OS-specific areas for resize/move operations.
	 */
	class BS_ED_EXPORT EditorWindowBase
	{
	public:
		virtual ~EditorWindowBase();

		/**	Moves the window on the desktop. Coordinates are in screen space. */
		virtual void setPosition(INT32 x, INT32 y);

		/**	Resizes the window. */
		virtual void setSize(UINT32 width, UINT32 height);

		/**	Returns the X position of the window in screen coordinates. */
		INT32 getLeft() const;

		/**	Returns the X position of the window in screen coordinates. */
		INT32 getTop() const;

		/**	Returns the width of the window in pixels. */
		UINT32 getWidth() const;

		/**	Returns the height of the window in pixels. */
		UINT32 getHeight() const;

		/**	Closes and destroys the window. */
		virtual void close();

		/**	Hides the window without closing it. */
		void hide();

		/**	Return true if this is the main editor window. */
		virtual bool isMain() const { return false; }

		/**	Called once every frame. Internal method. */
		virtual void update() { }

		/**	Returns the render window that this editor window is being rendered to. */
		SPtr<RenderWindow> getRenderWindow() const { return mRenderWindow; }

		/**	Returns the GUI widget used for displaying all GUI contents in the window. */
		HGUIWidget getGUIWidget() const { return mGUI; }

		/**	Returns the camera used for rendering the window GUI contents. */	
		HCamera getGUICamera() const { return mCamera; }

		/**	Checks whether the window currently has keyboard focus. */	
		bool hasFocus() const;

	protected:
		EditorWindowBase(bool isModal = false);
		EditorWindowBase(const SPtr<RenderWindow>& renderWindow);

		/**	Common code for constructing the object to be called from all constructors. */
		void construct(const SPtr<RenderWindow>& renderWindow);

		/**	Initializes elements that cannot be initialized in the constructor. Must be called right after construction. */
		virtual void initialize();

		/**	Callback that triggers whenever the underlying render window changes size. */
		virtual void resized() { }

		SPtr<RenderWindow> mRenderWindow;
		HSceneObject mSceneObject;
		HGUIWidget mGUI;
		HCamera mCamera;
		GameObjectHandle<WindowFrameWidget> mWindowFrame;
		bool mOwnsRenderWindow;
		bool mIsModal;
	private:
		HEvent mResizedConn;
	};

	/** @} */
}