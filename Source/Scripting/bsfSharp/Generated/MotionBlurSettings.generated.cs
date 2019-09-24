//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
using System;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;

namespace bs
{
	/** @addtogroup Rendering
	 *  @{
	 */

	/// <summary>Settings that control the motion blur effect.</summary>
	[ShowInInspector]
	public partial class MotionBlurSettings : ScriptObject
	{
		private MotionBlurSettings(bool __dummy0) { }

		public MotionBlurSettings()
		{
			Internal_MotionBlurSettings(this);
		}

		/// <summary>Enables or disables the motion blur effect.</summary>
		[ShowInInspector]
		[NativeWrapper]
		public bool Enabled
		{
			get { return Internal_getenabled(mCachedPtr); }
			set { Internal_setenabled(mCachedPtr, value); }
		}

		/// <summary>Determines which parts of the scene will trigger motion blur.</summary>
		[ShowInInspector]
		[NativeWrapper]
		public MotionBlurDomain Domain
		{
			get { return Internal_getdomain(mCachedPtr); }
			set { Internal_setdomain(mCachedPtr, value); }
		}

		/// <summary>Type of filter to use when filtering samples contributing to a blurred pixel.</summary>
		[ShowInInspector]
		[NativeWrapper]
		public MotionBlurFilter Filter
		{
			get { return Internal_getfilter(mCachedPtr); }
			set { Internal_setfilter(mCachedPtr, value); }
		}

		/// <summary>
		/// Determines the number of samples to take during motion blur filtering. Increasing this value will yield higher 
		/// quality blur at the cost of the performance.
		/// </summary>
		[ShowInInspector]
		[NativeWrapper]
		public MotionBlurQuality Quality
		{
			get { return Internal_getquality(mCachedPtr); }
			set { Internal_setquality(mCachedPtr, value); }
		}

		/// <summary>
		/// Determines the maximum radius over which the blur samples are allowed to be taken, in percent of the screen width 
		/// (e.g. with 1% radius, on 1920x1028 resolution the maximum radius in pixels will be 1920 * 0.01 = 20px). This clamps 
		/// the maximum velocity that can affect the blur, as higher velocities require higher radius. Very high values can 
		/// adversely affect performance as cache accesses become more random.
		/// </summary>
		[ShowInInspector]
		[NativeWrapper]
		public float MaximumRadius
		{
			get { return Internal_getmaximumRadius(mCachedPtr); }
			set { Internal_setmaximumRadius(mCachedPtr, value); }
		}

		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_MotionBlurSettings(MotionBlurSettings managedInstance);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern bool Internal_getenabled(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_setenabled(IntPtr thisPtr, bool value);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern MotionBlurDomain Internal_getdomain(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_setdomain(IntPtr thisPtr, MotionBlurDomain value);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern MotionBlurFilter Internal_getfilter(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_setfilter(IntPtr thisPtr, MotionBlurFilter value);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern MotionBlurQuality Internal_getquality(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_setquality(IntPtr thisPtr, MotionBlurQuality value);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern float Internal_getmaximumRadius(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_setmaximumRadius(IntPtr thisPtr, float value);
	}

	/** @} */
}
