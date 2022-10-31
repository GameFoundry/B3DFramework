//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsCorePrerequisites.h"
#include "Reflection/BsRTTIType.h"
#include "RTTI/BsColorRTTI.h"
#include "Renderer/BsRenderSettings.h"

namespace bs
{
	/** @cond RTTI */
	/** @addtogroup RTTI-Impl-Engine
	 *  @{
	 */

	class BS_CORE_EXPORT AutoExposureSettingsRTTI : public RTTIType<AutoExposureSettings, IReflectable, AutoExposureSettingsRTTI>
	{
	private:
		BS_BEGIN_RTTI_MEMBERS
			BS_RTTI_MEMBER_PLAIN(HistogramLog2Min, 0)
			BS_RTTI_MEMBER_PLAIN(HistogramLog2Max, 1)
			BS_RTTI_MEMBER_PLAIN(HistogramPctLow, 2)
			BS_RTTI_MEMBER_PLAIN(HistogramPctHigh, 3)
			BS_RTTI_MEMBER_PLAIN(MinEyeAdaptation, 4)
			BS_RTTI_MEMBER_PLAIN(MaxEyeAdaptation, 5)
			BS_RTTI_MEMBER_PLAIN(EyeAdaptationSpeedUp, 6)
			BS_RTTI_MEMBER_PLAIN(EyeAdaptationSpeedDown, 7)
		BS_END_RTTI_MEMBERS

	public:
		const String& GetRttiName()
		{
			static String name = "AutoExposureSettings";
			return name;
		}

		u32 GetRttiId()
		{
			return TID_AutoExposureSettings;
		}

		SPtr<IReflectable> NewRttiObject()
		{
			return B3DMakeShared<AutoExposureSettings>();
		}
	};

	class BS_CORE_EXPORT TonemappingSettingsRTTI : public RTTIType<TonemappingSettings, IReflectable, TonemappingSettingsRTTI>
	{
	private:
		BS_BEGIN_RTTI_MEMBERS
			BS_RTTI_MEMBER_PLAIN(FilmicCurveShoulderStrength, 0)
			BS_RTTI_MEMBER_PLAIN(FilmicCurveLinearStrength, 1)
			BS_RTTI_MEMBER_PLAIN(FilmicCurveLinearAngle, 2)
			BS_RTTI_MEMBER_PLAIN(FilmicCurveToeStrength, 3)
			BS_RTTI_MEMBER_PLAIN(FilmicCurveToeNumerator, 4)
			BS_RTTI_MEMBER_PLAIN(FilmicCurveToeDenominator, 5)
			BS_RTTI_MEMBER_PLAIN(FilmicCurveLinearWhitePoint, 6)
		BS_END_RTTI_MEMBERS

	public:
		const String& GetRttiName() override
		{
			static String name = "TonemappingSettings";
			return name;
		}

		u32 GetRttiId() override
		{
			return TID_TonemappingSettings;
		}

		SPtr<IReflectable> NewRttiObject() override
		{
			return B3DMakeShared<TonemappingSettings>();
		}
	};

	class BS_CORE_EXPORT WhiteBalanceSettingsRTTI : public RTTIType<WhiteBalanceSettings, IReflectable, WhiteBalanceSettingsRTTI>
	{
	private:
		BS_BEGIN_RTTI_MEMBERS
			BS_RTTI_MEMBER_PLAIN(Temperature, 0)
			BS_RTTI_MEMBER_PLAIN(Tint, 1)
		BS_END_RTTI_MEMBERS

	public:
		const String& GetRttiName() override
		{
			static String name = "WhiteBalanceSettings";
			return name;
		}

		u32 GetRttiId() override
		{
			return TID_WhiteBalanceSettings;
		}

		SPtr<IReflectable> NewRttiObject() override
		{
			return B3DMakeShared<WhiteBalanceSettings>();
		}
	};

	class BS_CORE_EXPORT ColorGradingSettingsRTTI : public RTTIType<ColorGradingSettings, IReflectable, ColorGradingSettingsRTTI>
	{
	private:
		BS_BEGIN_RTTI_MEMBERS
			BS_RTTI_MEMBER_PLAIN(Saturation, 0)
			BS_RTTI_MEMBER_PLAIN(Contrast, 1)
			BS_RTTI_MEMBER_PLAIN(Gain, 2)
			BS_RTTI_MEMBER_PLAIN(Offset, 3)
		BS_END_RTTI_MEMBERS

	public:
		const String& GetRttiName() override
		{
			static String name = "ColorGradingSettings";
			return name;
		}

		u32 GetRttiId() override
		{
			return TID_ColorGradingSettings;
		}

		SPtr<IReflectable> NewRttiObject() override
		{
			return B3DMakeShared<ColorGradingSettings>();
		}
	};

	class BS_CORE_EXPORT DepthOfFieldSettingsRTTI : public RTTIType<DepthOfFieldSettings, IReflectable, DepthOfFieldSettingsRTTI>
	{
	private:
		BS_BEGIN_RTTI_MEMBERS
			BS_RTTI_MEMBER_PLAIN(Enabled, 0)
			BS_RTTI_MEMBER_PLAIN(FocalDistance, 1)
			BS_RTTI_MEMBER_PLAIN(FocalRange, 2)
			BS_RTTI_MEMBER_PLAIN(NearTransitionRange, 3)
			BS_RTTI_MEMBER_PLAIN(FarTransitionRange, 4)
			BS_RTTI_MEMBER_PLAIN(NearBlurAmount, 5)
			BS_RTTI_MEMBER_PLAIN(FarBlurAmount, 6)
			BS_RTTI_MEMBER_PLAIN(Type, 7)
			BS_RTTI_MEMBER_PLAIN(MaxBokehSize, 8)
			BS_RTTI_MEMBER_REFL(BokehShape, 9)
			BS_RTTI_MEMBER_PLAIN(AdaptiveColorThreshold, 10)
			BS_RTTI_MEMBER_PLAIN(AdaptiveRadiusThreshold, 11)
			BS_RTTI_MEMBER_PLAIN(ApertureSize, 12)
			BS_RTTI_MEMBER_PLAIN(FocalLength, 13)
			BS_RTTI_MEMBER_PLAIN(SensorSize, 14)
			BS_RTTI_MEMBER_PLAIN(BokehOcclusion, 15)
			BS_RTTI_MEMBER_PLAIN(OcclusionDepthRange, 16)
		BS_END_RTTI_MEMBERS

	public:
		const String& GetRttiName() override
		{
			static String name = "DepthOfFieldSettings";
			return name;
		}

		u32 GetRttiId() override
		{
			return TID_DepthOfFieldSettings;
		}

		SPtr<IReflectable> NewRttiObject() override
		{
			return B3DMakeShared<DepthOfFieldSettings>();
		}
	};

	class BS_CORE_EXPORT AmbientOcclusionSettingsRTTI : public RTTIType<AmbientOcclusionSettings, IReflectable, AmbientOcclusionSettingsRTTI>
	{
	private:
		BS_BEGIN_RTTI_MEMBERS
			BS_RTTI_MEMBER_PLAIN(Enabled, 0)
			BS_RTTI_MEMBER_PLAIN(Radius, 1)
			BS_RTTI_MEMBER_PLAIN(Bias, 2)
			BS_RTTI_MEMBER_PLAIN(FadeRange, 3)
			BS_RTTI_MEMBER_PLAIN(FadeDistance, 4)
			BS_RTTI_MEMBER_PLAIN(Intensity, 5)
			BS_RTTI_MEMBER_PLAIN(Power, 6)
			BS_RTTI_MEMBER_PLAIN(Quality, 7)
		BS_END_RTTI_MEMBERS

	public:
		const String& GetRttiName() override
		{
			static String name = "AmbientOcclusionSettings";
			return name;
		}

		u32 GetRttiId() override
		{
			return TID_AmbientOcclusionSettings;
		}

		SPtr<IReflectable> NewRttiObject() override
		{
			return B3DMakeShared<AmbientOcclusionSettings>();
		}
	};

	class BS_CORE_EXPORT MotionBlurSettingsRTTI : public RTTIType<MotionBlurSettings, IReflectable, MotionBlurSettingsRTTI>
	{
	private:
		BS_BEGIN_RTTI_MEMBERS
			BS_RTTI_MEMBER_PLAIN(Enabled, 0)
			BS_RTTI_MEMBER_PLAIN(Domain, 1)
			BS_RTTI_MEMBER_PLAIN(Filter, 2)
			BS_RTTI_MEMBER_PLAIN(Quality, 3)
			BS_RTTI_MEMBER_PLAIN(MaximumRadius, 4)
		BS_END_RTTI_MEMBERS

	public:
		const String& GetRttiName() override
		{
			static String name = "MotionBlurSettings";
			return name;
		}

		u32 GetRttiId() override
		{
			return TID_MotionBlurSettings;
		}

		SPtr<IReflectable> NewRttiObject() override
		{
			return B3DMakeShared<MotionBlurSettings>();
		}
	};

	class BS_CORE_EXPORT TemporalAASettingsRTTI : public RTTIType<TemporalAASettings, IReflectable, TemporalAASettingsRTTI>
	{
	private:
		BS_BEGIN_RTTI_MEMBERS
			BS_RTTI_MEMBER_PLAIN(Enabled, 0)
			BS_RTTI_MEMBER_PLAIN(JitteredPositionCount, 1)
			BS_RTTI_MEMBER_PLAIN(Sharpness, 2)
		BS_END_RTTI_MEMBERS

	public:
		const String& GetRttiName() override
		{
			static String name = "TemporalAASettings";
			return name;
		}

		u32 GetRttiId() override
		{
			return TID_TemporalAASettings;
		}

		SPtr<IReflectable> NewRttiObject() override
		{
			return B3DMakeShared<TemporalAASettings>();
		}
	};

	class BS_CORE_EXPORT ScreenSpaceReflectionsSettingsRTTI : public RTTIType<ScreenSpaceReflectionsSettings, IReflectable, ScreenSpaceReflectionsSettingsRTTI>
	{
	private:
		BS_BEGIN_RTTI_MEMBERS
			BS_RTTI_MEMBER_PLAIN(Enabled, 0)
			BS_RTTI_MEMBER_PLAIN(Intensity, 1)
			BS_RTTI_MEMBER_PLAIN(MaxRoughness, 2)
			BS_RTTI_MEMBER_PLAIN(Quality, 3)
		BS_END_RTTI_MEMBERS

	public:
		const String& GetRttiName() override
		{
			static String name = "ScreenSpaceReflectionsSettings";
			return name;
		}

		u32 GetRttiId() override
		{
			return TID_ScreenSpaceReflectionsSettings;
		}

		SPtr<IReflectable> NewRttiObject() override
		{
			return B3DMakeShared<ScreenSpaceReflectionsSettings>();
		}
	};

	class BS_CORE_EXPORT BloomSettingsRTTI : public RTTIType<BloomSettings, IReflectable, BloomSettingsRTTI>
	{
	private:
		BS_BEGIN_RTTI_MEMBERS
			BS_RTTI_MEMBER_PLAIN(Enabled, 0)
			BS_RTTI_MEMBER_PLAIN(Quality, 1)
			BS_RTTI_MEMBER_PLAIN(Threshold, 2)
			BS_RTTI_MEMBER_PLAIN(Intensity, 3)
			BS_RTTI_MEMBER_PLAIN(Tint, 4)
			BS_RTTI_MEMBER_PLAIN(FilterSize, 5)
		BS_END_RTTI_MEMBERS

	public:
		const String& GetRttiName() override
		{
			static String name = "BloomSettings";
			return name;
		}

		u32 GetRttiId() override
		{
			return TID_BloomSettings;
		}

		SPtr<IReflectable> NewRttiObject() override
		{
			return B3DMakeShared<BloomSettings>();
		}
	};

	class BS_CORE_EXPORT ScreenSpaceLensFlareSettingsRTTI : public RTTIType<ScreenSpaceLensFlareSettings, IReflectable, ScreenSpaceLensFlareSettingsRTTI>
	{
	private:
		BS_BEGIN_RTTI_MEMBERS
			BS_RTTI_MEMBER_PLAIN(Enabled, 0)
			BS_RTTI_MEMBER_PLAIN(DownsampleCount, 1)
			BS_RTTI_MEMBER_PLAIN(Threshold, 2)
			BS_RTTI_MEMBER_PLAIN(GhostCount, 3)
			BS_RTTI_MEMBER_PLAIN(GhostSpacing, 4)
			BS_RTTI_MEMBER_PLAIN(Brightness, 5)
			BS_RTTI_MEMBER_PLAIN(FilterSize, 6)
			BS_RTTI_MEMBER_PLAIN(Halo, 7)
			BS_RTTI_MEMBER_PLAIN(HaloAspectRatio, 8)
			BS_RTTI_MEMBER_PLAIN(HaloRadius, 9)
			BS_RTTI_MEMBER_PLAIN(HaloThickness, 10)
			BS_RTTI_MEMBER_PLAIN(HaloThreshold, 11)
			BS_RTTI_MEMBER_PLAIN(ChromaticAberration, 12)
			BS_RTTI_MEMBER_PLAIN(ChromaticAberrationOffset, 13)
			BS_RTTI_MEMBER_PLAIN(BicubicUpsampling, 14)
		BS_END_RTTI_MEMBERS

	public:
		const String& GetRttiName() override
		{
			static String name = "ScreenSpaceLensFlareSettings";
			return name;
		}

		u32 GetRttiId() override
		{
			return TID_ScreenSpaceLensFlareSettings;
		}

		SPtr<IReflectable> NewRttiObject() override
		{
			return B3DMakeShared<ScreenSpaceLensFlareSettings>();
		}
	};

	class BS_CORE_EXPORT ChromaticAberrationSettingsRTTI : public RTTIType<ChromaticAberrationSettings, IReflectable, ChromaticAberrationSettingsRTTI>
	{
	private:
		BS_BEGIN_RTTI_MEMBERS
			BS_RTTI_MEMBER_PLAIN(Enabled, 0)
			BS_RTTI_MEMBER_PLAIN(Type, 1)
			BS_RTTI_MEMBER_PLAIN(ShiftAmount, 2)
			BS_RTTI_MEMBER_REFL(FringeTexture, 3)
		BS_END_RTTI_MEMBERS

	public:
		const String& GetRttiName() override
		{
			static String name = "ChromaticAberrationSettings";
			return name;
		}

		u32 GetRttiId() override
		{
			return TID_ChromaticAberrationSettings;
		}

		SPtr<IReflectable> NewRttiObject() override
		{
			return B3DMakeShared<ChromaticAberrationSettings>();
		}
	};

	class BS_CORE_EXPORT FilmGrainSettingsRTTI : public RTTIType<FilmGrainSettings, IReflectable, FilmGrainSettingsRTTI>
	{
	private:
		BS_BEGIN_RTTI_MEMBERS
			BS_RTTI_MEMBER_PLAIN(Enabled, 0)
			BS_RTTI_MEMBER_PLAIN(Intensity, 1)
			BS_RTTI_MEMBER_PLAIN(Speed, 2)
		BS_END_RTTI_MEMBERS

	public:
		const String& GetRttiName() override
		{
			static String name = "FilmGrainSettings";
			return name;
		}

		u32 GetRttiId() override
		{
			return TID_FilmGrainSettings;
		}

		SPtr<IReflectable> NewRttiObject() override
		{
			return B3DMakeShared<FilmGrainSettings>();
		}
	};

	class BS_CORE_EXPORT ShadowSettingsRTTI : public RTTIType<ShadowSettings, IReflectable, ShadowSettingsRTTI>
	{
	private:
		BS_BEGIN_RTTI_MEMBERS
			BS_RTTI_MEMBER_PLAIN(DirectionalShadowDistance, 0)
			BS_RTTI_MEMBER_PLAIN(NumCascades, 1)
			BS_RTTI_MEMBER_PLAIN(CascadeDistributionExponent, 2)
			BS_RTTI_MEMBER_PLAIN(ShadowFilteringQuality, 3)
		BS_END_RTTI_MEMBERS

	public:
		const String& GetRttiName() override
		{
			static String name = "ShadowSettings";
			return name;
		}

		u32 GetRttiId() override
		{
			return TID_ShadowSettings;
		}

		SPtr<IReflectable> NewRttiObject() override
		{
			return B3DMakeShared<ShadowSettings>();
		}
	};

	class BS_CORE_EXPORT RenderSettingsRTTI : public RTTIType<RenderSettings, IReflectable, RenderSettingsRTTI>
	{
	private:
		BS_BEGIN_RTTI_MEMBERS
			BS_RTTI_MEMBER_PLAIN(EnableAutoExposure, 0)
			BS_RTTI_MEMBER_REFL(AutoExposure, 1)
			BS_RTTI_MEMBER_PLAIN(EnableTonemapping, 2)
			BS_RTTI_MEMBER_REFL(Tonemapping, 3)
			BS_RTTI_MEMBER_REFL(WhiteBalance, 4)
			BS_RTTI_MEMBER_REFL(ColorGrading, 5)
			BS_RTTI_MEMBER_PLAIN(ExposureScale, 6)
			BS_RTTI_MEMBER_PLAIN(Gamma, 7)
			BS_RTTI_MEMBER_REFL(DepthOfField, 8)
			BS_RTTI_MEMBER_PLAIN(EnableFxaa, 9)
			BS_RTTI_MEMBER_REFL(AmbientOcclusion, 10)
			BS_RTTI_MEMBER_REFL(ScreenSpaceReflections, 11)
			BS_RTTI_MEMBER_PLAIN(EnableHdr, 12)
			BS_RTTI_MEMBER_PLAIN(EnableLighting, 13)
			BS_RTTI_MEMBER_PLAIN(EnableShadows, 14)
			BS_RTTI_MEMBER_PLAIN(OverlayOnly, 15)
			BS_RTTI_MEMBER_PLAIN(EnableIndirectLighting, 16)
			BS_RTTI_MEMBER_REFL(ShadowSettings, 17)
			BS_RTTI_MEMBER_PLAIN(EnableSkybox, 18)
			BS_RTTI_MEMBER_REFL(Bloom, 19)
			BS_RTTI_MEMBER_REFL(ScreenSpaceLensFlare, 20)
			BS_RTTI_MEMBER_REFL(MotionBlur, 21)
			BS_RTTI_MEMBER_REFL(FilmGrain, 22)
			BS_RTTI_MEMBER_REFL(ChromaticAberration, 23)
			BS_RTTI_MEMBER_REFL(TemporalAa, 24)
			BS_RTTI_MEMBER_PLAIN(EnableVelocityBuffer, 25)
		BS_END_RTTI_MEMBERS

	public:
		const String& GetRttiName() override
		{
			static String name = "RenderSettings";
			return name;
		}

		u32 GetRttiId() override
		{
			return TID_RenderSettings;
		}

		SPtr<IReflectable> NewRttiObject() override
		{
			return B3DMakeShared<RenderSettings>();
		}
	};

	/** @} */
	/** @endcond */
} // namespace bs
