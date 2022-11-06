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

	/// <summary>Settings that control the bloom effect. Bloom adds an extra highlight to bright areas of the scene.</summary>
	[ShowInInspector]
	public partial class BloomSettings : ScriptObject
	{
		private BloomSettings(bool __dummy0) { }

		public BloomSettings()
		{
			Internal_BloomSettings(this);
		}

		/// <summary>Enables or disables the bloom effect.</summary>
		[ShowInInspector]
		[NativeWrapper]
		public bool Enabled
		{
			get { return Internal_GetEnabled(mCachedPtr); }
			set { Internal_SetEnabled(mCachedPtr, value); }
		}

		/// <summary>
		/// Quality of the bloom effect. Higher values will use higher resolution texture for calculating bloom, at the cost of 
		/// lower performance. Valid range is [0, 3], default is 2.
		/// </summary>
		[ShowInInspector]
		[NativeWrapper]
		public int Quality
		{
			get { return Internal_GetQuality(mCachedPtr); }
			set { Internal_SetQuality(mCachedPtr, value); }
		}

		/// <summary>
		/// Determines the minimal threshold of pixel luminance to be included in the bloom calculations. Any pixel with 
		/// luminance below this value will be ignored for the purposes of bloom. The value represents luminance after it is 
		/// scaled by exposure. Set to zero or negative to disable the threshold and include all pixels in the calculations.
		/// </summary>
		[ShowInInspector]
		[NativeWrapper]
		public float Threshold
		{
			get { return Internal_GetThreshold(mCachedPtr); }
			set { Internal_SetThreshold(mCachedPtr, value); }
		}

		/// <summary>
		/// Determines the intensity of the bloom effect. Ideally should be in [0, 4] range but higher values are allowed.
		/// </summary>
		[ShowInInspector]
		[NativeWrapper]
		public float Intensity
		{
			get { return Internal_GetIntensity(mCachedPtr); }
			set { Internal_SetIntensity(mCachedPtr, value); }
		}

		/// <summary>
		/// Tint color to apply to the bloom highlight. A pure white means the bloom inherits the underlying scene color.
		/// </summary>
		[ShowInInspector]
		[NativeWrapper]
		public Color Tint
		{
			get
			{
				Color temp;
				Internal_GetTint(mCachedPtr, out temp);
				return temp;
			}
			set { Internal_SetTint(mCachedPtr, ref value); }
		}

		/// <summary>
		/// Determines the percent of the texture to account for when filtering for bloom. Larger values will include farther 
		/// away pixels.
		/// </summary>
		[ShowInInspector]
		[Range(0.01f, 1f, false)]
		[NativeWrapper]
		public float FilterSize
		{
			get { return Internal_GetFilterSize(mCachedPtr); }
			set { Internal_SetFilterSize(mCachedPtr, value); }
		}

		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_BloomSettings(BloomSettings managedInstance);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern bool Internal_GetEnabled(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SetEnabled(IntPtr thisPtr, bool value);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern int Internal_GetQuality(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SetQuality(IntPtr thisPtr, int value);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern float Internal_GetThreshold(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SetThreshold(IntPtr thisPtr, float value);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern float Internal_GetIntensity(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SetIntensity(IntPtr thisPtr, float value);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_GetTint(IntPtr thisPtr, out Color __output);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SetTint(IntPtr thisPtr, ref Color value);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern float Internal_GetFilterSize(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SetFilterSize(IntPtr thisPtr, float value);
	}

	/** @} */
}
