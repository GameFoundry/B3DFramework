//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "Platform/B3DPlatform.h"
#include "RenderAPI/B3DRenderWindow.h"

// Don't include macOS frameworks when generating script bindings, as it can't find them
#ifndef B3D_CODEGEN
#	include <Cocoa/Cocoa.h>
#endif

namespace b3d
{
	// Forward declare Cocoa types for SBGen purposes, since we didn't include Cocoa.h above
#if B3D_CODEGEN
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
		static void registerWindow(CocoaWindow* window);

		/** Notifies the system that a window is about to be destroyed. */
		static void unregisterWindow(CocoaWindow* window);

		/**
		 * Locks the access to the list of CocoaWindows ensuring no new windows can be created or destroyed. This must be
		 * called any time CocoaWindow is being used outside of the main thread to ensure the window doesn't get destroyed
		 * while being in use. Needs to be followed by unlockWindows().
		 */
		static void lockWindows();

		/** Releases the lock acquires by lockWindows(). */
		static void unlockWindows();

		/**
		 *  Returns a window based on its ID. Returns null if window cannot be found. Expects the caller to lock windows
		 *  using lockWindows() in case this is called from non-main thread.
		 */
		static CocoaWindow* getWindow(u32 windowId);

		/** Generates a Cocoa image from the provided pixel data. */
		static NSImage* createNSImage(const PixelData& data);

		/** Sends an event notifying the system that a key corresponding to an input command was pressed. */
		static void sendInputCommandEvent(InputCommandType inputCommand);

		/** Sends an event notifying the system that the user typed some text. */
		static void sendCharInputEvent(u32 character);

		/** Sends an event notifying the system that a pointer button was pressed. */
		static void sendPointerButtonPressedEvent(
			const Vector2I& pos,
			OSMouseButton button,
			const OSPointerButtonStates& buttonStates);

		/** Sends an event notifying the system that a pointer button was released. */
		static void sendPointerButtonReleasedEvent(
			const Vector2I& pos,
			OSMouseButton button,
			const OSPointerButtonStates& buttonStates);

		/** Sends an event notifying the system that the user clicked the left pointer button twice in quick succession. */
		static void sendPointerDoubleClickEvent(const Vector2I& pos, const OSPointerButtonStates& buttonStates);

		/** Sends an event notifying the system that the pointer moved. */
		static void sendPointerMovedEvent(const Vector2I& pos, const OSPointerButtonStates& buttonStates);

		/** Sends an event notifying the system the user has scrolled the mouse wheel. */
		static void sendMouseWheelScrollEvent(float delta);

		/** Notifies the system that some window-related event has occurred. */
		static void notifyWindowEvent(WindowEventType type, u32 windowId);

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
