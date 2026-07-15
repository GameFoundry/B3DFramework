//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DUtilityPrerequisites.h"
#include "Math/B3DVector2.h"
#include "Math/B3DArea2.h"

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
	struct WindowCreateInformation
	{
		String Title;
		i32 X = -1;
		i32 Y = -1;
		u32 Width = 20;
		u32 Height = 20;
		bool ShowDecorations = true;
		bool AllowResize = true;
		bool Modal = false;
		bool Floating = false;
		TShared<PixelData> Background;
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
			NSWindow* Window = nil;
			BSView* View = nil;
			BSWindowListener* Responder = nil;
			BSWindowDelegate* Delegate = nil;
			CALayer* Layer = nil;
			u32 NumDropTargets = 0;
			bool IsModal = false;
			NSUInteger Style = 0;
			bool IsFullscreen;
			NSRect WindowedRect;
			NSModalSession ModalSession = nil;
			void* UserData = nullptr;
		};
#else
		struct Pimpl;
#endif

		CocoaWindow(const WindowCreateInformation& createInformation);
		~CocoaWindow();

		/** Returns the current area of the window, relative to the top-left origin of the screen. */
		Area2I GetArea() const;

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

		/** Switches from fullscreen to windowed mode. */
		void SetWindowed();

		/** Switches from windowed to fullscreen mode. */
		void SetFullscreen();

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
		 * @param[in]	areas	Areas of the window (relative to the window origin in top-left corner) in which the drag
		 * 						operation in allowed.
		 */
		void SetDragZonesInternal(const Vector<Area2I>& areas);

		/** Attaches non-specific user data that can later be retrieved through GetUserDataInternal(). */
		void SetUserDataInternal(void* data);

		/** Returns user data attached to the object when SetUserDataInternal() was called. */
		void* GetUserDataInternal() const;

		/**
		 * Registers the window with the drag and drop manager and allows it to accept file drop operations. Each call
		 * to this method must eventually be followed with UnregisterForDragAndDropInternal().
		 */
		void RegisterForDragAndDropInternal();

		/**
		 * Unregisters the window from the drag and drop manager. This will need to be called multiple times if
		 * RegisterForDragAndDropInternal() was called multiple times.
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
