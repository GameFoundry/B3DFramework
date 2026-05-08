//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "Prerequisites/B3DPrerequisitesUtil.h"
#include "Math/B3DVector2I.h"
#include <X11/X.h>
#include <X11/Xutil.h>

namespace b3d
{
	/** @addtogroup Internal-Utility
	 *  @{
	 */

	/** @addtogroup Platform-Utility-Internal
	 *  @{
	 */

	/**	Descriptor used for creating a platform specific native window. */
	struct WindowCreateInformation
	{
		i32 X, Y;
		u32 Width, Height;
		u32 Screen;
		String Title;
		bool ShowDecorations;
		bool AllowResize;
		bool Modal;
		bool ShowOnTaskBar;
		bool Hidden;
		::Window Parent;
		::Window External;
		XVisualInfo VisualInfo;
		TShared<PixelData> Background;
	};

	/**
	 * Represents a X11 window. Note that all accesses (including creation and destruction) to objects of this class must
	 * be locked by the main X11 lock accessible through LinuxPlatform.
	 */
	class B3D_EXPORT LinuxWindow
	{
	public:
		LinuxWindow(const WindowCreateInformation& createInformation);
		~LinuxWindow();

		/**	Returns position of the left-most border of the window, relative to the screen. */
		i32 GetLeft() const;

		/**	Returns position of the top-most border of the window, relative to the screen. */
		i32 GetTop() const;

		/**	Returns width of the window in pixels. */
		u32 GetWidth() const;

		/**	Returns height of the window in pixels. */
		u32 GetHeight() const;

		/** Hides or shows the window. */
		void SetHidden(bool hidden);

		/**	Minimizes the window. */
		void Minimize();

		/**	Maximizes the window over the entire current screen. */
		void Maximize();

		/**	Restores the window to original position and size if it is minimized or maximized. */
		void Restore();

		/**	Change the size of the window. */
		void Resize(u32 width, u32 height);

		/**	Reposition the window. */
		void Move(i32 left, i32 top);

		/** Sets the icon to display for the window. */
		void SetIcon(const PixelData& icon);

		/**	Converts screen position into window local position. */
		Vector2I ScreenToWindowPos(const Vector2I& screenPos) const;

		/**	Converts window local position to screen position. */
		Vector2I WindowToScreenPos(const Vector2I& windowPos) const;

		/** Method that triggers whenever the window changes size or position. */
		void DoOnWindowMovedOrResized();

		/**
		 * @name Internal
		 * @{
		 */

		/**
		 * Destroys the window, cleaning up any resources and removing it from the display. No further methods should be
		 * called on this object after it has been destroyed.
		 */
		void DestroyInternal();

		/**
		 * Sets a portion of the window in which the user can click and drag in order to move the window. This is needed
		 * when window has no title bar, yet you still want to allow the user to drag it by clicking on some specific area
		 * (e.g. a title bar you manually render).
		 *
		 * @param[in]	rects	Areas of the window (relative to the window origin in top-left corner) in which the drag
		 * 						operation in allowed.
		 */
		void SetDragZonesInternal(const Vector<Rect2I>& rects);

		/**
		 * Notifies the window that user has started dragging the window using a custom drag zone. Provided parameter is the
		 * event that started the drag.
		 */
		void DragStartInternal(const XButtonEvent& event);

		/** Notifies the window the user has stopped the window drag operation. */
		void DragEndInternal();

		/** Returns the internal X11 window handle. */
		::Window GetXWindowInternal() const;

		/** Toggles between fullscreen and windowed mode. */
		void SetFullscreenInternal(bool fullscreen);

		/** Attaches non-specific user data that can later be retrieved through GetUserDataInternal(). */
		void SetUserDataInternal(void* data);

		/** Returns user data attached to the object when _setUserData was called. */
		void* GetUserDataInternal() const;

		/** @} */

	private:
		/** Checks if the window is currently maximized. */
		bool isMaximized() const;

		/** Checks if the window is currently minimized (iconified). */
		bool isMinimized();

		/**
		 * Maximizes a window if @p enable is true. If false restores the window to size/position before maximization
		 * occurred.
		 */
		void maximize(bool enable);

		/**
		 * Minimizes a window if @p enable is true. If false restores the window to size/position before minimization
		 * occurred.
		 */
		void minimize(bool enable);

		/** Shows or hides the window icon from the taskbar. */
		void showOnTaskbar(bool enable);

		/**
		 * Shows or hides window decorations. Decorations include window title bar, border and similar. Essentially anything
		 * not part of the main rendering area.
		 */
		void setShowDecorations(bool show);

		/**
		 * Switches the window between modal and normal mode. Modal window prevents input to their parent window until
		 * it is dismissed.
		 */
		void setIsModal(bool modal);

		struct Pimpl;
		Pimpl* m;
	};

	/** @} */
	/** @} */
} // namespace b3d
