//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DUtilityPrerequisites.h"
#include "windows.h"
#include "Math/B3DSize2.h"
#include "Math/B3DVector2.h"
#include "Math/B3DArea2.h"

namespace b3d
{
	/** @addtogroup Internal-Utility
	 *  @{
	 */

	/** @addtogroup Platform-Utility-Internal
	 *  @{
	 */

	/**	Descriptor used for creating a platform specific native window. */
	struct B3D_EXPORT WindowCreateInformation
	{
		WindowCreateInformation() = default;

		HINSTANCE Module = nullptr; /**< Instance to the local module. */
		HMONITOR Monitor = nullptr; /**< Handle ot the monitor onto which to display the window. */
		HWND Parent = nullptr; /**< Optional handle to the parent window if this window is to be a child of an existing window. */
		HWND External = nullptr; /**< Optional external window handle if the window was created externally. */
		void* CreationParams = nullptr; /**< Parameter that will be passed through the WM_CREATE message. */
		Size2I Size{kZeroTag}; /**< Size of the window area in pixels. This is client area if OuterDimensions if false, otherwise full window area. */
		bool Fullscreen = false; /**< Should the window be opened in fullscreen mode. */
		bool Hidden = false; /**< Should the window be hidden initially. */
		Vector2I Position{-1, -1}; /**< Position of the top-left corner of the window in pixels. -1 == screen center. Relative to provided monitor. This is position of the client area if OuterDimensions is false, otherwise position of the full window area. */
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
	class B3D_EXPORT Win32Window
	{
	public:
		Win32Window(const WindowCreateInformation& createInformation);
		~Win32Window();

		/** Initializes the window. To be called right after construction. */
		void Initialize();

		/**	Returns the area of the window relative to the screen. This includes the title-bar and border. */
		const Area2I& GetWindowArea() const;

		/**	Returns the area of the window's client area relative to the screen. This doesn't include the title-bar and border. */
		const Area2I& GetClientArea() const;

		/** Returns currently set DPI for the window. */
		UINT GetDPI() const;

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

		/** Change the size of the window. Note the dimensions represent the dimension of the client area. */
		void Resize(const Size2I& size);

		/**	Reposition the window. Note the coordinates represent the top-left corner of the window, not the client area. */
		void Move(const Vector2I& position);

		/**	Converts screen position into window local position. */
		Vector2I ScreenToWindowPosition(const Vector2I& screenPos) const;

		/**	Converts window local position to screen position. */
		Vector2I WindowToScreenPosition(const Vector2I& windowPos) const;

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
} // namespace b3d
