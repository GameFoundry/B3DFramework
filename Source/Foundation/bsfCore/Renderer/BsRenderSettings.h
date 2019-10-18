//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsCorePrerequisites.h"
#include "Reflection/BsIReflectable.h"
#include "Math/BsVector3.h"
#include "Image/BsColor.h"
#include "Image/BsTexture.h"

namespace bs
{
	/** @addtogroup Renderer
	 *  @{
	 */

	/** Settings that control automatic exposure (eye adaptation) post-process. */
	struct BS_CORE_EXPORT BS_SCRIPT_EXPORT(m:Rendering) AutoExposureSettings : public IReflectable
	{
		BS_SCRIPT_EXPORT()
		AutoExposureSettings() = default;

		/**
		 * Determines minimum luminance value in the eye adaptation histogram. The histogram is used for calculating the
		 * average brightness of the scene. Any luminance value below this value will not be included in the histogram and
		 * ignored in scene brightness calculations. In log2 units (-8 = 1/256). In the range [-16, 0].
		 */
		BS_SCRIPT_EXPORT(range:[-16,0],slider)
		float histogramLog2Min = -8.0f;

		/**
		 * Determines maximum luminance value in the eye adaptation histogram. The histogram is used for calculating the
		 * average brightness of the scene. Any luminance value above this value will not be included in the histogram and
		 * ignored in scene brightness calculations. In log2 units (4 = 16). In the range [0, 16].
		 */
		BS_SCRIPT_EXPORT(range:[0,16],slider)
		float histogramLog2Max = 4.0f;

		/**
		 * Percentage below which to ignore values in the eye adaptation histogram. The histogram is used for calculating
		 * the average brightness of the scene. Total luminance in the histogram will be summed up and multiplied by this
		 * value to calculate minimal luminance. Luminance values below the minimal luminance will be ignored and not used
		 * in scene brightness calculations. This allows you to remove outliers on the lower end of the histogram (for
		 * example a few very dark pixels in an otherwise bright image). In range [0.0f, 1.0f].
		 */
		BS_SCRIPT_EXPORT(range:[0,1],slider)
		float histogramPctLow = 0.8f;

		/**
		 * Percentage above which to ignore values in the eye adaptation histogram. The histogram is used for calculating
		 * the average brightness of the scene. Total luminance in the histogram will be summed up and multiplied by this
		 * value to calculate maximum luminance. Luminance values above the maximum luminance will be ignored and not used
		 * in scene brightness calculations. This allows you to remove outliers on the high end of the histogram (for
		 * example a few very bright pixels). In range [0.0f, 1.0f].
		 */
		BS_SCRIPT_EXPORT(range:[0,1],slider)
		float histogramPctHigh = 0.985f;

		/**
		 * Clamps the minimum eye adaptation scale to this value. This allows you to limit eye adaptation so that exposure
		 * is never too high (for example when in a very dark room you probably do not want the exposure to be so high that
		 * everything is still visible). In range [0.0f, 10.0f].
		 */
		BS_SCRIPT_EXPORT(range:[0,10],slider)
		float minEyeAdaptation = 0.003f;

		/**
		 * Clamps the maximum eye adaptation scale to this value. This allows you to limit eye adaptation so that exposure
		 * is never too low (for example when looking at a very bright light source you probably don't want the exposure to
		 * be so low that the rest of the scene is all white (overexposed). In range [0.0f, 10.0f].
		 */
		BS_SCRIPT_EXPORT(range:[0,10],slider)
		float maxEyeAdaptation = 2.0f;

		/**
		 * Determines how quickly does the eye adaptation adjust to larger values. This affects how quickly does the
		 * automatic exposure changes when the scene brightness increases. In range [0.01f, 20.0f].
		 */
		BS_SCRIPT_EXPORT(range:[0.01,20],slider)
		float eyeAdaptationSpeedUp = 3.0f;

		/**
		 * Determines how quickly does the eye adaptation adjust to smaller values. This affects how quickly does the
		 * automatic exposure changes when the scene brightness decreases. In range [0.01f, 20.0f].
		 */
		BS_SCRIPT_EXPORT(range:[0.01,20],slider)
		float eyeAdaptationSpeedDown = 3.0f;

		/************************************************************************/
		/* 								RTTI		                     		*/
		/************************************************************************/

		/** Enumerates all the fields in the type and executes the specified processor action for each field. */
		template<class P>
		void rttiEnumFields(P processor);
	public:
		friend class AutoExposureSettingsRTTI;
		static RTTITypeBase* getRTTIStatic();
		RTTITypeBase* getRTTI() const override;
	};

	/** Settings that control tonemap post-process. */
	struct BS_CORE_EXPORT BS_SCRIPT_EXPORT(m:Rendering) TonemappingSettings : public IReflectable
	{
		BS_SCRIPT_EXPORT()
		TonemappingSettings() = default;

		/**
		 * Controls the shoulder (upper non-linear) section of the filmic curve used for tonemapping. Mostly affects bright
		 * areas of the image and allows you to reduce over-exposure.
		 */
		BS_SCRIPT_EXPORT()
		float filmicCurveShoulderStrength = 0.15f;

		/**
		 * Controls the linear (middle) section of the filmic curve used for tonemapping. Mostly affects mid-range areas of
		 * the image.
		 */
		BS_SCRIPT_EXPORT()
		float filmicCurveLinearStrength = 0.5f;

		/**
		 * Controls the linear (middle) section of the filmic curve used for tonemapping. Mostly affects mid-range areas of
		 * the image and allows you to control how quickly does the curve climb.
		 */
		BS_SCRIPT_EXPORT()
		float filmicCurveLinearAngle = 0.1f;

		/**
		 * Controls the toe (lower non-linear) section of the filmic curve used for tonemapping. Mostly affects dark areas
		 * of the image and allows you to reduce under-exposure.
		 */
		BS_SCRIPT_EXPORT()
		float filmicCurveToeStrength = 0.2f;

		/** Controls the toe (lower non-linear) section of the filmic curve. used for tonemapping. Affects low-range. */
		BS_SCRIPT_EXPORT()
		float filmicCurveToeNumerator = 0.02f;

		/** Controls the toe (lower non-linear) section of the filmic curve used for tonemapping. Affects low-range. */
		BS_SCRIPT_EXPORT()
		float filmicCurveToeDenominator = 0.3f;

		/** Controls the white point of the filmic curve used for tonemapping. Affects the entire curve. */
		BS_SCRIPT_EXPORT()
		float filmicCurveLinearWhitePoint = 11.2f;

		/************************************************************************/
		/* 								RTTI		                     		*/
		/************************************************************************/

		/** Enumerates all the fields in the type and executes the specified processor action for each field. */
		template<class P>
		void rttiEnumFields(P processor);
	public:
		friend class TonemappingSettingsRTTI;
		static RTTITypeBase* getRTTIStatic();
		RTTITypeBase* getRTTI() const override;
	};

	/** Settings that control white balance post-process. */
	struct BS_CORE_EXPORT BS_SCRIPT_EXPORT(m:Rendering) WhiteBalanceSettings : public IReflectable
	{
		BS_SCRIPT_EXPORT()
		WhiteBalanceSettings() = default;

		/**
		 * Temperature used for white balancing, in Kelvins.
		 *
		 * Moves along the Planckian locus. In range [1500.0f, 15000.0f].
		 */
		BS_SCRIPT_EXPORT(range:[1500,15000],slider)
		float temperature = 6500.0f;

		/**
		 * Additional tint to be applied during white balancing. Can be used to further tweak the white balancing effect by
		 * modifying the tint of the light. The tint is chosen on the Planckian locus isothermal, depending on the light
		 * temperature specified by #temperature.
		 *
		 * In range [-1.0f, 1.0f].
		 */
		BS_SCRIPT_EXPORT(range:[-1,1],slider)
		float tint = 0.0f;

		/************************************************************************/
		/* 								RTTI		                     		*/
		/************************************************************************/

		/** Enumerates all the fields in the type and executes the specified processor action for each field. */
		template<class P>
		void rttiEnumFields(P processor);
	public:
		friend class WhiteBalanceSettingsRTTI;
		static RTTITypeBase* getRTTIStatic();
		RTTITypeBase* getRTTI() const override;
	};

	/** Settings that control color grading post-process. */
	struct BS_CORE_EXPORT BS_SCRIPT_EXPORT() ColorGradingSettings : public IReflectable
	{
		ColorGradingSettings() = default;

		/**
		 * Saturation to be applied during color grading. Larger values increase vibrancy of the image.
		 * In range [0.0f, 2.0f].
		 */
		BS_SCRIPT_EXPORT()
		Vector3 saturation = Vector3::ONE;

		/**
		 * Contrast to be applied during color grading. Larger values increase difference between light and dark areas of
		 * the image. In range [0.0f, 2.0f].
		 */
		BS_SCRIPT_EXPORT()
		Vector3 contrast = Vector3::ONE;

		/**
		 * Gain to be applied during color grading. Simply increases all color values by an equal scale.
		 * In range [0.0f, 2.0f].
		 */
		BS_SCRIPT_EXPORT()
		Vector3 gain = Vector3::ONE;

		/**
		 * Gain to be applied during color grading. Simply offsets all color values by an equal amount.
		 * In range [-1.0f, 1.0f].
		 */
		BS_SCRIPT_EXPORT()
		Vector3 offset = Vector3::ZERO;

		/************************************************************************/
		/* 								RTTI		                     		*/
		/************************************************************************/

		/** Enumerates all the fields in the type and executes the specified processor action for each field. */
		template<class P>
		void rttiEnumFields(P processor);
	public:
		friend class ColorGradingSettingsRTTI;
		static RTTITypeBase* getRTTIStatic();
		RTTITypeBase* getRTTI() const override;
	};

	/** Settings that control screen space ambient occlusion. */
	struct BS_CORE_EXPORT BS_SCRIPT_EXPORT(m:Rendering) AmbientOcclusionSettings : public IReflectable
	{
		BS_SCRIPT_EXPORT()
		AmbientOcclusionSettings() = default;

		/** Enables or disables the screen space ambient occlusion effect. */
		BS_SCRIPT_EXPORT()
		bool enabled = true;

		/**
		 * Radius (in world space, in meters) over which occluders are searched for. Smaller radius ensures better sampling
		 * precision but can miss occluders. Larger radius ensures far away occluders are considered but can yield lower
		 * quality or noise because of low sampling precision. Usually best to keep at around a meter, valid range
		 * is roughly [0.05, 5.0].
		 */
		BS_SCRIPT_EXPORT(range:[0.05,5],slider)
		float radius = 1.5f;

		/**
		 * Bias used to reduce false occlusion artifacts. Higher values reduce the amount of artifacts but will cause
		 * details to be lost in areas where occlusion isn't high. Value is in millimeters. Usually best to keep at a few
		 * dozen millimeters, valid range is roughly [0, 200].
		 */
		BS_SCRIPT_EXPORT(range:[0,200],slider)
		float bias = 1.0f;

		/**
		 * Distance (in view space, in meters) after which AO starts fading out. The fade process will happen over the
		 * range as specified by @p fadeRange.
		 */
		BS_SCRIPT_EXPORT()
		float fadeDistance = 500.0f;

		/**
		 * Range (in view space, in meters) in which AO fades out from 100% to 0%. AO starts fading out after the distance
		 * specified in @p fadeDistance.
		 */
		BS_SCRIPT_EXPORT()
		float fadeRange = 50.0f;

		/**
		 * Linearly scales the intensity of the AO effect. Values less than 1 make the AO effect less pronounced, and vice
		 * versa. Valid range is roughly [0.2, 2].
		 */
		BS_SCRIPT_EXPORT(range:[0.2,2],slider)
		float intensity = 1.0f;

		/**
		 * Controls how quickly does the AO darkening effect increase with higher occlusion percent. This is a non-linear
		 * control and will cause the darkening to ramp up exponentially. Valid range is roughly [1, 4], where 1 means no
		 * extra darkening will occur.
		 */
		BS_SCRIPT_EXPORT(range:[1,4],slider)
		float power = 4.0f;

		/**
		 * Quality level of generated ambient occlusion. In range [0, 4]. Higher levels yield higher quality AO at the cost
		 * of performance.
		 */
		BS_SCRIPT_EXPORT(range:[0,4],slider)
		UINT32 quality = 3;

		/************************************************************************/
		/* 								RTTI		                     		*/
		/************************************************************************/

		/** Enumerates all the fields in the type and executes the specified processor action for each field. */
		template<class P>
		void rttiEnumFields(P processor);
	public:
		friend class AmbientOcclusionSettingsRTTI;
		static RTTITypeBase* getRTTIStatic();
		RTTITypeBase* getRTTI() const override;
	};

	/** Types of available depth of field effects. */
	enum class BS_SCRIPT_EXPORT(m:Rendering) DepthOfFieldType
	{
		/** 
		 * Fastest depth of field variant, uses gaussian blur to approximate depth of field on near and far objects, but
		 * doesn't use any physically based methods for determining blur amount.
		 */
		Gaussian,
		/**
		 * Very expensive depth of field variant that allows you to use a bokeh texture, controlling the shape of the blur
		 * (usually intended to mimic real world camera aperature shapes). Blur is varied according to actual object
		 * distance and the effect is more physically based than gaussian blur (but not completely). Very expensive in
		 * terms of performance.
		 */
		Bokeh
	};

	/** Base class for both sim and core thread variants of DepthOfFieldSettings. */
	struct BS_CORE_EXPORT DepthOfFieldSettingsBase
	{
		DepthOfFieldSettingsBase() = default;

		/** Enables or disables the depth of field effect. */
		BS_SCRIPT_EXPORT()
		bool enabled = false;

		/** Type of depth of field effect to use. */
		BS_SCRIPT_EXPORT()
		DepthOfFieldType type = DepthOfFieldType::Gaussian;

		/** 
		 * Distance from the camera at which the focal plane is located in. Objects at this distance will be fully in focus.
		 * In world units (meters).
		 */
		BS_SCRIPT_EXPORT()
		float focalDistance = 0.75f;
		
		/** 
		 * Range within which the objects remain fully in focus. This range is applied relative to the focal distance.
		 * This parameter should usually be non-zero when using the Gaussian depth of field effect. When using other types
		 * of depth-of-field you can set this to zero for a more physically-based effect, or keep it non-zero for more
		 * artistic control. In world units (meters).
		 */
		BS_SCRIPT_EXPORT()
		float focalRange = 0.75f;

		/**
		 * Determines the size of the range within which objects transition from focused to fully unfocused, at the near 
		 * plane. Only relevant for Gaussian and Bokeh depth of field. In world units (meters).
		 */
		BS_SCRIPT_EXPORT()
		float nearTransitionRange = 0.25f;

		/**
		 * Determines the size of the range within which objects transition from focused to fully unfocused, at the far 
		 * plane. Only relevant for Gaussian and Bokeh depth of field. In world units (meters).
		 */
		BS_SCRIPT_EXPORT()
		float farTransitionRange = 0.25f;

		/**
		 * Determines the amount of blur to apply to fully unfocused objects that are closer to camera than the in-focus
		 * zone. Set to zero to disable near-field blur. Only relevant for Gaussian depth of field.
		 */
		BS_SCRIPT_EXPORT()
		float nearBlurAmount = 0.02f;

		/**
		 * Determines the amount of blur to apply to fully unfocused objects that are farther away from camera than the
		 * in-focus zone. Set to zero to disable far-field blur. Only relevant for Gaussian depth of field.
		 */
		BS_SCRIPT_EXPORT()
		float farBlurAmount = 0.02f;

		/** 
		 * Determines the maximum size of the blur kernel, in percent of view size. Larger values cost more performance. 
		 * Only relevant when using Bokeh depth of field.
		 */
		BS_SCRIPT_EXPORT(range:[0,1])
		float maxBokehSize = 0.025f;

		/** 
		 * Determines the maximum color difference between surrounding pixels allowed (as a sum of all channels) before
		 * higher fidelity sampling is triggered. Increasing this value can improve performance as less higher fidelity
		 * samples will be required, but may decrease quality of the effect. Only relevant when using Bokeh depth of
		 * field.
		 */
		BS_SCRIPT_EXPORT(range:[0,10.0])
		float adaptiveColorThreshold = 1.0f;

		/** 
		 * Determines the minimum circle of confusion size before higher fidelity sampling is triggered. Small values
		 * trigger high fidelity sampling because they can otherwise produce aliasing, and they are small enough so they
		 * don't cost much. Increasing this value can improve performance as less higher fidelity samples will be required, 
		 * but may decrease quality of the effect. Only relevant when using Bokeh depth of field.
		 */
		BS_SCRIPT_EXPORT(range:[0,1.0])
		float adaptiveRadiusThreshold = 0.1f;

		/** Camera aperture size in mm. Only relevant when using Bokeh depth of field. */
		BS_SCRIPT_EXPORT(range:[1,200])
		float apertureSize = 50.0f;

		/** Focal length of the camera's lens (e.g. 75mm). Only relevant when using Bokeh depth of field. */
		BS_SCRIPT_EXPORT(range:[1,500])
		float focalLength = 50.0f;

		/** 
		 * Camera sensor width and height, in mm. Used for controlling the size of the circle of confusion. Only relevant 
		 * when using Bokeh depth of field. 
		 */
		BS_SCRIPT_EXPORT()
		Vector2 sensorSize = Vector2(22.2f, 14.8f);

		/**
		 * Set to true if Bokeh flare should use soft depth testing to ensure they don't render on top of foreground
		 * geometry. This can help reduce background bleeding into the foreground, which can be especially noticeable
		 * if the background is much brighter than the foreground. Use @p occlusionDepthRange to tweak the effect.
		 *
		 */
		BS_SCRIPT_EXPORT()
		bool bokehOcclusion = true;

		/**
		 * Range in world units over which range to fade out Bokeh flare influence. Influence of the flare will be
		 * faded out as the depth difference between the flare's origin pixel and the destination pixel grows larger.
		 * Only relevant if @p bokehOcclusion is turned on.
		 */
		BS_SCRIPT_EXPORT()
		float occlusionDepthRange = 1.0f;
		
	protected:
		~DepthOfFieldSettingsBase() = default;
	};

	/** Template version of DepthOfFieldSettings that can be specialized for either core or simulation thread. */
	template<bool Core>
	struct BS_CORE_EXPORT TDepthOfFieldSettings : DepthOfFieldSettingsBase
	{
		using TextureType = CoreVariantHandleType<Texture, Core>;

		/** Texture to use for the bokeh shape. Only relevant when using Bokeh depth of field. */
		BS_SCRIPT_EXPORT()
		TextureType bokehShape;

		/************************************************************************/
		/* 								RTTI		                     		*/
		/************************************************************************/

		/** Enumerates all the fields in the type and executes the specified processor action for each field. */
		template<class P>
		void rttiEnumFields(P processor);

	protected:
		~TDepthOfFieldSettings() = default;
	};

	/** Settings that control the depth-of-field effect. */
	struct BS_CORE_EXPORT BS_SCRIPT_EXPORT() DepthOfFieldSettings : TDepthOfFieldSettings<false>, IReflectable
	{
		BS_SCRIPT_EXPORT()
		DepthOfFieldSettings() = default;

		/************************************************************************/
		/* 								RTTI		                     		*/
		/************************************************************************/
	public:
		friend class DepthOfFieldSettingsRTTI;
		static RTTITypeBase* getRTTIStatic();
		RTTITypeBase* getRTTI() const override;
	};

	namespace ct
	{
		/** Core thread variant of DepthOfFieldSettings. */
		struct BS_CORE_EXPORT BS_SCRIPT_EXPORT() DepthOfFieldSettings : TDepthOfFieldSettings<true>
		{
			DepthOfFieldSettings() = default;
		};
	}

	/** Determines which parts of the scene will trigger motion blur. */
	enum class BS_SCRIPT_EXPORT(m:Rendering) MotionBlurDomain
	{
		/** Camera movement and rotation will result in full-screen motion blur. */
		CameraOnly,

		/**
		 * Object movement and rotation will result in blurring of the moving object. Can be significantly more
		 * expensive than just using camera blur due to the requirement to use a velocity buffer (unless some
		 * other effect also requires it, in which case it will be re-used).
		 */
		ObjectOnly,

		/** Both the camera movement and object movement will result in motion blur. */
		CameraAndObject
	};

	/** Type of filter to use when filtering samples contributing to a blurred pixel. */
	enum class BS_SCRIPT_EXPORT(m:Rendering) MotionBlurFilter
	{
		/** Samples will be simply averaged together to create the blurred pixel. */
		Simple,

		/**
		 * A more advanced reconstruction filter will be used. This filter provides better blur quality at a
		 * performance cost. In particular the filter will improve blurring at object boundaries, allowing blur
		 * to extend beyond the object silhouette. It will also try to estimate blurred background and provide
		 * better weighting between background, center and foreground samples.
		 */
		Reconstruction
	};

	/** Determines the number of samples to take during motion blur filtering. */
	enum class BS_SCRIPT_EXPORT() MotionBlurQuality
	{
		/** 4 samples per pixel. */
		VeryLow,
		/** 6 samples per pixel. */
		Low,
		/** 8 samples per pixel. */
		Medium,
		/** 12 samples per pixel. */
		High,
		/** 16 samples per pixel. */
		Ultra
	};

	/** Settings that control the motion blur effect. */
	struct BS_CORE_EXPORT BS_SCRIPT_EXPORT(m:Rendering) MotionBlurSettings : public IReflectable
	{
		BS_SCRIPT_EXPORT()
		MotionBlurSettings() = default;

		/** Enables or disables the motion blur effect. */
		BS_SCRIPT_EXPORT()
		bool enabled = false;

		/** Determines which parts of the scene will trigger motion blur. */
		BS_SCRIPT_EXPORT()
		MotionBlurDomain domain = MotionBlurDomain::CameraAndObject;

		/** Type of filter to use when filtering samples contributing to a blurred pixel. */
		BS_SCRIPT_EXPORT()
		MotionBlurFilter filter = MotionBlurFilter::Reconstruction;

		/**
		 * Determines the number of samples to take during motion blur filtering. Increasing this value will
		 * yield higher quality blur at the cost of the performance.
		 */
		BS_SCRIPT_EXPORT()
		MotionBlurQuality quality = MotionBlurQuality::Medium;

		/**
		 * Determines the maximum radius over which the blur samples are allowed to be taken, in percent of the
		 * screen width (e.g. with 1% radius, on 1920x1028 resolution the maximum radius in pixels will be
		 * 1920 * 0.01 = 20px). This clamps the maximum velocity that can affect the blur, as higher velocities
		 * require higher radius. Very high values can adversely affect performance as cache accesses become more random. 
		 */
		BS_SCRIPT_EXPORT()
		float maximumRadius = 0.01f; // TODO - Set this in pixels, but always up/downsample to a specific resolution? Would improve cache performance.

		/************************************************************************/
		/* 								RTTI		                     		*/
		/************************************************************************/

		/** Enumerates all the fields in the type and executes the specified processor action for each field. */
		template<class P>
		void rttiEnumFields(P processor);
	public:
		friend class MotionBlurSettingsRTTI;
		static RTTITypeBase* getRTTIStatic();
		RTTITypeBase* getRTTI() const override;
	};

	/** Settings that control temporal anti-aliasing. */
	struct BS_CORE_EXPORT BS_SCRIPT_EXPORT(m :Rendering) TemporalAASettings : public IReflectable
	{
		BS_SCRIPT_EXPORT()
		TemporalAASettings() = default;

		/** Enables or disables temporal anti-aliasing. */
		BS_SCRIPT_EXPORT()
		bool enabled = false;

		/************************************************************************/
		/* 								RTTI		                     		*/
		/************************************************************************/

		/** Enumerates all the fields in the type and executes the specified processor action for each field. */
		template<class P>
		void rttiEnumFields(P processor);
	public:
		friend class TemporalAASettingsRTTI;
		static RTTITypeBase* getRTTIStatic();
		RTTITypeBase* getRTTI() const override;
	};

	/**
	 * Settings that control the screen space reflections effect. Screen space reflections provide high quality mirror-like
	 * reflections at low performance cost. They should be used together with reflection probes as the effects complement
	 * each other. As the name implies, the reflections are only limited to geometry drawn on the screen and the system will
	 * fall back to refl. probes when screen space data is unavailable. Similarly the system will fall back to refl. probes
	 * for rougher (more glossy rather than mirror-like) surfaces. Those surfaces require a higher number of samples to
	 * achieve the glossy look, so we instead fall back to refl. probes which are pre-filtered and can be quickly sampled.
	 */
	struct BS_CORE_EXPORT BS_SCRIPT_EXPORT(m:Rendering) ScreenSpaceReflectionsSettings : public IReflectable
	{
		BS_SCRIPT_EXPORT()
		ScreenSpaceReflectionsSettings() = default;

		/** Enables or disables the SSR effect. */
		BS_SCRIPT_EXPORT()
		bool enabled = true;

		/**
		 * Quality of the SSR effect. Higher values cast more sample rays, and march those rays are lower increments for
		 * better precision. This results in higher quality, as well as a higher performance requirement. Valid range is
		 * [0, 4], default is 2.
		 */
		BS_SCRIPT_EXPORT(range:[0,4],slider)
		UINT32 quality = 2;

		/** Intensity of the screen space reflections. Valid range is [0, 1]. Default is 1 (100%). */
		BS_SCRIPT_EXPORT(range:[0,1],slider)
		float intensity = 1.0f;

		/**
		 * Roughness at which screen space reflections start fading out and become replaced with refl. probes. Valid range
		 * is [0, 1]. Default is 0.8.
		 */
		BS_SCRIPT_EXPORT(range:[0,1],slider)
		float maxRoughness = 0.8f;

		/************************************************************************/
		/* 								RTTI		                     		*/
		/************************************************************************/

		/** Enumerates all the fields in the type and executes the specified processor action for each field. */
		template<class P>
		void rttiEnumFields(P processor);
	public:
		friend class ScreenSpaceReflectionsRTTI;
		static RTTITypeBase* getRTTIStatic();
		RTTITypeBase* getRTTI() const override;
	};

	/** Settings that control the bloom effect. Bloom adds an extra highlight to bright areas of the scene. */
	struct BS_CORE_EXPORT BS_SCRIPT_EXPORT(m:Rendering) BloomSettings : public IReflectable
	{
		BS_SCRIPT_EXPORT()
		BloomSettings() = default;

		/** Enables or disables the bloom effect. */
		BS_SCRIPT_EXPORT()
		bool enabled = false;

		/**
		 * Quality of the bloom effect. Higher values will use higher resolution texture for calculating bloom, at the cost
		 * of lower performance. Valid range is [0, 3], default is 2.
		 */
		BS_SCRIPT_EXPORT()
		UINT32 quality = 2;

		/**
		 * Determines the minimal threshold of pixel luminance to be included in the bloom calculations. Any pixel with
		 * luminance below this value will be ignored for the purposes of bloom. The value represents luminance after
		 * it is scaled by exposure. Set to zero or negative to disable the threshold and include all pixels in the
		 * calculations.
		 */
		BS_SCRIPT_EXPORT()
		float threshold = 1.0f;

		/**
		 * Determines the intensity of the bloom effect. Ideally should be in [0, 4] range but higher values are allowed.
		 */
		BS_SCRIPT_EXPORT()
		float intensity = 0.5f;

		/** Tint color to apply to the bloom highlight. A pure white means the bloom inherits the underlying scene color. */
		BS_SCRIPT_EXPORT()
		Color tint = Color::White;

		/**
		 * Determines the percent of the texture to account for when filtering for bloom. Larger values will include
		 * farther away pixels.
		 */
		BS_SCRIPT_EXPORT(range:[0.01,1.0])
		float filterSize = 0.15f;

		/************************************************************************/
		/* 								RTTI		                     		*/
		/************************************************************************/

		/** Enumerates all the fields in the type and executes the specified processor action for each field. */
		template<class P>
		void rttiEnumFields(P processor);
	public:
		friend class BloomSettingsRTTI;
		static RTTITypeBase* getRTTIStatic();
		RTTITypeBase* getRTTI() const override;
	};

	/** Settings that control the screen-space lens flare effect. */
	struct BS_CORE_EXPORT BS_SCRIPT_EXPORT(m:Rendering) ScreenSpaceLensFlareSettings : public IReflectable
	{
		BS_SCRIPT_EXPORT()
		ScreenSpaceLensFlareSettings() = default;

		/** Enables or disables the lens flare effect. */
		BS_SCRIPT_EXPORT()
		bool enabled = false;

		/**
		 * Determines how many times to downsample the scene texture before using it for lens flare effect. Lower values
		 * will use higher resolution texture for calculating lens flare, at the cost of lower performance.
		 * Valid range is [1, 6], default is 4.
		 */
		BS_SCRIPT_EXPORT(range:[1,6])
		UINT32 downsampleCount = 4;

		/**
		 * Determines the minimal threshold of pixel luminance to be included in the lens flare calculations. Any pixel
		 * with luminance below this value will be ignored for the purposes of lens flare. Set to zero or negative to
		 * disable the threshold and include all pixels in the calculations.
		 */
		BS_SCRIPT_EXPORT()
		float threshold = 32.0f;

		/** Determines the number of ghost features to appear, shown as blurred blobs of bright areas of the scene. */
		BS_SCRIPT_EXPORT(range:[1,10])
		UINT32 ghostCount = 2;

		/**
		 * Determines the distance between ghost features. Value is in normalized screen space, in range [0,1] where
		 * 1 represents the full screen length.
		 */
		BS_SCRIPT_EXPORT(range:[0,1])
		float ghostSpacing = 0.5f;

		/**
		 * Determines the brightness of the lens flare effect. Value of 1 means the lens flare will be displayed at the
		 * same intensity as the scene it was derived from. In range [0, 1], default being 0.05.
		 */
		BS_SCRIPT_EXPORT(range:[0,1])
		float brightness = 0.05f;

		/**
		 * Determines the size of the filter when blurring the lens flare features. Larger values yield a blurrier image
		 * and will require more performance.
		 */
		BS_SCRIPT_EXPORT(range:[0.01,1.0])
		float filterSize = 0.2f;

		/** Determines if a halo effect should be rendered as part of the lens flare. */
		BS_SCRIPT_EXPORT()
		bool halo = true;

		/**
		 * Determines how far away from the screen center does the halo appear, in normalized screen space (range [0,1])
		 * where 0.5 represents half screen length.
		 */
		BS_SCRIPT_EXPORT(range:[0,1])
		float haloRadius = 0.35f;

		/** Determines the thickness of the halo ring. In normalized screen space (range [0.01,0.5]). */
		BS_SCRIPT_EXPORT(range:[0.01,0.5])
		float haloThickness = 0.15f;

		/**
		 * Determines the minimal threshold of pixel luminance to be included for halo generation. Any pixel with
		 * luminance below this value will be ignored for the purposes of halo generation.
		 */
		BS_SCRIPT_EXPORT()
		float haloThreshold = 4.0f;

		/** Determines the shape of the halo. Set to value other than 1 to make the halo an oval rather than a circle. */
		BS_SCRIPT_EXPORT(range:[0,2])
		float haloAspectRatio = 1.0f;

		/**
		 * Enables or disables chromatic aberration of the lens flare and halo features. Chromatic aberration separates
		 * the values of red, green and blue channels according to a user provided offset.
		 */
		BS_SCRIPT_EXPORT()
		bool chromaticAberration = true;

		/**
		 * Determines the distance between pixels within which to sample different channels. The value is in
		 * UV coordinates, range [0, 1].
		 */
		BS_SCRIPT_EXPORT(range:[0,1])
		float chromaticAberrationOffset = 0.01f;

		/**
		 * Uses a higher quality upscaling when blending the lens flare features with scene color. Results in less blocky
		 * artifacts at a cost to performance.
		 */
		BS_SCRIPT_EXPORT()
		bool bicubicUpsampling = false;

		/************************************************************************/
		/* 								RTTI		                     		*/
		/************************************************************************/

		/** Enumerates all the fields in the type and executes the specified processor action for each field. */
		template<class P>
		void rttiEnumFields(P processor);
	public:
		friend class ScreenSpaceLensFlareSettingsRTTI;
		static RTTITypeBase* getRTTIStatic();
		RTTITypeBase* getRTTI() const override;
	};

	/** Types of available chromatic aberration effects. */
	enum class BS_SCRIPT_EXPORT(m:Rendering) ChromaticAberrationType
	{
		/** Simple chromatic aberration effect that is fast to execute. */
		Simple,

		/**
		 * More complex chromatic aberration effect that takes longer to execute but may yield
		 * more visually pleasing results than the simple variant.
		 */
		Complex
	};

	/** Base class used for both sim and core thread variants of ChromaticAberrationSettings. */
	struct BS_CORE_EXPORT ChromaticAberrationSettingsBase
	{
		ChromaticAberrationSettingsBase() = default;

		/** Enables or disables the effect. */
		BS_SCRIPT_EXPORT()
		bool enabled = false;

		/** Type of algorithm to use for rendering the effect. */
		BS_SCRIPT_EXPORT()
		ChromaticAberrationType type = ChromaticAberrationType::Simple;

		/**
		 * Determines the brightness of the lens flare effect. Value of 1 means the lens flare will be displayed at the
		 * same intensity as the scene it was derived from. In range [0, 1], default being 0.05.
		 */
		BS_SCRIPT_EXPORT(range:[0,1])
		float shiftAmount = 0.05f;

	protected:
		~ChromaticAberrationSettingsBase() = default;
	};

	/** Template version of ChromaticAberrationSettings that can be specialized for either core or simulation thread. */
	template<bool Core>
	struct BS_CORE_EXPORT TChromaticAberrationSettings : ChromaticAberrationSettingsBase 
	{
		using TextureType = CoreVariantHandleType<Texture, Core>;

		/**
		 * Optional texture to apply to generate the channel shift fringe, allowing you to modulate the shifted colors.
		 * This texture should be 3x1 size, where the first pixel modules red, second green and third blue channel.
		 * If using the complex aberration effect you can use a larger texture, Nx1 size.
		 */
		BS_SCRIPT_EXPORT()
		TextureType fringeTexture;

		/************************************************************************/
		/* 								RTTI		                     		*/
		/************************************************************************/

		/** Enumerates all the fields in the type and executes the specified processor action for each field. */
		template<class P>
		void rttiEnumFields(P processor);

	protected:
		~TChromaticAberrationSettings() = default;
	};

	/** Settings that control the chromatic aberration effect. */
	struct BS_CORE_EXPORT BS_SCRIPT_EXPORT() ChromaticAberrationSettings : TChromaticAberrationSettings<false>, IReflectable
	{
		BS_SCRIPT_EXPORT()
		ChromaticAberrationSettings() = default;

		/************************************************************************/
		/* 								RTTI		                     		*/
		/************************************************************************/
	public:
		friend class ChromaticAberrationSettingsRTTI;
		static RTTITypeBase* getRTTIStatic();
		RTTITypeBase* getRTTI() const override;
	};

	namespace ct
	{
		/** Core thread variant of ChromaticAberrationSettings. */
		struct BS_CORE_EXPORT ChromaticAberrationSettings : TChromaticAberrationSettings<true>
		{
			ChromaticAberrationSettings() = default;
		};
	}

	/** Settings that control the film grain effect. Film grains adds a time-varying noise effect over the entire image. */
	struct BS_CORE_EXPORT BS_SCRIPT_EXPORT(m:Rendering) FilmGrainSettings : public IReflectable
	{
		BS_SCRIPT_EXPORT()
		FilmGrainSettings() = default;

		/** Enables or disables the effect. */
		BS_SCRIPT_EXPORT()
		bool enabled = false;

		/** Controls how intense are the displayed film grains. */
		BS_SCRIPT_EXPORT(range:[0,100.0])
		float intensity = 16.0f;

		/** Controls at what speed do the film grains change. */
		BS_SCRIPT_EXPORT(range:[0,100.0])
		float speed = 10.0f;

		/************************************************************************/
		/* 								RTTI		                     		*/
		/************************************************************************/

		/** Enumerates all the fields in the type and executes the specified processor action for each field. */
		template<class P>
		void rttiEnumFields(P processor);
	public:
		friend class FilmGrainSettingsRTTI;
		static RTTITypeBase* getRTTIStatic();
		RTTITypeBase* getRTTI() const override;
	};

	/** Various options that control shadow rendering for a specific view. */
	struct BS_CORE_EXPORT BS_SCRIPT_EXPORT(m:Rendering) ShadowSettings : public IReflectable
	{
		BS_SCRIPT_EXPORT()
		ShadowSettings() = default;

		/**
		 * Maximum distance that directional light shadows are allowed to render at. Decreasing the distance can yield
		 * higher quality shadows nearer to the viewer, as the shadow map resolution isn't being used up on far away
		 * portions of the scene. In world units (meters).
		 */
		BS_SCRIPT_EXPORT()
		float directionalShadowDistance = 250.0f;
		
		/**
		 * Number of cascades to use for directional shadows. Higher number of cascades increases shadow quality as each
		 * individual cascade has less area to cover, but can significantly increase performance cost, as well as a minor
		 * increase in memory cost. Valid range is roughly [1, 6].
		 */
		BS_SCRIPT_EXPORT(range:[1,6],slider)
		UINT32 numCascades = 4;

		/**
		 * Allows you to control how are directional shadow cascades distributed. Value of 1 means the cascades will be
		 * linearly split, each cascade taking up the same amount of space. Value of 2 means each subsequent split will be
		 * twice the size of the previous one (meaning cascades closer to the viewer cover a smaller area, and therefore
		 * yield higher resolution shadows). Higher values increase the size disparity between near and far cascades at
		 * an exponential rate. Valid range is roughly [1, 4].
		 */
		BS_SCRIPT_EXPORT(range:[1,4],slider)
		float cascadeDistributionExponent = 3.0f;

		/**
		 * Determines the number of samples used for percentage closer shadow map filtering. Higher values yield higher
		 * quality shadows, at the cost of performance. Valid range is [1, 4].
		 */
		BS_SCRIPT_EXPORT(range:[1,4],slider)
		UINT32 shadowFilteringQuality = 4;

		/************************************************************************/
		/* 								RTTI		                     		*/
		/************************************************************************/

		/** Enumerates all the fields in the type and executes the specified processor action for each field. */
		template<class P>
		void rttiEnumFields(P processor);
	public:
		friend class ShadowSettingsRTTI;
		static RTTITypeBase* getRTTIStatic();
		RTTITypeBase* getRTTI() const override;
	};

	/** Base class for both sim and core thread variants of RenderSettings. */
	struct BS_CORE_EXPORT RenderSettingsBase
	{
		RenderSettingsBase() = default;

		/**
		 * Determines should automatic exposure be applied to the HDR image. When turned on the average scene brightness
		 * will be calculated and used to automatically expose the image to the optimal range. Use the parameters provided
		 * by autoExposure to customize the automatic exposure effect. You may also use exposureScale to
		 * manually adjust the automatic exposure. When automatic exposure is turned off you can use exposureScale to
		 * manually set the exposure.
		 */
		BS_SCRIPT_EXPORT()
		bool enableAutoExposure = true;

		/**
		 * Parameters used for customizing automatic scene exposure.
		 *
		 * @see	enableAutoExposure
		 */
		BS_SCRIPT_EXPORT()
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
		BS_SCRIPT_EXPORT()
		bool enableTonemapping = true;

		/**
		 * Parameters used for customizing tonemapping.
		 *
		 * @see	enableTonemapping
		 */
		BS_SCRIPT_EXPORT()
		TonemappingSettings tonemapping;

		/**
		 * Parameters used for customizing white balancing. White balancing converts a scene illuminated by a light of the
		 * specified temperature into a scene illuminated by a standard D65 illuminant (average midday light) in order to
		 * simulate the effects of chromatic adaptation of the human visual system.
		 */
		BS_SCRIPT_EXPORT()
		WhiteBalanceSettings whiteBalance;

		/** Parameters used for customizing color grading. */
		BS_SCRIPT_EXPORT()
		ColorGradingSettings colorGrading;

		/** Parameters used for customizing screen space ambient occlusion. */
		BS_SCRIPT_EXPORT()
		AmbientOcclusionSettings ambientOcclusion;

		/** Parameters used for customizing screen space reflections. */
		BS_SCRIPT_EXPORT()
		ScreenSpaceReflectionsSettings screenSpaceReflections;

		/** Parameters used for customizing the bloom effect. */
		BS_SCRIPT_EXPORT()
		BloomSettings bloom;

		/** Parameters used for customizing the screen space lens flare effect. */
		BS_SCRIPT_EXPORT()
		ScreenSpaceLensFlareSettings screenSpaceLensFlare;

		/** Parameters used for customizing the film grain effect. */
		BS_SCRIPT_EXPORT()
		FilmGrainSettings filmGrain;

		/** Parameters used for customizing the motion blur effect. */
		BS_SCRIPT_EXPORT()
		MotionBlurSettings motionBlur;

		/** Parameters used for customizing the temporal anti-aliasing effect. */
		BS_SCRIPT_EXPORT()
		TemporalAASettings temporalAA;

		/** Enables the fast approximate anti-aliasing effect. */
		BS_SCRIPT_EXPORT()
		bool enableFXAA = true;

		/**
		 * Log2 value to scale the eye adaptation by (for example 2^0 = 1). Smaller values yield darker image, while larger
		 * yield brighter image. Allows you to customize exposure manually, applied on top of eye adaptation exposure (if
		 * enabled). In range [-8, 8].
		 */
		BS_SCRIPT_EXPORT(range:[-8,8],slider)
		float exposureScale = 1.25f;

		/**
		 * Gamma value to adjust the image for. Larger values result in a brighter image. When tonemapping is turned
		 * on the best gamma curve for the output device is chosen automatically and this value can by used to merely tweak
		 * that curve. If tonemapping is turned off this is the exact value of the gamma curve that will be applied.
		 */
		BS_SCRIPT_EXPORT(range:[1,3],slider)
		float gamma = 2.2f;

		/**
		 * High dynamic range allows light intensity to be more correctly recorded when rendering by allowing for a larger
		 * range of values. The stored light is then converted into visible color range using exposure and a tone mapping
		 * operator.
		 */
		BS_SCRIPT_EXPORT()
		bool enableHDR = true;

		/**
		 * Determines if scene objects will be lit by lights. If disabled everything will be rendered using their albedo
		 * texture with no lighting applied.
		 */
		BS_SCRIPT_EXPORT()
		bool enableLighting = true;

		/** Determines if shadows cast by lights should be rendered. Only relevant if lighting is turned on. */
		BS_SCRIPT_EXPORT()
		bool enableShadows = true;

		/**
		 * Determines if the G-Buffer should contain per-pixel velocity information. This can be useful if you are rendering
		 * an effect that requires this information. Note that effects such as motion blur or temporal anti-aliasing
		 * might force the velocity buffer to be enabled regardless of this setting.
		 */
		BS_SCRIPT_EXPORT()
		bool enableVelocityBuffer = false;

		/** Parameters used for customizing shadow rendering. */
		BS_SCRIPT_EXPORT()
		ShadowSettings shadowSettings;

		/** Determines if indirect lighting (e.g. from light probes or the sky) is rendered. */
		BS_SCRIPT_EXPORT()
		bool enableIndirectLighting = true;

		/**
		 * Signals the renderer to only render overlays (like GUI), and not scene objects. Such rendering doesn't require
		 * depth buffer or multi-sampled render targets and will not render any scene objects. This can improve performance
		 * and memory usage for overlay-only views.
		 */
		BS_SCRIPT_EXPORT()
		bool overlayOnly = false;

		/**
		 * If enabled the camera will use the skybox for rendering the background. A skybox has to be present in the scene.
		 * When disabled the camera will use the clear color for rendering the background.
		 */
		BS_SCRIPT_EXPORT()
		bool enableSkybox = true;

		/**
		 * The absolute base cull-distance for objects rendered through this camera in world units. Objects will use this
		 * distance and apply their own factor to it to determine whether they should be visible.
		 */
		BS_SCRIPT_EXPORT()
		float cullDistance = FLT_MAX;

	protected:
		~RenderSettingsBase() = default;
	};

	/** Template version of RenderSettings that can be specialized for either core or simulation thread. */
	template<bool Core>
	struct BS_CORE_EXPORT TRenderSettings : RenderSettingsBase
	{
		/** Parameters used for customizing the gaussian depth of field effect. */
		BS_SCRIPT_EXPORT()
		CoreVariantType<DepthOfFieldSettings, Core> depthOfField;

		/** Parameters used for customizing the chromatic aberration effect. */
		BS_SCRIPT_EXPORT()
		CoreVariantType<ChromaticAberrationSettings, Core> chromaticAberration;

		/************************************************************************/
		/* 								RTTI		                     		*/
		/************************************************************************/

		/** Enumerates all the fields in the type and executes the specified processor action for each field. */
		template<class P>
		void rttiEnumFields(P processor);

	protected:
		~TRenderSettings() = default;
	};
	
	/** Settings that control rendering for a specific camera (view). */
	struct BS_CORE_EXPORT BS_SCRIPT_EXPORT(m:Rendering) RenderSettings : TRenderSettings<false>, IReflectable
	{
		BS_SCRIPT_EXPORT()
		RenderSettings() = default;
		virtual ~RenderSettings() = default;

		/************************************************************************/
		/* 								RTTI		                     		*/
		/************************************************************************/

	public:
		friend class RenderSettingsRTTI;
		static RTTITypeBase* getRTTIStatic();
		RTTITypeBase* getRTTI() const override;
	};

	namespace ct
	{
		/** Core thread variant of RenderSettings. */
		struct BS_CORE_EXPORT BS_SCRIPT_EXPORT(m :Rendering) RenderSettings : TRenderSettings<true>
		{
			RenderSettings() = default;
			virtual ~RenderSettings() = default;
		};
	}

	/** @} */
}
