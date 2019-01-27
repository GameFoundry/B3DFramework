//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
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
				Internal_getsaturation(mCachedPtr, out temp);
				return temp;
			}
			set { Internal_setsaturation(mCachedPtr, ref value); }
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
				Internal_getcontrast(mCachedPtr, out temp);
				return temp;
			}
			set { Internal_setcontrast(mCachedPtr, ref value); }
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
				Internal_getgain(mCachedPtr, out temp);
				return temp;
			}
			set { Internal_setgain(mCachedPtr, ref value); }
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
				Internal_getoffset(mCachedPtr, out temp);
				return temp;
			}
			set { Internal_setoffset(mCachedPtr, ref value); }
		}

		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_getsaturation(IntPtr thisPtr, out Vector3 __output);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_setsaturation(IntPtr thisPtr, ref Vector3 value);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_getcontrast(IntPtr thisPtr, out Vector3 __output);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_setcontrast(IntPtr thisPtr, ref Vector3 value);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_getgain(IntPtr thisPtr, out Vector3 __output);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_setgain(IntPtr thisPtr, ref Vector3 value);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_getoffset(IntPtr thisPtr, out Vector3 __output);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_setoffset(IntPtr thisPtr, ref Vector3 value);
	}
}
