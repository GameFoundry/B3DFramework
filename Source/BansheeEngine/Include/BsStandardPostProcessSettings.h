//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsPrerequisites.h"
#include "BsPostProcessSettings.h"
#include "BsVector3.h"

namespace bs
{
	/** @addtogroup Renderer
	 *  @{
	 */

	/** Settings that control automatic exposure (eye adaptation) post-process. */
	struct BS_EXPORT AutoExposureSettings : public IReflectable
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
		 * in scene brightness calculations. This allows you to remove outliers on the lower end of the histogram (for
		 * example a few very dark pixels in an otherwise bright image). In range [0.0f, 1.0f].
		 */
		float histogramPctLow;

		/**
		 * Percentage above which to ignore values in the eye adaptation histogram. The histogram is used for calculating
		 * the average brightness of the scene. Total luminance in the histogram will be summed up and multiplied by this
		 * value to calculate maximum luminance. Luminance values above the maximum luminance will be ignored and not used
		 * in scene brightness calculations. This allows you to remove outliers on the high end of the histogram (for 
		 * example a few very bright pixels). In range [0.0f, 1.0f].
		 */
		float histogramPctHigh;

		/**
		 * Clamps the minimum eye adaptation scale to this value. This allows you to limit eye adaptation so that exposure
		 * is never too high (for example when in a very dark room you probably do not want the exposure to be so high that
		 * everything is still visible). In range [0.0f, 10.0f].
		 */
		float minEyeAdaptation;

		/**
		 * Clamps the maximum eye adaptation scale to this value. This allows you to limit eye adaptation so that exposure
		 * is never too low (for example when looking at a very bright light source you probably don't want the exposure to
		 * be so low that the rest of the scene is all white (overexposed). In range [0.0f, 10.0f].
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

		/************************************************************************/
		/* 								RTTI		                     		*/
		/************************************************************************/
	public:
		friend class AutoExposureSettingsRTTI;
		static RTTITypeBase* getRTTIStatic();
		RTTITypeBase* getRTTI() const override;
	};

	/** Settings that control tonemap post-process. */
	struct BS_EXPORT TonemappingSettings : public IReflectable
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

		/************************************************************************/
		/* 								RTTI		                     		*/
		/************************************************************************/
	public:
		friend class TonemappingSettingsRTTI;
		static RTTITypeBase* getRTTIStatic();
		RTTITypeBase* getRTTI() const override;
	};

	/** Settings that control white balance post-process. */
	struct BS_EXPORT WhiteBalanceSettings : public IReflectable
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
		 * temperature specified by #temperature.
		 *
		 * In range [-1.0f, 1.0f].
		 */
		float tint;

		/************************************************************************/
		/* 								RTTI		                     		*/
		/************************************************************************/
	public:
		friend class WhiteBalanceSettingsRTTI;
		static RTTITypeBase* getRTTIStatic();
		RTTITypeBase* getRTTI() const override;
	};

	/** Settings that control color grading post-process. */
	struct BS_EXPORT ColorGradingSettings : public IReflectable
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

		/************************************************************************/
		/* 								RTTI		                     		*/
		/************************************************************************/
	public:
		friend class ColorGradingSettingsRTTI;
		static RTTITypeBase* getRTTIStatic();
		RTTITypeBase* getRTTI() const override;
	};

	/** Settings that control screen space ambient occlusion. */
	struct BS_EXPORT AmbientOcclusionSettings : public IReflectable
	{
		AmbientOcclusionSettings();

		/** Enables or disabled the screen space ambient occlusion effect. */
		bool enabled;

		/** 
		 * Radius (in world space, in meters) over which occluders are searched for. Smaller radius ensures better sampling
		 * precision but can miss occluders. Larger radius ensures far away occluders are considered but can yield lower
		 * quality or noise because of low sampling precision. Usually best to keep at around a few centimeters, valid range
		 * is roughly [0.005, 0.5].
		 */
		float radius;

		/**
		 * Bias used to reduce false occlusion artifacts. Higher values reduce the amount of artifacts but will cause
		 * details to be lost in areas where occlusion isn't high. Value is in millimeters. Usually best to keep at a few
		 * dozen millimeters, valid range is roughly [0, 200].
		 */
		float bias;

		/************************************************************************/
		/* 								RTTI		                     		*/
		/************************************************************************/
	public:
		friend class AmbientOcclusionSettingsRTTI;
		static RTTITypeBase* getRTTIStatic();
		RTTITypeBase* getRTTI() const override;
	};

	/** Settings that control the depth-of-field effect. */
	struct BS_EXPORT DepthOfFieldSettings : public IReflectable
	{
		DepthOfFieldSettings();

		/** Enables or disables the depth of field effect. */
		bool enabled;

		/** 
		 * Distance from the camera at which the focal plane is located in. Objects at this distance will be fully in focus.
		 */
		float focalDistance;
		
		/** 
		 * Range within which the objects remain fully in focus. This range is applied relative to the focal distance. 
		 * Only relevant if Gaussian depth of field is used as other methods don't use a constant in-focus range.
		 */
		float focalRange;

		/**
		 * Determines the size of the range within which objects transition from focused to fully unfocused, at the near 
		 * plane. Only relevant for Gaussian depth of field.
		 */
		float nearTransitionRange;

		/**
		 * Determines the size of the range within which objects transition from focused to fully unfocused, at the far 
		 * plane. Only relevant for Gaussian depth of field.
		 */
		float farTransitionRange;

		/** 
		 * Determines the amount of blur to apply to fully unfocused objects that are closer to camera than the in-focus
		 * zone. Set to zero to disable near-field blur. Only relevant for Gaussian depth of field.
		 */
		float nearBlurAmount;

		/** 
		 * Determines the amount of blur to apply to fully unfocused objects that are farther away from camera than the
		 * in-focus zone. Set to zero to disable far-field blur. Only relevant for Gaussian depth of field.
		 */
		float farBlurAmount;

		/************************************************************************/
		/* 								RTTI		                     		*/
		/************************************************************************/
	public:
		friend class DepthOfFieldSettingsRTTI;
		static RTTITypeBase* getRTTIStatic();
		RTTITypeBase* getRTTI() const override;
	};

	/** Settings that control the post-process operations. */
	struct BS_EXPORT StandardPostProcessSettings : public PostProcessSettings
	{
		StandardPostProcessSettings();

		/**
		 * Determines should automatic exposure be applied to the HDR image. When turned on the average scene brightness
		 * will be calculated and used to automatically expose the image to the optimal range. Use the parameters provided
		 * by autoExposure to customize the automatic exposure effect. You may also use exposureScale to
		 * manually adjust the automatic exposure. When automatic exposure is turned off you can use exposureScale to
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
		 * or under-exposed. Use #tonemapping to customize how tonemapping performed.
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

		/** Parameters used for customizing the depth of field effect. */
		DepthOfFieldSettings depthOfField;

		/** Parameters used for customizing screen space ambient occlusion. */
		AmbientOcclusionSettings ambientOcclusion;

		/** Enables the fast approximate anti-aliasing effect. */
		bool enableFXAA;

		/**
		 * Log2 value to scale the eye adaptation by (for example 2^0 = 1). Smaller values yield darker image, while larger
		 * yield brighter image. Allows you to customize exposure manually, applied on top of eye adaptation exposure (if
		 * enabled). In range [-8, 8].
		 */
		float exposureScale;

		/**
		 * Gamma value to adjust the image for. Larger values result in a brighter image. When tonemapping is turned
		 * on the best gamma curve for the output device is chosen automatically and this value can by used to merely tweak
		 * that curve. If tonemapping is turned off this is the exact value of the gamma curve that will be applied.
		 */
		float gamma;

		/** @copydoc PostProcessSettings::_getSyncData */
		void _getSyncData(UINT8* buffer, UINT32& size) override;

		/** @copydoc PostProcessSettings::_setSyncData */
		void _setSyncData(UINT8* buffer, UINT32 size) override;

		/************************************************************************/
		/* 								RTTI		                     		*/
		/************************************************************************/
	public:
		friend class StandardPostProcessSettingsRTTI;
		static RTTITypeBase* getRTTIStatic();
		RTTITypeBase* getRTTI() const override;
	};

	/** @} */
}
