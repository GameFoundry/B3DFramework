//************************************ B3D Framework - Copyright 2026 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DGpuDevice.h"
#include "B3DPrerequisites.h"
#include "RenderAPI/B3DRenderTarget.h"
#include "RenderAPI/B3DVideoModeInfo.h"

namespace b3d
{
	class GpuQueue;
	class RenderWindowManager;

	/** @addtogroup GpuBackend-Internal
	 *  @{
	 */

	/** Types of events that a RenderWindow can be notified of. */
	enum class WindowEventType
	{
		/** Triggered when window size changes. */
		Resized,
		/** Triggered when window position changes. */
		Moved,
		/** Window DPI scale changed. */
		DPIScaleChanged,
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
		/** Triggered when the OS requests the window to be redrawn. */
		Redraw,
	};

	/**	Signals which portion of a render window is dirty. */
	enum class RenderWindowDirtyFlag
	{
		Redraw = 1 << 0,
		Everything = 1 << 1,
	};

	typedef Flags<RenderWindowDirtyFlag> RenderWindowDirtyFlags;
	B3D_FLAGS_OPERATORS(RenderWindowDirtyFlag)

	/** @} */

	/** @addtogroup GpuBackend
	 *  @{
	 */

	/** Structure that is used for initializing a render window. */
	struct B3D_EXPORT B3D_SCRIPT_EXPORT(DocumentationGroup(RenderAPI), ExportAsStruct(true), API(Framework)) RenderWindowCreateInformation
	{
		/** Output monitor, frame buffer resize and refresh rate. This is the size of the window's client area (which means actual window may be larger due to title bar and border. */
		VideoMode VideoMode;

		/** Should the window be opened in fullscreen mode. */
		bool Fullscreen = false;

		/** Should the window wait for vertical sync before swapping buffers. */
		bool Vsync = false;

		/** Determines how many vsync intervals occur per frame. FPS = refreshRate/interval. Usually 1 when vsync active. */
		u32 VsyncInterval = 1;

		/** Should the window be hidden initially. */
		bool Hidden = false;

		/** Should the window be created with a depth/stencil buffer. */
		bool DepthBuffer = false;

		/** If higher than 1, texture containing multiple samples per pixel is created. */
		u32 MultisampleCount = 0;

		/** Hint about what kind of multisampling to use. Render system specific. */
		String MultisampleHint = "";

		/** Should the written color pixels be gamma corrected before write. */
		bool Gamma = false;

		/** Window origin on X axis in pixels. -1 == screen center. Relative to monitor provided in videoMode. */
		i32 Left = -1;

		/** Window origin on Y axis in pixels. -1 == screen center. Relative to monitor provided in videoMode. */
		i32 Top = -1;

		/** Title of the window. */
		String Title = "";

		/** Determines if the title-bar should be shown or not. */
		bool ShowTitleBar = true;

		/** Determines if the window border should be shown or not. */
		bool ShowBorder = true;

		/** Determines if the user can resize the window by dragging on the window edges. */
		bool AllowResize = true;

		/** Tool windows have no task bar entry and always remain on top of their parent window. */
		bool ToolWindow = false;

		/** When a modal window is open all other windows will be locked until modal window is closed. */
		bool Modal = false;

		/** Window will be created as hidden and only be shown when the first framebuffer swap happens. */
		bool HideUntilSwap = false;

		/** If true the window render surface will be created, which internally provides a swap chain and allows the GPU to render to the window. */
		bool CreateRenderSurface = true;

		/**
		 * When true, no OS window is created. Instead, the window renders to internal GPU textures that mimic a swap chain.
		 * All window-handle-dependent operations (focus, input capture, cursor, etc.) become no-ops.
		 * Use this for headless rendering in automated testing or offscreen rendering scenarios.
		 */
		bool Headless = false;
	};

	/**	Contains various properties that describe a render window. */
	class B3D_EXPORT RenderWindowProperties
	{
	public:
		RenderWindowProperties() = default;
		virtual ~RenderWindowProperties() = default;

		/**	True if window is running in fullscreen mode. */
		bool IsFullScreen = false;

		/**	Horizontal origin of the window in pixels. */
		i32 Left = 0;

		/**	Vertical origin of the window in pixels. */
		i32 Top = 0;

		/**	Indicates whether the window currently has keyboard focus. */
		bool HasFocus = false;

		/**	True if the window is hidden. */
		bool IsHidden = false;

		/**	True if the window is modal (blocks interaction with any non-modal window until closed). */
		bool IsModal = false;

		/**	True if the window is maximized. */
		bool IsMaximized = false;

		/** True if the window is minimized. */
		bool IsMinimized = false;

		/**
		 * True if the render target will wait for vertical sync before swapping buffers. This will eliminate
		 * tearing but may increase input latency.
		 */
		bool Vsync = false;

		/**
		 * Controls how often should the frame be presented in respect to display device refresh rate. Normal value is 1
		 * where it will match the refresh rate. Higher values will decrease the frame rate (for example present interval of
		 * 2 on 60Hz refresh rate will display at most 30 frames per second).
		 */
		u32 VsyncInterval = 1;
	};

	/**
	 * Operating system window with a specific position, size and style. Each window serves as a surface that can be
	 * rendered into by RenderAPI operations.
	 */
	class B3D_EXPORT RenderWindow : public RenderTarget
	{
		struct SyncPacket;
	public:
		virtual ~RenderWindow() = default;

		/**	Converts screen position into window local position. */
		virtual Vector2I ScreenToWindowPosition(const Vector2I& screenPosition) const = 0;

		/**	Converts window local position to screen position. */
		virtual Vector2I WindowToScreenPosition(const Vector2I& windowPosition) const = 0;

		/**
		 * Resize the window to specified width and height in pixels.
		 *
		 * @param	width		Width of the window in pixels.
		 * @param	height		Height of the window in pixels.
		 */
		virtual void Resize(u32 width, u32 height) = 0;

		/**
		 * Move the window to specified screen coordinates.
		 *
		 * @param	left		Position of the left border of the window on the screen.
		 * @param	top			Position of the top border of the window on the screen.
		 */
		virtual void Move(i32 left, i32 top) = 0;

		/** Hides the window. */
		virtual void Hide() = 0;

		/** Shows a previously hidden window. */
		virtual void Show() = 0;

		/**	Minimizes the window to the taskbar. */
		virtual void Minimize() = 0;

		/**	Maximizes the window over the entire current screen. */
		virtual void Maximize() = 0;

		/**	Restores the window to original position and size if it is minimized or maximized. */
		virtual void Restore() = 0;

		/**
		 * Switches the window to fullscreen mode. Child windows cannot go into fullscreen mode.
		 *
		 * @param	width		Width of the window frame buffer in pixels.
		 * @param	height		Height of the window frame buffer in pixels.
		 * @param	refreshRate	Refresh rate of the window in Hertz.
		 * @param	monitorIndex	Index of the monitor to go fullscreen on.
		 *
		 * @note	If the exact provided mode isn't available, closest one is used instead.
		 */
		virtual void SetFullscreen(u32 width, u32 height, float refreshRate = 60.0f, u32 monitorIndex = 0) = 0;

		/**
		 * Switches the window to fullscreen mode. Child windows cannot go into fullscreen mode.
		 *
		 * @param	videoMode	Mode retrieved from VideoModeInfo in GpuDevice.
		 */
		virtual void SetFullscreen(const VideoMode& videoMode);

		/**
		 * Switches the window to windowed mode.
		 *
		 * @param	width	Window width in pixels.
		 * @param	height	Window height in pixels.
		 */
		virtual void SetWindowed(u32 width, u32 height) = 0;

		/**
		 * Enables or disables vertical synchronization. When enabled the system will wait for monitor refresh before
		 * presenting the back buffer. This eliminates tearing but can result in increased input lag.
		 *
		 * @param enabled 		True to enable vsync, false to disable.
		 * @param interval 		Interval at which to perform the sync. Value of one means the sync will be performed for
		 * 						each monitor refresh, value of two means it will be performs for every second (half the
		 * 						rate), and so on.
		 */
		virtual void SetVSync(bool enabled, u32 interval = 1) = 0;

		/** Returns a platform-specific window handle. (e.g. HWND on Windows) */
		virtual u64 GetPlatformWindowHandle() const = 0;

		/**	Returns properties that describe the render window. */
		const RenderWindowProperties& GetRenderWindowProperties() const { return mRenderWindowProperties; }

		/** Closes and destroys the window. */
		void Destroy() override;

		/**
		 * Creates a new render window using the specified options. Optionally makes the created window a child of another
		 * window.
		 */
		static SPtr<RenderWindow> Create(const RenderWindowCreateInformation& createInformation, const SPtr<RenderWindow>& parentWindow = nullptr);

		/** Triggers when the OS requests that the window is closed (e.g. user clicks on the X button in the title bar). */
		Event<void()> OnCloseRequested;

		/**
		 * @name Internal
		 */

		/** Notifies the window that a specific event occurred as reported by the OS event loop. */
		void NotifyWindowEvent(WindowEventType type);

		/** Method that triggers whenever the window changes size or position. */
		virtual void DoOnWindowMovedOrResized() {}

		/** Method that triggers whenever the DPI scale changes. */
		virtual void DoOnDPIScaleChanged() {}

		/** @} */

	protected:
		friend class RenderWindowManager;
		friend class render::RenderWindow;

		RenderWindow(const RenderWindowCreateInformation& createInformation, u32 windowId, const SPtr<RenderWindow>& parentWindow);
		RenderProxySyncPacket* CreateRenderProxySyncPacket(FrameAllocator& allocator, u32 flags) override;

	protected:
		RenderWindowCreateInformation mCreateInformation;
		RenderWindowProperties mRenderWindowProperties;
		WeakSPtr<RenderWindow> mParentWindow;
		u32 mWindowId = 0;

		/************************************************************************/
		/* 								SERIALIZATION                      		*/
		/************************************************************************/
	public:
		friend class RenderWindowRTTI;
		static RTTIType* GetRttiStatic();
		RTTIType* GetRtti() const override;
	};

	/** @} */

	namespace render
	{
		/** @addtogroup GpuBackend
		 *  @{
		 */

		class IRenderWindowSurface;

		/** Render thread counterpart of b3d::RenderWindow. */
		class B3D_EXPORT RenderWindow : public RenderTarget
		{
			using Super = RenderTarget;
		public:
			RenderWindow(const RenderWindowCreateInformation& createInformation, u32 windowId, u64 platformWindowHandle, const SPtr<RenderWindow>& parentWindow);
			~RenderWindow() override = default;

			void Initialize() override;
			void Destroy() override;
			TAsyncOp<SPtr<PixelData>> ReadAsync(GpuCommandBuffer& commandBuffer, u32 colorSurfaceIndex = 0, u32 mipLevel = 0, u32 arrayLayer = 0) override;

			/** Called by the GPU backend after it requests swap chain back buffer to be presented. */
			virtual void NotifySwapBuffersRequested();

			/** Rebuilds the swap chain according to the currently set properties. */
			virtual void RebuildSwapChain();

			/** Returns true if the operating system has requested the window to be redrawn. This state is cleared when NotifySwapBuffersRequested() is called. */
			bool IsRedrawRequested() const { return mIsRedrawRequested; }

			/**	Returns properties that describe the render window. */
			const RenderWindowProperties& GetRenderWindowProperties() const { return mRenderWindowProperties; }

			/** Returns the internal render window surface, if any. */
			const SPtr<IRenderWindowSurface>& GetRenderWindowSurface() const { return mRenderWindowSurface; }

			/** Triggers whenever the window changes properties that are relevant for the swap chain. */
			virtual void DoOnSwapChainPropertiesModified();

			/** Triggered when the window swap chain has been recreated. */
			mutable Event<void()> OnSwapChainDidRebuild;

		protected:
			void SyncFromCoreObject(const CoreSyncData& data, FrameAllocator& allocator) override;

		protected:
			friend class b3d::RenderWindow;
			friend class RenderWindowManager;
			friend class b3d::RenderWindowManager;

			RenderWindowCreateInformation mCreateInformation;
			RenderWindowProperties mRenderWindowProperties;
			WeakSPtr<RenderWindow> mParentWindow;
			u32 mWindowId = 0;
			u64 mPlatformWindowHandle = 0;
			bool mShowOnSwap = false;
			bool mIsRedrawRequested = false;
			SPtr<IRenderWindowSurface> mRenderWindowSurface;
		};

		/** Structure used for initializing an implementation of RenderWindowSurface. */
		struct RenderWindowSurfaceCreateInformation
		{
			u64 PlatformWindowHandle = 0; /**< Platform specific handle to the window we're creating the surface for. */
			u32 Width = 0; /**< Width of the render surface, in pixels. */
			u32 Height = 0; /**< Height of the render surface, in pixels. */
			bool VSync = false;
			u32 VsyncInterval = 1; /**< Number of refresh cycles between presents when @c VSync is true. 1 = every refresh, 2 = half rate, etc. Backends that don't support variable present rate ignore this. */
			bool CreateDepthBuffer = false;
			bool UseHardwareSRGB = false;
			bool Headless = false; /**< When true, creates a headless surface for offscreen rendering (no OS window is created). */
		};

		/**
		 * Interface that acts as a bridge between a RenderWindow and a GpuBackend. One such interface should be implemented for each GPU backend.
		 * Internally creates the swap chain that allows the GPU to render to the window.
		 */
		class B3D_EXPORT IRenderWindowSurface
		{
		public:
			IRenderWindowSurface() = default;
			virtual ~IRenderWindowSurface() = default;

			/** Rebuilds the swap chain with new properties. */
			virtual void RebuildSwapChain(u32 width, u32 height, bool vsync) = 0;

			/** Presents the current back-buffer image, and acquires the next swap chain image for rendering. */
			virtual void SwapBuffers(GpuQueue& queue, GpuQueueMask syncMask) = 0;

			/** Marks the swap chain as invalid, so the systems knows to rebuild it on the next occasion. */
			virtual void MarkSwapChainAsInvalid() = 0;

			/** Releases any resources associated with the surface. */
			virtual void Destroy() = 0;

			/**
			 * Reads the contents of the current swap chain image. Issues copy commands into
			 * the command buffer and returns an async operation that triggers when read is complete.
			 *
			 * @param	commandBuffer	Command buffer to issue copy commands into.
			 * @return					Async operation that completes when read is complete.
			 */
			virtual TAsyncOp<SPtr<PixelData>> ReadAsync(GpuCommandBuffer& commandBuffer)
			{
				TAsyncOp<SPtr<PixelData>> op;
				op.CompleteOperation(nullptr);

				return op;
			}
		};

		/** @} */
	} // namespace render
} // namespace b3d
