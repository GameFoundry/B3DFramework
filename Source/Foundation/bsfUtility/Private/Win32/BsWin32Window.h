//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "Prerequisites/BsPrerequisitesUtil.h"
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
	struct B3D_UTILITY_EXPORT WindowCreateInformation
	{
		WindowCreateInformation() = default;

		HINSTANCE Module = nullptr; /**< Instance to the local module. */
		HMONITOR Monitor = nullptr; /**< Handle ot the monitor onto which to display the window. */
		HWND Parent = nullptr; /**< Optional handle to the parent window if this window is to be a child of an existing window. */
		HWND External = nullptr; /**< Optional external window handle if the window was created externally. */
		void* CreationParams = nullptr; /**< Parameter that will be passed through the WM_CREATE message. */
		u32 Width = 0; /**< Width of the window in pixels. */
		u32 Height = 0; /**< Height of the window in pixels. */
		bool Fullscreen = false; /**< Should the window be opened in fullscreen mode. */
		bool Hidden = false; /**< Should the window be hidden initially. */
		i32 Left = -1; /**< Window origin on X axis in pixels. -1 == screen center. Relative to provided monitor. */
		i32 Top = -1; /**< Window origin on Y axis in pixels. -1 == screen center. Relative to provided monitor. */
		String Title; /**< Title of the window. */
		bool ShowTitleBar = true; /**< Determines if the title-bar should be shown or not. */
		bool ShowBorder = true; /**< Determines if the window border should be shown or not. */
		bool AllowResize = true; /**< Determines if the user can resize the window by dragging on the window edges. */
		bool OuterDimensions = false; /**< Do our dimensions include space for things like title-bar and border. */
		bool EnableDoubleClick = true; /**< Does window accept double-clicks. */
		bool ToolWindow = false; /**< Tool windows have a different look than normal windows and have no task bar entry. */
		/**
		 * Optional background image to apply to the window. This must be a buffer of size
		 * backgroundWidth * backgroundHeight.
		 */
		Color* BackgroundPixels = nullptr;
		u32 BackgroundWidth = 0; /** Width of the background image. Only relevant if backgroundPixels is not null. */
		u32 BackgroundHeight = 0; /** Width of the background image. Only relevant if backgroundPixels is not null. */
		/** If true the window will support transparency based on the alpha channel of the background image. */
		bool AlphaBlending = false;
		bool Modal = false; /**< When a modal window is open all other windows will be locked until modal window is closed. */
		WNDPROC WndProc = nullptr; /**< Pointer to a function that handles windows message processing. */
	};

	/**	Represents a Windows native window. */
	class B3D_UTILITY_EXPORT Win32Window
	{
	public:
		Win32Window(const WindowCreateInformation& createInformation);
		~Win32Window();

		/** Initializes the window. To be called right after construction. */
		void Initialize();

		/**	Returns position of the left-most border of the window, relative to the screen. */
		i32 GetLeft() const;

		/**	Returns position of the top-most border of the window, relative to the screen. */
		i32 GetTop() const;

		/**	Returns width of the window in pixels. */
		u32 GetWidth() const;

		/**	Returns height of the window in pixels. */
		u32 GetHeight() const;

		/**	Returns the native window handle. */
		HWND GetHWnd() const;

		/**	Hide or show the window. */
		void SetHidden(bool hidden);

		/**	Restores or minimizes the window. */
		void SetActive(bool state);

		/**	Minimizes the window to the taskbar. */
		void Minimize();

		/**	Maximizes the window over the entire current screen. */
		void Maximize();

		/**	Restores the window to original position and size if it is minimized or maximized. */
		void Restore();

		/**	Change the size of the window. */
		void Resize(u32 width, u32 height);

		/**	Reposition the window. */
		void Move(i32 left, i32 top);

		/**	Converts screen position into window local position. */
		Vector2I ScreenToWindowPos(const Vector2I& screenPos) const;

		/**	Converts window local position to screen position. */
		Vector2I WindowToScreenPos(const Vector2I& windowPos) const;

		/**	Returns the window style flags used for creating it. */
		DWORD GetStyle() const;

		/**	Returns the extended window style flags used for creating it. */
		DWORD GetStyleEx() const;

		/**
		 * @name Internal
		 * @{
		 */

		/** Called when window is moved or resized externally. */
		void DoOnWindowMovedOrResized();

		/**
		 * Enables all open windows. Enabled windows can receive mouse and keyboard input. This includes even windows
		 * disabled because there is a modal window on top of them.
		 */
		void static EnableAllWindowsInternal();

		/**
		 * Restores disabled state of all windows that were disabled due to modal windows being on top of them. Companion
		 * method to EnableAllWindowsInternal() that can help restore original state after it is called.
		 */
		void static RestoreModalWindowsInternal();

		/** @} */

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
} // namespace bs
