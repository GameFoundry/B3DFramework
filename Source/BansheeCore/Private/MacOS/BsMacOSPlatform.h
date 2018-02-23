//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2017 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "Platform/BsPlatform.h"

// Don't include macOS frameworks when generating script bindings, as it can't find them
#ifndef BS_SBGEN
#include <Cocoa/Cocoa.h>
#endif

namespace bs
{
	// Forward declare Cocoa types for SBGen purposes, since we didn't include Cocoa.h above
#if BS_SBGEN
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
	class BS_CORE_EXPORT MacOSPlatform : public Platform
	{
	public:
		/** Notifies the system that a new window was created. */
		static void registerWindow(CocoaWindow* window);

		/** Notifies the system that a window is about to be destroyed. */
		static void unregisterWindow(CocoaWindow* window);

		/** Generates a Cocoa image from the provided pixel data. */
		static NSImage* createNSImage(const PixelData& data);

		/** Sends an event notifying the system that a key corresponding to an input command was pressed. */
		static void sendInputCommandEvent(InputCommandType inputCommand);

		/** Sends an event notifying the system that the user typed some text. */
		static void sendCharInputEvent(UINT32 character);

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
		static void notifyWindowEvent(WindowEventType type, UINT32 windowId);

		/** Returns the currently assigned custom cursor. */
		static NSCursor* _getCurrentCursor();

		/**
		 * Clips the cursor position to clip bounds, if clipping is enabled. Returns true if clipping occured, and updates
		 * @p pos to the clipped position.
		 */
		static bool _clipCursor(Vector2I& pos);

		/** Updates clip bounds that depend on window size. Should be called after window size changes. */
		static void _updateClipBounds(NSWindow* window);

		/** Moves the cursor to the specified position in screen coordinates. */
		static void _setCursorPosition(const Vector2I& position);
	};

	/** Converts an area in screen space with bottom left origin, to top left origin. */
	void flipY(NSScreen* screen, NSRect& rect);

	/** Converts a point in screen space with bottom left origin, to top left origin. */
	void flipY(NSScreen* screen, NSPoint &point);

	/** Converts a point in window space with bottom left origin, to top left origin. */
	void flipYWindow(NSWindow* window, NSPoint &point);

	/** @} */
}
