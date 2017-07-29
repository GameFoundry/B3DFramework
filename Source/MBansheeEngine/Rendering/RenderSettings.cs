//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
using System;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;

namespace BansheeEngine
{
    /// <summary>
    /// Settings that control automatic exposure (eye adaptation) post-process.
    /// </summary>
    [SerializeObject]
    public class AutoExposureSettings
    {
        /// <summary>
        /// Determines minimum luminance value in the eye adaptation histogram. The histogram is used for calculating the
        /// average brightness of the scene.Any luminance value below this value will not be included in the histogram and
        /// ignored in scene brightness calculations. In log2 units (-8 = 1/256). In the range [-16, 0].
        /// </summary>
        public float HistogramLog2Min;

        /// <summary>
        /// Determines maximum luminance value in the eye adaptation histogram. The histogram is used for calculating the
        /// average brightness of the scene.Any luminance value above this value will not be included in the histogram and
        /// ignored in scene brightness calculations. In log2 units (4 = 16). In the range [0, 16].
        /// </summary>
        public float HistogramLog2Max;

        /// <summary>
        /// Percentage below which to ignore values in the eye adaptation histogram. The histogram is used for calculating
        /// the average brightness of the scene.Total luminance in the histogram will be summed up and multiplied by this
        /// value to calculate minimal luminance. Luminance values below the minimal luminance will be ignored and not used
        /// in scene brightness calculations.This allows you to remove outliers on the lower end of the histogram (for
        /// example a few very dark pixels in an otherwise bright image). In range [0.0f, 1.0f].
        /// </summary>
        public float HistogramPctLow;

        /// <summary>
        /// Percentage above which to ignore values in the eye adaptation histogram. The histogram is used for calculating
        /// the average brightness of the scene.Total luminance in the histogram will be summed up and multiplied by this
        /// value to calculate maximum luminance. Luminance values above the maximum luminance will be ignored and not used
        /// in scene brightness calculations.This allows you to remove outliers on the high end of the histogram (for 
        /// example a few very bright pixels). In range [0.0f, 1.0f].
        /// </summary>
        public float HistogramPctHigh;

        /// <summary>
        /// Clamps the minimum eye adaptation scale to this value. This allows you to limit eye adaptation so that exposure
        /// is never too high (for example when in a very dark room you probably do not want the exposure to be so high that
        /// everything is still visible). In range [0.0f, 10.0f].
        /// </summary>
        public float MinEyeAdaptation;

        /// <summary>
        /// Clamps the maximum eye adaptation scale to this value. This allows you to limit eye adaptation so that exposure
        /// is never too low (for example when looking at a very bright light source you probably don't want the exposure to
        /// be so low that the rest of the scene is all white (overexposed). In range [0.0f, 10.0f].
        /// </summary>
        public float MaxEyeAdaptation;

        /// <summary>
        /// Determines how quickly does the eye adaptation adjust to larger values. This affects how quickly does the
        /// automatic exposure changes when the scene brightness increases. In range [0.01f, 20.0f].
        /// </summary>
        public float EyeAdaptationSpeedUp;

        /// <summary>
        /// Determines how quickly does the eye adaptation adjust to smaller values. This affects how quickly does the
        /// automatic exposure changes when the scene brightness decreases. In range [0.01f, 20.0f].
        /// </summary>
        public float EyeAdaptationSpeedDown;
    };

    /// <summary>
    /// Settings that control tonemap post-process.
    /// </summary>
    [SerializeObject]
    public class TonemappingSettings
    {
        /// <summary>
        /// Controls the shoulder (upper non-linear) section of the filmic curve used for tonemapping. Mostly affects bright
        /// areas of the image and allows you to reduce over-exposure.
        /// </summary>
        public float FilmicCurveShoulderStrength;

        /// <summary>
        /// Controls the linear (middle) section of the filmic curve used for tonemapping. Mostly affects mid-range areas of
        /// the image.
        /// </summary>
        public float FilmicCurveLinearStrength;

        /// <summary>
        /// Controls the linear (middle) section of the filmic curve used for tonemapping. Mostly affects mid-range areas of
        /// the image and allows you to control how quickly does the curve climb.
        /// </summary>
        public float FilmicCurveLinearAngle;

        /// <summary>
        /// Controls the toe (lower non-linear) section of the filmic curve used for tonemapping. Mostly affects dark areas
        /// of the image and allows you to reduce under-exposure.
        /// </summary>
        public float FilmicCurveToeStrength;

        /// <summary>
        /// Controls the toe (lower non-linear) section of the filmic curve. used for tonemapping. Affects low-range.
        /// </summary>
        public float FilmicCurveToeNumerator;

        /// <summary>
        /// Controls the toe (lower non-linear) section of the filmic curve used for tonemapping. Affects low-range.
        /// </summary>
        public float FilmicCurveToeDenominator;

        /// <summary>
        /// Controls the white point of the filmic curve used for tonemapping. Affects the entire curve.
        /// </summary>
        public float FilmicCurveLinearWhitePoint;
    };

    /// <summary>
    /// Settings that control white balance post-process.
    /// </summary>
    [SerializeObject]
    public class WhiteBalanceSettings
    {
        /// <summary>
        /// Temperature used for white balancing, in Kelvins.
        /// 
        /// Moves along the Planckian locus. In range [1500.0f, 15000.0f].
        /// </summary>
        public float Temperature;

        /// <summary>
        /// Additional tint to be applied during white balancing. Can be used to further tweak the white balancing effect by
        /// modifying the tint of the light.The tint is chosen on the Planckian locus isothermal, depending on the light
        /// temperature specified by <see cref="Temperature"/>.
        /// 
        /// In range [-1.0f, 1.0f].
        /// </summary>
        public float Tint;
    };

    /// <summary>
    /// Settings that control color grading post-process.
    /// </summary>
    [SerializeObject]
    public class ColorGradingSettings
    {
        /// <summary>
        /// Saturation to be applied during color grading. Larger values increase vibrancy of the image. 
        /// In range [0.0f, 2.0f].
        /// </summary>
        public Vector3 Saturation;

        /// <summary>
        /// Contrast to be applied during color grading. Larger values increase difference between light and dark areas of
        /// the image. In range [0.0f, 2.0f]. 
        /// </summary>
        public Vector3 Contrast;

        /// <summary>
        /// Gain to be applied during color grading. Simply increases all color values by an equal scale. 
        /// In range [0.0f, 2.0f].
        /// </summary>
        public Vector3 Gain;

        /// <summary>
        /// Gain to be applied during color grading. Simply offsets all color values by an equal amount.
        /// In range [-1.0f, 1.0f].
        /// </summary>
        public Vector3 Offset;
    };

    /// <summary>
    /// Settings that control rendering operations for a single camera.
    /// </summary>
    [SerializeObject]
    public class RenderSettings
    {
        /// <summary>
        /// Determines should automatic exposure be applied to the HDR image. When turned on the average scene brightness
        /// will be calculated and used to automatically expose the image to the optimal range. Use the parameters provided
        /// by <see cref="AutoExposure"/> to customize the automatic exposure effect.You may also use 
        /// <see cref="ExposureScale"/> to manually adjust the automatic exposure. When automatic exposure is turned off you
        /// can use <see cref="ExposureScale"/> to manually set the exposure.
        /// </summary>
        public bool EnableAutoExposure;

        /// <summary>
        /// Parameters used for customizing automatic scene exposure. <see cref="EnableAutoExposure"/>
        /// </summary>
        public AutoExposureSettings AutoExposure;

        /// <summary>
        /// Determines should the image be tonemapped. Tonemapping converts an HDR image into LDR image by applying
        /// a filmic curve to the image, simulating the effect of film cameras.Filmic curve improves image quality by
        /// tapering off lows and highs, preventing under- and over-exposure.This is useful if an image contains both
        /// very dark and very bright areas, in which case the global exposure parameter would leave some areas either over-
        /// or under-exposed. Use <see cref="Tonemapping"/> to customize how tonemapping performed.
        /// 
        /// If this is disabled, then color grading and white balancing will not be enabled either.Only relevant for HDR
        /// images.
        /// </summary>
        public bool EnableTonemapping;

        /// <summary>
        /// Parameters used for customizing tonemapping. <see cref="EnableTonemapping"/>
        /// </summary>
        public TonemappingSettings Tonemapping;

        /// <summary>
        /// Parameters used for customizing white balancing. White balancing converts a scene illuminated by a light of the
        /// specified temperature into a scene illuminated by a standard D65 illuminant (average midday light) in order to
        /// simulate the effects of chromatic adaptation of the human visual system.
        /// </summary>
        public WhiteBalanceSettings WhiteBalance;

        /// <summary>
        /// Parameters used for customizing color grading.
        /// </summary>
        public ColorGradingSettings ColorGrading;

        /// <summary>
        /// Log2 value to scale the eye adaptation by (for example 2^0 = 1). Smaller values yield darker image, while larger
        /// yield brighter image.Allows you to customize exposure manually, applied on top of eye adaptation exposure (if
        /// enabled). In range [-8, 8].
        /// </summary>
        public float ExposureScale;

        /// <summary>
        /// Gamma value to adjust the image for. Larger values result in a brighter image. When tonemapping is turned
        /// on the best gamma curve for the output device is chosen automatically and this value can by used to merely tweak
        /// that curve. If tonemapping is turned off this is the exact value of the gamma curve that will be applied.
        /// </summary>
        public float Gamma;

        /// <summary>
        /// High dynamic range allows light intensity to be more correctly recorded when rendering by allowing for a larger
        /// range of values. The stored light is then converted into visible color range using exposure and a tone mapping 
        /// operator.
        /// </summary>
        public bool EnableHDR;

        /// <summary>
        /// Determines if scene objects will be lit by lights. If disabled everything will be rendered using their albedo
        /// texture with no lighting applied.
        /// </summary>
        public bool EnableLighting;

        /// <summary>
        /// Determines if shadows cast by lights should be rendered. Only relevant if lighting is turned on.
        /// </summary>
        public bool EnableShadows;

        /// <summary>
        /// Signals the renderer to only render overlays (like GUI), and not scene objects. Such rendering doesn't require
        /// depth buffer or multi-sampled render targets and will not render any scene objects. This can improve performance
        /// and memory usage for overlay-only views. 
        /// </summary>
        public bool OverlayOnly;

        /// <summary>
        /// Creates a new instance of render settings with the optimal default values.
        /// </summary>
        /// <returns>New instance of render settings.</returns>
        public static RenderSettings CreateDefault()
        {
            return Internal_CreateDefault();
        }

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern RenderSettings Internal_CreateDefault();
    }
}
