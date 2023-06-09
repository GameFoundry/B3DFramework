//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
using System;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;

namespace bs
{
	/** @addtogroup Rendering
	 *  @{
	 */

	/// <summary>
	/// Allows you to change and retrieve the active renderer. Active renderer will be used for rendering all objects in the 
	/// following frame.
	/// </summary>
	[ShowInInspector]
	public partial class RendererManager : ScriptObject
	{
		private RendererManager(bool __dummy0) { }
		protected RendererManager() { }

		/// <summary>Queues GPU command capture of the next frame, if a frame capture is set up.</summary>
		public static void RequestFrameCapture()
		{
			Internal_RequestFrameCapture();
		}

		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_RequestFrameCapture();
	}

	/** @} */
}
