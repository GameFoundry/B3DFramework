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

	/// <summary>Settings that control screen space ambient occlusion.</summary>
	[ShowInInspector]
	public partial class AmbientOcclusionSettings : ScriptObject
	{
		private AmbientOcclusionSettings(bool __dummy0) { }

		public AmbientOcclusionSettings()
		{
			Internal_AmbientOcclusionSettings(this);
		}

		/// <summary>Enables or disables the screen space ambient occlusion effect.</summary>
		[ShowInInspector]
		[NativeWrapper]
		public bool Enabled
		{
			get { return Internal_GetEnabled(mCachedPtr); }
			set { Internal_SetEnabled(mCachedPtr, value); }
		}

		/// <summary>
		/// Radius (in world space, in meters) over which occluders are searched for. Smaller radius ensures better sampling 
		/// precision but can miss occluders. Larger radius ensures far away occluders are considered but can yield lower quality 
		/// or noise because of low sampling precision. Usually best to keep at around a meter, valid range is roughly [0.05, 
		/// 5.0].
		/// </summary>
		[ShowInInspector]
		[Range(0.05f, 5f, true)]
		[NativeWrapper]
		public float Radius
		{
			get { return Internal_GetRadius(mCachedPtr); }
			set { Internal_SetRadius(mCachedPtr, value); }
		}

		/// <summary>
		/// Bias used to reduce false occlusion artifacts. Higher values reduce the amount of artifacts but will cause details to 
		/// be lost in areas where occlusion isn&apos;t high. Value is in millimeters. Usually best to keep at a few dozen 
		/// millimeters, valid range is roughly [0, 200].
		/// </summary>
		[ShowInInspector]
		[Range(0f, 200f, true)]
		[NativeWrapper]
		public float Bias
		{
			get { return Internal_GetBias(mCachedPtr); }
			set { Internal_SetBias(mCachedPtr, value); }
		}

		/// <summary>
		/// Distance (in view space, in meters) after which AO starts fading out. The fade process will happen over the range as 
		/// specified by <see cref="fadeRange"/>.
		/// </summary>
		[ShowInInspector]
		[NativeWrapper]
		public float FadeDistance
		{
			get { return Internal_GetFadeDistance(mCachedPtr); }
			set { Internal_SetFadeDistance(mCachedPtr, value); }
		}

		/// <summary>
		/// Range (in view space, in meters) in which AO fades out from 100% to 0%. AO starts fading out after the distance 
		/// specified in <see cref="fadeDistance"/>.
		/// </summary>
		[ShowInInspector]
		[NativeWrapper]
		public float FadeRange
		{
			get { return Internal_GetFadeRange(mCachedPtr); }
			set { Internal_SetFadeRange(mCachedPtr, value); }
		}

		/// <summary>
		/// Linearly scales the intensity of the AO effect. Values less than 1 make the AO effect less pronounced, and vice 
		/// versa. Valid range is roughly [0.2, 2].
		/// </summary>
		[ShowInInspector]
		[Range(0.2f, 2f, true)]
		[NativeWrapper]
		public float Intensity
		{
			get { return Internal_GetIntensity(mCachedPtr); }
			set { Internal_SetIntensity(mCachedPtr, value); }
		}

		/// <summary>
		/// Controls how quickly does the AO darkening effect increase with higher occlusion percent. This is a non-linear 
		/// control and will cause the darkening to ramp up exponentially. Valid range is roughly [1, 4], where 1 means no extra 
		/// darkening will occur.
		/// </summary>
		[ShowInInspector]
		[Range(1f, 4f, true)]
		[NativeWrapper]
		public float Power
		{
			get { return Internal_GetPower(mCachedPtr); }
			set { Internal_SetPower(mCachedPtr, value); }
		}

		/// <summary>
		/// Quality level of generated ambient occlusion. In range [0, 4]. Higher levels yield higher quality AO at the cost of 
		/// performance.
		/// </summary>
		[ShowInInspector]
		[Range(0f, 4f, true)]
		[NativeWrapper]
		public int Quality
		{
			get { return Internal_GetQuality(mCachedPtr); }
			set { Internal_SetQuality(mCachedPtr, value); }
		}

		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_AmbientOcclusionSettings(AmbientOcclusionSettings managedInstance);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern bool Internal_GetEnabled(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SetEnabled(IntPtr thisPtr, bool value);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern float Internal_GetRadius(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SetRadius(IntPtr thisPtr, float value);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern float Internal_GetBias(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SetBias(IntPtr thisPtr, float value);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern float Internal_GetFadeDistance(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SetFadeDistance(IntPtr thisPtr, float value);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern float Internal_GetFadeRange(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SetFadeRange(IntPtr thisPtr, float value);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern float Internal_GetIntensity(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SetIntensity(IntPtr thisPtr, float value);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern float Internal_GetPower(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SetPower(IntPtr thisPtr, float value);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern int Internal_GetQuality(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SetQuality(IntPtr thisPtr, int value);
	}

	/** @} */
}
