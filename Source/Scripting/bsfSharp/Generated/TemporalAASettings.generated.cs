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

	/// <summary>Settings that control temporal anti-aliasing.</summary>
	[ShowInInspector]
	public partial class TemporalAASettings : ScriptObject
	{
		private TemporalAASettings(bool __dummy0) { }

		public TemporalAASettings()
		{
			Internal_TemporalAASettings(this);
		}

		/// <summary>Enables or disables temporal anti-aliasing.</summary>
		[ShowInInspector]
		[NativeWrapper]
		public bool Enabled
		{
			get { return Internal_GetEnabled(mCachedPtr); }
			set { Internal_SetEnabled(mCachedPtr, value); }
		}

		/// <summary>
		/// Number of different jittered positions to use. Each frame will use one position and subsequent frames will use 
		/// subsequent positions until this number of reached, at which point the positions start getting re-used from the start.
		/// </summary>
		[ShowInInspector]
		[Range(4f, 128f, false)]
		[NativeWrapper]
		public int JitteredPositionCount
		{
			get { return Internal_GetJitteredPositionCount(mCachedPtr); }
			set { Internal_SetJitteredPositionCount(mCachedPtr, value); }
		}

		/// <summary>Determines the distance between temporal AA samples. Larger values result in a sharper image.</summary>
		[ShowInInspector]
		[Range(0f, 1f, false)]
		[NativeWrapper]
		public float Sharpness
		{
			get { return Internal_GetSharpness(mCachedPtr); }
			set { Internal_SetSharpness(mCachedPtr, value); }
		}

		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_TemporalAASettings(TemporalAASettings managedInstance);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern bool Internal_GetEnabled(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SetEnabled(IntPtr thisPtr, bool value);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern int Internal_GetJitteredPositionCount(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SetJitteredPositionCount(IntPtr thisPtr, int value);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern float Internal_GetSharpness(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SetSharpness(IntPtr thisPtr, float value);
	}

	/** @} */
}
