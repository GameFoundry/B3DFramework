//************************************ B3D Framework - Copyright 2025 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
using System;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;

namespace b3d
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

		/// <summary>Queues capture of the next frame&apos;s GPU commands, if a command capture tool is available.</summary>
		public static void RequestGPUCommandCapture()
		{
			Internal_RequestGPUCommandCapture();
		}

		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_RequestGPUCommandCapture();
	}

	/** @} */
}
