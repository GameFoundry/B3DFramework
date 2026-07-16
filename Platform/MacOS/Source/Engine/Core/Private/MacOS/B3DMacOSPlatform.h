//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "Platform/B3DPlatform.h"
#include "GpuBackend/B3DRenderWindow.h"

// Don't include macOS frameworks when generating script bindings, as it can't find them.
// Also skip them in plain C++ translation units - Objective-C headers only parse in Objective-C++.
#if !defined(B3D_CODEGEN) && defined(__OBJC__)
#	include <Cocoa/Cocoa.h>
#endif

namespace b3d
{
	// Forward declare Cocoa types when Cocoa.h wasn't included above. Only usable as opaque
	// pointers in that case - translation units that touch their members must be Objective-C++.
#if B3D_CODEGEN || !defined(__OBJC__)
	class NSImage;
	class NSCursor;
	class NSScreen;
	class NSWindow;
	struct NSRect;
	struct NSPoint;
#endif

	class CocoaWindow;

	/** @addtogroup Platform-Internal
	 *  @{
	 */

	/** Contains various macOS specific platform functionality. */
	class B3D_EXPORT MacOSPlatform : public Platform
	{
	public:
		/** Notifies the system that a new window was created. */
		static void RegisterWindow(CocoaWindow* window);

		/** Notifies the system that a window is about to be destroyed. */
		static void UnregisterWindow(CocoaWindow* window);

		/**
		 * Locks the access to the list of CocoaWindows ensuring no new windows can be created or destroyed. This must be
		 * called any time CocoaWindow is being used outside of the main thread to ensure the window doesn't get destroyed
		 * while being in use. Needs to be followed by UnlockWindows().
		 */
		static void LockWindows();

		/** Releases the lock acquired by LockWindows(). */
		static void UnlockWindows();

		/**
		 *  Returns a window based on its ID. Returns null if window cannot be found. Expects the caller to lock windows
		 *  using LockWindows() in case this is called from a non-main thread.
		 */
		static CocoaWindow* GetWindow(u32 windowId);

		/** Generates a Cocoa image from the provided pixel data. */
		static NSImage* CreateNSImage(const PixelData& data);

		/** Sends an event notifying the system that a key corresponding to an input command was pressed. */
		static void SendInputCommandEvent(InputCommandType inputCommand);

		/** Sends an event notifying the system that the user typed some text. */
		static void SendCharInputEvent(u32 character);

		/** Sends an event notifying the system that a pointer button was pressed. */
		static void SendPointerButtonPressedEvent(
			const Vector2I& pos,
			OSMouseButton button,
			const OSPointerButtonStates& buttonStates);

		/** Sends an event notifying the system that a pointer button was released. */
		static void SendPointerButtonReleasedEvent(
			const Vector2I& pos,
			OSMouseButton button,
			const OSPointerButtonStates& buttonStates);

		/** Sends an event notifying the system that the user clicked the left pointer button twice in quick succession. */
		static void SendPointerDoubleClickEvent(const Vector2I& pos, const OSPointerButtonStates& buttonStates);

		/** Sends an event notifying the system that the pointer moved. */
		static void SendPointerMovedEvent(const Vector2I& pos, const OSPointerButtonStates& buttonStates);

		/** Sends an event notifying the system the user has scrolled the mouse wheel. */
		static void SendMouseWheelScrollEvent(float delta);

		/** Notifies the system that some window-related event has occurred. */
		static void NotifyWindowEvent(WindowEventType type, u32 windowId);

		/** Returns the currently assigned custom cursor. */
		static NSCursor* GetCurrentCursorInternal();

		/**
		 * Clips the cursor position to clip bounds, if clipping is enabled. Returns true if clipping occured, and updates
		 * @p pos to the clipped position.
		 */
		static bool ClipCursorInternal(Vector2I& pos);

		/** Updates clip bounds that depend on window size. Should be called after window size changes. */
		static void UpdateClipBoundsInternal(NSWindow* window);

		/** Moves the cursor to the specified position in screen coordinates. */
		static void SetCursorPositionInternal(const Vector2I& position);
	};

	/** Converts an area in screen space with bottom left origin, to top left origin. */
	void flipY(NSScreen* screen, NSRect& rect);

	/** Converts a point in screen space with bottom left origin, to top left origin. */
	void flipY(NSScreen* screen, NSPoint& point);

	/** Converts a point in window space with bottom left origin, to top left origin. */
	void flipYWindow(NSWindow* window, NSPoint& point);

	/** @} */
} // namespace b3d
