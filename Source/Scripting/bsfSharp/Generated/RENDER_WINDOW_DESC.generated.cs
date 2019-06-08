//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
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
			value.videoMode = VideoMode.Default();
			value.fullscreen = false;
			value.vsync = false;
			value.vsyncInterval = 1;
			value.hidden = false;
			value.depthBuffer = false;
			value.multisampleCount = 0;
			value.multisampleHint = "";
			value.gamma = false;
			value.left = -1;
			value.top = -1;
			value.title = "";
			value.showTitleBar = true;
			value.showBorder = true;
			value.allowResize = true;
			value.toolWindow = false;
			value.modal = false;
			value.hideUntilSwap = false;

			return value;
		}

		/// <summary>Output monitor, frame buffer resize and refresh rate.</summary>
		public VideoMode videoMode;
		/// <summary>Should the window be opened in fullscreen mode.</summary>
		public bool fullscreen;
		/// <summary>Should the window wait for vertical sync before swapping buffers.</summary>
		public bool vsync;
		/// <summary>
		/// Determines how many vsync intervals occur per frame. FPS = refreshRate/interval. Usually 1 when vsync active.
		/// </summary>
		public int vsyncInterval;
		/// <summary>Should the window be hidden initially.</summary>
		public bool hidden;
		/// <summary>Should the window be created with a depth/stencil buffer.</summary>
		public bool depthBuffer;
		/// <summary>If higher than 1, texture containing multiple samples per pixel is created.</summary>
		public int multisampleCount;
		/// <summary>Hint about what kind of multisampling to use. Render system specific.</summary>
		public string multisampleHint;
		/// <summary>Should the written color pixels be gamma corrected before write.</summary>
		public bool gamma;
		/// <summary>Window origin on X axis in pixels. -1 == screen center. Relative to monitor provided in videoMode.</summary>
		public int left;
		/// <summary>Window origin on Y axis in pixels. -1 == screen center. Relative to monitor provided in videoMode.</summary>
		public int top;
		/// <summary>Title of the window.</summary>
		public string title;
		/// <summary>Determines if the title-bar should be shown or not.</summary>
		public bool showTitleBar;
		/// <summary>Determines if the window border should be shown or not.</summary>
		public bool showBorder;
		/// <summary>Determines if the user can resize the window by dragging on the window edges.</summary>
		public bool allowResize;
		/// <summary>Tool windows have no task bar entry and always remain on top of their parent window.</summary>
		public bool toolWindow;
		/// <summary>When a modal window is open all other windows will be locked until modal window is closed.</summary>
		public bool modal;
		/// <summary>Window will be created as hidden and only be shown when the first framebuffer swap happens.</summary>
		public bool hideUntilSwap;
	}

	/** @} */
#endif
}
