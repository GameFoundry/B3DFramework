//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsCorePrerequisites.h"
#include "RenderAPI/BsRenderTarget.h"
#include "RenderAPI/BsVideoModeInfo.h"
#include "Math/BsVector2I.h"

namespace bs
{
	class RenderWindowManager;

	/** @addtogroup RenderAPI-Internal
	 *  @{
	 */

	/** Types of events that a RenderWindow can be notified of. */
	enum class WindowEventType
	{
		/** Triggered when window size changes. */
		Resized,
		/** Triggered when window position changes. */
		Moved,
		/** Triggered when window receives input focus. */
		FocusReceived,
		/** Triggered when window loses input focus. */
		FocusLost,
		/** Triggered when the window is minimized (iconified). */
		Minimized,
		/** Triggered when the window is expanded to cover the current screen. */
		Maximized,
		/** Triggered when the window leaves minimized or maximized state. */
		Restored,
		/** Triggered when the mouse pointer leaves the window area. */
		MouseLeft,
		/** Triggered when the user wants to close the window. */
		CloseRequested,
	};

	/** @} */

	/** @addtogroup RenderAPI
	 *  @{
	 */

	/** Structure that is used for initializing a render window. */
	struct BS_CORE_EXPORT RENDER_WINDOW_DESC
	{
		RENDER_WINDOW_DESC()
		: fullscreen(false), vsync(false), vsyncInterval(1), hidden(false), depthBuffer(true)
		, multisampleCount(0), multisampleHint(""), gamma(false), left(-1), top(-1), title("")
		, showTitleBar(true), showBorder(true), allowResize(true), toolWindow (false), modal(false)
		, hideUntilSwap(false)
		{ }

		VideoMode videoMode; /**< Output monitor, frame buffer resize and refresh rate. */
		bool fullscreen; /**< Should the window be opened in fullscreen mode. */
		bool vsync; /**< Should the window wait for vertical sync before swapping buffers. */
		UINT32 vsyncInterval; /**< Determines how many vsync intervals occur per frame. FPS = refreshRate/interval. Usually 1 when vsync active. */
		bool hidden; /**< Should the window be hidden initially. */
		bool depthBuffer; /**< Should the window be created with a depth/stencil buffer. */
		UINT32 multisampleCount; /**< If higher than 1, texture containing multiple samples per pixel is created. */
		String multisampleHint; /**< Hint about what kind of multisampling to use. Render system specific. */
		bool gamma; /**< Should the written color pixels be gamma corrected before write. */
		INT32 left; /**< Window origin on X axis in pixels. -1 == screen center. Relative to monitor provided in videoMode. */
		INT32 top; /**< Window origin on Y axis in pixels. -1 == screen center. Relative to monitor provided in videoMode. */
		String title; /**< Title of the window. */
		bool showTitleBar; /**< Determines if the title-bar should be shown or not. */
		bool showBorder; /**< Determines if the window border should be shown or not. */
		bool allowResize; /**< Determines if the user can resize the window by dragging on the window edges. */
		bool toolWindow; /**< Tool windows have no task bar entry and always remain on top of their parent window. */
		bool modal; /**< When a modal window is open all other windows will be locked until modal window is closed. */
		bool hideUntilSwap; /**< Window will be created as hidden and only be shown when the first framebuffer swap happens. */

		NameValuePairList platformSpecific; /**< Platform-specific creation options. */
	};

	/**	Contains various properties that describe a render window. */
	class BS_CORE_EXPORT RenderWindowProperties : public RenderTargetProperties
	{
	public:
		RenderWindowProperties(const RENDER_WINDOW_DESC& desc);
		virtual ~RenderWindowProperties() = default;

		/**	True if window is running in fullscreen mode. */
		bool isFullScreen = false;

		/**	Horizontal origin of the window in pixels. */
		INT32 left = 0;

		/**	Vertical origin of the window in pixels. */
		INT32 top = 0;

		/**	Indicates whether the window currently has keyboard focus. */
		bool hasFocus = false;

		/**	True if the window is hidden. */
		bool isHidden = false;

		/**	True if the window is modal (blocks interaction with any non-modal window until closed). */
		bool isModal = false;

		/**	True if the window is maximized. */
		bool isMaximized = false;
	};

	/**
	 * Operating system window with a specific position, size and style. Each window serves as a surface that can be
	 * rendered into by RenderAPI operations.
	 */
	class BS_CORE_EXPORT RenderWindow : public RenderTarget
	{
	public:
		virtual ~RenderWindow() { }

		/**	Converts screen position into window local position. */
		virtual Vector2I screenToWindowPos(const Vector2I& screenPos) const = 0;

		/**	Converts window local position to screen position. */
		virtual Vector2I windowToScreenPos(const Vector2I& windowPos) const = 0;

		/**	
		 * Resize the window to specified width and height in pixels. 
		 *
		 * @param[in]	width		Width of the window in pixels.
		 * @param[in]	height		Height of the window in pixels.
		 */
		virtual void resize(UINT32 width, UINT32 height);

		/**	
		 * Move the window to specified screen coordinates. 
		 *
		 * @param[in]	left		Position of the left border of the window on the screen.
		 * @param[in]	top			Position of the top border of the window on the screen.
		 * 
		 * @note This is an @ref asyncMethod "asynchronous method".
		 */
		virtual void move(INT32 left, INT32 top);

		/** 
		 * Hides the window. 
		 * 
		 * @note This is an @ref asyncMethod "asynchronous method".
		 */
		virtual void hide();

		/** 
		 * Shows a previously hidden window. 
		 * 
		 * @note This is an @ref asyncMethod "asynchronous method".
		 */
		virtual void show();

		/** 
		 * @copydoc ct::RenderWindow::minimize  
		 * 
		 * @note This is an @ref asyncMethod "asynchronous method".
		 */
		virtual void minimize();

		/** 
		 * @copydoc ct::RenderWindow::maximize 
		 * 
		 * @note This is an @ref asyncMethod "asynchronous method".
		 */
		virtual void maximize();

		/** 
		 * @copydoc ct::RenderWindow::restore  
		 * 
		 * @note This is an @ref asyncMethod "asynchronous method".
		 */
		virtual void restore();

		/** 
		 * @copydoc ct::RenderWindow::setFullscreen(UINT32, UINT32, float, UINT32) 
		 * 
		 * @note This is an @ref asyncMethod "asynchronous method".
		 */
		virtual void setFullscreen(UINT32 width, UINT32 height, float refreshRate = 60.0f, UINT32 monitorIdx = 0);

		/** 
		 * @copydoc ct::RenderWindow::setFullscreen(const VideoMode&) 
		 * 
		 * @note This is an @ref asyncMethod "asynchronous method".
		 */
		virtual void setFullscreen(const VideoMode& videoMode);

		/** 
		 * @copydoc ct::RenderWindow::setWindowed 
		 * 
		 * @note This is an @ref asyncMethod "asynchronous method".
		 */
		virtual void setWindowed(UINT32 width, UINT32 height);

		/**	Retrieves a core implementation of a render window usable only from the core thread. */
		SPtr<ct::RenderWindow> getCore() const;

		/**	Returns properties that describe the render window. */
		const RenderWindowProperties& getProperties() const;

		/** Closes and destroys the window. */
		void destroy() override;

		/**
		 * Creates a new render window using the specified options. Optionally makes the created window a child of another 
		 * window.
		 */
		static SPtr<RenderWindow> create(RENDER_WINDOW_DESC& desc, SPtr<RenderWindow> parentWindow = nullptr);

		/** Triggers when the OS requests that the window is closed (e.g. user clicks on the X button in the title bar). */
		Event<void()> onCloseRequested;

		/**
		 * @name Internal
		 */

		/** Notifies the window that a specific event occurred. Usually called by the platform specific main event loop. */
		void _notifyWindowEvent(WindowEventType type);

		/** Method that triggers whenever the window changes size or position. */
		virtual void _windowMovedOrResized() { }

		/** @} */

	protected:
		friend class RenderWindowManager;

		RenderWindow(const RENDER_WINDOW_DESC& desc, UINT32 windowId);

		/** Returns render window properties that may be edited. */
		RenderWindowProperties& getMutableProperties();

		/**	Updates window properties from the synced property data. */
		virtual void syncProperties() = 0;

	protected:
		RENDER_WINDOW_DESC mDesc;
		UINT32 mWindowId;
	};

	/** @} */

	namespace ct
	{
	/** @addtogroup RenderAPI-Internal
	 *  @{
	 */

	/** Core thread counterpart of bs::RenderWindow. */
	class BS_CORE_EXPORT RenderWindow : public RenderTarget
	{
	public:
		RenderWindow(const RENDER_WINDOW_DESC& desc, UINT32 windowId);
		virtual ~RenderWindow();

		/** 
		 * Switches the window to fullscreen mode. Child windows cannot go into fullscreen mode.
		 *
		 * @param[in]	width		Width of the window frame buffer in pixels.
		 * @param[in]	height		Height of the window frame buffer in pixels.
		 * @param[in]	refreshRate	Refresh rate of the window in Hertz.
		 * @param[in]	monitorIdx	Index of the monitor to go fullscreen on.
		 *
		 * @note	If the exact provided mode isn't available, closest one is used instead.
		 */
		virtual void setFullscreen(UINT32 width, UINT32 height, float refreshRate = 60.0f, UINT32 monitorIdx = 0) { }

		/**
		 * Switches the window to fullscreen mode. Child windows cannot go into fullscreen mode.
		 *
		 * @param[in]	videoMode	Mode retrieved from VideoModeInfo in RenderAPI.
		 */
		virtual void setFullscreen(const VideoMode& videoMode) { }

		/**
		 * Switches the window to windowed mode.
		 *
		 * @param[in]	width	Window width in pixels.
		 * @param[in]	height	Window height in pixels.
		 */
		virtual void setWindowed(UINT32 width, UINT32 height) { }

		/**	Hide or show the window. */
		virtual void setHidden(bool hidden);

		/**
		 * Makes the render target active or inactive. (for example in the case of a window, it will hide or restore the
		 * window). 
		 */
		virtual void setActive(bool state);

		/**	Minimizes the window to the taskbar. */
		virtual void minimize() { }

		/**	Maximizes the window over the entire current screen. */
		virtual void maximize() { }

		/**	Restores the window to original position and size if it is minimized or maximized. */
		virtual void restore() { }

		/**	Change the size of the window. */
		virtual void resize(UINT32 width, UINT32 height) = 0;

		/**	Reposition the window. */
		virtual void move(INT32 left, INT32 top) = 0;

		/**
		 * Enables or disables vertical synchronization. When enabled the system will wait for monitor refresh before
		 * presenting the back buffer. This eliminates tearing but can result in increased input lag.
		 *
		 * @param enabled 		True to enable vsync, false to disable.
		 * @param interval 		Interval at which to perform the sync. Value of one means the sync will be performed for
		 * 						each monitor refresh, value of two means it will be performs for every second (half the
		 * 						rate), and so on.
		 */
		virtual void setVSync(bool enabled, UINT32 interval = 1) = 0;

		/**	Returns properties that describe the render window. */
		const RenderWindowProperties& getProperties() const;

		/** Notifies the window that a specific event occurred. Usually called by the platform specific main event loop. */
		void _notifyWindowEvent(WindowEventType type);

		/** Method that triggers whenever the window changes size or position. */
		virtual void _windowMovedOrResized() { }
	protected:
		friend class bs::RenderWindow;
		friend class RenderWindowManager;
		friend class bs::RenderWindowManager;

		/**
		 * Returns window properties that are always kept in sync between core and sim threads.
		 *
		 * @note	Used for keeping up what are the most up to date settings.
		 */
		virtual RenderWindowProperties& getSyncedProperties() = 0;

		/** Updates window properties from the synced property data. */
		virtual void syncProperties() = 0;

		RENDER_WINDOW_DESC mDesc;
		SpinLock mLock;
		UINT32 mWindowId;
	};

	/** @} */
	}
}
