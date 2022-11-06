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

	/// <summary>Settings that control rendering for a specific camera (view).</summary>
	[ShowInInspector]
	public partial class RenderSettings : ScriptObject
	{
		private RenderSettings(bool __dummy0) { }

		public RenderSettings()
		{
			Internal_RenderSettings(this);
		}

		/// <summary>Parameters used for customizing the gaussian depth of field effect.</summary>
		[ShowInInspector]
		[NotNull]
		[PassByCopy]
		[NativeWrapper]
		public DepthOfFieldSettings DepthOfField
		{
			get { return Internal_GetDepthOfField(mCachedPtr); }
			set { Internal_SetDepthOfField(mCachedPtr, value); }
		}

		/// <summary>Parameters used for customizing the chromatic aberration effect.</summary>
		[ShowInInspector]
		[NotNull]
		[PassByCopy]
		[NativeWrapper]
		public ChromaticAberrationSettings ChromaticAberration
		{
			get { return Internal_GetChromaticAberration(mCachedPtr); }
			set { Internal_SetChromaticAberration(mCachedPtr, value); }
		}

		/// <summary>
		/// Determines should automatic exposure be applied to the HDR image. When turned on the average scene brightness will be 
		/// calculated and used to automatically expose the image to the optimal range. Use the parameters provided by 
		/// autoExposure to customize the automatic exposure effect. You may also use exposureScale to manually adjust the 
		/// automatic exposure. When automatic exposure is turned off you can use exposureScale to manually set the exposure.
		/// </summary>
		[ShowInInspector]
		[NativeWrapper]
		public bool EnableAutoExposure
		{
			get { return Internal_GetEnableAutoExposure(mCachedPtr); }
			set { Internal_SetEnableAutoExposure(mCachedPtr, value); }
		}

		/// <summary>Parameters used for customizing automatic scene exposure.</summary>
		[ShowInInspector]
		[NotNull]
		[PassByCopy]
		[NativeWrapper]
		public AutoExposureSettings AutoExposure
		{
			get { return Internal_GetAutoExposure(mCachedPtr); }
			set { Internal_SetAutoExposure(mCachedPtr, value); }
		}

		/// <summary>
		/// Determines should the image be tonemapped. Tonemapping converts an HDR image into LDR image by applying a filmic 
		/// curve to the image, simulating the effect of film cameras. Filmic curve improves image quality by tapering off lows 
		/// and highs, preventing under- and over-exposure. This is useful if an image contains both very dark and very bright 
		/// areas, in which case the global exposure parameter would leave some areas either over- or under-exposed. Use 
		/// #tonemapping to customize how tonemapping performed.
		///
		/// If this is disabled, then color grading and white balancing will not be enabled either. Only relevant for HDR images.
		/// </summary>
		[ShowInInspector]
		[NativeWrapper]
		public bool EnableTonemapping
		{
			get { return Internal_GetEnableTonemapping(mCachedPtr); }
			set { Internal_SetEnableTonemapping(mCachedPtr, value); }
		}

		/// <summary>Parameters used for customizing tonemapping.</summary>
		[ShowInInspector]
		[NotNull]
		[PassByCopy]
		[NativeWrapper]
		public TonemappingSettings Tonemapping
		{
			get { return Internal_GetTonemapping(mCachedPtr); }
			set { Internal_SetTonemapping(mCachedPtr, value); }
		}

		/// <summary>
		/// Parameters used for customizing white balancing. White balancing converts a scene illuminated by a light of the 
		/// specified temperature into a scene illuminated by a standard D65 illuminant (average midday light) in order to 
		/// simulate the effects of chromatic adaptation of the human visual system.
		/// </summary>
		[ShowInInspector]
		[NotNull]
		[PassByCopy]
		[NativeWrapper]
		public WhiteBalanceSettings WhiteBalance
		{
			get { return Internal_GetWhiteBalance(mCachedPtr); }
			set { Internal_SetWhiteBalance(mCachedPtr, value); }
		}

		/// <summary>Parameters used for customizing color grading.</summary>
		[ShowInInspector]
		[NotNull]
		[PassByCopy]
		[NativeWrapper]
		public ColorGradingSettings ColorGrading
		{
			get { return Internal_GetColorGrading(mCachedPtr); }
			set { Internal_SetColorGrading(mCachedPtr, value); }
		}

		/// <summary>Parameters used for customizing screen space ambient occlusion.</summary>
		[ShowInInspector]
		[NotNull]
		[PassByCopy]
		[NativeWrapper]
		public AmbientOcclusionSettings AmbientOcclusion
		{
			get { return Internal_GetAmbientOcclusion(mCachedPtr); }
			set { Internal_SetAmbientOcclusion(mCachedPtr, value); }
		}

		/// <summary>Parameters used for customizing screen space reflections.</summary>
		[ShowInInspector]
		[NotNull]
		[PassByCopy]
		[NativeWrapper]
		public ScreenSpaceReflectionsSettings ScreenSpaceReflections
		{
			get { return Internal_GetScreenSpaceReflections(mCachedPtr); }
			set { Internal_SetScreenSpaceReflections(mCachedPtr, value); }
		}

		/// <summary>Parameters used for customizing the bloom effect.</summary>
		[ShowInInspector]
		[NotNull]
		[PassByCopy]
		[NativeWrapper]
		public BloomSettings Bloom
		{
			get { return Internal_GetBloom(mCachedPtr); }
			set { Internal_SetBloom(mCachedPtr, value); }
		}

		/// <summary>Parameters used for customizing the screen space lens flare effect.</summary>
		[ShowInInspector]
		[NotNull]
		[PassByCopy]
		[NativeWrapper]
		public ScreenSpaceLensFlareSettings ScreenSpaceLensFlare
		{
			get { return Internal_GetScreenSpaceLensFlare(mCachedPtr); }
			set { Internal_SetScreenSpaceLensFlare(mCachedPtr, value); }
		}

		/// <summary>Parameters used for customizing the film grain effect.</summary>
		[ShowInInspector]
		[NotNull]
		[PassByCopy]
		[NativeWrapper]
		public FilmGrainSettings FilmGrain
		{
			get { return Internal_GetFilmGrain(mCachedPtr); }
			set { Internal_SetFilmGrain(mCachedPtr, value); }
		}

		/// <summary>Parameters used for customizing the motion blur effect.</summary>
		[ShowInInspector]
		[NotNull]
		[PassByCopy]
		[NativeWrapper]
		public MotionBlurSettings MotionBlur
		{
			get { return Internal_GetMotionBlur(mCachedPtr); }
			set { Internal_SetMotionBlur(mCachedPtr, value); }
		}

		/// <summary>Parameters used for customizing the temporal anti-aliasing effect.</summary>
		[ShowInInspector]
		[NotNull]
		[PassByCopy]
		[NativeWrapper]
		public TemporalAASettings TemporalAa
		{
			get { return Internal_GetTemporalAa(mCachedPtr); }
			set { Internal_SetTemporalAa(mCachedPtr, value); }
		}

		/// <summary>Enables the fast approximate anti-aliasing effect.</summary>
		[ShowInInspector]
		[NativeWrapper]
		public bool EnableFxaa
		{
			get { return Internal_GetEnableFxaa(mCachedPtr); }
			set { Internal_SetEnableFxaa(mCachedPtr, value); }
		}

		/// <summary>
		/// Log2 value to scale the eye adaptation by (for example 2^0 = 1). Smaller values yield darker image, while larger 
		/// yield brighter image. Allows you to customize exposure manually, applied on top of eye adaptation exposure (if 
		/// enabled). In range [-8, 8].
		/// </summary>
		[ShowInInspector]
		[Range(-8f, 8f, true)]
		[NativeWrapper]
		public float ExposureScale
		{
			get { return Internal_GetExposureScale(mCachedPtr); }
			set { Internal_SetExposureScale(mCachedPtr, value); }
		}

		/// <summary>
		/// Gamma value to adjust the image for. Larger values result in a brighter image. When tonemapping is turned on the best 
		/// gamma curve for the output device is chosen automatically and this value can by used to merely tweak that curve. If 
		/// tonemapping is turned off this is the exact value of the gamma curve that will be applied.
		/// </summary>
		[ShowInInspector]
		[Range(1f, 3f, true)]
		[NativeWrapper]
		public float Gamma
		{
			get { return Internal_GetGamma(mCachedPtr); }
			set { Internal_SetGamma(mCachedPtr, value); }
		}

		/// <summary>
		/// High dynamic range allows light intensity to be more correctly recorded when rendering by allowing for a larger range 
		/// of values. The stored light is then converted into visible color range using exposure and a tone mapping operator.
		/// </summary>
		[ShowInInspector]
		[NativeWrapper]
		public bool EnableHdr
		{
			get { return Internal_GetEnableHdr(mCachedPtr); }
			set { Internal_SetEnableHdr(mCachedPtr, value); }
		}

		/// <summary>
		/// Determines if scene objects will be lit by lights. If disabled everything will be rendered using their albedo texture 
		/// with no lighting applied.
		/// </summary>
		[ShowInInspector]
		[NativeWrapper]
		public bool EnableLighting
		{
			get { return Internal_GetEnableLighting(mCachedPtr); }
			set { Internal_SetEnableLighting(mCachedPtr, value); }
		}

		/// <summary>Determines if shadows cast by lights should be rendered. Only relevant if lighting is turned on.</summary>
		[ShowInInspector]
		[NativeWrapper]
		public bool EnableShadows
		{
			get { return Internal_GetEnableShadows(mCachedPtr); }
			set { Internal_SetEnableShadows(mCachedPtr, value); }
		}

		/// <summary>
		/// Determines if the G-Buffer should contain per-pixel velocity information. This can be useful if you are rendering an 
		/// effect that requires this information. Note that effects such as motion blur or temporal anti-aliasing might force 
		/// the velocity buffer to be enabled regardless of this setting.
		/// </summary>
		[ShowInInspector]
		[NativeWrapper]
		public bool EnableVelocityBuffer
		{
			get { return Internal_GetEnableVelocityBuffer(mCachedPtr); }
			set { Internal_SetEnableVelocityBuffer(mCachedPtr, value); }
		}

		/// <summary>Parameters used for customizing shadow rendering.</summary>
		[ShowInInspector]
		[NotNull]
		[PassByCopy]
		[NativeWrapper]
		public ShadowSettings ShadowSettings
		{
			get { return Internal_GetShadowSettings(mCachedPtr); }
			set { Internal_SetShadowSettings(mCachedPtr, value); }
		}

		/// <summary>Determines if indirect lighting (e.g. from light probes or the sky) is rendered.</summary>
		[ShowInInspector]
		[NativeWrapper]
		public bool EnableIndirectLighting
		{
			get { return Internal_GetEnableIndirectLighting(mCachedPtr); }
			set { Internal_SetEnableIndirectLighting(mCachedPtr, value); }
		}

		/// <summary>
		/// Signals the renderer to only render overlays (like GUI), and not scene objects. Such rendering doesn&apos;t require 
		/// depth buffer or multi-sampled render targets and will not render any scene objects. This can improve performance and 
		/// memory usage for overlay-only views.
		/// </summary>
		[ShowInInspector]
		[NativeWrapper]
		public bool OverlayOnly
		{
			get { return Internal_GetOverlayOnly(mCachedPtr); }
			set { Internal_SetOverlayOnly(mCachedPtr, value); }
		}

		/// <summary>
		/// If enabled the camera will use the skybox for rendering the background. A skybox has to be present in the scene. When 
		/// disabled the camera will use the clear color for rendering the background.
		/// </summary>
		[ShowInInspector]
		[NativeWrapper]
		public bool EnableSkybox
		{
			get { return Internal_GetEnableSkybox(mCachedPtr); }
			set { Internal_SetEnableSkybox(mCachedPtr, value); }
		}

		/// <summary>
		/// The absolute base cull-distance for objects rendered through this camera in world units. Objects will use this 
		/// distance and apply their own factor to it to determine whether they should be visible.
		/// </summary>
		[ShowInInspector]
		[NativeWrapper]
		public float CullDistance
		{
			get { return Internal_GetCullDistance(mCachedPtr); }
			set { Internal_SetCullDistance(mCachedPtr, value); }
		}

		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_RenderSettings(RenderSettings managedInstance);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern DepthOfFieldSettings Internal_GetDepthOfField(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SetDepthOfField(IntPtr thisPtr, DepthOfFieldSettings value);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern ChromaticAberrationSettings Internal_GetChromaticAberration(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SetChromaticAberration(IntPtr thisPtr, ChromaticAberrationSettings value);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern bool Internal_GetEnableAutoExposure(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SetEnableAutoExposure(IntPtr thisPtr, bool value);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern AutoExposureSettings Internal_GetAutoExposure(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SetAutoExposure(IntPtr thisPtr, AutoExposureSettings value);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern bool Internal_GetEnableTonemapping(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SetEnableTonemapping(IntPtr thisPtr, bool value);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern TonemappingSettings Internal_GetTonemapping(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SetTonemapping(IntPtr thisPtr, TonemappingSettings value);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern WhiteBalanceSettings Internal_GetWhiteBalance(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SetWhiteBalance(IntPtr thisPtr, WhiteBalanceSettings value);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern ColorGradingSettings Internal_GetColorGrading(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SetColorGrading(IntPtr thisPtr, ColorGradingSettings value);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern AmbientOcclusionSettings Internal_GetAmbientOcclusion(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SetAmbientOcclusion(IntPtr thisPtr, AmbientOcclusionSettings value);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern ScreenSpaceReflectionsSettings Internal_GetScreenSpaceReflections(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SetScreenSpaceReflections(IntPtr thisPtr, ScreenSpaceReflectionsSettings value);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern BloomSettings Internal_GetBloom(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SetBloom(IntPtr thisPtr, BloomSettings value);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern ScreenSpaceLensFlareSettings Internal_GetScreenSpaceLensFlare(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SetScreenSpaceLensFlare(IntPtr thisPtr, ScreenSpaceLensFlareSettings value);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern FilmGrainSettings Internal_GetFilmGrain(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SetFilmGrain(IntPtr thisPtr, FilmGrainSettings value);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern MotionBlurSettings Internal_GetMotionBlur(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SetMotionBlur(IntPtr thisPtr, MotionBlurSettings value);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern TemporalAASettings Internal_GetTemporalAa(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SetTemporalAa(IntPtr thisPtr, TemporalAASettings value);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern bool Internal_GetEnableFxaa(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SetEnableFxaa(IntPtr thisPtr, bool value);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern float Internal_GetExposureScale(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SetExposureScale(IntPtr thisPtr, float value);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern float Internal_GetGamma(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SetGamma(IntPtr thisPtr, float value);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern bool Internal_GetEnableHdr(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SetEnableHdr(IntPtr thisPtr, bool value);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern bool Internal_GetEnableLighting(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SetEnableLighting(IntPtr thisPtr, bool value);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern bool Internal_GetEnableShadows(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SetEnableShadows(IntPtr thisPtr, bool value);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern bool Internal_GetEnableVelocityBuffer(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SetEnableVelocityBuffer(IntPtr thisPtr, bool value);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern ShadowSettings Internal_GetShadowSettings(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SetShadowSettings(IntPtr thisPtr, ShadowSettings value);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern bool Internal_GetEnableIndirectLighting(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SetEnableIndirectLighting(IntPtr thisPtr, bool value);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern bool Internal_GetOverlayOnly(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SetOverlayOnly(IntPtr thisPtr, bool value);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern bool Internal_GetEnableSkybox(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SetEnableSkybox(IntPtr thisPtr, bool value);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern float Internal_GetCullDistance(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SetCullDistance(IntPtr thisPtr, float value);
	}

	/** @} */
}
