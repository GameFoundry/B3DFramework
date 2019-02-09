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
			get { return Internal_getenabled(mCachedPtr); }
			set { Internal_setenabled(mCachedPtr, value); }
		}

		/// <summary>
		/// Quality of the bloom effect. Higher values will use higher resolution texture for calculating bloom, at the cost of 
		/// lower performance. Valid range is [0, 3], default is 2.
		/// </summary>
		[ShowInInspector]
		[NativeWrapper]
		public int Quality
		{
			get { return Internal_getquality(mCachedPtr); }
			set { Internal_setquality(mCachedPtr, value); }
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
			get { return Internal_getthreshold(mCachedPtr); }
			set { Internal_setthreshold(mCachedPtr, value); }
		}

		/// <summary>
		/// Determines the intensity of the bloom effect. Ideally should be in [0, 4] range but higher values are allowed.
		/// </summary>
		[ShowInInspector]
		[NativeWrapper]
		public float Intensity
		{
			get { return Internal_getintensity(mCachedPtr); }
			set { Internal_setintensity(mCachedPtr, value); }
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
				Internal_gettint(mCachedPtr, out temp);
				return temp;
			}
			set { Internal_settint(mCachedPtr, ref value); }
		}

		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_BloomSettings(BloomSettings managedInstance);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern bool Internal_getenabled(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_setenabled(IntPtr thisPtr, bool value);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern int Internal_getquality(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_setquality(IntPtr thisPtr, int value);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern float Internal_getthreshold(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_setthreshold(IntPtr thisPtr, float value);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern float Internal_getintensity(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_setintensity(IntPtr thisPtr, float value);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_gettint(IntPtr thisPtr, out Color __output);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_settint(IntPtr thisPtr, ref Color value);
	}

	/** @} */
}
