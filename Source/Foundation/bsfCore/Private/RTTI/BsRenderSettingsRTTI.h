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

	class B3D_CORE_EXPORT AutoExposureSettingsRTTI : public RTTIType<AutoExposureSettings, IReflectable, AutoExposureSettingsRTTI>
	{
	private:
		B3D_RTTI_BEGIN_MEMBERS
			B3D_RTTI_MEMBER_PLAIN(HistogramLog2Min, 0)
			B3D_RTTI_MEMBER_PLAIN(HistogramLog2Max, 1)
			B3D_RTTI_MEMBER_PLAIN(HistogramPctLow, 2)
			B3D_RTTI_MEMBER_PLAIN(HistogramPctHigh, 3)
			B3D_RTTI_MEMBER_PLAIN(MinEyeAdaptation, 4)
			B3D_RTTI_MEMBER_PLAIN(MaxEyeAdaptation, 5)
			B3D_RTTI_MEMBER_PLAIN(EyeAdaptationSpeedUp, 6)
			B3D_RTTI_MEMBER_PLAIN(EyeAdaptationSpeedDown, 7)
		B3D_RTTI_END_MEMBERS

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

	class B3D_CORE_EXPORT TonemappingSettingsRTTI : public RTTIType<TonemappingSettings, IReflectable, TonemappingSettingsRTTI>
	{
	private:
		B3D_RTTI_BEGIN_MEMBERS
			B3D_RTTI_MEMBER_PLAIN(FilmicCurveShoulderStrength, 0)
			B3D_RTTI_MEMBER_PLAIN(FilmicCurveLinearStrength, 1)
			B3D_RTTI_MEMBER_PLAIN(FilmicCurveLinearAngle, 2)
			B3D_RTTI_MEMBER_PLAIN(FilmicCurveToeStrength, 3)
			B3D_RTTI_MEMBER_PLAIN(FilmicCurveToeNumerator, 4)
			B3D_RTTI_MEMBER_PLAIN(FilmicCurveToeDenominator, 5)
			B3D_RTTI_MEMBER_PLAIN(FilmicCurveLinearWhitePoint, 6)
		B3D_RTTI_END_MEMBERS

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

	class B3D_CORE_EXPORT WhiteBalanceSettingsRTTI : public RTTIType<WhiteBalanceSettings, IReflectable, WhiteBalanceSettingsRTTI>
	{
	private:
		B3D_RTTI_BEGIN_MEMBERS
			B3D_RTTI_MEMBER_PLAIN(Temperature, 0)
			B3D_RTTI_MEMBER_PLAIN(Tint, 1)
		B3D_RTTI_END_MEMBERS

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

	class B3D_CORE_EXPORT ColorGradingSettingsRTTI : public RTTIType<ColorGradingSettings, IReflectable, ColorGradingSettingsRTTI>
	{
	private:
		B3D_RTTI_BEGIN_MEMBERS
			B3D_RTTI_MEMBER_PLAIN(Saturation, 0)
			B3D_RTTI_MEMBER_PLAIN(Contrast, 1)
			B3D_RTTI_MEMBER_PLAIN(Gain, 2)
			B3D_RTTI_MEMBER_PLAIN(Offset, 3)
		B3D_RTTI_END_MEMBERS

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

	class B3D_CORE_EXPORT DepthOfFieldSettingsRTTI : public RTTIType<DepthOfFieldSettings, IReflectable, DepthOfFieldSettingsRTTI>
	{
	private:
		B3D_RTTI_BEGIN_MEMBERS
			B3D_RTTI_MEMBER_PLAIN(Enabled, 0)
			B3D_RTTI_MEMBER_PLAIN(FocalDistance, 1)
			B3D_RTTI_MEMBER_PLAIN(FocalRange, 2)
			B3D_RTTI_MEMBER_PLAIN(NearTransitionRange, 3)
			B3D_RTTI_MEMBER_PLAIN(FarTransitionRange, 4)
			B3D_RTTI_MEMBER_PLAIN(NearBlurAmount, 5)
			B3D_RTTI_MEMBER_PLAIN(FarBlurAmount, 6)
			B3D_RTTI_MEMBER_PLAIN(Type, 7)
			B3D_RTTI_MEMBER_PLAIN(MaxBokehSize, 8)
			B3D_RTTI_MEMBER_REFL(BokehShape, 9)
			B3D_RTTI_MEMBER_PLAIN(AdaptiveColorThreshold, 10)
			B3D_RTTI_MEMBER_PLAIN(AdaptiveRadiusThreshold, 11)
			B3D_RTTI_MEMBER_PLAIN(ApertureSize, 12)
			B3D_RTTI_MEMBER_PLAIN(FocalLength, 13)
			B3D_RTTI_MEMBER_PLAIN(SensorSize, 14)
			B3D_RTTI_MEMBER_PLAIN(BokehOcclusion, 15)
			B3D_RTTI_MEMBER_PLAIN(OcclusionDepthRange, 16)
		B3D_RTTI_END_MEMBERS

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

	class B3D_CORE_EXPORT AmbientOcclusionSettingsRTTI : public RTTIType<AmbientOcclusionSettings, IReflectable, AmbientOcclusionSettingsRTTI>
	{
	private:
		B3D_RTTI_BEGIN_MEMBERS
			B3D_RTTI_MEMBER_PLAIN(Enabled, 0)
			B3D_RTTI_MEMBER_PLAIN(Radius, 1)
			B3D_RTTI_MEMBER_PLAIN(Bias, 2)
			B3D_RTTI_MEMBER_PLAIN(FadeRange, 3)
			B3D_RTTI_MEMBER_PLAIN(FadeDistance, 4)
			B3D_RTTI_MEMBER_PLAIN(Intensity, 5)
			B3D_RTTI_MEMBER_PLAIN(Power, 6)
			B3D_RTTI_MEMBER_PLAIN(Quality, 7)
		B3D_RTTI_END_MEMBERS

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

	class B3D_CORE_EXPORT MotionBlurSettingsRTTI : public RTTIType<MotionBlurSettings, IReflectable, MotionBlurSettingsRTTI>
	{
	private:
		B3D_RTTI_BEGIN_MEMBERS
			B3D_RTTI_MEMBER_PLAIN(Enabled, 0)
			B3D_RTTI_MEMBER_PLAIN(Domain, 1)
			B3D_RTTI_MEMBER_PLAIN(Filter, 2)
			B3D_RTTI_MEMBER_PLAIN(Quality, 3)
			B3D_RTTI_MEMBER_PLAIN(MaximumRadius, 4)
		B3D_RTTI_END_MEMBERS

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

	class B3D_CORE_EXPORT TemporalAASettingsRTTI : public RTTIType<TemporalAASettings, IReflectable, TemporalAASettingsRTTI>
	{
	private:
		B3D_RTTI_BEGIN_MEMBERS
			B3D_RTTI_MEMBER_PLAIN(Enabled, 0)
			B3D_RTTI_MEMBER_PLAIN(JitteredPositionCount, 1)
			B3D_RTTI_MEMBER_PLAIN(Sharpness, 2)
		B3D_RTTI_END_MEMBERS

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

	class B3D_CORE_EXPORT ScreenSpaceReflectionsSettingsRTTI : public RTTIType<ScreenSpaceReflectionsSettings, IReflectable, ScreenSpaceReflectionsSettingsRTTI>
	{
	private:
		B3D_RTTI_BEGIN_MEMBERS
			B3D_RTTI_MEMBER_PLAIN(Enabled, 0)
			B3D_RTTI_MEMBER_PLAIN(Intensity, 1)
			B3D_RTTI_MEMBER_PLAIN(MaxRoughness, 2)
			B3D_RTTI_MEMBER_PLAIN(Quality, 3)
		B3D_RTTI_END_MEMBERS

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

	class B3D_CORE_EXPORT BloomSettingsRTTI : public RTTIType<BloomSettings, IReflectable, BloomSettingsRTTI>
	{
	private:
		B3D_RTTI_BEGIN_MEMBERS
			B3D_RTTI_MEMBER_PLAIN(Enabled, 0)
			B3D_RTTI_MEMBER_PLAIN(Quality, 1)
			B3D_RTTI_MEMBER_PLAIN(Threshold, 2)
			B3D_RTTI_MEMBER_PLAIN(Intensity, 3)
			B3D_RTTI_MEMBER_PLAIN(Tint, 4)
			B3D_RTTI_MEMBER_PLAIN(FilterSize, 5)
		B3D_RTTI_END_MEMBERS

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

	class B3D_CORE_EXPORT ScreenSpaceLensFlareSettingsRTTI : public RTTIType<ScreenSpaceLensFlareSettings, IReflectable, ScreenSpaceLensFlareSettingsRTTI>
	{
	private:
		B3D_RTTI_BEGIN_MEMBERS
			B3D_RTTI_MEMBER_PLAIN(Enabled, 0)
			B3D_RTTI_MEMBER_PLAIN(DownsampleCount, 1)
			B3D_RTTI_MEMBER_PLAIN(Threshold, 2)
			B3D_RTTI_MEMBER_PLAIN(GhostCount, 3)
			B3D_RTTI_MEMBER_PLAIN(GhostSpacing, 4)
			B3D_RTTI_MEMBER_PLAIN(Brightness, 5)
			B3D_RTTI_MEMBER_PLAIN(FilterSize, 6)
			B3D_RTTI_MEMBER_PLAIN(Halo, 7)
			B3D_RTTI_MEMBER_PLAIN(HaloAspectRatio, 8)
			B3D_RTTI_MEMBER_PLAIN(HaloRadius, 9)
			B3D_RTTI_MEMBER_PLAIN(HaloThickness, 10)
			B3D_RTTI_MEMBER_PLAIN(HaloThreshold, 11)
			B3D_RTTI_MEMBER_PLAIN(ChromaticAberration, 12)
			B3D_RTTI_MEMBER_PLAIN(ChromaticAberrationOffset, 13)
			B3D_RTTI_MEMBER_PLAIN(BicubicUpsampling, 14)
		B3D_RTTI_END_MEMBERS

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

	class B3D_CORE_EXPORT ChromaticAberrationSettingsRTTI : public RTTIType<ChromaticAberrationSettings, IReflectable, ChromaticAberrationSettingsRTTI>
	{
	private:
		B3D_RTTI_BEGIN_MEMBERS
			B3D_RTTI_MEMBER_PLAIN(Enabled, 0)
			B3D_RTTI_MEMBER_PLAIN(Type, 1)
			B3D_RTTI_MEMBER_PLAIN(ShiftAmount, 2)
			B3D_RTTI_MEMBER_REFL(FringeTexture, 3)
		B3D_RTTI_END_MEMBERS

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

	class B3D_CORE_EXPORT FilmGrainSettingsRTTI : public RTTIType<FilmGrainSettings, IReflectable, FilmGrainSettingsRTTI>
	{
	private:
		B3D_RTTI_BEGIN_MEMBERS
			B3D_RTTI_MEMBER_PLAIN(Enabled, 0)
			B3D_RTTI_MEMBER_PLAIN(Intensity, 1)
			B3D_RTTI_MEMBER_PLAIN(Speed, 2)
		B3D_RTTI_END_MEMBERS

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

	class B3D_CORE_EXPORT ShadowSettingsRTTI : public RTTIType<ShadowSettings, IReflectable, ShadowSettingsRTTI>
	{
	private:
		B3D_RTTI_BEGIN_MEMBERS
			B3D_RTTI_MEMBER_PLAIN(DirectionalShadowDistance, 0)
			B3D_RTTI_MEMBER_PLAIN(NumCascades, 1)
			B3D_RTTI_MEMBER_PLAIN(CascadeDistributionExponent, 2)
			B3D_RTTI_MEMBER_PLAIN(ShadowFilteringQuality, 3)
		B3D_RTTI_END_MEMBERS

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

	class B3D_CORE_EXPORT RenderSettingsRTTI : public RTTIType<RenderSettings, IReflectable, RenderSettingsRTTI>
	{
	private:
		B3D_RTTI_BEGIN_MEMBERS
			B3D_RTTI_MEMBER_PLAIN(EnableAutoExposure, 0)
			B3D_RTTI_MEMBER_REFL(AutoExposure, 1)
			B3D_RTTI_MEMBER_PLAIN(EnableTonemapping, 2)
			B3D_RTTI_MEMBER_REFL(Tonemapping, 3)
			B3D_RTTI_MEMBER_REFL(WhiteBalance, 4)
			B3D_RTTI_MEMBER_REFL(ColorGrading, 5)
			B3D_RTTI_MEMBER_PLAIN(ExposureScale, 6)
			B3D_RTTI_MEMBER_PLAIN(Gamma, 7)
			B3D_RTTI_MEMBER_REFL(DepthOfField, 8)
			B3D_RTTI_MEMBER_PLAIN(EnableFxaa, 9)
			B3D_RTTI_MEMBER_REFL(AmbientOcclusion, 10)
			B3D_RTTI_MEMBER_REFL(ScreenSpaceReflections, 11)
			B3D_RTTI_MEMBER_PLAIN(EnableHdr, 12)
			B3D_RTTI_MEMBER_PLAIN(EnableLighting, 13)
			B3D_RTTI_MEMBER_PLAIN(EnableShadows, 14)
			B3D_RTTI_MEMBER_PLAIN(OverlayOnly, 15)
			B3D_RTTI_MEMBER_PLAIN(EnableIndirectLighting, 16)
			B3D_RTTI_MEMBER_REFL(ShadowSettings, 17)
			B3D_RTTI_MEMBER_PLAIN(EnableSkybox, 18)
			B3D_RTTI_MEMBER_REFL(Bloom, 19)
			B3D_RTTI_MEMBER_REFL(ScreenSpaceLensFlare, 20)
			B3D_RTTI_MEMBER_REFL(MotionBlur, 21)
			B3D_RTTI_MEMBER_REFL(FilmGrain, 22)
			B3D_RTTI_MEMBER_REFL(ChromaticAberration, 23)
			B3D_RTTI_MEMBER_REFL(TemporalAa, 24)
			B3D_RTTI_MEMBER_PLAIN(EnableVelocityBuffer, 25)
		B3D_RTTI_END_MEMBERS

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
