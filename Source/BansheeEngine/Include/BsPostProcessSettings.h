//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsPrerequisites.h"
#include "BsIReflectable.h"
#include "BsVector3.h"

namespace BansheeEngine
{
	/** @addtogroup Renderer
	 *  @{
	 */

	/** Settings that control automatic exposure (eye adaptation) post-process. */
	struct AutoExposureSettings
	{
		AutoExposureSettings();

		/**
		 * Determines minimum luminance value in the eye adaptation histogram. The histogram is used for calculating the
		 * average brightness of the scene. Any luminance value below this value will not be included in the histogram and
		 * ignored in scene brightness calculations. In log2 units (-8 = 1/256). In the range [-16, 0].
		 */
		float histogramLog2Min;

		/**
		 * Determines maximum luminance value in the eye adaptation histogram. The histogram is used for calculating the
		 * average brightness of the scene. Any luminance value above this value will not be included in the histogram and
		 * ignored in scene brightness calculations. In log2 units (4 = 16). In the range [0, 16].
		 */
		float histogramLog2Max;

		/**
		 * Percentage below which to ignore values in the eye adaptation histogram. The histogram is used for calculating
		 * the average brightness of the scene. Total luminance in the histogram will be summed up and multiplied by this
		 * value to calculate minimal luminance. Luminance values below the minimal luminance will be ignored and not used
		 * in scene brightness calculations. This allows you to remove outliers on the lower end of the histogram (e.g. a
		 * few very dark pixels in an otherwise bright image). In range [0.0f, 1.0f].
		 */
		float histogramPctLow;

		/**
		 * Percentage above which to ignore values in the eye adaptation histogram. The histogram is used for calculating
		 * the average brightness of the scene. Total luminance in the histogram will be summed up and multiplied by this
		 * value to calculate maximum luminance. Luminance values above the maximum luminance will be ignored and not used
		 * in scene brightness calculations. This allows you to remove outliers on the high end of the histogram (e.g. a few
		 * very bright pixels). In range [0.0f, 1.0f].
		 */
		float histogramPctHigh;

		/**
		 * Clamps the minimum eye adaptation scale to this value. This allows you to limit eye adaptation so that exposure
		 * is never too high (e.g. when in a very dark room you probably do not want the exposure to be so high that
		 * everything is still visible). In range [0.0f, 10.0f].
		 */
		float minEyeAdaptation;

		/**
		 * Clamps the maximum eye adaptation scale to this value. This allows you to limit eye adaptation so that exposure
		 * is never too low (e.g. when looking at a very bright light source you probably don't want the exposure to be so
		 * low that the rest of the scene is all white (overexposed). In range [0.0f, 10.0f].
		 */
		float maxEyeAdaptation;

		/**
		 * Determines how quickly does the eye adaptation adjust to larger values. This affects how quickly does the
		 * automatic exposure changes when the scene brightness increases. In range [0.01f, 20.0f]. 
		 */
		float eyeAdaptationSpeedUp;

		/**
		 * Determines how quickly does the eye adaptation adjust to smaller values. This affects how quickly does the
		 * automatic exposure changes when the scene brightness decreases. In range [0.01f, 20.0f].
		 */
		float eyeAdaptationSpeedDown;
	};

	/** Settings that control tonemap post-process. */
	struct TonemappingSettings
	{
		TonemappingSettings();

		/**
		 * Controls the shoulder (upper non-linear) section of the filmic curve used for tonemapping. Mostly affects bright
		 * areas of the image and allows you to reduce over-exposure.
		 */
		float filmicCurveShoulderStrength;

		/**
		 * Controls the linear (middle) section of the filmic curve used for tonemapping. Mostly affects mid-range areas of
		 * the image.
		 */
		float filmicCurveLinearStrength;

		/**
		 * Controls the linear (middle) section of the filmic curve used for tonemapping. Mostly affects mid-range areas of
		 * the image and allows you to control how quickly does the curve climb.
		 */
		float filmicCurveLinearAngle;

		/**
		 * Controls the toe (lower non-linear) section of the filmic curve used for tonemapping. Mostly affects dark areas
		 * of the image and allows you to reduce under-exposure.
		 */
		float filmicCurveToeStrength;

		/** Controls the toe (lower non-linear) section of the filmic curve. used for tonemapping. Affects low-range. */
		float filmicCurveToeNumerator;

		/** Controls the toe (lower non-linear) section of the filmic curve used for tonemapping. Affects low-range. */
		float filmicCurveToeDenominator;

		/** Controls the white point of the filmic curve used for tonemapping. Affects the entire curve. */
		float filmicCurveLinearWhitePoint;
	};

	/** Settings that control white balance post-process. */
	struct WhiteBalanceSettings
	{
		WhiteBalanceSettings();

		/**
		 * Temperature used for white balancing, in Kelvins.
		 *
		 * Moves along the Planckian locus. In range [1500.0f, 15000.0f].
		 */
		float temperature;

		/**
		 * Additional tint to be applied during white balancing. Can be used to further tweak the white balancing effect by
		 * modifying the tint of the light. The tint is chosen on the Planckian locus isothermal, depending on the light
		 * temperature specified by ::temperature.
		 *
		 * In range [-1.0f, 1.0f].
		 */
		float tint;
	};

	/** Settings that control color grading post-process. */
	struct ColorGradingSettings
	{
		ColorGradingSettings();

		/**
		 * Saturation to be applied during color grading. Larger values increase vibrancy of the image.
		 * In range [0.0f, 2.0f].
		 */
		Vector3 saturation;

		/**
		 * Contrast to be applied during color grading. Larger values increase difference between light and dark areas of
		 * the image. In range [0.0f, 2.0f]. 
		 */
		Vector3 contrast;

		/**
		 * Gain to be applied during color grading. Simply increases all color values by an equal scale.
		 * In range [0.0f, 2.0f].
		 */
		Vector3 gain;

		/**
		 * Gain to be applied during color grading. Simply offsets all color values by an equal amount.
		 * In range [-1.0f, 1.0f].
		 */
		Vector3 offset;
	};

	/** Settings that control the post-process operations. */
	struct BS_EXPORT PostProcessSettings : IReflectable
	{
		PostProcessSettings();

		/**
		 * Determines should automatic exposure be applied to the HDR image. When turned on the average scene brightness
		 * will be calculated and used to automatically expose the image to the optimal range. Use the parameters provided
		 * by ::autoExposure to customize the automatic exposure effect. You may also use ::exposureScale to
		 * manually adjust the automatic exposure. When automatic exposure is turned off you can use ::exposureScale to
		 * manually set the exposure.
		 */
		bool enableAutoExposure;

		/**
		 * Parameters used for customizing automatic scene exposure.
		 *
		 * @see	enableAutoExposure
		 */
		AutoExposureSettings autoExposure;

		/**
		 * Determines should the image be tonemapped. Tonemapping converts an HDR image into LDR image by applying
		 * a filmic curve to the image, simulating the effect of film cameras. Filmic curve improves image quality by
		 * tapering off lows and highs, preventing under- and over-exposure. This is useful if an image contains both
		 * very dark and very bright areas, in which case the global exposure parameter would leave some areas either over-
		 * or under-exposed. Use ::tonemapping to customize how tonemapping performed.
		 *
		 * If this is disabled, then color grading and white balancing will not be enabled either. Only relevant for HDR
		 * images.
		 */
		bool enableTonemapping;

		/**
		 * Parameters used for customizing tonemapping.
		 *
		 * @see	enableTonemapping
		 */
		TonemappingSettings tonemapping;

		/**
		 * Parameters used for customizing white balancing. White balancing converts a scene illuminated by a light of the
		 * specified temperature into a scene illuminated by a standard D65 illuminant (average midday light) in order to
		 * simulate the effects of chromatic adaptation of the human visual system.
		 */
		WhiteBalanceSettings whiteBalance;

		/** Parameters used for customizing color grading. */
		ColorGradingSettings colorGrading;

		/**
		 * Log2 value to scale the eye adaptation by (e.g. 2^0 = 1). Smaller values yield darker image, while larger yield
		 * brighter image. Allows you to customize exposure manually, applied on top of eye adaptation exposure (if
		 * enabled). In range [-8, 8].
		 */
		float exposureScale;

		/**
		 * Gamma value to adjust the image for. Larger values result in a brighter image. When tonemapping is turned
		 * on the best gamma curve for the output device is chosen automatically and this value can by used to merely tweak
		 * that curve. If tonemapping is turned off this is the exact value of the gamma curve that will be applied.
		 */
		float gamma;

		/** Creates a new post process settings object. */
		static SPtr<PostProcessSettings> create();

		/************************************************************************/
		/* 								RTTI		                     		*/
		/************************************************************************/

	public:
		friend class PostProcessSettingsRTTI;
		static RTTITypeBase* getRTTIStatic();
		RTTITypeBase* getRTTI() const override;
	};

	/** @} */
}
