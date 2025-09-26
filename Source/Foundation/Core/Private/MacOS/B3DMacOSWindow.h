//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "Prerequisites/B3DPrerequisitesUtil.h"
#include "Math/B3DVector2I.h"
#include "Math/B3DRect2I.h"

#ifdef BS_COCOA_INTERNALS
#	import <Cocoa/Cocoa.h>
#endif

#ifdef BS_COCOA_INTERNALS
@class BSWindowDelegate;
@class BSWindowListener;
@class BSView;
@class BSWindow;
@class CALayer;
#endif

namespace b3d
{
	class CocoaDropTarget;

	/** @addtogroup Internal-Utility
	 *  @{
	 */

	/** @addtogroup Platform-Utility-Internal
	 *  @{
	 */

	/**	Descriptor used for creating a platform specific native window. */
	struct WINDOW_DESC
	{
		String title;
		i32 x = -1;
		i32 y = -1;
		u32 width = 20;
		u32 height = 20;
		bool showDecorations = true;
		bool allowResize = true;
		bool modal = false;
		bool floating = false;
		SPtr<PixelData> background;
	};

	/**
	 * Represents a Cocoa window. Note this class should only be used from the sim thread as Cocoa does not support
	 * event handling, windows or views outside of the main thread.
	 */
	class B3D_EXPORT CocoaWindow
	{
	public:
#ifdef BS_COCOA_INTERNALS
		struct Pimpl
		{
			NSWindow* window = nil;
			BSView* view = nil;
			BSWindowListener* responder = nil;
			BSWindowDelegate* delegate = nil;
			CALayer* layer = nil;
			u32 numDropTargets = 0;
			bool isModal = false;
			NSUInteger style = 0;
			bool isFullscreen;
			NSRect windowedRect;
			NSModalSession modalSession = nil;
			void* userData = nullptr;
		};
#else
		struct Pimpl;
#endif

		CocoaWindow(const WINDOW_DESC& desc);
		~CocoaWindow();

		/** Returns the current area of the window, relative to the top-left origin of the screen. */
		Rect2I getArea() const;

		/** Hides the window. */
		void hide();

		/** Shows (unhides) the window. */
		void show();

		/**	Minimizes the window. */
		void minimize();

		/**	Maximizes the window over the entire current screen. */
		void maximize();

		/**	Restores the window to original position and size if it is minimized or maximized. */
		void restore();

		/**	Change the size of the window. */
		void resize(u32 width, u32 height);

		/**	Reposition the window. */
		void move(i32 left, i32 top);

		/** Switches from fullscreen to windowed mode. */
		void setWindowed();

		/** Switches from windowed to fullscreen mode. */
		void setFullscreen();

		/**	Converts screen position into window local position. */
		Vector2I screenToWindowPos(const Vector2I& screenPos) const;

		/**	Converts window local position to screen position. */
		Vector2I windowToScreenPos(const Vector2I& windowPos) const;

		/**
		 * @name Internal
		 * @{
		 */

		/**
		 * Destroys the window, cleaning up any resources and removing it from the display. No further methods should be
		 * called on this object after it has been destroyed. This called automatically in the destructor.
		 */
		void DestroyInternal();

		/** Returns an identifier that unique identifies this window. */
		u32 GetWindowIdInternal() const { return mWindowId; }

		/**
		 * Sets a portion of the window in which the user can click and drag in order to move the window. This is needed
		 * when window has no title bar, yet you still want to allow the user to drag it by clicking on some specific area
		 * (e.g. a title bar you manually render).
		 *
		 * @param[in]	rects	Areas of the window (relative to the window origin in top-left corner) in which the drag
		 * 						operation in allowed.
		 */
		void SetDragZonesInternal(const Vector<Rect2I>& rects);

		/** Attaches non-specific user data that can later be retrieved through GetUserDataInternal(). */
		void SetUserDataInternal(void* data);

		/** Returns user data attached to the object when _setUserData was called. */
		void* GetUserDataInternal() const;

		/**
		 * Registers the window with the drag and drop manager and allows it to accept file drop operations. Each call
		 * to this method must eventually be followed with _unregisterForDragAndDrop.
		 */
		void RegisterForDragAndDropInternal();

		/**
		 * Unregisters the window from the drag and drop manager. This will need to be called multiple times if
		 * _registerForDragAndDrop was called multiple times.
		 */
		void UnregisterForDragAndDropInternal();

		/** Lets the window know that the provided OpenGL context will be rendering to it. */
		void RegisterGLContextInternal(void* context);

		/** Assigns a CALayer to the windows' view, and enables layer backing on the view. */
		void SetLayerInternal(void* layer);

		/** Returns the assigned CALayer to the window view. */
		void* GetLayerInternal() const;

		/** Returns internal private data for use by friends. */
		Pimpl* GetPrivateDataInternal() const { return m; }

		/** @} */

	private:
		Pimpl* m;
		u32 mWindowId;
	};

	/** @} */
	/** @} */
} // namespace b3d
