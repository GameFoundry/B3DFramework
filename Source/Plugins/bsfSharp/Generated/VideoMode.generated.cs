//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
using System;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;

#if IS_B3D
namespace BansheeEngine
#else
namespace bs
#endif
{
#if !IS_B3D
	/** @addtogroup RenderAPI
	 *  @{
	 */

	/// <summary>
	/// Video mode contains information about how a render window presents its information to an output device like a  monitor.
	/// </summary>
	[StructLayout(LayoutKind.Sequential), SerializeObject]
	public partial struct VideoMode
	{
		/// <summary>Initializes the struct with default values.</summary>
		public static VideoMode Default()
		{
			VideoMode value = new VideoMode();
			value.width = 1280;
			value.height = 720;
			value.refreshRate = 60f;
			value.outputIdx = 0;
			value.isCustom = true;

			return value;
		}

		/// <summary>Creates a new video mode.</summary>
		/// <param name="width">Width of the frame buffer in pixels.</param>
		/// <param name="height">Height of the frame buffer in pixels.</param>
		/// <param name="refreshRate">How often should the output device refresh the output image in hertz.</param>
		/// <param name="outputIdx">
		/// Output index of the output device. Normally this means output monitor. 0th index always represents the primary device 
		/// while order of others is undefined.
		/// </param>
		public VideoMode(int width, int height, float refreshRate = 60f, int outputIdx = 0)
		{
			this.width = width;
			this.height = height;
			this.refreshRate = refreshRate;
			this.outputIdx = outputIdx;
			this.isCustom = true;
		}

		/// <summary>Width of the front/back buffer in pixels.</summary>
		public int width;
		/// <summary>Height of the front/back buffer in pixels.</summary>
		public int height;
		/// <summary>Refresh rate in hertz.</summary>
		public float refreshRate;
		/// <summary>Index of the parent video output.</summary>
		public int outputIdx;
		/// <summary>
		/// Determines was video mode user created or provided by the API/OS. API/OS created video modes can contain  additional 
		/// information that allows the video mode to be used more accurately and you should use them when possible.
		/// </summary>
		public bool isCustom;
	}

	/** @} */
#endif
}
