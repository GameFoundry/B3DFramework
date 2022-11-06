//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
using System;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;

namespace bs
{
#if !IS_B3D
	/** @addtogroup RenderAPI
	 *  @{
	 */

	/// <summary>Structure that is used for initializing a render window.</summary>
	[StructLayout(LayoutKind.Sequential), SerializeObject]
	public partial struct RenderWindowDesc
	{
		/// <summary>Initializes the struct with default values.</summary>
		public static RenderWindowDesc Default()
		{
			RenderWindowDesc value = new RenderWindowDesc();
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

			return value;
		}

		/// <summary>Output monitor, frame buffer resize and refresh rate.</summary>
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
	}

	/** @} */
#endif
}
