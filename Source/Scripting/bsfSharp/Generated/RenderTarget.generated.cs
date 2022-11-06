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

	/// <summary>Render target is a frame buffer or a texture that the render system renders the scene to.</summary>
	[ShowInInspector]
	public partial class RenderTarget : ScriptObject
	{
		private RenderTarget(bool __dummy0) { }
		protected RenderTarget() { }

		[NativeWrapper]
		public int Width
		{
			get { return Internal_GetWidth(mCachedPtr); }
		}

		[NativeWrapper]
		public int Height
		{
			get { return Internal_GetHeight(mCachedPtr); }
		}

		[NativeWrapper]
		public bool GammaCorrection
		{
			get { return Internal_GetGammaCorrection(mCachedPtr); }
		}

		[ShowInInspector]
		[NativeWrapper]
		public int Priority
		{
			get { return Internal_GetPriority(mCachedPtr); }
			set { Internal_SetPriority(mCachedPtr, value); }
		}

		[NativeWrapper]
		public int SampleCount
		{
			get { return Internal_GetSampleCount(mCachedPtr); }
		}

		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern int Internal_GetWidth(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern int Internal_GetHeight(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern bool Internal_GetGammaCorrection(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern int Internal_GetPriority(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SetPriority(IntPtr thisPtr, int priority);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern int Internal_GetSampleCount(IntPtr thisPtr);
	}

	/** @} */
}
