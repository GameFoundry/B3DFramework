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

	/// <summary>
	/// Settings that control the screen space reflections effect. Screen space reflections provide high quality mirror-like 
	/// reflections at low performance cost. They should be used together with reflection probes as the effects complement 
	/// each other. As the name implies, the reflections are only limited to geometry drawn on the screen and the system will 
	/// fall back to refl. probes when screen space data is unavailable. Similarly the system will fall back to refl. probes 
	/// for rougher (more glossy rather than mirror-like) surfaces. Those surfaces require a higher number of samples to 
	/// achieve the glossy look, so we instead fall back to refl. probes which are pre-filtered and can be quickly sampled.
	/// </summary>
	[ShowInInspector]
	public partial class ScreenSpaceReflectionsSettings : ScriptObject
	{
		private ScreenSpaceReflectionsSettings(bool __dummy0) { }

		public ScreenSpaceReflectionsSettings()
		{
			Internal_ScreenSpaceReflectionsSettings(this);
		}

		/// <summary>Enables or disables the SSR effect.</summary>
		[ShowInInspector]
		[NativeWrapper]
		public bool Enabled
		{
			get { return Internal_getenabled(mCachedPtr); }
			set { Internal_setenabled(mCachedPtr, value); }
		}

		/// <summary>
		/// Quality of the SSR effect. Higher values cast more sample rays, and march those rays are lower increments for better 
		/// precision. This results in higher quality, as well as a higher performance requirement. Valid range is [0, 4], 
		/// default is 2.
		/// </summary>
		[ShowInInspector]
		[NativeWrapper]
		public int Quality
		{
			get { return Internal_getquality(mCachedPtr); }
			set { Internal_setquality(mCachedPtr, value); }
		}

		/// <summary>Intensity of the screen space reflections. Valid range is [0, 1]. Default is 1 (100%).</summary>
		[ShowInInspector]
		[NativeWrapper]
		public float Intensity
		{
			get { return Internal_getintensity(mCachedPtr); }
			set { Internal_setintensity(mCachedPtr, value); }
		}

		/// <summary>
		/// Roughness at which screen space reflections start fading out and become replaced with refl. probes. Valid range is 
		/// [0, 1]. Default is 0.8.
		/// </summary>
		[ShowInInspector]
		[NativeWrapper]
		public float MaxRoughness
		{
			get { return Internal_getmaxRoughness(mCachedPtr); }
			set { Internal_setmaxRoughness(mCachedPtr, value); }
		}

		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_ScreenSpaceReflectionsSettings(ScreenSpaceReflectionsSettings managedInstance);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern bool Internal_getenabled(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_setenabled(IntPtr thisPtr, bool value);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern int Internal_getquality(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_setquality(IntPtr thisPtr, int value);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern float Internal_getintensity(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_setintensity(IntPtr thisPtr, float value);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern float Internal_getmaxRoughness(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_setmaxRoughness(IntPtr thisPtr, float value);
	}

	/** @} */
}
