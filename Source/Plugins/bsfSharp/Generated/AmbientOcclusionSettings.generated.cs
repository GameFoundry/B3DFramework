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

	/// <summary>Settings that control screen space ambient occlusion.</summary>
	[ShowInInspector]
	public partial class AmbientOcclusionSettings : ScriptObject
	{
		private AmbientOcclusionSettings(bool __dummy0) { }

		public AmbientOcclusionSettings()
		{
			Internal_AmbientOcclusionSettings(this);
		}

		/// <summary>Enables or disabled the screen space ambient occlusion effect.</summary>
		[ShowInInspector]
		[NativeWrapper]
		public bool Enabled
		{
			get { return Internal_getenabled(mCachedPtr); }
			set { Internal_setenabled(mCachedPtr, value); }
		}

		/// <summary>
		/// Radius (in world space, in meters) over which occluders are searched for. Smaller radius ensures better sampling 
		/// precision but can miss occluders. Larger radius ensures far away occluders are considered but can yield lower quality 
		/// or noise because of low sampling precision. Usually best to keep at around a meter, valid range is roughly [0.05, 
		/// 5.0].
		/// </summary>
		[ShowInInspector]
		[NativeWrapper]
		public float Radius
		{
			get { return Internal_getradius(mCachedPtr); }
			set { Internal_setradius(mCachedPtr, value); }
		}

		/// <summary>
		/// Bias used to reduce false occlusion artifacts. Higher values reduce the amount of artifacts but will cause details to 
		/// be lost in areas where occlusion isn't high. Value is in millimeters. Usually best to keep at a few dozen 
		/// millimeters, valid range is roughly [0, 200].
		/// </summary>
		[ShowInInspector]
		[NativeWrapper]
		public float Bias
		{
			get { return Internal_getbias(mCachedPtr); }
			set { Internal_setbias(mCachedPtr, value); }
		}

		/// <summary>
		/// Distance (in view space, in meters) after which AO starts fading out. The fade process will happen over the range as 
		/// specified by <see cref="fadeRange"/>.
		/// </summary>
		[ShowInInspector]
		[NativeWrapper]
		public float FadeDistance
		{
			get { return Internal_getfadeDistance(mCachedPtr); }
			set { Internal_setfadeDistance(mCachedPtr, value); }
		}

		/// <summary>
		/// Range (in view space, in meters) in which AO fades out from 100% to 0%. AO starts fading out after the distance 
		/// specified in <see cref="fadeDistance"/>.
		/// </summary>
		[ShowInInspector]
		[NativeWrapper]
		public float FadeRange
		{
			get { return Internal_getfadeRange(mCachedPtr); }
			set { Internal_setfadeRange(mCachedPtr, value); }
		}

		/// <summary>
		/// Linearly scales the intensity of the AO effect. Values less than 1 make the AO effect less pronounced, and vice 
		/// versa. Valid range is roughly [0.2, 2].
		/// </summary>
		[ShowInInspector]
		[NativeWrapper]
		public float Intensity
		{
			get { return Internal_getintensity(mCachedPtr); }
			set { Internal_setintensity(mCachedPtr, value); }
		}

		/// <summary>
		/// Controls how quickly does the AO darkening effect increase with higher occlusion percent. This is a non-linear 
		/// control and will cause the darkening to ramp up exponentially. Valid range is roughly [1, 4], where 1 means no extra 
		/// darkening will occur.
		/// </summary>
		[ShowInInspector]
		[NativeWrapper]
		public float Power
		{
			get { return Internal_getpower(mCachedPtr); }
			set { Internal_setpower(mCachedPtr, value); }
		}

		/// <summary>
		/// Quality level of generated ambient occlusion. In range [0, 4]. Higher levels yield higher quality AO at the cost of 
		/// performance.
		/// </summary>
		[ShowInInspector]
		[NativeWrapper]
		public uint Quality
		{
			get { return Internal_getquality(mCachedPtr); }
			set { Internal_setquality(mCachedPtr, value); }
		}

		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_AmbientOcclusionSettings(AmbientOcclusionSettings managedInstance);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern bool Internal_getenabled(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_setenabled(IntPtr thisPtr, bool value);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern float Internal_getradius(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_setradius(IntPtr thisPtr, float value);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern float Internal_getbias(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_setbias(IntPtr thisPtr, float value);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern float Internal_getfadeDistance(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_setfadeDistance(IntPtr thisPtr, float value);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern float Internal_getfadeRange(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_setfadeRange(IntPtr thisPtr, float value);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern float Internal_getintensity(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_setintensity(IntPtr thisPtr, float value);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern float Internal_getpower(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_setpower(IntPtr thisPtr, float value);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern uint Internal_getquality(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_setquality(IntPtr thisPtr, uint value);
	}

	/** @} */
}
