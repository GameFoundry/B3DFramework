//************************************ B3D Framework - Copyright 2025 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
using System;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;

namespace b3d
{
#if !IS_B3D
	/** @addtogroup GpuBackend
	 *  @{
	 */

	/// <summary>Structure that is used for initializing a render window.</summary>
	[StructLayout(LayoutKind.Sequential), SerializeObject]
	public partial struct RenderWindowCreateInformation
	{
		/// <summary>Initializes the struct with default values.</summary>
		public static RenderWindowCreateInformation Default()
		{
			RenderWindowCreateInformation value = new RenderWindowCreateInformation();
			value.VideoMode = VideoMode.Default();
			value.Fullscreen = false;
			value.Vsync = false;
			value.VsyncInterval = 1;
			value.Hidden = false;
			value.DepthBuffer = false;
			value.MultisampleCount = 0;
			value.MultisampleHint = "";
			value.Gamma = false;
			value.Left = -1;
			value.Top = -1;
			value.Title = "";
			value.ShowTitleBar = true;
			value.ShowBorder = true;
			value.AllowResize = true;
			value.ToolWindow = false;
			value.Modal = false;
			value.HideUntilSwap = false;
			value.CreateRenderSurface = true;
			value.Headless = false;

			return value;
		}

		/// <summary>
		/// Output monitor, frame buffer resize and refresh rate. This is the size of the window&apos;s client area (which means 
		/// actual window may be larger due to title bar and border.
		/// </summary>
		public VideoMode VideoMode;
		/// <summary>Should the window be opened in fullscreen mode.</summary>
		public bool Fullscreen;
		/// <summary>Should the window wait for vertical sync before swapping buffers.</summary>
		public bool Vsync;
		/// <summary>
		/// Determines how many vsync intervals occur per frame. FPS = refreshRate/interval. Usually 1 when vsync active.
		/// </summary>
		public int VsyncInterval;
		/// <summary>Should the window be hidden initially.</summary>
		public bool Hidden;
		/// <summary>Should the window be created with a depth/stencil buffer.</summary>
		public bool DepthBuffer;
		/// <summary>If higher than 1, texture containing multiple samples per pixel is created.</summary>
		public int MultisampleCount;
		/// <summary>Hint about what kind of multisampling to use. Render system specific.</summary>
		public string MultisampleHint;
		/// <summary>Should the written color pixels be gamma corrected before write.</summary>
		public bool Gamma;
		/// <summary>Window origin on X axis in pixels. -1 == screen center. Relative to monitor provided in videoMode.</summary>
		public int Left;
		/// <summary>Window origin on Y axis in pixels. -1 == screen center. Relative to monitor provided in videoMode.</summary>
		public int Top;
		/// <summary>Title of the window.</summary>
		public string Title;
		/// <summary>Determines if the title-bar should be shown or not.</summary>
		public bool ShowTitleBar;
		/// <summary>Determines if the window border should be shown or not.</summary>
		public bool ShowBorder;
		/// <summary>Determines if the user can resize the window by dragging on the window edges.</summary>
		public bool AllowResize;
		/// <summary>Tool windows have no task bar entry and always remain on top of their parent window.</summary>
		public bool ToolWindow;
		/// <summary>When a modal window is open all other windows will be locked until modal window is closed.</summary>
		public bool Modal;
		/// <summary>Window will be created as hidden and only be shown when the first framebuffer swap happens.</summary>
		public bool HideUntilSwap;
		/// <summary>
		/// If true the window render surface will be created, which internally provides a swap chain and allows the GPU to 
		/// render to the window.
		/// </summary>
		public bool CreateRenderSurface;
		/// <summary>
		/// When true, no OS window is created. Instead, the window renders to internal GPU textures that mimic a swap chain. All 
		/// window-handle-dependent operations (focus, input capture, cursor, etc.) become no-ops. Use this for headless 
		/// rendering in automated testing or offscreen rendering scenarios.
		/// </summary>
		public bool Headless;
	}

	/** @} */
#endif
}
