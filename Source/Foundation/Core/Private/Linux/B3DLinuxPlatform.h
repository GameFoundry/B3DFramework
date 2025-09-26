//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "Platform/B3DPlatform.h"
#include "Private/Linux/B3DLinuxInput.h"
#include <X11/X.h>
#include <X11/Xlib.h>

namespace b3d
{
	class LinuxWindow;

	/** @addtogroup Platform-Internal
	 *  @{
	 */

	/**
	 * Contains various Linux specific platform functionality;
	 */
	class B3D_EXPORT LinuxPlatform : public Platform
	{
	public:
		/** Returns the active X11 display. */
		static ::Display* getXDisplay();

		/** Returns the main X11 window. Caller must ensure the main window has been created. */
		static ::Window getMainXWindow();

		/** Retruns the absolute path to the user's home directory. */
		static Path getHomeDir();

		/** Locks access to the X11 system, not allowing any other thread to access it. Must be used for every X11 access. */
		static void lockX();

		/** Unlocks access to the X11 system. Must follow every call to lockX(). */
		static void unlockX();

		/** Notifies the system that a new window was created. */
		static void RegisterWindowInternal(::Window xWindow, LinuxWindow* window);

		/** Notifies the system that a window is about to be destroyed. */
		static void UnregisterWindowInternal(::Window xWindow);

		/** Generates a X11 Pixmap from the provided pixel data. */
		static Pixmap createPixmap(const PixelData& data, u32 depth);

		/** Mutex for accessing buttonEvents / mouseEvent. */
		static Mutex eventLock;

		/**
		 * Stores events captured on the core thread, waiting to be processed by the main thread.
		 * Always lock on eventLock when accessing this.
		 */
		static Queue<LinuxButtonEvent> buttonEvents;

		/**
		 * Stores accumulated mouse motion events, waiting to be processed by the main thread.
		 * Always lock on eventLock when accessing this.
		 */
		static LinuxMouseMotionEvent mouseMotionEvent;
	};

	/** @} */
} // namespace b3d
