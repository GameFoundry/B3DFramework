//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "Prerequisites/BsPrerequisitesUtil.h"
#include "Math/BsVector2I.h"
#include "windows.h"

namespace bs
{
	/** @addtogroup Internal-Utility
	 *  @{
	 */

	/** @addtogroup Platform-Utility-Internal
	 *  @{
	 */

	/**	Descriptor used for creating a platform specific native window. */
	struct BS_UTILITY_EXPORT WINDOW_DESC
	{
		WINDOW_DESC() = default;

		HINSTANCE module = nullptr; /**< Instance to the local module. */
		HMONITOR monitor = nullptr; /**< Handle ot the monitor onto which to display the window. */
		HWND parent = nullptr; /**< Optional handle to the parent window if this window is to be a child of an existing window. */
		HWND external = nullptr; /**< Optional external window handle if the window was created externally. */
		void* creationParams = nullptr; /**< Parameter that will be passed through the WM_CREATE message. */
		UINT32 width = 0; /**< Width of the window in pixels. */
		UINT32 height = 0; /**< Height of the window in pixels. */
		bool fullscreen = false; /**< Should the window be opened in fullscreen mode. */
		bool hidden = false; /**< Should the window be hidden initially. */
		INT32 left = -1; /**< Window origin on X axis in pixels. -1 == screen center. Relative to provided monitor. */
		INT32 top = -1; /**< Window origin on Y axis in pixels. -1 == screen center. Relative to provided monitor. */
		String title; /**< Title of the window. */
		bool showTitleBar = true; /**< Determines if the title-bar should be shown or not. */
		bool showBorder = true; /**< Determines if the window border should be shown or not. */
		bool allowResize = true; /**< Determines if the user can resize the window by dragging on the window edges. */
		bool outerDimensions = false; /**< Do our dimensions include space for things like title-bar and border. */
		bool enableDoubleClick = true; /**< Does window accept double-clicks. */
		bool toolWindow = false; /**< Tool windows have a different look than normal windows and have no task bar entry. */
		/**
		 * Optional background image to apply to the window. This must be a buffer of size 
		 * backgroundWidth * backgroundHeight. 
		 */
		Color* backgroundPixels = nullptr;
		UINT32 backgroundWidth = 0; /** Width of the background image. Only relevant if backgroundPixels is not null. */
		UINT32 backgroundHeight = 0; /** Width of the background image. Only relevant if backgroundPixels is not null. */
		/** If true the window will support transparency based on the alpha channel of the background image. */
		bool alphaBlending = false; 
		bool modal = false; /**< When a modal window is open all other windows will be locked until modal window is closed. */
		WNDPROC wndProc = nullptr; /**< Pointer to a function that handles windows message processing. */
	};

	/**	Represents a Windows native window. */
	class BS_UTILITY_EXPORT Win32Window
	{
	public:
		Win32Window(const WINDOW_DESC& desc);
		~Win32Window();

		/**	Returns position of the left-most border of the window, relative to the screen. */
		INT32 getLeft() const;

		/**	Returns position of the top-most border of the window, relative to the screen. */
		INT32 getTop() const;

		/**	Returns width of the window in pixels. */
		UINT32 getWidth() const;

		/**	Returns height of the window in pixels. */
		UINT32 getHeight() const;

		/**	Returns the native window handle. */
		HWND getHWnd() const;

		/**	Hide or show the window. */
		void setHidden(bool hidden);

		/**	Restores or minimizes the window. */
		void setActive(bool state);

		/**	Minimizes the window to the taskbar. */
		void minimize();

		/**	Maximizes the window over the entire current screen. */
		void maximize();

		/**	Restores the window to original position and size if it is minimized or maximized. */
		void restore();

		/**	Change the size of the window. */
		void resize(UINT32 width, UINT32 height);

		/**	Reposition the window. */
		void move(INT32 left, INT32 top);

		/**	Converts screen position into window local position. */
		Vector2I screenToWindowPos(const Vector2I& screenPos) const;

		/**	Converts window local position to screen position. */
		Vector2I windowToScreenPos(const Vector2I& windowPos) const;

		/**	Returns the window style flags used for creating it. */
		DWORD getStyle() const;

		/**	Returns the extended window style flags used for creating it. */
		DWORD getStyleEx() const;

		/** Called when window is moved or resized externally. */
		void _windowMovedOrResized();

		/** 
		 * Enables all open windows. Enabled windows can receive mouse and keyboard input. This includes even windows 
		 * disabled because there is a modal window on top of them.
		 */
		void static _enableAllWindows();

		/**
		 * Restores disabled state of all windows that were disabled due to modal windows being on top of them. Companion
		 * method to _enableAllWindows() that can help restore original state after it is called. 
		 */
		void static _restoreModalWindows();

	private:
		friend class Win32WindowManager;

		struct Pimpl;
		Pimpl* m;

		static Vector<Win32Window*> sAllWindows;
		static Vector<Win32Window*> sModalWindowStack;
		static Mutex sWindowsMutex;
	};

	/** @} */
	/** @} */
}
