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

	/// <summary>Settings that control the screen-space lens flare effect.</summary>
	[ShowInInspector]
	public partial class ScreenSpaceLensFlareSettings : ScriptObject
	{
		private ScreenSpaceLensFlareSettings(bool __dummy0) { }

		public ScreenSpaceLensFlareSettings()
		{
			Internal_ScreenSpaceLensFlareSettings(this);
		}

		/// <summary>Enables or disables the lens flare effect.</summary>
		[ShowInInspector]
		[NativeWrapper]
		public bool Enabled
		{
			get { return Internal_getenabled(mCachedPtr); }
			set { Internal_setenabled(mCachedPtr, value); }
		}

		/// <summary>
		/// Determines how many times to downsample the scene texture before using it for lens flare effect. Lower values  will 
		/// use higher resolution texture for calculating lens flare, at the cost of lower performance.  Valid range is [1, 6], 
		/// default is 4.
		/// </summary>
		[ShowInInspector]
		[Range(1f, 6f, false)]
		[NativeWrapper]
		public int DownsampleCount
		{
			get { return Internal_getdownsampleCount(mCachedPtr); }
			set { Internal_setdownsampleCount(mCachedPtr, value); }
		}

		/// <summary>
		/// Determines the minimal threshold of pixel luminance to be included in the lens flare calculations. Any pixel  with 
		/// luminance below this value will be ignored for the purposes of lens flare. Set to zero or negative to  disable the 
		/// threshold and include all pixels in the calculations.
		/// </summary>
		[ShowInInspector]
		[NativeWrapper]
		public float Threshold
		{
			get { return Internal_getthreshold(mCachedPtr); }
			set { Internal_setthreshold(mCachedPtr, value); }
		}

		/// <summary>
		/// Determines the number of ghost features to appear, shown as blurred blobs of bright areas of the scene.
		/// </summary>
		[ShowInInspector]
		[Range(1f, 10f, false)]
		[NativeWrapper]
		public int GhostCount
		{
			get { return Internal_getghostCount(mCachedPtr); }
			set { Internal_setghostCount(mCachedPtr, value); }
		}

		/// <summary>
		/// Determines the distance between ghost features. Value is in normalized screen space, in range [0,1] where 1 
		/// represents the full screen length.
		/// </summary>
		[ShowInInspector]
		[Range(0f, 1f, false)]
		[NativeWrapper]
		public float GhostSpacing
		{
			get { return Internal_getghostSpacing(mCachedPtr); }
			set { Internal_setghostSpacing(mCachedPtr, value); }
		}

		/// <summary>
		/// Determines the brightness of the lens flare effect. Value of 1 means the lens flare will be displayed at the same 
		/// intensity as the scene it was derived from. In range [0, 1], default being 0.05.
		/// </summary>
		[ShowInInspector]
		[Range(0f, 1f, false)]
		[NativeWrapper]
		public float Brightness
		{
			get { return Internal_getbrightness(mCachedPtr); }
			set { Internal_setbrightness(mCachedPtr, value); }
		}

		/// <summary>
		/// Determines the size of the filter when blurring the lens flare features. Larger values yield a blurrier image and 
		/// will require more performance.
		/// </summary>
		[ShowInInspector]
		[Range(0.01f, 1f, false)]
		[NativeWrapper]
		public float FilterSize
		{
			get { return Internal_getfilterSize(mCachedPtr); }
			set { Internal_setfilterSize(mCachedPtr, value); }
		}

		/// <summary>Determines if a halo effect should be rendered as part of the lens flare.</summary>
		[ShowInInspector]
		[NativeWrapper]
		public bool Halo
		{
			get { return Internal_gethalo(mCachedPtr); }
			set { Internal_sethalo(mCachedPtr, value); }
		}

		/// <summary>
		/// Determines how far away from the screen center does the halo appear, in normalized screen space (range [0,1]) where 
		/// 0.5 represents half screen length.
		/// </summary>
		[ShowInInspector]
		[Range(0f, 1f, false)]
		[NativeWrapper]
		public float HaloRadius
		{
			get { return Internal_gethaloRadius(mCachedPtr); }
			set { Internal_sethaloRadius(mCachedPtr, value); }
		}

		/// <summary>Determines the thickness of the halo ring. In normalized screen space (range [0.01,0.5]).</summary>
		[ShowInInspector]
		[Range(0.01f, 0.5f, false)]
		[NativeWrapper]
		public float HaloThickness
		{
			get { return Internal_gethaloThickness(mCachedPtr); }
			set { Internal_sethaloThickness(mCachedPtr, value); }
		}

		/// <summary>
		/// Determines the minimal threshold of pixel luminance to be included for halo generation. Any pixel with luminance 
		/// below this value will be ignored for the purposes of halo generation.
		/// </summary>
		[ShowInInspector]
		[NativeWrapper]
		public float HaloThreshold
		{
			get { return Internal_gethaloThreshold(mCachedPtr); }
			set { Internal_sethaloThreshold(mCachedPtr, value); }
		}

		/// <summary>
		/// Determines the shape of the halo. Set to value other than 1 to make the halo an oval rather than a circle.
		/// </summary>
		[ShowInInspector]
		[Range(0f, 2f, false)]
		[NativeWrapper]
		public float HaloAspectRatio
		{
			get { return Internal_gethaloAspectRatio(mCachedPtr); }
			set { Internal_sethaloAspectRatio(mCachedPtr, value); }
		}

		/// <summary>
		/// Enables or disables chromatic aberration of the lens flare and halo features. Chromatic aberration separates the 
		/// values of red, green and blue channels according to a user provided offset.
		/// </summary>
		[ShowInInspector]
		[NativeWrapper]
		public bool ChromaticAberration
		{
			get { return Internal_getchromaticAberration(mCachedPtr); }
			set { Internal_setchromaticAberration(mCachedPtr, value); }
		}

		/// <summary>
		/// Determines the distance between pixels within which to sample different channels. The value is in  UV coordinates, 
		/// range [0, 1].
		/// </summary>
		[ShowInInspector]
		[Range(0f, 1f, false)]
		[NativeWrapper]
		public float ChromaticAberrationOffset
		{
			get { return Internal_getchromaticAberrationOffset(mCachedPtr); }
			set { Internal_setchromaticAberrationOffset(mCachedPtr, value); }
		}

		/// <summary>
		/// Uses a higher quality upscaling when blending the lens flare features with scene color. Results in less blocky 
		/// artifacts at a cost to performance.
		/// </summary>
		[ShowInInspector]
		[NativeWrapper]
		public bool BicubicUpsampling
		{
			get { return Internal_getbicubicUpsampling(mCachedPtr); }
			set { Internal_setbicubicUpsampling(mCachedPtr, value); }
		}

		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_ScreenSpaceLensFlareSettings(ScreenSpaceLensFlareSettings managedInstance);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern bool Internal_getenabled(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_setenabled(IntPtr thisPtr, bool value);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern int Internal_getdownsampleCount(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_setdownsampleCount(IntPtr thisPtr, int value);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern float Internal_getthreshold(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_setthreshold(IntPtr thisPtr, float value);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern int Internal_getghostCount(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_setghostCount(IntPtr thisPtr, int value);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern float Internal_getghostSpacing(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_setghostSpacing(IntPtr thisPtr, float value);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern float Internal_getbrightness(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_setbrightness(IntPtr thisPtr, float value);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern float Internal_getfilterSize(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_setfilterSize(IntPtr thisPtr, float value);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern bool Internal_gethalo(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_sethalo(IntPtr thisPtr, bool value);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern float Internal_gethaloRadius(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_sethaloRadius(IntPtr thisPtr, float value);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern float Internal_gethaloThickness(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_sethaloThickness(IntPtr thisPtr, float value);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern float Internal_gethaloThreshold(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_sethaloThreshold(IntPtr thisPtr, float value);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern float Internal_gethaloAspectRatio(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_sethaloAspectRatio(IntPtr thisPtr, float value);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern bool Internal_getchromaticAberration(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_setchromaticAberration(IntPtr thisPtr, bool value);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern float Internal_getchromaticAberrationOffset(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_setchromaticAberrationOffset(IntPtr thisPtr, float value);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern bool Internal_getbicubicUpsampling(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_setbicubicUpsampling(IntPtr thisPtr, bool value);
	}

	/** @} */
}
