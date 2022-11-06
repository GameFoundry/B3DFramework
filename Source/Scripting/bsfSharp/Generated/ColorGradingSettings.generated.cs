//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
using System;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;

namespace bs
{
	/// <summary>Settings that control color grading post-process.</summary>
	[ShowInInspector]
	public partial class ColorGradingSettings : ScriptObject
	{
		private ColorGradingSettings(bool __dummy0) { }
		protected ColorGradingSettings() { }

		/// <summary>
		/// Saturation to be applied during color grading. Larger values increase vibrancy of the image. In range [0.0f, 2.0f].
		/// </summary>
		[ShowInInspector]
		[NativeWrapper]
		public Vector3 Saturation
		{
			get
			{
				Vector3 temp;
				Internal_GetSaturation(mCachedPtr, out temp);
				return temp;
			}
			set { Internal_SetSaturation(mCachedPtr, ref value); }
		}

		/// <summary>
		/// Contrast to be applied during color grading. Larger values increase difference between light and dark areas of the 
		/// image. In range [0.0f, 2.0f].
		/// </summary>
		[ShowInInspector]
		[NativeWrapper]
		public Vector3 Contrast
		{
			get
			{
				Vector3 temp;
				Internal_GetContrast(mCachedPtr, out temp);
				return temp;
			}
			set { Internal_SetContrast(mCachedPtr, ref value); }
		}

		/// <summary>
		/// Gain to be applied during color grading. Simply increases all color values by an equal scale. In range [0.0f, 2.0f].
		/// </summary>
		[ShowInInspector]
		[NativeWrapper]
		public Vector3 Gain
		{
			get
			{
				Vector3 temp;
				Internal_GetGain(mCachedPtr, out temp);
				return temp;
			}
			set { Internal_SetGain(mCachedPtr, ref value); }
		}

		/// <summary>
		/// Gain to be applied during color grading. Simply offsets all color values by an equal amount. In range [-1.0f, 1.0f].
		/// </summary>
		[ShowInInspector]
		[NativeWrapper]
		public Vector3 Offset
		{
			get
			{
				Vector3 temp;
				Internal_GetOffset(mCachedPtr, out temp);
				return temp;
			}
			set { Internal_SetOffset(mCachedPtr, ref value); }
		}

		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_GetSaturation(IntPtr thisPtr, out Vector3 __output);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SetSaturation(IntPtr thisPtr, ref Vector3 value);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_GetContrast(IntPtr thisPtr, out Vector3 __output);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SetContrast(IntPtr thisPtr, ref Vector3 value);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_GetGain(IntPtr thisPtr, out Vector3 __output);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SetGain(IntPtr thisPtr, ref Vector3 value);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_GetOffset(IntPtr thisPtr, out Vector3 __output);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SetOffset(IntPtr thisPtr, ref Vector3 value);
	}
}
