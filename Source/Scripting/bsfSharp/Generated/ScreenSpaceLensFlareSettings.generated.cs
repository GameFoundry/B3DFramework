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
			get { return Internal_GetEnabled(mCachedPtr); }
			set { Internal_SetEnabled(mCachedPtr, value); }
		}

		/// <summary>
		/// Determines how many times to downsample the scene texture before using it for lens flare effect. Lower values will 
		/// use higher resolution texture for calculating lens flare, at the cost of lower performance. Valid range is [1, 6], 
		/// default is 4.
		/// </summary>
		[ShowInInspector]
		[Range(1f, 6f, false)]
		[NativeWrapper]
		public int DownsampleCount
		{
			get { return Internal_GetDownsampleCount(mCachedPtr); }
			set { Internal_SetDownsampleCount(mCachedPtr, value); }
		}

		/// <summary>
		/// Determines the minimal threshold of pixel luminance to be included in the lens flare calculations. Any pixel with 
		/// luminance below this value will be ignored for the purposes of lens flare. Set to zero or negative to disable the 
		/// threshold and include all pixels in the calculations.
		/// </summary>
		[ShowInInspector]
		[NativeWrapper]
		public float Threshold
		{
			get { return Internal_GetThreshold(mCachedPtr); }
			set { Internal_SetThreshold(mCachedPtr, value); }
		}

		/// <summary>
		/// Determines the number of ghost features to appear, shown as blurred blobs of bright areas of the scene.
		/// </summary>
		[ShowInInspector]
		[Range(1f, 10f, false)]
		[NativeWrapper]
		public int GhostCount
		{
			get { return Internal_GetGhostCount(mCachedPtr); }
			set { Internal_SetGhostCount(mCachedPtr, value); }
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
			get { return Internal_GetGhostSpacing(mCachedPtr); }
			set { Internal_SetGhostSpacing(mCachedPtr, value); }
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
			get { return Internal_GetBrightness(mCachedPtr); }
			set { Internal_SetBrightness(mCachedPtr, value); }
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
			get { return Internal_GetFilterSize(mCachedPtr); }
			set { Internal_SetFilterSize(mCachedPtr, value); }
		}

		/// <summary>Determines if a halo effect should be rendered as part of the lens flare.</summary>
		[ShowInInspector]
		[NativeWrapper]
		public bool Halo
		{
			get { return Internal_GetHalo(mCachedPtr); }
			set { Internal_SetHalo(mCachedPtr, value); }
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
			get { return Internal_GetHaloRadius(mCachedPtr); }
			set { Internal_SetHaloRadius(mCachedPtr, value); }
		}

		/// <summary>Determines the thickness of the halo ring. In normalized screen space (range [0.01,0.5]).</summary>
		[ShowInInspector]
		[Range(0.01f, 0.5f, false)]
		[NativeWrapper]
		public float HaloThickness
		{
			get { return Internal_GetHaloThickness(mCachedPtr); }
			set { Internal_SetHaloThickness(mCachedPtr, value); }
		}

		/// <summary>
		/// Determines the minimal threshold of pixel luminance to be included for halo generation. Any pixel with luminance 
		/// below this value will be ignored for the purposes of halo generation.
		/// </summary>
		[ShowInInspector]
		[NativeWrapper]
		public float HaloThreshold
		{
			get { return Internal_GetHaloThreshold(mCachedPtr); }
			set { Internal_SetHaloThreshold(mCachedPtr, value); }
		}

		/// <summary>
		/// Determines the shape of the halo. Set to value other than 1 to make the halo an oval rather than a circle.
		/// </summary>
		[ShowInInspector]
		[Range(0f, 2f, false)]
		[NativeWrapper]
		public float HaloAspectRatio
		{
			get { return Internal_GetHaloAspectRatio(mCachedPtr); }
			set { Internal_SetHaloAspectRatio(mCachedPtr, value); }
		}

		/// <summary>
		/// Enables or disables chromatic aberration of the lens flare and halo features. Chromatic aberration separates the 
		/// values of red, green and blue channels according to a user provided offset.
		/// </summary>
		[ShowInInspector]
		[NativeWrapper]
		public bool ChromaticAberration
		{
			get { return Internal_GetChromaticAberration(mCachedPtr); }
			set { Internal_SetChromaticAberration(mCachedPtr, value); }
		}

		/// <summary>
		/// Determines the distance between pixels within which to sample different channels. The value is in UV coordinates, 
		/// range [0, 1].
		/// </summary>
		[ShowInInspector]
		[Range(0f, 1f, false)]
		[NativeWrapper]
		public float ChromaticAberrationOffset
		{
			get { return Internal_GetChromaticAberrationOffset(mCachedPtr); }
			set { Internal_SetChromaticAberrationOffset(mCachedPtr, value); }
		}

		/// <summary>
		/// Uses a higher quality upscaling when blending the lens flare features with scene color. Results in less blocky 
		/// artifacts at a cost to performance.
		/// </summary>
		[ShowInInspector]
		[NativeWrapper]
		public bool BicubicUpsampling
		{
			get { return Internal_GetBicubicUpsampling(mCachedPtr); }
			set { Internal_SetBicubicUpsampling(mCachedPtr, value); }
		}

		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_ScreenSpaceLensFlareSettings(ScreenSpaceLensFlareSettings managedInstance);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern bool Internal_GetEnabled(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SetEnabled(IntPtr thisPtr, bool value);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern int Internal_GetDownsampleCount(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SetDownsampleCount(IntPtr thisPtr, int value);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern float Internal_GetThreshold(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SetThreshold(IntPtr thisPtr, float value);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern int Internal_GetGhostCount(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SetGhostCount(IntPtr thisPtr, int value);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern float Internal_GetGhostSpacing(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SetGhostSpacing(IntPtr thisPtr, float value);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern float Internal_GetBrightness(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SetBrightness(IntPtr thisPtr, float value);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern float Internal_GetFilterSize(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SetFilterSize(IntPtr thisPtr, float value);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern bool Internal_GetHalo(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SetHalo(IntPtr thisPtr, bool value);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern float Internal_GetHaloRadius(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SetHaloRadius(IntPtr thisPtr, float value);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern float Internal_GetHaloThickness(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SetHaloThickness(IntPtr thisPtr, float value);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern float Internal_GetHaloThreshold(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SetHaloThreshold(IntPtr thisPtr, float value);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern float Internal_GetHaloAspectRatio(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SetHaloAspectRatio(IntPtr thisPtr, float value);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern bool Internal_GetChromaticAberration(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SetChromaticAberration(IntPtr thisPtr, bool value);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern float Internal_GetChromaticAberrationOffset(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SetChromaticAberrationOffset(IntPtr thisPtr, float value);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern bool Internal_GetBicubicUpsampling(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SetBicubicUpsampling(IntPtr thisPtr, bool value);
	}

	/** @} */
}
